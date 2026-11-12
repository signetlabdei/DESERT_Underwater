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
 * @file   clientkeyexchange-header.h
 * @author Davide Costa
 * @version 1.0.0
 *
 * \brief Provides the headers of the client key exchange layer.
 *
 */

#ifndef UWTLS_CKE_H
#define UWTLS_CKE_H

#include <packet.h>

/** Alias defined to access the client key exchange header */
#define HDR_CLIENTKEYEXCHANGE(p) (hdr_clientkeyexchange::access(p))

extern packet_t PT_CLIENTKEYEXCHANGE; /**< CLIENTKEYEXCHANGE packet type */

/**
 * Content header of the client key exchange layer
 */
typedef struct hdr_clientkeyexchange {
    uint16_t encrypted_premaster_len;
    uint8_t encrypted_premaster[256];
    
    static int offset_;
    inline static int& offset() { return offset_; }
    inline static hdr_clientkeyexchange* access(const Packet* p) {
        return (hdr_clientkeyexchange*) p->access(offset_);
    }
} hdr_clientkeyexchange;

#endif // UWTLS_CKE_H
