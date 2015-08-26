//
// Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Provides the initialization of uwrov libraries.
 *
 * Provides the initialization of uwrov libraries. In addition,
 * it provides both <i>UWROV</i> monitoring and control packets header description.
 * 
 */

#include <tclcl.h>
#include <sap.h>

#include "uwrov-packet.h"
extern EmbeddedTcl UwrovTclCode;

static class UwROVMonPktClass : public PacketHeaderClass {
public:

    UwROVMonPktClass() : PacketHeaderClass("PacketHeader/UWROV", sizeof (hdr_uwROV_monitoring)) {
        this->bind();
        bind_offset(&hdr_uwROV_monitoring::offset_);
    }
} class_uwROV_pkt;
/**
 * Adds the header for <i>hdr_uwROV</i> packets in ns2.
 */
static class UwROVCtrPktClass : public PacketHeaderClass {
public:

    UwROVCtrPktClass() : PacketHeaderClass("PacketHeader/UWROVCtr", sizeof (hdr_uwROV_ctr)) {
        this->bind();
        bind_offset(&hdr_uwROV_ctr::offset_);
    }
} class_uwROVCtr_pkt;


extern "C" int Uwrov_Init() {
	UwrovTclCode.load();
	return 0;
}
