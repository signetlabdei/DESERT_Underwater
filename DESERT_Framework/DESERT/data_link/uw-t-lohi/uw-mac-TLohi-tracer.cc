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
//

/**
 * @file   uw-mac-TLohi-tracer.cc
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the declaration and the implementation of a ns-2 tracer for T-LOHI protocol
 *
 */


#include "uw-mac-TLohi.h"
#include "wake-up-pkt-hdr.h"
#include <mac.h>

/**
 * Class that represents a Tracer for T-LOHI protocol 
 */
class TLohiTracer : public Tracer
{
 public:
	/**
	 * Constructor of the class
	 */
  TLohiTracer();
 protected:
	/**
	 * Format the output for the tracer
	 * @param Packet* pointer to the packet to trace
	 * @param SAP* pointer to the Service Access Point to trace
	 */
  void format(Packet *p, SAP* sap);
	/**
	 * Print the type of packet
	 * @param TLOHI_PKT_TYPE type of the packet
	 * @return char* pointer to the description of the packet 
	 */
  char* printType(TLOHI_PKT_TYPE type);
};



TLohiTracer::TLohiTracer() : Tracer(4) {}



void TLohiTracer::format(Packet *p, SAP *sap)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_mac* mach = HDR_MAC(p);

  if (ch->ptype() == PT_TLOHI) {
      hdr_tlohi* tlohih = HDR_TLOHI(p);

      writeTrace(sap, " MACSRC=%d MACDST=%d SN=%d TS=%f TP=%s SZ=%d",
               mach->macSA(),
               mach->macDA(),
               tlohih->sn,
	       tlohih->ts,
               printType(tlohih->pkt_type),
	       ch->size());
  }

  else if(ch->ptype() == PT_WKUP) {
      writeTrace(sap, " TONE");
  }

  else return;
}

extern "C" int Uwmactlohitracer_Init()
{  
  SAP::addTracer(new TLohiTracer);
  return 0;
}
extern "C" int  Cyguwmactlohitracer_Init()
{
  Uwmactlohitracer_Init();
}

char* TLohiTracer::printType(TLOHI_PKT_TYPE type)
{
 if ( type == DATA_PKT) return "DAT";
 else if (type == ACK_PKT) return "ACK";
 else return "ERR";
}

