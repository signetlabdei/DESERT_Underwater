//
// Copyright (c) 2019 Regents of the SIGNET lab, University of Padova.
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
//

/**
 * @file   uwpolling_SINK.cpp
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Implementation of Uwpolling_SINK class
 *
 */

#include "uwpolling_SINK.h"
#include "mmac.h"
#include "mac.h"
#include "uwcbr-module.h"
#include "mphy_pktheader.h"
#include "rng.h"

#include <algorithm>
#include <sstream>
#include <sys/time.h>

bool Uwpolling_SINK::initialized = false;
map<Uwpolling_SINK::UWPOLLING_SINK_STATUS, string> Uwpolling_SINK::status_info;
map<Uwpolling_SINK::UWPOLLING_SINK_REASON, string> Uwpolling_SINK::reason_info;
map<Uwpolling_SINK::UWPOLLING_PKT_TYPE, string> Uwpolling_SINK::pkt_type_info;


/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwpollingModule_SINK_Class : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwpollingModule_SINK_Class()
		: TclClass("Module/UW/POLLING/SINK")
	{
	}

	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new Uwpolling_SINK());
	}
} class_module_uwpolling_sink;

Uwpolling_SINK::Uwpolling_SINK()
	: MMac()
	, T_data(0)
	, T_data_gurad(0)
	, backoff_tuner(0)
	, sink_id(0)
	, RxDataEnabled(false)
	, Triggered(false)
	, triggerEnabled(true)
	, T_in(0)
	, T_fin(0)
	, BOffTime(0)
	, AUV_mac_addr(0)
	, n_probe_sent(0)
	, n_trigger_received(0)
	, n_trigger_dropped(0)
	, n_ack_sent(0)
	, n_curr_rx_pkts(0)
	, expected_id(1)
	, last_rx(0)
	, send_ACK(false)
	, missing_id_list()
	, expected_last_id(0)
	, prev_expect_last_id(0)
	, duplicate_pkts(0)
	, first_rx_pkt(true)
	, PROBE_uid(0)
	, curr_data_pkt(0)
	, curr_probe_pkt(0)
	, curr_trigger_pkt(0)
	, curr_ack_pkt(0)
	, last_reason()
	, curr_state(UWPOLLING_SINK_STATUS_IDLE)
	, prev_state(UWPOLLING_SINK_STATUS_IDLE)
	, backoff_timer(this)
	, rx_data_timer(this)
	, fout(0)
	, out_file_stats(0)
	, sea_trial(0)
	, print_stats(0)
	, n_run(0)
	, useAdaptiveTdata(0)
	, ack_enabled(1)
	, max_n_ack(100)
	, T_guard(1)
	, max_payload(125)
	, modem_data_bit_rate(900)
{
	bind("T_data_guard_", (double *) &T_data_gurad);
	bind("backoff_tuner_", (double *) &backoff_tuner);
	bind("sink_id_", (uint *) &sink_id);
	bind("sea_trial_", (int *) &sea_trial);
	bind("n_run_", (int *) &n_run);
	bind("print_stats_", (int *) &print_stats);
	bind("useAdaptiveTdata_", (int *) &useAdaptiveTdata);
	bind("ack_enabled_", (int *) &ack_enabled);
	bind("max_n_ack_", (int *) &max_n_ack);
	bind("T_guard_", (double *) &T_guard);
	bind("max_payload_", (int *) &max_payload);
	bind("modem_data_bit_rate_", (int *) &modem_data_bit_rate);

	if (T_data_gurad <= 0)
		T_data_gurad = MIN_T_DATA;
}

Uwpolling_SINK::~Uwpolling_SINK()
{
}

int
Uwpolling_SINK::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "initialize") == 0) {
			if (!initialized)
				initInfo();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getProbeSent") == 0) {
			tcl.resultf("%d", getProbeSent());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getAckSent") == 0) {
			tcl.resultf("%d", getAckSent());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getTriggerReceived") == 0) {
			tcl.resultf("%d", getTriggerReceived());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getTriggerDropped") == 0) {
			tcl.resultf("%d", getTriggerDropped());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getDuplicatedPkts") == 0) {
			tcl.resultf("%d", getDuplicatedPkt());
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setMacAddr") == 0) {
			addr = atoi(argv[2]);
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);	
}

