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
* @file   uwmulti-stack-Control.h
* @author Filippo Campagnaro, Federico Guerra
* @version 1.0.0
*
* \brief Definition of UwMultiTrafficControl class.
*
*/

#ifndef UWMULTI_TRAFFIC_Control_H
#define UWMULTI_TRAFFIC_Control_H

#include <rng.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>
#include <map>
#include <queue>
#include <iostream>
#include <string.h>
#include <cmath>
#include <climits>

// DEFINE BEHAVIORS
#define DEFAULT 1

// DEFINE STATES

typedef std::map <int, int> UpTrafficMap; /**< app_type, module_id>*/
typedef std::pair <int, int> BehaviorItem; /**< module_id, behavior>*/
typedef std::map <int, BehaviorItem> BehaviorMap; /**< stack_id, behavior>*/
typedef std::map <int, BehaviorMap> DownTrafficMap; /**< app_type, BehaviorMap*/
typedef std::queue<Packet*> Buffer;
typedef std::map <int, Buffer*> DownTrafficBuffer; /**< app_type, PacketQueue*/

/**
 * Class used to represents the UwMultiTrafficControl layer of a node.
 */
class UwMultiTrafficControl : public Module {


public:

  /**
   * Constructor of UwMultiPhy class.
   */
  UwMultiTrafficControl();

  /**
   * Destructor of UwMultiPhy class.
   */
  virtual ~UwMultiTrafficControl() { }

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
   * recv method. It is called when a packet is received from the other layers
   *
   * @param Packet* Pointer to the packet that are going to be received
   */
  virtual void recv(Packet *p);

protected:

  int debug_; /**< Flag to activate debug verbosity.*/
  UpTrafficMap up_map; /**< Map of upper layers.*/
  DownTrafficMap down_map; /**< Map of lower layers.*/
  DownTrafficBuffer down_buffer; /**< Map of buffer per traffic types*/
  
  /** 
   * Handle a packet coming from upper layers
   * 
   * @param p pointer to the packet
  */
  virtual void recvFromUpperLayers(Packet *p);

  /** 
   * manage to tx a packet of traffic type
   *
   * @param traffic application traffic id
   *
   */
  virtual void manageBuffer(int traffic);

  /** 
   * insert a packet of a certain type in the buffer
   *
   * @param p pointer of the packet
   * @param traffic application traffic id
   */
  virtual void insertInBuffer(Packet *p, int traffic);

  /** 
   * remove a packet of a certain type from the buffer
   * and return it
   *
   * @param traffic application traffic id
   * @return the pointer of the removed packet
   */
  virtual Packet * removeFromBuffer(int traffic);

  /** 
   * get a packet of a certain type from the buffer
   * and return it
   *
   * @param traffic application traffic id
   * @return the pointer of the removed packet
   */
  virtual Packet * getFromBuffer(int traffic);

  /**
   * Set to which upper layer forward a specific kind of traffic received from the lower layers
   * 
   * @param traffic application traffic id
   * @param upper_layer_id unique identifier of the upper layer id
   */
  void inline insertTraffic2UpLayer(int traffic, int upper_layer_id) { 
    up_map[traffic] = upper_layer_id; 
  }

  /**
   * Set to which upper layer forward a specific kind of traffic received from the lower layers
   * 
   * @param application traffic id
   * @param lower_layer_stack unique identifier of the lower layer stack
   * @param check_range if <i>TRUE</i> follows the CHECK_RANGE behavior, else the ROBUST one
   */
  void inline insertTraffic2LowerLayer(int traffic, int lower_layer_stack, int lower_layer_id, int behavior) { 
    down_map[traffic][lower_layer_stack] = std::make_pair(lower_layer_id, behavior); 
    //std::cout << "down_map(" << traffic << ", " << lower_layer_stack << ") = " << (down_map[traffic][lower_layer_stack]).first << std::endl;
  }

  /** 
   * return the Best Lower Layer id where to forward the packet of <i>traffic</i> type
   * 
   * @param traffic application traffic id
   *
   * @return the layer id
   */
  virtual int getBestLowerLayer(int traffic, Packet *p= NULL);

   /** 
   * return the Upper Layer id where to forward the received packet of <i>traffic</i> type
   * 
   * @param traffic application traffic id
   *
   * @return the layer id
   */
  virtual int getUpperLayer(int traffic);
  
  /** 
   * remove the behavior from the traffic lower layers matrix
   *
   * @param traffic application traffic id
   * @param lower_layer_stack lower layer stack id
   *
   */
  virtual void eraseTraffic2LowerLayer(int traffic, int lower_layer_stack);
  
  /** 
   * remove the traffic from the lower layers matrix
   *
   * @param traffic application traffic id
   *
   */
  virtual void eraseTraffic2Low(int traffic);
  
  /** 
   * remove the traffic from the upper layers matrix
   *
   * @param traffic application traffic id
   *
   */
  virtual void eraseTraffic2Up(int traffic);

  /** 
   * add an upper layer from the tag name in the map
   *
   * @param traffic application traffic id
   * @param tag: tag of the upper module
   */
  virtual void addUpLayerFromTag(int traffic_id, std::string tag);

  /** 
   * add a lower layer from the tag name in the map
   *
   * @param traffic application traffic id
   * @param tag: tag of the lower module
   * @param behavior: behavior of the layer
   */
  virtual void addLowLayerFromTag(int traffic_id, std::string tag, int behavior);

private:
  //Variables
};

#endif /* UWMULTI_TRAFFIC_Control_H  */
