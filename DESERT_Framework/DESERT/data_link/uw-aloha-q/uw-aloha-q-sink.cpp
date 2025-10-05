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
 * @file   uw-aloha-q-sink.cpp
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * \brief Provides the implementation of UW_ALOHAQ_SINK class
 *
 */


#include "uw-aloha-q-sink.h"
#include <iostream>
#include <stdint.h>
#include <clmsg-discovery.h>
#include <mac.h>
#include <uwmmac-clmsg.h>
#include <uwcbr-module.h>


/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwAloha_Q_SINKModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	UwAloha_Q_SINKModuleClass()
		: TclClass("Module/UW/ALOHAQ_SINK")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwAloha_Q_SINK());
	}

} class__module_uw_aloha_q_sink;


UwAloha_Q_SINK::UwAloha_Q_SINK()
	: MMac()
	, sink_status(IDLE)
	, packet_sent_curr_slot_(0)
	, ack_phy_id(0)
	
{
	bind("debug_", (int *) &debug_);
	bind("sea_trial_", (int *) &sea_trial_);
	bind("HDR_size_", (int *) &HDR_size);
	bind("ACK_size_", (int *) &ACK_size);
	bind("mac2phy_delay_", (double *) &mac2phy_delay_);

	if (mac2phy_delay_ <= 0) {
		cerr << NOW << " UwALOHAQ() not valid mac2phy_delay_ < 0!! set to 1e-9 by default "
			 	<< std::endl; 
		mac2phy_delay_ = 1e-9;
	}
}

UwAloha_Q_SINK::~UwAloha_Q_SINK()
{
}

void UwAloha_Q_SINK::recvFromUpperLayers(Packet *p)
{
	//SINK MODE - No receiving from upper layers	
	drop(p, 1, "SINK - NO_RECEVIVING_FROM_UPPER_LAYERS");	
}

int 
UwAloha_Q_SINK::getLayerIdFromTag(const std::string& tag) 
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
	return (-1); // or whatever to say that it’s not found
}

void 
UwAloha_Q_SINK::txAck(int dest_addr)
{
	
	Packet *ack_pkt = Packet::alloc();
	initPkt(ack_pkt, dest_addr);
	
	ack_phy_id = getLayerIdFromTag(phy_ack_tag);
	Mac2PhyTurnOn(ack_phy_id);	
	Mac2PhyStartTx(ack_phy_id, ack_pkt);
	
	std::cout << NOW << " SINK ( " << addr << 
			") :  SINK Sending ack packet via CHANNEL - " << ack_phy_id << std::endl;
	sink_status = TRANSMITTING;

}

void
UwAloha_Q_SINK::Phy2MacEndTx(const Packet *p)
{
	
	packet_sent_curr_slot_++;
	
	std::cout << NOW << " SINK ( " << addr << ") : Finished sending ACK" 
			<< std::endl;
	incrDataPktsTx();
	
	sink_status = IDLE;
}

void
UwAloha_Q_SINK::Phy2MacStartRx(const Packet *p)
{
		
	std::cout << NOW << " SINK " << addr << 
			" : Started receiving data packet, sink status is :: " << sink_status 
			<< std::endl;
	
	if (sink_status == IDLE) {
		sink_status = RECEIVING;
	} else if (sink_status == RECEIVING) {
		std::cout << NOW << " SINK " << addr << 
				" : Started receiving data packet, but already receiving :: " 
				<< sink_status << std::endl;	
	}		
}

void
UwAloha_Q_SINK::Phy2MacEndRx(Packet *p)
{

		hdr_cmn *ch = HDR_CMN(p);
		packet_t rx_pkt_type = ch->ptype();
		hdr_mac *mach = HDR_MAC(p);
		int dest_mac = mach->macDA();
		int src_mac = mach->macSA();
		

		if (ch->error()) {
			if (debug_)
				cout << NOW << " TDMA(" << addr
						<< ")::Phy2MacEndRx() dropping corrupted pkt from node = "
						<< src_mac << std::endl;

			incrErrorPktsRx();
			drop(p, 1, "CORRUPTED_PKT");
		} else {
			if (dest_mac != addr) {
				drop(p, 1, "PACKET_NOT_FOR_ME");

				if (debug_)
					std::cout << NOW << " SINK " << addr << ": Packet not for me, packet is for "
							<< dest_mac << std::endl;
			} else if (rx_pkt_type == PT_MMAC_ACK) {	
				std::cout << NOW << "? SINK Received ACK packet ?" << src_mac
						<< std::endl;
				drop(p, 1, "RECEIVED_ACK_PKT");
			}
			else {
				//sendUp(p);
				incrDataPktsRx();
				txAck(src_mac);

				if (debug_)
					std::cout << NOW << " ID " << addr
							<< ": Received data packet from " << src_mac
							<< std::endl;
			}
		}
		
	sink_status = IDLE;
}


void
UwAloha_Q_SINK::initPkt(Packet *p, int dest_addr)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);

	
	ch->ptype() = PT_MMAC_ACK;
	ch->size() = ACK_size;
	mach->macSA() = addr;
	mach->macDA() = dest_addr;
	
}

int
UwAloha_Q_SINK::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	
	if (argc == 2) {
		if (strcasecmp(argv[1], "get_upper_data_pkts_rx") == 0) {
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
		if (strcasecmp(argv[2], "setMacAddr") == 0) {
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
		else if (strcasecmp(argv[1], "setPhyAckTag") == 0) {
			phy_ack_tag = argv[2];
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}

int
UwAloha_Q_SINK::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_UWMMAC_ENABLE) {
		ClMsgUwMmacEnable *command = dynamic_cast<ClMsgUwMmacEnable *>(m);
	}
	return MMac::recvSyncClMsg(m);
}