int
Uwpolling_SINK::crLayCommand(ClMessage *m)
{
	switch (m->type()) {
		default:
			return MMac::crLayCommand(m);
	}
}

void
Uwpolling_SINK::BackOffTimer::expire(Event *e)
{
	if (module->debug_) {
		std::cout << NOW << "Uwpolling_SINK(" << module->addr
				  << ")::BACKOFF_TIMER::EXPIRED" << std::endl;
	}
	timer_status = UWPOLLING_EXPIRED;
	module->BackOffTimerExpired();
}

void
Uwpolling_SINK::Rx_Data_Timer::expire(Event *e)
{
	if (module->debug_) {
		std::cout << NOW << "Uwpolling_SINK(" << module->addr
				  << ")::RX_DATA_TIMER::EXPIRED" << std::endl;
	}
	timer_status = UWPOLLING_EXPIRED;
	module->RxDataTimerExpired();
}

void 
Uwpolling_SINK::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();
	if (ch->error()) { //discard packet
		//DISCARD PACKET AND INCREMENTS DEDICATED COUNTER
		if (ch->ptype() == PT_TRIGGER) {
			if (debug_)
				std::cout << NOW << "Uwpolling_SINK(" << addr
						  << ")::PHY2MACENDRX::DROP_TRIGGER" << std::endl;
			if (sea_trial && print_stats)
				out_file_stats << left << "[" << getEpoch() << "]::" << NOW
							   << "::Uwpolling_SINK(" << addr
							   << ")::PKT_TRIGGER_DROP_ERROR" << endl;
			incrTriggerDropped();
		} 
		refreshReason(UWPOLLING_SINK_REASON_PKT_ERROR);
		drop(p, 1, UWPOLLING_SINK_DROP_REASON_ERROR);
	} else {
		if (dest_mac == addr || dest_mac == (int) MAC_BROADCAST) { //Check MAC address
			if (ch->ptype() == PT_TRIGGER) {
				curr_trigger_pkt = p->copy();
				Packet::free(p);
				refreshReason(UWPOLLING_SINK_REASON_RX_TRIGGER);
				stateRxTrigger();
			} else if (ch->ptype() != PT_POLL && ch->ptype() != PT_PROBE &&
					ch->ptype() != PT_ACK_SINK) { //data pkt
				
				curr_data_pkt = p->copy();
				Packet::free(p);
				refreshReason(UWPOLLING_SINK_REASON_RX_DATA);
				stateRxData();
			}  else {
				if (ch->ptype() == PT_POLL && RxDataEnabled && useAdaptiveTdata) {
					hdr_POLL *pollh = HDR_POLL(p);
					rx_data_timer.schedule(pollh->POLL_time());
					if (debug_)
						std::cout << NOW << "Uwpolling_SINK(" << addr
								<< ")::Resched rx data timer, timeout=" 
								<< pollh->POLL_time() << std::endl;
				}
				// PT_PROBE, PT_ACK_SINK and PT_POLL are not considerd by 
				// the SINK
				drop(p, 1, UWPOLLING_SINK_DROP_REASON_UNKNOWN_TYPE);
			}
		} else {
			//PACKET NOT FOR ME, DISCARD IT
			drop(p, 1, UWPOLLING_SINK_DROP_REASON_WRONG_RECEIVER);
		}
	}
}

void
Uwpolling_SINK::stateIdle()
{
	/* Reset timer and move to STATE_IDLE*/
	if (debug_) {
		std::cout << NOW << "Uwpolling_SINK(" << addr << ")::IDLE_STATE"
				  << std::endl;
	}
	refreshState(UWPOLLING_SINK_STATUS_IDLE);
	Triggered = false;
	RxDataEnabled = false;
	send_ACK = false;
	n_curr_rx_pkts = 0;
	first_rx_pkt = true;
	triggerEnabled = true;
	backoff_timer.force_cancel();
	rx_data_timer.force_cancel();
}

