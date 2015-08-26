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
 * @file   uwpolling_AUV.cc
 * @author Federico Favaro
 * @version 2.0.0
 *
 * \brief Provides the implementation of Uwpolling_AUV class
 *
 */

#include <sstream>

#include "mac.h"
#include "uwcbr-module.h"
#include "uwpolling_AUV.h"
#include "uwpolling_cmn_hdr.h"

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwpollingModule_AUV_Class : public TclClass {
public:

    /**
     * Constructor of the class
     */
    UwpollingModule_AUV_Class() : TclClass("Module/UW/POLLING/AUV") {
    }

    /**
     * Creates the TCL object needed for the tcl language interpretation
     * @return Pointer to an TclObject
     */
    TclObject* create(int, const char*const*) {
        return (new Uwpolling_AUV());
    }
} class_module_uwpolling_auv;

Uwpolling_AUV::~Uwpolling_AUV() {
}

bool Uwpolling_AUV::initialized = false;

std::map< Uwpolling_AUV::UWPOLLING_AUV_STATUS, std::string > Uwpolling_AUV::status_info;
std::map< Uwpolling_AUV::UWPOLLING_AUV_REASON, std::string> Uwpolling_AUV::reason_info;
std::map< Uwpolling_AUV::UWPOLLING_PKT_TYPE, std::string> Uwpolling_AUV::pkt_type_info;

Uwpolling_AUV::Uwpolling_AUV()
: data_timer(this),
probe_timer(this),
poll_timer(this),
polling_index(0),
curr_trigger_packet(0),
curr_poll_packet(0),
curr_data_packet(0),
curr_probe_packet(0),
sea_trial_(0),
print_stats_(0),
modem_data_bit_rate(900),
DATA_POLL_guard_time_(0),
n_run(0),
curr_state(UWPOLLING_AUV_STATUS_IDLE),
prev_state(UWPOLLING_AUV_STATUS_IDLE),
RxDataEnabled(false),
RxProbeEnabled(false),
TxPollEnabled(false),
N_expected_pkt(0),
packet_index(0),
curr_polled_node_address(0),
curr_backoff_time(0),
curr_RTT(0),
curr_Tmeasured(0),
probe_rtt(0),
wrong_node_data_sent(0),
initial_time(0),
n_trigger_tx(0),
n_probe_rx(0),
n_poll_tx(0),
begin(true),
stop_time(0),
pkt_time(0),
total_time(0),
TRIGGER_uid(0),
POLL_uid(0),
N_dropped_probe_pkts(0),
N_dropped_probe_wrong_state(0) {
    bind("max_payload_", (int*) & max_payload);
    bind("T_probe_", (double*) & T_probe);
    bind("T_min_", (double*) & T_min);
    bind("T_max_", (double*) & T_max);
    bind("T_guard_", (double*) & T_guard);
    bind("max_polled_node_", (int*) & max_polled_node);
    bind("sea_trial_", (int*) &sea_trial_);
    bind("print_stats_", (int*) &print_stats_);
    bind("modem_data_bit_rate_", (int*) &modem_data_bit_rate);
    bind("n_run_", (int*) &n_run);
    bind("Data_Poll_guard_time_", (int*) &DATA_POLL_guard_time_);

    if (max_polled_node <= 0) {
        max_polled_node = 1;
    }
    if (max_polled_node > MAX_POLLED_NODE) {
        max_polled_node = MAX_POLLED_NODE;
    }

}

int Uwpolling_AUV::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            if (initialized == false) initInfo();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "run") == 0) {
            stateIdle();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "stop_count_time") == 0) {
            stop_count_time();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "GetTotalReceivingTime") == 0) {
            tcl.resultf("%f", GetTotalReceivingTime());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getTriggerSent") == 0) {
            tcl.resultf("%d", getTriggerTx());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getWrongNodeDataSent") == 0) {
            tcl.resultf("%d", getWrongNodeDataSent());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getProbeReceived") == 0) {
            tcl.resultf("%d", getProbeRx());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getPollSent") == 0) {
            tcl.resultf("%d", getPollSent());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getDroppedProbePkts") == 0) {
            tcl.resultf("%d", getDroppedProbePkts());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getDroppedProbeWrongState") == 0) {
            tcl.resultf("%d", getDroppedProbeWrongState());
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "setMacAddr") == 0) {
            addr = atoi(argv[2]);
            if (debug_) std::cout << "UWPOLLING MAC address of the AUV is " << addr << std::endl;
            return TCL_OK;
        }
    }
    return MMac::command(argc, argv);
}

