//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwtp-pkt.h
 * @author Saiful Azad
 * @version 1.0.0
 * 
 * @brief  Declaration of the header of UWTP packet
 */

#ifndef UWTP_PKT_H_
#define UWTP_PKT_H_

#include <packet.h>

/* 
=====================================================================
Packet Formats...
===================================================================== 
*/

#define UWTP_DATA	  	0x01
#define UWTP_ACK		0x02
#define UWTP_NACK		0x03

/*
 * UWTRANS Header Macros
 */
#define HDR_UWTP(p) ((struct hdr_uwtp*)hdr_uwtp::access(p))
#define HDR_UWTP_DATA(p) ((struct hdr_uwtp_data*)hdr_uwtp::access(p))
#define HDR_UWTP_ACK(p) ((struct hdr_uwtp_ack*)hdr_uwtp::access(p))
#define HDR_UWTP_NACK(p) ((struct hdr_uwtp_nack*)hdr_uwtp::access(p))

/**
* UWTRANS Header
*/

struct hdr_uwtp {
        char        type_;
	inline char& getType() { return type_; }

	// Header access methods
	static int offset_; // required by PacketHeaderManager
	inline static int& offset() { return offset_; }
	inline static struct hdr_uwtrans* access(const Packet* p) {
		return (struct hdr_uwtrans*) p->access(offset_);
	}
};

/**
* Header of <i>Data</i> Packet. Header fields are: type of the packet, source port address, destination port address
*/
struct hdr_uwtp_data {
        char        	type_;		// Packet Type
    	u_int16_t 	sport_;
    	u_int16_t 	dport_;

	inline char&		getType() { return type_; }
	inline u_int16_t&	getSport() { return sport_; }
	inline u_int16_t&	getDport() { return dport_; }

	inline int size() {
		return (sizeof(char)+sizeof(u_int16_t)*2);
	}
};

/**
* Header of <i>ACK</i> packet. A ack will be sent only if a node receive a packet successfully.
* Header fields are: packet type, source port address, destination port address and sequence number of the received packet.
*/
struct hdr_uwtp_ack {
        char        	type_;        	// Packet Type
    	u_int16_t 	sport_;
    	u_int16_t 	dport_;
	u_int32_t	seq_no_;

	inline char&		getType() { return type_; }
	inline u_int16_t&	getSport() { return sport_; }
	inline u_int16_t&	getDport() { return dport_; }
	inline u_int32_t&	getSeqNo() { return seq_no_; }
	
	inline int size() { 
		return (sizeof(char)+sizeof(u_int16_t)*2+sizeof(u_int32_t));
	}
	

};

/**
* Header of <i>NACK</i> packet. If a node receives a out of order packet, it assumes some of the packets are missing and sends <i>NACK</i> packet. 
* Header fields are: packet type, source port address, destination port address and sequence number of the missing packet(s).
*/
struct hdr_uwtp_nack {
        char        	type_;        	// Packet Type
    	u_int16_t 	sport_;
    	u_int16_t 	dport_;
	u_int32_t	seq_no_;

	inline char&		getType() { return type_; }
	inline u_int16_t&	getSport() { return sport_; }
	inline u_int16_t&	getDport() { return dport_; }
	inline u_int32_t&	getSeqNo() { return seq_no_; }
	
	inline int size() { 
		return (sizeof(char)+sizeof(u_int16_t)*2+sizeof(u_int32_t));
	}
	

};

/**
 * Union of all headers for space calculation of header
 */
union hdr_all_uwtp {
  hdr_uwtp         	huwt;
  hdr_uwtp_data		huwtd;
  hdr_uwtp_ack		huwta;
  hdr_uwtp_nack		huwtna;
};


#endif //UWTP_PKT_H_
