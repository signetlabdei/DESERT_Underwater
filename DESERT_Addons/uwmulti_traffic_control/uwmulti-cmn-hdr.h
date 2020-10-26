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
//

/**
 * @file	uwmulti_cmn_hdr.h
 * @author	Enrico Lovisotto
 * @version     1.0.0
 *
 * \brief	Common structures and variables in the protocol
 *
 *
 */

#ifndef UWMULTI_CMN_HDR_H
#define UWMULTI_CMN_HDR_H

#include <mmac.h>
#include <module.h>
#include <packet.h>

#define HDR_UWMULTIPHY_PING(p) \
	(hdr_UWMULTIPHY_PING::access(p)) /**< alias defined to access the UWMULTIPHY_PING HEADER */
#define HDR_UWMULTIPHY_PONG(p)                                          \
	(hdr_UWMULTIPHY_PONG::access(p)) /**< alias defined to access the UWMULTIPHY_PONG HEADER */
#define HDR_UWMULTIPHY_DATA(p)                                                       \
	(hdr_uwmultiphy_data::access(p)) /**< alias defined to access the DATA HEADER */

extern packet_t PT_UWMULTIPHY_DATA;
extern packet_t PT_UWMULTIPHY_PING;
extern packet_t PT_UWMULTIPHY_PONG;

/**
 * Header of the DATA message
 */
typedef struct hdr_uwmultiphy_data {
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_uwmultiphy_data *
	access(const Packet *p)
	{
		return (struct hdr_uwmultiphy_data *) p->access(offset_);
	}
} hdr_uwmultiphy_data;


/**
 * Header of the uwmultiphy_pong message
 */
typedef struct hdr_uwmultiphy_pong {
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_uwmultiphy_pong *
	access(const Packet *p)
	{
		return (struct hdr_uwmultiphy_pong *) p->access(offset_);
	}
} hdr_uwmultiphy_pong;

typedef struct hdr_uwmultiphy_ping {
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_uwmultiphy_ping *
	access(const Packet *p)
	{
		return (struct hdr_uwmultiphy_ping *) p->access(offset_);
	}
} hdr_uwmultiphy_ping;


#endif
