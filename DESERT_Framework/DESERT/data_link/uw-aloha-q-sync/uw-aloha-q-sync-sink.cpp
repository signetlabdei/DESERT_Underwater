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
//

/**
 * @file   uw-aloha-q-sync-sink.cpp
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UWALOHAQ-SYNC-SINK</i>.
 *
 */



#include "uw-aloha-q-sync-sink.h"
#include <iostream>
#include <stdint.h>
#include <clmsg-discovery.h>
#include <mac.h>
#include <uwmmac-clmsg.h>
#include <uwcbr-module.h>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <ctime>
#include <vector>
#include <queue>

using std::vector; 

extern packet_t PT_ALOHAQ_SYNC_ACK;

/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwAloha_Q_Sync_SINKModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	UwAloha_Q_Sync_SINKModuleClass()
		: TclClass("Module/UW/ALOHAQ_SYNC_SINK")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwAloha_Q_Sync_SINK());
	}

} class__module_uw_aloha_q_sync_sink;

void
UwAlohaQSyncTimerSink::expire(Event *e)
{
	((UwAloha_Q_Sync_SINK*) module)->handleTimerExpiration();
}


UwAloha_Q_Sync_SINK::UwAloha_Q_Sync_SINK()
	: MMac()
	, alohaq_sync_sink_timer(this)
	, sink_status(IDLE)
	, sink_slot_status(TRANSMIT_PERIOD)
	, start_time(0)
	, out_file_stats(0)
	, max_queue_size(0)
	, name_label_("")
	, t_guard (0.06)
{
	bind("queue_size_", (int *) &max_queue_size);
	bind("debug_", (int *) &debug_);
	bind("sea_trial_", (int *) &sea_trial_);
	bind("HDR_size_", (int *) &HDR_size);
	bind("ACK_size_", (int *) &ACK_size);
	bind("mac2phy_delay_", (double *) &mac2phy_delay_);
	bind("t_dp", (double *) &t_dp);
	bind("t_prop_max", (double *) &t_prop_max);
	bind("nn", (int *) &nn);
	bind("slot_duration_factor", (double *) &slot_duration_factor);

	if (max_queue_size < 0) {
		cerr << NOW << " UwALOHAQ() not valid max_queue_size < 0!! set to 1 by default " 
			 	<< std::endl;
		max_queue_size = 1;
	}
	
	if (mac2phy_delay_ <= 0) {
		cerr << NOW << " UwALOHAQ() not valid mac2phy_delay_ < 0!!" 
				<< "set to 1e-9 by default " << std::endl; 
		mac2phy_delay_ = 1e-9;
	}
}

UwAloha_Q_Sync_SINK::~UwAloha_Q_Sync_SINK()
{
}


void
UwAloha_Q_Sync_SINK::handleTimerExpiration()
{

	if (debug_) {
		std::cout << NOW << "Sink switched state " << std::endl; 
	}
	if (sink_slot_status == RECV_PERIOD) {	
		sink_slot_status = TRANSMIT_PERIOD;
		txAck();
		alohaq_sync_sink_timer.resched(t_prop_max + nn/5*t_guard);	
	} else {
		succ_macs.clear();		
		sink_slot_status = RECV_PERIOD;
		alohaq_sync_sink_timer.resched(nn*slot_duration_factor*t_dp + t_prop_max + t_dp);	
	}

	
}

void UwAloha_Q_Sync_SINK::recvFromUpperLayers(Packet *p)
{
	//SINK MODE - No receiving from upper layers	
	Packet::free(p);	
}

void 
UwAloha_Q_Sync_SINK::txAck()
{
	Packet *ack_pkt = Packet::alloc();
	initPkt(ack_pkt);
		
	Mac2PhyStartTx(ack_pkt);
	
	if(debug_){
		std::cout << NOW << " SINK ( " << addr << ") :  SINK Sending ack packet "
				<< std::endl;
	}
	
	sink_status = TRANSMITTING;
}

void
UwAloha_Q_Sync_SINK::Phy2MacEndTx(const Packet *p)
{	
	if (sea_trial_)
		out_file_stats << left << "[" << "]::" << NOW
				<< "SINK(" << addr << ")::Finished transmitting ACK"
				<< std::endl;
	if (debug_) {	
		std::cout << NOW << " Sink ( " << addr << 
				") :  Tx_ finished transmitting ACK" << std::endl;
	}
	
	incrDataPktsTx();	
	sink_status = IDLE;
}

