//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwpolling_AUV.cpp
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

#include <algorithm>
#include <float.h>
#include <fstream>
#include <sstream>
#include "uwphy-clmsg.h"

/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwpollingModule_AUV_Class : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwpollingModule_AUV_Class()
		: TclClass("Module/UW/POLLING/AUV")
	{
	}

	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new Uwpolling_AUV());
	}
} class_module_uwpolling_auv;

Uwpolling_AUV::~Uwpolling_AUV()
{
}

bool Uwpolling_AUV::initialized = false;

std::map<Uwpolling_AUV::UWPOLLING_AUV_STATUS, std::string>
		Uwpolling_AUV::status_info;
std::map<Uwpolling_AUV::UWPOLLING_AUV_REASON, std::string>
		Uwpolling_AUV::reason_info;
std::map<Uwpolling_AUV::UWPOLLING_PKT_TYPE, std::string>
		Uwpolling_AUV::pkt_type_info;

Uwpolling_AUV::Uwpolling_AUV()
	: data_timer(this)
	, probe_timer(this)
	, poll_timer(this)
	, ack_timer(this)
	, list_probbed_node()
	, probbed_sink()
	, polling_index(0)
	, sink_inserted(true)
	, curr_trigger_packet(0)
	, curr_poll_packet(0)
	, curr_data_packet(0)
	, curr_probe_packet(0)
	, curr_tx_data_packet(0)
	, curr_ack_packet(0)
	, T_ack_timer(20)
	, sea_trial_(0)
	, print_stats_(0)
	, modem_data_bit_rate(900)
	, DATA_POLL_guard_time_(0)
	, n_run(0)
	, curr_state(UWPOLLING_AUV_STATUS_IDLE)
	, prev_state(UWPOLLING_AUV_STATUS_IDLE)
	, RxDataEnabled(false)
	, RxProbeEnabled(false)
	, TxEnabled(false)
	, N_expected_pkt(0)
	, packet_index(0)
	, curr_polled_node_address(0)
//	, curr_backoff_time(0)
	, curr_RTT(0)
	, curr_Tmeasured(0)
	, probe_rtt(0)
	, wrong_node_data_sent(0)
	, initial_time(0)
	, n_trigger_tx(0)
	, n_probe_rx(0)
	, n_ack_rx(0)
	, n_poll_tx(0)
	, begin(true)
	, stop_time(0)
	, pkt_time(0)
	, total_time(0)
	, TRIGGER_uid(0)
	, POLL_uid(0)
	, N_dropped_probe_pkts(0)
	, n_dropped_ack_pkts(0)
	, N_dropped_probe_wrong_state(0)
	, tx_buffer()
	, max_buffer_size(50)
	, uid_tx_pkt(1)
	, curr_is_sink(false)
	, max_tx_pkts(20)
	, n_pkts_to_tx(0)
	, last_pkt_uid(0)
	, enableAckRx(false)
	, acked(true)
	, rx_pkts_map()
	, ack_enabled(1)//modified
	, enable_adaptive_backoff(false)
	, backoff_LUT_file("")
	, lut_token_separator(',')
	, backoff_LUT()
	, probe_counters()
	, full_knowledge(false)
	, last_probe_lost(0)
{
	bind("max_payload_", (int *) &max_payload);
	bind("T_probe_guard_", (double *) &T_probe_guard);
	bind("T_min_", (double *) &T_min);
	bind("T_max_", (double *) &T_max);
	bind("T_guard_", (double *) &T_guard);
	bind("T_ack_timer_", (double *) &T_ack_timer);
	bind("max_polled_node_", (int *) &max_polled_node);
	bind("sea_trial_", (int *) &sea_trial_);
	bind("print_stats_", (int *) &print_stats_);
	bind("modem_data_bit_rate_", (int *) &modem_data_bit_rate);
	bind("n_run_", (int *) &n_run);
	bind("Data_Poll_guard_time_", (int *) &DATA_POLL_guard_time_);
	bind("max_buffer_size_", (uint *) &max_buffer_size);
	bind("max_tx_pkts_", (uint *) &max_tx_pkts);
	bind("ack_enabled_", (int *) &ack_enabled); //modified
	bind("full_knowledge_", (uint *) &full_knowledge);
	
	mac2phy_delay_ = 5e-3;
	if (max_polled_node <= 0) {
		max_polled_node = 1;
	}
	if (max_polled_node > MAX_POLLED_NODE) {
		max_polled_node = MAX_POLLED_NODE;
	}
	T_probe = T_max + T_probe_guard;
	probe_counters.resetCounters();
}

