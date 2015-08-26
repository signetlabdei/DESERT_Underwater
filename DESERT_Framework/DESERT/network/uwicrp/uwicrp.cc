//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwicrp.cc
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Packets' class implementation.
 * 
 */

#include "uwicrp-hdr-ack.h"
#include "uwicrp-hdr-data.h"
#include "uwicrp-hdr-status.h"

#include <tclcl.h>

int hdr_uwicrp_ack::offset_ = 0;        /**< Offset used to access in <i>hdr_uwicrp_ack</i> packets header. */
int hdr_uwicrp_data::offset_ = 0;       /**< Offset used to access in <i>hdr_uwicrp_data</i> packets header. */
int hdr_uwicrp_status::offset_ = 0;     /**< Offset used to access in <i>hdr_uwicrp_status</i> packets header. */

/**
 * Adds the header for <i>hdr_uwicrp_ack</i> packets in ns2.
 */
static class UwicrpAckPktClass : public PacketHeaderClass {
public:

    UwicrpAckPktClass() : PacketHeaderClass("PacketHeader/UWICRP_ACK", sizeof(hdr_uwicrp_ack)) {
        this->bind();
        bind_offset(&hdr_uwicrp_ack::offset_);
    }
} class_uwicrp_ack_pkt;

/**
 * Adds the header for <i>hdr_uwicrp_data</i> packets in ns2.
 */
static class UwicrpDataPktClass : public PacketHeaderClass {
public:

    UwicrpDataPktClass() : PacketHeaderClass("PacketHeader/UWICRP_DATA", sizeof(hdr_uwicrp_data)) {
        this->bind();
        bind_offset(&hdr_uwicrp_data::offset_);
    }
} class_uwicrp_data_pkt;

/**
 * Adds the header for <i>hdr_uwicrp_status</i> packets in ns2.
 */
static class UwicrpStatusPktClass : public PacketHeaderClass {
public:

    UwicrpStatusPktClass() : PacketHeaderClass("PacketHeader/UWICRP_STATUS", sizeof(hdr_uwicrp_status)) {
        this->bind();
        bind_offset(&hdr_uwicrp_status::offset_);
    }
} class_uwicrp_status_pkt;
