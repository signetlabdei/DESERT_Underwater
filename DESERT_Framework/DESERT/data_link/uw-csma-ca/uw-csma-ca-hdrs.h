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
 * @file   uw-csma-ca-hdrs.h
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the headers of ctrl packets of uw-csma-ca module
 *
 */

#ifndef UWCSMA_CA_HDRS
#define UWCSMA_CA_HDRS

#include <stdint.h>
#include <packet.h>

#define CA_RTS_HDR_ACCESS(p) (hdr_ca_RTS::access(p))
#define CA_CTS_HDR_ACCESS(p) (hdr_ca_CTS::access(p))

extern packet_t PT_CA_RTS;
extern packet_t PT_CA_CTS;
extern packet_t PT_CA_ACK;

typedef struct hdr_ca_RTS {
private:
	uint8_t tx_time;

public:
	static int offset_;
	uint8_t &
	get_tx_time()
	{
		return tx_time;
	}
	inline static int &
	get_offset()
	{
		return offset_;
	}
	inline void
	set_tx_time(int time)
	{
		tx_time = time;
	}
	inline static struct hdr_ca_RTS *
	access(const Packet *p)
	{
		return (struct hdr_ca_RTS *) p->access(offset_);
	}
} hdr_ca_RTS;

typedef struct hdr_ca_CTS {
private:
	uint8_t tx_time;

public:
	static int offset_;
	uint8_t &
	get_tx_time()
	{
		return tx_time;
	}
	inline static int &
	get_offset()
	{
		return offset_;
	}
	inline void
	set_tx_time(int time)
	{
		tx_time = time;
	}
	inline static struct hdr_ca_CTS *
	access(const Packet *p)
	{
		return (struct hdr_ca_CTS *) p->access(offset_);
	}
} hdr_ca_CTS;

#endif