int Uwpolling_AUV::crLayCommand(ClMessage* m) {
    switch (m->type()) {
        default:
            return MMac::crLayCommand(m);
    }
}

void Uwpolling_AUV::DataTimer::expire(Event* e) {
    if (module->debug_) std::cout << NOW << "Uwpolling_AUV(" << module->addr << ")::DATA_TIMER::EXPIRED::Packet_received:" << module->packet_index <<
            "::TOT_PACKET_EXPECTED:" << module->N_expected_pkt << std::endl;
    timer_status = UWPOLLING_EXPIRED;
    module->DataTOExpired();

}

void Uwpolling_AUV::ProbeTimer::expire(Event* e) {
    timer_status = UWPOLLING_EXPIRED;
    if (module->debug_) std::cout << NOW << "Uwpolling_AUV(" << module->addr << ")::PROBE_TIMER::EXPIRED" << std::endl;
    module->ProbeTOExpired();

}

void Uwpolling_AUV::PollTimer::expire(Event* e) {
    timer_status = UWPOLLING_EXPIRED;
    if (module->debug_) std::cout << NOW << "Uwpolling_AUV(" << module->addr << ")::POLL_TIMER::EXPIRED" << std::endl;
    module->ChangeNodePolled();
}

void Uwpolling_AUV::DataTOExpired() {
    RxDataEnabled = false;
    refreshReason(UWPOLLING_AUV_REASON_RX_DATA_TO);
    ChangeNodePolled();
}

void Uwpolling_AUV::ProbeTOExpired() {
    RxProbeEnabled = false;
    TxPollEnabled = true;
    refreshReason(UWPOLLING_AUV_REASON_PROBE_TO_EXPIRED);
    stateTxPoll();
}

void Uwpolling_AUV::Mac2PhyStartTx(Packet* p) {
    MMac::Mac2PhyStartTx(p);
}

void Uwpolling_AUV::stateTxTrigger() {
    refreshState(UWPOLLING_AUV_STATUS_TX_TRIGGER);
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::TX_TRIGGER::T_max= " << T_max << " T_min = " << T_min << "::uid = " << TRIGGER_uid << std::endl;
    Packet* p = Packet::alloc();
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    hdr_TRIGGER* triggerh = HDR_TRIGGER(p);
    cmh->ptype() = PT_TRIGGER;
    mach->set(MF_CONTROL, addr, MAC_BROADCAST);
    mach->macSA() = addr;
    mach->macDA() = MAC_BROADCAST;
    triggerh->t_fin() = (int) (T_max * 100);
    triggerh->t_in() = (int) (T_min * 100);
    TRIGGER_uid++;
    triggerh->TRIGGER_uid_ = TRIGGER_uid;
    cmh->size() = sizeof (hdr_TRIGGER);
    if (sea_trial_ && print_stats_) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::TX_TRIGGER_ID_" << TRIGGER_uid << endl;
    curr_trigger_packet = p->copy();
    Packet::free(p);
    refreshReason(UWPOLLING_AUV_REASON_TX_TRIGGER);
    TxTrigger();
}

void Uwpolling_AUV::TxTrigger() {
    if (debug_) std::cout << NOW << "Uwpolling_AUV (" << addr << ")::TX_TRIGGER" << std::endl;
    incrCtrlPktsTx();
    incrTriggerTx();
    Mac2PhyStartTx(curr_trigger_packet);
}

void Uwpolling_AUV::Phy2MacEndTx(const Packet* p) {
    hdr_cmn* cmh = hdr_cmn::access(p);
    if (cmh->ptype() == PT_TRIGGER) {
        refreshReason(UWPOLLING_AUV_REASON_TX_TRIGGER);
        stateWaitProbe();
    } else {
        stateWaitData();
    }
}

void Uwpolling_AUV::stateWaitProbe() {
    refreshState(UWPOLLING_AUV_STATUS_WAIT_PROBE);
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::scheduling_PROBE_TIMER_T= " << T_probe << std::endl;
    RxProbeEnabled = true;
    if (T_probe < 0) {
        cerr << "Scheduling PROBE timer ----> negative value " << T_probe << endl;
        exit(1);
    }
    probe_timer.schedule(T_probe);
}

void Uwpolling_AUV::stateWaitData() {
    refreshState(UWPOLLING_AUV_STATUS_WAIT_DATA);
    RxDataEnabled = true;
    double data_timer_value = GetDataTimerValue();
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << "STATE_WAIT_DATA::Data_Timer = " << data_timer_value << std::endl;
    data_timer.schedule(data_timer_value);
    packet_index = 0;
}

