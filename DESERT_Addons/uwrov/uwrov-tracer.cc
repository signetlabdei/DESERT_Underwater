//
// Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwrov-tracer.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 * 
 * \brief Provides a tracer class for <i>UWROV</i> packets.
 * 
 * Provides a tracer class for <i>UWROV</i> packets.
 */


#include "uwrov-module.h"

/**
 * Class that defines a tracer for <i>hdr_uwROV_monitoring</i> packets.
 */
class UWROVTracer : public Tracer {
public:
    UWROVTracer();
protected:
    void format(Packet *p, SAP* sap);
};

UWROVTracer::UWROVTracer() : Tracer(4) {
}

void UWROVTracer::format(Packet *p, SAP *sap) {
    hdr_cmn *ch = hdr_cmn::access(p);

    if (ch->ptype() != PT_UWCBR)
        return;
}

extern "C" int UwROVtracer_Init() {
    SAP::addTracer(new UWROVTracer);
    return 0;
}

extern "C" int CyguwROVtracer_Init() {
    UwROVtracer_Init();
    return 0;
}


