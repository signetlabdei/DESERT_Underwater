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
 * @file   uw-mac-DACAP-alter.cpp
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the implementation of DACAP class
 *
 */

#include "uw-mac-DACAP-alter.h"
#include <mac.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <rng.h>
//#include <ip-clmsg.h>
//#include <mll-clmsg.h>
#include <stdio.h>
#include <stdlib.h>

static const double prop_speed = 1500.0;

/**
 * Enumeration that represent the possible state of the protocol
 */
enum {
  NOT_SET = -1, NO_ACK_MODE = 1, ACK_MODE, RTS_PKT, CTS_PKT, WRN_PKT, DATA_PKT, ACK_PKT,
  STATE_IDLE, STATE_WAIT_CTS, 
  STATE_DEFER_DATA, STATE_SEND_DATA, STATE_WAIT_ACK, STATE_BACKOFF, 
  STATE_CTS_RECEIVED, STATE_WAIT_DATA, STATE_DATA_RECEIVED, 
  STATE_SEND_ACK, STATE_SEND_RTS, STATE_SEND_CTS, STATE_WAIT_WRN, STATE_SEND_WRN,
  STATE_WAIT_XCTS, STATE_WAIT_XDATA, STATE_WAIT_XACK, STATE_WAIT_XWRN, STATE_RECONTEND_WINDOW,
  REASON_DEFER, REASON_NOACK, REASON_NODATA, REASON_NOCTS, REASON_CTS_RECEIVED,
  REASON_ACK_RECEIVED, REASON_RTS_RECEIVED, REASON_DATA_RECEIVED, REASON_WRN_RECEIVED,
  REASON_BACKOFF_END, REASON_DEFER_END, REASON_DATA_SENT, REASON_ACK_SENT, 
  REASON_BACKOFF_PENDING, REASON_RTS_SENT, REASON_CTS_SENT, REASON_INTERFERENCE,
  REASON_TX_ENDED, REASON_DATA_PENDING, REASON_NOWRN , REASON_WRN_END, REASON_SAME_RTS_RECEIVED,
  REASON_MAX_TX_TRIES, REASON_XACK_END, REASON_WAIT_RECONTEND_END, REASON_XCTS_END, REASON_XDATA_END, 
  REASON_XRTS_RX, REASON_XCTS_RX, REASON_XDATA_RX, REASON_XACK_RX, REASON_WAIT_XWRN_END 
};



extern packet_t PT_DACAP;

/**
 * Class that represent the binding of the protocol with tcl
 */
static class DACAPModuleClass : public TclClass {
public:
  /**
   * Constructor of the class 
   */
  DACAPModuleClass() : TclClass("Module/UW/DACAP") {}
  TclObject* create(int, const char*const*) {
    return (new MMacDACAP());
  }
} class_module_dacap;

void DACAPBTimer::expire(Event* e)
{
  if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") backoff expired, current state = " 
                           << module->info[module->curr_state] << endl;

  module->last_reason = REASON_BACKOFF_END;
  module->exitBackoff();

  if (module->curr_state == STATE_BACKOFF) {

     if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") next state = " 
                           << module->info[STATE_IDLE] << endl;

     module->stateIdle();
  }
}

void DACAPTimer::expire(Event *e)
{
  switch (module->curr_state) {

    case(STATE_RECONTEND_WINDOW): {
                                    
        
        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                         << module->info[module->curr_state] << " Recontend Window expired, next state = " 
                         << module->info[STATE_IDLE] << endl;

        module->last_reason = REASON_WAIT_RECONTEND_END;
        module->stateIdle(); 
    }
    break;

    case(STATE_WAIT_CTS): { 
        
        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                         << module->info[module->curr_state] << " handshake not completed, next state = " 
                         << module->info[STATE_BACKOFF] << endl;

        module->last_reason = REASON_NOCTS;
        module->stateBackoff(); 
    }
    break;

    case(STATE_WAIT_WRN): { 

        if (module->defer_data == false) {     
   
           if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                                    << module->info[module->curr_state] << " warning not received, next state = " 
                                    << module->info[STATE_SEND_DATA] << endl;

           module->last_reason = REASON_NOWRN;
           module->stateSendData();
        }
        else {
 
           if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                                    << module->info[module->curr_state] << " warning received, next state = " 
                                    << module->info[STATE_DEFER_DATA] << endl;

           module->last_reason = REASON_WRN_RECEIVED;
           module->stateDeferData();          
        } 
    }
    break;

    case(STATE_DEFER_DATA): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                         << module->info[module->curr_state] << " defer data complete, next state = " 
                         << module->info[STATE_SEND_DATA] << endl;

        module->last_reason = REASON_DEFER_END;
        module->stateSendData();

    }
    break;

    case(STATE_WAIT_ACK): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                                 << module->info[module->curr_state] << " ack not received, next state = " 
                                 << module->info[STATE_BACKOFF] << endl;

        module->last_reason = REASON_NOACK;
        module->stateBackoff();
    }
    break;

    case(STATE_BACKOFF): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                                 << module->info[module->curr_state] << " backoff expired, next state = " 
                                 << module->info[STATE_IDLE] << endl;

        module->last_reason = REASON_BACKOFF_END;
        module->exitBackoff();
        module->stateIdle();
    }
    break;

    case(STATE_WAIT_DATA): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                                 << module->info[module->curr_state] << " data not received, next state = " 
                                 << module->info[STATE_IDLE] << endl;

        module->last_reason = REASON_NODATA;
        module->stateIdle();
    }
    break;

    case(STATE_SEND_WRN): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                         << module->info[module->curr_state] << " send WRN Window ended, next state = " 
                         << module->info[STATE_WAIT_DATA] << endl;

        module->last_reason = REASON_WRN_END;
        module->stateWaitData();
    }
    break;

   case(STATE_WAIT_XCTS): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                         << module->info[module->curr_state] << " , next state = " 
                         << module->info[STATE_WAIT_XWRN] << endl;

        module->last_reason = REASON_XCTS_END;
        module->stateWaitXWarning();
    }
    break;

   case(STATE_WAIT_XWRN): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                         << module->info[module->curr_state] << " , next state = " 
                         << module->info[STATE_WAIT_XDATA] << endl;

        module->last_reason = REASON_WAIT_XWRN_END;
        module->stateWaitXData();
    }
    break;

   case(STATE_WAIT_XDATA): {
        module->last_reason = REASON_XDATA_END;
        if (module->op_mode == ACK_MODE) { 
           module->stateWaitXAck();

           if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                                    << module->info[module->curr_state] << " , next state = " 
                                    << module->info[STATE_WAIT_XACK] << endl;
        }
        else {
           module->stateIdle();

           if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                                    << module->info[module->curr_state] << " , next state = " 
                                    << module->info[STATE_IDLE] << endl;
        } 
    }
    break;

   case(STATE_WAIT_XACK): {

        if (module->debug_) cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() current state = " 
                         << module->info[module->curr_state] << " , next state = " 
                         << module->info[STATE_IDLE] << endl;

        module->last_reason = REASON_XACK_END;
        module->stateIdle();
    }
    break;

    default: {
        cout << NOW  << " MMacDACAP("<< module->addr << ") timer expire() logical error, current state = " 
             << module->info[module->curr_state] << endl;
        exit(1);
    }
    break;

  }

}

