//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file   uw-csma-aloha-triggered-node.cc
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the implementation of UwCsmaAloha_Triggered_NODE class
 *
 */


#include "uw-csma-aloha-triggered-node.h"
#include <mac.h>
#include <cmath>
#include <climits>
#include <iomanip>
#include <rng.h>

enum {
    NOT_SET = -1, SESSION_DISTANCE_NOT_SET = 0
};

static class UwCsmaAloha_Triggered_NODEModuleClass : public TclClass {
public:

    UwCsmaAloha_Triggered_NODEModuleClass() : TclClass("Module/UW/CSMA_ALOHA/TRIGGERED/NODE") {
    }

    TclObject* create(int, const char*const*) {
        return (new UwCsmaAloha_Triggered_NODE());
    }
} class_module_uw_csma_aloha_triggered_node;

void UwCsmaAloha_Triggered_NODE::ListenTimer::expire(Event *e) {
    timer_status = UW_CS_ALOHA_TRIG_NODE_EXPIRED;

    if (module->curr_state == UW_CS_ALOHA_TRIG_NODE_STATE_LISTEN) {

        if (module->debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << module->addr << ") timer expire() current state = "
                << module->status_info[module->curr_state] << "; listening period expired, next state = "
                << module->status_info[UW_CS_ALOHA_TRIG_NODE_STATE_TX_DATA] << endl;

        module->refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_TIMEOUT);
        module->stateTxData();
    } else {
        if (module->debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << module->addr << ")::ListenTimer::expired() " << endl;
    }
}

void UwCsmaAloha_Triggered_NODE::TransmissionTimer::expire(Event *e) {
    timer_status = UW_CS_ALOHA_TRIG_NODE_EXPIRED;
    if (module->can_transmit == true) {
        if (module->debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << module->addr << ") Transmission timer expire() current state = "
                << module->status_info[module->curr_state] << endl;
        module->can_transmit = false;
    } else {
        if (module->debug_) std::cerr << "Transmission Timer expired, can_transmit == false" << std::endl;
    }
}


const double UwCsmaAloha_Triggered_NODE::prop_speed = 1500.0;
int UwCsmaAloha_Triggered_NODE::u_pkt_id;
bool UwCsmaAloha_Triggered_NODE::initialized = false;


map< UwCsmaAloha_Triggered_NODE::UW_CS_ALOHA_TRIG_NODE_STATUS, string> UwCsmaAloha_Triggered_NODE::status_info;
map< UwCsmaAloha_Triggered_NODE::UW_CS_ALOHA_TRIG_NODE_REASON_STATUS, string> UwCsmaAloha_Triggered_NODE::reason_info;

UwCsmaAloha_Triggered_NODE::UwCsmaAloha_Triggered_NODE()
: listen_timer(this),
tx_timer(this),
u_data_id(0),
last_sent_data_id(-1),
curr_data_pkt(0),
last_data_id_rx(NOT_SET),
print_transitions(false),
has_buffer_queue(false),
curr_state(UW_CS_ALOHA_TRIG_NODE_STATE_IDLE),
prev_state(UW_CS_ALOHA_TRIG_NODE_STATE_IDLE),
prev_prev_state(UW_CS_ALOHA_TRIG_NODE_STATE_IDLE),
last_reason(UW_CS_ALOHA_TRIG_NODE_REASON_NOT_SET)
{
    u_pkt_id = 0;
    mac2phy_delay_ = 1e-19;

    bind("HDR_size_", (int*) & HDR_size);
    bind("debug_", (double*) &debug_);
    bind("max_payload_", (int*) &max_payload);
    bind("buffer_pkts_", (int*) &buffer_pkts);
    bind("listen_time_", &listen_time);
    bind("tx_timer_duration_", (int*) &tx_timer_duration);

    if (buffer_pkts > 0) has_buffer_queue = true;
    if (listen_time <= 0.0) listen_time = 1e-19;
}

UwCsmaAloha_Triggered_NODE::~UwCsmaAloha_Triggered_NODE() {

}

// TCL command interpreter

int UwCsmaAloha_Triggered_NODE::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            if (initialized == false) initInfo();
            if (print_transitions) fout.open("/tmp/ALOHAstateTransitions.txt", ios_base::app);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "printTransitions") == 0) {
            print_transitions = true;
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getQueueSize") == 0) {
            tcl.resultf("%d", Q.size());
            return TCL_OK;
        }
    }
    return MMac::command(argc, argv);
}

