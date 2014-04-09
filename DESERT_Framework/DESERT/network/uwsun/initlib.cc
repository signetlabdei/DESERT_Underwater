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
 * @file   network/uwsun/initlib.cc
 * @author Giovanni Toso
 * @version 1.1.1
 *
 * \brief Provides the initialization of uwsun libraries.
 *
 * Provides the initialization of uwsun libraries.
 */

#include "sun-hdr-ack.h"
#include "sun-hdr-data.h"
#include "sun-hdr-pathestablishment.h"
#include "sun-hdr-probe.h"

#include <tclcl.h>

extern EmbeddedTcl SunInitTclCode;

packet_t PT_SUN_ACK;
packet_t PT_SUN_DATA;
packet_t PT_SUN_PATH_EST;
packet_t PT_SUN_PROBE;

extern "C" int Sun_Init() {
    PT_SUN_ACK = p_info::addPacket("SUN/ACK");
    PT_SUN_DATA = p_info::addPacket("SUN/DATA");
    PT_SUN_PATH_EST = p_info::addPacket("SUN/PEST");
    PT_SUN_PROBE = p_info::addPacket("SUN/PROBE");
    SunInitTclCode.load();

    return 0;
}

extern "C" int Cygsun_Init() {
    Sun_Init();
    
    return 0;
}
