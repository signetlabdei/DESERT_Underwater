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

/**
 * @file   ranging/uwranging_tokenbus/initlib.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * \brief Provides the initialization of the uwranging_tokenbus libraries
 *
 */

#include <tclcl.h>
#include <iostream>
#include "uwranging_tokenbus_hdr.h"

int hdr_uwranging_tokenbus::offset_ = 0;
packet_t PT_UWRANGING_TOKENBUS;


/**
 * Class that describe the Header of a token bus packet
 */
static class TokenBusRangingHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	TokenBusRangingHeaderClass()
		: PacketHeaderClass("PacketHeader/UWRANGING_TOKENBUS", sizeof(hdr_uwranging_tokenbus))
	{
		this->bind();
		bind_offset(&hdr_uwranging_tokenbus::offset_);
	}
} class_hdr_ranging_tokenbus;


extern EmbeddedTcl Uwranging_tokenbusTclCode;
extern "C" int
Uwranging_tokenbus_Init()
{
	PT_UWRANGING_TOKENBUS = p_info::addPacket("UWRANGING_TOKENBUS");
	Uwranging_tokenbusTclCode.load();
	return 0;
}
