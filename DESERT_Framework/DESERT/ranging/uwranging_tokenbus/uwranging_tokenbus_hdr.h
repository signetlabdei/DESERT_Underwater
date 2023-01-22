//
// Copyright (c) 2021 Regents of the SIGNET lab, University of Padova.
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
//

/**
 * @file	uwranging_tokenbus_hdr.h
 * @author	Antonio Montanari
 * @version 1.0.0
 *
 * \brief	Common structures and variables in the protocol
 *
 *
 */

#ifndef UWRANGINGTOKENBUS_HDR_H
#define UWRANGINGTOKENBUS_HDR_H

#include <cstdint>
#include <packet.h>
#include <vector>

extern packet_t PT_UWRANGING_TOKENBUS;

// #include "half.hpp" //for using half precision library from https://half.sourceforge.net/
//using half_float::half;
typedef float uwrange_time_t;	/**< set here the size and precision of the time measures (half/float/double/uint16...)*/

/**
 * Header of the token bus protocol
 */
typedef struct hdr_uwranging_tokenbus {
public:
	static int offset_; /**< Required by the PacketHeaderManager. */
	uwrange_time_t token_hold_; /**< time elapsed from token rx and token pass */
	std::vector<uwrange_time_t> times_; /**< Holds the times calculated by the node */
	bool token_resend_ = false; /**< flag set if token is retransmitted when TokenPass timer expires */

	/**
	 * Returns a reference to the travel times array
	 * @returns a reference to the travel times array
	 */
	std::vector<uwrange_time_t> & times()
	{
		return (times_);
	}

	/**
	 * Returns a reference to token_hold_ value
	 * @returns a reference to token_hold_
	 */
	uwrange_time_t & token_hold()
	{
		return (token_hold_);
	}

	/**
	 * Returns a reference to token_resend_
	 * @returns a reference to token_resend_
	 */
	bool & token_resend()
	{
		return (token_resend_);
	}
	/**
	 * Returns the size of this header
	 * @returns the size of this header
	 */
	size_t getSize() const
	{
		return sizeof(uwrange_time_t)*(times_.size() + 1); //1 bit for bool token_resend missing
	}

	/**
	 * Returns a reference to the offset_ variable
	 * @returns a reference to the offset_ variable
	 */
	inline static int & offset()
	{
		return offset_;
	}

	/**
	 * Returns a pointer to the tokenbus_ranging header of a packet
	 * @param p Packet
	 * @returns a pointer to the tokenbus_ranging header of the packet p
	 */
	inline static struct hdr_uwranging_tokenbus *
	access(const Packet *p)
	{
		return (struct hdr_uwranging_tokenbus *) p->access(offset_);
	}
} hdr_uwranging_tokenbus;

#define HDR_UWRANGING_TOKENBUS(p) \
	(hdr_uwranging_tokenbus::access(p))	/**< alias defined to access the TOKEN BUS HEADER*/

#endif