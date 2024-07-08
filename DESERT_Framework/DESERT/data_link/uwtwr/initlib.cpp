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
 * @file   uwtwr/initlib.cpp
 * @author Yuehan Jiang
 * @version 1.0.0
 *
 * @brief Provide the initialization of the uwtwr libraries
 */

#include <tclcl.h>
#include "uwtwr_AAUV.h"
#include "uwtwr_PAUV.h"
#include "uwtwr_NODE.h"
#include "uwtwr_cmn_hdr.h"
#include "sap.h"
#include "packet.h"

int hdr_POLL::offset_ = 0;
int hdr_ACK_NODE::offset_ = 0;

packet_t PT_POLL;
packet_t PT_ACK_NODE;

/**
 * Class that describe the Header of POLL Packet
 */
static class PollHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	PollHeaderClass()
		: PacketHeaderClass("PacketHeader/POLL", sizeof(hdr_POLL))
	{
		this->bind();
		bind_offset(&hdr_POLL::offset_);
	}
} class_hdr_POLL;

/**
 * Class that describe the Header of ACK sent by the node
 */
static class AckNodeHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	AckNodeHeaderClass()
		: PacketHeaderClass("PacketHeader/ACK_NODE", sizeof(hdr_ACK_NODE))
	{
		this->bind();
		bind_offset(&hdr_ACK_NODE::offset_);
	}
} class_hdr_ACK_NODE;

extern EmbeddedTcl uwtwr_default;

extern "C" int
Uwtwr_Init()
{ 
	PT_POLL = p_info::addPacket("UWTWR/POLL");
	PT_ACK_NODE = p_info::addPacket("UWTWR/ACK_NODE");
	uwtwr_default.load();
	return 0;
}