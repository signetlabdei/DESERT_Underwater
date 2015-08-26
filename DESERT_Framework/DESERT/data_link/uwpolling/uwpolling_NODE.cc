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
 * @file   uwpolling_NODE.cc
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Implementation of Uwpolling_NODE class 
 *
 */

#include "uwpolling_NODE.h"
#include "uwpolling_cmn_hdr.h"
#include "mac.h"
#include "mmac.h"

#include "uwcbr-module.h"

#include <sstream>
#include <sys/time.h>

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwpollingModule_NODE_Class : public TclClass {
public:

    /**
     * Constructor of the class
     */
    UwpollingModule_NODE_Class() : TclClass("Module/UW/POLLING/NODE") {
    }

    /**
     * Creates the TCL object needed for the tcl language interpretation
     * @return Pointer to an TclObject
     */
    TclObject* create(int, const char*const*) {
        return (new Uwpolling_NODE());
    }
} class_module_uwpolling_node;

void Uwpolling_NODE::BackOffTimer::expire(Event* e) {
    if (module->debug_) std::cout<< NOW << "Uwpolling_NODE("<< module->addr << ")::BACKOFF_TIMER::EXPIRED" << std::endl; 
    timer_status = UWPOLLING_EXPIRED;
    if (module->Triggered) {
        module->BackOffTimerExpired();
    }
}

void Uwpolling_NODE::BackOffTimerExpired() {
    refreshReason(UWPOLLING_NODE_REASON_BACKOFF_TIMER_EXPIRED);
    stateTxProbe();
}

void Uwpolling_NODE::Rx_Poll_Timer::expire(Event* e) {
    if (module->debug_) std::cout<< NOW << "Uwpolling_NODE("<< module->addr << ")::RX_POLL_TIMER::EXPIRED" << std::endl;
    timer_status = UWPOLLING_EXPIRED;
    module->RxPollTimerExpired();
}

void Uwpolling_NODE::Tx_Data_Timer::expire(Event* e) {
    if (module->debug_) std::cout<< NOW << "Uwpolling_NODE("<< module->addr << ")::DATA_TIMER::EXPIRED" << std::endl;
    timer_status = UWPOLLING_EXPIRED;
    module->stateTxData();
}

void Uwpolling_NODE::RxPollTimerExpired() {
    if (RxPollEnabled) {
        RxPollEnabled = false;
        refreshReason(UWPOLLING_NODE_REASON_RX_POLL_TIMER_EXPIRED);
        stateIdle();
    }
}

bool Uwpolling_NODE::initialized = false;
map< Uwpolling_NODE::UWPOLLING_NODE_STATUS , string > Uwpolling_NODE::status_info;                                                                    
map< Uwpolling_NODE::UWPOLLING_NODE_REASON, string> Uwpolling_NODE::reason_info;                                                                      
map< Uwpolling_NODE::UWPOLLING_PKT_TYPE, string> Uwpolling_NODE::pkt_type_info;

Uwpolling_NODE::Uwpolling_NODE()
:
T_poll(0),
backoff_tuner(0),
max_payload(0),
buffer_data_pkts(0),
node_id(0),
sea_trial(0),
print_stats(0),
n_run(0),
Intra_data_Guard_Time(0),
polled(false),
RxPollEnabled(false),
Triggered(false),
LastPacket(false),
MaxTimeStamp(0),
T_in(0),
T_fin(0),
BOffTime(0),
AUV_mac_addr(0),
N_data_pkt_2_TX(0),
packet_index(0),
n_probe_sent(0),
n_times_polled(0),
n_trigger_received(0),
N_polled_node(0),
PROBE_uid(0),
curr_data_pkt(0),
curr_probe_pkt(0),
curr_poll_pkt(0),
curr_trigger_pkt(0),
backoff_timer(this),
rx_poll_timer(this),
tx_data_timer(this),
fout(0),
out_file_stats(0)
{

    bind("T_poll_", (double*) & T_poll);
    bind("backoff_tuner_", (double*) & backoff_tuner);
    bind("max_payload_", (int*) & max_payload);
    bind("buffer_data_pkts_", (int*) & buffer_data_pkts);
    bind("Max_DATA_Pkts_TX_", (int*) & max_data_pkt_tx);
    bind("node_id_", (uint*) & node_id);
    bind("print_stats_", (int*) &print_stats);
    bind("sea_trial_", (int*) &sea_trial);
    bind("intra_data_guard_time_", (int*) &Intra_data_Guard_Time);
    bind("n_run_",(int*) &n_run);


    if (T_poll <= 0) T_poll = MIN_T_POLL;
    if (buffer_data_pkts > MAX_BUFFER_SIZE) buffer_data_pkts = MAX_BUFFER_SIZE;
    if (buffer_data_pkts <= 0) buffer_data_pkts = 1;
    if (max_data_pkt_tx > buffer_data_pkts) {
        max_data_pkt_tx = buffer_data_pkts;
    }
    if (max_data_pkt_tx == 0) {
        max_data_pkt_tx = 1;
    }
}

