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

/**
 * @file   uwmulti-stack-controller-phy-slave.h
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Definition of UwMultiStackControllerPhySlave class.
 *
 */

#ifndef UWOPTICAL_ACOUSTIC_CONTROLLER_PHY_SLAVE_H
#define UWOPTICAL_ACOUSTIC_CONTROLLER_PHY_SLAVE_H

#include "uwmulti-stack-controller-phy.h"
#include <mac.h>

/**
 * Class used to represents the UwMultiStackControllerPhySlave layer of a node.
 */
class UwMultiStackControllerPhySlave : public UwMultiStackControllerPhy {

public:
  
  /**
   * Constructor of UwMultiPhy class.
  **/ 
  UwMultiStackControllerPhySlave();
  
  /**
   * Destructor of UwMultiPhy class.
  **/
  virtual ~UwMultiStackControllerPhySlave() { }
    
  /**
   * TCL command interpreter. It implements the following OTcl methods:
   *
   * @param argc Number of arguments in <i>argv</i>.
   * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
   *
   * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
  **/
  virtual int command(int, const char*const*);

  /**
   * It manages each packet reception, either from the upper and the lower layer
   * 
   * @param p pointer to the packet will be received
   * @param idSrc unique id of the module that has sent the packet
   * 
   * @see SAP, ChSAP
  **/
  virtual void recv(Packet *p, int idSrc);

    
protected:
  // Variables
  int slave_lower_layer_;
  
  /** 
   * Return the best layer to forward the packet when the system works in AUTOMATIC_MODE.
   * It has to be overload in the extended classes to implement the choice rule.
   * 
   * @param p pointer to the packet
   *
   * @return id of the module representing the best layer
  **/
  virtual int getBestLayer(Packet *p);

  /** 
   * It implements the slave choice rule to choose the lower layer when the system works 
   * in AUTOMATIC_MODE. 
   * 
   * @param p pointer to the packet
   * @param idSrc unique id of the module that has sent the packet
  **/
  virtual void updateSlave(Packet *p, int idSrc);

private:
  //Variables
};

#endif /* UWOPTICAL_ACOUSTIC_CONTROLLER_H  */