int MMacDACAP::u_pkt_id;
// int MMacDACAP::u_data_id;
map< int , string > MMacDACAP::info;

MMacDACAP::MMacDACAP() 
  : timer(this),
  backoff_timer(this),
  txsn(1),
  u_data_id(0),
  backoff_counter(0),
  RTS_sent_time(NOT_SET),
  session_distance(NOT_SET),
  curr_dest_addr(NOT_SET),
  sleep_node_1(NOT_SET),
  sleep_node_2(NOT_SET),
  backoff_duration(NOT_SET),
  backoff_start_time(NOT_SET),
  backoff_remaining(NOT_SET),
  last_reason(NOT_SET),
  backoff_first_start(NOT_SET),
  curr_data_pkt(0),
  TxActive(false),
  RxActive(false),
  defer_data(false),
  session_active(false),
  backoff_pending(false),
  warning_sent(false),
  backoff_freeze_mode(false),
  print_transitions(false),
  has_buffer_queue(false),
  multihop_mode(false),
  curr_state(STATE_IDLE), 
  prev_state(STATE_IDLE),
  op_mode(ACK_MODE),
  wrn_pkts_tx(0),
  wrn_pkts_rx(0),
  rts_pkts_tx(0),
  rts_pkts_rx(0),
  cts_pkts_tx(0),
  cts_pkts_rx(0),
  sum_defer_time(0),
  defer_times_no(0),
  last_data_id_tx(NOT_SET),
  last_data_id_rx(NOT_SET),
  start_tx_time(0),       
  srtt(0),      
  sumrtt(0),      
  sumrtt2(0),     
  rttsamples(0)

{ 
  u_pkt_id = 0;

  bind("t_min", (double*)& t_min);
  bind("T_W_min", (double*)& T_W_min);
  bind("delta_D", (double*)& delta_D);
  bind("delta_data", (double*)& delta_data);
  bind("max_prop_delay", (double*)& max_prop_delay);
  bind("CTS_size", (int*)& CTS_size);
  bind("RTS_size", (int*)& RTS_size);
  bind("WRN_size", (int*)& WRN_size);
  bind("HDR_size", (int*)& HDR_size);
  bind("ACK_size", (int*)& ACK_size);
  bind("backoff_tuner", (double*)& backoff_tuner);
  bind("wait_costant", (double*)& wait_costant);
  bind("debug_", (int*)&debug_); //degug mode
  bind("max_payload", (int*)&max_payload);
  bind("max_tx_tries", (double*)&max_tx_tries);
  bind("buffer_pkts", (int*)&buffer_pkts);
  bind("alpha_", (double*)&alpha_);
  bind("max_backoff_counter", (double*)&max_backoff_counter);

  if (buffer_pkts > 0) has_buffer_queue = true;
  if (max_tx_tries <= 0) max_tx_tries = HUGE_VAL;
  if (max_backoff_counter <= 0) max_backoff_counter = HUGE_VAL;

  if (info.empty()) initInfo();


}

MMacDACAP::~MMacDACAP()
{

}

