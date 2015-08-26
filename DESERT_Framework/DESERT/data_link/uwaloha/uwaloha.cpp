//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the 
//    names of its contributors may be used to endorse or promote products 
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file   uwaloha.cpp
 * @author Saiful Azad
 * @version 1.0.0
 * 
 * @brief Class that provide the implementation of ALOHA protocol
 */


#include "uwaloha.h"
#include <mac.h>
#include <cmath>
#include <climits>
#include <iomanip>
#include <rng.h>

enum {
    NOT_SET = -1, SESSION_DISTANCE_NOT_SET = 0
};

/**
 * Class that describe the binding with tcl scripting language
 */
static class UWALOHAModuleClass : public TclClass {
public:

    /**
     * Constructor of the class
     */
    UWALOHAModuleClass() : TclClass("Module/UW/ALOHA") {
    }

    TclObject* create(int, const char*const*) {
        return (new UWAloha());
    }
} class_module_uwaloha;

void UWAloha::AckTimer::expire(Event *e) {
    timer_status = UWALOHA_EXPIRED;
    if (module->curr_state == UWALOHA_STATE_WAIT_ACK) {


        if (module->uwaloha_debug) cout << NOW << "  UWAloha (" << module->addr << ") timer expire() current state = "
                << module->status_info[module->curr_state] << "; ACK not received, next state = "
                << module->status_info[UWALOHA_STATE_BACKOFF] << endl;

        module->refreshReason(UWALOHA_REASON_ACK_TIMEOUT);
        module->stateBackoff();
    } else {
        if (module->uwaloha_debug) cout << NOW << "  UWAloha (" << module->addr << ")::AckTimer::expired() " << endl;
    }
}

void UWAloha::BackOffTimer::expire(Event *e) {
    timer_status = UWALOHA_EXPIRED;
    if (module->curr_state == UWALOHA_STATE_BACKOFF) {


        if (module->uwaloha_debug) cout << NOW << "  UWAloha (" << module->addr << ") timer expire() current state = "
                << module->status_info[module->curr_state] << "; backoff expired, next state = "
                << module->status_info[UWALOHA_STATE_IDLE] << endl;

        module->refreshReason(UWALOHA_REASON_BACKOFF_TIMEOUT);
        module->exitBackoff();
        module->stateIdle();
    } else {
        if (module->uwaloha_debug) cout << NOW << "  UWAloha (" << module->addr << ")::BackOffTimer::expired() " << endl;
    }
}

const double UWAloha::prop_speed = 1500.0;
bool UWAloha::initialized = false;


map< UWAloha::UWALOHA_STATUS, string> UWAloha::status_info;
map< UWAloha::UWALOHA_REASON_STATUS, string> UWAloha::reason_info;
map< UWAloha::UWALOHA_PKT_TYPE, string> UWAloha::pkt_type_info;

UWAloha::UWAloha()
: ack_timer(this),
backoff_timer(this),
txsn(1),
last_sent_data_id(-1),
curr_data_pkt(0),
last_data_id_rx(NOT_SET),
curr_tx_rounds(0),
print_transitions(false),
has_buffer_queue(true),
curr_state(UWALOHA_STATE_IDLE),
prev_state(UWALOHA_STATE_IDLE),
prev_prev_state(UWALOHA_STATE_IDLE),
ack_mode(UWALOHA_NO_ACK_MODE),
last_reason(UWALOHA_REASON_NOT_SET),
start_tx_time(0),
recv_data_id(-1),
srtt(0),
sumrtt(0),
sumrtt2(0),
rttsamples(0) {
    mac2phy_delay_ = 1e-19;

    bind("HDR_size_", (int*) & HDR_size);
    bind("ACK_size_", (int*) & ACK_size);
    bind("max_tx_tries_", (int*) & max_tx_tries);
    bind("wait_constant_", (double*) & wait_constant);
    bind("uwaloha_debug_", (int*) & uwaloha_debug); //degug mode
    bind("max_payload_", (int*) & max_payload);
    bind("ACK_timeout_", (double*) & ACK_timeout);
    bind("alpha_", (double*) &alpha_);
    bind("buffer_pkts_", (int*) &buffer_pkts);
    bind("backoff_tuner_", (double*) &backoff_tuner);
    bind("max_backoff_counter_", (int*) &max_backoff_counter);
    //bind("MAC_addr_", (int*)&addr);

    if (max_tx_tries <= 0) max_tx_tries = INT_MAX;
    if (buffer_pkts > 0) has_buffer_queue = true;
}

