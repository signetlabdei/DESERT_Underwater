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
 * @file   data_link/uw-smart-ofdm/initlib.cc
 * @author Sara Falleni
 * @version 1.0.0
 *
 * @brief Provides the initializazion of the uw-smart-ofdm libraries
 */

#include <tclcl.h>
#include <uw-smart-ofdm.h>

extern EmbeddedTcl UWSmartOFDMTclCode;

packet_t PT_MMAC_CTS;
packet_t PT_MMAC_RTS;
packet_t PT_MMAC_ACK;
packet_t PT_MMAC_DATA;
int hdr_OFDMMAC::offset_ = 0;
packet_t PT_OFDMMAC;

/**
 * Class that describe the Header of OFDM Packet
 */
static class OFDMMACHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	OFDMMACHeaderClass()
		: PacketHeaderClass("PacketHeader/OFDMMAC", sizeof(hdr_OFDMMAC))
	{
		this->bind();
		bind_offset(&hdr_OFDMMAC::offset_);
	}
} class_hdr_OFDMMAC;

extern "C" int
Uwsmartofdm_Init()
{
	PT_MMAC_CTS = p_info::addPacket("MMAC_CTS");
	PT_MMAC_RTS = p_info::addPacket("MMAC_RTS");
	PT_MMAC_ACK = p_info::addPacket("MMAC_ACK");		
	PT_MMAC_DATA = p_info::addPacket("MMAC_DATA");
	PT_OFDMMAC = p_info::addPacket("PT_TDRLMAC");
	UWSmartOFDMTclCode.load();
	return 0;
}

extern "C" int
Cyguwsmartofdm_Init()
{
	return Uwsmartofdm_Init();
}
