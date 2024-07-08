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
 * @file   uwtwr_NODE.cpp
 * @author Yuehan Jiang
 * @version 1.0.0
 *
 * @brief Provides the implementation of UWTWR_NODE class
 */

#include "uwtwr_NODE.h"
#include "uwtwr_cmn_hdr.h"
#include "mac.h"
#include "mmac.h"

#include "uwcbr-module.h"
#include "rng.h"

#include <sstream>
#include <sys/time.h>
#include <iomanip>

static class UWTWR_NODE_Class : public TclClass
{
public:
	UWTWR_NODE_Class()
		: TclClass("Module/UW/TWR/NODE")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UWTWR_NODE());
	}
} class_uwtwr_node;

bool UWTWR_NODE::initialized = false;
std::map<UWTWR_NODE::UWTWR_NODE_STATUS, std::string>
		UWTWR_NODE::status_info;
std::map<UWTWR_NODE::UWTWR_NODE_PKT_TYPE, std::string>
		UWTWR_NODE::pkt_type_info;

UWTWR_NODE::UWTWR_NODE()
	: node_id(0)
	, RxPollEnabled(true)
	, T_backoff(0.1)
	, AUV_mac_addr(0)
	, n_ack_sent(0)
	, n_times_polled(0)
	, ACK_uid(0)
	, curr_poll_pkt(0)
	, curr_ack_pkt(0)
	, backoff_timer(this)
	, n_poll_dropped(0)
	, curr_state(UWTWR_NODE_STATUS_IDLE)
	, prev_state(UWTWR_NODE_STATUS_IDLE)

{
	bind("node_id_", (uint *) &node_id);
	bind("T_backoff_", (double *) &T_backoff);
	bind("ACK_size_", (int *) &ACK_size);
}

UWTWR_NODE::~UWTWR_NODE()
{
}

