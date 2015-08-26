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
 * @file   uwoptical-mpropagation.h
 * @author Filippo Campagnaro, Federico Favaro, Federico Guerra
 * @version 1.0.0
 *
 * \brief Definition of UwOpticalMPropagation class.
 *
 */

#ifndef UWOPTICAL_MPROPAGATION_H 
#define UWOPTICAL_MPROPAGATION_H

#include <mpropagation.h>
#include <mphy.h>
#include <iostream>

/**
 * Class used to represents the UWOPTICAL_MPROPAGATION.
 */
class UwOpticalMPropagation : public MPropagation
{  
public:

  /**
   * Constructor of the UwOpticalMPropagation class
   */
  UwOpticalMPropagation();

  /**
   * Destructor of the UwOpticalMPropagation class
   */
  virtual ~UwOpticalMPropagation() { }

  /**
   * TCL command interpreter. It implements the following OTcl methods:
   * 
   * @param argc Number of arguments in <i>argv</i>.
   * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
   * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
   * 
   */
  virtual int command(int, const char*const*);
  
  /**
   * Calculate the gain following the Lambert and Beer's law
   *
   * @param Packet* Pointer to the packet that has to be received.
   * @return the gain due to the optical propagation.
   * 
   */
  virtual double getGain(Packet* p);

  int debug_;

protected:

  /**
   * Calculate the gain following the Lambert and Beer's law
   *
   * @param d Distance between transmitter and receiver.
   * @param angle Longitude angle between transmitter and receiver.
   * @return the gain due to the optical propagation.
   * 
   */
  virtual double getLambertBeerGain(double d, double angle);
  double Ar_; /**< Receiver area [m^2] */
  double At_; /**< Transmitter size [m^2] */
  double c_; /**< Beam light attenuation coefficient c = a + b [m^-1] */
  double theta_; /**< Transmitting beam diverge angle [rad] */
  bool omnidirectional_; /**< Flag to set whether the system is omnidirectional or not. By default it is false */
};



#endif /* UWOPTICAL_MPROPAGATION_H */