double Uwpolling_AUV::GetDataTimerValue() {
    UpdateRTT();
    double timer_value = 0;
    if (!sea_trial_) {
        computeTxTime(UWPOLLING_DATA_PKT);
        timer_value = (N_expected_pkt)*(Tdata + curr_RTT + T_guard);
    } else {
        double Tdata = (max_payload * 8) / modem_data_bit_rate;
        timer_value = (N_expected_pkt)*(Tdata + curr_RTT + T_guard);
    }
    if (timer_value <= 0) {
        cout << "Uwpolling_AUV(" << addr << ")::WARNING::Data Timer Value <= 0.Setting HARDCODED = 20 sec" << std::endl;
        timer_value = 20;
    }
    return timer_value;
}

void Uwpolling_AUV::Phy2MacStartRx(const Packet* p) {
    hdr_cmn* cmh = hdr_cmn::access(p);
    if (cmh->ptype() == PT_PROBE) {
        if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACSTARTRX::PROBE_PACKET" << std::endl;
    } else {
        if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACSTARTRX::DATA_PACKET" << std::endl;
    }
}

void Uwpolling_AUV::Phy2MacEndRx(Packet* p) {
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    hdr_MPhy* ph = hdr_MPhy::access(p);
    int dest_mac = mach->macDA();
    double gen_time = ph->txtime;
    double received_time = ph->rxtime;
    double diff_time = received_time - gen_time;

    if (cmh->error()) {
        if (cmh->ptype_ == PT_POLL) {
            std::cout << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX_DROP_POLL" << endl;
        } else if (cmh->ptype_ == PT_PROBE) {
            if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX_DROP_PROBE_FROM_" << mach->macSA() << endl;
            if (sea_trial_ && print_stats_) out_file_stats << left << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX::DROP_PROBE_FROM_NODE_" << mach->macSA() << endl;
            incrDroppedProbePkts();
        } else if (cmh->ptype_ == PT_TRIGGER) {
            std::cerr << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX_DROP_TRIGGER" << endl;
        } else {
            if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX::DROP_DATA_FROM_NODE_" << mach->macSA() << endl;
            if (sea_trial_ && print_stats_) out_file_stats << left << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX::DROP_DATA_FROM_NODE_" << mach->macSA() << endl;
            incrErrorPktsRx();
        }
        refreshReason(UWPOLLING_AUV_REASON_PACKET_ERROR);
        drop(p, 1, UWPOLLING_AUV_DROP_REASON_ERROR);
    } else {
        if ((dest_mac == addr) || (dest_mac == (int) MAC_BROADCAST)) {
            if (cmh->ptype_ == PT_PROBE) {
                incrCtrlPktsRx();
                refreshReason(UWPOLLING_AUV_REASON_PROBE_RECEIVED);
                curr_probe_packet = p->copy();
                Packet::free(p);
                hdr_PROBE* prh = hdr_PROBE::access(curr_probe_packet);
                if (debug_)std::cout << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX::PROBE pkt::N_pkt=" << prh->n_pkts_ << "::Backoff_time=" << prh->backoff_time() << "::Id_nodo=" << prh->id_node() << std::endl;
                probe_rtt = diff_time;
                if (begin) {
                    begin = false;
                    initial_time = NOW;
                }
                stateRxProbe();
            } else {
                if ((cmh->ptype_ != PT_POLL) && (cmh->ptype_ != PT_PROBE) && (cmh->ptype_ != PT_TRIGGER)) {
                    refreshReason(UWPOLLING_AUV_REASON_DATA_RX);
                    curr_data_packet = p->copy();
                    Packet::free(p);
                    pkt_time = NOW;
                    stateRxData();
                }
            }
        } else {
            if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::PHY2MACENDRX::DROP_DATA_WRONG_DEST_" << mach->macDA() << endl;
            incrXCtrlPktsRx();
            if (sea_trial_ && print_stats_) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::PHY2MACENDRX::DROP_DATA_WRONG_DEST_" << mach->macDA() << endl;
            refreshReason(UWPOLLING_AUV_REASON_PACKET_ERROR);
            drop(p, 1, UWPOLLING_AUV_DROP_REASON_WRONG_RECEIVER);
        }
    }
}