Uwpolling_NODE::~Uwpolling_NODE() {

}

int Uwpolling_NODE::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            if (!initialized) initInfo();
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "getDataQueueSize") == 0) {
            tcl.resultf("%d", Q_data.size());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getProbeSent") == 0) {
            tcl.resultf("%d", getProbeSent());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getTimesPolled") == 0) {
            tcl.resultf("%d", getTimesPolled());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getTriggerReceived") == 0) {
            tcl.resultf("%d", getTriggerReceived());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getTriggerDropped") == 0) {
            tcl.resultf("%d", getTriggerDropped());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getPollDropped") == 0) {
            tcl.resultf("%d", getPollDropped());
            return TCL_OK;
        }

    }
    else if (argc == 3) {
        if (strcasecmp(argv[1], "setMacAddr") == 0) {
            addr = atoi(argv[2]);
            return TCL_OK;
        }
    }
    return MMac::command(argc, argv);
}

int Uwpolling_NODE::crLayCommand(ClMessage* m) {
    switch (m->type()) {
        default:
            return MMac::crLayCommand(m);
    }
}


void Uwpolling_NODE::initInfo() {
    initialized = true;
    
    if (sea_trial && print_stats)
    {
        std::stringstream stat_file;
        stat_file << "./Uwpolling_NODE_" << addr << "_" << n_run << ".out";
        std::cout << stat_file.str().c_str() << endl;
        out_file_stats.open(stat_file.str().c_str(),std::ios_base::app);
        out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<< addr << ")::NS_START" << endl;
    }
    
    status_info[UWPOLLING_NODE_STATUS_IDLE] = "Idle State";
    status_info[UWPOLLING_NODE_STATUS_RX_POLL] = "Receiving POLL from AUV";
    status_info[UWPOLLING_NODE_STATUS_RX_TRIGGER] = "Receiving Trigger from AUV";
    status_info[UWPOLLING_NODE_STATUS_TX_DATA] = "Transmitting Data to AUV";
    status_info[UWPOLLING_NODE_STATUS_TX_PROBE] = "Transmitting Probe to AUV";
    status_info[UWPOLLING_NODE_STATUS_WAIT_POLL] = "Waiting for the reception of POLL packet";

    pkt_type_info[UWPOLLING_DATA_PKT] = "Data Packet from Application Layer";
    pkt_type_info[UWPOLLING_POLL_PKT] = "Poll packet";
    pkt_type_info[UWPOLLING_TRIGGER_PKT] = "Trigger packet";
    pkt_type_info[UWPOLLING_PROBE_PKT] = "Probe packet";

    reason_info[UWPOLLING_NODE_REASON_NOT_SET] = "Reason not set";
    reason_info[UWPOLLING_NODE_REASON_BACKOFF_TIMER_EXPIRED] = "BackOff expired";
    reason_info[UWPOLLING_NODE_REASON_RX_POLL_TIMER_EXPIRED] = "Receving POLL Time-Out";
    reason_info[UWPOLLING_NODE_REASON_TX_DATA] = "Data transmission";
    reason_info[UWPOLLING_NODE_REASON_RX_POLL] = "Receiving a poll from the AUV";
    reason_info[UWPOLLING_NODE_REASON_RX_TRIGGER] = "Receiving a trigger from the AUV";
    reason_info[UWPOLLING_NODE_REASON_TX_PROBE] = "Transmitting probe to the AUV";
    reason_info[UWPOLLING_NODE_REASON_EMPTY_DATA_QUEUE] = "Data Queue empty";
    reason_info[UWPOLLING_NODE_REASON_PKT_ERROR] = "Received a Corrupted Packet";
    reason_info[UWPOLLING_NODE_REASON_WRONG_RECEIVER] = "Packet not for this receiver";
    reason_info[UWPOLLING_NODE_REASON_WRONG_STATE] = "Receiving a Packet in Wrong State (--> NOT enabled to receive packets)";
    reason_info[UWPOLLING_NODE_REASON_WRONG_TYPE] = "Receiving a Packet of Wrong Type";
    reason_info[UWPOLLING_NODE_REASON_LIST_NOT_POLLED] = "This receiver is in the polling list but is not polled";
    reason_info[UWPOLLING_NODE_REASON_NOT_IN_LIST] = "This receiver is not in the polling list";
}

