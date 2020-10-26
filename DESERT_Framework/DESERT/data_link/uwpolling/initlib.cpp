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
 * @file   data_link/uwpolling/initlib.cpp
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the initialization of the uwpolling libraries
 *
 */

#include <tclcl.h>
#include "uwpolling_AUV.h"
#include "uwpolling_NODE.h"
#include "uwpolling_SINK.h"
#include "uwpolling_cmn_hdr.h"
#include "sap.h"
#include "packet.h"

int hdr_PROBE::offset_ = 0;
int hdr_TRIGGER::offset_ = 0;
int hdr_POLL::offset_ = 0;
int hdr_AUV_MULE::offset_ = 0;
int hdr_ACK_SINK::offset_ = 0;
int hdr_PROBE_SINK::offset_ = 0;

packet_t PT_TRIGGER;
packet_t PT_PROBE;
packet_t PT_PROBE_SINK;
packet_t PT_POLL;
packet_t PT_AUV_MULE;
packet_t PT_ACK_SINK;

/**
 * Class that describe the Header of PROBE Packet
 */
static class ProbeHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	ProbeHeaderClass()
		: PacketHeaderClass("PacketHeader/PROBE", sizeof(hdr_PROBE))
	{
		this->bind();
		bind_offset(&hdr_PROBE::offset_);
	}
} class_hdr_PROBE;

/**
 * Class that describe the Header of TRIGGER Packet
 */
static class TriggerHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	TriggerHeaderClass()
		: PacketHeaderClass("PacketHeader/TRIGGER", sizeof(hdr_TRIGGER))
	{
		this->bind();
		bind_offset(&hdr_TRIGGER::offset_);
	}
} class_hdr_TRIGGER;

/**
 * Class that describe the Header of POLL Packet
 */
static class PollHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	PollHeaderClass()
		: PacketHeaderClass("PacketHeader/POLL", sizeof(hdr_POLL))
	{
		this->bind();
		bind_offset(&hdr_POLL::offset_);
	}
} class_hdr_POLL;

/**
 * Class that describe the Header of AUV_MULEt
 */
static class AuvMuleHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	AuvMuleHeaderClass()
		: PacketHeaderClass("PacketHeader/AUV_MULE", sizeof(hdr_AUV_MULE))
	{
		this->bind();
		bind_offset(&hdr_AUV_MULE::offset_);
	}
} class_hdr_AUV_MULE;

/**
 * Class that describe the Header of ACK sent by the sink
 */
static class AckSinkHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	AckSinkHeaderClass()
		: PacketHeaderClass("PacketHeader/ACK_SINK", sizeof(hdr_ACK_SINK))
	{
		this->bind();
		bind_offset(&hdr_ACK_SINK::offset_);
	}
} class_hdr_ACK_SINK;

/**
 * Class that describe the Header of PROBE_SINK Packet
 */
static class ProbeSinkHeaderClass : public PacketHeaderClass
{
public:
	/**
	 * Constructor of the class
	 */
	ProbeSinkHeaderClass()
		: PacketHeaderClass("PacketHeader/PROBE_SINK", sizeof(hdr_PROBE_SINK))
	{
		this->bind();
		bind_offset(&hdr_PROBE_SINK::offset_);
	}
} class_hdr_PROBE_SINK;

extern EmbeddedTcl uwpolling_default;

extern "C" int
Uwpolling_Init()
{
	PT_PROBE = p_info::addPacket("UWPOLLING/PROBE");
	PT_TRIGGER = p_info::addPacket("UWPOLLING/TRIGGER");
	PT_POLL = p_info::addPacket("UWPOLLING/POLL");
	PT_AUV_MULE = p_info::addPacket("UWPOLLING/AUV_MULE");
	PT_ACK_SINK =p_info::addPacket("UWPOLLING/ACK_SINK");
	PT_PROBE_SINK = p_info::addPacket("UWPOLLING/PROBE_SINK");
	uwpolling_default.load();
	return 0;
}
