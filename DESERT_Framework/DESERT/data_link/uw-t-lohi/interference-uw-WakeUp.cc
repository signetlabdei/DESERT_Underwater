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
 * @file   interference-uw-WakeUp.cc
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the implementations of the inteference model for the Tone channel.
 *
 */

#include "interference-uw-WakeUp.h"

// #include <iostream>
// #include <iomanip>
#include "mphy.h"

#define UW_WAKEUP_MODNAME "WKUP"

/**
 * Class that represents the binding with the tcl configuration script
 */
static class MInterferenceMIVWkUPClass : public TclClass {
public:
  /**
   * Constructor of the class
   */
  MInterferenceMIVWkUPClass() : TclClass("MInterference/MIV/WKUP") {}
  /**
   * Create the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
    return (new MInterfMivUwWakeUp);
  }
} class_minterference_miv_wkup;

MInterfMivUwWakeUp::MInterfMivUwWakeUp()
{

}


MInterfMivUwWakeUp::~MInterfMivUwWakeUp()
{

}

double MInterfMivUwWakeUp::getInterferencePower(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);
  if ( (ph->modulationType) != UW_WAKEUP_MODNAME) return (getInterferencePower(ph->Pr, ph->rxtime, ph->duration));  
  else return 0;
}
      