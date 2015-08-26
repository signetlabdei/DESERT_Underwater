//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
//

/**
 * @file   uwtdma.cpp
 * @author Filippo Campagnaro
 * @version 1.0.0
 * 
 * \brief Provides the implementation of the class <i>UWTDMA</i>.
 * 
 */

#include "uwtdma.h"

#include <iostream>

#include <stdint.h>

static class TDMAModuleClass : public TclClass {
public:

	/**
	 * Constructor of the class
	*/
	TDMAModuleClass() : TclClass("Module/UW/TDMA") {}

	/**
	* Creates the TCL object needed for the tcl language interpretation
	* @return Pointer to an TclObject
	*/
	TclObject* create(int, const char*const*) {
		return (new UwTDMA());
	}

} class_uwtdma;

void UwTDMATimer::expire(Event *e) {
    ((UwTDMA *)module)->change_tdma_status();
}

UwTDMA::UwTDMA() : MMac(), tdma_timer(this) {
    bind("slot_status", &slot_status);
    bind("slot_duration", &slot_duration);
    bind("frame_time", &frame_time);
    bind("guard_time", &guard_time);
    bind("debug_", &debug_);
    channel_status = UW_CHANNEL_IDLE;
}

UwTDMA::~UwTDMA() {
}

int UwTDMA::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
 	if (argc==2){
    	if(strcasecmp(argv[1], "start") == 0){
	      start();
	      return TCL_OK;
	    }
	    else if(strcasecmp(argv[1], "stop") == 0){
	      tdma_timer.cancel();
	      return TCL_OK;
	    } 
	    else if (strcasecmp(argv[1], "get_buffer_size") == 0){
	    	tcl.resultf("%d", buffer.size());
		  	return TCL_OK;
		}
		else if (strcasecmp(argv[1], "get_upper_data_pkts_rx") == 0){
	    	tcl.resultf("%d", up_data_pkts_rx);
		  	return TCL_OK;
		}
  	}		
	else if (argc==3){
		if(strcasecmp(argv[1], "setSlotStatus") == 0){
     		slot_status=atoi(argv[2]);
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setSlotDuration") == 0){
			slot_duration=atof(argv[2]);
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}

void UwTDMA::recvFromUpperLayers(Packet* p) {
 	buffer.push(p);
	incrUpperDataRx();
  txData();
}

void UwTDMA::stateTxData(){
	channel_status=UW_CHANNEL_IDLE;
	txData();
}

void UwTDMA::txData(){
	if(slot_status==UW_TDMA_STATUS_MY_SLOT && channel_status==UW_CHANNEL_IDLE){
		if(buffer.size()>0){
	    Packet* p = buffer.front();
	    buffer.pop();
	    Mac2PhyStartTx(p);
	    incrDataPktsTx();
    }
  }
  else if(debug_){
   	if(slot_status!=UW_TDMA_STATUS_MY_SLOT)
   		std::cout << NOW << " UwTDMA(" << addr << ")::txData() " 
        << " Wait my slot to send " << std::endl;
  	else
      std::cout << NOW << " UwTDMA(" << addr << ")::txData() "
        << " Waiting earlier packet expires to send the current one " << std::endl;
	}
}

void UwTDMA::Mac2PhyStartTx(Packet* p)
{
	channel_status=UW_CHANNEL_BUSY;
  MMac::Mac2PhyStartTx(p);
  if(debug_<-5)
   	std::cout << NOW << " Send packet " << std::endl;
}

void UwTDMA::Phy2MacEndTx(const Packet* p)
{
	stateTxData();
}

void UwTDMA::Phy2MacStartRx(const Packet* p){
  channel_status=UW_CHANNEL_BUSY;
}

void UwTDMA::Phy2MacEndRx(Packet* p){
  incrDataPktsRx();
  sendUp(p);
  channel_status=UW_CHANNEL_IDLE;
  if(slot_status==UW_TDMA_STATUS_MY_SLOT){
    txData();
  }
}

void UwTDMA::change_tdma_status(){
	if(slot_status==UW_TDMA_STATUS_MY_SLOT){
	  tdma_timer.resched(frame_time-slot_duration+guard_time);
	  slot_status=UW_TDMA_STATUS_NOT_MY_SLOT;
	  if(debug_)
	    std::cout << NOW << " UwTDMA(" << addr << ")::change_tdma_status() Off "  
        << " for " << frame_time-slot_duration+guard_time << " s " << std::endl;
  	}
	else{
	  tdma_timer.resched(slot_duration-guard_time);
	  slot_status=UW_TDMA_STATUS_MY_SLOT;
	  if(debug_)
	    std::cout << NOW << " UwTDMA(" << addr << ")::change_tdma_status() On "
        << " for " << slot_duration-guard_time << "" << std::endl;
	  stateTxData();
  	}
}

void UwTDMA::start(){
	if(slot_status==UW_TDMA_STATUS_MY_SLOT)
		tdma_timer.resched(1); // go off
	else
		tdma_timer.resched(1+guard_time); // go on and start to transmit
	if(debug_)
		std::cout << NOW << " UwTDMA(" << addr << ")::start() "
    <<" status: " << slot_status << " " << std::endl;
}
