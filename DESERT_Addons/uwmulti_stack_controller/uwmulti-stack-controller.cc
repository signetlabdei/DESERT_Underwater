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
 * @file   uwmulti-stack-controller.cc
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of UwMultiStackController class.
 *
 */

#include "uwmulti-stack-controller.h"

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwMultiStackControllerClass : public TclClass 
{
public:
  /**
   * Constructor of the class
   */
  UwMultiStackControllerClass() : TclClass("Module/UW/MULTI_STACK_CONTROLLER") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) 
  {
    return (new UwMultiStackController);
  }
} class_stack_controller;

const double UwMultiStackController::threshold_not_exist = nan("");;
const int UwMultiStackController::layer_not_exist = -1;

UwMultiStackController::UwMultiStackController() 
: 
  Module(),
  debug_(0),
  min_delay_(0),
  switch_mode_(UW_MANUAL_SWITCH),
  lower_id_active_(0),
  signaling_pktSize_(1)
{
	bind("debug_", &debug_);
	bind("min_delay_", &min_delay_);
	bind("switch_mode_", (int*) &switch_mode_);
	bind("set_lower_id_active_", &lower_id_active_);
  bind("signaling_pktSize_", &signaling_pktSize_);
}

int UwMultiStackController::command(int argc, const char*const* argv) 
{
	if (argc == 2) 
  {
		if(strcasecmp(argv[1], "setAutomaticSwitch") == 0) 
    {
      switch_mode_ = UW_AUTOMATIC_SWITCH;
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setManualSwitch") == 0) 
    {
      switch_mode_ = UW_MANUAL_SWITCH;
			return TCL_OK;
		}
	}
	else if (argc == 3) 
  {
		if(strcasecmp(argv[1], "setManualLowerlId") == 0)
    {
      lower_id_active_ = atoi(argv[2]);
			return TCL_OK;
		}
	}
	else if (argc == 4) 
  {
    /**
     * parameters: layer_id, layer_order, a positive and
     * unique integer to order the set if physical ids
    */
		if(strcasecmp(argv[1], "addLayer") == 0)
    {
      addLayer(atoi(argv[2]),atoi(argv[3]));
			return TCL_OK;
		}
	}
  else if (argc == 5) 
  {
    /**
     * parameters: layer_id1, layer_id2, transition threshold
    */
    if(strcasecmp(argv[1], "addThreshold") == 0)
    {
      addThreshold(atoi(argv[2]),atoi(argv[3]),atof(argv[4]));
      return TCL_OK;
    }
  }
	
  return Module::command(argc, argv);     
} /* UwMultiStackController::command */

void UwMultiStackController::addLayer(int id, int order)
{
	assert(order > 0);
  id2order.erase(id);
  id2order.insert((std::pair<int,int>(id,order)));
  order2id.erase(order);
  order2id.insert((std::pair<int,int>(order,id)));
}

void UwMultiStackController::addThreshold(int i, int j, double thres_ij){
  assert (id2order.find(i) != id2order.end() && id2order.find(j) != id2order.end() && i!=j);
  setThreshold(i,j,thres_ij);
}

void UwMultiStackController::recv(Packet* p)
{
 	hdr_cmn *ch = HDR_CMN(p);
  	if(ch->direction() == hdr_cmn::UP)
    {
      sendUp(p, min_delay_);
    }
  	else
    {
      //direction DOWN: packet is coming from upper layers
      recvFromUpperLayers(p);
    }
}

void UwMultiStackController::recvFromUpperLayers(Packet *p)
{
	/*hdr_cmn *ch = HDR_CMN(p);*/

	if(switch_mode_ == UW_AUTOMATIC_SWITCH) /*&& ch->ptype() == CONTROLLED)*/
  {
		sendDown( getBestLayer(p), p, min_delay_);
	}
	else 
  {
		sendDown(lower_id_active_, p, min_delay_);
  }
}

bool UwMultiStackController::isLayerAvailable(int id)
{
	return id2order.find(id) != id2order.end();
}

double UwMultiStackController::getMetricFromSelectedLowerLayer(int id, Packet* p)
{
	ClMsgController m(id, p);
 	sendSyncClMsgDown(&m);
 	return m.getMetrics();
}

double UwMultiStackController::getThreshold(int i, int j) { 
  ThresMatrix::iterator it = threshold_map.find(i); 
  if (it != threshold_map.end()) {
    ThresMap thres_i = it->second;
    ThresMap::iterator it_thres_ij = thres_i.find(j);
    if(it_thres_ij != thres_i.end())
      return it_thres_ij->second;
    else
      return UwMultiStackController::threshold_not_exist;
  }
  return threshold_not_exist;
}

void UwMultiStackController::eraseThreshold(int i, int j) { 
  ThresMatrix::iterator it = threshold_map.find(i); 
  if (it != threshold_map.end()) {
    ThresMap thres_i = it->second;
    ThresMap::iterator it_thres_ij = thres_i.find(j);
    if(it_thres_ij != thres_i.end())
      thres_i.erase(j);
    if(thres_i.size() == 0)
      threshold_map.erase(i);
  }
}