int UwCsmaAloha_Triggered_NODE::crLayCommand(ClMessage* m) {
    switch (m->type()) {

        default:
            return Module::crLayCommand(m);
    }
}

void UwCsmaAloha_Triggered_NODE::initInfo() {

    initialized = true;

    if ((print_transitions) && (system(NULL))) 
    {
        if (! system("rm -f /tmp/UW_CSMA_ALOHA_TRIGGERED_NODE.txt"))
        {
            std::cerr << "Cannot rm -f /tmp/UW_CSMA_ALOHA_TRIGGERED_NODE.txt";
        }
        if (! system("touch /tmp/UW_CSMA_ALOHA_TRIGGERED_NODE.txt") )
        {
            std::cerr << "Cannot touch /tmp/UW_CSMA_ALOHA_TRIGGERED_NODE.txt";
        }
    }

    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_IDLE] = "Idle state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_TX_DATA] = "Transmit DATA state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_DATA_RX] = "DATA received state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_LISTEN] = "Listening channel state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_RX_IDLE] = "Start rx Idle state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_RX_LISTEN] = "Start rx Listen state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_CHK_LISTEN_TIMEOUT] = "Check Listen timeout state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";
    status_info[UW_CS_ALOHA_TRIG_NODE_STATE_RX_TRIGGER] = "Received a TRIGGER packet from AUV";

    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_DATA_PENDING] = "DATA pending from upper layers";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_DATA_RX] = "DATA received";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_DATA_TX] = "DATA transmitted";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_DATA_EMPTY] = "DATA queue empty";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN] = "DATA pending, listening to channel";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_TIMEOUT] = "DATA pending, end of listening period";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_START_RX] = "Start rx pkt";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_PENDING] = "Listen to channel pending";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_PKT_ERROR] = "Erroneous pkt";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_TRIGGER_RX] = "Received a TRIGGER packet from AUV";
    reason_info[UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_AFTER_TRIGGER] = "TRIGGER received ---> Listen to the channel";

}



void UwCsmaAloha_Triggered_NODE::recvFromUpperLayers(Packet* p) {
    if ((has_buffer_queue == true) && ((Q.size()) < (buffer_pkts)) || (has_buffer_queue == false)) {
        initPkt(p);
        Q.push(p);
        incrUpperDataRx();
        waitStartTime();

        if (curr_state == UW_CS_ALOHA_TRIG_NODE_STATE_IDLE) {
            refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_DATA_PENDING);
            stateListen();
        } else {
            if (debug_)
                cerr << showpoint << NOW << " " << __PRETTY_FUNCTION__
                    << " transmitting packet" << endl;
        }
    } else {
        incrDiscardedPktsTx();
        drop(p, 1, UW_CS_ALOHA_TRIG_NODE_DROP_REASON_BUFFER_FULL);
    }
}

void UwCsmaAloha_Triggered_NODE::initPkt(Packet* p) {
    hdr_cmn* ch = hdr_cmn::access(p);
    int curr_size = ch->size();

    ch->size() = curr_size + HDR_size;
    data_sn_queue.push(u_data_id);
    u_data_id++;
}

void UwCsmaAloha_Triggered_NODE::Mac2PhyStartTx(Packet* p) {
    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Mac2PhyStartTx() start tx packet" << endl;

    MMac::Mac2PhyStartTx(p);
}

void UwCsmaAloha_Triggered_NODE::Phy2MacEndTx(const Packet* p) {


    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Phy2MacEndTx() end tx packet" << endl;

    switch (curr_state) {

        case(UW_CS_ALOHA_TRIG_NODE_STATE_TX_DATA):
        {
            refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_DATA_TX);

            if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Phy2MacEndTx() DATA sent, from "
                    << status_info[curr_state] << " to " << status_info[UW_CS_ALOHA_TRIG_NODE_STATE_IDLE] << endl;

            stateIdle();
        }
            break;

        default:
        {
            cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Phy2MacEndTx() logical error, current state = "
                    << status_info[curr_state] << endl;
        }
            break;

    }

}

