/*
 * Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Padova (SIGNET lab) nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   uwoptical-channel.cc
 * @author Federico Guerra
 * @version 1.0.0
 *
 * \brief Implementation of UwOpticalChannel class.
 *
 */

#include <iostream>
#include <cassert>

#include "uwoptical-channel.h"

#define SPEED_OF_LIGHT_VACUUM (3e8)
#define REFRACTIVE_INDEX_MIN (1)
#define REFRACTIVE_INDEX_WATER (1.33)

/**
 * Adds the module for UwOpticalChannel in ns2.
 */
static class UwOpticalChannelClass : public TclClass {
public:
	UwOpticalChannelClass() : TclClass("Module/UW/Optical/Channel") {}
	TclObject* create(int, const char*const*) {
		return (new UwOpticalChannel());
	}
} class_uwoptical_channel_module;

UwOpticalChannel::UwOpticalChannel() 
:
  ChannelModule(),
  refractive_index(REFRACTIVE_INDEX_WATER),
  speed_of_light(SPEED_OF_LIGHT_VACUUM)
{
  bind("RefractiveIndex_", (double*)&refractive_index);

  if (refractive_index < REFRACTIVE_INDEX_MIN)
  {
    refractive_index = REFRACTIVE_INDEX_MIN;
  }

  speed_of_light /= refractive_index;
}

int UwOpticalChannel::command(int argc, const char*const* argv) {
    //Tcl& tcl = Tcl::instance();
    return ChannelModule::command(argc, argv);
}

void UwOpticalChannel::sendUpPhy(Packet *p,ChSAP *chsap)
{
  Scheduler &s = Scheduler::instance();
  struct hdr_cmn *hdr = HDR_CMN(p);
  
  hdr->direction() = hdr_cmn::UP;
  
  Position *sourcePos = chsap->getPosition();
  ChSAP *dest;

  if (debug_) cout << "UwOpticalChannel::sendUpPhy() sending packet" << endl;
  
  for (int i=0; i < getChSAPnum(); i++) 
  {
    dest = (ChSAP*)getChSAP(i);

    if (chsap == dest) // it's the source node -> skip it
      continue;

    s.schedule(dest,
         p->copy(), 
         getPropDelay(sourcePos, dest->getPosition()));
  }

  Packet::free(p);
}

void UwOpticalChannel::recv(Packet *p, ChSAP* chsap)
{
	sendUpPhy(p, chsap);
}

double UwOpticalChannel::getPropDelay(Position *src, Position* dst)
{
  double distance = src->getDist(dst);
  
  assert(distance >= 0.0);
  
  double delay = distance / speed_of_light;
  
  if (debug_) cout << "UwOpticalChannel::getPropDelay() distance = " << distance 
                   << "; speed = " << speed_of_light << "; delay = " << delay << endl;
  
  return delay;
}

