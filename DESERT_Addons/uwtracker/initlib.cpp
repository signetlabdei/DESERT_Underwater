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
 * @file   initlib.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Provides the initialization of uwtracker libraries.
 *
 * Provides the initialization of uwtracker libraries. In addition,
 * it provides both <i>UWTRACKER</i> monitoring and control packets header description.
 * 
 */

#include <tclcl.h>
#include <sap.h>

#include "uwtracker-packet.h"
extern EmbeddedTcl UwtrackerTclCode;

packet_t PT_UWTRACKER;

/**
 * Adds the header for <i>hdr_uwTRACK</i> packets in ns2.
 */
static class UwTrackPktClass : public PacketHeaderClass {
public:

    UwTrackPktClass() : PacketHeaderClass("PacketHeader/UWTRACK", sizeof (hdr_uwTracker)) {
        this->bind();
        bind_offset(&hdr_uwTracker::offset_);
    }
} class_uwTRACK_pkt;


extern "C" int Uwtracker_Init() {
    PT_UWTRACKER = p_info::addPacket("UWTRACK");
	UwtrackerTclCode.load();
	return 0;
}
