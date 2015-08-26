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
 * @file   uwoptical-channel.h
 * @author Federico Guerra
 * @version 1.0.0
 *
 * \brief Provides class and APIs of the underwater optical channel
 *
 */

#ifndef UW_OPTICAL_CHANNEL_H
#define UW_OPTICAL_CHANNEL_H

#include <channel-module.h>
#include <stdlib.h>
#include <tclcl.h>

/**
 * UwOpticalChannel extends Miracle channel class and implements the underwater optical channel
 */
class  UwOpticalChannel : public ChannelModule
{

public:

  /**
  * Constructor of UwOpticalChannel class.
  */
  UwOpticalChannel();

  /**
  * Destructor of UwOpticalChannel class.
  */
  virtual ~ UwOpticalChannel() { }
  
  /**
  * Performs the reception of packets from upper and lower layers.
  * 
  * @param Packet* Pointer to the packet received.
  * @param ChSAP* Pointer to the channel Service Access Point (SAP)
  */
  virtual void recv(Packet *p, ChSAP *chsap);
  
  /**
  * TCL command interpreter. It implements the following OTcl methods:
  * 
  * @param argc Number of arguments in <i>argv</i>.
  * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
  * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
  * 
  */
  virtual int command(int argc, const char*const* argv);

protected:
  
  /**
  * Returns the underwater optical propagation delay between the two given Position pointers.
  * @param Position* s pointer to the first Position object 
  * @param Position* d pointer to the second Position object
  * @return underwater optical propagation delay [s]
  */
  virtual double getPropDelay(Position *s, Position* d);
  
  /**
  * Sends the given Packet* to the upper PHY layer
  * 
  * @param Packet* Pointer to the packet received.
  * @param ChSAP* Pointer to the channel Service Access Point (SAP)
  */
  void sendUpPhy(Packet *p,ChSAP *chsap);
  
  double refractive_index; /**< refractive index of the underwater medium. */
  double speed_of_light; /**< Speed of light in the underwater medium. */
};

#endif /* UW_OPTICAL_CHANNEL_H */
