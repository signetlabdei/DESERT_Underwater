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
 * @file	uwofdmphy_hdr.h
 * @author	Sara Falleni
 * @version     1.0.0
 *
 * \brief	Common structures and variables in the protocol
 *
 *
 */

#ifndef UWOFDMPHY_HDR_H
#define UWOFDMPHY_HDR_H

#include <mmac.h>
#include <module.h>
#include <packet.h>
#include <string>

#define HDR_OFDM(p) \
	(hdr_OFDM::access(p)) /**< alias defined to access the PROBE HEADER */

#define MAX_CARRIERS 16 	//This can be changed to do simulations with more carriers 
extern packet_t PT_OFDM;

/**
 * Header of the OFDM message with fields to implement a multi carrier system
 */
typedef struct hdr_OFDM {

	static int offset_; 			/**< Required by the PacketHeaderManager. */
	int carriers [MAX_CARRIERS]; 	// Carriers vector: 1 if used 0 otherwise 
	double carrierSize;    			// Carrier size
  	int carrierNum;     			// NUmber of subcarriers 
	string carMod [MAX_CARRIERS];	// Carriers Modulation vector
	bool nativeOFDM = false;		// If a packet was created by an OFDM node
	int srcID;						// ID of the node creating the packet 

	/**
	 * Reference to the offset_ variable.
	 */
	inline static int &
	offset()
	{
		return hdr_OFDM::offset_;
	}

	inline static struct hdr_OFDM *
	access(const Packet *p)
	{
		return (struct hdr_OFDM *) p->access(hdr_OFDM::offset_);
	}
} hdr_OFDM;


#endif