int
Uwpolling_AUV::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "initialize") == 0) {
			if (initialized == false)
				initInfo();
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
			if (debug_)
				std::cout << "UWPOLLING MAC address of the AUV is " << addr
						  << std::endl;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getRxFromNode") == 0) {
			tcl.resultf("%d", getRxPkts(atoi(argv[2])));
			return TCL_OK;
		} else if (strcasecmp(argv[1], "set_adaptive_backoff_LUT") == 0) {
			backoff_LUT_file = std::string(argv[2]);
			if (initBackoffLUT()) {
				return TCL_OK;
			} else {
				fprintf(stderr, "Uwpolling_AUV::LUT file not opened");
				return TCL_ERROR;
			}
		} else if (strcasecmp(argv[1], "setLUTSeparator") == 0) {
			lut_token_separator = *(argv[2]);
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}
bool 
Uwpolling_AUV::initBackoffLUT()
{
	ifstream input_file_;
	string line_;
	input_file_.open(backoff_LUT_file.c_str());
	double n;
	double b;
	if (input_file_.is_open()) {
		enable_adaptive_backoff = true;
		backoff_LUT.clear();
		while (std::getline(input_file_, line_)) {
			::std::stringstream line_stream(line_);
			
			line_stream >> n;
			line_stream.ignore(ENTRY_MAX_SIZE, lut_token_separator);
			line_stream >> b;
			backoff_LUT[n] = b;
		}
		input_file_.close();
		return true;
	} 
	return false;
}
double 
Uwpolling_AUV::getMaxBackoffTime()
{
	if(!enable_adaptive_backoff) {
		T_probe = T_max + T_probe_guard;
		return T_max;
	}
	uint n_n = 0;
	if(full_knowledge) {
		ClMsgUwPhyGetLostPkts m(true);
		sendSyncClMsg(&m);
		n_n = (m.getLostPkts() - last_probe_lost) 
			+	probe_counters.n_probe_received;
		last_probe_lost = m.getLostPkts();
	} else {
		n_n = probe_counters.getNumberOfNeighbors();
	}
	//if no probe received, either there are no nodes or backoff is too low. Use 
	//Max available backoff
	if(n_n == 0) {
		auto it = backoff_LUT.end();
		it --;
		T_max = it->second;
		T_probe = it->second+T_probe_guard;
		//return it->second;
	} else {
		auto it = backoff_LUT.lower_bound(n_n);
		if (it == backoff_LUT.end()) {
			it--;
			T_max = it->second;
			T_probe = it->second+T_probe_guard;
			//return it->second;
		} else {
			if(it == backoff_LUT.begin()) {
				T_max = it->second;
				T_probe = it->second+T_probe_guard;
				//return it->second;
			} else {
				if (it->first == n_n) {
					T_max = it->second;
					T_probe = it->second+T_probe_guard;
					//return it->second;
				} else {
					it--;
					double n_low = it->first;
					double b_low = it->second;
					double n_up = (++it)->first;
					double b_up = it->second;
					double optimal_backoff = linearInterpolator(
						n_n, n_low, n_up, b_low, b_up); 
					T_max = optimal_backoff;
					T_probe = optimal_backoff+T_probe_guard;
					//return optimal_backoff;
				}
			}
		}
	}
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  << ")::getMaxBackoffTime::N_probe_rx="
				  << probe_counters.n_probe_received
				  << "::N_probe_detected:" << probe_counters.n_probe_detected
				  << "::estimate_N_neighbor:" << n_n
				  << "::optimal_backoff:" << T_max
				  << std::endl;
	return T_max;
}

double
Uwpolling_AUV::linearInterpolator(
		double x, double x1, double x2, double y1, double y2)
{
	double m = (y1 - y2) / (x1 - x2);
	double q = y1 - m * x1;
	return m * x + q;
}

int
Uwpolling_AUV::crLayCommand(ClMessage *m)
{
	switch (m->type()) {
		default:
			return MMac::crLayCommand(m);
	}
}

void
Uwpolling_AUV::DataTimer::expire(Event *e)
{
	if (module->debug_)
		std::cout << module->getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << module->addr
				  << ")::DATA_TIMER::EXPIRED::Packet_received:"
				  << module->packet_index
				  << "::TOT_PACKET_EXPECTED:" << module->N_expected_pkt
				  << std::endl;
	timer_status = UWPOLLING_EXPIRED;
	module->DataTOExpired();
}

void
Uwpolling_AUV::ProbeTimer::expire(Event *e)
{
	timer_status = UWPOLLING_EXPIRED;
	if (module->debug_)
		std::cout << module->getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << module->addr
				  << ")::PROBE_TIMER::EXPIRED" << std::endl;
	module->ProbeTOExpired();
}
void
Uwpolling_AUV::AckTimer::expire(Event *e)
{
	timer_status = UWPOLLING_EXPIRED;
	if (module->debug_)
		std::cout << module->getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << module->addr
				  << ")::ACK_TIMER::EXPIRED" << std::endl;
	module->ackTOExpired();
}

void
Uwpolling_AUV::PollTimer::expire(Event *e)
{
	timer_status = UWPOLLING_EXPIRED;
	if (module->debug_)
		std::cout << module->getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << module->addr
				  << ")::POLL_TIMER::EXPIRED" << std::endl;
	module->ChangeNodePolled();
}

void
Uwpolling_AUV::DataTOExpired()
{
	RxDataEnabled = false;
	refreshReason(UWPOLLING_AUV_REASON_RX_DATA_TO);
	ChangeNodePolled();
}

void
Uwpolling_AUV::ProbeTOExpired()
{
	RxProbeEnabled = false;
	TxEnabled = true;
	refreshReason(UWPOLLING_AUV_REASON_PROBE_TO_EXPIRED);
	if (polling_index > 0) {
		stateTx();
	} else {
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  	<< ")::PROBE_TIMER::NO_PROBE_RX" << std::endl;
		stateIdle();
	}
		
}

void
Uwpolling_AUV::ackTOExpired()
{
	enableAckRx = false;
	acked = false;
	ChangeNodePolled();
}

void
Uwpolling_AUV::Mac2PhyStartTx(Packet *p)
{
	MMac::Mac2PhyStartTx(p);
}

void
Uwpolling_AUV::stateTxTrigger()
{
	refreshState(UWPOLLING_AUV_STATUS_TX_TRIGGER);
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  << ")::TX_TRIGGER::T_max= " << T_max << " T_min = " << T_min
				  << "::uid = " << TRIGGER_uid << std::endl;
	Packet *p = Packet::alloc();
	hdr_cmn *cmh = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_TRIGGER *triggerh = HDR_TRIGGER(p);
	cmh->ptype() = PT_TRIGGER;
	mach->set(MF_CONTROL, addr, MAC_BROADCAST);
	mach->macSA() = addr;
	mach->macDA() = MAC_BROADCAST;
	triggerh->t_fin() = (int) (getMaxBackoffTime() * 100);
	triggerh->t_in() = (int) (T_min * 100);
	TRIGGER_uid++;
	triggerh->TRIGGER_uid_ = TRIGGER_uid;
	cmh->size() = sizeof(hdr_TRIGGER);
	if (sea_trial_ && print_stats_)
		out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					   << "::Uwpolling_AUV(" << addr << ")::TX_TRIGGER_ID_"
					   << TRIGGER_uid << endl;
	curr_trigger_packet = p->copy();
	Packet::free(p);
	refreshReason(UWPOLLING_AUV_REASON_TX_TRIGGER);
	TxTrigger();
	probe_counters.resetCounters();
}

