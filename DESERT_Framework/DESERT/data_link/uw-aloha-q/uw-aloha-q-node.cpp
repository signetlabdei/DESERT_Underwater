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
 * @file   uw-aloha-q-node.cpp
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * \brief Provides the implementation of UW_ALOHAQ_NODE class
 *
 */


#include "uw-aloha-q-node.h"
#include <iostream>
#include <stdint.h>
#include <clmsg-discovery.h>
#include <mac.h>
#include <rng.h>
#include <uwmmac-clmsg.h>
#include <uwcbr-module.h>
#include <vector>
#include <algorithm>

/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwAloha_Q_NODEModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	UwAloha_Q_NODEModuleClass()
		: TclClass("Module/UW/ALOHAQ_NODE")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwAloha_Q_NODE());
	}

} class_module_uw_aloha_q_node;

void
UwAlohaQTimer::expire(Event *e)
{
	((UwAloha_Q_NODE*) module)->handleTimerExpiration();
}

UwAloha_Q_NODE::UwAloha_Q_NODE()
	: MMac()
	, alohaq_timer(this)
	, slot_status(UW_ALOHAQ_STATUS_NOT_MY_SLOT)
	, transceiver_status(IDLE)
	, ack_status(ACK_NOT_RECEIVED)
	, backoff_status(HALT)
	, HDR_size(0)
	, max_packet_per_slot(1)
	, packet_sent_curr_slot_(0)
	, packet_sent_curr_frame(0)
	, enable(true)
	, curr_slot(0)
	, my_curr_slot(1)
	, data_phy_id(0)
	, decide_backoff(0)
	
{
	bind("max_queue_size_", (int *) &max_queue_size);
	bind("debug_", (int *) &debug_);
	bind("sea_trial_", (int *) &sea_trial_);
	bind("HDR_size_", (int *) &HDR_size);
	bind("max_packet_per_slot", (int *) &max_packet_per_slot);
	bind("slot_duration", (double *) &slot_duration);
	bind("start_time", (double *) &start_time);
	bind("mac2phy_delay_", (double *) &mac2phy_delay_);
	bind("guard_time", (double *) &guard_time);
	bind("tot_slots", (int *) &tot_slots);
	bind("backoff_mode", (int *) &backoff_mode);


	if (max_queue_size < 0) {
		std::cerr << NOW << " UwALOHAQ() not valid max_queue_size < 0!! set to 1 by default " 
			 << std::endl;
		max_queue_size = 1;
	}
	if (max_packet_per_slot < 0) {
		std::cerr << NOW << " UwALOHAQ() not valid max_packet_per_slot < 0!! set to 1 by default " 
			 << std::endl;
		max_packet_per_slot = 1;
	}
	if (slot_duration < 0) {
		std::cerr << NOW << " UwALOHAQ() not valid slot_duration < 0!! set to 0.1 by default " 
			 << std::endl;
		slot_duration = 0.1;
	}
	if (mac2phy_delay_ <= 0) {
		std::cerr << NOW << " UwALOHAQ() not valid mac2phy_delay_ < 0!! set to 1e-9 by default "
			 << std::endl; 
		mac2phy_delay_ = 1e-9;
	}
	if (guard_time <= 0) {
		std::cerr << NOW << " UwALOHAQ() not valid guard_time < 0!! set to 0 by default "
			 << std::endl; 
		guard_time = 0;
	}
	if (tot_slots <= 0) {
		std::cerr << NOW << " UwALOHAQ() not valid tot_slots < 0!! set to 1 by default "
			 << std::endl; 
		tot_slots = 1;
	}
	
}


int 
UwAloha_Q_NODE::getLayerIdFromTag(const std::string& tag) 
{
	ClMsgDiscovery m;
	m.addSenderData((const PlugIn*) this, getLayer(), getId(),
	getStackId(), name() , getTag());
	sendSyncClMsgDown(&m);
	DiscoveryStorage low_layer_storage = m.findTag(tag.c_str());
	if (low_layer_storage.getSize() == 1) {
		DiscoveryData low_layer = (*low_layer_storage.begin()).second;
		return low_layer.getId();
	}
  	return (-1); // not found
}

int
UwAloha_Q_NODE::decide_if_backoff(int slot){
	
	double max = *std::max_element(Q_table.begin(), Q_table.end());
	
	if (Q_table [slot] == max) {
		return (0);
	} else {
		return (1);
	}
}