int UWTWR_NODE::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	// look at uwpolling_NODE.cpp
	if (argc == 2) {
		if (strcasecmp(argv[1], "initialize") == 0) {
			if (!initialized)
				initInfo();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getAckSent") == 0) {
			tcl.resultf("%d", getAckSent());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getTimesPolled") == 0) {
			tcl.resultf("%d", getTimesPolled());
			return TCL_OK;
		}else if (strcasecmp(argv[1], "getPollDropped") == 0) {
			tcl.resultf("%d", getPollDropped());
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

int UWTWR_NODE::crLayCommand(ClMessage *m)
{
	switch (m->type()) {
		default:
			return MMac::crLayCommand(m);
	}
}

void UWTWR_NODE::initInfo()
{
	initialized = true;

	status_info[UWTWR_NODE_STATUS_IDLE] = "Idle state";
	status_info[UWTWR_NODE_STATUS_RX_POLL] = "Receiving a POLL Packet";
	status_info[UWTWR_NODE_STATUS_TX_ACK] = "Transmitting ACK Packet";

	pkt_type_info[UWTWR_POLL_PKT] = "Poll packet";
	pkt_type_info[UWTWR_ACK_PKT] = "Ack packet";
}

// // To be implemented
// void UWTWR_NODE::recvFromUpperLayers(Packet *p)
// {
// }

void UWTWR_NODE::BackOffTimer::expire(Event *e)
{
	if (module->debug_)
		std::cout << module->ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << module->addr
					<< ")::BACKOFF_TIMER::EXPIRED" << std::endl;
	timer_status = UWTWR_NODE_EXPIRED;
	if (module->polled) {
		module->BackOffTimerExpired();
	}
}

void UWTWR_NODE::BackOffTimerExpired()
{
	if (polled) {
		stateTxAck();
	} else {
		if (debug_) {
			std::cout << std::setprecision(10) << NOW << "UWTWR_NODE(" << addr
					<< ") Backoff timer expired but node not polled" 
					<< std::endl;
		}
	}
}

void UWTWR_NODE::Mac2PhyStartTx(Packet *p)
{
	if (debug_)
		std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
					<< ")::MAC2PHYSTARTTX::ACK_PACKET" << std::endl;
	MMac::Mac2PhyStartTx(p);
}

void UWTWR_NODE::Phy2MacEndTx(const Packet *p)
{
	if (debug_)
		std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
					<< ")::MAC2PHYENDTX::ACK_PACKET" << std::endl;
	stateIdle();
}

void UWTWR_NODE::Phy2MacStartRx(const Packet *p)
{
	hdr_cmn *cmh = hdr_cmn::access(p);
	if (cmh->ptype() == PT_POLL) {
		if (debug_)
			std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
					  << ")::PHY2MACSTARTRX::POLL_PACKET" << std::endl;
	}
}

void UWTWR_NODE::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();

	if (ch->error()) {
		if (ch->ptype() == PT_POLL) {
			if (debug_)
			    std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
						  << ")::PHY2MACENDRX::DROP_POLL" << std::endl;
		incrPollDropped();
		}
		drop(p, 1, UWTWR_NODE_DROP_REASON_ERROR);
	} else if ((dest_mac == addr) || (dest_mac == (int) MAC_BROADCAST)) {
		if (ch->ptype() == PT_POLL) {
			hdr_POLL *pollh = HDR_POLL(p);
			if (debug_)
			    std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
						  << ")::PHY2MACENDRX::RX_POLL::POLLED_NODE= "
						  << pollh->id_ << std::endl;
			curr_poll_pkt = p->copy();
			Packet::free(p);
			stateRxPoll();
		} else {
			drop(p, 1, UWTWR_NODE_DROP_REASON_UNKNOWN_TYPE);
		}
	} else {
		incrXCtrlPktsRx();
		if (ch->ptype() == PT_POLL) {
			if (debug_)
			    std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
						  << ")::PHY2MACENDRX::WARNING!!POLL packet not in "
							 "broadcast!!"
						  << std::endl;
		}
		drop(p, 1, UWTWR_NODE_DROP_REASON_WRONG_RECEIVER);
	}
}

void UWTWR_NODE::initPkt(UWTWR_NODE_PKT_TYPE pkt_type)
{
	if(pkt_type == UWTWR_ACK_PKT) {
		Packet *p = Packet::alloc();
		hdr_ACK_NODE *ackh = HDR_ACK_NODE(p);
		hdr_cmn *ch = hdr_cmn::access(p);
		hdr_mac *mach = HDR_MAC(p);
		ch->ptype() = PT_ACK_NODE;
		mach->ftype() = MF_CONTROL;
		mach->macDA() = AUV_mac_addr;
		mach->macSA() = addr;
		// set size of ACK pkt (10B)
		ch->size() = ACK_size;
		// ch->size() = sizeof(hdr_ACK_NODE);
		ackh->id_node_ = node_id;
		// save the ACK_uid to ACK pkt
		ackh->ACK_uid_ = ACK_uid;
		curr_ack_pkt = p->copy();
		Packet::free(p);
	}
}

void UWTWR_NODE::stateTxAck()
{
	if (polled) {
		refreshState(UWTWR_NODE_STATUS_TX_ACK);
		initPkt(UWTWR_ACK_PKT);
		hdr_ACK_NODE *ackh = HDR_ACK_NODE(curr_ack_pkt);
		// ACK_uid++;
		// ackh->ACK_uid_ = ACK_uid;
		if (debug_)
			    std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
						  << ")::STATE_TX_ACK_ID_" << ACK_uid
						  << std::endl;
		TxAck();
	}
}

void UWTWR_NODE::TxAck()
{
	incrCtrlPktsTx();
	incrAckSent();
	Mac2PhyStartTx(curr_ack_pkt);
}


void UWTWR_NODE::stateRxPoll()
{
	if (RxPollEnabled) {
		refreshState(UWTWR_NODE_STATUS_RX_POLL);

		hdr_POLL *pollh = HDR_POLL(curr_poll_pkt);
		hdr_mac *mach = HDR_MAC(curr_poll_pkt);
		AUV_mac_addr = mach->macSA();
		// Save the RX POLL id as the corresponding ACK id
		ACK_uid = pollh->POLL_uid_;
		if (debug_)
		    std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
					  << ")::STATE_RX_POLL::Node_POLLED = " << pollh->id_
					  << " rx from MAC=" << AUV_mac_addr <<std::endl;
		
		int polled_node = pollh->id_;
		if (node_id == (uint) polled_node) {
			// correct node is polled
			polled = true;

			Packet::free(curr_poll_pkt);
			incrTimesPolled();
			// schedule backoff time
			if (debug_)
				std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr
						  << ")::scheduling_BACKOFF_TIMER_T= " << T_backoff << std::endl;
			backoff_timer.schedule(T_backoff);
			RxPollEnabled = false;
			// stateTxAck();
		} else {
			drop(curr_poll_pkt, 1, UWTWR_NODE_DROP_REASON_NOT_POLLED);
		}
	} else {
		if (debug_)
		    std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UTWR_NODE(" << addr
					  << ")::STATE_RX_POLL::NODE_NOT_POLLED" << std::endl;
		drop(curr_poll_pkt, 1, UWTWR_NODE_DROP_REASON_WRONG_STATE);
	}
}

void UWTWR_NODE::stateIdle()
{
	if (debug_)
	    std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_NODE(" << addr << ")::IDLE_STATE"
				  << std::endl;
	refreshState(UWTWR_NODE_STATUS_IDLE);
	polled = false;
	RxPollEnabled = true;
	backoff_timer.force_cancel();
}

void UWTWR_NODE::waitForUser()
{
	std::string response;
	std::cout << "Press Enter to continue";
	std::getline(std::cin, response);
}