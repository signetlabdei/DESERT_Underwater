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
 * @file  initlib.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief file to initialize the libuwphysical.so library
 */

#include <tclcl.h>
#include "uwmulti-traffic-range-crt.h"
#include <uwmulti-cmn-hdr.h>

extern EmbeddedTcl UwMultiTrafficControlInitTclCode;

int hdr_uwm_tr::offset_;
// new protocol
int hdr_uwmultiphy_pong::offset_ = 0;
int hdr_uwmultiphy_ping::offset_ = 0;
int hdr_uwmultiphy_data::offset_ = 0;

packet_t PT_MULTI_TR_PROBE;
packet_t PT_MULTI_TR_PROBE_ACK;

packet_t PT_UWMULTIPHY_DATA;
packet_t PT_UWMULTIPHY_PING;
packet_t PT_UWMULTIPHY_PONG;

static class MultiTrHeaderClass : public PacketHeaderClass
{
public:
    /**
     * Constructor of the class
     */
    MultiTrHeaderClass()
    : PacketHeaderClass("PacketHeader/MULTI_TR", sizeof(hdr_uwm_tr))
    {
        this->bind();
        bind_offset(&hdr_uwm_tr::offset_);
    }
} class_hdr_MULTI_TR;

/**
 * Class that describe the Header of UWMULTIPHY_PING Packet
 */
static class UwmultiphyPingHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwmultiphyPingHeaderClass()
		: PacketHeaderClass("PacketHeader/UWMULTIPHY_PING", sizeof(hdr_uwmultiphy_ping))
	{
		this->bind();
		bind_offset(&hdr_uwmultiphy_ping::offset_);
	}
} class_hdr_uwmultiphy_ping;

/**
 * Class that describe the Header of UWMULTIPHY_PONG Packet
 */
static class UwmultiphyPongHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwmultiphyPongHeaderClass()
		: PacketHeaderClass("PacketHeader/UWMULTIPHY_PONG", sizeof(hdr_uwmultiphy_pong))
	{
		this->bind();
		bind_offset(&hdr_uwmultiphy_pong::offset_);
	}
} class_hdr_uwmultiphy_pong;

/**
 * Class that describe the Header of UWMULTIPHY_DATA Packet
 */
static class UwmultiphyDataHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwmultiphyDataHeaderClass()
		: PacketHeaderClass("PacketHeader/UWMULTIPHY_DATA", sizeof(hdr_uwmultiphy_data))
	{
		this->bind();
		bind_offset(&hdr_uwmultiphy_data::offset_);
	}
} class_hdr_uwmultiphy_data;

extern "C" int Uwmulti_traffic_control_Init()
{
  	PT_MULTI_TR_PROBE = p_info::addPacket("MULTI_TR_PROBE");
  	PT_MULTI_TR_PROBE_ACK = p_info::addPacket("MULTI_TR_PROBE_ACK");

    PT_UWMULTIPHY_PING = p_info::addPacket((char*) "UWMULTIPHY/UWMULTIPHY_PING");
    PT_UWMULTIPHY_PONG = p_info::addPacket((char*) "UWMULTIPHY/UWMULTIPHY_PONG");

    PT_UWMULTIPHY_DATA = p_info::addPacket((char*) "UWMULTIPHY/UWMULTIPHY_DATA");

  	UwMultiTrafficControlInitTclCode.load();
  	return 0;
}

extern "C" int Cygmulti_traffic_control_Init()
{
    Uwmulti_traffic_control_Init();
    return 0;
}