void 
Uwpolling_SINK::stateRxTrigger()
{
	if (triggerEnabled) {
		if (RxDataEnabled && !send_ACK) { //probe was sent, but no packets has been received by AUV
			RxDataEnabled = false;
			rx_data_timer.force_cancel();
		}
		//triggerEnabled = false;
		Triggered = true;
		refreshState(UWPOLLING_SINK_STATUS_RX_TRIGGER);
		incrTriggerReceived();
		hdr_mac *mach = HDR_MAC(curr_trigger_pkt);
		AUV_mac_addr = mach->macSA();
		if (debug_) 
			std::cout << NOW << "Uwpolling_SINK(" << addr
				  << ")::STATE_RX_TRIGGER, rx from MAC="
				  << AUV_mac_addr<< std::endl; 
		
		hdr_TRIGGER* trig_h = HDR_TRIGGER(curr_trigger_pkt);
		T_fin = (double)trig_h->t_fin()/100;
		T_in = (double)trig_h->t_in()/100;
		T_data = T_fin + T_data_gurad;
		BOffTime = getBackOffTime();
		if (BOffTime < 0) {
			std::cerr << "Negative backoff value " << std::endl;
			BOffTime = T_fin;
		}
		backoff_timer.schedule(BOffTime);
		Packet::free(curr_trigger_pkt);
	} else {
		Packet::free(curr_trigger_pkt);
		if (debug_) 
			std::cout << NOW << "Uwpolling_SINK(" << addr
					<< ")::RX_TRIGGER_NOT_ENABLED" << std::endl;
	}
}

double 
Uwpolling_SINK::getBackOffTime()
{
	double random = ((double)(RNG::defaultrng()->uniform(INT_MAX) % (int)((T_fin-T_in)*100)))/100 + T_in;
	if (debug_)
		std::cout << NOW << "Uwpolling_SINK(" << addr
				  << ")::BACKOFF_TIMER_VALUE = " << backoff_tuner * random
				  << std::endl;
	return (backoff_tuner * random);
}

void 
Uwpolling_SINK::BackOffTimerExpired()
{
	if (Triggered) {
		refreshReason(UWPOLLING_SINK_REASON_BACKOFF_TIMER_EXPIRED);
		stateTxProbe();
	} else {
		if (debug_) {
			std::cout << NOW << "Uwpolling_SINK(" << addr
					<< ") Backoff timer expired but node not triggered" 
					<< std::endl;
		}
	}
}

void 
Uwpolling_SINK::stateTxProbe()
{
	if (Triggered) {
		refreshState(UWPOLLING_SINK_STATUS_TX_PROBE);
		initPkt(UWPOLLING_PROBE_PKT);
		hdr_PROBE_SINK *probehdr = HDR_PROBE_SINK(curr_probe_pkt);
		if (sea_trial && print_stats) {
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					<< "::Uwpolling_SINK(" << addr
					<< ")::TX_PROBE_ID_" << probehdr->PROBE_uid() << std::endl;
		}
		if (debug_) {
			std::cout << NOW << "Uwpolling_SINK(" << addr
					<< ")::STATE_TX_PROBE_ID_" << probehdr->PROBE_uid() 
					<< std::endl;
		}
		TxProbe();
	}
}

