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
 * @file   uw-aloha-q-sync-node.cpp
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * \brief Provides the implementation of UWAlohaQSyncNode Class
 *
 */

#include "uw-aloha-q-sync-node.h"
#include <iostream>
#include <stdint.h>
#include <rng.h>
#include <clmsg-discovery.h>
#include <mac.h>
#include <uwmmac-clmsg.h>
#include <uwcbr-module.h>
#include <vector>
#include <algorithm>

using std::vector; 


/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwAloha_Q_Sync_NODEModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	UwAloha_Q_Sync_NODEModuleClass()
		: TclClass("Module/UW/ALOHAQ_SYNC_NODE")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwAloha_Q_Sync_NODE());
	}

} class_module_uw_aloha_q_sync_node;

void
UwAlohaQSyncTimer::expire(Event *e)
{
	((UwAloha_Q_Sync_NODE*) module)->handleTimerExpiration();
}

UwAloha_Q_Sync_NODE::UwAloha_Q_Sync_NODE()
	: MMac()
	, alohaq_sync_timer(this)
	, start_time(0)
	, transceiver_status(IDLE)
	, slot_status (RECEIVE)
	, out_file_stats(0)
	, HDR_size(0)
	, packet_sent_curr_frame(0)
	, enable(true)
	, curr_slot(0)
	, curr_subslot(0)
	, my_curr_slot(0)
	, my_curr_subslot(0)
	, t_guard (0.06)
{
	bind("max_queue_size_", (int *) &max_queue_size);
	bind("debug_", (int *) &debug_);
	bind("sea_trial_", (int *) &sea_trial_);
	bind("HDR_size_", (int *) &HDR_size);
	bind("t_dp", (double *) &t_dp);
	bind("t_prop_max", (double *) &t_prop_max);
	bind("drop_old_", (int *) &drop_old_);
	bind("checkPriority_", (int *) &checkPriority);
	bind("mac2phy_delay_", (double *) &mac2phy_delay_);
	bind("slot_duration_factor", (double *) &slot_duration_factor);
	bind("nn", (int *) &nn);
	bind("subslot_num", (int *) &subslot_num);


	if (max_queue_size < 0) {
		cerr << NOW << " UwALOHAQ() not valid max_queue_size < 0!! set to 1 by default " 
			 << std::endl;
		max_queue_size = 1;
	}
	if (slot_duration_factor < 1) {
		cerr << NOW << " UwALOHAQ() not valid slot_duration_factor < 1!!" 
				<< "set to 1.5 by default " << std::endl;
		slot_duration_factor = 1.5;
	}
	if (drop_old_ == 1 && checkPriority == 1) {
		cerr << NOW << " UwALOHAQ() drop_old_ and checkPriority cannot be set both to 1!! "
			 << "checkPriority set to 0 by default " << std::endl;
		checkPriority = 0; 
	}
	if (mac2phy_delay_ <= 0) {
		cerr << NOW << " UwALOHAQ() not valid mac2phy_delay_ < 0!! set to 1e-9 by default "
			 << std::endl; 
		mac2phy_delay_ = 1e-9;
	}
	if (subslot_num < 1) {
		cerr << NOW << " UwALOHAQ() not valid subslot_num < 1!! set to 1 by default "
			 << std::endl; 
		subslot_num = 1;
	}
	
}


void
UwAloha_Q_Sync_NODE::findMySlot()
{

	std::vector<int> max_arr_slot; 
	std::vector<int> max_arr_subslot;
	
	double max = Q_table[0][0];
	int i;
	int j;
	
	for (i=0; i < nn; i++){
		for(j=0; j < subslot_num; j++){		
	    		if (max < Q_table[i][j]) {
	        		max = Q_table[i][j];
	    		}
	    	}
	}	
	for (i=0; i < nn; i++) {
		for (j=0; j < subslot_num; j++) {
	    		if (max == Q_table[i][j]) { 
	        		max_arr_slot.push_back(i); 
	        		max_arr_subslot.push_back(j); 
	    		}
	    	}
	}
	int my_slot;
	int my_subslot;
	
	if (max_arr_slot.size() == 1) {
		my_slot = max_arr_slot[0];
		my_subslot = max_arr_subslot[0];
	} else {
		int pos = RNG::defaultrng()->uniform(0, max_arr_slot.size());
		my_slot = max_arr_slot[pos];
		my_subslot = max_arr_subslot[pos];
	}
	
	my_curr_slot = my_slot;
	my_curr_subslot = my_subslot;
	
}

void
UwAloha_Q_Sync_NODE::updateQ_table(int ack_received)
{
	Q_table[my_curr_slot][my_curr_subslot] = Q_table[my_curr_slot][my_curr_subslot] 
	+ 0.1 * (ack_received  - Q_table[my_curr_slot][my_curr_subslot]);
		
	// PRINT Q TABLE
	if (debug_) {
		std::cout << NOW << " ALOHAQ node ID (" << addr 
			<< ")My Q table now is " << std::endl;	
		for (int i = 0; i < nn; i++) {
			for(int j = 0; j < subslot_num; j++) {
	   			std::cout << Q_table[i][j] << endl;
	    		}
	    		std::cout << "/" << endl;
		}
	}
}

