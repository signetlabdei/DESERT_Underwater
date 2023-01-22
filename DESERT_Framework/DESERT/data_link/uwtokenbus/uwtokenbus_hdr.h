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
 * @file	uwtokenbus_hdr.h
 * @author	Antonio Montanari
 * @version 1.0.0
 *
 * \brief	Common structures and variables in the protocol
 *
 *
 */

#ifndef UWTOKENBUS_HDR_H
#define UWTOKENBUS_HDR_H

#include <cstdint>
#include <packet.h>
#include <limits>

typedef uint_least16_t tokenid_t; /**< set here the size of the tokenid counter*/
constexpr size_t TOKENIDMAX = std::numeric_limits<tokenid_t>::max(); 

extern packet_t PT_UWTOKENBUS;
/**
 * Header of the token bus protocol
 */
typedef struct hdr_tokenbus {
public:
	static int offset_; /**< Required by the PacketHeaderManager. */
	tokenid_t token_id_; /**< progressive token id, indicates which node has the token */
	
	/**
	 * Returns a reference to the token_id variable
	 * @returns a reference to the token_id variable
	 */
	tokenid_t & tokenId()
	{
		return (token_id_);
	}
	
	/**
	 * Returns the size of this header
	 * @returns the size of this header
	 */
	size_t getSize() const
	{
		return sizeof(tokenid_t); 
	}

	/**
	 * Returns a reference to the offset variable
	 * @returns a reference to the offset variable
	 */
	inline static int & offset()
	{
		return offset_;
	}

	/**
	 * Returns a pointer to the tokenbus header of the packet p
	 * @param p Packet
	 * @returns a pointer to the tokenbus header of the packet p
	 */
	inline static struct hdr_tokenbus *
	access(const Packet *p)
	{
		return (struct hdr_tokenbus *) p->access(offset_);
	}
} hdr_tokenbus;

#define HDR_TOKENBUS(p) \
	(hdr_tokenbus::access(p))	/**< alias defined to access the TOKEN BUS HEADER*/

#endif