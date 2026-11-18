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
 * @file   uwtwr_AAUV.cpp
 * @author Yuehan Jiang
 * @version 1.0.0
 *
 * @brief Provides the implementation of UWTWR_AAUV class
 */

#include <sstream>
#include "mac.h"
#include "uwcbr-module.h"
#include "uwtwr_AAUV.h"
#include "uwtwr_cmn_hdr.h"

#include <algorithm>
#include <float.h>
#include <fstream>
#include <sstream>
#include "uwphy-clmsg.h"
#include <iomanip>

static class UWTWR_AAUV_Class : public TclClass
{
public:
	UWTWR_AAUV_Class()
		: TclClass("Module/UW/TWR/AAUV")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UWTWR_AAUV());
	}
} class_uwtwr_aauv;

bool UWTWR_AAUV::initialized = false;

std::map<UWTWR_AAUV::UWTWR_AAUV_STATUS, std::string>
		UWTWR_AAUV::status_info;

UWTWR_AAUV::UWTWR_AAUV()
	: ack_timer(this)
	, poll_size(2) // Default to 2 nodes (configutable in TCL)
	, polling_rotation_index(0) // Start from first node
	, curr_poll_packet(0)
	, curr_ack_packet(0)
	, T_ack_timer(1) // send POLL every 1 second, need to be set in Tcl
	, curr_state(UWTWR_AAUV_STATUS_IDLE)
	, prev_state(UWTWR_AAUV_STATUS_IDLE) // not used
	, TxEnabled(false)
	, n_poll_tx(0)
	, n_ack_rx(0)
	, POLL_uid(0)
	, RxAckEnabled(false)
	, ack_enabled(1)
	, n_dropped_ack_pkts(0)
	, is_running(false)
{
	bind("T_ack_timer_", (double *) &T_ack_timer);
	bind("ack_enabled_", (int *) &ack_enabled);
	bind("POLL_size_", (int *) &POLL_size);
	bind("ACK_size_", (int *) &ACK_size);
	bind("poll_size_", (int *) &poll_size);
}

UWTWR_AAUV::~UWTWR_AAUV()
{
}

int UWTWR_AAUV::command(int argc, const char *const * argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "initialize") == 0) {
			// Initialize poll_node_ids with default sequence based on poll_size
			poll_node_ids.clear();
			for (int i = 0; i < poll_size; i++) {
				poll_node_ids.push_back(i); // Poll nodes 0, 1, ..., poll_size-1
			}
			if (!initialized)
				initInfo();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "run") == 0) {
			is_running = true;
			stateIdle();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "stop") == 0) {
			is_running = false;
			if (ack_timer.isActive()) {
				ack_timer.stop();
			}
			RxAckEnabled = false;
			TxEnabled = false;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getAckRx") == 0) {
			tcl.resultf("%d", getAckRx());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getDroppedAckPkts") == 0) {
			tcl.resultf("%d", getDroppedAckPkts());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getPollSent") == 0) {
			tcl.resultf("%d", getPollSent());
			return TCL_OK;
		}
	} 
	if (argc >= 3) {
		if (strcasecmp(argv[1], "setMacAddr") == 0) {
			addr = atoi(argv[2]);
			return TCL_OK;
		}
		// set custom polling sequence
		if (strcasecmp(argv[1], "setPollSequence") == 0) {
			poll_node_ids.clear();
			for (int i = 2; i < argc; i++) {
				poll_node_ids.push_back(atoi(argv[i]));
			}
			poll_size = poll_node_ids.size();
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}

int UWTWR_AAUV::crLayCommand(ClMessage *m)
{
	switch (m->type()) {
		default:
			return MMac::crLayCommand(m);
	}
}

void UWTWR_AAUV::ACKTimer::expire(Event *e)
{
	timer_status = UWTWR_AAUV_EXPIRED;
	module->AckTOExpired();
}

void UWTWR_AAUV::AckTOExpired()
{
	RxAckEnabled = false;
	// go back to IDLE and poll the other node only if still running
	if (is_running) {
		stateIdle();
	}
}

// Is it not needed if no data packets to send
// void UWTWR_AAUV::recvFromUpperLayers(Packet *p)
// {
// }

void UWTWR_AAUV::Phy2MacStartRx(const Packet *p)
{
	hdr_cmn *cmh = hdr_cmn::access(p);
	hdr_MPhy *ph = hdr_MPhy::access(p);
	double gen_time = ph->txtime;
	double received_time = ph->rxtime;
	diff_time = received_time - gen_time;
	if (cmh->ptype() == PT_ACK_NODE) {
		if (debug_)
			std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
					  << ")::RX_ACK_PACKET::TOF::" << diff_time << std::endl;
	}
}

void UWTWR_AAUV::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *cmh = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_MPhy *ph = hdr_MPhy::access(p);
	int dest_mac = mach->macDA();
	double gen_time = ph->txtime;
	double received_time = ph->rxtime;
	diff_time = received_time - gen_time;

	if (cmh->error()) {
		if (cmh->ptype_ == PT_ACK_NODE) {
			if (debug_)
				std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
						<< ")::PHY2MACENDRX_DROP_ACK" << std::endl;
			incrDroppedAckPkts();
		} else {
			if (debug_)
				std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
						<< ")::PHY2MACENDRX_DROP_POLL" << std::endl;
			incrErrorPktsRx();
		}
	} else {
		if ((dest_mac == addr) || (dest_mac == (int) MAC_BROADCAST)) {
			if (cmh->ptype() == PT_ACK_NODE && ack_enabled) {
				incrAckRx();
				curr_ack_packet = p->copy();
				Packet::free(p);
				stateRxAck();
				//check ACK value and discard acked packets
			} 
		} else {
			if (debug_)
				std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
						  << ")::PHY2MACENDRX::DROP_ACK_WRONG_DEST_"
						  << mach->macDA() << std::endl;
			incrXCtrlPktsRx();
			drop(p, 1, UWPOLLING_AUV_DROP_REASON_WRONG_RECEIVER);
		}
	}
}