void Uwpolling_AUV::computeTxTime(UWPOLLING_PKT_TYPE pkt) {
    Packet* fuzzy_pkt;
    fuzzy_pkt = Packet::alloc();
    switch (pkt) {
        case(UWPOLLING_POLL_PKT):
        {
            hdr_cmn* cmh = HDR_CMN(fuzzy_pkt);
            hdr_mac* mach = HDR_MAC(fuzzy_pkt);
            mach->macSA() = addr;
            cmh->size() = sizeof (id_poll) * polling_index;
            Tpoll = Mac2PhyTxDuration(fuzzy_pkt);
        }
            break;
        case(UWPOLLING_PROBE_PKT):
        {
            hdr_cmn* cmh = HDR_CMN(fuzzy_pkt);
            hdr_mac* mach = HDR_MAC(fuzzy_pkt);
            mach->macSA() = addr;
            cmh->size() = sizeof (hdr_PROBE);
            Tprobe = Mac2PhyTxDuration(fuzzy_pkt);
        }
            break;
        case(UWPOLLING_TRIGGER_PKT):
        {
            hdr_cmn* cmh = HDR_CMN(fuzzy_pkt);
            cmh->size() = sizeof (hdr_TRIGGER);
            hdr_mac* mach = HDR_MAC(fuzzy_pkt);
            mach->macSA() = addr;
            //cmh->size() = 10;
            Ttrigger = Mac2PhyTxDuration(fuzzy_pkt);
        }
            break;
        case(UWPOLLING_DATA_PKT):
        {
            hdr_cmn* cmh = HDR_CMN(fuzzy_pkt);
            hdr_mac* mach = HDR_MAC(fuzzy_pkt);
            mach->macDA() = addr;
            cmh->size() = max_payload;
            Tdata = Mac2PhyTxDuration(fuzzy_pkt);
        }
    }
    Packet::free(fuzzy_pkt);
}

void Uwpolling_AUV::UpdateRTT() {
    curr_RTT = curr_Tmeasured;
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::UPDATE_RTT--->RTT = " << curr_RTT << std::endl;

}

void Uwpolling_AUV::stateRxData() {
    refreshState(UWPOLLING_AUV_STATUS_RX_DATA);
    if (RxDataEnabled) {
        hdr_mac* mach = HDR_MAC(curr_data_packet);
        hdr_uwcbr* cbrh = HDR_UWCBR(curr_data_packet);
        if (sea_trial_ && print_stats_) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::STATE_RX_DATA::RX_DATA_ID_" << cbrh->sn_ << "_FROM_NODE_" << mach->macSA() << endl;
        if (mach->macSA() == curr_polled_node_address) {
            if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::STATE_RX_DATA::RX_DATA_ID_" << cbrh->sn_ << "_FROM_NODE_" << mach->macSA() << endl;
            incrDataPktsRx();
            packet_index++;
            sendUp(curr_data_packet);
            if (packet_index == N_expected_pkt) {
                refreshReason(UWPOLLING_AUV_REASON_LAST_PACKET_RECEIVED);
                data_timer.force_cancel();
                if (sea_trial_) {
                    poll_timer.schedule(DATA_POLL_guard_time_);
                } else {
                    ChangeNodePolled();
                }
            }
        } else {
            incrWrongNodeDataSent();
            if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::STATE_RX_DATA::DROP_DATA_WRONG_NODE_" << mach->macSA_ << std::endl;
            if (sea_trial_ && print_stats_) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::STATE_RX_DATA::DROP_DATA_FROM_NODE_" << mach->macDA() << "_NOT_POLLED" << endl;
            drop(curr_data_packet, 1, UWPOLLING_AUV_DROP_REASON_WRONG_STATE);
        }
    } else {
        incrXDataPktsRx();
        hdr_mac* mac = HDR_MAC(curr_data_packet);
        if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::STATE_RX_DATA::DROP_DATA_WRONG_STATE=" << status_info[curr_state] << std::endl;
        if (sea_trial_ && print_stats_) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::STATE_RX_DATA::DROP_DATA_FROM_NODE_" << mac->macDA() << "_NOT_POLLED" << endl;
        drop(curr_data_packet, 1, UWPOLLING_AUV_DROP_REASON_WRONG_STATE);
    }
}