void 
Uwpolling_SINK::initPkt(UWPOLLING_PKT_TYPE pkt_type)
{
	if (pkt_type == UWPOLLING_PROBE_PKT) {
		Packet *p = Packet::alloc();
		hdr_PROBE_SINK *probehdr = HDR_PROBE_SINK(p);
		hdr_cmn *ch = hdr_cmn::access(p);
		hdr_mac *mach = HDR_MAC(p);
		ch->ptype() = PT_PROBE_SINK;
		ch->size() = sizeof(hdr_PROBE_SINK);
		mach->ftype() = MF_CONTROL;
		mach->macDA() = AUV_mac_addr;
		mach->macSA() = addr;
		probehdr->id_sink() = sink_id;
		probehdr->PROBE_uid() = PROBE_uid++;
		if(missing_id_list.empty()){
			probehdr->id_ack() = last_rx+1;
		} else {
			probehdr->id_ack() = missing_id_list.front(); 
		}
		curr_probe_pkt = p->copy();
		Packet::free(p);
	} else if (pkt_type == UWPOLLING_ACK_PKT) {
		Packet *p = Packet::alloc();
		hdr_ACK_SINK *ackh = HDR_ACK_SINK(p);
		hdr_cmn *ch = hdr_cmn::access(p);
		hdr_mac *mach = HDR_MAC(p);
		ch->ptype() = PT_ACK_SINK;
		
		mach->ftype() = MF_CONTROL;
		mach->macDA() = AUV_mac_addr;
		mach->macSA() = addr;
		std::vector<uint16_t> & ack = ackh->id_ack();
		std::list<uint16_t>::iterator it = missing_id_list.begin();
		if (it == missing_id_list.end()) {
			if (ack.size() <= max_n_ack) {
				ack.push_back(last_rx+1);
			} else {
				std::cout << "Uwpolling_SINK(" << addr << ")::max number of "
					<< "ack reached" << std::endl;
			}
		} else {
			for (; it!= missing_id_list.end(); it++){
				if (ack.size() <= max_n_ack) {
					ack.push_back(*it);	
				} else {
					std::cout << "Uwpolling_SINK(" << addr << ")::max number "
						<< "of ack reached" << std::endl;
					break;
				}
			}
		}
		ch->size() = ack.size() * sizeof(uint16_t);
		if (debug_)
			std::cout << NOW << "Uwpolling_SINK(" << addr
					<< ")::ack list size=" << ack.size() << std::endl;
		curr_ack_pkt = p->copy();
		Packet::free(p);
	}
}

void 
Uwpolling_SINK::TxProbe()
{
	incrProbeSent();
	incrCtrlPktsTx();
	Mac2PhyStartTx(curr_probe_pkt);
}


void 
Uwpolling_SINK::Mac2PhyStartTx(Packet *p)
{
	MMac::Mac2PhyStartTx(p); // Send down the packet to phy layer
}

void 
Uwpolling_SINK::Phy2MacEndTx(const Packet *p)
{
	hdr_cmn* ch = HDR_CMN(p); 
	if (ch->ptype() == PT_PROBE_SINK) {
		refreshReason(UWPOLLING_SINK_REASON_TX_PROBE);
		stateWaitData();
	} else if (ch->ptype() == PT_ACK_SINK) {
		refreshReason(UWPOLLING_SINK_REASON_TX_ACK);
		stateIdle();
	}
}

void 
Uwpolling_SINK::stateWaitData()
{
	refreshState(UWPOLLING_SINK_STATUS_WAIT_DATA);
	RxDataEnabled = true;
	//Compute T_data in some way
	if (debug_)
		std::cout << NOW << "Uwpolling_SINK(" << addr
				  << ")STATE_WAIT_DATA::Data_Timer = " << T_data << std::endl;
	rx_data_timer.schedule(T_data);
	n_curr_rx_pkts = 0;
}