UWAloha::~UWAloha() {

}

// TCL command interpreter

int UWAloha::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "setAckMode") == 0) {
            ack_mode = UWALOHA_ACK_MODE;
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setNoAckMode") == 0) {
            ack_mode = UWALOHA_NO_ACK_MODE;
            return TCL_OK;
        } else if (strcasecmp(argv[1], "initialize") == 0) {
            if (initialized == false) initInfo();
            if (print_transitions) fout.open("/tmp/ALOHAstateTransitions.txt", ios_base::app);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "printTransitions") == 0) {
            print_transitions = true;
            return TCL_OK;
        }            // stats functions
        else if (strcasecmp(argv[1], "getQueueSize") == 0) {
            tcl.resultf("%d", mapPacket.size());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getUpLayersDataRx") == 0) {
            tcl.resultf("%d", getUpLayersDataPktsRx());
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "setMacAddr") == 0) {
            addr = atoi(argv[2]);
            if (debug_) cout << "Aloha MAC address of current node is " << addr << endl;
            return TCL_OK;
        }
    }
    return MMac::command(argc, argv);
}

void UWAloha::initInfo() {

    initialized = true;

    if ((print_transitions) && (system(NULL))) {
        system("rm -f /tmp/ALOHAstateTransitions.txt");
        system("touch /tmp/ALOHAstateTransitions.txt");
    }

    status_info[UWALOHA_STATE_IDLE] = "Idle state";
    status_info[UWALOHA_STATE_TX_DATA] = "Transmit DATA state";
    status_info[UWALOHA_STATE_TX_ACK] = "Transmit ACK state";
    status_info[UWALOHA_STATE_WAIT_ACK] = "Wait for ACK state";
    status_info[UWALOHA_STATE_DATA_RX] = "DATA received state";
    status_info[UWALOHA_STATE_ACK_RX] = "ACK received state";
    status_info[UWALOHA_STATE_RX_IDLE] = "Start rx Idle state";
    status_info[UWALOHA_STATE_RX_WAIT_ACK] = "Start rx Wait ACK state";
    status_info[UWALOHA_STATE_CHK_ACK_TIMEOUT] = "Check Wait ACK timeout state";
    status_info[UWALOHA_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";
    status_info[UWALOHA_STATE_BACKOFF] = "Backoff state";
    status_info[UWALOHA_STATE_RX_BACKOFF] = "Start rx Backoff state";
    status_info[UWALOHA_STATE_CHK_BACKOFF_TIMEOUT] = "Check Backoff timeout state";

    reason_info[UWALOHA_REASON_DATA_PENDING] = "DATA pending from upper layers";
    reason_info[UWALOHA_REASON_DATA_RX] = "DATA received";
    reason_info[UWALOHA_REASON_DATA_TX] = "DATA transmitted";
    reason_info[UWALOHA_REASON_ACK_TX] = "ACK tranmsitted";
    reason_info[UWALOHA_REASON_ACK_RX] = "ACK received";
    reason_info[UWALOHA_REASON_ACK_TIMEOUT] = "ACK timeout";
    reason_info[UWALOHA_REASON_DATA_EMPTY] = "DATA queue empty";
    reason_info[UWALOHA_REASON_MAX_TX_TRIES] = "DATA dropped due to max tx rounds";
    reason_info[UWALOHA_REASON_START_RX] = "Start rx pkt";
    reason_info[UWALOHA_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
    reason_info[UWALOHA_REASON_WAIT_ACK_PENDING] = "Wait for ACK timer pending";
    reason_info[UWALOHA_REASON_PKT_ERROR] = "Erroneous pkt";
    reason_info[UWALOHA_REASON_BACKOFF_TIMEOUT] = "Backoff expired";
    reason_info[UWALOHA_REASON_BACKOFF_PENDING] = "Backoff timer pending";

    pkt_type_info[UWALOHA_ACK_PKT] = "ACK pkt";
    pkt_type_info[UWALOHA_DATA_PKT] = "DATA pkt";
    pkt_type_info[UWALOHA_DATAMAX_PKT] = "MAX payload DATA pkt";
}

void UWAloha::updateRTT(double curr_rtt) {
    srtt = alpha_ * srtt + (1 - alpha_) * curr_rtt;
    sumrtt += curr_rtt;
    sumrtt2 += curr_rtt*curr_rtt;
    rttsamples++;
    ACK_timeout = (sumrtt / rttsamples);
}

void UWAloha::updateAckTimeout(double rtt) {
    updateRTT(rtt);
    //   double curr_rtt = getRTT();

    //   if (curr_rtt > 0) ACK_timeout = min(ACK_timeout, getRTT() );

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::updateAckTimeout() curr ACK_timeout = "
            << ACK_timeout << endl;
    //   waitForUser();
}

void UWAloha::exitBackoff() {
    backoff_timer.stop();
}

double UWAloha::getBackoffTime() {
    incrTotalBackoffTimes();
    double random = RNG::defaultrng()->uniform_double();

    backoff_timer.incrCounter();
    double counter = backoff_timer.getCounter();
    if (counter > max_backoff_counter) counter = max_backoff_counter;

    double backoff_duration = backoff_tuner * random * 2.0 * ACK_timeout * pow(2.0, counter);

    backoffSumDuration(backoff_duration);

    if (uwaloha_debug) {
        cout << NOW << "  UWAloha (" << addr << ")::getBackoffTime() backoff time = "
                << backoff_duration << " s" << endl;
    }
    return backoff_duration;
}

double UWAloha::computeTxTime(UWALOHA_PKT_TYPE type) {
    double duration;
    Packet* temp_data_pkt;
    map< pktSeqNum, Packet*> ::iterator it_p;

    if (type == UWALOHA_DATA_PKT) {
        if (!mapPacket.empty()) {
            it_p = mapPacket.begin();
            temp_data_pkt = ((*it_p).second)->copy();
            //temp_data_pkt = (Q.front())->copy();  
            hdr_cmn *ch = HDR_CMN(temp_data_pkt);
            ch->size() = HDR_size + ch->size();
        } else {
            temp_data_pkt = Packet::alloc();
            hdr_cmn *ch = HDR_CMN(temp_data_pkt);
            ch->size() = HDR_size + max_payload;
        }
    } else if (type == UWALOHA_ACK_PKT) {
        temp_data_pkt = Packet::alloc();
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = ACK_size;
    }
    duration = Mac2PhyTxDuration(temp_data_pkt);
    Packet::free(temp_data_pkt);
    return (duration);
}

void UWAloha::recvFromUpperLayers(Packet* p) {
    if (((has_buffer_queue == true) && (mapPacket.size() < buffer_pkts)) || (has_buffer_queue == false)) {
        initPkt(p, UWALOHA_DATA_PKT);
        //Q.push(p);
        putPktInQueue(p);
        incrUpperDataRx();
        waitStartTime();

        if (curr_state == UWALOHA_STATE_IDLE) {
            refreshReason(UWALOHA_REASON_DATA_PENDING);
            stateTxData();

            //          /* if (uwaloha_debug) */ 
            // 	   cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
            // 	       << " mac busy => enqueueing packet" << endl;
        } else {
            //          /* if (uwaloha_debug) */ 
            // 	   cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
            // 	        << " transmitting packet" << endl;
            return; // I'm still waiting to go to IDLE status.
        }
    } else {
        incrDiscardedPktsTx();
        drop(p, 1, UWALOHA_DROP_REASON_BUFFER_FULL);
    }
}

void UWAloha::initPkt(Packet* p, UWALOHA_PKT_TYPE type, int dest_addr) {
    hdr_cmn* ch = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);

    int curr_size = ch->size();

    switch (type) {

        case(UWALOHA_DATA_PKT):
        {
            ch->size() = curr_size + HDR_size;
        }
            break;

        case(UWALOHA_ACK_PKT):
        {
            ch->ptype() = PT_MMAC_ACK;
            ch->size() = ACK_size;
            ch->uid() = recv_data_id;
            mach->macSA() = addr;
            mach->macDA() = dest_addr;
        }
            break;

    }

}

void UWAloha::Mac2PhyStartTx(Packet* p) {
    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Mac2PhyStartTx() start tx packet" << endl;

    MMac::Mac2PhyStartTx(p);
}

void UWAloha::Phy2MacEndTx(const Packet* p) {

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Phy2MacEndTx() end tx packet" << endl;

    switch (curr_state) {

        case(UWALOHA_STATE_TX_DATA):
        {
            refreshReason(UWALOHA_REASON_DATA_TX);
            if (ack_mode == UWALOHA_ACK_MODE) {

                if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Phy2MacEndTx() DATA sent,from "
                        << status_info[curr_state] << " to "
                        << status_info[UWALOHA_STATE_WAIT_ACK] << endl;

                stateWaitAck();
            } else {

                if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Phy2MacEndTx() DATA sent, from "
                        << status_info[curr_state] << " to " << status_info[UWALOHA_STATE_IDLE] << endl;

                stateIdle();
            }
        }
            break;

        case(UWALOHA_STATE_TX_ACK):
        {
            refreshReason(UWALOHA_REASON_ACK_TX);

            if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Phy2MacEndTx() ack sent, from "
                    << status_info[curr_state] << " to " << status_info[UWALOHA_STATE_IDLE] << endl;
            stateIdle();
        }
            break;

        default:
        {
            cout << NOW << "  UWAloha (" << addr << ")::Phy2MacEndTx() logical error, current state = "
                    << status_info[curr_state] << endl;
            stateIdle();
            //exit(1);
        }
            break;

    }

}

void UWAloha::Phy2MacStartRx(const Packet* p) {
    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Phy2MacStartRx() rx Packet " << endl;

}

void UWAloha::Phy2MacEndRx(Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    packet_t rx_pkt_type = ch->ptype();
    hdr_mac* mach = HDR_MAC(p);
    hdr_MPhy* ph = HDR_MPHY(p);

    int source_mac = mach->macSA();
    int dest_mac = mach->macDA();

    double gen_time = ph->txtime;
    double received_time = ph->rxtime;
    double diff_time = received_time - gen_time;

    double distance = diff_time * prop_speed;

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Phy2MacEndRx() "
            << status_info[curr_state] << ", received a pkt type = "
            << ch->ptype() << ", src addr = " << mach->macSA()
        << " dest addr = " << mach->macDA()
        << ", estimated distance between nodes = " << distance << " m " << endl;

    if (ch->error()) {

        if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::Phy2MacEndRx() dropping corrupted pkt " << endl;
        incrErrorPktsRx();

        refreshReason(UWALOHA_REASON_PKT_ERROR);
        drop(p, 1, UWALOHA_DROP_REASON_ERROR);
        return;
    } else {
        if (dest_mac == addr || dest_mac == MAC_BROADCAST) {
            if (rx_pkt_type == PT_MMAC_ACK) {
                refreshReason(UWALOHA_REASON_ACK_RX);
                stateRxAck(p);
            } else if (curr_state != UWALOHA_STATE_WAIT_ACK) {
                refreshReason(UWALOHA_REASON_DATA_RX);
                stateRxData(p);
            } else {
                refreshReason(UWALOHA_REASON_PKT_NOT_FOR_ME);
                //Packet::free(p);
                drop(p, 1, UWALOHA_DROP_REASON_WRONG_RECEIVER);
                return;
            }
        } else {
            refreshReason(UWALOHA_REASON_PKT_NOT_FOR_ME);
            //      Packet::free(p);
            drop(p, 1, UWALOHA_DROP_REASON_WRONG_RECEIVER);
            return;
        }
    }
}

void UWAloha::txData() {
    Packet* data_pkt = curr_data_pkt->copy();

    if ((ack_mode == UWALOHA_NO_ACK_MODE)) {
        eraseItemFromPktQueue(getPktSeqNum(data_pkt));
    }

    incrDataPktsTx();
    incrCurrTxRounds();
    Mac2PhyStartTx(data_pkt);
}

void UWAloha::txAck(int dest_addr) {
    Packet* ack_pkt = Packet::alloc();
    initPkt(ack_pkt, UWALOHA_ACK_PKT, dest_addr);

    incrAckPktsTx();
    Mac2PhyStartTx(ack_pkt);
}

void UWAloha::stateCheckAckExpired() {
    refreshState(UWALOHA_STATE_CHK_ACK_TIMEOUT);

   if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateCheckAckExpired()" << endl;

    map< pktSeqNum, AckTimer> ::iterator it_a;
    it_a = mapAckTimer.begin();


    if (print_transitions) printStateInfo();
    if (((*it_a).second).isActive()) {
        refreshReason(UWALOHA_REASON_WAIT_ACK_PENDING);
        refreshState(UWALOHA_STATE_WAIT_ACK);
    } else if (((*it_a).second).isExpired()) {
        refreshReason(UWALOHA_REASON_ACK_TIMEOUT);
        stateBackoff();
    } else {
        cerr << NOW << "  UWAloha (" << addr << ")::stateCheckAckExpired() ack_timer logical error, current timer state = "
                << status_info[curr_state] << endl;
        stateIdle();
        //exit(1);  
    }
}

void UWAloha::stateCheckBackoffExpired() {
    refreshState(UWALOHA_STATE_CHK_BACKOFF_TIMEOUT);

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateCheckBackoffExpired()" << endl;
    if (print_transitions) printStateInfo();
    if (backoff_timer.isActive()) {
        refreshReason(UWALOHA_REASON_BACKOFF_PENDING);
        stateBackoff();
    } else if (backoff_timer.isExpired()) {
        refreshReason(UWALOHA_REASON_BACKOFF_TIMEOUT);
        exitBackoff();
        stateIdle();
    } else {
        cerr << NOW << "  UWAloha (" << addr << ")::stateCheckBackoffExpired() backoff_timer logical error, current timer state = "
                << status_info[curr_state] << endl;
        stateIdle();
        //exit(1);  
    }
}

void UWAloha::stateIdle() {
    mapAckTimer.clear();
    backoff_timer.stop();

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateIdle() queue size = " << mapPacket.size() << endl;

    refreshState(UWALOHA_STATE_IDLE);

    if (print_transitions) printStateInfo();

    if (!mapPacket.empty()) {
        stateTxData();
    }
}

void UWAloha::stateRxIdle() {
    refreshState(UWALOHA_STATE_RX_IDLE);

    if (print_transitions) printStateInfo();
}

void UWAloha::stateBackoff() {
    backoff_timer.force_cancel();
    refreshState(UWALOHA_STATE_BACKOFF);

    if (backoff_timer.isFrozen()) backoff_timer.unFreeze();
    else backoff_timer.schedule(getBackoffTime());

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateBackoff() " << endl;
    if (print_transitions) printStateInfo(backoff_timer.getDuration());
}

void UWAloha::stateRxBackoff() {
    backoff_timer.freeze();
    refreshState(UWALOHA_STATE_RX_BACKOFF);

    if (print_transitions) printStateInfo();
}

void UWAloha::stateTxData() {
    refreshState(UWALOHA_STATE_TX_DATA);

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateTxData() " << endl;
    if (print_transitions) printStateInfo();

    map< pktSeqNum, Packet*> ::iterator it_p;
    it_p = mapPacket.begin();
    curr_data_pkt = (*it_p).second;
    int seq_num;
    seq_num = getPktSeqNum(curr_data_pkt);


    map< pktSeqNum, AckTimer> ::iterator it_a;

    if (seq_num != last_sent_data_id) {
        putAckTimerInMap(seq_num);
        it_a = mapAckTimer.find(seq_num);
        resetCurrTxRounds();
        backoff_timer.resetCounter();
        ((*it_a).second).resetCounter();
        hdr_mac* mach = HDR_MAC(curr_data_pkt);
        start_tx_time = NOW; // we set curr RTT
        last_sent_data_id = seq_num;
        txData();
    }
    else {

        if (mapAckTimer.size() == 0) {
            putAckTimerInMap(seq_num);
            it_a = mapAckTimer.find(seq_num);
            ((*it_a).second).resetCounter();
            incrCurrTxRounds();
            backoff_timer.incrCounter();
            if (curr_tx_rounds < max_tx_tries) {
                hdr_mac* mach = HDR_MAC(curr_data_pkt);
                start_tx_time = NOW; // we set curr RTT
                last_sent_data_id = seq_num;
                txData();
            }
            else {
                eraseItemFromPktQueue(seq_num);
                incrDroppedPktsTx();

                refreshReason(UWALOHA_REASON_MAX_TX_TRIES);

                if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::statePreTxData() curr_tx_rounds " << curr_tx_rounds
                        << " > max_tx_tries = " << max_tx_tries << endl;

                stateIdle();
            }
        } else {
            stateCheckAckExpired();
        }
    }
}

void UWAloha::stateWaitAck() {

    map< pktSeqNum, AckTimer> ::iterator it_a;
    it_a = mapAckTimer.begin();

    ((*it_a).second).stop();
    refreshState(UWALOHA_STATE_WAIT_ACK);

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateWaitAck() " << endl;
    if (print_transitions) printStateInfo();

    ((*it_a).second).incrCounter();
    ((*it_a).second).schedule(ACK_timeout + 2 * wait_constant);
}

void UWAloha::stateRxWaitAck() {
    refreshState(UWALOHA_STATE_RX_WAIT_ACK);

    if (print_transitions) printStateInfo();
}

void UWAloha::stateTxAck(int dest_addr) {
    refreshState(UWALOHA_STATE_TX_ACK);

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateTxAck() dest addr " << dest_addr << endl;
    if (print_transitions) printStateInfo();

    txAck(dest_addr);
}

void UWAloha::stateRxData(Packet* data_pkt) {
    ack_timer.stop();
    refreshState(UWALOHA_STATE_DATA_RX);

    if (uwaloha_debug) cout << NOW << "  UWAloha (" << addr << ")::stateRxData() " << endl;
    refreshReason(UWALOHA_REASON_DATA_RX);

    hdr_mac* mach = HDR_MAC(data_pkt);
    int dst_addr = mach->macSA();

    hdr_cmn* ch = hdr_cmn::access(data_pkt);
    recv_data_id = ch->uid();
    ch->size() = ch->size() - HDR_size;
    incrDataPktsRx();
    sendUp(data_pkt);

    if (ack_mode == UWALOHA_ACK_MODE) stateTxAck(dst_addr);
    else stateIdle();

}

void UWAloha::stateRxAck(Packet* p) {

    map< pktSeqNum, AckTimer> ::iterator it_a;
    it_a = mapAckTimer.begin();

    ((*it_a).second).stop();
    refreshState(UWALOHA_STATE_ACK_RX);
    if (uwaloha_debug) cout << NOW << " UWAloha (" << addr << ")::stateRxAck() " << endl;

    int seq_num;
    seq_num = getPktSeqNum(p);

    Packet::free(p);

    refreshReason(UWALOHA_REASON_ACK_RX);

    eraseItemFromPktQueue(seq_num);
    eraseItemFrommapAckTimer(seq_num);
    updateAckTimeout(NOW - start_tx_time);
    incrAckPktsRx();
    stateIdle();

}

void UWAloha::printStateInfo(double delay) {
    if (uwaloha_debug) cout << NOW << " UWAloha (" << addr << ")::printStateInfo() " << "from " << status_info[prev_state]
            << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;

    if (curr_state == UWALOHA_STATE_BACKOFF) {
        fout << left << setw(10) << NOW << "  UWAloha (" << addr << ")::printStateInfo() "
                << "from " << status_info[prev_state]
                << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason]
                << ". Backoff duration = " << delay << endl;
    } else {
        fout << left << setw(10) << NOW << "  UWAloha (" << addr << ")::printStateInfo() "
                << "from " << status_info[prev_state]
                << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;
    }
}

void UWAloha::waitForUser() {
    std::string response;
    std::cout << "Press Enter to continue";
    std::getline(std::cin, response);
}
