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
 * @file   uwflooding-hdr.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Provides the header description for UWFLOODING packets.
 *
 * Provides the header description for UWFLOODING packets.
 */

#ifndef HDR_UWFLOODING_H
#define HDR_UWFLOODING_H

#include <packet.h>

#define HDR_UWFLOODING(p) (hdr_uwflooding::access(p))

extern packet_t PT_UWFLOODING;

/**
 * <i>hdr_uwflooding</i> describes packets used by <i>UWFLOODING</i>.
 */
typedef struct hdr_uwflooding {
    
    uint8_t     ttl_;            /**< Time to leave of the packet. */
    static int  offset_;         /**< Required by the PacketHeaderManager. */
    
    /**
     * Reference to the uid_ variable.
     */
    inline uint8_t& ttl() {
        return ttl_;
    }
    
    /**
     * Reference to the offset_ variable.
     */
    inline static int& offset() {
        return offset_;
    }
    
    inline static struct hdr_uwflooding* access(const Packet * p) {
        return (struct hdr_uwflooding*) p->access(offset_);
    }
} hdr_uwflooding;

#endif // HDR_UWFLOODING_H