void Uwpolling_AUV::ChangeNodePolled() {
    RxDataEnabled = false;
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::CHANGE_NODE_POLLED::" << list_probbed_node[0].id_node << "::" <<
            list_probbed_node[1].id_node << "::" << list_probbed_node[2].id_node << "::" << list_probbed_node[3].id_node << std::endl;
    if (polling_index > 1) {
        for (int i = 0; i < polling_index - 1; i++) {
            list_probbed_node[i] = list_probbed_node[i + 1];
        }
        list_probbed_node[polling_index - 1].id_node = (int) NAN;
        for (int it = polling_index; it < max_polled_node; it++) {
            list_probbed_node[it].id_node = (int) NAN;
        }
        polling_index--;
        TxPollEnabled = true;
        stateTxPoll();
    } else {
        refreshReason(UWPOLLING_AUV_REASON_LAST_POLLED_NODE);
        stateIdle();
    }
}

void Uwpolling_AUV::stateRxProbe() {
    if (RxProbeEnabled) {
        hdr_PROBE* probeh = HDR_PROBE(curr_probe_packet);
        if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::STATE_RX_PROBE" << "::N_packet_expected=" << probeh->n_pkts_
                << "::id_node=" << probeh->id_node_ << "::BackOff Time=" << probeh->backoff_time_ << std::endl;
        refreshState(UWPOLLING_AUV_STATUS_RX_PROBES);
        incrCtrlPktsRx();
        incrProbeRx();
        hdr_mac* mach = HDR_MAC(curr_probe_packet);
        if (sea_trial_ && print_stats_) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::RX_PROBE_ID_" << probeh->PROBE_uid_ << "_FROM_NODE_" << mach->macSA() << endl;
        list_probbed_node[polling_index].backoff_time = ((double) probeh->backoff_time_) / 100;
        list_probbed_node[polling_index].id_node = probeh->id_node();
        list_probbed_node[polling_index].n_pkts = probeh->n_pkts();
        list_probbed_node[polling_index].time_stamp = ((double) probeh->ts()) / 100;
        list_probbed_node[polling_index].mac_address = mach->macSA();
        /*
         * New mode for calculate RTT: based on TX_time and RX_time of PROBE calculation
         * ---> Now backoff time is not useful anymore
         * ---> More guard_time_ is needed
         */
        list_probbed_node[polling_index].Tmeasured = probe_rtt;
        polling_index++;
        Packet::free(curr_probe_packet);
        if (polling_index == max_polled_node) {
            refreshReason(UWPOLLING_AUV_REASON_MAX_PROBE_RECEIVED);
            RxProbeEnabled = false;
            TxPollEnabled = true;
            probe_timer.force_cancel();
            stateTxPoll();
        }
    } else {
        incrDroppedProbeWrongState();
        if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << "):: dropping Probe Wrong State " << status_info[curr_state] << std::endl;
        drop(curr_probe_packet, 1, UWPOLLING_AUV_DROP_REASON_WRONG_STATE);
    }

}

void Uwpolling_AUV::stateTxPoll() {
    if (TxPollEnabled) {
        if (polling_index > 0) {
            refreshState(UWPOLLING_AUV_STATUS_TX_POLL);
            Packet* p = Packet::alloc();
            hdr_cmn* cmh = hdr_cmn::access(p);
            hdr_mac* mach = HDR_MAC(p);
            cmh->ptype() = PT_POLL;
            cmh->size() = 2;
            mach->set(MF_CONTROL, addr, MAC_BROADCAST);
            mach->macSA() = addr;
            mach->macDA() = MAC_BROADCAST;
            curr_poll_packet = p->copy();
            Packet::free(p);
            SortNode2Poll();
            hdr_POLL* pollh = HDR_POLL(curr_poll_packet);
            POLL_uid++;
            pollh->POLL_uid_ = POLL_uid;
            if (sea_trial_ && print_stats_) out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::TX_POLL_ID_" << POLL_uid << "_NODE_POLLED_" << curr_node_id << endl;
            if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::STATE_TX_POLL::Node polled = " << pollh->id_ << "::NODE_TO_POLL= " << polling_index << std::endl;
            TxPoll();
        } else {
            if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::STATE_TX_POLL--->IDLE--->No node to POLL" << std::endl;
            stateIdle();
        }
    } else {
        if (debug_) std::cerr << NOW << "Uwpolling_AUV(" << addr << ")---> going in stateTxPoll from WRONG STATE---> current_state: " << status_info[curr_state] << std::endl;

    }
}

void Uwpolling_AUV::TxPoll() {
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::TX_POLL" << std::endl;
    incrCtrlPktsTx();
    incrPollTx();
    Mac2PhyStartTx(curr_poll_packet);
}

