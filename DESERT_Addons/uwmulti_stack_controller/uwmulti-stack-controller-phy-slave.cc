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
 * @file   uwmulti-stack-controller-phy-slave.cc
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of UwMultiStackControllerPhySlave class.
 *
 */

#include "uwmulti-stack-controller-phy-slave.h"

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwMultiStackControllerPhySlaveClass : public TclClass {
public:
  /**
   * Constructor of the class
   */
  UwMultiStackControllerPhySlaveClass() : TclClass("Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
    return (new UwMultiStackControllerPhySlave);
  }
} class_uwmulti_stack_controller_phy_slave;

UwMultiStackControllerPhySlave::UwMultiStackControllerPhySlave() 
: 
  UwMultiStackControllerPhy()
{ 
  
}

int UwMultiStackControllerPhySlave::command(int argc, const char*const* argv) 
{
  Tcl& tcl = Tcl::instance();
  if (argc == 3) 
  {
    if(strcasecmp(argv[1], "setManualLowerlId") == 0)
    {
      lower_id_active_ = atoi(argv[2]);
      slave_lower_layer_ = lower_id_active_; // first initialization of the automatic slave_lower_layer
      return TCL_OK;
    }
  }
  return UwMultiStackControllerPhy::command(argc, argv);     
} /* UwMultiStackControllerPhySlave::command */

void UwMultiStackControllerPhySlave::recv(Packet *p, int idSrc)
{
  updateSlave(p,idSrc);
  UwMultiStackControllerPhy::recv(p, idSrc);
}

int UwMultiStackControllerPhySlave::getBestLayer(Packet *p) { 
  assert(switch_mode_ == UW_AUTOMATIC_SWITCH);

  int mac_addr = -1;
  ClMsgPhy2MacAddr msg;
  sendSyncClMsg(&msg);
  mac_addr = msg.getAddr();

  if (debug_)
  {
    std::cout << NOW << " ControllerPhySlave("<< mac_addr <<")::getBestLayer(Packet *p) "<< std::endl;
  }
  lower_id_active_ = slave_lower_layer_; 

  return  slave_lower_layer_; 
  }

void UwMultiStackControllerPhySlave::updateSlave(Packet *p, int idSrc)
{
  int mac_addr = -1;
  hdr_mac* mach = HDR_MAC(p);
  ClMsgPhy2MacAddr msg;
  sendSyncClMsg(&msg);
  mac_addr = msg.getAddr();
  if (mach->macDA() == mac_addr || mach->macDA() == MAC_BROADCAST)
  {
    if (debug_)
    {
      std::cout << NOW << " ControllerPhySlave("<< mac_addr <<")::updateSlave " 
                << msg.getAddr() << ": " << slave_lower_layer_ << " --> " << idSrc << std::endl;
    }
    slave_lower_layer_ = idSrc;
  }
}
