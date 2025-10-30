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
//

/**
 * @file	uwranging_tdoa_hdr.h
 * @author	Antonio Montanari
 * @version 1.0.0
 *
 * \brief	Common structures and variables in the protocol
 *
 *
 */

#ifndef UWRANGING_TDOA_HDR_H
#define UWRANGING_TDOA_HDR_H

#include <cmath>
#include <packet.h>
#include <sys/types.h>

#define MAX_NUM_NODES 32
#define HDR_RANGING_TDOA(p)    \
	(hdr_ranging_tdoa::access( \
			p)) /**< alias defined to access the hdr_ranging_tdoa HEADER*/

extern packet_t PT_UWRANGING_TDOA;

typedef float
		uwrange_time_t; /**< Precision of the holdover times measurement. */
typedef uint8_t uwrange_pkt_t; /**< Number of bits to store a packet id. */
typedef uint8_t uwrange_node_t; /**< Number of bits to store a node id. */

/**
 * Define the struct of a single tdoa payload entry
 */
typedef struct tdoa_entry {
	uwrange_time_t time; /**< Holdover time.  */
	uwrange_pkt_t id; /**< Id of the packet. */
	uwrange_node_t node; /**< Id of the source node. */

	tdoa_entry(uwrange_time_t time, uwrange_node_t node, uwrange_pkt_t id)
		: time(time)
		, id(id)
		, node(node)
	{
	}

	tdoa_entry()
		: tdoa_entry(-1., -1, -1)
	{
	}
} tdoa_entry;

/**
 * Header of the token bus protocol
 */
typedef struct hdr_ranging_tdoa {
	static int offset_; /**< Required by the PacketHeaderManager. */
	uwrange_pkt_t source_pkt_id; /**< Ranging packet id. */
	uwrange_node_t source_node_id; /**< Source node id. */
	uwrange_node_t times_size_; /** Number of entries in the packet. */
	tdoa_entry times_[MAX_NUM_NODES]; /**< Array of entries to be sent */

	inline uint8_t &
	times_size()
	{
		return times_size_;
	}

	/**
	 * Returns the size of a ranging header with all the entries
	 * @returns Returns the size of a ranging header with all the entries
	 */
	size_t
	getSize() const
	{
		return std::ceil(times_size_ * sizeof(tdoa_entry) +
				sizeof(uwrange_node_t) + sizeof(uwrange_pkt_t));
	}

	/**
	 * Returns the size of a ranging header with a given number of entries
	 * @returns Returns the size of a ranging header with a given number of
	 * entries
	 */
	static size_t
	getSize(int entries)
	{
		return std::ceil(entries * sizeof(tdoa_entry) + sizeof(uwrange_node_t) +
				sizeof(uwrange_pkt_t));
	}

	/**
	 * Returns a reference to the offset_ variable
	 * @returns a reference to the offset_ variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	/**
	 * Returns a pointer to the tokenbus_ranging header of a packet
	 * @param p Packet
	 * @returns a pointer to the tokenbus_ranging header of the packet p
	 */
	inline static struct hdr_ranging_tdoa *
	access(const Packet *p)
	{
		return (struct hdr_ranging_tdoa *) p->access(offset_);
	}
} hdr_ranging_tdoa;

#endif
