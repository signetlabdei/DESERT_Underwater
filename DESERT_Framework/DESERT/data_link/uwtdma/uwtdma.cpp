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
 * @file   uwtdma.h
 * @author Filippo Campagnaro
 * @author Roberto Francescon
 * @version 1.0.0
 * 
 * @brief Provides the implementation of the class <i>UWTDMA</i>.
 * 
 */

#include "uwtdma.h"
#include <iostream>
#include <stdint.h>
#include <mac.h>

/**
 * Class that represent the binding of the protocol with tcl
 */
static class TDMAModuleClass : public TclClass 
{

 public:

  /**
   * Constructor of the TDMAGenericModule class
   */
  TDMAModuleClass() : TclClass("Module/UW/TDMA"){}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*)
  {
    return (new UwTDMA());
  }

} class_uwtdma;

void UwTDMATimer::expire(Event *e)
{
  ((UwTDMA *)module)->changeStatus();
}

UwTDMA::UwTDMA() 
:
  MMac(), 
  tdma_timer(this), 
  slot_status(UW_TDMA_STATUS_NOT_MY_SLOT), 
  slot_duration(0),
  start_time(0),
  transceiver_status(IDLE),
  out_file_stats(0),
  guard_time(0),
  tot_slots(0)
{
  bind("frame_duration", (double*) &frame_duration);
  bind("debug_", (int*) &debug_);
  bind("sea_trial_", (int*) &sea_trial_);
  bind("fair_mode", (int*) &fair_mode);
  if (fair_mode == 1)
  {
    bind("guard_time", (double*) &guard_time);
    bind("tot_slots", (int*) &tot_slots);
  }
}

UwTDMA::~UwTDMA() {}

void UwTDMA::recvFromUpperLayers(Packet* p)
{
  initPkt(p);
  buffer.push(p);
  incrUpperDataRx();
  txData();
}

void UwTDMA::stateTxData()
{
  if (transceiver_status==TRANSMITTING)
    transceiver_status=IDLE;
  txData();
}

void UwTDMA::txData()
{
  if(slot_status==UW_TDMA_STATUS_MY_SLOT && transceiver_status==IDLE)
  {
    if(buffer.size()>0)
    {
      Packet* p = buffer.front();
      buffer.pop();
      Mac2PhyStartTx(p);
      incrDataPktsTx();
    }
  }
  else if(debug_<-5)
  {
    if(slot_status!=UW_TDMA_STATUS_MY_SLOT)
      std::cout << NOW << " ID " << addr << ": Wait my slot to send" 
                << std::endl;
    else
      std::cout << NOW << " ID " << addr 
                << ": Wait earlier packet expires to send the current one" 
                << std::endl;
  }
}

void UwTDMA::Mac2PhyStartTx(Packet* p)
{
  if (sea_trial_ != 1)
    assert(transceiver_status == IDLE);

  transceiver_status=TRANSMITTING;
  MMac::Mac2PhyStartTx(p);

  if(debug_<-5)
    std::cout << NOW <<" ID "<< addr << ": Sending packet" << std::endl;
  if(sea_trial_)
    out_file_stats << left << "[" << getEpoch() << "]::" << NOW 
                   << "::TDMA_node("<< addr << ")::PCK_SENT" << std::endl;

}

void UwTDMA::Phy2MacEndTx(const Packet* p)
{
  transceiver_status = IDLE; 
  txData();
}

void UwTDMA::Phy2MacStartRx(const Packet* p)
{
  if (sea_trial_ != 1)
    assert(transceiver_status != RECEIVING);

  if (transceiver_status == IDLE)
    transceiver_status=RECEIVING;
}

void UwTDMA::Phy2MacEndRx(Packet* p)
{
  if (transceiver_status != TRANSMITTING)
  {
    hdr_cmn* ch = HDR_CMN(p);
    hdr_mac* mach = HDR_MAC(p);
    int dest_mac = mach->macDA();
    int src_mac = mach->macSA();

    if (ch->error())
    {
      if (debug_) 
        cout << NOW << " TDMA(" << addr 
             << ")::Phy2MacEndRx() dropping corrupted pkt " << std::endl;
    
      incrErrorPktsRx();
      Packet::free(p);
    }
    else 
    {  
      if ( dest_mac != addr && dest_mac != MAC_BROADCAST ) 
      {
        rxPacketNotForMe(p);

        if (debug_<-5)
          std::cout << NOW << " ID " << addr << ": packet was for " << dest_mac
                    << std::endl;
      }
      else 
      {
        sendUp(p);
        incrDataPktsRx();

        if (debug_<-5)
          std::cout << NOW <<" ID "<< addr << ": Received packet from "
	            << src_mac << std::endl;
        if(sea_trial_)
          out_file_stats << left << "[" << getEpoch() << "]::" << NOW 
                         << "::TDMA_node("<< addr << ")::PCK_FROM:" 
                         <<  src_mac << std::endl;
      }
    }

    transceiver_status=IDLE;

    if(slot_status==UW_TDMA_STATUS_MY_SLOT)
      txData();

  }
  else 
  {
    Packet::free(p);
    if (debug_)
      std::cout << NOW <<" ID "<< addr <<": Received packet while transmitting "
                << std::endl;
    if(sea_trial_)
      out_file_stats << left << "[" << getEpoch() << "]::" << NOW 
		     << "::TDMA_node("<< addr << ")::RCVD_PCK_WHILE_TX" 
		     << std::endl;
  }
}