int 
UwAloha_Q_NODE::findMySlot()
{
	
	vector<int> max_arr;
	
	double max = *std::max_element(Q_table.begin(), Q_table.end());
	int i;
	
	for (i=0; i < tot_slots; i++) {
		if (max == Q_table[i]) { 
	    		max_arr.push_back(i);  
	    	}
	}
	int my_slot;
	
	if (max_arr.size() == 1) {
		my_slot = max_arr[0];
	} else {
		int pos = RNG::defaultrng()->uniform(0, max_arr.size());
		my_slot = max_arr[pos];
	}
	
	return my_slot;
}

void
UwAloha_Q_NODE::updateQ_table(int ack_received)
{	
	
	Q_table[curr_slot] = Q_table[curr_slot] + 0.1 * (ack_received  
			- Q_table[curr_slot]);
	
	if(debug_) {
		std::cout << NOW << " ALOHAQ node ID (" << addr 
				<< ")My Q table now is " << std::endl;	
		for (int i = 0; i < tot_slots; i++) {
	    		std::cout << Q_table[i] << endl;
		}	
	}	
}

void
UwAloha_Q_NODE::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	if (buffer.size() < max_queue_size) {
		initPkt(p);
		buffer.push_back(p);
 	} else {
		drop(p, 1, "BUFFER_OVERLOAD");
	}
}

void
UwAloha_Q_NODE::txData()
{
	if (packet_sent_curr_slot_ < max_packet_per_slot) {
		if (slot_status == UW_ALOHAQ_STATUS_MY_SLOT && transceiver_status == 
				IDLE && packet_sent_curr_frame == 0) {
			if (buffer.size() > 0) {
				Packet *p = buffer.front();
				buffer.pop_front();
				//sendDown(p);
				data_phy_id = getLayerIdFromTag(phy_data_tag);
				Mac2PhyTurnOn(data_phy_id);
				Mac2PhyStartTx(data_phy_id, p);
				
				transceiver_status = TRANSMITTING;
				if(debug_){
					std::cout << NOW << " ALOHAQ NODE -  " << addr 
							<< ": Sending packet through CHANNEL ID " << data_phy_id 
							<< " - now waiting ACK - Mac2PhyStarTx" << std::endl;
				}
				
				incrDataPktsTx();
				
				packet_sent_curr_frame++;
			}
		} else if (debug_) {
			if (slot_status != UW_ALOHAQ_STATUS_MY_SLOT) {
				std::cout << NOW << " ALOHAQ NODE STATUS -  " << addr << " : Not my slot!"
						<< std::endl;
			} else {
				std::cout << NOW << " Node ID " << addr
						<< ": Wait earlier packet expires to send the current one"
						<< std::endl;
			}
		}
	}
	else {
		if (debug_)
			cout << NOW << " ALOHAQ(" << addr
					<< ")::already_tx max packet = " << max_packet_per_slot << std::endl;
	}
}

void
UwAloha_Q_NODE::Phy2MacEndTx(const Packet *p)
{
	transceiver_status = WAIT_ACK;
	packet_sent_curr_slot_++;
	if (debug_) {
		std::cout << NOW << " ALOHAQ NODE -  " << addr << 
				" Packet transmitted ----- Phy2MacEndTx" << std::endl;
	}
}

void
UwAloha_Q_NODE::Phy2MacStartRx(const Packet *p)
{
	if(debug_) {
		std::cout << NOW << " ALOHAQ NODE -  " << addr << 
				" Started receiving packet" << std::endl;	
	}
	if (transceiver_status == WAIT_ACK)
		transceiver_status = RECEIVING;
}

void
UwAloha_Q_NODE::Phy2MacEndRx(Packet *p)
{
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
			if (dest_mac != addr) {
				rxPacketNotForMe(p);

				if (debug_)
					std::cout << NOW << " ID " << addr << ": PACKET WAS NOT FOR ME, packet was for "
							<< dest_mac << std::endl;
			} else if (rx_pkt_type == PT_MMAC_ACK) {
				stateRxAck(p);
				if (debug_ )
					std::cout << NOW << " ID " << addr
							<< ": Received ACK packet from " << src_mac
							<< std::endl;
			} else {
				if(debug_){
					std::cout << NOW << " ALOHAQ NODE - " << addr
							<< " : Received NOT ACK " << src_mac << "   " << rx_pkt_type 
							<< std::endl;
				}
			 	drop(p, 1, "RECEIVED_NOT_ACK");
                        }
		}

		transceiver_status = IDLE;

	} else {
		if (debug_)
			std::cout << NOW << " ID " << addr
					  << ": Received packet while transmitting, dropping " << std::endl;
		drop(p, 1, "TRANSCEIVER NOT IDLE");	
	}
}