void UwCsmaAloha_Triggered_NODE::Phy2MacStartRx(const Packet* p) {
    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Phy2MacStartRx() rx Packet " << endl;

    refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_START_RX);

    switch (curr_state) {

        case(UW_CS_ALOHA_TRIG_NODE_STATE_IDLE):
            stateRxIdle();
            break;

        case(UW_CS_ALOHA_TRIG_NODE_STATE_LISTEN):
            stateRxListen();
            break;
        default:
        {
            if (debug_) std::cerr << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Phy2MacStartRx() logical warning, current state = "
                    << status_info[curr_state] << std::endl;
        }

    }
}

void UwCsmaAloha_Triggered_NODE::Phy2MacEndRx(Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    packet_t rx_pkt_type = ch->ptype();
    hdr_mac* mach = HDR_MAC(p);
    hdr_MPhy* ph = HDR_MPHY(p);

    int dest_mac = mach->macDA();

    double gen_time = ph->txtime;
    double received_time = ph->rxtime;
    double diff_time = received_time - gen_time;

    double distance = diff_time * prop_speed;

    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Phy2MacEndRx() "
            << status_info[curr_state] << ", received a pkt type = "
            << ch->ptype() << ", src addr = " << mach->macSA()
        << " dest addr = " << mach->macDA()
        << ", estimated distance between nodes = " << distance << " m " << endl;

    if (ch->error()) {

        if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::Phy2MacEndRx() dropping corrupted pkt " << endl;
        incrErrorPktsRx();

        refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_PKT_ERROR);
        drop(p, 1, UW_CS_ALOHA_TRIG_NODE_DROP_REASON_ERROR);
        stateRxPacketNotForMe(NULL);
    } else {
        if (dest_mac == (int)addr || dest_mac == (int)MAC_BROADCAST) {
            if (rx_pkt_type == (int)PT_MMAC_TRIGGER) {
                refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_TRIGGER_RX);
                stateRxTrigger(p);
            } else {
                refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_PKT_NOT_FOR_ME);
                stateRxPacketNotForMe(p);
            }
        } else {
            refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_PKT_NOT_FOR_ME);
            stateRxPacketNotForMe(p);
        }
    }
}

void UwCsmaAloha_Triggered_NODE::stateRxTrigger(Packet* p) {
    if (debug_) cout << NOW << "Csma_Aloha_NODE(" << addr << ")::stateRxTrigger---> can_transmit" << can_transmit << endl;
    if (can_transmit == false) {
        refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_RX_TRIGGER);
        can_transmit = true;
        tx_timer.schedule(tx_timer_duration);
        if (!Q.empty()) {
            refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_AFTER_TRIGGER);
            stateListen();
        }
    } else {
        if (debug_) cout << NOW << "Csma_Aloha_NODE(" << addr << ")::stateRxTrigger---> can_transmit already true " << endl;
    }
}

void UwCsmaAloha_Triggered_NODE::txData() {
    Packet* data_pkt = curr_data_pkt->copy();
    queuePop();

    incrDataPktsTx();
    Mac2PhyStartTx(data_pkt);
}

void UwCsmaAloha_Triggered_NODE::stateRxPacketNotForMe(Packet* p) {
    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateRxPacketNotForMe() pkt for another address. Dropping pkt" << endl;
    if (p != NULL) Packet::free(p);

    refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_WRONG_PKT_RX);

    switch (prev_state) {

        case UW_CS_ALOHA_TRIG_NODE_STATE_RX_IDLE:
            stateIdle();
            break;

        case UW_CS_ALOHA_TRIG_NODE_STATE_RX_LISTEN:
            stateCheckListenExpired();
            break;

        default:
            if (debug_) cerr << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateRxPacketNotForMe() logical error, prev state = "
                    << status_info[prev_state] << endl;
            //exit(1);

    }
}

