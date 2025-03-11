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
 * @file   uwtwr_PAUV.cpp
 * @author Yuehan Jiang
 * @version 1.0.0
 *
 * @brief Provides the implementation of UWTWR_PAUV class
 */

#include "uwtwr_PAUV.h"
#include "uwtwr_cmn_hdr.h"
#include "mac.h"
#include "mmac.h"

#include "uwcbr-module.h"
#include "rng.h"

#include <algorithm>
#include <sstream>
#include <sys/time.h>
#include <iomanip>

static class UWTWR_PAUV_Class : public TclClass
{
public:
	UWTWR_PAUV_Class()
		: TclClass("Module/UW/TWR/PAUV")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UWTWR_PAUV());
	}
} class_uwtwr_pauv;

bool UWTWR_PAUV::initialized = false;
std::map<UWTWR_PAUV::UWTWR_PAUV_STATUS, std::string> UWTWR_PAUV::status_info;

UWTWR_PAUV::UWTWR_PAUV()
	: curr_state(UWTWR_PAUV_STATUS_IDLE)
	, prev_state(UWTWR_PAUV_STATUS_IDLE)
	, n_poll_rx(0)
	, n_ack_rx(0)
	, n_poll_dropped(0)
	, n_ack_dropped(0)
{}

UWTWR_PAUV::~UWTWR_PAUV()
{
}

int UWTWR_PAUV::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "initialize") == 0) {
			if (!initialized)
				initInfo();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getPollDropped") == 0) {
			tcl.resultf("%d", getPollDropped());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getAckDropped") == 0) {
			tcl.resultf("%d", getDroppedAckPkts());
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

int UWTWR_PAUV::crLayCommand(ClMessage *m)
{
	switch (m->type()) {
		default:
			return MMac::crLayCommand(m);
	}
}

void UWTWR_PAUV::initInfo()
{
	initialized = true;
	status_info[UWTWR_PAUV_STATUS_IDLE] = "Idle state";
	status_info[UWTWR_PAUV_STATUS_RX_POLL] = "Receiving a POLL Packet";
	status_info[UWTWR_PAUV_STATUS_RX_ACK] = "Receiving ACK Packet";
}

void UWTWR_PAUV::Phy2MacStartRx(const Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	if (ch->ptype() == PT_POLL) {
		hdr_POLL *pollh = HDR_POLL(p);
		if (debug_) {
			std::cout << pollh->POLL_uid_ << "::" << std::setprecision(10) << NOW << "::UWTWR_PAUV(" << addr
						<< ")::PHY2MACSTARTRX::RX_POLL::POLLED_NODE::"
						<< pollh->id_ << "::LAST_TOF::" << pollh->tof_ 
						<< "::AAUV_POS_X::" << NOW << "::AAUV_POS_Y::" << NOW <<std::endl; // post-processing using NOW to calculate AAUV position
		}
	} else {
		hdr_ACK_NODE *ackh = HDR_ACK_NODE(p);
		ACK_uid = ackh->ACK_uid_;
		if (debug_) {
			std::cout << ACK_uid << "::" << std::setprecision(10) << NOW << "::UWTWR_PAUV(" << addr
						<< ")::PHY2MACSTARTRX::RX_ACK::ACK_NODE::"
						<< ackh->id_node() << "::NONE::" << 0
						<< "::NONE::" << 0 << "::NONE::" << 0 << std::endl;
		}
	}
}

void UWTWR_PAUV::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();
	if (ch->error()) {
		if (ch->ptype() == PT_POLL) {
			if (debug_)
				std::cout << getEpoch() << "::" << std::setprecision(10) << NOW << "::UWTWR_PAUV(" << addr
						<< ")::PHY2MACENDRX_DROP_POLL" << std::endl;
			incrPollDropped();
		} else {
			if (debug_)
				std::cout << getEpoch() << "::" << std::setprecision(10) << NOW << "::UWTWR_PAUV(" << addr
						<< ")::PHY2MACENDRX_DROP_ACK" << std::endl;
			incrDroppedAckPkts();
		}
	} else {
		if (ch->ptype() == PT_POLL) {
			hdr_POLL *pollh = HDR_POLL(p);
			// if (debug_) {
			// 	std::cout << getEpoch() << "::" << std::setprecision(10) << NOW << "::UWTWR_PAUV(" << addr
			// 				<< ")::PHY2MACENDRX::RX_POLL::POLLED_NODE= "
			// 				<< pollh->id_ << std::endl;
			// }
			incrPollRx();
			stateRxPoll();
		} else {
			hdr_ACK_NODE *ackh = HDR_ACK_NODE(p);
			// if (debug_) {
			// 	std::cout << getEpoch() << "::" << std::setprecision(10) << NOW << "::UWTWR_PAUV(" << addr
			// 				<< ")::PHY2MACENDRX::RX_ACK::ACK_NODE= "
			// 				<< ackh->id_node() << std::endl;
			// }
			incrAckRx();
			stateRxAck();
		}
	}
}

void UWTWR_PAUV::stateRxPoll()
{
	refreshState(UWTWR_PAUV_STATUS_RX_POLL);
	stateIdle();
}

void UWTWR_PAUV::stateRxAck()
{
	refreshState(UWTWR_PAUV_STATUS_RX_ACK);
	stateIdle();
}

void UWTWR_PAUV::stateIdle()
{
	// if (debug_)
	// 	std::cout << getEpoch() << "::" << std::setprecision(10) << NOW << "::UWTWR_PAUV(" << addr << ")::IDLE_STATE"
	// 			  << std::endl;
	refreshState(UWTWR_PAUV_STATUS_IDLE);
}