// TCL command interpreter
int MMacDACAP::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc==2)
    {
      if(strcasecmp(argv[1], "printTransitions") == 0)	
	{
          print_transitions = true;
          if (print_transitions) fout.open("/tmp/DACAPstateTransitions.txt",ios_base::app);
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setAckMode") == 0)	
	{
	  op_mode = ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setNoAckMode") == 0)	
	{
          op_mode = NO_ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setBackoffFreeze") == 0)	
	{
          backoff_freeze_mode = true;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setBackoffNoFreeze") == 0)	
	{
          backoff_freeze_mode = false;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setMultiHopMode") == 0)	
	{
          multihop_mode = true;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getQueueSize") == 0)	
	{
	  tcl.resultf("%d",getQueueSize());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanDeferTime") == 0)	
	{
	  tcl.resultf("%f",getMeanDeferTime());
      	  return TCL_OK;
	}
     else if(strcasecmp(argv[1], "getTotalDeferTimes") == 0)	
	{
	  tcl.resultf("%d",getTotalDeferTimes());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getWrnPktsTx") == 0)	
	{
	  tcl.resultf("%d",getWrnPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getWrnPktsRx") == 0)	
	{
	  tcl.resultf("%d",getWrnPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getRtsPktsTx") == 0)	
	{
	  tcl.resultf("%d",getRtsPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getRtsPktsRx") == 0)	
	{
	  tcl.resultf("%d",getRtsPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getCtsPktsTx") == 0)	
	{
	  tcl.resultf("%d",getCtsPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getCtsPktsRx") == 0)	
	{
	  tcl.resultf("%d",getCtsPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getUpLayersDataRx") == 0)
      {
          tcl.resultf("%d",getUpLayersDataPktsRx());
          return TCL_OK;
      }
    }
    else if(argc==3){
		if(strcasecmp(argv[1],"setMacAddr") == 0)
		{
			addr = atoi(argv[2]);
			if (debug_) cout << "DACAP MAC address of current node is " << addr <<endl;
			return TCL_OK;
		}
	}
  return MMac::command(argc, argv);
}


int MMacDACAP::crLayCommand(ClMessage* m)
{
  switch (m->type()) 
    {
    
      //case whatever: 
      //    return 0;
      //    break;
    
    default:
      return Module::crLayCommand(m);    
    }  
}

void MMacDACAP::initInfo()
{
  if ( system(NULL) ) {
      system("rm -f /tmp/DACAPstateTransitions.txt");
      if (print_transitions) system("touch /tmp/DACAPstateTransitions.txt");
  }

  info[STATE_IDLE] = "Idle State";
  info[STATE_WAIT_CTS] = "Wait CTS State"; 
  info[STATE_DEFER_DATA] = "Defer Data State";
  info[STATE_SEND_DATA] = "Send Data State"; 
  info[STATE_WAIT_ACK] = "Wait ACK State"; 
  info[STATE_BACKOFF] = "Backoff State";
  info[STATE_CTS_RECEIVED] = "CTS Received State";
  info[STATE_WAIT_DATA] = "Wait Data State";
  info[STATE_DATA_RECEIVED] = "Data Received State";
  info[STATE_SEND_ACK] = "Send ACK State"; 
  info[STATE_SEND_RTS] = "Send RTS State";
  info[STATE_SEND_CTS] = "Send CTS State";
  info[STATE_WAIT_WRN] = "Wait WRN Window State";
  info[STATE_SEND_WRN] = "Send WRN Window State";
  info[STATE_WAIT_XCTS] = "Wait XCTS State";
  info[STATE_WAIT_XDATA] = "Wait XDATA State";
  info[STATE_WAIT_XACK] = "Wait XACK State";
  info[STATE_WAIT_XWRN] = "Wait XWRN State";
  info[STATE_RECONTEND_WINDOW] = "Wait Recontend Window State";

  info[RTS_PKT] = "RTS pkt";
  info[CTS_PKT] = "CTS pkt";
  info[WRN_PKT] = "Warning pkt";
  info[DATA_PKT] = "Data pkt";
  info[ACK_PKT] = "ACK pkt";
  info[REASON_DEFER] = "xCTS, or xRTS received";
  info[REASON_NOACK] = "ACK timeout";
  info[REASON_NODATA] = "Data timeout";
  info[REASON_NOCTS] = "CTS timeout";
  info[REASON_CTS_RECEIVED] = "CTS received";
  info[REASON_ACK_RECEIVED] = "ACK received";
  info[REASON_RTS_RECEIVED]= "RTS received";
  info[REASON_DATA_RECEIVED] = "DATA received";
  info[REASON_WRN_RECEIVED] = "WRN received";
  info[REASON_BACKOFF_END] = "Backoff ended";
  info[REASON_DEFER_END] = "Defer time elapsed";
  info[REASON_DATA_SENT] = "Data sent";
  info[REASON_BACKOFF_PENDING] = "Backoff pending";
  info[REASON_ACK_SENT] = "ACK sent";
  info[REASON_RTS_SENT] = "RTS sent";
  info[REASON_CTS_SENT] = "CTS sent";
  info[REASON_INTERFERENCE] = "xRTS or xCTS received";
  info[REASON_TX_ENDED] = "Interfering trasmission ended";
  info[REASON_DATA_PENDING] = "Data from upper layers pending in queue";
  info[REASON_NOWRN] = "Wait WRN Window ended with no WRN";
  info[REASON_WRN_END] = "Send WRN window ended";
  info[REASON_SAME_RTS_RECEIVED] = "RTS received for the same current DATA pkt";
  info[REASON_MAX_TX_TRIES] = "DATA dropped due to max tx rounds";
  info[REASON_XCTS_END] = "xCTS wait window ended";
  info[REASON_XDATA_END] = "xDATA wait window ended";
  info[REASON_XACK_END] = "xACK wait window ended";
  info[REASON_XACK_RX] = "xACK received";
  info[REASON_XCTS_RX] = "xCTS received";
  info[REASON_XDATA_RX] = "xDATA received";
  info[REASON_XRTS_RX] = "xRTS received";
  info[REASON_WAIT_XWRN_END] = "xWRN wait window ended";
  info[REASON_WAIT_RECONTEND_END] = "Recontend window ended";
}



double MMacDACAP::computeTxTime(int type)
{

  double duration;
  Packet* temp_data_pkt;

  if (type == DATA_PKT) {
     if (!Q.empty()) {
        temp_data_pkt = (Q.front())->copy();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = HDR_size + ch->size();
     }
     else { 
        temp_data_pkt = Packet::alloc();
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = HDR_size + max_payload;
     }
  }
  else if (type == RTS_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = RTS_size;
  }
  else if (type == CTS_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = CTS_size;
  }
  else if (type == ACK_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = ACK_size;
  }
  else if (type == WRN_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = WRN_size;
  }
  duration = Mac2PhyTxDuration(temp_data_pkt);
  Packet::free(temp_data_pkt);
  return(duration);
}

double MMacDACAP::computeWaitTime(int mode, double distance)
{

  double t_data = computeTxTime(DATA_PKT);

  if (debug_) {
      cout << NOW  << " MMacDACAP(" << addr << ")::computeWaitTime() tx duration = " << t_data << endl;
  }

  double T_w;

  double t_1 = (t_min - min( (delta_D / prop_speed), min (t_data, 2.0 * max_prop_delay - t_min ) ) ) / 2.0;
  double t_2 = (t_min - delta_data) / 2.0;
  double t_3 = min(t_1 , (t_min + T_W_min - (2.0 * delta_D / prop_speed))/4.0);

  if ( mode == NO_ACK_MODE ) {
     if ( (distance / prop_speed) < t_1 ) {
        T_w = t_min - (2.0 * distance / prop_speed);
     }
     else {
        T_w = 2.0 * (distance + delta_D) / prop_speed - t_min;
     }
     if (T_w < (2.0 * delta_D / prop_speed)) T_w = 2.0 * delta_D / prop_speed;
  }
  else {
     if ( ((distance / prop_speed) < t_2) && ((distance / prop_speed) > t_1) ) {
        T_w = (2.0 * (distance + delta_D) / prop_speed) - t_min;
     }
     else if ( (distance / prop_speed) > max(t_2,t_3) ){
        T_w = (2.0 * (distance + delta_D) / prop_speed) - T_W_min;
     }
     else {
        T_w = t_min - 2.0 * (distance / prop_speed);
     }

     if ( T_w < ( max( (2.0 * delta_D / prop_speed) , T_W_min ) ) ) T_w =  max( (2 * delta_D / prop_speed) , T_W_min ) ;

  }

  return(T_w);
}

inline void MMacDACAP::exitBackoff()
{
  if (backoff_freeze_mode == true) timer.force_cancel();
  backoff_timer.force_cancel();

  backoffEndTime();
  backoff_pending = false;
  backoff_start_time = NOT_SET;
  backoff_duration = NOT_SET;
  backoff_remaining = NOT_SET;
  backoff_duration = NOT_SET;
  backoff_first_start = NOT_SET;
}

void MMacDACAP::exitSleep() {
  sleep_node_1 = NOT_SET;
  sleep_node_2 = NOT_SET;
}

inline void MMacDACAP::freezeBackoff()
{
  timer.force_cancel();

  double elapsed_time = NOW - backoff_start_time;
  backoff_pending = true;

  if (backoff_start_time == NOT_SET) {
      cout << NOW  << " MMacDACAP(" << addr << ")::freezeBackoff() error, backoff start time not set " << endl;
      exit(1);      
  }

  backoff_remaining = backoff_duration - elapsed_time;

  if ( backoff_remaining < (-0.3)) {
      cout << NOW  << " MMacDACAP(" << addr << ")::freezeBackoff() error, backoff elapsed time = "
           << elapsed_time << " > backoff duration = " << backoff_duration << " !!" << endl;
      exit(1);
  }
  else {
      backoff_remaining = abs(backoff_duration - elapsed_time);
      
      if (debug_){
          cout << NOW  << " MMacDACAP("<< addr <<")::freezeBackoff() elapsed time = " << elapsed_time 
               << " < backoff duration = " << backoff_duration << ", remaining backoff = "
               << backoff_remaining << endl; 
      }
  }

}

double MMacDACAP::getRecontendTime() {

  double random = RNG::defaultrng()->uniform_double();

  while (random == 0) { 
     random = RNG::defaultrng()->uniform_double();
  }

  return(random * 2.0 * max_prop_delay + wait_costant);
}

double MMacDACAP::getBackoffTime()
{
  incrTotalBackoffTimes();

  double random = RNG::defaultrng()->uniform_double();
  double duration;

  while (random == 0) { 
     random = RNG::defaultrng()->uniform_double();
  }
 


  if (backoff_counter > max_backoff_counter) backoff_counter = max_backoff_counter;




  random = backoff_tuner * random * 2.0 * max_prop_delay * pow( 2.0, (double) backoff_counter);


  if (debug_){
       cout << NOW  << " MMacDACAP("<< addr <<")::getBackoffTime() backoff time = " << random << " s " << endl;

  }

  duration = random;  
  return duration;
}

void MMacDACAP::recvFromUpperLayers(Packet* p)
{
  if ( ((has_buffer_queue == true) && (Q.size() < buffer_pkts)) || (has_buffer_queue == false) ) {
  
     incrUpperDataRx();
     waitStartTime();

     initPkt(p, DATA_PKT);
     Q.push(p);

     if ( (curr_state == STATE_IDLE) && (session_active == false) && (RxActive == false) ) // se sono libero comincio handshake
       {
         last_reason = REASON_DATA_PENDING;
         stateSendRTS();


       }
     else
       {

       }
  }
  else {
     incrDiscardedPktsTx();
     drop(p, 1, DACAP_DROP_REASON_BUFFER_FULL);
  }
}

void MMacDACAP::initPkt( Packet* p, int type)
{

  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_dacap* dacaph = HDR_DACAP(p);
  hdr_mac* mach = HDR_MAC(p);

  dacaph->sn = txsn;
  txsn++;
  dacaph->ts = Scheduler::instance().clock();
  dacaph->dacap_type = type;
  dacaph->tx_tries = 0;
  
  int curr_size = ch->size();

  if (type != DATA_PKT) {
    ch->timestamp() = dacaph->ts;
    ch->ptype() = PT_DACAP;
    if (curr_data_pkt != 0) dacaph->data_sn = HDR_DACAP(curr_data_pkt)->data_sn;
    else dacaph->data_sn = u_data_id;
  }
  else { 
     dacaph->orig_type = ch->ptype();
     ch->ptype() = PT_DACAP;
  }

  switch(type) {
  
    case(RTS_PKT): {
      ch->size() = RTS_size;
      ch->uid() = u_pkt_id++;
      mach->set(MF_CONTROL, addr, curr_dest_addr);
    }
    break;

    case(CTS_PKT): {
      ch->size() = CTS_size;
      ch->uid() = u_pkt_id++;
      mach->set(MF_CONTROL, addr, curr_dest_addr);
     }
    break;

    case(WRN_PKT): {
      ch->size() = WRN_size;
      ch->uid() = u_pkt_id++;
      mach->set(MF_CONTROL, addr, curr_dest_addr);
    }
    break;

    case(DATA_PKT): {
      ch->size() = curr_size + HDR_size;
      dacaph->data_sn = u_data_id++;
      mach->macSA() = addr;
    } 
    break;

    case(ACK_PKT): {
      ch->size() = CTS_size;
      ch->uid() = u_pkt_id++;
      mach->set(MF_CONTROL, addr, curr_dest_addr);
    }
    break;

  }

}

void MMacDACAP::Phy2MacEndTx(const Packet* p)
{ 

  TxActive = false;

  switch(curr_state) {

    case(STATE_SEND_DATA): {

        if ((debug_ > 0) && (op_mode == ACK_MODE)) cout << NOW  << " MMacDACAP("<< addr <<")::Phy2MacEndTx() DATA sent, from " 
                                                        << info[curr_state] << " to " << info[STATE_WAIT_ACK] << endl;

        else if ((debug_ > 0) && (op_mode == NO_ACK_MODE)) 
                                         cout << NOW  << " MMacDACAP("<< addr <<")::Phy2MacEndTx() DATA sent, from " 
                                              << info[curr_state] << " to " << info[STATE_IDLE] << endl;

        last_reason = REASON_DATA_SENT;
        if (op_mode == ACK_MODE) stateWaitACK();
        else if ( (multihop_mode == true) && (Q.size() > 0) ) stateRecontendWindow();
        else stateIdle();
    }
    break;


    case(STATE_WAIT_DATA): {

    }
    break;

    case(STATE_SEND_ACK): {
        if (backoff_pending == true) {

        if (debug_) cout << NOW  << " MMacDACAP("<< addr <<")::Phy2MacEndTx() ack sent and backoff pending, from " 
                         << info[curr_state] << " to " << info[STATE_BACKOFF] << endl;

             last_reason = REASON_BACKOFF_PENDING;
             stateBackoff();
             return;
        }
        else {

            if (debug_) cout << NOW  << " MMacDACAP("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                             << info[curr_state] << " to " << info[STATE_IDLE] << endl;

            last_reason = REASON_ACK_SENT;
            stateIdle();
        } 
    }
    break;

    case(STATE_SEND_RTS): {
        if (debug_) cout << NOW  << " MMacDACAP("<< addr <<")::Phy2MacEndTx() RTS sent, from " 
                         << info[curr_state] << " to " << info[STATE_WAIT_CTS] << endl;

        last_reason = REASON_RTS_SENT; 
        stateWaitCTS();
    }
    break;

    case(STATE_SEND_CTS): {
        if (debug_) cout << NOW  << " MMacDACAP("<< addr <<")::Phy2MacEndTx() CTS sent, from " 
                         << info[curr_state] << " to " << info[STATE_SEND_WRN] << endl;

       last_reason = REASON_CTS_SENT; 
       stateSendWarning();
    }
    break;

    case(STATE_SEND_WRN): {

    }
    break;

    default: {
        cout << NOW  << " MMacDACAP("<< addr <<")::Phy2MacEndTx() logical error, current state = " 
             << info[curr_state] << endl;
        exit(1);
    }
    break;

  }

}

void MMacDACAP::Phy2MacStartRx(const Packet* p)
{
  RxActive = true;
}


void MMacDACAP::Phy2MacEndRx(Packet* p)
{

  hdr_cmn* ch = HDR_CMN(p);
  packet_t rx_pkt_type = ch->ptype();

  if (rx_pkt_type != PT_DACAP) {
       drop(p, 1, DACAP_DROP_REASON_UNKNOWN_TYPE);
       return;
  }

  hdr_dacap* dacaph = HDR_DACAP(p); 
  hdr_mac* mach = HDR_MAC(p);
  hdr_MPhy* ph = HDR_MPHY(p);
 
  int source_mac = mach->macSA();
  double tx_time = ph->txtime;
  double rx_time = ph->rxtime;
  double diff_time = rx_time - tx_time;
  double distance = diff_time * prop_speed;

  if (ch->error()) {
    drop(p, 1, "ERR");
    incrErrorPktsRx();

    return;
  }
  else {
    
    if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::Phy2MacEndRx() " 
                     << info[curr_state] << ", received a pkt type = " 
                     << info[dacaph->dacap_type] << ", src addr = " << mach->macSA() 
                     << " dest addr = " << mach->macDA() 
                     << ", estimated distance between nodes = " << distance << " m " << endl;

    switch(curr_state) {

      case(STATE_IDLE): {
         rxStateIdle(p);
      }
      break;

      case(STATE_RECONTEND_WINDOW): {
         rxStateRecontendWindow(p);
      }
      break;

      case(STATE_WAIT_CTS): {
         rxStateWaitCTS(p);
      }
      break;

     case(STATE_WAIT_WRN): {
         rxStateWaitWarning(p);
      }
      break;

      case(STATE_WAIT_ACK): {
         rxStateWaitACK(p);
      }
      break;

      case(STATE_BACKOFF): {
         rxStateBackoff(p);
      }
      break;

      case(STATE_WAIT_DATA): {
         rxStateWaitData(p);
      }
      break;

      case(STATE_SEND_WRN): {
         rxStateSendWarning(p);
      }
      break;

      case(STATE_WAIT_XCTS): {
         rxStateWaitXCts(p);
      }
      break;

     case(STATE_WAIT_XWRN): {
         rxStateWaitXWarning(p);
      }
      break;

      case(STATE_WAIT_XDATA): {
         rxStateWaitXData(p);
      }
      break;

      case(STATE_WAIT_XACK): {
         rxStateWaitXAck(p);
      }
      break;

      default: {

          if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::Phy2MacEndRx() dropping pkt, wrong state" << endl;

          Packet::free(p);
          return;
      }
      break;

    }
  }
}

void MMacDACAP::setBackoffNodes(double node_a, double node_b) {
  if ( (sleep_node_1 == NOT_SET) && (sleep_node_2 == NOT_SET) ) {
                     sleep_node_1 = node_a;
                     sleep_node_2 = node_b;
   }
   if (debug_) {
      cout << NOW  << " MMacDACAP("<< addr << ")::setBackoffNodes() mac addr involved = "
           << sleep_node_1 << " , " << sleep_node_2 << " . next state = "
           << info[STATE_BACKOFF] << endl;
  }
}


void MMacDACAP::txAck() {
  Packet* ack_pkt = Packet::alloc();      
  initPkt(ack_pkt , ACK_PKT);
  
  incrCtrlPktsTx();
  incrAckPktsTx();
  Mac2PhyStartTx(ack_pkt); 
  TxActive = true;
}

void MMacDACAP::txData() {
  Packet* data_pkt = (Q.front())->copy();       
 
  if ( (op_mode == NO_ACK_MODE) ) queuePop(true);  

  incrDataPktsTx();
  Mac2PhyStartTx(data_pkt); 
  TxActive = true;
}

void MMacDACAP::txRts() {
  Packet* rts_pkt = Packet::alloc();      
 
  //IpClMsgUpdRoute m(curr_data_pkt);
  //sendSyncClMsg(&m);
  
  //MllClMsgUpdMac m2(curr_data_pkt);
  //sendSyncClMsg(&m2);
    
  hdr_mac* mach = HDR_MAC(curr_data_pkt);

  curr_dest_addr = mach->macDA();
 
  initPkt(rts_pkt , RTS_PKT);


  
  incrCtrlPktsTx();
  incrRtsPktsTx();

  TxActive = true;
  start_tx_time = NOW; // we set curr RTT
  Mac2PhyStartTx(rts_pkt); 
}

void MMacDACAP::txCts() {
  Packet* cts_pkt = Packet::alloc();      
  initPkt(cts_pkt , CTS_PKT);

  incrCtrlPktsTx();
  incrCtsPktsTx();


  
  Mac2PhyStartTx(cts_pkt); 
  TxActive = true;
}

void MMacDACAP::txWrn()
{
  warning_sent = true;
  Packet* wrn_ptk = Packet::alloc();      
  initPkt(wrn_ptk , WRN_PKT); 
  Mac2PhyStartTx(wrn_ptk); 
  incrWrnPktsTx();
  TxActive = true;
}

void MMacDACAP::rxStateIdle(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p); 
   
   if (dest_mac == addr) { // RTS for me
      if (pkt_type == RTS_PKT) {
         incrCtrlPktsRx();
         incrRtsPktsRx();

         curr_dest_addr = source_mac; 
         session_active = true;
         session_distance = distance;
         if (last_data_id_rx != pkt_data_sn) last_data_id_rx = pkt_data_sn;
         last_reason = REASON_RTS_RECEIVED;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateIdle() next state = "
                           << info[STATE_SEND_CTS] << endl;

         stateSendCTS(); 
         return;
      }
   }
   else { // not for me
      if (pkt_type == RTS_PKT)   {
         incrXCtrlPktsRx();
         setBackoffNodes(source_mac , dest_mac);
         last_reason = REASON_XRTS_RX;
         stateWaitXCts();  
         return;   
      }
      else if (pkt_type == CTS_PKT) {
         incrXCtrlPktsRx();
         setBackoffNodes(source_mac , dest_mac);
         last_reason = REASON_XCTS_RX;
         stateWaitXWarning();
         return;
      }
      else if (pkt_type == DATA_PKT) {
         incrXDataPktsRx();
         setBackoffNodes(source_mac , dest_mac);
         last_reason = REASON_XDATA_RX;
         stateWaitXAck();
         return;
      }
   }
}

void MMacDACAP::rxStateBackoff(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p); 

   if (dest_mac == addr) { // for me
         if (pkt_type == RTS_PKT) {  // RTS
            incrCtrlPktsRx();
            incrRtsPktsRx();

            session_distance = distance; 
            curr_dest_addr = source_mac;
            if (last_data_id_rx != pkt_data_sn) last_data_id_rx = pkt_data_sn; 
            last_reason = REASON_RTS_RECEIVED;
            session_active = true;

            if (backoff_freeze_mode == true) freezeBackoff();
            else exitBackoff();

            if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateBackoff() next state = "
                              << info[STATE_SEND_CTS] << endl;

            stateSendCTS();
            return;
         }
   }
   else { 
      if (pkt_type == RTS_PKT)   {
         incrXCtrlPktsRx();
         setBackoffNodes(source_mac, dest_mac);
         last_reason = REASON_XRTS_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateBackoff() next state = "
                           << info[STATE_WAIT_XCTS] << endl;

         stateWaitXCts();  
         return;   
      }
      else if (pkt_type == CTS_PKT) {
         incrXCtrlPktsRx();
         setBackoffNodes(source_mac, dest_mac);
         last_reason = REASON_XCTS_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateBackoff() next state = "
                           << info[STATE_WAIT_XWRN] << endl;

         stateWaitXWarning();
         return;
      }
      else if (pkt_type == DATA_PKT) {
         incrXDataPktsRx();
         setBackoffNodes(source_mac, dest_mac);
         last_reason = REASON_XDATA_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateBackoff() next state = "
                           << info[STATE_WAIT_XACK] << endl;

         stateWaitXAck();
         return;
      }
   }
}

void MMacDACAP::rxStateWaitCTS(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p); 

   if ( (dest_mac != addr) && (pkt_type == CTS_PKT) ) { 
      incrXCtrlPktsRx();
      if ( (op_mode == NO_ACK_MODE) && (diff_time < t_min) )  { 
   
         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitCTS() diff time = " 
                  << diff_time << " < t_min = " << t_min << " ; Defering data " << endl;
         }
   
         defer_data = true;
         return;
      }
      else if ( (op_mode == ACK_MODE) && (diff_time < T_W_min ) )  { //  ack
   
         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitCTS() diff time = " 
                  << diff_time << " < T_W_min = " << T_W_min << " ; Defering data " << endl;
               //waitForUser();
         }
   
         defer_data = true;
         return;
   
      }
   }
   else if ( (dest_mac != addr) && (pkt_type == RTS_PKT) ) { //  xRTS
      incrXCtrlPktsRx();
      if ( (op_mode == ACK_MODE) && (diff_time < max_prop_delay) ) { //  ack
   
         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitCTS() diff time = " 
                  << diff_time << " < max_prop_delay = " << max_prop_delay << " ; Defering data " << endl;
         }
   
         defer_data = true;
         return;
      }
   }
   else if ( (dest_mac == addr) && (source_mac == curr_dest_addr) && (pkt_type == CTS_PKT) ) { 
      // CTS for me
      incrCtrlPktsRx();
      incrCtsPktsRx();

      session_distance = distance;
      last_reason = REASON_CTS_RECEIVED;   

      if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitCTS() next state = "
                       << info[STATE_WAIT_WRN] << endl;   

      stateWaitWarning();
      return;
   }
}

void MMacDACAP::rxStateWaitACK(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p);

   if ( (dest_mac == addr) && (source_mac == curr_dest_addr) && (pkt_type == ACK_PKT) ) { 
   // ACK 
      incrCtrlPktsRx();
      incrAckPktsRx(); 

      queuePop(true);

      if (backoff_pending == false) {

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitACK() next state = "
                          << info[STATE_IDLE] << endl;

         last_reason = REASON_ACK_RECEIVED;
         if ( (multihop_mode == true) && (Q.size() > 0) ) stateRecontendWindow();
         else stateIdle();
      }
      else {

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitACK() next state = "
                          << info[STATE_BACKOFF] << endl;

         last_reason = REASON_BACKOFF_PENDING;
         stateBackoff();
      }
   }
}

void MMacDACAP::rxStateWaitData(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   if ( (dest_mac == addr) && (source_mac == curr_dest_addr) && (pkt_type == DATA_PKT) ) { 
   //DATA for me

      if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitData() next state = "
                        << info[STATE_DATA_RECEIVED] << endl;

      last_reason = REASON_DATA_RECEIVED;
      stateDataReceived(p); 
      return;
   }

   Packet::free(p); 

   if ((dest_mac == addr) && (source_mac == curr_dest_addr) && (pkt_type == RTS_PKT) 
                                                   && (last_data_id_rx == pkt_data_sn)) {
      incrCtrlPktsRx();
      incrRtsPktsRx();

      curr_dest_addr = source_mac; 
      session_active = true;
      session_distance = distance;

      last_reason = REASON_SAME_RTS_RECEIVED;
      stateSendCTS(); 
      return;
   } 
}

void MMacDACAP::rxStateWaitXCts(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p);   

   if ( (dest_mac != addr) && ( (source_mac == sleep_node_1) || (source_mac == sleep_node_2) )) { 

      if ( ( (pkt_type == ACK_PKT) && ( op_mode == ACK_MODE) ) 
                                    || ( (pkt_type == DATA_PKT) && (op_mode == NO_ACK_MODE) ) ) {
         
         if (pkt_type == ACK_PKT) incrXCtrlPktsRx();
         else incrXDataPktsRx();
   
         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXCts() DATA or ACK received "
                     << "from nodes involved. quitting sleep. next state = "
                     << info[STATE_IDLE] << endl;
         
         }
         exitSleep();
   
         last_reason = REASON_TX_ENDED;   
         stateIdle(); 
         return;
      }
      else if (pkt_type == CTS_PKT ) {
         incrXCtrlPktsRx();
         last_reason = REASON_XCTS_RX;
   
         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXCts() next state = "
                           << info[STATE_WAIT_XWRN] << endl;
   
         stateWaitXWarning();
         return;
      }
      else if (pkt_type == DATA_PKT ) {
         incrXDataPktsRx();
         last_reason = REASON_XDATA_RX;
   
         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXCts() next state = "
                           << info[STATE_WAIT_XACK] << endl;
   
         stateWaitXAck();
         return;
      }
   }
}

void MMacDACAP::rxStateWaitXData(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p); // rilascio il pkt  

   if ( (dest_mac != addr) && ( (source_mac == sleep_node_1) || (source_mac == sleep_node_2) )) { 

      if ( ( (pkt_type == ACK_PKT) && ( op_mode == ACK_MODE) ) 
                                    || ( (pkt_type == DATA_PKT) && (op_mode == NO_ACK_MODE) ) ) {
         
         if (pkt_type == ACK_PKT) incrXCtrlPktsRx();
         else incrXDataPktsRx();

         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXData() DATA or ACK received "
                     << "from nodes involved. quitting sleep. next state = "
                     << info[STATE_IDLE] << endl;
               //waitForUser();
         }

         exitSleep();
         last_reason = REASON_TX_ENDED;   
         stateIdle(); // esco da backoff
         return;
      }
      else if ( dacaph->dacap_type == DATA_PKT ) {
         incrXDataPktsRx();
         last_reason = REASON_XDATA_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXData() next state = "
                           << info[STATE_WAIT_XACK] << endl;

         stateWaitXAck();
         return;
      }
   }
}

void MMacDACAP::rxStateWaitXAck(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p);   

   if ( (dest_mac != addr) && ( (source_mac == sleep_node_1) || (source_mac == sleep_node_2) )) { 

      if ( ( (pkt_type == ACK_PKT) && ( op_mode == ACK_MODE) ) 
                                    || ( (pkt_type == DATA_PKT) && (op_mode == NO_ACK_MODE) ) ) {
         
         if (pkt_type == ACK_PKT) incrXCtrlPktsRx();
         else incrXDataPktsRx();

         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXAck() DATA or ACK received "
                     << "from nodes involved. quitting sleep. next state = "
                     << info[STATE_IDLE] << endl;
               //waitForUser();
         }
         exitSleep();
         last_reason = REASON_TX_ENDED;   
         stateIdle(); // esco da backoff
      }
   }
}

void MMacDACAP::rxStateWaitXWarning(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p);   

   if ( (dest_mac != addr) && ( (source_mac == sleep_node_1) || (source_mac == sleep_node_2) )) { 

      if ( ( (pkt_type == ACK_PKT) && ( op_mode == ACK_MODE) ) 
                                    || ( (pkt_type == DATA_PKT) && (op_mode == NO_ACK_MODE) ) ) {
         
         if (pkt_type == ACK_PKT) incrXCtrlPktsRx();
         else incrXDataPktsRx();

         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXWarning() DATA or ACK received "
                     << "from nodes involved. quitting sleep. next state = "
                     << info[STATE_IDLE] << endl;
               //waitForUser();
         }
         exitSleep();
         last_reason = REASON_TX_ENDED;   
         stateIdle(); // esco da backoff
         return;
      }
      else if ( dacaph->dacap_type == DATA_PKT ) {
         incrXDataPktsRx();
         last_reason = REASON_XDATA_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitXWarning() next state = "
                           << info[STATE_WAIT_XACK] << endl;

         stateWaitXAck();
         return;
      }
   }
}

void MMacDACAP::rxStateWaitWarning(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p);  

   if ( (dest_mac != addr) && (source_mac == CTS_PKT) ) { //  xCTS
      incrXCtrlPktsRx();
      if ( (op_mode == NO_ACK_MODE) && (diff_time < t_min) )  { //  no ack

         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitWarning() diff time = " 
                  << diff_time << " < t_min = " << t_min << " ; Defering data " << endl;
                        }

         defer_data = true;
         return;
      }
      else if ( (op_mode == ACK_MODE) && (diff_time < T_W_min ) )  { // condizione ack

         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitWarning() diff time = " 
                  << diff_time << " < T_W_min = " << T_W_min << " ; Defering data " << endl;
              
         }

         defer_data = true;
         return;
      }
   }
   else if ( (dest_mac != addr) && (pkt_type == RTS_PKT) ) { //  xRTS
      incrXCtrlPktsRx();
      if ( (op_mode == ACK_MODE) && (diff_time < max_prop_delay) ) { //  ack

         if (debug_) {
               cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitWarning() diff time = " 
                  << diff_time << " < max_prop_delay = " << max_prop_delay << " ; Defering data " << endl;
               //waitForUser();
         }

         defer_data = true;
         return;
      }
   }
   else if ( (dest_mac == addr) && (source_mac == curr_dest_addr) && (pkt_type == WRN_PKT) ) { 
      //  WRN 
      incrWrnPktsRx();

      if (debug_) {
            cout << NOW  << " MMacDACAP("<< addr << ")::rxStateWaitWarning() WRN received, defering data " << endl;
      }

      defer_data = true;
      return;
   }
}

void MMacDACAP::rxStateSendWarning(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   if ( (dest_mac == addr) && (source_mac == curr_dest_addr) && (pkt_type == DATA_PKT) ) { 
      //ricevo DATA x me

      if (debug_) {   
            cout << NOW  << " MMacDACAP("<< addr << ")::rxStateSendWarning() DATA received in " << info[curr_state]
                  << " logical error!!!! " << endl;
            waitForUser();
      }

      last_reason = REASON_DATA_RECEIVED;
      stateDataReceived(p); 
      return;
   }

   Packet::free(p); 

   if ((dest_mac == addr) && (source_mac == curr_dest_addr) && (pkt_type == RTS_PKT) 
                                             && (last_data_id_rx == pkt_data_sn)) {
         
         incrCtrlPktsRx();
         incrRtsPktsRx();

         curr_dest_addr = source_mac; 
         session_active = true;
         session_distance = distance;
         last_reason = REASON_SAME_RTS_RECEIVED;
         stateSendCTS(); 
         return;
   } 
   else if ( (dest_mac != addr) && (pkt_type == RTS_PKT) ) { 
      if ( diff_time < ( 2 * max_prop_delay - t_min) ) { 
         incrXCtrlPktsRx();

         if (debug_) {
            cout << NOW  << " MMacDACAP("<< addr << ")::rxStateSendWarning() difference time = "
                  << diff_time << " < " << ( 2 * max_prop_delay - t_min) 
                  << " sending a warning " << endl;
         }

         txWrn(); 
         return;
      }
   }
   else if ( (dest_mac != addr) && (pkt_type == CTS_PKT) ) { //ricevo xCTS
      if ( (diff_time < ( 2 * max_prop_delay - T_W_min)) && (op_mode == ACK_MODE) ) { // se è ack mode
         incrXCtrlPktsRx();

         if (debug_) {
            cout << NOW  << " MMacDACAP("<< addr << ")::rxStateSendWarning() difference time = "
                  << diff_time << " < " << ( 2 * max_prop_delay - T_W_min) 
                  << " sending a warning " << endl;
         }

         txWrn();
         return;
      }
   }
}

void MMacDACAP::rxStateRecontendWindow(Packet* p)
{
   RxActive = false;

   hdr_dacap* dacaph = HDR_DACAP(p); 
   hdr_mac* mach = HDR_MAC(p);
   hdr_MPhy* ph = HDR_MPHY(p);  

   int source_mac = mach->macSA();
   int dest_mac = mach->macDA();
   int pkt_type = dacaph->dacap_type;
   int pkt_data_sn = dacaph->data_sn;
   double tx_time = ph->txtime;
   double rx_time = ph->rxtime;
   double diff_time = rx_time - tx_time;
   double distance = diff_time * prop_speed;

   Packet::free(p); 

   if (dest_mac == addr) { // RTS for me
      if (pkt_type == RTS_PKT) {
         incrCtrlPktsRx();
         incrRtsPktsRx();

         curr_dest_addr = source_mac; 
         session_active = true;
         session_distance = distance;
         if (last_data_id_rx != pkt_data_sn) last_data_id_rx = pkt_data_sn;
         last_reason = REASON_RTS_RECEIVED;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateRecontendWindow() next state = "
                           << info[STATE_SEND_CTS] << endl;

         stateSendCTS(); 
         return;
      }
   }
   else { 
      if (dacaph->dacap_type == RTS_PKT)   {
         incrXCtrlPktsRx();
         setBackoffNodes(source_mac, dest_mac );
         last_reason = REASON_XRTS_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateRecontendWindow() next state = "
                           << info[STATE_WAIT_XCTS] << endl;

         stateWaitXCts();  
         return;   
      }
      else if ( pkt_type == CTS_PKT ) {
         incrXCtrlPktsRx();
         setBackoffNodes(source_mac, dest_mac );
         last_reason = REASON_XCTS_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateRecontendWindow() next state = "
                           << info[STATE_WAIT_XWRN] << endl;

         stateWaitXWarning();
         return;
      }
      else if (pkt_type == DATA_PKT) {
         incrXDataPktsRx();
         setBackoffNodes(source_mac, dest_mac );
         last_reason = REASON_XDATA_RX;

         if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::rxStateRecontendWindow() next state = "
                           << info[STATE_WAIT_XACK] << endl;

         stateWaitXAck();
         return;
      }
   }
}

void MMacDACAP::stateIdle()
{
  timer.force_cancel();
  refreshState(STATE_IDLE);

  printStateInfo();

  warning_sent = false;
  session_active = false;
  defer_data = false;

  curr_dest_addr = NOT_SET; 
  exitSleep();

  if ( !Q.empty() ) {
     if ( HDR_DACAP(Q.front())->data_sn != last_data_id_tx) {
        session_distance = NOT_SET;
        backoff_counter = 0; 
        last_data_id_tx = HDR_DACAP(Q.front())->data_sn;
     }
     if ( HDR_DACAP(Q.front())->tx_tries < max_tx_tries ) {
        last_reason = REASON_DATA_PENDING;
        HDR_DACAP(Q.front())->tx_tries++;
        stateSendRTS();
     }
     else {
       queuePop(false);
       incrDroppedPktsTx();

       last_reason = REASON_MAX_TX_TRIES;

       if (debug_) cout << NOW << "  MMacDACAP("<< addr << ")::stateIdle() dropping pkt, max tx tries reached" << endl;


       stateIdle();
     }
  }
}

void MMacDACAP::stateSendRTS()
{
  session_active = true;

  timer.force_cancel();
  refreshState(STATE_SEND_RTS);

  printStateInfo();

  curr_data_pkt = Q.front();

  txRts();
}


void MMacDACAP::stateBackoff()
{
  timer.force_cancel();
  refreshState(STATE_BACKOFF);

  warning_sent = false;
  defer_data = false;

  if (backoff_pending == false) {

     if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::stateBackoff() starting a new backoff" << endl;

     backoffStartTime();
     backoff_duration = getBackoffTime();
     backoff_start_time = NOW;

     if ( (prev_state == STATE_WAIT_CTS) || (prev_state == STATE_WAIT_ACK) ) backoff_counter++; 


     if (backoff_freeze_mode == true) timer.resched(backoff_duration);
     else backoff_timer.resched(backoff_duration);
  }
  else if (backoff_freeze_mode == true) { // backoff freeze mode on, we need to restore freezed backoff

     if (backoff_remaining == NOT_SET) {
        cout << NOW  << " MMacDACAP("<< addr << ")::stateBackoff() error!!! backoff remaining not set!!!" << endl;
        exit(1);
     }

     if (debug_) cout << NOW  << " MMacDACAP("<< addr << ")::stateBackoff() continuing previous backoff, "
                      << " time remaining = " << backoff_remaining << " s" << endl;

     backoff_start_time = NOW;
     backoff_duration = backoff_remaining;
     timer.resched(backoff_remaining);
  }
  // no freeze,backoff pending and timer already counting, we wait for an event

  

  printStateInfo(backoff_duration);
 
}


void  MMacDACAP::stateWaitXCts() {
  timer.force_cancel();
  refreshState(STATE_WAIT_XCTS);



  printStateInfo();


  timer.resched(2.0*max_prop_delay + computeTxTime(CTS_PKT));
}


void MMacDACAP::stateWaitXData() {
  timer.force_cancel();
  refreshState(STATE_WAIT_XDATA);



  printStateInfo();


  timer.resched(2.0*max_prop_delay + computeTxTime(DATA_PKT) + computeWaitTime(op_mode, ((max_prop_delay*2.0)*1500.0) ));
}

void MMacDACAP::stateWaitXAck() {
  timer.force_cancel();
  refreshState(STATE_WAIT_XACK);



  printStateInfo();


  timer.resched(2.0*max_prop_delay + computeTxTime(ACK_PKT));
}

void MMacDACAP::stateWaitXWarning() {
  timer.force_cancel();
  refreshState(STATE_WAIT_XWRN);



  printStateInfo();


  if (op_mode == ACK_MODE) timer.resched(3.0 * max_prop_delay - T_W_min + computeTxTime(WRN_PKT) + computeTxTime(RTS_PKT) + wait_costant);
  else timer.resched(3.0 * max_prop_delay - t_min + computeTxTime(WRN_PKT) + computeTxTime(RTS_PKT) + wait_costant);
}

void MMacDACAP::stateWaitWarning()
{
  timer.force_cancel();
  refreshState(STATE_WAIT_WRN);

  printStateInfo();
  


  if (op_mode == ACK_MODE) timer.resched(2.0 * max_prop_delay - T_W_min + computeTxTime(WRN_PKT) + wait_costant);
  else timer.resched(2.0 * max_prop_delay - t_min + computeTxTime(WRN_PKT) + wait_costant);
}

void MMacDACAP::stateSendWarning()
{
  timer.force_cancel();
  refreshState(STATE_SEND_WRN);

  printStateInfo();

//   timer.resched(2 * max_prop_delay - min(T_W_min,t_min) + computeTxTime(WRN_PKT) + wait_costant);

  if (op_mode == ACK_MODE) timer.resched(2.0 * max_prop_delay - T_W_min + wait_costant);
  else timer.resched(2.0 * max_prop_delay - t_min + wait_costant);
}

void MMacDACAP::stateWaitCTS()
{
  timer.force_cancel();

  refreshState(STATE_WAIT_CTS);

  printStateInfo();


  timer.resched(2.0 * max_prop_delay + computeTxTime(CTS_PKT) + computeTxTime(RTS_PKT) + wait_costant); 
}

void MMacDACAP::stateWaitData()
{
  timer.force_cancel();
  refreshState(STATE_WAIT_DATA);
  double delay;

  printStateInfo();

  timer.resched(2.0 * max_prop_delay + computeTxTime(DATA_PKT) + computeWaitTime(op_mode, (max_prop_delay*2.0)*1500.0) + wait_costant); 
}

void MMacDACAP::stateWaitACK()
{
  timer.force_cancel();
  refreshState(STATE_WAIT_ACK);

  printStateInfo();


  timer.resched( 2.0 * max_prop_delay + computeTxTime(ACK_PKT) + computeTxTime(DATA_PKT) + wait_costant); 
}

void MMacDACAP::stateDeferData()
{
  timer.force_cancel();
  refreshState(STATE_DEFER_DATA);
  incrTotalDeferTimes();

  double defer_delay = computeWaitTime(op_mode, session_distance);
  deferEndTime(defer_delay);

  if (debug_) {
       cout << NOW  << " MMacDACAP(" << addr << ")::stateDeferData() defer delay = " << defer_delay << endl;
  }

  if (debug_) printStateInfo(defer_delay);
 
  timer.resched(defer_delay);
}


void MMacDACAP::stateSendData()
{
  timer.force_cancel();
  refreshState(STATE_SEND_DATA);

  defer_data = false; 

  printStateInfo();

  txData();
}


void MMacDACAP::stateCTSReceived()
{
  timer.force_cancel();
  refreshState(STATE_CTS_RECEIVED);
  
  printStateInfo();

  if (defer_data == true) stateDeferData();
  else stateSendData();
}


void MMacDACAP::stateDataReceived(Packet* data_pkt)
{
  timer.force_cancel();
  refreshState(STATE_DATA_RECEIVED);
  
  printStateInfo();

  incrDataPktsRx();
  
  hdr_cmn* ch = hdr_cmn::access(data_pkt);
  hdr_dacap* dacaph = HDR_DACAP(data_pkt);
  ch->ptype() = dacaph->orig_type;
  ch->size() = ch->size() - HDR_size;

  sendUp(data_pkt); 

  if (op_mode == ACK_MODE) stateSendAck();
  else stateIdle();
}

void MMacDACAP::stateSendAck()
{
  timer.force_cancel();
  refreshState(STATE_SEND_ACK);

  printStateInfo();

  txAck();
}

void MMacDACAP::stateSendCTS()
{
  timer.force_cancel();
  refreshState(STATE_SEND_CTS);

  printStateInfo();

  txCts();
}

void MMacDACAP::stateRecontendWindow() {
  timer.force_cancel();
  refreshState(STATE_RECONTEND_WINDOW);

  double delay = getRecontendTime();
  printStateInfo(delay);

  timer.resched(delay);
}

void MMacDACAP::printStateInfo(double delay)
{
  if (debug_) cout << NOW << " MMacDACAP("<< addr << ")::printStateInfo() " << "from " << info[prev_state] 
                   << " to " << info[curr_state] << " reason: " << info[last_reason] << endl;

  if (print_transitions) {
     if (curr_state == STATE_BACKOFF) {
         fout << left << setw(10) << NOW << " MMacDACAP("<< addr << ")::printStateInfo() " 
              << "from " << info[prev_state] 
              << " to " << info[curr_state] << " reason: " << info[last_reason] 
              << ". Backoff duration = " << delay << "; backoff cnt = " << backoff_counter << endl;
     }
     else if (curr_state == STATE_DEFER_DATA) {
         fout << left << setw(10) << NOW << " MMacDACAP("<< addr << ")::printStateInfo() " 
              << "from " << info[prev_state] 
              << " to " << info[curr_state] << " reason: " << info[last_reason] 
              << ". Defering delay = " << delay << endl;
     }
     else if (curr_state == STATE_RECONTEND_WINDOW) {
         fout << left << setw(10) << NOW << " MMacDACAP("<< addr << ")::printStateInfo() " 
              << "from " << info[prev_state] 
              << " to " << info[curr_state] << " reason: " << info[last_reason] 
              << ". Waiting delay = " << delay << endl;
     }
     else {
         fout << left << setw(10) << NOW << " MMacDACAP("<< addr << ")::printStateInfo() " 
              << "from " << info[prev_state] 
              << " to " << info[curr_state] << " reason: " << info[last_reason] << endl;
     }
  }
}
  
inline void MMacDACAP::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
} 



    
	
