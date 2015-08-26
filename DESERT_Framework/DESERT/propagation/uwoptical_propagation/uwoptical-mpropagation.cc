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
 * @file   uwoptical-propagation.cc
 * @author Filippo Campagnaro, Federico Favaro, Federico Guerra
 * @version 1.0.0
 *
 * \brief Implementation of UwOpticalMPropagation class.
 *
 */

#include <node-core.h>
#include "uwoptical-mpropagation.h"
#include <math.h>


/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwOpticalMPropagationClass : public TclClass {
public:

  /**
   * Constructor of the class
   */
  UwOpticalMPropagationClass() : TclClass("Module/UW/OPTICAL/Propagation") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
    return (new UwOpticalMPropagation);
  }
} class_UwOpticalMPropagation;


UwOpticalMPropagation::UwOpticalMPropagation()
:
Ar_(1),
At_(1),
c_(0),
theta_(0),
omnidirectional_(false)
{
  /*bind_error("token_separator_", &token_separator_);*/
  bind("Ar_", &Ar_);
  bind("At_", &At_);
  bind("c_", &c_);
  bind("theta_", &theta_);
  bind("debug_", &debug_);
}

int UwOpticalMPropagation::command(int argc, const char*const* argv){
  if (argc == 2) {
    if (strcasecmp(argv[1], "setOmnidirectional") == 0) {
      omnidirectional_ = true;
      return TCL_OK;
    } 
    else if (strcasecmp(argv[1], "setDirectional") == 0) {
      omnidirectional_ = false;
      return TCL_OK;
    } 
  }
  else if (argc == 3) {
    if (strcasecmp(argv[1], "setAr") == 0) {
      Ar_ = strtod(argv[2], NULL);
      return TCL_OK;
    } 
    else if (strcasecmp(argv[1], "setAt") == 0) {
      At_ = strtod(argv[2], NULL);
      return TCL_OK;
    }
    else if (strcasecmp(argv[1], "setC") == 0) {
      c_ = strtod(argv[2], NULL);
      return TCL_OK;
    }
    else if (strcasecmp(argv[1], "setTheta") == 0) {
      theta_ = strtod(argv[2], NULL);
      return TCL_OK;
    }
  }
  return MPropagation::command(argc, argv);
}

double UwOpticalMPropagation::getGain(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);
  Position* sp = ph->srcPosition;
  Position* rp = ph->dstPosition;
  assert(sp);
  assert(rp);
  double dist = sp->getDist(rp);
  double beta = sp->getZ() == rp->getZ() ? 0 : M_PI/2 - abs(sp->getRelZenith(rp));
  double PCgain=getLambertBeerGain(dist,beta);
  if (debug_)
    std::cout << NOW << " UwOpticalMPropagation::getGain()" << " dist="
              << dist << " gain=" << PCgain << std::endl;

  return PCgain;
}

double UwOpticalMPropagation::getLambertBeerGain(double d, double beta){
  double cosBeta = omnidirectional_ ? 1 : cos(beta);
  double L = d / cosBeta;
  double PCgain = 2 * Ar_ * cosBeta / (M_PI * pow(L, 2.0) * (1 - cos(theta_)) + 2 * At_) 
         * exp(-c_*d);
  return (PCgain == PCgain) ? PCgain : 0;
}
