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
 * @file   handshake-header.h
 * @author Davide Costa
 * @version 1.0.0
 *
 * \brief Provides the headers of the handshake layer.
 *
 */

#ifndef UWTLS_HANDSHAKE_H
#define UWTLS_HANDSHAKE_H

#include <packet.h>

/** Alias defined to access the HANDSHAKE header */
#define HDR_HANDSHAKE(p) (hdr_handshake::access(p))

extern packet_t PT_HANDSHAKE; /**< HANDSHAKE packet type */

/**
 * Content header of the TLS record layer
 */
typedef struct hdr_handshake {
	uint8_t type;		// 1=ClientHello, 2=ServerHello, 11=Certificate
	uint8_t length[3];	// Handshake message body length
	
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static hdr_handshake* access(const Packet* p) {
		return (hdr_handshake*) p->access(offset_);
	}
} hdr_handshake;

#endif // UWTLS_HANDSHAKE_H
