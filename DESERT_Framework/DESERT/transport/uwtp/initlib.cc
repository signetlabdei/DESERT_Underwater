//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file   transport/uwtp/initlib.cc
 * @author Saiful Azad
 * @version 1.0.0
 *
 * \brief Provides the initialization of uwtp libraries.
 *
 * Provides the initialization of uwtp libraries.
 */
#include <tclcl.h>
#include "uwtp-pkt.h"

packet_t PT_UWTP;

int hdr_uwtp::offset_;
/**
 * Class that describe the Header of UWTP Protocol
 */
static class UWTPHeaderClass : public PacketHeaderClass {
public:
	UWTPHeaderClass() : PacketHeaderClass("PacketHeader/UWTP", sizeof(hdr_all_uwtp)) {
		bind_offset(&hdr_uwtp::offset_);
	}
} class_rtProtomUWTP_hdr;

extern EmbeddedTcl UwTpInitTclCode;

extern "C" int Uwtp_Init() {
    UWTPHeaderClass* uwtph = new UWTPHeaderClass;
    uwtph->bind();
    PT_UWTP = p_info::addPacket("UWTP");
    UwTpInitTclCode.load();
    return 0;
}

extern "C" int Cyguwtp_Init() {
    Uwtp_Init();
}
