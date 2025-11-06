//
// Copyright (c) 2024 Regents of the SIGNET lab, University of Padova.
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
 * @file	uwjammer_cmn_hdr.h
 * @author	Emanuele Coccolo
 * @version     1.0.0
 *
 * \brief	Header of a jammer packet.
 *
 *
 */

#ifndef UWJAMMER_CMN_HDR_H
#define UWJAMMER_CMN_HDR_H

#include <packet.h>

#define HDR_JAMMER(p) \
	(hdr_JAMMER::access(p)) /**< alias defined to access the JAMMER HEADER */

extern packet_t PT_JAMMER;

/**
 * Header of the JAMMER message
 */
typedef struct hdr_JAMMER {

	uint JAMMER_uid_; /**< JAMMER packet unique ID. */
	uint id_node_; /**< ID of the node. */
	static int offset_;

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	/**
	 * Reference to id_node variable
	 */
	uint &
	id_node()
	{
		return id_node_;
	}

	/**
	 * Reference to the JAMMER_uid variable
	 */
	uint &
	JAMMER_uid()
	{
		return JAMMER_uid_;
	}

	inline static struct hdr_JAMMER *
	access(const Packet *p)
	{
		return (struct hdr_JAMMER *) p->access(offset_);
	}
} hdr_JAMMER;

#endif