void
Uwpolling_AUV::TxTrigger()
{
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV (" << addr << ")::TX_TRIGGER"
				  << std::endl;
	incrCtrlPktsTx();
	incrTriggerTx();
	Mac2PhyStartTx(curr_trigger_packet);
}

void
Uwpolling_AUV::Phy2MacEndTx(const Packet *p)
{
	hdr_cmn *cmh = hdr_cmn::access(p);
	if (cmh->ptype() == PT_TRIGGER) {
		refreshReason(UWPOLLING_AUV_REASON_TX_TRIGGER);
		stateWaitProbe();
	} else if (cmh->ptype() == PT_POLL) {
		stateWaitData();
	} else {
		if (n_tx_pkts < n_pkts_to_tx) {
			stateTxData();
		} else {
			n_tx_pkts = 0;
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  << ")::LAST_DATA_PKTS_TX" << std::endl;

			if (!ack_enabled) { //modified
				ChangeNodePolled();
			} else {
				stateWaitAck();
			}	  
			
			//ChangeNodePolled();
		}
	}
}

void
Uwpolling_AUV::stateWaitProbe()
{
	refreshState(UWPOLLING_AUV_STATUS_WAIT_PROBE);
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  << ")::scheduling_PROBE_TIMER_T= " << T_probe << std::endl;
	RxProbeEnabled = true;
	if (T_probe < 0) {
		cerr << "Scheduling PROBE timer ----> negative value " << T_probe
			 << endl;
		exit(1);
	}
	probe_timer.schedule(T_probe);
}

void
Uwpolling_AUV::stateWaitData()
{
	refreshState(UWPOLLING_AUV_STATUS_WAIT_DATA);
	RxDataEnabled = true;
	double data_timer_value = GetDataTimerValue();
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  << ")STATE_WAIT_DATA::Data_Timer = " << data_timer_value
				  << std::endl;
	data_timer.schedule(data_timer_value);
	packet_index = 0;
}

double
Uwpolling_AUV::GetDataTimerValue()
{
	UpdateRTT();
	double timer_value = 0;
	if (!sea_trial_) {
		computeTxTime(UWPOLLING_DATA_PKT);
		timer_value = (N_expected_pkt * Tdata) + 2*curr_RTT + T_guard;
	} else {
		double Tdata = T_guard + (max_payload * 8.0) / modem_data_bit_rate;
		timer_value = (N_expected_pkt * Tdata) + 2*curr_RTT + T_guard;
	}
	if (timer_value <= 0) {
		timer_value = 20;
		if (debug_)
		cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
			 	<< ")::WARNING::Data Timer Value <= 0. Setting to 20 sec"
			 << std::endl;
		
	}
	return timer_value;
}

void
Uwpolling_AUV::Phy2MacStartRx(const Packet *p)
{
	hdr_cmn *cmh = hdr_cmn::access(p);
	if (cmh->ptype() == PT_PROBE || cmh->ptype() == PT_PROBE_SINK) {
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << ")::PHY2MACSTARTRX::PROBE_PACKET" << std::endl;
	} else if (cmh->ptype() == PT_ACK_SINK) {
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << ")::PHY2MACSTARTRX::ACK_PACKET" << std::endl;
	} else {
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << ")::PHY2MACSTARTRX::DATA_PACKET" << std::endl;
	}
}

