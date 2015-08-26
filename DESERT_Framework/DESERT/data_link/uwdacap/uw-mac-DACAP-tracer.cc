//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
 * @file   uw-mac-DACAP-tracer.cc
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the implementation of DACAP Tracer
 *
 */


#include "uw-mac-DACAP-alter.h"
#include <mac.h>

enum {
  RTS_PKT = 3, CTS_PKT = 4, WRN_PKT = 5, DATA_PKT = 6, ACK_PKT = 7
};

/**
 * DACAP Tracer class
 */
class DACAPTracer : public Tracer
{
 public:
	/**
	 * Constructor of DACAPTracer class 
	 */
  DACAPTracer();
 protected:
	/**
	 * Format the output of the trace
	 * @param Packet* Pointer to the packet to trace
	 * @param SAP* Pointer to a the service access point where the trace is occurring
	 */
  void format(Packet *p, SAP* sap);
	/**
	 * Print the type of packet to trace 
	 * @param int type of packet 
	 * @return Pointer to the string of character that describe the packet 
	 */
  char* printType(int type);
};



DACAPTracer::DACAPTracer() : Tracer(4) {}



void DACAPTracer::format(Packet *p, SAP *sap)
{
  hdr_cmn* ch = hdr_cmn::access(p);
   
  if (ch->ptype() != PT_DACAP)
    return;

  hdr_dacap* dacaph = HDR_DACAP(p);
  hdr_mac* mach = HDR_MAC(p);

  writeTrace(sap, " MACSRC=%d MACDST=%d SN=%d TS=%f TP=%s SZ=%d",
               mach->macSA(),
               mach->macDA(),
               dacaph->sn,
	       dacaph->ts,
               printType(dacaph->dacap_type),
	       ch->size());
    
}

extern "C" int Uwmacdacaptracer_Init()
{  
  SAP::addTracer(new DACAPTracer);
  return 0;
}
extern "C" int  Cyguwmacdacaptracer_Init()
{
  Uwmacdacaptracer_Init();
}

char* DACAPTracer::printType(int type)
{
 if ( type == RTS_PKT) return "RTS";
 else if (type == CTS_PKT) return "CTS";
 else if (type == WRN_PKT) return "WRN";
 else if (type == ACK_PKT) return "ACK";
 else if (type == DATA_PKT) return "DAT";
 else return "ERR";

}