void UWTWR_AAUV::Mac2PhyStartTx(Packet *p)
{
	hdr_cmn *cmh = hdr_cmn::access(p);
	if (cmh->ptype() == PT_POLL) {
		stateWaitAck();
	} else {
		stateWaitAck();
	}
	MMac::Mac2PhyStartTx(p);
}

void UWTWR_AAUV::Phy2MacEndTx(const Packet *p)
{
	// if (debug_)
	// 	std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
	// 			  << ")::PHYEndTx" << std::endl;
}

void UWTWR_AAUV::stateTxPoll()
{
	if(TxEnabled) {
		SetNodePoll();
		refreshState(UWTWR_AAUV_STATUS_TX_POLL);
		Packet *p = Packet::alloc();
		hdr_cmn *cmh = hdr_cmn::access(p);
		hdr_mac *mach = HDR_MAC(p);
		// hdr_POLL *pollh = HDR_POLL(p);
		cmh->ptype() = PT_POLL;

		//Set the size to POLL_size with correct bytes number
		cmh->size() = POLL_size;
		mach->set(MF_CONTROL, addr, MAC_BROADCAST);
		mach->macSA() = addr;
		mach->macDA() = MAC_BROADCAST;
		curr_poll_packet = p->copy();
		Packet::free(p);
		hdr_POLL *pollh = HDR_POLL(curr_poll_packet);
		POLL_uid++;
		pollh->POLL_uid_ = POLL_uid;
		pollh->id_ = curr_node_id;

		// Save info (tof) to be sent in POLL in header (easier for now)
		pollh->tof_ = diff_time;

		if (debug_)
			std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
						<< ")::STATE_TX_POLL::NODE::" << pollh->id_
						<< std::endl;
		TxPoll();
	} else {
		if (debug_)
			std::cerr << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
					  << ")---> going in stateTxPoll from WRONG STATE---> "
						"current_state: "
					  << status_info[curr_state] << std::endl;
	}
}

void UWTWR_AAUV::TxPoll()
{
	// if(debug_)
	// 	std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr << ")::TX_POLL"
	// 			  << std::endl;
	incrCtrlPktsTx();
	incrPollTx();
	Mac2PhyStartTx(curr_poll_packet);
}

void UWTWR_AAUV::SetNodePoll()
{
	// Get current node to poll from rotation list
	curr_node_id = poll_node_ids[polling_rotation_index];

	// Move to next node in rotation
	polling_rotation_index = (polling_rotation_index + 1) % poll_size;
}

// when to set the state to idle? TCl run--idle, waitAck--idle, RxAck--idle
void UWTWR_AAUV::stateIdle()
{
	refreshState(UWTWR_AAUV_STATUS_IDLE);
	// if (debug_)
	// 	std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr << ")::IDLE STATE "
	// 			  << std::endl;
	TxEnabled = true;
	// Only continue polling if is_running flag is true
	if (is_running) {
		stateTxPoll();
	}
}

void UWTWR_AAUV::initInfo()
{
	initialized = true;

	status_info[UWTWR_AAUV_STATUS_IDLE] = "Idle state";
	status_info[UWTWR_AAUV_STATUS_TX_POLL] = "Transmitting a POLL Packet";
	status_info[UWTWR_AAUV_STATUS_WAIT_ACK] = "Waiting for an ACK Packet";
	status_info[UWTWR_AAUV_STATUS_RX_ACK] = "Receiving ACK Packet";
}

void UWTWR_AAUV::stateWaitAck()
{
	refreshState(UWTWR_AAUV_STATUS_WAIT_ACK);
	TxEnabled = false;
	RxAckEnabled = true;
	// if (debug_)
	// 	std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr
	// 			  << ")::scheduling_ACK_TIMER_T= " << T_ack_timer << std::endl;
	ack_timer.schedule(T_ack_timer);
}

void UWTWR_AAUV::stateRxAck()
{
	if(RxAckEnabled) {
		// if (debug_)
		// 	std::cout << POLL_uid << "::" <<std::fixed<< std::setprecision(9) << NOW << "::UWTWR_AAUV(" << addr 
		// 			<< ")::stateRxAck()ACK received" << std::endl;
		refreshState(UWTWR_AAUV_STATUS_RX_ACK);
		RxAckEnabled = false;
		// handleAck();

		// maybe should let ack_timer expire
		// ack_timer.force_cancel();
		// stateIdle();
	}
	
}