void UwCsmaAloha_Triggered_NODE::stateCheckListenExpired() {
    refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_CHK_LISTEN_TIMEOUT);

    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateCheckListenExpired()" << endl;
    if (print_transitions) printStateInfo();
    if (listen_timer.isActive()) {
        refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_PENDING);
        refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_LISTEN);
    } else if (listen_timer.isExpired()) {
        refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_TIMEOUT);
        if (!(prev_state == UW_CS_ALOHA_TRIG_NODE_STATE_WRONG_PKT_RX || prev_state == UW_CS_ALOHA_TRIG_NODE_STATE_DATA_RX)) stateTxData();
        else stateListen();
    } else {
        if (debug_) cerr << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateCheckListenExpired() listen_timer logical error, current timer state = "
                << status_info[curr_state] << endl;
    }
}

void UwCsmaAloha_Triggered_NODE::stateCheckTxTimerExpired() {
}

void UwCsmaAloha_Triggered_NODE::stateIdle() {
    listen_timer.stop();

    refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_IDLE);

    if (print_transitions) printStateInfo();

    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateIdle() queue size = " << Q.size() << endl;
    ////////////////
    if (can_transmit) {
        if (!Q.empty()) {
            refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN);
            stateListen();
        }
    }
    ///////////////////
}

void UwCsmaAloha_Triggered_NODE::stateRxIdle() {
    refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_RX_IDLE);

    if (print_transitions) printStateInfo();
}

void UwCsmaAloha_Triggered_NODE::stateListen() {
    listen_timer.stop();
    refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_LISTEN);

    listen_timer.incrCounter();

    double time = listen_time * RNG::defaultrng()->uniform_double() + wait_costant;

    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateListen() listen time = " << time << endl;

    if (print_transitions) printStateInfo();

    listen_timer.schedule(time);
}

void UwCsmaAloha_Triggered_NODE::stateRxListen() {
    refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_RX_LISTEN);

    if (print_transitions) printStateInfo();
}

void UwCsmaAloha_Triggered_NODE::stateTxData() {
    if (can_transmit) {
        refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_TX_DATA);


        if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateTxData() " << endl;
        if (print_transitions) printStateInfo();

        curr_data_pkt = Q.front();

        if (data_sn_queue.front() != last_sent_data_id) {
            listen_timer.resetCounter();
        }
        last_sent_data_id = data_sn_queue.front();
        txData();
    } else {
        if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateTxData() ---> Not Allowed To Transmit " << endl;
    }
}

void UwCsmaAloha_Triggered_NODE::stateRxData(Packet* data_pkt) {
    refreshState(UW_CS_ALOHA_TRIG_NODE_STATE_DATA_RX);

    if (debug_) cout << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::stateRxData() " << endl;
    refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_DATA_RX);


    switch (prev_state) {

        case UW_CS_ALOHA_TRIG_NODE_STATE_RX_IDLE:
        {
            hdr_cmn* ch = hdr_cmn::access(data_pkt);
            ch->size() = ch->size() - HDR_size;
            incrDataPktsRx();
            sendUp(data_pkt);
            stateIdle();
        }
            break;

        case UW_CS_ALOHA_TRIG_NODE_STATE_RX_LISTEN:
        {
            hdr_cmn* ch = hdr_cmn::access(data_pkt);
            ch->size() = ch->size() - HDR_size;
            incrDataPktsRx();
            sendUp(data_pkt);
            stateCheckListenExpired();
        }
            break;



        default:

            if (debug_) cerr << NOW << " UwCsmaAloha_Triggered_NODE(" << addr << ")::stateRxData() logical error, prev state = " << status_info[prev_state]
                    << endl;

    }
}

void UwCsmaAloha_Triggered_NODE::printStateInfo(double delay) {
    if (debug_) cout << NOW << " UwCsmaAloha_Triggered_NODE(" << addr << ")::printStateInfo() " << "from " << status_info[prev_state]
            << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;


    fout << left << setw(10) << NOW << "  UwCsmaAloha_Triggered_NODE(" << addr << ")::printStateInfo() "
            << "from " << status_info[prev_state]
            << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;
}

void UwCsmaAloha_Triggered_NODE::waitForUser() {
    std::string response;
    std::cout << "Press Enter to continue";
    std::getline(std::cin, response);
} 

