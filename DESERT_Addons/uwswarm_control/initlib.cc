//
// Copyright (c) 2024 Regents of the SIGNET lab, University of Padova.
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
 * @file   initlib.cc
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 * \brief Provides the initialization of uwswarm_control libraries.
 *
 * Provides the initialization of uwswarm_control libraries.
 *
 */

#include "uwsc-clmsg.h"
#include "uwsc-tracker-follower-packet.h"
#include <tclcl.h>

extern EmbeddedTcl Uwswarm_controlTclCode;

int hdr_uwSCFTracker::offset_; /**< Offset used to access in
								  <i>hdr_uwTracker</i> packets header. */

packet_t PT_UWSCFTRACKER;

ClMessage_t CLMSG_MC2CTR_SETPOS;
ClMessage_t CLMSG_MC2CTR_SETSTATUS;
ClMessage_t CLMSG_CTR2MC_GETPOS;
ClMessage_t CLMSG_TRACK2MC_TRACKPOS;
ClMessage_t CLMSG_TRACK2MC_GETSTATUS;

/**
 * Adds the header for <i>hdr_uwTRACK</i> packets in ns2.
 */
static class UwSCFTrackPktClass : public PacketHeaderClass
{
public:
	UwSCFTrackPktClass()
		: PacketHeaderClass("PacketHeader/UWSCFTRACK", sizeof(hdr_uwSCFTracker))
	{
		this->bind();
		bind_offset(&hdr_uwSCFTracker::offset_);
	}
} class_uwSCFTRACK_pkt;

extern "C" int
Uwswarm_control_Init()
{
	PT_UWSCFTRACKER = p_info::addPacket("UWSCFTRACK");
	CLMSG_MC2CTR_SETPOS = ClMessage::addClMessage();
	CLMSG_MC2CTR_SETSTATUS = ClMessage::addClMessage();
	CLMSG_CTR2MC_GETPOS = ClMessage::addClMessage();
	CLMSG_TRACK2MC_TRACKPOS = ClMessage::addClMessage();
	CLMSG_TRACK2MC_GETSTATUS = ClMessage::addClMessage();
	Uwswarm_controlTclCode.load();
	return 0;
}