void UwTDMA::initPkt(Packet* p)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_mac* mach = HDR_MAC(p);

  int curr_size = ch->size();

  ch->size() = curr_size + HDR_size;
}

void UwTDMA::rxPacketNotForMe(Packet* p)
{
  if ( p != NULL ) 
    Packet::free(p);
}

void UwTDMA::changeStatus()
{
  if(slot_status==UW_TDMA_STATUS_MY_SLOT)
  {
    slot_status=UW_TDMA_STATUS_NOT_MY_SLOT;
    tdma_timer.resched(frame_duration-slot_duration+guard_time);

    if(debug_<-5)
      std::cout << NOW << " Off ID " << addr << " " 
                << frame_duration-slot_duration+guard_time << "" << std::endl;
    if(sea_trial_)
      out_file_stats << left << "[" << getEpoch() << "]::" << NOW 
                     << "::TDMA_node("<< addr << ")::Off" << std::endl;
  } 
  else 
  {
    slot_status=UW_TDMA_STATUS_MY_SLOT;
    tdma_timer.resched(slot_duration-guard_time);

    if(debug_<-5)
      std::cout << NOW << " On ID " << addr << " " << slot_duration-guard_time 
                << " " << std::endl;
    if(sea_trial_)
      out_file_stats << left << "[" << getEpoch() << "]::" << NOW 
                     << "::TDMA_node("<< addr << ")::On" << std::endl;

    stateTxData();
  }
}

void UwTDMA::start(double delay)
{
  if(sea_trial_)  
  {
    std::stringstream stat_file;
    stat_file << "./TDMA_node_" << addr << ".out";
    std::cout << stat_file.str().c_str() << std::endl;
    out_file_stats.open(stat_file.str().c_str(), std::ios_base::app);

    out_file_stats << left << "[" << getEpoch() << "]::" << NOW 
                   << "::TDMA_node("<< addr << ")::Start_simulation" 
                   << std::endl;
  }

  tdma_timer.sched(delay);

  if(debug_<-5)
    std::cout << NOW << " Status " << slot_status << " on ID " << addr 
  	      << " " << std::endl;
}

void UwTDMA::stop()
{
  tdma_timer.cancel();
  if(sea_trial_)
      out_file_stats << left << "[" << getEpoch() << "]::" << NOW 
                     << "::TDMA_node("<< addr << ")::Terminate_simulation" 
                     << std::endl;
}

int UwTDMA::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if (argc==2)
  {
    if(strcasecmp(argv[1], "start") == 0)
    {
      if (fair_mode == 1)
      {
        if (tot_slots==0)
        {
          std::cout<<"Error: number of slots set to 0"<<std::endl;
          return TCL_ERROR;
        }
        else 
        {
          slot_duration = frame_duration/tot_slots;
          if (slot_duration - guard_time < 0)
	      {
	        std::cout<<"Error: guard time or frame set incorrectly"<<std::endl; 
            return TCL_ERROR;
          }
          else
	      {
            start(slot_number*slot_duration);
            return TCL_OK;
          }
        }
      }
      
      start(start_time);
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "stop") == 0)
    {
      stop();
      return TCL_OK;
    } 
    else if (strcasecmp(argv[1], "get_buffer_size") == 0)
    {
      tcl.resultf("%d", buffer.size());
      return TCL_OK;
    }
    else if (strcasecmp(argv[1], "get_upper_data_pkts_rx") == 0)
    {
      tcl.resultf("%d", up_data_pkts_rx);
      return TCL_OK;
    }
    else if (strcasecmp(argv[1], "get_sent_pkts") == 0)
    {
      tcl.resultf("%d", data_pkts_tx);
      return TCL_OK;
    }
    else if (strcasecmp(argv[1], "get_recv_pkts") == 0)
    {
      tcl.resultf("%d", data_pkts_rx);
      return TCL_OK;
    }
  }		
  else if (argc==3)
  {
    if(strcasecmp(argv[1], "setStartTime") == 0)
    {
      start_time=atof(argv[2]);
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "setSlotDuration") == 0)
    {
      if (fair_mode==1)
      {
	std::cout<<"Fair mode is being used! Change to generic TDMA"<<std::endl;
        return TCL_ERROR;
      }
      else
      {
        slot_duration=atof(argv[2]);
        return TCL_OK;
      }
    }
    else if(strcasecmp(argv[1], "setGuardTime") == 0)
    {
    if (fair_mode==1)
      {
	std::cout<<"Fair mode is being used! Change to generic TDMA"<<std::endl;
        return TCL_ERROR;
      }
      else
      {
        guard_time=atof(argv[2]);
        return TCL_OK;
      }
    }
    else if(strcasecmp(argv[1], "setSlotNumber") == 0)
    {
      slot_number=atoi(argv[2]);
      return TCL_OK;
    }
    else if(strcasecmp(argv[1],"setMacAddr") == 0)
    {
      addr = atoi(argv[2]);
      if(debug_)  cout << "TDMA MAC address of current node is " 
		       << addr << std::endl;
      return TCL_OK;
    }
  }
  return MMac::command(argc, argv);
}