void Uwpolling_NODE::stateTxProbe() {
    if (Triggered) {
        refreshState(UWPOLLING_NODE_STATUS_TX_PROBE);
        initPkt(UWPOLLING_PROBE_PKT);
        hdr_PROBE* probehdr = HDR_PROBE(curr_probe_pkt);
        if (probehdr->n_pkts() > 0) {
           PROBE_uid++;
           probehdr->PROBE_uid_ = PROBE_uid;
           if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<< addr << ")::TX_PROBE_ID_"<<PROBE_uid<<"_N_PKTS="<<(uint)probehdr->n_pkts_<<endl;
           if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::STATE_TX_PROBE_ID_"<<PROBE_uid<<"_N_PKTS="<<(uint)probehdr->n_pkts_<<endl;
            TxPRobe();
        }
        else {
            if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::STATE_TX_PROBE::NOT_TX_PROBE--->Empty Data Queue" << std::endl;
            refreshReason(UWPOLLING_NODE_REASON_EMPTY_DATA_QUEUE);
            Packet::free(curr_probe_pkt);
            stateIdle();
        }
    }
}

void Uwpolling_NODE::TxPRobe() {
    incrCtrlPktsTx();
    incrProbeSent();
    Mac2PhyStartTx(curr_probe_pkt);
}

void Uwpolling_NODE::stateWaitPoll() {
    refreshState(UWPOLLING_NODE_STATUS_WAIT_POLL);
    if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::STATE_WAIT_POLL---> Timer_rx_poll =  " << rx_poll_timer.getDuration() << std::endl;
    RxPollEnabled = true;
    if (T_poll < 0) {
        cerr << "Scheduling POLL timer ----> negative value " << T_poll << endl;
        T_poll = 60;
    }
    rx_poll_timer.schedule(T_poll);
}

void Uwpolling_NODE::stateRxPoll() {
    if (RxPollEnabled) {
        refreshState(UWPOLLING_NODE_STATUS_RX_POLL);
        hdr_POLL* pollh = HDR_POLL(curr_poll_pkt);
        if ( debug_ ) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::STATE_RX_POLL::Node_POLLED = " << pollh->id_ << std::endl;
        int polled_node = pollh->id_;
        if ( node_id == (uint)polled_node)
        {
            polled = true;
            if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<<addr << ")::POLLED_POLL_ID_" << pollh->POLL_uid_ << endl;
            refreshReason(UWPOLLING_NODE_REASON_RX_POLL);
            Packet::free(curr_poll_pkt);
            incrTimesPolled();
            stateTxData();
            
        }
            else {
                refreshReason(UWPOLLING_NODE_REASON_NOT_IN_LIST);
                drop(curr_poll_pkt, 1, UWPOLLING_NODE_DROP_REASON_NOT_POLLED);
            }
    }
    else 
    {
        if ( debug_ ) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::STATE_RX_POLL::NODE_NOT_POLLED" << endl;
        refreshReason(UWPOLLING_NODE_REASON_WRONG_STATE);
        drop(curr_poll_pkt, 1, UWPOLLING_NODE_DROP_REASON_WRONG_STATE);
    }
}

void Uwpolling_NODE::recvFromUpperLayers(Packet* p) {
    if ((int) Q_data.size() < buffer_data_pkts) {
        hdr_uwcbr* cbrh = HDR_UWCBR(p);
        if (debug_) std::cout << NOW << "UWPOLLING_NODE(" << addr << ")::RECV_FROM_U_LAYERS_ID_" << cbrh->sn_ << endl;
        if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<<addr << ")::RECV_FROM_U_LAYERS_ID_" << cbrh->sn_ << endl;
        Q_data.push(p);
    }
    else {
        if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW <<  "Uwpolling_NODE("<<addr << ")::DROP_FULL_QUEUE" << endl;
        if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::DROP_FULL_QUEUE" << std::endl;
        incrDiscardedPktsTx();
        drop(p, 1, UWPOLLING_NODE_DROP_REASON_BUFFER_FULL);
    }
}

