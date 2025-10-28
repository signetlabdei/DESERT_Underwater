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
 * @file   uw-aloha-q-sync-hdrs.h
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * \brief Provides the headers of ack packets of uw-aloha-q-sync-sink module
 *
 */

#ifndef UWALOHA_Q_SYNC_HDRS
#define UWALOHA_Q_SYNC_HDRS

#include <packet.h>
#include <vector>

#define ALOHAQ_SYNC_ACK_HDR_ACCESS(p) (aloha_q_sync_ACK::access(p))

extern packet_t PT_ALOHAQ_SYNC_ACK;

typedef struct aloha_q_sync_ACK {

public:

	std::vector<int> succ_macs;
	
	static int offset_;
	std::vector<int> get_succ_macs()
	{
		return succ_macs;
	}
	inline void
	set_succ_macs(vector<int> succ_macs_arr)
	{
		succ_macs.clear();
		succ_macs.reserve(30);
		for (int i = 0 ; i < succ_macs_arr.size() ; i++){
			succ_macs.push_back(succ_macs_arr[i]);
		}	
	}
	inline static int &
	offset()
	{
		return offset_;
	}
	inline static struct aloha_q_sync_ACK *
	access(const Packet *p)
	{
		return (struct aloha_q_sync_ACK *) p->access(offset_);
	}
} aloha_q_sync_ACK;

#endif
