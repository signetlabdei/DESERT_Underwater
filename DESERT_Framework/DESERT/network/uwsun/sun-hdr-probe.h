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
 * @file   sun-hdr-probe.h
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Provides the Probe Messages header description.
 *
 */

#ifndef HDR_SUN_PROBE_H
#define	HDR_SUN_PROBE_H

#include "sun-ipr-common-structures.h"

#include <packet.h>

#define HDR_SUN_PROBE(p) (hdr_sun_probe::access(p))

extern packet_t PT_SUN_PROBE;

/**
 * <i>hdr_sun_probe</i> describes probe packets used by <i>UWSUN</i>
 */
typedef struct hdr_sun_probe { 

    static int offset_; /**< Required by the PacketHeaderManager */

    /**
     * Reference to the offset_ variable
     */
    inline static int& offset() {
        return offset_;
    }

    inline static struct hdr_sun_probe * access(const Packet * p) {
        return (struct hdr_sun_probe*) p->access(offset_);
    }
} hdr_sun_probe;

#endif // HDR_SUN_PROBE_H

