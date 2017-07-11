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
 * @file   uwmulti-stack-controller-phy-master.h
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Definition of UwMultiStackControllerPhyMaster class.
 * This controller can switch from TWO layers thanks to the statistics collected.
 *
 */

#ifndef UWOPTICAL_ACOUSTIC_CONTROLLER_MASTER_H
#define UWOPTICAL_ACOUSTIC_CONTROLLER_MASTER_H

#include "uwmulti-stack-controller-phy.h"
#include <mac.h>
 
#include <limits>

class UwMultiStackControllerPhyMaster;

/**
 * Class used to represents the UwMultiStackControllerPhyMaster layer of a node.
 */
class UwMultiStackControllerPhyMaster : public UwMultiStackControllerPhy {

public:

  /**
   * Constructor of UwMultiPhy class.
   */
  UwMultiStackControllerPhyMaster();
  
  /**
   * Destructor of UwMultiPhy class.
   */
  virtual ~UwMultiStackControllerPhyMaster() { }
  
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
   * It manages each packet reception, either from the upper and the lower layer
   * 
   * @param p pointer to the packet will be received
   * @param idSrc unique id of the module that has sent the packet
   * 
   * @see SAP, ChSAP
  **/
  virtual void recv(Packet *p, int idSrc);
    
protected:
  // Internal classes
  class UwMultiStackSignalingTimer : public TimerHandler {
  public:

      UwMultiStackSignalingTimer(UwMultiStackControllerPhyMaster *m) : TimerHandler() {
          module = m;
      }

  protected:
      virtual void expire(Event *e);
      UwMultiStackControllerPhyMaster* module;
  };

  // Variables
  int signaling_active_; /**If true master is in signaling mode, otherwise not */
  int last_layer_used_; /**ID of the last PHY layer used */
  double power_statistics_; /** Average received power from the closest node according to the IIR filter*/
  int power_stat_node_; /** Address of the node which the master is collecting the average power*/
  double alpha_; /**< IIR parameter */
  double signaling_period_; /** Period to check the best layer if no interactions occurs*/
  int signaling_sent_; /** Number of signaling packets sent*/
  UwMultiStackSignalingTimer signaling_timer_; /**< UwMultiStack signaling timer handler */

  /** 
   * Return the best layer to forward the packet when the system works in AUTOMATIC_MODE.
   * It overloads in the extended classes to implement the choice rule.
   * 
   * @param p pointer to the packet
   *
   * @return id of the module representing the best layer
  */
  virtual int getBestLayer(Packet *p);

  /** 
   * If signaling is active, check the average power thresholds at each packet received
   *
  */
  virtual void checkAndSignal();

  /** 
   * If signaling is active and signaling timer expires, check the average power thresholds at each packet received
   *
  */
  virtual void resetCheckAndSignal();

  /** 
   * Check the average power thresholds
   *
  */
  virtual int checkBestLayer();

  /** 
   * Signals the best phy
   *
  */
  virtual void signalsBestPhy();

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
  * @see NodeCore, ClMessage, ClSAP, ClTracer, UwMultiStackControllerPhy
  **/
  int recvSyncClMsg(ClMessage* m);

  /** 
   * It implements the slave choice rule to choose the lower layer when the system works 
   * in AUTOMATIC_MODE. 
   * 
   * @param p pointer to the packet
   * @param idSrc unique id of the module that has sent the packet
  */
  virtual void updateMasterStatistics(Packet *p, int idSrc);
  
  /** 
   * Return the next layer in order which can achieve shorter range with higer bitrete.
   * 
   * @param layer_id id of the current layer
   *
   * @return id of the next layer in order
  */
  inline int getShorterRangeLayer(int layer_id){ return (getId(getOrder(layer_id) + 1) == UwMultiStackController::layer_not_exist)?
                                                 layer_id : getId(getOrder(layer_id) + 1);
	}
  
  /** 
   * Return the previous layer in order which can achieve longer range with lower bitrete.
   * 
   * @param layer_id id of the current layer
   *
   * @return id of the previous layer in order
  */
  inline int getLongerRangeLayer(int layer_id){ return (getId(getOrder(layer_id) - 1) == UwMultiStackController::layer_not_exist)?
                                                layer_id : getId(getOrder(layer_id) - 1);
	}

private:
    //Variables
};

#endif /* UWOPTICAL_ACOUSTIC_CONTROLLER_H  */