void
UwAloha_Q_Sync_NODE::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	if (buffer.size() < max_queue_size) {
		initPkt(p);
		buffer.push_back(p);
		
 	} else {
 		drop(p, 1, "BUFFER_FULL");
		//Packet::free(p);
	}
}

void
UwAloha_Q_Sync_NODE::txData()
{
	if (packet_sent_curr_frame ==  0) {
		if (transceiver_status == IDLE) { 
			if (buffer.size() > 0) {
				Packet *p = buffer.front();
				buffer.pop_front();
				//sendDown(p);				
				Mac2PhyStartTx(p);
				incrDataPktsTx();
				packet_sent_curr_frame = 1;
			}
		} else {
			if (debug_) {
				std::cout << NOW << " ALOHAQ NODE STATUS -  " << addr << 
						" : Not in IDLE state - cannot transmit " << transceiver_status << std::endl;
			}
		}
	} if (debug_) {
		std::cout << NOW << " AlohaQ(" << addr
			<< ")::already_tx sent a packet  " << std::endl;
	}
}

void
UwAloha_Q_Sync_NODE::Mac2PhyStartTx(Packet *p)
{
	MMac::Mac2PhyStartTx(p);
		
	if (transceiver_status == IDLE)
		transceiver_status = TRANSMITTING;
	else if (debug_) {
		std::cout << NOW << " AlohaQ node -  " 
				<< addr << " Transceiver already active - cannot receive" << std::endl;
	}
	if (debug_)
		std::cout << NOW << " AlohaQ node -  " << addr 
				<< ": Sending packet - now waiting ACK - Mac2PhyStarTx" << std::endl;
	
}
					  

void
UwAloha_Q_Sync_NODE::Phy2MacEndTx(const Packet *p)
{
	transceiver_status = IDLE;
	
	std::cout << NOW << " AlohaQ Node -  " << addr 
			<< " Tx_ packet transmitted ----- Phy2MacEndTx" << std::endl;

}

void
UwAloha_Q_Sync_NODE::Phy2MacStartRx(Packet *p)
{

	if (slot_status == TRANSMIT) {
		drop(p, 1, "RECEIVING_IN_TRANSMISSION_WINDOW");
		if (debug_)
			std::cout << NOW << " AlohaQ Node -  " << addr << 
					" Dropped packet in transmitting frame part" << std::endl;		
	} else {
		std::cout << NOW << " AlohaQ Node -  " << addr << 
				" Rx_ started receiving ----- Phy2MacStartRx-- " << std::endl;	
	
		if (transceiver_status == IDLE) {
			transceiver_status = RECEIVING;
		} else if (debug_) {
			std::cout << NOW << " AlohaQ Node -  " << addr << 
					" Transceiver already active - cant receive" << std::endl;
		}
	}	
}

void
UwAloha_Q_Sync_NODE::Phy2MacEndRx(Packet *p)                
{
	transceiver_status = IDLE;
	
	if(debug_){
		std::cout << NOW << " AlohaQ Node -  " << addr << 
			" Rx_ received ----- Phy2MacEndRx" << std::endl;
	}	
	
	if (transceiver_status != TRANSMITTING) {
		hdr_cmn *ch = HDR_CMN(p);
		packet_t rx_pkt_type = ch->ptype();
		hdr_mac *mach = HDR_MAC(p);
		int dest_mac = mach->macDA();
		int src_mac = mach->macSA();
		
		if (ch->error()) {
			if (debug_)
				cout << NOW << " ALOHAQ(" << addr
					 	<< ")::Phy2MacEndRx() dropping corrupted pkt from node = "
					 	<< src_mac << std::endl;

			incrErrorPktsRx();
			drop(p, 1, "CORRUPTED_PKT");
		} else {
			if (dest_mac != addr && dest_mac != 0) { //0 is considered broadcast adress
				rxPacketNotForMe(p);

				if (debug_)
					std::cout << NOW << " ID " << addr << ": Packet is not for me, packet is for "
							  << dest_mac << std::endl;
			} else if (rx_pkt_type == PT_ALOHAQ_SYNC_ACK && dest_mac == 0) {
				aloha_q_sync_ACK *ack = ALOHAQ_SYNC_ACK_HDR_ACCESS(p);
				ack_data = ack -> get_succ_macs(); 
				stateRxAck(p, addr);

				if (debug_)
					std::cout << NOW << " ID " << addr
							<< ": Received ACK packet from " << src_mac
							<< std::endl;
			} else { 
				std::cout << NOW << " AlohaQ Node - " << addr
						<< " : Received NOT ACK " << src_mac << "   " << rx_pkt_type 
						<< std::endl;
				drop(p, 1, "RECEIVED_NOT_ACK");
			}
		}
	} else if (debug_){
			std::cout << NOW << " ID " << addr
					<< ": Received packet while transmitting, dropping " 
					<< std::endl;	
			drop(p, 1, "TRANSCEIVER NOT IDLE");	
	}

	
}

