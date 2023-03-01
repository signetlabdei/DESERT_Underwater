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
 * @file	uwofdmmac_hdr.h
 * @author	Sara Falleni
 * @version     1.0.0
 * @date 	12 October 2021
 * \brief	Common necessary fields for smart-ofdm
 *
 *
 */

#ifndef UWOFDMMAC_HDR_H
#define UWOFDMMAC_HDR_H
#define MAX_AVAL_CAR 10 //like the number of carriers in the system


#include <mmac.h>
#include <module.h>
#include <packet.h>
#include <string>

#define HDR_OFDMMAC(p) \
	(hdr_OFDMMAC::access(p)) /**< alias defined to access the PROBE HEADER */


extern packet_t PT_OFDMMAC;

/**
 * Header of the OFDM message with fields to implement a multi carrier system
 */
// Total size of MAC header is 17 bytes 
typedef struct hdr_OFDMMAC {

	static int offset_; /**< Required by the PacketHeaderManager. 4 bytes*/

	int usage_carriers[MAX_AVAL_CAR];      /// Vector with available carriers. fields are -1 if no more carriers are available (1 byte for now)
								// Picks from occupancy_table
	int bytesToSend; // for RTS 4 bytes 

	double timeReserved; // for CTS, how long is each carriers reserved for 8 bytes 

	/**
	 * Reference to the offset_ variable.
	 */
	inline static int &
	offset()
	{
		return hdr_OFDMMAC::offset_;
	}

	inline static struct hdr_OFDMMAC *
	access(const Packet *p)
	{
		return (struct hdr_OFDMMAC *) p->access(hdr_OFDMMAC::offset_);
	}
} hdr_OFDMMAC;


#endif