void
Uwpolling_AUV::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *cmh = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_MPhy *ph = hdr_MPhy::access(p);
	int dest_mac = mach->macDA();
	double gen_time = ph->txtime;
	double received_time = ph->rxtime;
	double diff_time = received_time - gen_time;

	if (cmh->error()) {
		if (cmh->ptype_ == PT_POLL) {
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						<< ")::PHY2MACENDRX_DROP_POLL" << endl;
		} else if (cmh->ptype_ == PT_PROBE || cmh->ptype() == PT_PROBE_SINK) {
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::PHY2MACENDRX_DROP_PROBE_FROM_" << mach->macSA()
						  << endl;
			if (sea_trial_ && print_stats_)
				out_file_stats << left << "[" << getEpoch() << "]::"
							   << NOW << "::Uwpolling_AUV(" << addr
							   << ")::PHY2MACENDRX::DROP_PROBE_FROM_NODE_"
							   << mach->macSA() << endl;
			incrDroppedProbePkts();
			probe_counters.n_probe_detected++;
		} else if ((cmh->ptype_ == PT_ACK_SINK) && ack_enabled) { //modified
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						<< ")::PHY2MACENDRX_DROP_ACK" << endl;
			incrDroppedAckPkts();
		}else if (cmh->ptype_ == PT_TRIGGER) {
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						<< ")::PHY2MACENDRX_DROP_TRIGGER" << endl;
		} else {
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::PHY2MACENDRX::DROP_DATA_FROM_NODE_"
						  << mach->macSA() << endl;
			if (sea_trial_ && print_stats_)
				out_file_stats << left << "[" << getEpoch() << "]::"
							   << NOW << "Uwpolling_AUV(" << addr
							   << ")::PHY2MACENDRX::DROP_DATA_FROM_NODE_"
							   << mach->macSA() << endl;
			incrErrorPktsRx();
		}
		refreshReason(UWPOLLING_AUV_REASON_PACKET_ERROR);
		drop(p, 1, UWPOLLING_AUV_DROP_REASON_ERROR);
	} else {
		if ((dest_mac == addr) || (dest_mac == (int) MAC_BROADCAST)) {
			if (cmh->ptype_ == PT_PROBE || cmh->ptype() == PT_PROBE_SINK) {
				probe_counters.incrementCounters();
				incrCtrlPktsRx();
				refreshReason(UWPOLLING_AUV_REASON_PROBE_RECEIVED);
				curr_probe_packet = p->copy();
				Packet::free(p);
				probe_rtt = diff_time;
				if (debug_)
					std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
							  << ")::PHY2MACENDRX::PROBE, probe rtt"<< 2*probe_rtt << std::endl;
				
				if (begin) {
					begin = false;
					initial_time = NOW;
				}
				stateRxProbe();
			} else  if ((cmh->ptype() == PT_ACK_SINK) && ack_enabled) { //modified
				incrAckRx();
				curr_ack_packet = p->copy();
				Packet::free(p);
				stateAckRx();
				//check ACK value and diacard acked packets
				
			} else {
				if ((cmh->ptype_ != PT_POLL) && (cmh->ptype_ != PT_PROBE) &&
						(cmh->ptype_ != PT_TRIGGER)) {
					refreshReason(UWPOLLING_AUV_REASON_DATA_RX);
					curr_data_packet = p->copy();
					Packet::free(p);
					pkt_time = NOW;
					stateRxData();
				}
			}
		} else {
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::PHY2MACENDRX::DROP_DATA_WRONG_DEST_"
						  << mach->macDA() << endl;
			incrXCtrlPktsRx();
			if (sea_trial_ && print_stats_)
				out_file_stats << left << "[" << getEpoch() << "]::" << NOW
							   << "::Uwpolling_AUV(" << addr
							   << ")::PHY2MACENDRX::DROP_DATA_WRONG_DEST_"
							   << mach->macDA() << endl;
			refreshReason(UWPOLLING_AUV_REASON_PACKET_ERROR);
			drop(p, 1, UWPOLLING_AUV_DROP_REASON_WRONG_RECEIVER);
		}
	}
}

void
Uwpolling_AUV::computeTxTime(UWPOLLING_PKT_TYPE pkt)
{
	Packet *fuzzy_pkt;
	fuzzy_pkt = Packet::alloc();
	switch (pkt) {
		case (UWPOLLING_POLL_PKT): {
			hdr_cmn *cmh = HDR_CMN(fuzzy_pkt);
			hdr_mac *mach = HDR_MAC(fuzzy_pkt);
			mach->macSA() = addr;
			cmh->size() = sizeof(id_poll) * polling_index;
			Tpoll = Mac2PhyTxDuration(fuzzy_pkt);
		} break;
		case (UWPOLLING_PROBE_PKT): {
			hdr_cmn *cmh = HDR_CMN(fuzzy_pkt);
			hdr_mac *mach = HDR_MAC(fuzzy_pkt);
			mach->macSA() = addr;
			cmh->size() = sizeof(hdr_PROBE);
			Tprobe = Mac2PhyTxDuration(fuzzy_pkt);
		} break;
		case (UWPOLLING_TRIGGER_PKT): {
			hdr_cmn *cmh = HDR_CMN(fuzzy_pkt);
			cmh->size() = sizeof(hdr_TRIGGER);
			hdr_mac *mach = HDR_MAC(fuzzy_pkt);
			mach->macSA() = addr;
			// cmh->size() = 10;
			Ttrigger = Mac2PhyTxDuration(fuzzy_pkt);
		} break;
		case (UWPOLLING_DATA_PKT): {
			hdr_cmn *cmh = HDR_CMN(fuzzy_pkt);
			hdr_mac *mach = HDR_MAC(fuzzy_pkt);
			mach->macDA() = addr;
			cmh->size() = max_payload;
			Tdata = Mac2PhyTxDuration(fuzzy_pkt);
		}
	}
	Packet::free(fuzzy_pkt);
}

void
Uwpolling_AUV::UpdateRTT()
{
	curr_RTT = curr_Tmeasured;
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  << ")::UPDATE_RTT--->RTT = " << curr_RTT << std::endl;
}

