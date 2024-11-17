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
 * @file   uwstap_clmsg.h
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * \brief Definition of ClMsg for UwTap class.
 *
 */

#ifndef UWTAP_CLMSG_H
#define UWTAP_CLMSG_H

#include <clmessage.h>
#include <packet.h>

extern ClMessage_t CLMSG_TAP_PKT;
enum class clmsg_tap_type {START_TX,END_TX,START_RX,END_RX,NONE};

class ClMsgTapPkt : public ClMessage {

public:	
	/**
	 * Constructor for broadcast ClMsg
	 */
	ClMsgTapPkt();

	/**
	 * Constructor for broadcast ClMsg
	 */
	ClMsgTapPkt(Packet* packet,clmsg_tap_type type ,double timestamp = NOW, double tx_duration = 0.0);

	/**
	 * Constructor for UNICAST ClMsg
	 * @param dest_id id of the destiantion module
	 */
	ClMsgTapPkt(int dest_id);

	/**
	 * Destructor
	 */
	virtual ~ClMsgTapPkt() = default;

	/**
	 * Returns the pointer to the overheard packet
	 * @param packet pointer reference to the packet to be returned
	 * @return true if the packet is present, false otherwise
	 */
	bool getPacket(Packet* packet);

	/**
	 * Returns the timestamp of the overheard packet
	 * @return the timestamp of the overheard packet
	 */
	double getTimestamp() const;
	/**
	 * Returns the timestamp of the overheard packet
	 * @return the timestamp of the overheard packet
	 */
	double getTxDuration() const;

	/**
	 * Set the packet to send via clmsg
	 * @param packet pointer to the packet
	 * @return true if the packet is present, false otherwise
	 */
	void setPacket(Packet* packet);

	static const uint CLMSG_TAP_VERBOSITY = 3; /** Verbosity level.*/
	Packet* pkt; /** packet to send via clmsg to other layers. */
	clmsg_tap_type clmsg_type;
	double timestamp; /**rx or tx timestamp*/
	double tx_duration; /** stores Mac2PhyTxDuration(p)*/
}; 

#endif  