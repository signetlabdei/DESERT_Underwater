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
* @file   uwmulti-stack-controller.h
* @author Federico Favaro, Federico Guerra, Filippo Campagnaro
* @version 1.0.0
*
* \brief Definition of UwMultiStackController class.
*
*/

#ifndef UWMULTI_STACK_CONTROLLER_H
#define UWMULTI_STACK_CONTROLLER_H

#include <rng.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>
#include <map>

#include <iostream>
#include <string.h>
#include <cmath>
#include <climits>
#include "controller-clmsg.h"

typedef std::map <int, double> ThresMap; /**< Threshoold map <PHY_order, threshold>*/
typedef std::map <int, ThresMap> ThresMatrix; /**< Thresholds matrix*/

/**
 * Class used to represents the UwMultiStackController layer of a node.
 */
class UwMultiStackController : public Module {


public:

  // constant definitions
  static double const threshold_not_exist; /**< This constant is returned when a searched threshold does not exist>*/
  static int const layer_not_exist; /**< This constant is returned when a searched layer does not exist>*/

  /**
   * Constructor of UwMultiPhy class.
   */
  UwMultiStackController();

  /**
   * Destructor of UwMultiPhy class.
   */
  virtual ~UwMultiStackController() { }

  /**
   * TCL command interpreter. It implements the following OTcl methods:
   *
   * @param argc Number of arguments in <i>argv</i>.
   * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> 
   *             is the name of the object).
   *
   * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
   */
  virtual int command(int, const char*const*);

  /**
   * Add a layer in the layer map
   * 
   * @param id unique identifier of the module
   * @param order of the id in the controller logic
   */
  virtual void addLayer(int id, int order);

  /** 
   * set the threshold value for the transition from layer i to layer j, checking first whether
   * the layers exists 
   *
   * @param i id of the layer i
   * @param j id of the layer j
   * @param thres_ij threshold to pass from i to j
   */
  virtual void addThreshold(int i, int j, double thres_ij);

  /**
   * recv method. It is called when a packet is received from the other layers
   *
   * @param Packet* Pointer to the packet that are going to be received
   */
  virtual void recv(Packet *p);
  
  /** 
   * return the order of the id for the controller logic
   * 
   * @param id to select the layer 
   *
   * @return the order of the id
   */
  int inline getOrder(int layer_id) { return id2order.find(layer_id)==id2order.end() ? 
                                              UwMultiStackController::layer_not_exist :
                                              id2order.find(layer_id)->second; }
  
  /** 
   * return the id of the controlled layer given its order in the controller logic
   * 
   * @param order of the mapped layer
   *
   * @return the layer id
   */
  int inline getId(int layer_order) { return order2id.find(layer_order)==order2id.end() ? 
                                              UwMultiStackController::layer_not_exist :
                                              order2id.find(layer_order)->second; }
protected:
  // Variables
  /**< Switch modes >*/
  enum Mode
  {
    UW_MANUAL_SWITCH = 0, /**< State to switch-mode manually.*/
    UW_AUTOMATIC_SWITCH /**< State to switch-mode automatically.*/
  };

  int debug_; /**< Flag to activate debug verbosity.*/
  double min_delay_; 
  Mode switch_mode_; /** <Current switch mode (either AUTOMATIC or MANUAL).*/
  int lower_id_active_; /**< Id of the current lower layer active. It is used only in MANUAL MODE.*/
  std::map<int, int> id2order; /**< Maps each layer id to its order in the threshold matrix. (layer_id, order).*/
  ThresMatrix threshold_map; /**< Returns the switch layer theshold given a layer order.*/
  std::map<int, int> order2id; /**< Return the layer order given its order in the threshold matrix. (layer_order, layer_id).*/
  int signaling_pktSize_; /** By default the signaling is not employed, if it is needed, here where to set the signaling packet size*/
  /** 
   * Handle a packet coming from upper layers
   * 
   * @param p pointer to the packet
  */
  virtual void recvFromUpperLayers(Packet *p);

  /** 
   * Return the best layer to forward the packet when the system works in AUTOMATIC_MODE.
   * It has to be overloaded in the extended classes to implement the choice rule.
   *  
   * @param p pointer to the packet
   *
   * @return id of the module representing the best layer. ///@fgue what if there is no layer id active?
  */
  virtual inline int  getBestLayer(Packet *p) { assert(switch_mode_ == UW_AUTOMATIC_SWITCH); 
                                                return  lower_id_active_; }

  /** 
   * return if the specified layer, identified by id, is available
   * 
   * @param id unique identifier of the module 
   *
   * @return if the specified layer is available
   */
  virtual bool isLayerAvailable(int id); 

  /** 
   * return the new metrics value obtained from the selected lower layer,
   * in proactive way via ClMessage
   * 
   * @param id to select the lower layer 
   * @param p pointer to the packet 
   *
   * @return the value of the new value of the metrics obtained in proactive way ///@fgue what happens if the requested id is not present?
   */
  virtual double getMetricFromSelectedLowerLayer(int id, Packet* p);
  
  /** 
   * get the threshold value for the transition from layer i to layer j, checking first whether
   * the layers exists 
   *
   * @param i id of the layer i
   * @param j id of the layer j
   *
   * @return the threshold
   */
  virtual double getThreshold(int i, int j);
  
  /** 
   * remove the threshold value for the transition from layer i to layer j, checking first whether
   * the layers exists 
   *
   * @param i id of the layer i
   * @param j id of the layer j
   *
   */
  virtual void eraseThreshold(int i, int j);
  
  /** 
   * set the threshold value for the transition from layer i to layer j
   * 
   * @param i id of the layer i
   * @param j id of the layer j
   * @param thres_ij threshold to pass from i to j
   */
  void inline setThreshold(int i, int j, double thres_ij) { threshold_map[i][j] = thres_ij; }

private:
  //Variables
};

#endif /* UWMULTI_STACK_CONTROLLER_H  */
