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
 * @file   data_link/uwtokenbus/initlib.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * \brief Provides the initialization of the uwtokenbus libraries
 *
 */

#include <tclcl.h>
#include <iostream>
#include "uwtokenbus_hdr.h"

int hdr_tokenbus::offset_ = 0;
packet_t PT_UWTOKENBUS;

/**
 * Class that describe the Header of a token bus packet
 */
static class TokenBusHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	TokenBusHeaderClass()
		: PacketHeaderClass("PacketHeader/UWTOKENBUS", sizeof(hdr_tokenbus))
	{
		this->bind();
		bind_offset(&hdr_tokenbus::offset_);
	}
} class_hdr_tokenbus;


extern EmbeddedTcl uwtokenbus_default;
extern "C" int
Uwtokenbus_Init()
{
	PT_UWTOKENBUS = p_info::addPacket("UWTOKENBUS");
	uwtokenbus_default.load();
	return 0;
}
