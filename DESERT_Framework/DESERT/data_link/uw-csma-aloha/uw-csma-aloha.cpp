//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file   uw-csma-aloha.cpp
 * @author Federico Guerra, Saiful Azad and Federico Favaro
 * @version 1.0.0
 * 
 * \brief Provides the implementation of CsmaAloha Class
 * 
 */


#include "uw-csma-aloha.h"
#include <mac.h>
#include <cmath>
#include <climits>
#include <iomanip>
#include <rng.h>
#include <stdio.h>
#include <stdlib.h>


enum {
    NOT_SET = -1, SESSION_DISTANCE_NOT_SET = 0
};
/**
 * Class that represents the binding with the tcl configuration script 
 */
static class CSMAModuleClass : public TclClass {
public:
  /**
   * Constructor of the class
   */
  CSMAModuleClass() : TclClass("Module/UW/CSMA_ALOHA") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
    return (new CsmaAloha());
  }
} class_module_csma;


void CsmaAloha::AckTimer::expire(Event *e) {
  timer_status = CSMA_EXPIRED;
  if (module->curr_state == CSMA_STATE_WAIT_ACK) {


        if (module->debug_) cout << NOW << "  CsmaAloha("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << "; ACK not received, next state = " 
                         << module->status_info[CSMA_STATE_BACKOFF] << endl;

        module->refreshReason(CSMA_REASON_ACK_TIMEOUT);
        module->stateBackoff(); 
  }
  else {
    if (module->debug_ ) cout << NOW << "  CsmaAloha("<< module->addr << ")::AckTimer::expired() " << endl;
  }  
}


void CsmaAloha::BackOffTimer::expire(Event *e) {
  timer_status = CSMA_EXPIRED;
  if (module->curr_state == CSMA_STATE_BACKOFF ) {
    
    if (module->debug_) cout << NOW << "  CsmaAloha("<< module->addr << ") timer expire() current state = " 
                             << module->status_info[module->curr_state] << "; backoff expired, next state = " 
                             << module->status_info[CSMA_STATE_IDLE] << endl;

    module->refreshReason(CSMA_REASON_BACKOFF_TIMEOUT);
    module->exitBackoff();
    module->stateIdle();
  }
  else {
    if (module->debug_ ) cout << NOW << "  CsmaAloha("<< module->addr << ")::BackoffTimer::expired() " << endl;
  }  
}


void CsmaAloha::ListenTimer::expire(Event *e) {   
  timer_status = CSMA_EXPIRED;

  if (module->curr_state == CSMA_STATE_LISTEN ) {

    if (module->debug_) cout << NOW << "  CsmaAloha("<< module->addr << ") timer expire() current state = " 
                      << module->status_info[module->curr_state] << "; listening period expired, next state = " 
                      << module->status_info[CSMA_STATE_TX_DATA] << endl;

    module->refreshReason(CSMA_REASON_LISTEN_TIMEOUT);
    module->stateTxData();
  }
  else {
    if (module->debug_ ) cout << NOW << "  CsmaAloha("<< module->addr << ")::ListenTimer::expired() " << endl;
  }  
}


const double CsmaAloha::prop_speed = 1500.0;
int CsmaAloha::u_pkt_id;
bool CsmaAloha::initialized = false;


map< CsmaAloha::CSMA_STATUS , string> CsmaAloha::status_info;
map< CsmaAloha::CSMA_REASON_STATUS, string> CsmaAloha::reason_info;
map< CsmaAloha::CSMA_PKT_TYPE, string> CsmaAloha::pkt_type_info;

