//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
 * @file   tls-record-header.h
 * @author Davide Costa
 * @version 1.0.0
 *
 * \brief Provides the headers of the TLS record layer.
 *
 */

#ifndef UWTLS_RECORD_H
#define UWTLS_RECORD_H

#include <packet.h>

/** Alias defined to access the TLS record header */
#define HDR_TLS_RECORD(p) (hdr_tls_record::access(p))

extern packet_t PT_TLS_RECORD; /**< TLS record packet type */

/**
 * Content header of the TLS record layer
 */
typedef struct hdr_tls_record {
	uint8_t contentType;	// 22 = handshake
	uint16_t version; 	// TLS 1.2: 0x0303
	uint16_t length;	// Fragment length
	
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static hdr_tls_record* access(const Packet* p) {
		return (hdr_tls_record*) p->access(offset_);
	}
} hdr_tls_record;

#endif // UWTLS_RECORD_H