void UwAloha_Q_NODE::stateRxAck(Packet *p)
{	
	if(slot_status == UW_ALOHAQ_STATUS_MY_SLOT) {
		incrDataPktsRx();
		if(debug_) {
			std::cout << NOW << " ID " << addr
					<< ": Received ACK in my slot " << std::endl;
		}
		ack_status = ACK_RECEIVED;
		updateQ_table(1);
	} else {
		if(debug_) {
			std::cout << NOW << " ID " << addr
					<< ": Received ACK in another slot " << std::endl;
		}
		drop(p, 1, "RECEIVED OUTSIDE MY SLOT");
	}
	
	transceiver_status = IDLE;
}

void
UwAloha_Q_NODE::initPkt(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);
	
	ch->size() += HDR_size;	
}

void
UwAloha_Q_NODE::rxPacketNotForMe(Packet *p)
{
	if(debug_) {
		std::cout << NOW << "Node ID: " << addr <<  "Packet not for me " << std::endl;
	}
	if (p != NULL)
		drop(p, 1, "PACKET_NOT_FOR_ME");
}

void
UwAloha_Q_NODE::handleTimerExpiration()
{	
	
	backoff_status = HALT;
	
	if (slot_status == UW_ALOHAQ_STATUS_MY_SLOT) {
		slot_status = UW_ALOHAQ_STATUS_NOT_MY_SLOT;
		if ( ack_status == ACK_NOT_RECEIVED) {
			updateQ_table(-1);
					
			decide_backoff = decide_if_backoff(curr_slot);
			if (backoff_mode == B2 && decide_backoff == 1) {
				backoff_status = ADD_BACKOFF;
			}
			if (backoff_mode == B1) {
				backoff_status = ADD_BACKOFF;
			}
		}		
		
		std::cout << NOW << " OFF node id " << addr << std::endl;
	}
	
	transceiver_status = IDLE;			  
	curr_slot++;
	
	//////////////////////////'BREAK' BETWEEN TWO SLOTS///////////////////////////
	
	if (curr_slot == tot_slots) {
		curr_slot = 0;
		packet_sent_curr_frame = 0;
		my_curr_slot = findMySlot();			  
	}
				  
	packet_sent_curr_slot_ = 0;
	
	if(debug_)
		std::cout << NOW << " NODE ID " << addr << " current slot is , my slot to send is "
					  << curr_slot << " - " << my_curr_slot
					  << std::endl;
		
	if (my_curr_slot == curr_slot){ 
		slot_status = UW_ALOHAQ_STATUS_MY_SLOT;
		txData();
		std::cout << NOW << " ON node id " << addr << std::endl;
	} else {
		std::cout << NOW << " still OFF node id " << addr << std::endl;
	}
	
	ack_status = ACK_NOT_RECEIVED;
	
	if (backoff_status == ADD_BACKOFF) {
		double rnd = RNG::defaultrng()->uniform_double();
		alohaq_timer.resched(slot_duration + rnd);
	} else {
		alohaq_timer.resched(slot_duration);
	}
}

void
UwAloha_Q_NODE::start(double delay)
{

	enable = true;

	vector <double> Q_table_template(tot_slots, 0);
	Q_table = Q_table_template;
	
	//alohaq_timer.sched(delay + slot_duration);
	alohaq_timer.sched(slot_duration);

	if (debug_)
		std::cout << NOW << " Status " << slot_status << " on ID " << addr
				<< std::endl;
}

void
UwAloha_Q_NODE::stop()
{
	enable = false;
	alohaq_timer.cancel();

}

int
UwAloha_Q_NODE::command(int argc, const char *const *argv)
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
			std::stringstream ss(argv[2]);
			double st;
			
			if (ss >> st){
				start_time = st;
			
				if (start_time < 0){
					tcl.resultf("Error: negative number");
					return TCL_ERROR;
				}
			
				return TCL_OK;
			}
		
			tcl.resultf("Error: invalid number");
			return TCL_ERROR;
		
		} else if (strcasecmp(argv[1], "setMacAddr") == 0) {
			std::stringstream ss(argv[2]);
			int ma;
			
			if (ss >> ma){
				addr = ma;
			
				if (addr < 0){
					tcl.resultf("Error: negative number");
					return TCL_ERROR;
				}
			
				return TCL_OK;
			}
		
			tcl.resultf("Error: invalid number");
			return TCL_ERROR;
		} 
		else if (strcasecmp(argv[1], "setPhyDataTag") == 0) {
			phy_data_tag = argv[2];
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}

int
UwAloha_Q_NODE::recvSyncClMsg(ClMessage *m)
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