CsmaAloha::CsmaAloha() 
: ack_timer(this),
  listen_timer(this),
  backoff_timer(this),
  u_data_id(0),
  last_sent_data_id(-1),
  session_distance(SESSION_DISTANCE_NOT_SET),
  curr_data_pkt(0),
  last_data_id_rx(NOT_SET),
  curr_tx_rounds(0),
  TxActive(false),
  RxActive(false),
  session_active(false),
  print_transitions(false),
  has_buffer_queue(false),
  curr_state(CSMA_STATE_IDLE), 
  prev_state(CSMA_STATE_IDLE),
  prev_prev_state(CSMA_STATE_IDLE),
  ack_mode(CSMA_ACK_MODE),
  last_reason(CSMA_REASON_NOT_SET),
  start_tx_time(0),       
  srtt(0),      
  sumrtt(0),      
  sumrtt2(0),     
  rttsamples(0)
{ 
  u_pkt_id = 0;
  mac2phy_delay_ = 1e-19;
  
  bind("HDR_size_", (int*)& HDR_size);
  bind("ACK_size_", (int*)& ACK_size);
  bind("max_tx_tries_", (int*)& max_tx_tries);
  bind("wait_costant_", (double*)& wait_costant);
  bind("debug_", (double*)&debug_);
  bind("max_payload_", (int*)&max_payload);
  bind("ACK_timeout_", (double*)& ACK_timeout);
  bind("alpha_", (double*)&alpha_);
  bind("backoff_tuner_", (double*)&backoff_tuner);
  bind("buffer_pkts_", (int*)&buffer_pkts);
  bind("max_backoff_counter_", (int*)&max_backoff_counter);
  bind("listen_time_", &listen_time);
                     
  if (max_tx_tries <= 0) max_tx_tries = INT_MAX;
  if (buffer_pkts > 0) has_buffer_queue = true;
  if (listen_time <= 0.0) listen_time = 1e-19;
}

CsmaAloha::~CsmaAloha()
{

}

// TCL command interpreter
int CsmaAloha::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if (argc==2)
    {
      if(strcasecmp(argv[1], "setAckMode") == 0)
	{
	  ack_mode = CSMA_ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setNoAckMode") == 0)	
	{
          ack_mode = CSMA_NO_ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "initialize") == 0)	
	{
          if (initialized == false) initInfo();          
          if (print_transitions) fout.open("/tmp/CSMAstateTransitions.txt",ios_base::app);
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "printTransitions") == 0)	
	{
          print_transitions = true;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getQueueSize") == 0)	
	{
	  tcl.resultf("%d",Q.size());
      	  return TCL_OK;
        }
      else if(strcasecmp(argv[1],"getUpLayersDataRx") == 0)
      {
          tcl.resultf("%d",getUpLayersDataPktsRx());
          return TCL_OK;
      }
    }
    else if(argc==3){
		if(strcasecmp(argv[1],"setMacAddr") == 0)
		{
			addr = atoi(argv[2]);
			if(debug_) cout << "Csma_Aloha MAC address of current node is " << addr <<endl;
			return TCL_OK;
		}
	}
  return MMac::command(argc, argv);
}


int CsmaAloha::crLayCommand(ClMessage* m)
{
  switch (m->type()) 
    {
    
    
    default:
      return Module::crLayCommand(m);    
    }  
}