void
UwAloha_Q_Sync_SINK::Phy2MacStartRx(Packet *p)
{

	if (sink_status == IDLE) {
		if (debug_) {
			std::cout << NOW << " Sink ( " << addr << ") :  Rx_ started receiving" 
					<< std::endl;
		}
		sink_status = RECEIVING;
	} else {
		if (debug_) {
			std::cout << "Sink is not in IDLE mode, can't receive" << std::endl;
		}
		Packet::free(p);
	}
}

void
UwAloha_Q_Sync_SINK::Phy2MacEndRx(Packet *p)
{

	hdr_cmn *ch = HDR_CMN(p);
	packet_t rx_pkt_type = ch->ptype();
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();
	int src_mac = mach->macSA();

	if (ch->error()) {
		if (debug_)
			cout << NOW << " Sink(" << addr
					<< ")::Phy2MacEndRx() dropping corrupted pkt from node = "
					<< src_mac << std::endl;

			incrErrorPktsRx();
			Packet::free(p);
	} else {
		if (dest_mac != addr) {
			Packet::free(p);

			if (debug_)
				std::cout << NOW << " Sink " << addr << ": Packet not for me, packet is for "
							 << dest_mac << std::endl;
		} else if (rx_pkt_type == PT_ALOHAQ_SYNC_ACK) {
				
			std::cout << NOW << "Sink received ACK packet, dropping" << src_mac
					<< std::endl;
			Packet::free(p);
		} else if (sink_slot_status == TRANSMIT_PERIOD) {
			
			std::cout << NOW << "Received outside receiving frame interval" << src_mac
					<< std::endl;
			Packet::free(p);
			
		} else {
			sendUp(p);
			incrDataPktsRx();	
			succ_macs.push_back(src_mac);

			if (debug_)
				std::cout << NOW << " ID " << addr
						<< ": Received data packet from " << src_mac
						<< std::endl;
			if (sea_trial_)
				out_file_stats << left << "[" << "]::" << NOW
							<< "::ALOHAQ Sink_node(" << addr
							<< ")::PCK_FROM:" << src_mac << std::endl;
		}
	}
		
	sink_status = IDLE;
}


void
UwAloha_Q_Sync_SINK::initPkt(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);

	
	ch->ptype() = PT_ALOHAQ_SYNC_ACK;
	ch->size() = ACK_size * nn;
	mach->macSA() = addr;
	mach->macDA() = 0;
	
	aloha_q_sync_ACK *ack = ALOHAQ_SYNC_ACK_HDR_ACCESS(p);
	
	ack -> set_succ_macs(succ_macs); 
	enable = false;
		
}

int
UwAloha_Q_Sync_SINK::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	
	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {			
			start(start_time);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "stop") == 0) {
			stop();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_upper_data_pkts_rx") == 0) {
			tcl.resultf("%d", up_data_pkts_rx);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_sent_pkts") == 0) {
			tcl.resultf("%d", data_pkts_tx);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_recv_pkts") == 0) {
			tcl.resultf("%d", data_pkts_rx);
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setStartTime") == 0) {
			start_time = atof(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setMacAddr") == 0) {
			addr = atoi(argv[2]);
			if (debug_)
				cout << "ALOHAQ SINK MAC address is "  << addr
					 << std::endl;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setLogLabel") == 0) {
			name_label_ = argv[2];
			if (debug_)
				cout << "ALOHAQ_SINK name_label_ " << name_label_
					 << std::endl;
			return TCL_OK;
		} 
	}
	return MMac::command(argc, argv);
}


void
UwAloha_Q_Sync_SINK::start(double delay)
{

	enable = true;
	srand((unsigned) (time(0) + addr));
	if (sea_trial_) {
		std::stringstream stat_file;
		stat_file << "./ALOHAQ_SINK.out";
		std::cout << stat_file.str().c_str() << std::endl;
		out_file_stats.open(stat_file.str().c_str(), std::ios_base::app);	
	}
	
	succ_macs.reserve(30); 
	
	alohaq_sync_sink_timer.sched(0);
}

void
UwAloha_Q_Sync_SINK::stop()
{	
	succ_macs.clear();
	
	enable = false;
	alohaq_sync_sink_timer.cancel();
}

int
UwAloha_Q_Sync_SINK::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_UWMMAC_ENABLE) {
		ClMsgUwMmacEnable *command = dynamic_cast<ClMsgUwMmacEnable *>(m);
	}
	return MMac::recvSyncClMsg(m);
}