void Uwpolling_NODE::stateTxData() {
    if (polled) {
        if (curr_state == UWPOLLING_NODE_STATUS_RX_POLL) {
            packet_index = 1;
            LastPacket = false;
        }
        else {
            packet_index++;
        }
        refreshState(UWPOLLING_NODE_STATUS_TX_DATA);
        if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::STATE_TX_DATA_ " << packet_index << "::OF::" << N_data_pkt_2_TX << std::endl;
        curr_data_pkt = (Q_data.front())->copy();
        Q_data.pop();
        hdr_uwcbr* cbrh = HDR_UWCBR(curr_data_pkt);
        if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<<addr << ")::TX_DATA_" << packet_index << "_OF_" << N_data_pkt_2_TX << "_PACKET_INDEX_"<< cbrh->sn_<< endl;
        if (packet_index == N_data_pkt_2_TX) LastPacket = true;
        hdr_mac* mach = HDR_MAC(curr_data_pkt);
        mach->macSA() = addr;
        mach->macDA() = AUV_mac_addr;
        TxData();
    }
}

void Uwpolling_NODE::Mac2PhyStartTx(Packet* p) {
    MMac::Mac2PhyStartTx(p);
}

void Uwpolling_NODE::TxData() {
    incrDataPktsTx();
    Mac2PhyStartTx(curr_data_pkt);

}

void Uwpolling_NODE::Phy2MacEndTx(const Packet* p) {
    switch (curr_state) {
        case(UWPOLLING_NODE_STATUS_TX_PROBE):
        {
            refreshReason(UWPOLLING_NODE_REASON_TX_PROBE);
            stateWaitPoll();
            break;
        }
        case(UWPOLLING_NODE_STATUS_TX_DATA):
        {
            refreshReason(UWPOLLING_NODE_REASON_TX_DATA);
            if (LastPacket) {
                stateIdle();
            } else {
                Packet* dummy = Packet::alloc();
                hdr_cmn* ch = HDR_CMN(dummy);
                ch->size() = max_payload;
                hdr_mac* mach = HDR_MAC(dummy);
                mach->macDA() = AUV_mac_addr;
                //double dataPacket_duration = Mac2PhyTxDuration(dummy);
                //double dataPacket_duration = (max_payload*8)/modem_data_bit_rate;
                double dataPacket_duration = Intra_data_Guard_Time;
                //cout << "Data Packet duration" << dataPacket_duration << endl;
                Packet::free(dummy);
                /*
                 * Instead of transmit the new packet just after the transmission of the previous one
                 * i wait for a little time (the duration of the reception of the old one). 
                 * This way, the AUV doesn't drop the new one because it is synchronized on the old one
                 */
                if (dataPacket_duration < 0) {
                    cerr << "Scheduling Tx_Data timer ----> negative value " << dataPacket_duration << endl;
                    dataPacket_duration = 8;
                }
                tx_data_timer.schedule(dataPacket_duration);
            }
            break;
        }
        default:
        {
            if(debug_) std::cerr << NOW << "UWPOLLING_NODE(" << addr << ")::PHY2MACENDTX::logical_error,state = " << status_info[curr_state] << ", prev_state = " << status_info[prev_state] << std::endl;
        }
    }
}

void Uwpolling_NODE::initPkt(UWPOLLING_PKT_TYPE pkt_type) {
    if (pkt_type == UWPOLLING_PROBE_PKT) {
        Packet* p = Packet::alloc();
        hdr_PROBE* probehdr = HDR_PROBE(p);
        hdr_cmn* ch = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        if (Q_data.size() > 0) {
            hdr_cmn* chd = hdr_cmn::access(Q_data.back());
            MaxTimeStamp = chd->timestamp();
            if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::INIT_PROBE::"
                    "Backoff_time=" << BOffTime << "::MaxTimeStamp=" << MaxTimeStamp << "::Number of pkts to TX=" << Q_data.size() << std::endl;
            ch->ptype() = PT_PROBE;
            //ch->size() += sizeof (hdr_PROBE);
            ch->size() = sizeof (hdr_PROBE);
            //probehdr->backoff_time() = BOffTime;
            //probehdr->ts() = MaxTimeStamp;
            probehdr->backoff_time() = (uint16_t)(BOffTime*100);
            probehdr->ts() = (uint16_t) (MaxTimeStamp*100);
            probehdr->id_node() = node_id;
            if ((int) Q_data.size() <= max_data_pkt_tx) {
                probehdr->n_pkts() = (int) Q_data.size();
                N_data_pkt_2_TX = (int) Q_data.size();
            } else {
                probehdr->n_pkts() = (int) max_data_pkt_tx;
                N_data_pkt_2_TX = (int) max_data_pkt_tx;
            }
            mach->set(MF_CONTROL, addr, AUV_mac_addr);
            mach->macDA() = AUV_mac_addr;
            mach->macSA() = addr;
        }
        else {
            probehdr->n_pkts() = 0;
        }
        curr_probe_pkt = p->copy();
        Packet::free(p);
    }
}