void CsmaAloha::initInfo()
{

  initialized = true;

  if ( (print_transitions) && (system(NULL)) ) {
      system("rm -f /tmp/CSMAstateTransitions.txt");
      system("touch /tmp/CSMAstateTransitions.txt");
  }

  status_info[CSMA_STATE_IDLE] = "Idle state";
  status_info[CSMA_STATE_BACKOFF] = "Backoff state"; 
  status_info[CSMA_STATE_TX_DATA] = "Transmit DATA state"; 
  status_info[CSMA_STATE_TX_ACK] = "Transmit ACK state";
  status_info[CSMA_STATE_WAIT_ACK] = "Wait for ACK state"; 
  status_info[CSMA_STATE_DATA_RX] = "DATA received state"; 
  status_info[CSMA_STATE_ACK_RX] = "ACK received state"; 
  status_info[CSMA_STATE_LISTEN] = "Listening channel state";
  status_info[CSMA_STATE_RX_IDLE] = "Start rx Idle state";
  status_info[CSMA_STATE_RX_BACKOFF] = "Start rx Backoff state";
  status_info[CSMA_STATE_RX_LISTEN] = "Start rx Listen state";
  status_info[CSMA_STATE_RX_WAIT_ACK] = "Start rx Wait ACK state";
  status_info[CSMA_STATE_CHK_LISTEN_TIMEOUT] = "Check Listen timeout state";
  status_info[CSMA_STATE_CHK_BACKOFF_TIMEOUT] = "Check Backoff timeout state";
  status_info[CSMA_STATE_CHK_ACK_TIMEOUT] = "Check Wait ACK timeout state";
  status_info[CSMA_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";
  
  reason_info[CSMA_REASON_DATA_PENDING] = "DATA pending from upper layers"; 
  reason_info[CSMA_REASON_DATA_RX] = "DATA received";
  reason_info[CSMA_REASON_DATA_TX] = "DATA transmitted"; 
  reason_info[CSMA_REASON_ACK_TX] = "ACK tranmsitted";
  reason_info[CSMA_REASON_ACK_RX] = "ACK received"; 
  reason_info[CSMA_REASON_BACKOFF_TIMEOUT] = "Backoff expired"; 
  reason_info[CSMA_REASON_ACK_TIMEOUT] = "ACK timeout"; 
  reason_info[CSMA_REASON_DATA_EMPTY] = "DATA queue empty";
  reason_info[CSMA_REASON_MAX_TX_TRIES] = "DATA dropped due to max tx rounds";
  reason_info[CSMA_REASON_LISTEN] = "DATA pending, listening to channel";
  reason_info[CSMA_REASON_LISTEN_TIMEOUT] = "DATA pending, end of listening period";
  reason_info[CSMA_REASON_START_RX] = "Start rx pkt";
  reason_info[CSMA_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
  reason_info[CSMA_REASON_BACKOFF_PENDING] = "Backoff timer pending";
  reason_info[CSMA_REASON_WAIT_ACK_PENDING] = "Wait for ACK timer pending";
  reason_info[CSMA_REASON_LISTEN_PENDING] = "Listen to channel pending";
  reason_info[CSMA_REASON_PKT_ERROR] = "Erroneous pkt";
  
  pkt_type_info[CSMA_ACK_PKT] = "ACK pkt";
  pkt_type_info[CSMA_DATA_PKT] = "DATA pkt"; 
  pkt_type_info[CSMA_DATAMAX_PKT] = "MAX payload DATA pkt";
}

void CsmaAloha::resetSession()
{   
  session_distance = SESSION_DISTANCE_NOT_SET; 
}

void CsmaAloha::updateRTT(double curr_rtt)
{
  srtt = alpha_ * srtt + (1-alpha_) * curr_rtt;
  sumrtt += curr_rtt;
  sumrtt2 += curr_rtt*curr_rtt;
  rttsamples++;
  ACK_timeout = ( srtt / rttsamples );
}

void CsmaAloha::updateAckTimeout(double rtt) {
  updateRTT(rtt);

  if (debug_) cout << NOW << "  CsmaAloha(" << addr << ")::updateAckTimeout() curr ACK_timeout = " 
                   << ACK_timeout << endl;
}

bool CsmaAloha::keepDataPkt(int serial_number) {
  bool keep_packet;
  if (serial_number > last_data_id_rx) {
    keep_packet = true;
    last_data_id_rx = serial_number;
  }
  else keep_packet = false;
  return keep_packet;
}

double CsmaAloha::computeTxTime(CSMA_PKT_TYPE type)
{
  double duration;
  Packet* temp_data_pkt;

  if (type == CSMA_DATA_PKT) {
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
  else if (type == CSMA_ACK_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = ACK_size;
  }
  duration = Mac2PhyTxDuration(temp_data_pkt );
  Packet::free(temp_data_pkt);
  return(duration);
}


void CsmaAloha::exitBackoff()
{
  backoff_timer.stop();
}


double CsmaAloha::getBackoffTime()
{
  incrTotalBackoffTimes();
  double random = RNG::defaultrng()->uniform_double();

  backoff_timer.incrCounter();
  int counter = backoff_timer.getCounter();
  if ( counter > max_backoff_counter ) counter = max_backoff_counter;

  double backoff_duration = backoff_tuner * random * 2.0 * ACK_timeout * pow( 2.0, counter );
  backoffSumDuration(backoff_duration);

  if (debug_){
       cout << NOW << "  CsmaAloha("<< addr <<")::getBackoffTime() backoff time = " 
            << backoff_duration << " s" << endl;
 
  }
  return backoff_duration;
}

void CsmaAloha::recvFromUpperLayers(Packet* p)
{ 
  if ( ((has_buffer_queue == true) && (Q.size() < buffer_pkts)) || (has_buffer_queue == false) ) 
  {
     initPkt(p , CSMA_DATA_PKT);
     Q.push(p);
     incrUpperDataRx();
     waitStartTime();

     if ( curr_state == CSMA_STATE_IDLE ) 
       {
         refreshReason(CSMA_REASON_DATA_PENDING);
         stateListen();
       }
  }
  else {
     incrDiscardedPktsTx();
     drop(p, 1, CSMA_DROP_REASON_BUFFER_FULL);
  }
}

void CsmaAloha::initPkt( Packet* p, CSMA_PKT_TYPE type, int dest_addr ) {
  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_mac* mach = HDR_MAC(p);

  int curr_size = ch->size();

  switch(type) {
  
    case(CSMA_DATA_PKT): {
      ch->size() = curr_size + HDR_size;
      data_sn_queue.push(u_data_id);
      u_data_id++;
    } 
    break;

    case(CSMA_ACK_PKT): {
      ch->ptype() = PT_MMAC_ACK;
      ch->size() = ACK_size;
      ch->uid() = u_pkt_id++;
      mach->set(MF_CONTROL,addr,dest_addr);
      mach->macSA() = addr;
      mach->macDA() = dest_addr;
    }
    break;

  }

}

void CsmaAloha::Mac2PhyStartTx(Packet* p) {
  if (debug_) cout << NOW << "  CsmaAloha("<< addr <<")::Mac2PhyStartTx() start tx packet" << endl;
  

  MMac::Mac2PhyStartTx(p);
}


void CsmaAloha::Phy2MacEndTx(const Packet* p) { 




  if (debug_) cout << NOW << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() end tx packet" << endl;

  switch(curr_state) {

    case(CSMA_STATE_TX_DATA): {
      refreshReason(CSMA_REASON_DATA_TX);
      if (ack_mode == CSMA_ACK_MODE) {

        if (debug_) cout << NOW << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() DATA sent,from "
                        << status_info[curr_state] << " to " 
                        << status_info[CSMA_STATE_WAIT_ACK] << endl;

        stateWaitAck(); 
      }
      else{

        if (debug_) cout << NOW << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() DATA sent, from " 
                        << status_info[curr_state] << " to " << status_info[CSMA_STATE_IDLE] << endl;
    
        stateIdle();
      }
    }
    break;

    case(CSMA_STATE_TX_ACK): {
      refreshReason(CSMA_REASON_ACK_TX);

      if ( prev_prev_state == CSMA_STATE_RX_BACKOFF ) {
        if (debug_) cout << NOW  << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[CSMA_STATE_CHK_BACKOFF_TIMEOUT] << endl;
          
        stateCheckBackoffExpired();
      }
      else if ( prev_prev_state == CSMA_STATE_RX_LISTEN ) {
        if (debug_) cout << NOW  << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[CSMA_STATE_CHK_LISTEN_TIMEOUT] << endl;
        
        stateCheckListenExpired();
      }
      else if ( prev_prev_state == CSMA_STATE_RX_IDLE ) {

        if (debug_) cout << NOW  << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                         << status_info[curr_state] << " to " << status_info[CSMA_STATE_IDLE] << endl;

        stateIdle();
      } 
      else {
      
        cout << NOW << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() logical error in timers, current state = " 
              << status_info[curr_state] << endl;
        stateIdle();
      }
    }
    break;

    default: {
        cout << NOW << "  CsmaAloha("<< addr <<")::Phy2MacEndTx() logical error, current state = " 
             << status_info[curr_state] << endl;
       stateIdle(); 
    }
    break;

  }

}


void CsmaAloha::Phy2MacStartRx(const Packet* p) {
  if (debug_) cout << NOW << "  CsmaAloha("<< addr <<")::Phy2MacStartRx() rx Packet " << endl; 


  refreshReason(CSMA_REASON_START_RX);

  switch(curr_state) { 
    
    case(CSMA_STATE_IDLE): 
      stateRxIdle();
    break;
    
    case(CSMA_STATE_LISTEN): 
      stateRxListen();
    break;
      
    case(CSMA_STATE_BACKOFF): 
      stateRxBackoff();
    break;
      
    case(CSMA_STATE_WAIT_ACK): 
      stateRxWaitAck();
    break;
    
    default: {
      cerr << NOW << "  CsmaAloha("<< addr << ")::Phy2MacStartRx() logical warning, current state = " 
           << status_info[curr_state] << endl;
      stateIdle();
    }
    
  }
  

}


void CsmaAloha::Phy2MacEndRx(Packet* p) {

 
  hdr_cmn* ch = HDR_CMN(p);
  packet_t rx_pkt_type = ch->ptype();
  hdr_mac* mach = HDR_MAC(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  int source_mac = mach->macSA();
  int dest_mac = mach->macDA();

  double gen_time = ph->txtime;
  double received_time = ph->rxtime;
  double diff_time = received_time - gen_time;

  double distance = diff_time * prop_speed;

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::Phy2MacEndRx() " 
                   << status_info[curr_state] << ", received a pkt type = " 
                   << ch->ptype() << ", src addr = " << mach->macSA() 
                   << " dest addr = " << mach->macDA() 
                   << ", estimated distance between nodes = " << distance << " m " << endl;

  if ( ch->error() ) {

    if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::Phy2MacEndRx() dropping corrupted pkt " << endl;
    incrErrorPktsRx();

    refreshReason(CSMA_REASON_PKT_ERROR);
    drop(p, 1, CSMA_DROP_REASON_ERROR);
    stateRxPacketNotForMe(NULL);
  }
  else {
    if ( dest_mac == addr || dest_mac == MAC_BROADCAST ) {
      if ( rx_pkt_type == PT_MMAC_ACK ) {
        refreshReason(CSMA_REASON_ACK_RX);
        stateRxAck(p);
      }
      else if ( curr_state != CSMA_STATE_RX_WAIT_ACK ) {
        refreshReason(CSMA_REASON_DATA_RX);
        stateRxData(p);
      }
      else {
        refreshReason(CSMA_REASON_PKT_NOT_FOR_ME);
        stateRxPacketNotForMe(p);
      }
    }
    else {
      refreshReason(CSMA_REASON_PKT_NOT_FOR_ME);
      stateRxPacketNotForMe(p);
    }
  }
}

void CsmaAloha::txData()
{ 
  Packet* data_pkt = curr_data_pkt->copy();  
 
  if ( (ack_mode == CSMA_NO_ACK_MODE) ) {
     queuePop();
  }                                           
 

  
  incrDataPktsTx();
  incrCurrTxRounds();
  Mac2PhyStartTx(data_pkt); 
}

void CsmaAloha::txAck( int dest_addr )
{
  Packet* ack_pkt = Packet::alloc();
  initPkt( ack_pkt , CSMA_ACK_PKT, dest_addr );

  incrAckPktsTx();
  Mac2PhyStartTx(ack_pkt);
}

void CsmaAloha::stateRxPacketNotForMe(Packet* p) {
  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateRxPacketNotForMe() pkt for another address. Dropping pkt" << endl;
  if ( p != NULL ) Packet::free(p);
  
  refreshState( CSMA_STATE_WRONG_PKT_RX );
  
  switch( prev_state ) {
  
    case CSMA_STATE_RX_IDLE:
      stateIdle();
      break;
      
    case CSMA_STATE_RX_LISTEN:
      stateCheckListenExpired();
      break;
      
    case CSMA_STATE_RX_BACKOFF:
      stateCheckBackoffExpired();
      break;
      
    case CSMA_STATE_RX_WAIT_ACK:
      stateCheckAckExpired();
      break;
      
    default:
      cerr << NOW << "  CsmaAloha("<< addr << ")::stateRxPacketNotForMe() logical error, current state = " 
           << status_info[curr_state] << endl;
      stateIdle();
      
  }
}


void CsmaAloha::stateCheckListenExpired() { 
  refreshState(CSMA_STATE_CHK_LISTEN_TIMEOUT);

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateCheckListenExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( listen_timer.isActive() ) {
    refreshReason( CSMA_REASON_LISTEN_PENDING );
    refreshState( CSMA_STATE_LISTEN );
  }
  else if ( listen_timer.isExpired() ) {
    refreshReason( CSMA_REASON_LISTEN_TIMEOUT );
    if ( !( prev_state == CSMA_STATE_TX_ACK || prev_state == CSMA_STATE_WRONG_PKT_RX
         || prev_state == CSMA_STATE_ACK_RX || prev_state == CSMA_STATE_DATA_RX ) ) stateTxData();
    else stateListen();
  }
  else {
    cerr << NOW << "  CsmaAloha("<< addr << ")::stateCheckListenExpired() listen_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    stateIdle();  
  }
}


void CsmaAloha::stateCheckAckExpired() {
  refreshState(CSMA_STATE_CHK_ACK_TIMEOUT);

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateCheckAckExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( ack_timer.isActive() ) {
    refreshReason( CSMA_REASON_WAIT_ACK_PENDING );
    refreshState( CSMA_STATE_WAIT_ACK );
  }
  else if ( ack_timer.isExpired() ) {
    refreshReason( CSMA_REASON_ACK_TIMEOUT );
    stateBackoff();
  }
  else {
    cerr << NOW << "  CsmaAloha("<< addr << ")::stateCheckAckExpired() ack_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    stateIdle();  
  }
}


void CsmaAloha::stateCheckBackoffExpired() {
  refreshState(CSMA_STATE_CHK_BACKOFF_TIMEOUT);

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateCheckBackoffExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( backoff_timer.isActive() ) {
    refreshReason( CSMA_REASON_BACKOFF_PENDING );
    stateBackoff();
  }
  else if ( backoff_timer.isExpired() ) {
    refreshReason( CSMA_REASON_BACKOFF_TIMEOUT );
    exitBackoff();
    stateIdle();
  }
  else {
    cerr << NOW << "  CsmaAloha("<< addr << ")::stateCheckBackoffExpired() backoff_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    stateIdle();  
  }
}
  
  
void CsmaAloha::stateIdle() {
  ack_timer.stop();
  backoff_timer.stop();
  listen_timer.stop();
  resetSession();
  
  refreshState(CSMA_STATE_IDLE);

  if (print_transitions) printStateInfo();

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateIdle() queue size = " << Q.size() << endl;

  if ( !Q.empty() ) {
    refreshReason(CSMA_REASON_LISTEN);
    stateListen();
  }
}


void CsmaAloha::stateRxIdle() {
  refreshState(CSMA_STATE_RX_IDLE);

  if (print_transitions) printStateInfo();
}


void CsmaAloha::stateListen() {
  listen_timer.stop();
  refreshState(CSMA_STATE_LISTEN);

  listen_timer.incrCounter();
  
  double time = listen_time * RNG::defaultrng()->uniform_double() + wait_costant;

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateListen() listen time = " << time << endl;

  if (print_transitions) printStateInfo();

  listen_timer.schedule( time );
}


void CsmaAloha::stateRxListen() {
  refreshState(CSMA_STATE_RX_LISTEN);

  if (print_transitions) printStateInfo();
}


void CsmaAloha::stateBackoff() {
  refreshState(CSMA_STATE_BACKOFF);

  if ( backoff_timer.isFrozen() ) backoff_timer.unFreeze();
  else backoff_timer.schedule( getBackoffTime() );

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateBackoff() " << endl;
  if (print_transitions) printStateInfo(backoff_timer.getDuration());
}


void CsmaAloha::stateRxBackoff() {
  backoff_timer.freeze();
  refreshState(CSMA_STATE_RX_BACKOFF);

  if (print_transitions) printStateInfo();
}


void CsmaAloha::stateTxData()
{
  refreshState(CSMA_STATE_TX_DATA);


  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateTxData() " << endl;
  if (print_transitions) printStateInfo();

  curr_data_pkt = Q.front();
                     

  if ( data_sn_queue.front() != last_sent_data_id) {
     resetCurrTxRounds();
     ack_timer.resetCounter();
     listen_timer.resetCounter();
     backoff_timer.resetCounter(); 
  }
  if ( curr_tx_rounds < max_tx_tries ) { 
     hdr_mac* mach = HDR_MAC(curr_data_pkt);
	
	 mach->macSA() = addr;
     start_tx_time = NOW; 
     last_sent_data_id = data_sn_queue.front();
     txData();
  }
  else {
    queuePop(false);
    incrDroppedPktsTx();

    refreshReason(CSMA_REASON_MAX_TX_TRIES);

    if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateTxData() curr_tx_rounds " << curr_tx_rounds
                     << " > max_tx_tries = " << max_tx_tries << endl;

    stateIdle();
  }
}

void CsmaAloha::stateWaitAck() {
  ack_timer.stop();
  refreshState(CSMA_STATE_WAIT_ACK);

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateWaitAck() " << endl;
  if (print_transitions) printStateInfo();
  
  ack_timer.incrCounter();
  ack_timer.schedule(ACK_timeout + 2*wait_costant); 
}


void CsmaAloha::stateRxWaitAck() {
  refreshState(CSMA_STATE_RX_WAIT_ACK);

  if (print_transitions) printStateInfo();
}


void CsmaAloha::stateTxAck( int dest_addr ) {
  refreshState(CSMA_STATE_TX_ACK);

  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateTxAck() dest addr " << dest_addr << endl;
  if (print_transitions) printStateInfo();
 
  txAck( dest_addr );
}


void CsmaAloha::stateRxData(Packet* data_pkt) {
  refreshState( CSMA_STATE_DATA_RX );
  
  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateRxData() in state " << status_info[curr_state] << endl;
  refreshReason( CSMA_REASON_DATA_RX );

  hdr_mac* mach = HDR_MAC(data_pkt);
  int dst_addr = mach->macSA();
  
  switch( prev_state ) {
    
    case CSMA_STATE_RX_IDLE: {
      hdr_cmn* ch = hdr_cmn::access(data_pkt);
      ch->size() = ch->size() - HDR_size;
      incrDataPktsRx();
      sendUp(data_pkt);

      if (ack_mode == CSMA_ACK_MODE) stateTxAck(dst_addr);
      else stateIdle();
    }
    break;
      
    case CSMA_STATE_RX_LISTEN: {
      hdr_cmn* ch = hdr_cmn::access(data_pkt);
      ch->size() = ch->size() - HDR_size;
      incrDataPktsRx();
      sendUp(data_pkt);

      if (ack_mode == CSMA_ACK_MODE) stateTxAck(dst_addr);
      else stateCheckListenExpired();
    }
    break;
   
    case CSMA_STATE_RX_BACKOFF: {
      hdr_cmn* ch = hdr_cmn::access(data_pkt);
      ch->size() = ch->size() - HDR_size;
      incrDataPktsRx();
      sendUp(data_pkt);       
      if (ack_mode == CSMA_ACK_MODE) stateTxAck(dst_addr);
      else stateCheckBackoffExpired();
    }
    break;
      
      
    default: 

      cerr << NOW << " CsmaAloha("<< addr << ")::stateRxData() logical error, prev state = " << status_info[prev_state]
           << endl;

  }
}


void CsmaAloha::stateRxAck(Packet* p) {
  ack_timer.stop();
  refreshState(CSMA_STATE_ACK_RX);
  if (debug_) cout << NOW << "  CsmaAloha("<< addr << ")::stateRxAck() " << endl;

  Packet::free(p);

  refreshReason(CSMA_REASON_ACK_RX);

  switch( prev_state ) {
    
    case CSMA_STATE_RX_IDLE:
      stateIdle();
      break;
      
    case CSMA_STATE_RX_LISTEN:
      stateCheckListenExpired();
      break;
      
    case CSMA_STATE_RX_BACKOFF:
      stateCheckBackoffExpired();
      break;
      
    case CSMA_STATE_RX_WAIT_ACK:
      queuePop();
      updateAckTimeout(NOW - start_tx_time);
      incrAckPktsRx();
      stateIdle();      
      break;
      
    default: 

      cerr << NOW << " CsmaAloha("<< addr << ")::stateRxAck() logical error, prev state = " << status_info[prev_state]
           << endl;

  }
}

void CsmaAloha::printStateInfo(double delay)
{
  if (debug_) cout << NOW << " CsmaAloha("<< addr << ")::printStateInfo() " << "from " << status_info[prev_state] 
                   << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;

  if (curr_state == CSMA_STATE_BACKOFF) {
      fout <<left << setw(10) << NOW << "  CsmaAloha("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] 
           << ". Backoff duration = " << delay << endl;
  }
  else {
      fout << left << setw(10) << NOW << "  CsmaAloha("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;
  }
}

void CsmaAloha::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
} 

