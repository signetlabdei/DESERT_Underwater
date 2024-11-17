//
// Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwranging_tdoa/initlib.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the initializazion of the uwranging_tdoa libraries
 */

#include <tclcl.h>
#include "uwranging_tdoa_hdr.h"

int hdr_ranging_tdoa::offset_ = 0;
packet_t PT_UWRANGING_TDOA;

/**
 * Class that describe the Header of a PT_UWRANGING_TDOA packet
 */
static class UwRangingTDOAHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwRangingTDOAHeaderClass()
		: PacketHeaderClass("PacketHeader/UWRANGING_TDOA", sizeof(hdr_ranging_tdoa))
	{
		this->bind();
		bind_offset(&hdr_ranging_tdoa::offset_);
	}
} class_hdr_ranging_tdoa;




extern EmbeddedTcl Uwranging_tdoaTclCode;

extern "C" int
Uwranging_tdoa_Init()
{
	PT_UWRANGING_TDOA = p_info::addPacket("UWRANGING_TDOA");
	Uwranging_tdoaTclCode.load();
	return 0;
}

