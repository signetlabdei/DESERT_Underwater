//
// Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwtap_clmsg.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * \brief Definition of ClMsg for UwTap class.
 *
 */

#include "uwtap_clmsg.h"
#include <phymac-clmsg.h>
#include <mmac.h>


ClMsgTapPkt::ClMsgTapPkt()
    : ClMessage(ClMsgTapPkt::CLMSG_TAP_VERBOSITY, CLMSG_TAP_PKT)
    , pkt(nullptr)
    , clmsg_type(clmsg_tap_type::NONE)
    ,timestamp(NOW)
    , tx_duration(0.0)
{
}

ClMsgTapPkt::ClMsgTapPkt(Packet* packet,clmsg_tap_type type,double timestamp, double tx_duration)
    : ClMessage(ClMsgTapPkt::CLMSG_TAP_VERBOSITY, 
			CLMSG_TAP_PKT)
    , pkt(packet)
    , clmsg_type(type)
    , timestamp(timestamp)
    , tx_duration(tx_duration)
{
}

ClMsgTapPkt::ClMsgTapPkt(int dest_id)
    : ClMessage(ClMsgTapPkt::CLMSG_TAP_VERBOSITY,
			CLMSG_TAP_PKT,UNICAST,dest_id)
    , pkt(nullptr)
{
}


bool
ClMsgTapPkt::getPacket(Packet *packet)
{
    if(pkt != nullptr) {
        packet = pkt;
        return true;
    }
    return false;
}

void
ClMsgTapPkt::setPacket(Packet* packet)
{
    pkt = packet;
}

double 
ClMsgTapPkt::getTimestamp() const
{
    return timestamp;
}

double 
ClMsgTapPkt::getTxDuration() const
{
    return tx_duration;
}
