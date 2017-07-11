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
 * @file   uwmulti-stack-controller-phy.cc
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of UwMultiStackControllerPhy class.
 *
 */

#include "uwmulti-stack-controller-phy.h"

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwMultiStackControllerPhyClass : public TclClass {
public:
  /**
   * Constructor of the class
   */
  UwMultiStackControllerPhyClass() : TclClass("Module/UW/MULTI_STACK_CONTROLLER_PHY") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
      return (new UwMultiStackControllerPhy);
  }
} class_uwmulti_stack_controller_phy;

map< UwMultiStackControllerPhy::UWPHY_CONTROLLER_STATE, string> UwMultiStackControllerPhy::state_info;

UwMultiStackControllerPhy::UwMultiStackControllerPhy() 
: 
UwMultiStackController(),
receiving_id(0),
current_state(UWPHY_CONTROLLER_STATE_IDLE)
{
  initInfo(); 
}

void UwMultiStackControllerPhy::initInfo() 
{
  state_info[UWPHY_CONTROLLER_STATE_IDLE] = "STATE_IDLE";
  state_info[UWPHY_CONTROLLER_STATE_BUSY_2_TX] = "STATE_BUSY_2_TX";
  state_info[UWPHY_CONTROLLER_STATE_BUSY_2_RX] = "STATE_BUSY_2_RX";
}

int UwMultiStackControllerPhy::command(int argc, const char*const* argv) 
{
  Tcl& tcl = Tcl::instance();
  if (argc == 2) 
  {
    if(strcasecmp(argv[1], "getStatus") == 0)
    {
      tcl.resultf("%d", (int)(current_state));
      return TCL_OK;
    }
	}
	
  return UwMultiStackController::command(argc, argv);     
} /* UwMultiStackControllerPhy::command */

int UwMultiStackControllerPhy::recvSyncClMsg(ClMessage* m) 
{
  int mac_addr = -1;
  ClMsgPhy2MacAddr msg;
  sendSyncClMsg(&msg);
  mac_addr = msg.getAddr();
  if (debug_)
  {
    std::cout << NOW << " ControllerPhy("<< mac_addr <<")::recvSyncClMsg(ClMessage* m), state_info: " 
              << state_info[current_state];
  }
  
  if (m->direction() == DOWN)//mac2phy something
  {
    if (debug_)
      std::cout << " direction = DOWN "<< std::endl;
    m->setDest(lower_id_active_);
    sendSyncClMsgDown(m);
    return 0;
  }
  else if (m->type() == CLMSG_PHY2MAC_STARTRX)
  {
    if (debug_)
      std::cout << " type = CLMSG_PHY2MAC_STARTRX "<< std::endl;
    if (current_state == UWPHY_CONTROLLER_STATE_IDLE)
    {
      stateBusy2Rx(m->getSource());
      sendSyncClMsgUp(m);
      return 0;
    }
    else
    {
      if (debug_)
      {
        std::cout <<"ControllerPhy("<< mac_addr <<")::recvSyncClMsg(ClMessage* m), nothing done."<<std::endl;
      }
      return 0;
    }
  }
  else 
  {
    if (m->type() == CLMSG_PHY2MAC_ENDTX && current_state == UWPHY_CONTROLLER_STATE_BUSY_2_TX)
    {
      if (debug_)
        std::cout << " endTx "<< std::endl;
      stateIdle();
    }
    else {
      if (debug_)
      {
        std::cout << " altro, ClMsg type = " << m->type() << std::endl;
      } 
    }
    
    sendSyncClMsgUp(m);
    return 0;
  }
}

void UwMultiStackControllerPhy::stateIdle() 
{
  int mac_addr = -1;
  ClMsgPhy2MacAddr msg;
  sendSyncClMsg(&msg);
  mac_addr = msg.getAddr();
  if (debug_)
  {
    std::cout << NOW << " ControllerPhy("<< mac_addr <<")::stateIdle(), state_info: " << state_info[current_state] 
              << std::endl;
  }
  current_state = UWPHY_CONTROLLER_STATE_IDLE;
  receiving_id = 0;
}

void UwMultiStackControllerPhy::stateBusy2Rx(int id) 
{
  int mac_addr = -1;
  ClMsgPhy2MacAddr msg;
  sendSyncClMsg(&msg);
  mac_addr = msg.getAddr();
  if (debug_)
  {
    std::cout << NOW << " ControllerPhy("<< mac_addr <<")::stateBusy2Rx(id), state_info: " 
              << state_info[current_state] << " phy_id = " << id << std::endl;
  }
  current_state = UWPHY_CONTROLLER_STATE_BUSY_2_RX;
  receiving_id = id;
}

void UwMultiStackControllerPhy::stateBusy2Tx(Packet *p) 
{
  int mac_addr = -1;
  ClMsgPhy2MacAddr msg;
  sendSyncClMsg(&msg);
  mac_addr = msg.getAddr();
  if (debug_)
  {
    std::cout << NOW << " ControllerPhy("<< mac_addr <<")::stateBusy2Tx(), state_info: " 
              << state_info[current_state] << std::endl;
  }
  assert(current_state == UWPHY_CONTROLLER_STATE_IDLE);
  current_state = UWPHY_CONTROLLER_STATE_BUSY_2_TX;
  recvFromUpperLayers(p);
}

void UwMultiStackControllerPhy::recv(Packet *p, int idSrc) 
{
  int mac_addr = -1;
  ClMsgPhy2MacAddr msg;
  sendSyncClMsg(&msg);
  mac_addr = msg.getAddr();
  hdr_cmn *ch = HDR_CMN(p);
  if (ch->direction() == hdr_cmn::DOWN && current_state == UWPHY_CONTROLLER_STATE_IDLE) 
  {
    //direction DOWN: packet is coming from upper layers
    if(debug_)
      std::cout << NOW << " ControllerPhy("<< mac_addr <<")::recv(Packet *p, int idSrc) FromUpperLayer idSrc = " 
                << idSrc << std::endl;
    stateBusy2Tx(p);
  }
  else if (current_state == UWPHY_CONTROLLER_STATE_BUSY_2_RX && idSrc == receiving_id) 
  {
    if(debug_)
      std::cout << NOW << " ControllerPhy("<< mac_addr <<")::recv(Packet *p, int idSrc) sendUp" 
                << std::endl;
    sendUp(p, min_delay_);
    stateIdle();
  }
  else {
    if (debug_)
      std::cout << NOW << " ControllerPhy("<< mac_addr <<")::recv(Packet *p, int idSrc) block direction = " 
              << ch->direction() << " idSrc = " << idSrc << std::endl;
    //Packet::free(p);?
  }
}