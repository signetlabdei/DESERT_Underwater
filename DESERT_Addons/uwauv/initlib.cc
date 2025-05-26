//
// Copyright (c) 2023 Regents of the SIGNET lab, University of Padova.
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
 * @author Alessia Ortile
 * @version 1.0.0
 *
 * \brief Provides the initialization of uwauv libraries.
 *
 * Provides the initialization of uwauv libraries. In addition,
 * it provides both <i>UWAUV</i> monitoring and control packets header description.
 * 
 */

#include <tclcl.h>
#include <sap.h>

#include "uwauv-packet.h"
extern EmbeddedTcl UwauvTclCode;

packet_t PT_UWAUV;
packet_t PT_UWAUV_CTR;
packet_t PT_UWAUV_ERROR;

static class UwAUVMonPktClass : public PacketHeaderClass {
public:

    UwAUVMonPktClass() : PacketHeaderClass("PacketHeader/UWAUV", sizeof (hdr_uwAUV_monitoring)) {
        this->bind();
        bind_offset(&hdr_uwAUV_monitoring::offset_);
    }
} class_uwAUV_pkt;
/**
 * Adds the header for <i>hdr_uwAUV</i> packets in ns2.
 */
static class UwAUVCtrPktClass : public PacketHeaderClass {
public:

    UwAUVCtrPktClass() : PacketHeaderClass("PacketHeader/UWAUVCtr", sizeof (hdr_uwAUV_ctr)) {
        this->bind();
        bind_offset(&hdr_uwAUV_ctr::offset_);
    }
} class_uwAUVCtr_pkt;

static class UwAUVErrorPktClass : public PacketHeaderClass {
public:

    UwAUVErrorPktClass() : PacketHeaderClass("PacketHeader/UWAUVError", sizeof (hdr_uwAUV_error)) {
        this->bind();
        bind_offset(&hdr_uwAUV_error::offset_);
    }
} class_uwAUVError_pkt;



extern "C" int Uwauv_Init() {
    PT_UWAUV = p_info::addPacket("UWAUV");
    PT_UWAUV_CTR = p_info::addPacket("UWAUVCtr");
    PT_UWAUV_ERROR = p_info::addPacket("UWAUVError");
	UwauvTclCode.load();
	return 0;
}

