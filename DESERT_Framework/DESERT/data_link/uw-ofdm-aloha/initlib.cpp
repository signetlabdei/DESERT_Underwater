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
 * @file   data_link/uw-ofdm-aloha/initlib.cc
 * @author Sara Falleni
 * @version 1.0.0
 *
 * @brief Provides the initializazion of the uw-ofdm-aloha libraries
 */

#include <tclcl.h>
#include "uwofdmphy_hdr.h"

extern EmbeddedTcl UWOFDMAlohaTclCode;

packet_t PT_OFDM;
int hdr_OFDM::offset_ = 0;

/**
 * Class that describe the Header of OFDM Packet
 */
static class OFDMALOHAHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	OFDMALOHAHeaderClass()
		: PacketHeaderClass("PacketHeader/OFDMALOHA", sizeof(hdr_OFDM))
	{
		this->bind();
	}
} class_hdr_OFDMALOHA;

extern "C" int
Uwofdmaloha_Init()
{
	UWOFDMAlohaTclCode.load();
	return 0;
}

extern "C" int
Cyguwofdmaloha_Init()
{
	return Uwofdmaloha_Init();
}
