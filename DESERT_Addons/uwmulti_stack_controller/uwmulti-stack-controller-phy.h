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
* @file   uwmulti-stack-controller-phy.h
* @author Federico Favaro, Federico Guerra, Filippo Campagnaro
* @version 1.0.0
*
* \brief Definition of UwMultiStackControllerPhy class.
*
*/

#ifndef UWOPTICAL_ACOUSTIC_CONTROLLER_PHY_H
#define UWOPTICAL_ACOUSTIC_CONTROLLER_PHY_H

#include "uwmulti-stack-controller.h"
#include "phymac-clmsg.h"

#include <map>
#include <string>

class UwMultiStackControllerPhy : public UwMultiStackController {

public:

  /**
   * Constructor of UwMultiPhy class.
   */
  UwMultiStackControllerPhy();
  
  /**
   * Destructor of UwMultiPhy class.
   */
  virtual ~UwMultiStackControllerPhy() { }

  /**
   * TCL command interpreter. It implements the following OTcl methods:
   *
   * @param argc Number of arguments in <i>argv</i>.
   * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
   *
   * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
   */
  virtual int command(int, const char*const*);

  /**
  * Cross-Layer messages synchronous interpreter. It has to be properly extended in order to 
  * interpret custom cross-layer messages used by this particular plug-in.
  * This type of communication need to be directly answered in the message exchanged in 
  * order to be synchronous with the source.
  * 
  * @param m an instance of <i>ClMessage</i> that represent the message received and used for the answer
  *
  * @return zero if successful
  * 
  * @see NodeCore, ClMessage, ClSAP, ClTracer
  **/
  virtual int recvSyncClMsg(ClMessage* m);


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

  int receiving_id; /**< current receiving PHY ID */
  
  enum UWPHY_CONTROLLER_STATE 
  {
    UWPHY_CONTROLLER_STATE_IDLE = 1, 
    UWPHY_CONTROLLER_STATE_BUSY_2_TX, 
    UWPHY_CONTROLLER_STATE_BUSY_2_RX
  };
  
  UWPHY_CONTROLLER_STATE current_state;
  
  static map< UWPHY_CONTROLLER_STATE , string > state_info;

  /**
   * This function is used to initialize the UwMultiStackControllerPhy debugging info.
  */
  virtual void initInfo();

  /**
  * Node is in Idle state. It changes its state only when it has to manage 
  * a packet reception.
  */
  virtual void stateIdle();

  /**
  * Called when a node is receiving correctely a packet from the lower
  * layer. It sets the state busy due to the reception.
  *
  * @param id the identifier of the lower layer
  */
  virtual void stateBusy2Rx(int id);

  /**
  * Called when a node is transmitting a packet. The state is set 
  * busy due to the transmitting operation.
  *
  * @param p pointer to the packet will be received
  */
  virtual void stateBusy2Tx(Packet *p);

private:
  //Variables
};

#endif /* UWOPTICAL_ACOUSTIC_CONTROLLER_PHY_H  */