void UwAloha_Q_Sync_NODE::stateRxAck(Packet *p, int addr)
{
	transceiver_status = IDLE;
	if (debug_) {
		std::cout << NOW << " ID " << addr
				<< ": Received ACK in reception interval " << std::endl;
	}		
	/////PRINT ACK DATA
	if (debug_) {
		for (int i = 0; i < ack_data.size(); i++){
	    		std::cout << ack_data[i] << endl;
		}
	}
		
	if (std::find(ack_data.begin(), ack_data.end(), addr)!=ack_data.end()) { 
			
		incrDataPktsRx();
			     
		updateQ_table(1);
		out_file_stats << "1" << std::endl; //used for calculating convergence time			
	} else {	
		updateQ_table(-1);
		out_file_stats << "0" << std::endl; //used for calculating convergence time	
	}		
}

void
UwAloha_Q_Sync_NODE::initPkt(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	ch->size() += HDR_size;
}

void
UwAloha_Q_Sync_NODE::rxPacketNotForMe(Packet *p)
{
	if (debug_)
		std::cout << NOW << "Node ID" << addr << "Packet not for me "
				<< std::endl;
	if (p != NULL)
		drop(p, 1, "PACKET_NOT_FOR_ME");
}

void
UwAloha_Q_Sync_NODE::handleTimerExpiration()
{
	if (curr_slot == 0) {			
		slot_status = TRANSMIT;
		transceiver_status = IDLE;
		packet_sent_curr_frame = 0;
		
		findMySlot();		
	}
	
	if (my_curr_slot == curr_slot && my_curr_subslot == curr_subslot) {
		txData();
		if (debug_)
			std::cout << NOW << " ON node id " << addr << " "
					<< std::endl;
	} else {
		if (debug_)
			std::cout << NOW << " ALOHAQ NODE STATUS -  " << addr << " : Not my slot!"
					<< std::endl;	
		
	}
	if (debug_) {
		std::cout << NOW << " NODE ID " << addr << 
				" current slot , my slot to send is -- current subslot - my subslot to send is"
				<< curr_slot << " - " << my_curr_slot << "---------" << curr_subslot << "-" 
				<< my_curr_subslot << std::endl;
	}
	if ( curr_slot == nn - 1  && curr_subslot == subslot_num - 1) {
		slot_status = RECEIVE; 
		
		curr_slot = 0;
		curr_subslot = 0;		
		alohaq_sync_timer.resched(2 * t_prop_max + t_dp + nn/5*t_guard + (slot_duration_factor * t_dp / subslot_num));
			
	} else if (curr_subslot == subslot_num - 1){
		curr_subslot = 0;
		curr_slot = curr_slot + 1;
		alohaq_sync_timer.resched(slot_duration_factor * t_dp / subslot_num);
		
	} else {
	
		alohaq_sync_timer.resched(slot_duration_factor * t_dp / subslot_num);
		curr_subslot = curr_subslot + 1;	
	}	
}

void
UwAloha_Q_Sync_NODE::start(double delay)
{

	enable = true;
	
	vector<vector<double>> Q_table_template(nn, vector<double> (subslot_num, 0));
	Q_table = Q_table_template;
	
	alohaq_sync_timer.sched(delay);
}

void
UwAloha_Q_Sync_NODE::stop()
{
	enable = false;
	alohaq_sync_timer.cancel();

}

int
UwAloha_Q_Sync_NODE::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {			
			start(start_time);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "stop") == 0) {
			stop();
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
			try {
				start_time = std::stof(argv[2]);
				
				if (start_time < 0){
					std::cerr << "Error: negative number" << std::endl;
					return 1;
				}
				
			} catch (const std::invalid_argument&){
				std::cerr << "Error: invalid number" << std::endl;
				return 1;
			} catch (const std::out_of_range&){
				std::cerr << "Error: number out of range" << std::endl;
				return 1;
			}
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setMacAddr") == 0) {
			try {
				addr = std::stoi(argv[2]);
				
				if (addr < 0){
					std::cerr << "Error: negative number" << std::endl;
					return 1;
				}
				
			} catch (const std::invalid_argument&){
				std::cerr << "Error: invalid number" << std::endl;
				return 1;
			} catch (const std::out_of_range&){
				std::cerr << "Error: number out of range" << std::endl;
				return 1;
			}
			if (debug_)
				cout << "MAC address of current node is " << addr
					 << std::endl;
			return TCL_OK;
		} 
	}
	return MMac::command(argc, argv);
}


int
UwAloha_Q_Sync_NODE::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_UWMMAC_ENABLE) {
		ClMsgUwMmacEnable *command = dynamic_cast<ClMsgUwMmacEnable *>(m);
		if (command->getReqType() == ClMsgUwMmac::SET_REQ) {
			(command->getEnable()) ? start(NOW + start_time) : stop();
			command->setReqType(ClMsgUwMmac::SET_REPLY);
			return 0;
		}
	}
	return MMac::recvSyncClMsg(m);
}