void Uwpolling_NODE::stateIdle() {
    if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::IDLE_STATE" << std::endl;
    refreshState(UWPOLLING_NODE_STATUS_IDLE);
    polled = false;
    Triggered = false;
    RxPollEnabled = false;
    backoff_timer.force_cancel();
    rx_poll_timer.force_cancel();
}

void Uwpolling_NODE::stateRxTrigger() {
    refreshState(UWPOLLING_NODE_STATUS_RX_TRIGGER);
    Triggered = true;
    incrTriggerReceived();
    hdr_TRIGGER* triggerhdr = HDR_TRIGGER(curr_trigger_pkt);
    hdr_mac* mach = HDR_MAC(curr_trigger_pkt);
    if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<<addr << ")::RX_TRIGGER_ID_"<< triggerhdr->TRIGGER_uid_ << endl;
    if (debug_) {
        std::cout << NOW << "Uwpolling_NODE(" << addr << ")::STATE_RX_TRIGGER --> HEADER :: T_fin = " << triggerhdr->t_fin() << " T_in = " << triggerhdr->t_in() << std::endl;
    }
    AUV_mac_addr = mach->macSA();
    T_fin = (double) triggerhdr->t_fin()/100;
    T_in = (double) triggerhdr->t_in()/100;
    BOffTime = getBackOffTime();
    if (BOffTime < 0) {
        cerr << "Scheduling Backoff timer ---> negative value " << BOffTime << endl;
        BOffTime = 5;
    }
    backoff_timer.schedule(BOffTime);
}

double Uwpolling_NODE::getBackOffTime() {
    srand(time(NULL) + addr);
    int random = rand() % (int)T_fin + (int)T_in;
    if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::BACKOFF_TIMER_VALUE = " << backoff_tuner * random << std::endl;
    return (backoff_tuner * random);
}

void Uwpolling_NODE::Phy2MacEndRx(Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    packet_t rx_pkt_type = ch->ptype();
    hdr_mac* mach = HDR_MAC(p);
    int dest_mac = mach->macDA();
    if (ch->error()) {

        if (ch->ptype() == PT_TRIGGER) {
            if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::PHY2MACENDRX::DROP_TRIGGER" << std::endl;
            if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<<addr << ")::PKT_TRIGGER_DROP_ERROR" << endl;
            incrTriggerDropped();
        } else if (ch->ptype() == PT_POLL) {
            if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::PHY2MACENDRX::DROP_POLL" << std::endl;
            if (sea_trial && print_stats) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_NODE("<<addr << ")::PKT_POLL_DROP_ERROR" << endl;
            incrPollDropped();
        }
        refreshReason(UWPOLLING_NODE_REASON_PKT_ERROR);
        drop(p, 1, UWPOLLING_NODE_DROP_REASON_ERROR);
    } else if ((dest_mac == addr) || (dest_mac == (int) MAC_BROADCAST)) {
        if (rx_pkt_type == PT_TRIGGER) {
            refreshReason(UWPOLLING_NODE_REASON_RX_TRIGGER);
            curr_trigger_pkt = p->copy();
            Packet::free(p);
            stateRxTrigger();
        } else if (rx_pkt_type == PT_POLL) {
            refreshReason(UWPOLLING_NODE_REASON_RX_POLL);
            hdr_POLL* pollh = HDR_POLL(p);
           if (debug_) std::cout << NOW << "Uwpolling_NODE (" << addr << ")::PHY2MACENDRX::RX_POLL::POLLED_NODE= " << pollh->id_ << std::endl;
            rx_poll_timer.force_cancel();
            curr_poll_pkt = p->copy();
            Packet::free(p);
            stateRxPoll();
        } else {
            drop(p, 1, UWPOLLING_NODE_DROP_REASON_UNKNOWN_TYPE);
        }
    } else {
        incrXCtrlPktsRx();
        if (rx_pkt_type == PT_TRIGGER)
        {
            if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::PHY2MACENDRX::WARNING!!TRIGGER packet not in broadcast!!" << std::endl;
            
        } 
        else if (rx_pkt_type == PT_POLL)
        {
            if (debug_) std::cout << NOW << "Uwpolling_NODE(" << addr << ")::PHY2MACENDRX::WARNING!!POLL packet not in broadcast!!" << std::endl;
        }
        drop(p, 1, UWPOLLING_NODE_DROP_REASON_WRONG_RECEIVER);
    }
}

void Uwpolling_NODE::waitForUser() {
    std::string response;
    std::cout << "Press Enter to continue";
    std::getline(std::cin, response);
}
