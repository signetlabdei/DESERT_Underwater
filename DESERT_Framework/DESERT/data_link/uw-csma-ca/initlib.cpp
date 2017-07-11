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
 * @file   data_link/uw-csma-ca/initlib.cc
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the initialization of uw-csma-aloha libraries
 *
 */

#include <tclcl.h>
#include "uw-csma-ca-hdrs.h"

extern EmbeddedTcl CsmaCaTclCode;

int hdr_ca_RTS::offset_ = 0;
int hdr_ca_CTS::offset_ = 0;

packet_t PT_CA_RTS;
packet_t PT_CA_CTS;
packet_t PT_CA_ACK;

static class Ca_RTS_HeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	Ca_RTS_HeaderClass()
		: PacketHeaderClass("PacketHeader/CA_RTS", sizeof(hdr_ca_RTS))
	{
		this->bind();
		bind_offset(&hdr_ca_RTS::offset_);
	}
} class_Ca_RTS_HeaderClass;

static class Ca_CTS_HeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	Ca_CTS_HeaderClass()
		: PacketHeaderClass("PacketHeader/CA_CTS", sizeof(hdr_ca_CTS))
	{
		this->bind();
		bind_offset(&hdr_ca_CTS::offset_);
	}
} class_Ca_CTS_HeaderClass;

extern "C" int
Uwcsmaca_Init()
{
	PT_CA_RTS = p_info::addPacket("UWCSMACA/RTS");
	PT_CA_CTS = p_info::addPacket("UWCSMACA/CTS");
	PT_CA_ACK = p_info::addPacket("UWCSMACA/ACK");
	CsmaCaTclCode.load();
	return 0;
}

extern "C" int
Cygcsmaca_Init()
{
	return Uwcsmaca_Init();
}