void Uwpolling_AUV::SortNode2Poll() {
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::SORT_NODE_TO_POLL " << std::endl;
    hdr_POLL* pollh = HDR_POLL(curr_poll_packet);
    /*sorting*/
    bool scambio = true;
    probbed_node fuzzy;
    while (scambio) {
        scambio = false;
        for (int i = 0; i < polling_index - 1; i++) {
            if (list_probbed_node[i].time_stamp < list_probbed_node[i + 1].time_stamp) {
                scambio = true;
                fuzzy = list_probbed_node[i];
                list_probbed_node[i] = list_probbed_node[i + 1];
                list_probbed_node[i + 1] = fuzzy;
            }
        }
    }
    curr_polled_node_address = list_probbed_node[0].mac_address;
    N_expected_pkt = list_probbed_node[0].n_pkts;
    curr_backoff_time = list_probbed_node[0].backoff_time;
    curr_Tmeasured = list_probbed_node[0].Tmeasured;
    pollh->id_ = list_probbed_node[0].id_node;
    curr_node_id = pollh->id_;
}

void Uwpolling_AUV::stateIdle() {
    refreshState(UWPOLLING_AUV_STATUS_IDLE);
    if (debug_) std::cout << NOW << "Uwpolling_AUV(" << addr << ")::IDLE STATE " << std::endl;
    data_timer.force_cancel();
    probe_timer.force_cancel();
    polling_index = 0;
    stateTxTrigger();
}

void Uwpolling_AUV::initInfo() {
    initialized = true;

    if (sea_trial_ && print_stats_) {
        std::stringstream stat_file;
        stat_file << "./Uwpolling_AUV_" << addr << "_" << n_run << ".out";
        out_file_stats.open(stat_file.str().c_str(), std::ios_base::app);
        out_file_stats << left << "[" << getEpoch() << "]::" << NOW << "::Uwpolling_AUV(" << addr << ")::NS_START" << endl;
    }

    status_info[UWPOLLING_AUV_STATUS_IDLE] = "Idle state";
    status_info[UWPOLLING_AUV_STATUS_RX_DATA] = "Receiving Data Packet";
    status_info[UWPOLLING_AUV_STATUS_RX_PROBES] = "Receiving Probe Packet";
    status_info[UWPOLLING_AUV_STATUS_TX_POLL] = "Transmitting a POLL Packet";
    status_info[UWPOLLING_AUV_STATUS_TX_TRIGGER] = "Transmitting a TRIGGER Packet";
    status_info[UWPOLLING_AUV_STATUS_WAIT_PROBE] = "Waiting for a PROBE Packet";
    status_info[UWPOLLING_AUV_STATUS_WAIT_DATA] = "Waiting for a DATA Packet";

    pkt_type_info[UWPOLLING_DATA_PKT] = "Data Packet";
    pkt_type_info[UWPOLLING_POLL_PKT] = "Poll packet";
    pkt_type_info[UWPOLLING_TRIGGER_PKT] = "Trigger packet";
    pkt_type_info[UWPOLLING_PROBE_PKT] = "Probe packet";

    reason_info[UWPOLLING_AUV_REASON_DATA_RX] = "Received a Data Packet";
    reason_info[UWPOLLING_AUV_REASON_PROBE_RECEIVED] = "Received a PROBE packet";
    reason_info[UWPOLLING_AUV_REASON_TX_POLL] = "POLL Packet transmitted";
    reason_info[UWPOLLING_AUV_REASON_TX_TRIGGER] = "Trigger Packet transmitted";
    reason_info[UWPOLLING_AUV_REASON_RX_DATA_TO] = "Receiving Data Time-Out";
    reason_info[UWPOLLING_AUV_REASON_LAST_PACKET_RECEIVED] = "Last Packet Received from the Node";
    reason_info[UWPOLLING_AUV_REASON_LAST_POLLED_NODE] = "Last Node from the list Polled";
    reason_info[UWPOLLING_AUV_REASON_MAX_PROBE_RECEIVED] = "Maximum Number of Probe Received";
    reason_info[UWPOLLING_AUV_REASON_PACKET_ERROR] = "Packet Error";
    reason_info[UWPOLLING_AUV_REASON_PROBE_TO_EXPIRED] = "Receiving PROBE Time-Out";
}

void Uwpolling_AUV::waitForUser() {
    std::string response;
    std::cout << "Press Enter to continue";
    std::getline(std::cin, response);
}

void Uwpolling_AUV::stop_count_time() {
    stop_time = pkt_time;
    total_time = stop_time - initial_time;
}