void
Uwpolling_AUV::stateRxData()
{
	refreshState(UWPOLLING_AUV_STATUS_RX_DATA);
	if (RxDataEnabled) {
		hdr_mac *mach = HDR_MAC(curr_data_packet);
		hdr_uwcbr *cbrh = HDR_UWCBR(curr_data_packet);
		int mac_sa = mach->macSA();
		if (sea_trial_ && print_stats_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::Uwpolling_AUV(" << addr
						   << ")::STATE_RX_DATA::RX_DATA_ID_" << cbrh->sn_
						   << "_FROM_NODE_" << mach->macSA() << endl;
		if (mach->macSA() == curr_polled_node_address) {
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::STATE_RX_DATA::RX_DATA_ID_" << cbrh->sn_
						  << "_FROM_NODE_" << mach->macSA() << endl;
			incrDataPktsRx();
			packet_index++;
			rx_pkts_map[curr_polled_node_address]++;
			sendUp(curr_data_packet);

			if (packet_index == N_expected_pkt) {
				refreshReason(UWPOLLING_AUV_REASON_LAST_PACKET_RECEIVED);
				data_timer.force_cancel();
				if (sea_trial_) {
					poll_timer.schedule(DATA_POLL_guard_time_);
				} else {
					ChangeNodePolled();
				}
				if (debug_)
					std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::RECEIVED_" << packet_index << "_PKTS"
						  << "_FROM_NODE_" << mac_sa << endl;
			}

		} else {
			incrWrongNodeDataSent();
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::STATE_RX_DATA::DROP_DATA_WRONG_NODE_"
						  << mach->macSA_ << std::endl;
			if (sea_trial_ && print_stats_)
				out_file_stats << left << "[" << getEpoch() << "]::" << NOW
							   << "::Uwpolling_AUV(" << addr
							   << ")::STATE_RX_DATA::DROP_DATA_FROM_NODE_"
							   << mach->macDA() << "_NOT_POLLED" << endl;
			drop(curr_data_packet, 1, UWPOLLING_AUV_DROP_REASON_WRONG_STATE);
		}
	} else {
		incrXDataPktsRx();
		hdr_mac *mac = HDR_MAC(curr_data_packet);
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << ")::STATE_RX_DATA::DROP_DATA_WRONG_STATE="
					  << status_info[curr_state] << std::endl;
		if (sea_trial_ && print_stats_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::Uwpolling_AUV(" << addr
						   << ")::STATE_RX_DATA::DROP_DATA_FROM_NODE_"
						   << mac->macDA() << "_NOT_POLLED" << endl;
		drop(curr_data_packet, 1, UWPOLLING_AUV_DROP_REASON_WRONG_STATE);
	}
}

void
Uwpolling_AUV::ChangeNodePolled()
{
	RxDataEnabled = false;
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				  << ")::CHANGE_NODE_POLLED::" << std::endl;
	if (polling_index > 1) {
		list_probbed_node.erase(list_probbed_node.begin());
		polling_index--;
		TxEnabled = true;
		stateTx();
	} else {
		list_probbed_node.clear();
		refreshReason(UWPOLLING_AUV_REASON_LAST_POLLED_NODE);
		stateIdle();
	}
}

void
Uwpolling_AUV::stateRxProbe()
{
	if (RxProbeEnabled) {
		hdr_cmn* ch = HDR_CMN(curr_probe_packet);
		hdr_mac *mach = HDR_MAC(curr_probe_packet);

		refreshState(UWPOLLING_AUV_STATUS_RX_PROBES);
		incrCtrlPktsRx();
		incrProbeRx();
		
		if (ch->ptype() == PT_PROBE) {
			hdr_PROBE *probeh = HDR_PROBE(curr_probe_packet);
			addNode2List();

			if (debug_) 
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
						<< ")::STATE_RX_PROBE"
						<< "::N_packet_expected=" << probeh->n_pkts_
						<< "::id_node=" << probeh->id_node_
						//<< "::BackOff Time=" << probeh->backoff_time_
						<< std::endl;
		
			if (sea_trial_ && print_stats_)
				out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::Uwpolling_AUV(" << addr << ")::RX_PROBE_ID_"
						   << probeh->PROBE_uid_ << "_FROM_NODE_"
						   << mach->macSA() << endl;

		} else  if (ch->ptype() == PT_PROBE_SINK) {
			hdr_PROBE_SINK *probeh = HDR_PROBE_SINK(curr_probe_packet);
			//handle ack in the probe packet
			if(!acked) {
				acked = true;
				handleProbeAck();
			}
			
			addSink2List();

			if (debug_) 
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
					  << ")::STATE_RX_PROBE_SINK"
					  << "::id_sink=" << probeh->id_sink_
					  << "::ACK_ID=" << probeh->id_ack_
					  << std::endl;
		
			if (sea_trial_ && print_stats_)
				out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::Uwpolling_AUV(" << addr << ")::RX_PROBE_ID_"
						   << probeh->PROBE_uid_ << "_FROM_NODE_"
						   << mach->macSA() << endl;


		} else {
			std::cerr << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << "wrong packet type" << std::endl;
		}

		Packet::free(curr_probe_packet);
		
		if (polling_index == max_polled_node) {
			refreshReason(UWPOLLING_AUV_REASON_MAX_PROBE_RECEIVED);
			RxProbeEnabled = false;
			TxEnabled = true;
			probe_timer.force_cancel();
			stateTx();
		}
	} else {
		incrDroppedProbeWrongState();
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << "):: dropping Probe Wrong State "
					  << status_info[curr_state] << std::endl;
		drop(curr_probe_packet, 1, UWPOLLING_AUV_DROP_REASON_WRONG_STATE);
	}
}

void
Uwpolling_AUV::stateTxPoll()
{
	if (TxEnabled) {
		if (polling_index > 0) {
			refreshState(UWPOLLING_AUV_STATUS_TX_POLL);
			Packet *p = Packet::alloc();
			hdr_cmn *cmh = hdr_cmn::access(p);
			hdr_mac *mach = HDR_MAC(p);
			cmh->ptype() = PT_POLL;
			cmh->size() = 2;
			mach->set(MF_CONTROL, addr, MAC_BROADCAST);
			mach->macSA() = addr;
			mach->macDA() = MAC_BROADCAST;
			curr_poll_packet = p->copy();
			Packet::free(p);
			hdr_POLL *pollh = HDR_POLL(curr_poll_packet);
			POLL_uid++;
			pollh->POLL_uid_ = POLL_uid;
			pollh->id_ = curr_node_id;
			pollh->POLL_time() = getPollTime();
			if (sea_trial_ && print_stats_)
				out_file_stats << left << "[" << getEpoch() << "]::" << NOW
							   << "::Uwpolling_AUV(" << addr << ")::TX_POLL_ID_"
							   << POLL_uid << "_NODE_POLLED_" << curr_node_id
							   << endl;
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::STATE_TX_POLL::Node polled = " << pollh->id_
						  << "::NODE_TO_POLL= " << polling_index << std::endl;
			TxPoll();
		} else {
			if (debug_)
				std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
						  << ")::STATE_TX_POLL--->IDLE--->No node to POLL"
						  << std::endl;
			stateIdle();
		}
	} else {
		if (debug_)
			std::cerr << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << ")---> going in stateTxPoll from WRONG STATE---> "
						 "current_state: "
					  << status_info[curr_state] << std::endl;
	}
}

void
Uwpolling_AUV::TxPoll()
{
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr << ")::TX_POLL"
				  << std::endl;
	incrCtrlPktsTx();
	incrPollTx();
	Mac2PhyStartTx(curr_poll_packet);
}

void
Uwpolling_AUV::SortNode2Poll()
{
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr << ")::SORT_NODE_TO_POLL "
				  << std::endl;
	/**ELEMENTS SHOULD BE ORDERED, ONLY NEED TO INSERT SINK IN THE RIGHT POSITION*/
	uint n_pkts = tx_buffer.size() + temp_buffer.size();
	if (!sink_inserted) {
		std::vector<probbed_node>::iterator it = list_probbed_node.begin();
		for(; it!= list_probbed_node.end(); it++) {
			if (n_pkts > max_tx_pkts)
				break;
			n_pkts += (*it).n_pkts;
		}
		list_probbed_node.insert(it,probbed_sink);
		sink_inserted = true;
	}
	std::vector<probbed_node>::iterator it = list_probbed_node.begin();
	

	if (!list_probbed_node.empty()){
		curr_polled_node_address = list_probbed_node[0].mac_address;
		N_expected_pkt = list_probbed_node[0].n_pkts;
		curr_Tmeasured = list_probbed_node[0].Tmeasured;
		curr_node_id = list_probbed_node[0].id_node;
		curr_is_sink = list_probbed_node[0].is_sink_;
	}
}

void
Uwpolling_AUV::stateIdle()
{
	refreshState(UWPOLLING_AUV_STATUS_IDLE);
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr << ")::IDLE STATE "
				  << std::endl;
	data_timer.force_cancel();
	probe_timer.force_cancel();
	polling_index = 0;
	TxEnabled = false;
	n_tx_pkts = 0;
	stateTxTrigger();
}

void
Uwpolling_AUV::initInfo()
{
	initialized = true;

	if (sea_trial_ && print_stats_) {
		std::stringstream stat_file;
		stat_file << "./Uwpolling_AUV_" << addr << "_" << n_run << ".out";
		out_file_stats.open(stat_file.str().c_str(), std::ios_base::app);
		out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					   << "::Uwpolling_AUV(" << addr << ")::NS_START" << endl;
	}

	status_info[UWPOLLING_AUV_STATUS_IDLE] = "Idle state";
	status_info[UWPOLLING_AUV_STATUS_RX_DATA] = "Receiving Data Packet";
	status_info[UWPOLLING_AUV_STATUS_RX_PROBES] = "Receiving Probe Packet";
	status_info[UWPOLLING_AUV_STATUS_TX_POLL] = "Transmitting a POLL Packet";
	status_info[UWPOLLING_AUV_STATUS_TX_TRIGGER] =
			"Transmitting a TRIGGER Packet";
	status_info[UWPOLLING_AUV_STATUS_TX_DATA] = "Transmitting a DATA Packet";		
	status_info[UWPOLLING_AUV_STATUS_WAIT_PROBE] = "Waiting for a PROBE Packet";
	status_info[UWPOLLING_AUV_STATUS_WAIT_DATA] = "Waiting for a DATA Packet";
	status_info[UWPOLLING_AUV_STATUS_WAIT_ACK] = "Waiting for an ACK Packet";

	pkt_type_info[UWPOLLING_DATA_PKT] = "Data Packet";
	pkt_type_info[UWPOLLING_POLL_PKT] = "Poll packet";
	pkt_type_info[UWPOLLING_TRIGGER_PKT] = "Trigger packet";
	pkt_type_info[UWPOLLING_PROBE_PKT] = "Probe packet";

	reason_info[UWPOLLING_AUV_REASON_DATA_RX] = "Received a Data Packet";
	reason_info[UWPOLLING_AUV_REASON_PROBE_RECEIVED] =
			"Received a PROBE packet";
	reason_info[UWPOLLING_AUV_REASON_TX_POLL] = "POLL Packet transmitted";
	reason_info[UWPOLLING_AUV_REASON_TX_TRIGGER] = "Trigger Packet transmitted";
	reason_info[UWPOLLING_AUV_REASON_RX_DATA_TO] = "Receiving Data Time-Out";
	reason_info[UWPOLLING_AUV_REASON_LAST_PACKET_RECEIVED] =
			"Last Packet Received from the Node";
	reason_info[UWPOLLING_AUV_REASON_LAST_POLLED_NODE] =
			"Last Node from the list Polled";
	reason_info[UWPOLLING_AUV_REASON_MAX_PROBE_RECEIVED] =
			"Maximum Number of Probe Received";
	reason_info[UWPOLLING_AUV_REASON_PACKET_ERROR] = "Packet Error";
	reason_info[UWPOLLING_AUV_REASON_PROBE_TO_EXPIRED] =
			"Receiving PROBE Time-Out";
}

void
Uwpolling_AUV::waitForUser()
{
	std::string response;
	std::cout << "Press Enter to continue";
	std::getline(std::cin, response);
}

void
Uwpolling_AUV::stop_count_time()
{
	stop_time = pkt_time;
	total_time = stop_time - initial_time;
}



void
Uwpolling_AUV::recvFromUpperLayers(Packet *p)
{
	if ((uint)(tx_buffer.size() + temp_buffer.size()) < max_buffer_size) {
		//since packets in the temp_buffer can be reinserted in the tx_buffer
		//after an ACK, i need to count for the temp buffer size as well, 
		//to avoid buffer overflow when packets are reinserted in the tx buffer
		hdr_uwcbr *cbrh = HDR_UWCBR(p);
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::UWPOLLING_AUV(" << addr
					  << ")::RECV_FROM_U_LAYERS_ID_" << cbrh->sn_ 
					  << "::MAC_UID_"<< uid_tx_pkt << endl;
		if (sea_trial_ && print_stats_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::Uwpolling_AUV(" << addr
						   << ")::RECV_FROM_U_LAYERS_ID_" << cbrh->sn_ << endl;
		hdr_AUV_MULE* auvh = HDR_AUV_MULE(p);
		auvh->pkt_uid() = uid_tx_pkt++; 
		hdr_cmn* ch = HDR_CMN(p);
		ch->size() += sizeof(hdr_AUV_MULE);
		tx_buffer.push_back(p);
	} else {
		if (sea_trial_ && print_stats_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::Uwpolling_AUV(" << addr << ")::DROP_FULL_QUEUE"
						   << endl;
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
					  << ")::DROP_FULL_QUEUE" << std::endl;
		incrDiscardedPktsTx();
		drop(p, 1, UWPOLLING_AUV_DROP_REASON_BUFFER_FULL);
	}
}

void
Uwpolling_AUV::stateTx()
{
	if (TxEnabled) {
		SortNode2Poll();	
		if(curr_is_sink){

			if (!ack_enabled) { //modified, removed all ack things
				if (tx_buffer.empty()) {
					if (debug_)
						std::cout << getEpoch() << "::" << NOW << "::UWPOLLING_AUV(" << addr
							<< ")::NO_PKTS_TO_TX, TX BUFFER IS EMPTY" << std::endl;
					ChangeNodePolled();
				} else {
					n_pkts_to_tx = std::min((uint)tx_buffer.size(),max_tx_pkts);
					std::deque<Packet*>::iterator it = tx_buffer.begin();
					for(uint i = 1; i < n_pkts_to_tx; i++) {
						it++;
					}
					hdr_AUV_MULE* auvh = HDR_AUV_MULE(*it);
					last_pkt_uid = auvh->pkt_uid();
					if(debug_)
						std::cout << getEpoch() << "::" << NOW << "::UWPOLLING_AUV(" << addr
							<< ")TX_DATA_PKTS, N PKTS TO TX=" << n_pkts_to_tx 
							<< ", UID LAST PKTS=" << last_pkt_uid <<std::endl;
					
					computeTxTime(UWPOLLING_DATA_PKT);
					//question: i don't undestand ack_timer.schedule(T_ack_timer);	
					stateTxData();				
				}

			}else {
				if (tx_buffer.empty()) {
					if (debug_)
						std::cout << getEpoch() << "::" << NOW << "::UWPOLLING_AUV(" << addr
							<< ")::NO_PKTS_TO_TX, TX BUFFER IS EMPTY" << std::endl;
					ChangeNodePolled();
				} else {
					if (!acked) { 
						handleNoAck();
					}
					n_pkts_to_tx = std::min((uint)tx_buffer.size(),max_tx_pkts);
					std::deque<Packet*>::iterator it = tx_buffer.begin();
					for(uint i = 1; i < n_pkts_to_tx; i++) {
						it++;
					}
					hdr_AUV_MULE* auvh = HDR_AUV_MULE(*it);
					last_pkt_uid = auvh->pkt_uid();
					acked = false;
					if(debug_)
						std::cout << getEpoch() << "::" << NOW << "::UWPOLLING_AUV(" << addr
							<< ")TX_DATA_PKTS, N PKTS TO TX=" << n_pkts_to_tx 
							<< ", UID LAST PKTS=" << last_pkt_uid <<std::endl;
					
					if (!sea_trial_) {
						computeTxTime(UWPOLLING_DATA_PKT);
					} else {
						Tdata = T_guard + (max_payload * 8.0)/modem_data_bit_rate;
					}
					double T_ack_timer = n_pkts_to_tx*Tdata;
					T_ack_timer = T_ack_timer + 0.6* T_ack_timer;
					double est_rtt_ack = 2*curr_Tmeasured;
					T_ack_timer = T_ack_timer +  est_rtt_ack + T_guard;
					if (debug_)
						std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
							<< ")::scheduling_ACK_TIMER_T= " << T_ack_timer 
							<< " Est_rtt= " << est_rtt_ack << std::endl;
					ack_timer.schedule(T_ack_timer);
					stateTxData();
				}
			}

		} else {
			stateTxPoll();
		}
	}
}

void
Uwpolling_AUV::stateTxData()
{
	if (TxEnabled) {
		refreshState(UWPOLLING_AUV_STATUS_TX_DATA);

		curr_tx_data_packet = tx_buffer.front();
		tx_buffer.pop_front();
		temp_buffer.push_back(curr_tx_data_packet->copy());
				
		hdr_AUV_MULE* auvh = HDR_AUV_MULE(curr_tx_data_packet);
		auvh->last_pkt_uid() = last_pkt_uid;
		n_tx_pkts++;	

		txData();
	}
}

void
Uwpolling_AUV::txData()
{
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr << ")::TX_DATA"
				  << std::endl;
	incrDataPktsTx();
	Mac2PhyStartTx(curr_tx_data_packet);
}

void
Uwpolling_AUV::stateWaitAck()
{
	refreshState(UWPOLLING_AUV_STATUS_WAIT_ACK);
	TxEnabled = false;
	enableAckRx = true;
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr
				<< ")::stateWaitAck"<<  std::endl;
}

void
Uwpolling_AUV::stateAckRx()
{
	if(enableAckRx) {
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
					<< ")::stateAckRx()ACK received" << std::endl;
		refreshState(UWPOLLING_AUV_STATUS_RX_ACK);
		enableAckRx = false;
		acked = true;
		handleAck();
		
		ack_timer.force_cancel();
		Packet::free(curr_ack_packet);
		ChangeNodePolled();
	}
}

void
Uwpolling_AUV::handleAck()
{
	hdr_ACK_SINK* ackh = HDR_ACK_SINK(curr_ack_packet);
	std::vector<uint16_t> & ack_list = ackh->id_ack();

	Packet* front_p = temp_buffer.back();
	hdr_AUV_MULE* auvh_tmp = HDR_AUV_MULE(front_p);
	if (ack_list.front() == auvh_tmp->pkt_uid()+1) {
		if (debug_)
			std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
					<< ")::handleAck()::NO_ERROR" << std::endl;
		while (!temp_buffer.empty()) {
			Packet* p = temp_buffer.back();
			temp_buffer.pop_back();
			Packet::free(p);
		}
	} else {
		while (!temp_buffer.empty()) {

			Packet* p = temp_buffer.back();
			temp_buffer.pop_back();
			hdr_AUV_MULE* auvh = HDR_AUV_MULE(p);
			if(std::find(ack_list.begin(),ack_list.end(),auvh->pkt_uid()) 
					!= ack_list.end()) {
				if (debug_)
					std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
						<< ")::handleAck()::RX_ACK,PKT_ID" 
						<< auvh->pkt_uid()<< std::endl;
				tx_buffer.push_front(p);
			} else {
				Packet::free(p);
			}
		}
	}
}

void
Uwpolling_AUV::handleNoAck()
{
	if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
				<< ")::handleNoAck()::reinsert pkts in tx buffer" << std::endl;
	while (!temp_buffer.empty()) {
		Packet* p = temp_buffer.back();
		temp_buffer.pop_back();
		tx_buffer.push_front(p);
	}
}

void 
Uwpolling_AUV::handleProbeAck()
{
	hdr_PROBE_SINK *probeh = HDR_PROBE_SINK(curr_probe_packet);
	
	
	Packet* front_p = temp_buffer.back();
	hdr_AUV_MULE* auvh_tmp = HDR_AUV_MULE(front_p);
	if(probeh->id_ack() == auvh_tmp->pkt_uid()+1) {
		if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
				<< ")::handleProbeAck()::rx ACK in probe pkt,no error" 
				<< std::endl;
		while (!temp_buffer.empty()) {
			Packet* p = temp_buffer.back();
			temp_buffer.pop_back();
			Packet::free(p);
		}
	} else {
		if (debug_)
		std::cout << getEpoch() << "::" << NOW << "::Uwpolling_AUV(" << addr 
				<< ")::handleProbeAck()::rx ACK in probe pkt,ack_id = " 
				<< probeh->id_ack() << std::endl;
		while (!temp_buffer.empty()) {
			Packet* p = temp_buffer.back();
			temp_buffer.pop_back();
			hdr_AUV_MULE* auvh = HDR_AUV_MULE(p);
			if (auvh->pkt_uid() >= probeh->id_ack()) {
				tx_buffer.push_front(p);
			} else {
				Packet::free(p);
			}
		}
	}

}

void 
Uwpolling_AUV::addNode2List() 
{
	hdr_PROBE *probeh = HDR_PROBE(curr_probe_packet);
	hdr_mac *mach = HDR_MAC(curr_probe_packet);

	//create element of the list
	probbed_node new_node;
	new_node.is_sink_ = false;
	new_node.id_node = probeh->id_node();
	new_node.n_pkts = probeh->n_pkts();
	new_node.time_stamp = ((double)probeh->ts())/100;
	new_node.mac_address = mach->macSA();
	new_node.Tmeasured = probe_rtt; //FTT
	if (rx_pkts_map.find(new_node.mac_address) != rx_pkts_map.end()) {
		new_node.policy_weight = 
				((double)new_node.n_pkts)/rx_pkts_map[new_node.mac_address];
	} else { //never received a packet form this node
		new_node.policy_weight = DBL_MAX; //MAX PRIORITY
	}
	std::vector<probbed_node>::iterator it = list_probbed_node.begin();
	for (;it != list_probbed_node.end(); it++) {
		if ((*it).policy_weight < new_node.policy_weight) {
			break;
		}
	}
	list_probbed_node.insert(it,new_node);
	polling_index ++;
}

void 
Uwpolling_AUV::addSink2List()
{
	hdr_PROBE_SINK *probeh = HDR_PROBE_SINK(curr_probe_packet);
	hdr_mac *mach = HDR_MAC(curr_probe_packet);
	probbed_sink.is_sink_ = true;
	probbed_sink.mac_address = mach->macSA();
	probbed_sink.Tmeasured = probe_rtt; //FTT
	probbed_sink.id_ack = probeh->id_ack();
	probbed_sink.id_node = probeh->id_sink();
	probbed_sink.policy_weight = -1; //NOT_VALID
	polling_index++;
	sink_inserted = false;
}

uint16_t
Uwpolling_AUV::getPollTime()
{
	double poll_time = 0;
	if (!sea_trial_) {
		computeTxTime(UWPOLLING_DATA_PKT);
	} else {
		Tdata = T_guard + (max_payload * 8.0) / modem_data_bit_rate;
	}
	for (uint i = 0; i < list_probbed_node.size(); i++) {    		
		if (list_probbed_node[i].is_sink_) {
			poll_time += (std::min((uint)tx_buffer.size()+
					(uint)temp_buffer.size(),max_tx_pkts) * Tdata) + 
					2*list_probbed_node[i].Tmeasured + T_guard; //check if a specific values is needed when buffer is empty
		} else {
			poll_time += (list_probbed_node[i].n_pkts * Tdata) + 
					2*list_probbed_node[i].Tmeasured + T_guard;
		}	
	}
	return (uint16_t)(std::ceil(poll_time));  
}

uint 
Uwpolling_AUV::getRxPkts(int mac_addr)
{
	std::map<int,uint>::iterator it = rx_pkts_map.find(mac_addr);
	if (it != rx_pkts_map.end()) {
		return it->second;
	} else {
		return -1;
	}

}
