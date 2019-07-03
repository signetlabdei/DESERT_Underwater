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
 * @file   network/uwPositionBasedRouting/initlib.cpp
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Provides the initialization of uwPositionBasedRouting libraries.
 *
 * Provides the initialization of uwPositionBasedRouting libraries.
 */
 
#include "uwPosBasedRt-hdr.h"

#include <tclcl.h>

int hdr_uwpos_based_rt::offset_ = 0;

extern EmbeddedTcl UwPosBasedRtInitTclCode;

packet_t PT_UWPOSBASEDRT;

/**
 * Adds the header for <i>hdr_uwpos_based_rt</i> packets in ns2.
 */
static class UwPosBasedRtPktClass : public PacketHeaderClass
{
public:
	UwPosBasedRtPktClass()
		: PacketHeaderClass("PacketHeader/PosBasedRt", sizeof(hdr_uwpos_based_rt))
	{
		this->bind();
		bind_offset(&hdr_uwpos_based_rt::offset_);
	}
} class_uwposbasedrt_pkt;

extern "C" int
Uwposbasedrt_Init()
{
	PT_UWPOSBASEDRT = p_info::addPacket("PosBasedRt");
	UwPosBasedRtInitTclCode.load();
	return 0;
}

extern "C" int
Cyguposbasedrt_Init()
{
	Uwposbasedrt_Init();
	return 0;
}