//
// Copyright (c) 2021 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwtap.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 */

#include "uwtap.h"
#include "phymac-clmsg.h"
#include "uwtap_clmsg.h"

extern ClMessage_t CLMSG_TAP_PKT;
/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwTapClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwTapClass()
		: TclClass("Module/UW/TAP")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwTap());
	}
} class_uwtap;

int
UwTap::recvSyncClMsg(ClMessage *m)
{
	// TAP is supposed to be transparent to other layers
	if (m->direction() == DOWN) {
		sendSyncClMsgDown(m);
		return 0;
	} else if (m->direction() == UP) {
		sendSyncClMsgUp(m);
		return 0;
	}

	return 0;
}

void
UwTap::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	if (ch->direction() == hdr_cmn::UP) {
		// ch->timestamp() = NOW;
		ClMsgTapPkt msg = ClMsgTapPkt(
				p, clmsg_tap_type::END_RX, NOW, Tap2PhyTxDuration(p));
		sendSyncClMsg(&msg);
		sendUp(p);
	} else if (ch->direction() == hdr_cmn::DOWN) {
		// ch->txtime() = Tap2PhyTxDuration(p);
		ClMsgTapPkt msg = ClMsgTapPkt(
				p, clmsg_tap_type::START_TX, NOW, Tap2PhyTxDuration(p));
		sendSyncClMsg(&msg);
		sendDown(p);
	}
}

double
UwTap::Tap2PhyTxDuration(Packet *pkt)
{
	ClMsgMac2PhyGetTxDuration m(pkt);
	sendSyncClMsgDown(&m);
	return (m.getDuration());
}
