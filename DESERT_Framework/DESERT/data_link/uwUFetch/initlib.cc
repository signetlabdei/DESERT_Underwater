//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file  initlib.cc
 * @author Brolo Loris
 * @version 1.0.0
 * 
 * \brief file to initialize the libuwufetch.so library
 */

#include <tclcl.h>
#include "packet.h"
#include "uwUFetch_AUV.h"
#include "uwUFetch_NODE.h"
#include "uwUFetch_cmn_hdr.h"

int hdr_TRIGGER_UFETCH::offset_ = 0;
int hdr_RTS_UFETCH::offset_ = 0;
int hdr_CTS_UFETCH::offset_ = 0;
int hdr_BEACON_UFETCH::offset_ = 0;
int hdr_PROBE_UFETCH::offset_ = 0;
int hdr_POLL_UFETCH::offset_ = 0;
int hdr_CBEACON_UFETCH::offset_ = 0;

packet_t PT_TRIGGER_UFETCH;
packet_t PT_RTS_UFETCH;
packet_t PT_CTS_UFETCH;
packet_t PT_BEACON_UFETCH;
packet_t PT_PROBE_UFETCH;
packet_t PT_POLL_UFETCH;
packet_t PT_CBEACON_UFETCH;

/**
 * Class that describe the header of TRIGGER Packet
 */
static class TRIGGERHeaderClass: public PacketHeaderClass {
    public:
		/**
		 * Constructor of the class
		 */
        TRIGGERHeaderClass() : PacketHeaderClass("PacketHeader/TRIGGER_UFETCH", sizeof(hdr_TRIGGER_UFETCH)) {
            this->bind();
          bind_offset(&hdr_TRIGGER_UFETCH::offset_);   
        }
}class_hdr_TRIGGER;

/**
 * Class that describe the header of RTS Packet
 */
static class RTSHeaderClass: public PacketHeaderClass {
    public:
		/**
		 * Constructor of the class
		 */
        RTSHeaderClass() : PacketHeaderClass("PacketHeader/RTS_UFETCH", sizeof(hdr_RTS_UFETCH)) {
            this->bind();
          bind_offset(&hdr_RTS_UFETCH::offset_);   
        }
}class_hdr_RTS;

/**
 * Class that describe the header of CTS Packet
 */
static class CTSHeaderClass: public PacketHeaderClass {
    public:
		/**
		 * Constructor of the class
		 */
        CTSHeaderClass() : PacketHeaderClass("PacketHeader/CTS_UFETCH", sizeof(hdr_CTS_UFETCH)) {
            this->bind();
          bind_offset(&hdr_CTS_UFETCH::offset_);   
        }
}class_hdr_CTS;

/**
 * Class that describe the header of BEACON Packet
 */
static class BEACONHeaderClass: public PacketHeaderClass {
    public:
		/**
		 * Constructor of the class
		 */
        BEACONHeaderClass() : PacketHeaderClass("PacketHeader/BEACON_UFETCH", sizeof(hdr_BEACON_UFETCH)) {
            this->bind();
          bind_offset(&hdr_BEACON_UFETCH::offset_);   
        }
}class_hdr_BEACON;

/**
 * Class that describe the header of PROBE Packet
 */
static class PROBEHeaderClass: public PacketHeaderClass {
    public:
		/**
		 * Constructor of the class
		 */
        PROBEHeaderClass() : PacketHeaderClass("PacketHeader/PROBE_UFETCH", sizeof(hdr_PROBE_UFETCH)) {
            this->bind();
          bind_offset(&hdr_PROBE_UFETCH::offset_);   
        }
}class_hdr_PROBE;

/**
 * Class that describe the header of POLL Packet
 */
static class POLLHeaderClass: public PacketHeaderClass {
    public:
		/**
		 * Constructor of the class
		 */
        POLLHeaderClass() : PacketHeaderClass("PacketHeader/POLL_UFETCH", sizeof(hdr_POLL_UFETCH)) {
            this->bind();
          bind_offset(&hdr_POLL_UFETCH::offset_);   
        }
}class_hdr_POLL;

/**
 * Class that describe the header of DATA Packet
 */
static class CBEACONHeaderClass: public PacketHeaderClass {
    public:
		/**
		 * Constructor of the class
		 */
        CBEACONHeaderClass() : PacketHeaderClass("PacketHeader/CBEACON_UFETCH", sizeof(hdr_CBEACON_UFETCH)) {
            this->bind();
          bind_offset(&hdr_CBEACON_UFETCH::offset_);   
        }
}class_hdr_CBEACON;

extern EmbeddedTcl UwUFetchTclCode;

extern "C" int Uwufetch_Init() {
   PT_TRIGGER_UFETCH = p_info::addPacket("PacketHeader/TRIGGER_UFETCH");
   PT_RTS_UFETCH = p_info::addPacket("PacketHeader/RTS_UFETCH");
   PT_CTS_UFETCH = p_info::addPacket("PacketHeader/CTS_UFETCH");
   PT_BEACON_UFETCH = p_info::addPacket("PacketHeader/BEACON_UFETCH");
   PT_PROBE_UFETCH = p_info::addPacket("PacketHeader/PROBE_UFETCH");
   PT_POLL_UFETCH = p_info::addPacket("PacketHeader/POLL_UFETCH");
   PT_CBEACON_UFETCH = p_info::addPacket("PacketHeader/CBEACON_UFETCH");
   
        UwUFetchTclCode.load();
        return 0;
}

