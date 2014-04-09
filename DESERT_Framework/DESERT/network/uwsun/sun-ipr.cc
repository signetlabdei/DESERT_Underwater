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
 * @file   sun-ipr.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Packets' class implementation.
 * 
 */

#include "sun-hdr-ack.h"
#include "sun-hdr-data.h"
#include "sun-hdr-pathestablishment.h"
#include "sun-hdr-probe.h"

#include <tclcl.h>

int hdr_sun_ack::offset_ = 0;       /**< Offset used to access in <i>hdr_sun_ack</i> packets header. */
int hdr_sun_data::offset_ = 0;      /**< Offset used to access in <i>hdr_sun_data</i> packets header. */
int hdr_sun_path_est::offset_ = 0;  /**< Offset used to access in <i>hdr_sun_path_est</i> packets header. */
int hdr_sun_probe::offset_ = 0;     /**< Offset used to access in <i>hdr_sun_probe</i> packets header. */

/**
 * Adds the header for <i>hdr_sun_ack</i> packets in ns2.
 */
static class SunAckPktClass : public PacketHeaderClass {
public:

    SunAckPktClass() : PacketHeaderClass("PacketHeader/SUN_ACK", sizeof(hdr_sun_ack)) {
        this->bind();
        bind_offset(&hdr_sun_ack::offset_);
    }
} class_sun_ack_pkt;

/**
 * Adds the header for <i>hdr_sun_data</i> packets in ns2.
 */
static class SunDataPktClass : public PacketHeaderClass {
public:

    SunDataPktClass() : PacketHeaderClass("PacketHeader/SUN_DATA", sizeof(hdr_sun_data)) {
        this->bind();
        bind_offset(&hdr_sun_data::offset_);
    }
} class_sun_data_pkt;

/**
 * Adds the header for <i>hdr_sun_path_est</i> packets in ns2.
 */
static class SunPestPktClass : public PacketHeaderClass {
public:

    SunPestPktClass() : PacketHeaderClass("PacketHeader/SUN_PEST", sizeof(hdr_sun_path_est)) {
        this->bind();
        bind_offset(&hdr_sun_path_est::offset_);
    }
} class_sun_pest_pkt;

/**
 * Adds the header for <i>hdr_sun_probe</i> packets in ns2.
 */
static class SunProbePktClass : public PacketHeaderClass {
public:

    SunProbePktClass() : PacketHeaderClass("PacketHeader/SUN_PROBE", sizeof(hdr_sun_probe)) {
        this->bind();
        bind_offset(&hdr_sun_probe::offset_);
    }
} class_sun_probe_pkt;