void
Uwpolling_SINK::stateRxData()
{
	if (RxDataEnabled) {
		refreshState(UWPOLLING_SINK_STATUS_RX_DATA);
		send_ACK = true; //At least one packet is received, so an ACK has to be sent
		triggerEnabled = false;
		hdr_mac *mach = HDR_MAC(curr_data_pkt);
		hdr_uwcbr *cbrh = HDR_UWCBR(curr_data_pkt);
		hdr_AUV_MULE *auvh = HDR_AUV_MULE(curr_data_pkt);
		hdr_cmn* ch = HDR_CMN(curr_data_pkt);
		hdr_MPhy *ph = HDR_MPHY(curr_data_pkt);
		uint16_t auv_uid = auvh->pkt_uid();

		if (first_rx_pkt) {
			first_rx_pkt = false;
			prev_expect_last_id = expected_last_id;
			expected_last_id = auvh->last_pkt_uid();
			double expected_pkts = 0;
			if (!ack_enabled) {
				expected_pkts = expected_last_id - auv_uid + 1;
			} else {
				expected_pkts = max(expected_last_id - prev_expect_last_id, 0) +
									missing_id_list.size();
			}

			double duration = 0;
			if (ph->duration > 0) {
				duration = sea_trial ? (ph->duration + T_guard) : ph->duration;
			} else {
				duration = (max_payload*8.0)/modem_data_bit_rate;
				duration = sea_trial ? (duration + T_guard) : duration;
			}						
			double new_dataTO = (expected_pkts-1)*duration;//-1 because we don't have to consider the packet just received
			new_dataTO = new_dataTO + 0.5*new_dataTO; 
			if(debug_)
				std::cout << NOW << "Uwpolling_SINK(" << addr
						<< ")RX_DATA_TIMER::RESCHEDULED::Data_Timer = " 
						<< new_dataTO << " n_packets " << expected_pkts << " pckt duration " << ph->duration <<std::endl;
			rx_data_timer.schedule(new_dataTO);		
		}
		
		if (sea_trial && print_stats) {
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::Uwpolling_SINK(" << addr
						   << ")::STATE_RX_DATA::RX_DATA_ID_" << cbrh->sn_
						   << "_FROM_NODE_" << mach->macSA() << endl;
		}
		if (debug_) {
			std::cout << NOW << "Uwpolling_SINK(" << addr
					<< ")::STATE_RX_DATA::RX_DATA_ID_" << cbrh->sn_
					<< "::POLLING_UID_" << auv_uid
					<< "_FROM_NODE_" << mach->macSA() << endl;
		}
		
		if (auv_uid == last_rx+1) { //no missing packet
			last_rx = auv_uid;
		} else if(auv_uid > last_rx+1){ //lost some packets in between
			uint16_t n_miss = (auv_uid-1) - last_rx;
			addMissPkt2List(n_miss);
			last_rx = auv_uid;
		} 
		
		if(auv_uid <= prev_expect_last_id) { //retransmission
			
			std::list<uint16_t>::iterator it = 
				std::find(missing_id_list.begin(),missing_id_list.end(),
						auv_uid);
			if (it != missing_id_list.end()) { //the packet was previously lost
				//erase packet from list
				missing_id_list.erase(it);
			} else { // duplicate packet
				if(debug_)
					std::cout << NOW << "Uwpolling_SINK(" << addr
							<< ")received duplicate packet with id2 " 
							<< auv_uid << std::endl;
				
				incrDuplicatedPkt(); //the packet will be sendUp and discarded 
								  //by the application
			}
		}

		incrDataPktsRx();
		n_curr_rx_pkts++;
		ch->size() -= sizeof(hdr_AUV_MULE);
		sendUp(curr_data_pkt);
		
		if (expected_last_id == auv_uid) {
			rx_data_timer.force_cancel();
			n_curr_rx_pkts = 0;
			RxDataEnabled = false;
			first_rx_pkt = true;
			refreshReason(UWPOLLING_SINK_REASON_MAX_DATA_RECEIVED);
			if (!ack_enabled){
				stateIdle();
			} else {
				stateTxAck();
			}
		}
		
	}
}

void 
Uwpolling_SINK::RxDataTimerExpired()
{
	RxDataEnabled = false;
	n_curr_rx_pkts = 0;
	first_rx_pkt = true;
	refreshReason(UWPOLLING_SINK_REASON_RX_DATA_TIMER_EXPIRED);
	if (send_ACK) {
		if (expected_last_id > last_rx) {
			uint16_t n_miss = expected_last_id - last_rx;
			addMissPkt2List(n_miss);
		}
		if (!ack_enabled){ //modified 
			stateIdle();
		} else {
			stateTxAck();
		}
	} else { //no packet has been received
		stateIdle();
	}
	 
}

void 
Uwpolling_SINK::addMissPkt2List(uint16_t n_pkts)
{
	for (uint16_t i = 1; i <= n_pkts; i++) {

		std::list<uint16_t>::iterator it = std::find(missing_id_list.begin(),
				missing_id_list.end(), last_rx+i);

		if (it == missing_id_list.end()) {
			missing_id_list.push_back(last_rx+i);
		} 
	}
}


void
Uwpolling_SINK::stateTxAck()
{
	if (send_ACK) {
		if(debug_)
			std::cout << NOW << "Uwpolling_SINK(" << addr
					<< ")stateTxAck()" << std::endl;
		refreshState(UWPOLLING_SINK_STATUS_TX_ACK);
		initPkt(UWPOLLING_ACK_PKT);
		txAck();
	}
}

void
Uwpolling_SINK::txAck()
{
	send_ACK = false;
	incrAckSent();
	incrCtrlPktsTx();
	Mac2PhyStartTx(curr_ack_pkt);
}

void 
Uwpolling_SINK::initInfo()
{
	initialized = true;

	if (sea_trial && print_stats) {
		std::stringstream stat_file;
		stat_file << "./Uwpolling_SINK_" << addr << "_" << n_run << ".out";
		std::cout << stat_file.str().c_str() << endl;
		out_file_stats.open(stat_file.str().c_str(), std::ios_base::app);
		out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					   << "::Uwpolling_SINK(" << addr << ")::NS_START" << endl;
	}

	status_info[UWPOLLING_SINK_STATUS_IDLE] = "Idle State";
	status_info[UWPOLLING_SINK_STATUS_RX_TRIGGER] ="Receiving Trigger from AUV";
	status_info[UWPOLLING_SINK_STATUS_TX_PROBE] = "Transmitting Probe to AUV";
	status_info[UWPOLLING_SINK_STATUS_WAIT_DATA] =
			"Waiting for the reception of DATA packet";
	status_info[UWPOLLING_SINK_STATUS_RX_DATA] = "Receiving Data from AUV";
	status_info[UWPOLLING_SINK_STATUS_TX_ACK] = "Transmitting ACK from AUV";

	pkt_type_info[UWPOLLING_DATA_PKT] = "Data Packet from Application Layer";
	pkt_type_info[UWPOLLING_POLL_PKT] = "Poll packet";
	pkt_type_info[UWPOLLING_TRIGGER_PKT] = "Trigger packet";
	pkt_type_info[UWPOLLING_PROBE_PKT] = "Probe packet";
	pkt_type_info[UWPOLLING_ACK_PKT] = "Ack packet";

	reason_info[UWPOLLING_SINK_REASON_RX_DATA] = 
			"Received a Data Packet";
	reason_info[UWPOLLING_SINK_REASON_RX_TRIGGER] =
			"Receiving a trigger from the AUV";
	reason_info[UWPOLLING_SINK_REASON_PKT_ERROR] =
			"Received a Corrupted Packet";
	reason_info[UWPOLLING_SINK_REASON_TX_PROBE] =
			"Transmitting PROBE to the AUV";
	reason_info[UWPOLLING_SINK_REASON_TX_ACK] = 
			"Transmitting ACK to the AUV";
	reason_info[UWPOLLING_SINK_REASON_BACKOFF_TIMER_EXPIRED] =
			"BackOff expired";
	reason_info[UWPOLLING_SINK_REASON_RX_DATA_TIMER_EXPIRED] =
			"Data timer expired Time-Out";
	reason_info[UWPOLLING_SINK_REASON_NOT_SET] = 
			"Reason not set";
	reason_info[UWPOLLING_SINK_REASON_MAX_DATA_RECEIVED] = 
			"Maximum Number of Data packets Received";
	reason_info[UWPOLLING_SINK_REASON_WRONG_TYPE] =
			"Receiving a Packet of Wrong Type";
	reason_info[UWPOLLING_SINK_REASON_WRONG_RECEIVER] =
			"Packet not for this receiver";
	reason_info[UWPOLLING_SINK_REASON_WRONG_STATE] =
			"Receiving a Packet in Wrong State (--> NOT enabled to receive "
			"packets)";
}

void
Uwpolling_SINK::waitForUser()
{
	std::string response;
	std::cout << "Press Enter to continue";
	std::getline(std::cin, response);
}