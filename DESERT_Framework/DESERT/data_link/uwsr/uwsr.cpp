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

/**
 * @file   uwsr.cpp
 * @author Saiful Azad
 * @version 1.0.0
 * 
 * @brief Implementation of UWSR protocol
 */


#include "uwsr.h"
#include <mac.h>
#include <cmath>
#include <climits>
#include <iomanip>
#include <rng.h>
#include <algorithm>
#include <vector>
#include <ctime>
#include <cstdlib>

/**
 * Class that represents the binding with tcl scripting language
 */
static class UWSRModuleClass : public TclClass {
public:
  /**
   * Constructor of the class
   */
  UWSRModuleClass() : TclClass("Module/UW/USR") {}
  TclObject* create(int, const char*const*) {
    return (new MMacUWSR());
  }
} class_module_uwsr;


void MMacUWSR::AckTimer::expire(Event *e) {
  timer_status = UWSR_EXPIRED;
  module->incrPktsLostCount ();

  if (module->curr_state == UWSR_STATE_WAIT_ACK || module->curr_state == UWSR_STATE_PRE_TX_DATA) {

        if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << "; ACK not received, next state = " 
                         << module->status_info[UWSR_STATE_BACKOFF] << endl;

        module->refreshReason(UWSR_REASON_ACK_TIMEOUT);
	module->eraseExpiredItemsFrommapAckandCalc();   
        module->stateBackoff();
  }
  else {
    if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ")::AckTimer::expired() in state " << module->status_info[module->curr_state] << endl;
  }  
}


void MMacUWSR::BackOffTimer::expire(Event *e) {
  timer_status = UWSR_EXPIRED;
  if (module->curr_state == UWSR_STATE_BACKOFF ) {
    
    if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ") timer expire() current state = " 
                             << module->status_info[module->curr_state] << "; backoff expired, next state = " 
                             << module->status_info[UWSR_STATE_IDLE] << endl;

    module->refreshReason(UWSR_REASON_BACKOFF_TIMEOUT);
    module->exitBackoff();
    module->stateIdle();
  }
  else {
    if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ")::BackoffTimer::expired() " << endl;
  }  
}


void MMacUWSR::ListenTimer::expire(Event *e) {   
  timer_status = UWSR_EXPIRED;

  if (module->curr_state == UWSR_STATE_LISTEN ) {

    if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ")::timer expire() current state = " 
                      << module->status_info[module->curr_state] << "; listening period expired, next state = " 
                      << module->status_info[UWSR_STATE_PRE_TX_DATA] << endl;

    module->refreshReason(UWSR_REASON_LISTEN_TIMEOUT);
    module->statePreTxData();
  }
  else {
    if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ")::ListenTimer::expired() " << endl;
  }  
}


void MMacUWSR::WaitTxTimer::expire(Event *e) {
  timer_status = UWSR_EXPIRED;

    if (module->curr_state == UWSR_STATE_PRE_TX_DATA || module->curr_state == UWSR_STATE_RX_IN_PRE_TX_DATA) {

    if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ")::timer expire() current state = " 
                      << module->status_info[module->curr_state] << "; wait tx period expired, next state = " 
                      << module->status_info[UWSR_STATE_TX_DATA] << endl;

    module->refreshReason(UWSR_REASON_WAIT_TX_TIMEOUT);
    module->stateTxData();
    }
    else {
      if (module->uwsr_debug) cout << NOW << " MMacUWSR("<< module->addr << ")::wait tx timer expired() " << endl;
    }
}


const double MMacUWSR::prop_speed = 1500.0;
bool MMacUWSR::initialized = false;


map< MMacUWSR::UWSR_STATUS , string> MMacUWSR::status_info;
map< MMacUWSR::UWSR_REASON_STATUS, string> MMacUWSR::reason_info;
map< MMacUWSR::UWSR_PKT_TYPE, string> MMacUWSR::pkt_type_info;

MMacUWSR::MMacUWSR() 
: wait_tx_timer(this),
  ack_timer(this),
  listen_timer(this),
  backoff_timer(this),
  txsn(1),
  last_sent_data_id(-1),
  curr_data_pkt(0),
  last_data_id_rx(-1),
  print_transitions(false),
  has_buffer_queue(true),
  curr_state(UWSR_STATE_IDLE), 
  prev_state(UWSR_STATE_IDLE),
  prev_prev_state(UWSR_STATE_IDLE),
  last_reason(UWSR_REASON_NOT_SET),
  start_tx_time(0),       
  srtt(0),      
  sumrtt(0),      
  sumrtt2(0),     
  rttsamples(0),
  round_trip_time(0),
  wait_tx_time(0),
  sumwtt(0),
  wttsamples(0),
  recv_data_id(-1),
  backoff_count(0),
  pkt_tx_count(0),
  curr_tx_rounds(0),
  pkts_sent_1RTT(0), 
  acks_rcv_1RTT(0),
  pkts_lost_counter(0),
  prv_mac_addr(-1),
  window_size(0),
  hit_count(0),
  total_pkts_tx(0),
  latest_ack_timeout(0)


{ 
  mac2phy_delay_ = 1e-19;
  curr_tx_rounds = 0;
  
  bind("HDR_size_", (int*)& HDR_size);
  bind("ACK_size_", (int*)& ACK_size);
  bind("max_tx_tries_", (int*)& max_tx_tries);
  bind("wait_costant_", (double*)& wait_constant);
  bind("uwsr_debug", (double*)&uwsr_debug); //degug mode
  bind("max_payload_", (int*)&max_payload);
  bind("ACK_timeout_", (double*)& ACK_timeout);
  bind("alpha_", (double*)&alpha_);
  bind("backoff_tuner_", (double*)&backoff_tuner);
  bind("buffer_pkts_", (int*)&buffer_pkts);
  bind("max_backoff_counter_", (int*)&max_backoff_counter);
  bind("listen_time_", &listen_time);
  bind("guard_time_",(double*)&guard_time);
  bind("node_speed_", (double*)&node_speed);
  bind("var_k_", (double*)&var_k);
  bind("uwsr_debug_", (int*)& uwsr_debug);
                     
  if (max_tx_tries <= 0) max_tx_tries = INT_MAX;
  if (buffer_pkts > 0) has_buffer_queue = true;
  if ( listen_time <= 0.0 ) listen_time = 1e-19;
}

MMacUWSR::~MMacUWSR()
{

}

// TCL command interpreter
int MMacUWSR::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if (argc==2)
    {
      if(strcasecmp(argv[1], "initialize") == 0)	
	{
          if (initialized == false) initInfo();          
          if (print_transitions) fout.open("/tmp/UWSRstateTransitions.txt",ios_base::app);
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "printTransitions") == 0)	
	{
          print_transitions = true;
	  return TCL_OK;
	}
      // stats functions
      else if(strcasecmp(argv[1], "getQueueSize") == 0)	
	{
	  tcl.resultf("%d",mapPacket.size());
      	  return TCL_OK;
        }
      else if (strcasecmp(argv[1], "getBackoffCount") == 0)	
	{
	  tcl.resultf("%d",getBackoffCount());
      	  return TCL_OK;
        }
      else if (strcasecmp(argv[1], "getAvgPktsTxIn1RTT") == 0)	
	{
	  tcl.resultf("%f",getAvgPktsTxIn1RTT());
      	  return TCL_OK;
        }
    }
    else if(argc==3){
		if(strcasecmp(argv[1],"setMacAddr") == 0)
		{
			addr = atoi(argv[2]);
			if(debug_) cout << "UwSR MAC address of current node is " << addr <<endl;
			return TCL_OK;
		}
	}
  return MMac::command(argc, argv);
}

void MMacUWSR::initInfo()
{

  initialized = true;

  if ( (print_transitions) && (system(NULL)) ) {
      system("rm -f /tmp/UWSRstateTransitions.txt");
      system("touch /tmp/UWSRstateTransitions.txt");
  }

  status_info[UWSR_STATE_IDLE] = "Idle state";
  status_info[UWSR_STATE_BACKOFF] = "Backoff state"; 
  status_info[UWSR_STATE_TX_DATA] = "Transmit DATA state"; 
  status_info[UWSR_STATE_TX_ACK] = "Transmit ACK state";
  status_info[UWSR_STATE_WAIT_ACK] = "Wait for ACK state"; 
  status_info[UWSR_STATE_DATA_RX] = "DATA received state"; 
  status_info[UWSR_STATE_ACK_RX] = "ACK received state"; 
  status_info[UWSR_STATE_LISTEN] = "Listening channel state";
  status_info[UWSR_STATE_RX_IDLE] = "Start rx Idle state";
  status_info[UWSR_STATE_RX_BACKOFF] = "Start rx Backoff state";
  status_info[UWSR_STATE_RX_LISTEN] = "Start rx Listen state";
  status_info[UWSR_STATE_RX_WAIT_ACK] = "Start rx Wait ACK state";
  status_info[UWSR_STATE_CHK_LISTEN_TIMEOUT] = "Check Listen timeout state";
  status_info[UWSR_STATE_CHK_BACKOFF_TIMEOUT] = "Check Backoff timeout state";
  status_info[UWSR_STATE_CHK_ACK_TIMEOUT] = "Check Wait ACK timeout state";
  status_info[UWSR_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";
  status_info[UWSR_STATE_WAIT_TX] = "Waiting for transmitting another packet";
  status_info[UWSR_STATE_CHK_WAIT_TX_TIMEOUT] = "Check wait tx timeout state";
  status_info[UWSR_STATE_WAIT_ACK_WAIT_TX] = "Moving from wait tx state to rx wait ack state";
  status_info[UWSR_STATE_RX_DATA_TX_DATA] = "Data receive in txData state and moving to new state";
  
  reason_info[UWSR_REASON_DATA_PENDING] = "DATA pending from upper layers"; 
  reason_info[UWSR_REASON_DATA_RX] = "DATA received";
  reason_info[UWSR_REASON_DATA_TX] = "DATA transmitted"; 
  reason_info[UWSR_REASON_ACK_TX] = "ACK tranmsitted";
  reason_info[UWSR_REASON_ACK_RX] = "ACK received"; 
  reason_info[UWSR_REASON_BACKOFF_TIMEOUT] = "Backoff expired"; 
  reason_info[UWSR_REASON_ACK_TIMEOUT] = "ACK timeout"; 
  reason_info[UWSR_REASON_DATA_EMPTY] = "DATA queue empty";
  reason_info[UWSR_REASON_MAX_TX_TRIES] = "DATA dropped due to max tx rounds";
  reason_info[UWSR_REASON_LISTEN] = "DATA pending, listening to channel";
  reason_info[UWSR_REASON_LISTEN_TIMEOUT] = "DATA pending, end of listening period";
  reason_info[UWSR_REASON_START_RX] = "Start rx pkt";
  reason_info[UWSR_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
  reason_info[UWSR_REASON_BACKOFF_PENDING] = "Backoff timer pending";
  reason_info[UWSR_REASON_WAIT_ACK_PENDING] = "Wait for ACK timer pending";
  reason_info[UWSR_REASON_LISTEN_PENDING] = "Listen to channel pending";
  reason_info[UWSR_REASON_PKT_ERROR] = "Erroneous pkt";
  reason_info[UWSR_REASON_WAIT_TX] = "Waiting for transmitting another packet";
  reason_info[UWSR_REASON_WAIT_TX_PENDING] = "Transmission pending";
  reason_info[UWSR_REASON_WAIT_TX_TIMEOUT] = "Waiting for tx timeout";
  
  pkt_type_info[UWSR_ACK_PKT] = "ACK pkt";
  pkt_type_info[UWSR_DATA_PKT] = "DATA pkt"; 
  pkt_type_info[UWSR_DATAMAX_PKT] = "MAX payload DATA pkt";
}

void MMacUWSR::updateTxStatus (macAddress mac_addr, int rcv_acks) 
{
  map<macAddress,txStatusPair>::iterator it_tx;
  it_tx = mapTxStatus.find(mac_addr);

  if (it_tx == mapTxStatus.end()) {
	mapTxStatus.insert(make_pair(mac_addr, make_pair(getPktsSentIn1RTT(), rcv_acks)));
  }
  else {
	if ((it_tx->second).first != getPktsSentIn1RTT()) (it_tx->second).first = getPktsSentIn1RTT();
	if ((it_tx->second).second != rcv_acks) (it_tx->second).second = rcv_acks;
  }
}

int MMacUWSR::calWindowSize(macAddress mac_addr)
{
  map<macAddress,txStatusPair>::iterator it_tx;
  it_tx = mapTxStatus.find(mac_addr);

  if (it_tx == mapTxStatus.end()) {
	window_size = 1;
  }
  else {
	if ((it_tx->second).first == (it_tx->second).second) window_size = max(window_size, ((it_tx->second).first + 1));
        else window_size = (floor((it_tx->second).first * var_k));
  }
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::window size " << max(1, window_size) << endl;
  return max(1, window_size);
}

void MMacUWSR::putRTTInMap(int mac_addr, double rtt) {
  
  double time = NOW;
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::putRTTInMap() mac add " << mac_addr << "rtt " << rtt << "time " << time << endl;
  
  map<macAddress, rttPair> :: iterator it_d;
  it_d = mapRTT.find(mac_addr);
  
  if ( it_d == mapRTT.end() ) {
    mapRTT.insert(make_pair(mac_addr, make_pair(rtt,time)));
  }
  else {
    if (rtt != (it_d->second).first || time != (it_d->second).second) {
      (it_d->second).first = rtt;
      (it_d->second).second = time;
    }
    else {
      //do nothing. keep the RTT saved in the map
    }
  }

}


int MMacUWSR::getPktsCanSendIn1RTT(int mac_addr) {
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::getPktsCanSendIn1RTT() rcv mac addr " << mac_addr << endl;
  
  int pkts_can_send_1RTT = 1;
  
  
  map<macAddress,rttPair> :: iterator it_d = mapRTT.find(mac_addr);
  
  if (it_d != mapRTT.end()) {
	
    double tx_time = (computeTxTime(UWSR_DATA_PKT) + computeTxTime(UWSR_ACK_PKT) + guard_time);
	
    double apprx_travel_dis = 2 * node_speed * (NOW - (it_d->second).second);
    double apprx_curr_rtt = (it_d->second).first - (apprx_travel_dis / prop_speed);
    
    pkts_can_send_1RTT = max (1, (int)(floor ( apprx_curr_rtt / tx_time )));
     	
  }    
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::No pkts can send in 1 RTT is " << pkts_can_send_1RTT << endl;
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::Window size: " << window_size << endl;
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Pkts can transmit in 1 RTT is " << min (window_size, pkts_can_send_1RTT) << endl;
  return min(calWindowSize(mac_addr), pkts_can_send_1RTT);
}


bool MMacUWSR::chkItemInmapTxRounds(int mac_addr, int seq_num) { 
  map< usrPair, txRounds> :: iterator it_t; 
  it_t = mapTxRounds.find(make_pair(mac_addr,seq_num));
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::chkItemInmapTxRounds()"<< endl;
  if (it_t != mapTxRounds.end()) return true;
  else return false;
}

double MMacUWSR::calcWaitTxTime(int mac_addr) {
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::calcWaitTxTime() rcv mac addr " << mac_addr << endl;
  
  double wait_time;
  double rtt_time;
  double pkts_can_tx;
  
  map<macAddress,rttPair> :: iterator it_d;
  it_d = mapRTT.find(mac_addr);
  
  if (it_d == mapRTT.end()) {
    cerr << NOW << " MMacUWSR("<< addr <<")::calcWaitTxTime() is accessed in inappropriate time" << endl;
    exit(1);
  }
  else rtt_time = (it_d->second).first; 
  pkts_can_tx = getPktsCanSendIn1RTT(mac_addr);
   
  wait_time = ((computeTxTime(UWSR_ACK_PKT) / 2 + rtt_time - (pkts_can_tx - 1) * computeTxTime(UWSR_DATA_PKT)) / (pkts_can_tx - 0.5));
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::round trip time " << rtt_time << endl;
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::calcWaitTxTime() wait time " << wait_time << endl;
  return wait_time;
}


bool MMacUWSR::checkMultipleTx(int rcv_mac_addr) { 
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::checkMultipleTx() rcv mac addr " << rcv_mac_addr << endl;
  
  Packet* nxt_data_pkt;
  
  map< usrPair,Packet*> :: iterator it_p;
  map< usrPair,AckTimer> :: iterator it_a;
  
  int nxt_mac_addr;
  
  if (mapPacket.size() == 0) return false;
  else if (mapPacket.size() <= mapAckTimer.size()) return false;
  else if (getPktsCanSendIn1RTT(rcv_mac_addr) < 2) return false;
  else {
     for (it_p = mapPacket.begin(); it_p != mapPacket.end(); it_p++) {
	it_a = mapAckTimer.find((*it_p).first);
	if ((*it_a).first != (*it_p).first) {
		nxt_data_pkt = (*it_p).second;
		if (uwsr_debug) cout<< NOW << " MMacUWSR("<< addr <<")::Next Packet transmitting: "<<nxt_data_pkt<<endl;
		hdr_mac* mach = HDR_MAC(nxt_data_pkt);
		nxt_mac_addr = mach->macDA();
		if (rcv_mac_addr == nxt_mac_addr) break;
	}
	else nxt_mac_addr = -1;
    }
    
    if (rcv_mac_addr == nxt_mac_addr && getPktsCanSendIn1RTT(rcv_mac_addr) > getPktsSentIn1RTT ()) return true;
    else return false;
  }
}


int MMacUWSR::checkAckTimer (CHECK_ACK_TIMER type) {

  int iteration_count = 0;
  int active_count = 0;
  int idle_count = 0;
  int expired_count = 0;
  int value = 0;
  
  map< usrPair,AckTimer> :: iterator it_a;

  for (it_a = mapAckTimer.begin(); it_a != mapAckTimer.end(); it_a++) {
	if (((*it_a).second).isActive()){
		active_count += 1;
	}
	else if (((*it_a).second).isExpired()) {
		expired_count += 1;
	}
	else if (((*it_a).second).isIdle()) {
		idle_count += 1;
	}
	else {
		 cerr<<"Ack Timer is in wrong state"<<endl;
		exit(1);
	}
	iteration_count += 1; 
  }
  
   if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::No of item in ack map: "<<mapAckTimer.size()<<endl;
   if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::No of iteration count: "<<iteration_count<<endl; 
   if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::No of active count: "<<active_count<<endl;
   if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::No of expired count: "<<expired_count<<endl;
   if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::No of idle count: "<<idle_count<<endl;
  

  if (type == CHECK_ACTIVE) {
    if (mapAckTimer.size() == 0) {
	value = 1;
    }
    else {
	value = floor(active_count / mapAckTimer.size());
    }
  }
  else if (type == CHECK_EXPIRED) {
	value = expired_count;
  }
  else if (type == CHECK_IDLE) {
	value = idle_count;
  }
  else {
	if (uwsr_debug) cout<< NOW << " MMacUWSR("<< addr <<")::Ack Timer is in wrong state"<<endl;
	exit(1);
  }
  
	if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::returning value: "<<value<<endl;
  	return value;

}

void MMacUWSR::eraseExpiredItemsFrommapAckandCalc() {
  
  if (uwsr_debug) cout<< NOW << " MMacUWSR("<< addr <<")::Erasing expired items from map ack and calc"<<endl;
  
  map< usrPair,AckTimer> :: iterator it_a1, it_a2;
  
  it_a1 = mapAckTimer.begin();
  
  while (it_a1 != mapAckTimer.end()) {
    
    it_a2 = it_a1;
    it_a1++;
    
    if (((*it_a2).second).isExpired()) {
		int mac_addr = (it_a2->first).first;
		int seq_num = (it_a2->first).second;
		eraseItemFrommapAckTimer(mac_addr, seq_num);
		eraseItemFrommapCalcAck(mac_addr, seq_num);
	}
    else {
	// do nothing
	}
  }
}


double MMacUWSR::computeTxTime(UWSR_PKT_TYPE type)
{
  map< usrPair,Packet*> :: iterator it_p;
  
  double duration;
  Packet* temp_data_pkt;

  if (type == UWSR_DATA_PKT) {
     if (!mapPacket.empty()) {
        it_p = mapPacket.begin();
        temp_data_pkt = ((*it_p).second)->copy();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = HDR_size + ch->size();
     }
     else { 
        temp_data_pkt = Packet::alloc();
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = HDR_size + max_payload;
     }
  }
  else if (type == UWSR_ACK_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = ACK_size;
  }
  duration = Mac2PhyTxDuration(temp_data_pkt );
  Packet::free(temp_data_pkt);
  return(duration);
}


void MMacUWSR::exitBackoff()
{
  backoff_timer.stop();
}


double MMacUWSR::getBackoffTime()
{
  incrTotalBackoffTimes();
  double random = RNG::defaultrng()->uniform_double();

  backoff_timer.incrCounter();
  double counter = backoff_timer.getCounter();
  if ( counter > max_backoff_counter ) counter = max_backoff_counter;

  double backoff_duration = backoff_tuner * random * 2.0 * ACK_timeout * pow( 2.0, counter );
  backoffSumDuration(backoff_duration);

  if (uwsr_debug) {
        cout << NOW << " MMacUWSR("<< addr <<")::getBackoffTime() backoff time = " 
            << backoff_duration << " latest ack timeout = " << (latest_ack_timeout - NOW) << endl; 
  }
  
  return max((latest_ack_timeout - NOW) + wait_constant, backoff_duration);
}

void MMacUWSR::recvFromUpperLayers(Packet* p)
{ 
  if ( ((has_buffer_queue == true) && (mapPacket.size() < buffer_pkts)) || (has_buffer_queue == false) ) {
     initPkt(p , UWSR_DATA_PKT);
     putPktInQueue(p);
     incrUpperDataRx();
     waitStartTime();

     if ( curr_state == UWSR_STATE_IDLE ) 
       {
         refreshReason(UWSR_REASON_DATA_PENDING);
         stateListen();
       }
  }
  else {
     incrDiscardedPktsTx();
     drop(p, 1, UWSR_DROP_REASON_BUFFER_FULL);
  }
}

void MMacUWSR::initPkt( Packet* p, UWSR_PKT_TYPE type, int dest_addr ) {
  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_mac* mach = HDR_MAC(p);

  int curr_size = ch->size();

  switch(type) {
  
    case(UWSR_DATA_PKT): {
      ch->size() = curr_size + HDR_size;
    } 
    break;

    case(UWSR_ACK_PKT): {
      ch->ptype() = PT_MMAC_ACK;
      ch->size() = ACK_size;
      ch->uid() = recv_data_id;
      mach->set(MF_CONTROL,addr,dest_addr);
      mach->macSA() = addr;
      mach->macDA() = dest_addr;
    }
    break;

  }

}

void MMacUWSR::Mac2PhyStartTx(Packet* p) {
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Mac2PhyStartTx() start tx packet" << endl;

  MMac::Mac2PhyStartTx(p);
  
}


void MMacUWSR::Phy2MacEndTx(const Packet* p) { 
  
  hdr_cmn* ch = hdr_cmn::access(p);
  int seq_num = ch -> uid();
  
  hdr_mac* mach = HDR_MAC(p);
  int dst_mac_addr = mach->macDA();
  prv_mac_addr = dst_mac_addr;

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Phy2MacEndTx() end tx packet" << endl;
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Phy2MacEndTx() dst mac addr " << dst_mac_addr << endl;

  switch(curr_state) {

    case(UWSR_STATE_TX_DATA): {
	refreshReason(UWSR_REASON_DATA_TX);
	
	incrPktsSentIn1RTT ();
	double wait_time, ack_time;
	double ack_timeout_time;
	
	map<int, rttPair> :: iterator it_d;
	it_d = mapRTT.find(dst_mac_addr);
	
	if (it_d == mapRTT.end()) ack_timeout_time = ACK_timeout + 2*wait_constant;
	else ack_timeout_time = getRTTInMap(dst_mac_addr) + 2*wait_constant;
	
	ack_time = NOW + ack_timeout_time;

	putAckTimerInMap(dst_mac_addr, seq_num);
	map< usrPair,AckTimer> :: iterator it_a;
	it_a = mapAckTimer.find(make_pair(dst_mac_addr,seq_num));
        ((*it_a).second).stop();
	((*it_a).second).schedule(ack_timeout_time);
	
	if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Phy2MacEndTx(), ack_timeout_time: " << ack_time << " latest_ack_timeout " << latest_ack_timeout << endl;
	if (ack_time > latest_ack_timeout) latest_ack_timeout = ack_time;
	if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Phy2MacEndTx(), ack_timeout_time: " << ack_time << " latest_ack_timeout " << latest_ack_timeout << endl;

	if(checkMultipleTx(dst_mac_addr)) {  
	if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Phy2MacEndTx() DATA sent,from "
                        << status_info[curr_state] << " to " << status_info[UWSR_STATE_PRE_TX_DATA] << endl;

		wait_time = calcWaitTxTime(dst_mac_addr);
		wait_tx_timer.stop();
		wait_tx_timer.incrCounter();
		wait_tx_timer.schedule(wait_time);

        	statePreTxData(); 	  
	}
	
	else {
	  
	if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Phy2MacEndTx() DATA sent,from "
                        << status_info[curr_state] << " to " << status_info[UWSR_STATE_WAIT_ACK] << endl;

	    	updateTxStatus(dst_mac_addr, 0);
		calTotalPktsTx();
	 	stateWaitAck(); 
	}
      
    }
    break;

    case(UWSR_STATE_TX_ACK): {
      refreshReason(UWSR_REASON_ACK_TX);

      if ( prev_prev_state == UWSR_STATE_RX_BACKOFF ) {
      if (uwsr_debug) cout << NOW  << " MMacUWSR("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[UWSR_STATE_CHK_BACKOFF_TIMEOUT] << endl;
          
        stateCheckBackoffExpired();
      }
      else if ( prev_prev_state == UWSR_STATE_RX_WAIT_ACK ) {
      if (uwsr_debug) cout << NOW  << " MMacUWSR("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[UWSR_STATE_CHK_BACKOFF_TIMEOUT] << endl;
          
        stateCheckAckExpired();
      }
      else if ( prev_prev_state == UWSR_STATE_RX_IN_PRE_TX_DATA ) {
      if (uwsr_debug) cout << NOW  << " MMacUWSR("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[UWSR_STATE_CHK_BACKOFF_TIMEOUT] << endl;
          
        stateCheckWaitTxExpired();
      }
      else if ( prev_prev_state == UWSR_STATE_RX_LISTEN ) {
      if (uwsr_debug) cout << NOW  << " MMacUWSR("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[UWSR_STATE_CHK_LISTEN_TIMEOUT] << endl;
        
        stateCheckListenExpired();
      }
      else if ( prev_prev_state == UWSR_STATE_RX_IDLE ) {
      if (uwsr_debug) cout << NOW  << " MMacUWSR("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                         << status_info[curr_state] << " to " << status_info[UWSR_STATE_IDLE] << endl;

        stateIdle();
      } 
      else {
        cerr << NOW << " MMacUWSR("<< addr <<")::Phy2MacEndTx() logical error in timers, current state = " 
              << status_info[curr_state] << endl;
        exit(1);
      }
      
    }
    break;

    default: {
        cerr << NOW << "  MMacUWSR("<< addr <<")::Phy2MacEndTx() logical error, current state = " 
             << status_info[curr_state] << endl;
        exit(1);
    }
    break;

  }

}


void MMacUWSR::Phy2MacStartRx(const Packet* p) {
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr <<")::Phy2MacStartRx() rx Packet " << endl; 

  refreshReason(UWSR_REASON_START_RX);

  switch(curr_state) { 
   
    case(UWSR_STATE_IDLE): 
     stateRxIdle();
     break;

    case(UWSR_STATE_LISTEN): 
     stateRxListen();
     break;
     
    case(UWSR_STATE_BACKOFF): 
      stateRxBackoff();
      break;
    
    case(UWSR_STATE_PRE_TX_DATA):
      stateRxinPreTxData();
      break;
     
    case(UWSR_STATE_WAIT_ACK): 
     stateRxWaitAck(); 
     break;
    
    default: {
      cerr << NOW << "  MMacUWSR("<< addr << ")::Phy2MacStartRx() logical warning, current state = " 
           << status_info[curr_state] << endl;
    }
    
  }
  

}


void MMacUWSR::Phy2MacEndRx(Packet* p) {
 
  hdr_cmn* ch = HDR_CMN(p);
  packet_t rx_pkt_type = ch->ptype();
  hdr_mac* mach = HDR_MAC(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  int source_mac = mach->macSA();
  int dest_mac = mach->macDA();

  double gen_time = ph->txtime;
  double received_time = ph->rxtime;
  double diff_time = received_time - gen_time;
  putRTTInMap(source_mac, 2*diff_time);

  double distance = diff_time * prop_speed;
  int seq_num = getPktSeqNum(p);
  map< usrPair,AckTimer> :: iterator it_a;

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::Phy2MacEndRx() " 
                   << status_info[curr_state] << ", received a pkt type = " 
                   << ch->ptype() << ", src addr = " << mach->macSA() 
                   << " dest addr = " << mach->macDA() << " RTT = " << 2*diff_time
                   << ", estimated distance between nodes = " << distance << " m " << endl;

  if ( ch->error() ) {

    if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::Phy2MacEndRx() dropping corrupted pkt " << endl;
    incrErrorPktsRx();

    refreshReason(UWSR_REASON_PKT_ERROR);
    drop(p, 1, UWSR_DROP_REASON_ERROR);
    stateRxPacketNotForMe(NULL);
  }
  else {
    if ( dest_mac == addr || dest_mac == MAC_BROADCAST ) {
      if ( rx_pkt_type == PT_MMAC_ACK ) {
	it_a = mapAckTimer.find(make_pair(source_mac, seq_num));
	if (it_a != mapAckTimer.end()) {
	  refreshReason(UWSR_REASON_ACK_RX);
	  stateRxAck(p);
	}
	else {
	  drop(p, 1, UWSR_DROP_REASON_ERROR);
	  stateRxPacketNotForMe(NULL);
	}      
      }
      else {
        refreshReason(UWSR_REASON_DATA_RX);
        stateRxData(p);
      }
    }
    else {
      refreshReason(UWSR_REASON_PKT_NOT_FOR_ME);
      stateRxPacketNotForMe(p);
    }
  }
}

void MMacUWSR::stateTxData()
{ 
  refreshState(UWSR_STATE_TX_DATA);

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateTxData" << endl;
	
  Packet* data_pkt = curr_data_pkt->copy();      // copio pkt 
  int seq_num = getPktSeqNum(data_pkt);
  int mac_addr = getMacAddress(data_pkt);
  
  start_tx_time = NOW;
  map< usrPair,txStartTime> :: iterator it_ca;
  it_ca = mapCalcAck.find(make_pair(mac_addr, seq_num));
  if (it_ca == mapCalcAck.end()) putStartTxTimeInMap(mac_addr,seq_num,start_tx_time);
  
  incrDataPktsTx();

  Mac2PhyStartTx(data_pkt);

}

void MMacUWSR::txAck( int dest_addr )
{
  Packet* ack_pkt = Packet::alloc();
  initPkt( ack_pkt , UWSR_ACK_PKT, dest_addr );

  incrAckPktsTx();
  Mac2PhyStartTx(ack_pkt);
}

void MMacUWSR::stateRxPacketNotForMe(Packet* p) {
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateRxPacketNotForMe() pkt for another address. Dropping pkt" << endl;
  if ( p != NULL ) Packet::free(p);
  
  refreshState( UWSR_STATE_WRONG_PKT_RX );
 
  
    switch( prev_state ) {
  
    case UWSR_STATE_RX_IDLE:
      stateIdle();
      break;
      
    case UWSR_STATE_RX_LISTEN:
      stateCheckListenExpired();
      break;
      
    case UWSR_STATE_RX_BACKOFF:
      stateCheckBackoffExpired();
      break;
    
     case UWSR_STATE_RX_IN_PRE_TX_DATA:
      stateCheckWaitTxExpired();
      break;
      
    case UWSR_STATE_RX_WAIT_ACK:
      stateCheckAckExpired();
      break;
      
    default: 
      cerr << NOW << "  MMacUWSR("<< addr << ")::stateRxPacketNotForMe() logical error, current state = " 
           << status_info[curr_state] << endl;
      curr_state = prev_state;
      prev_state = prev_prev_state;
      break;
      
  }
  
}


void MMacUWSR::stateCheckListenExpired() { 
  refreshState(UWSR_STATE_CHK_LISTEN_TIMEOUT);

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateCheckListenExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( listen_timer.isActive() ) {
    refreshReason( UWSR_REASON_LISTEN_PENDING );
    refreshState( UWSR_STATE_LISTEN );
  }
  else if ( listen_timer.isExpired() ) {
    refreshReason( UWSR_REASON_LISTEN_TIMEOUT );
    if ( !( prev_state == UWSR_STATE_TX_ACK || prev_state == UWSR_STATE_WRONG_PKT_RX
         || prev_state == UWSR_STATE_ACK_RX || prev_state == UWSR_STATE_DATA_RX ) ) stateTxData();
    else stateListen();
  }
  else {
    cerr << NOW << "  MMacUWSR("<< addr << ")::stateCheckListenExpired() listen_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    exit(1);  
  }
}


void MMacUWSR::stateCheckAckExpired() 
{
  refreshState(UWSR_STATE_CHK_ACK_TIMEOUT);

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateCheckAckExpired()" << endl;
  if (print_transitions) printStateInfo();
  
  if (mapAckTimer.size() == 0) stateIdle();
  else if (checkAckTimer(CHECK_ACTIVE)) {
    refreshReason( UWSR_REASON_WAIT_ACK_PENDING );
    refreshState( UWSR_STATE_WAIT_ACK );
  }
  else if (checkAckTimer(CHECK_EXPIRED) > 0 ) {
    refreshReason( UWSR_REASON_ACK_TIMEOUT );
    eraseExpiredItemsFrommapAckandCalc();
    stateBackoff();
  }	
  else {
	  cerr << NOW << "  MMacUWSR("<< addr << ")::stateCheckAckExpired() ack_timer logical error, current timer state = " 
	  << status_info[curr_state] << endl;
	  exit(1); 
  }

}


void MMacUWSR::stateCheckBackoffExpired() {
  refreshState(UWSR_STATE_CHK_BACKOFF_TIMEOUT);

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateCheckBackoffExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( backoff_timer.isActive() ) {
    refreshReason( UWSR_REASON_BACKOFF_PENDING );
    stateBackoff();
  }
  else if ( backoff_timer.isExpired() ) {
    refreshReason( UWSR_REASON_BACKOFF_TIMEOUT );
    exitBackoff();
    stateIdle();
  }
  else {
    cerr << NOW << "  MMacUWSR("<< addr << ")::stateCheckBackoffExpired() backoff_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    exit(1);  
  }
}

void MMacUWSR::stateCheckWaitTxExpired() {
  
  refreshState(UWSR_STATE_CHK_WAIT_TX_TIMEOUT);

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateCheckWaitTxExpired()" << endl;
   
  if (print_transitions) printStateInfo();
  
  if (checkAckTimer(CHECK_EXPIRED) > 0) {
    refreshReason( UWSR_REASON_ACK_TIMEOUT );
    eraseExpiredItemsFrommapAckandCalc();
    stateBackoff();	
  }

  else if (checkAckTimer(CHECK_ACTIVE)) {
     if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::wait tx is active:" << wait_tx_timer.isActive()<<endl;
     if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::wait tx is expired:" << wait_tx_timer.isExpired()<< endl;
     if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::wait tx is idle:" << wait_tx_timer.isIdle()<< endl;

    if ( wait_tx_timer.isActive() ) {
	refreshReason(UWSR_REASON_WAIT_TX_PENDING );
	refreshState(UWSR_STATE_PRE_TX_DATA);
	}
	  
    else if ( wait_tx_timer.isExpired() ) {
	refreshReason( UWSR_REASON_WAIT_TX_TIMEOUT );
	refreshState(UWSR_STATE_PRE_TX_DATA);
	stateTxData();
	}
	  
     else {
	  cerr << NOW << "  MMacUWSR("<< addr << ")::stateCheckWaitTxExpired() wait_tx_timer logical error, current timer state = " << status_info[curr_state] << endl;
	  exit(1);  
	}
  }

  else {
      
     cerr << NOW << "  MMacUWSR("<< addr << ")::stateCheckAckExpired() ack_timer logical error, current timer state = " 
	  << status_info[curr_state] << endl;
     exit(1);   

      }
  }
  
    
void MMacUWSR::stateIdle() {

  rstPktsLostCount ();
  if (uwsr_debug) cout <<  NOW << " MMacUWSR("<< addr << ")::stateIdle(), Pkts sent in one RTT: " << getPktsSentIn1RTT() << endl;
  rstPktsSentIn1RTT ();
  rstAcksRcvIn1RTT();
  backoff_timer.stop();
  listen_timer.stop();
  wait_tx_timer.stop();

  refreshState(UWSR_STATE_IDLE);

  if (print_transitions) printStateInfo();

  if ( !mapPacket.empty() ) {
    refreshReason(UWSR_REASON_LISTEN);
    stateListen();
  }
}


void MMacUWSR::stateRxIdle() {
  refreshState(UWSR_STATE_RX_IDLE);

  if (print_transitions) printStateInfo();
}


void MMacUWSR::stateListen() {
  listen_timer.stop();
  refreshState(UWSR_STATE_LISTEN);

  listen_timer.incrCounter();
  
  double time = listen_time * RNG::defaultrng()->uniform_double() + wait_constant;

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateListen() listen time = " << time << endl;

  if (print_transitions) printStateInfo();

  listen_timer.schedule( time );
}


void MMacUWSR::stateRxListen() {
  refreshState(UWSR_STATE_RX_LISTEN);

  if (print_transitions) printStateInfo();
}


void MMacUWSR::stateBackoff() {

  rstPktsLostCount ();
  wait_tx_timer.stop();
  
  refreshState(UWSR_STATE_BACKOFF);

  setBackoffCount();
  
  if ( backoff_timer.isFrozen() ) backoff_timer.unFreeze();
  else backoff_timer.schedule( getBackoffTime() );
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateBackoff() get backoff time " << getBackoffTime() << endl;

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateBackoff() " << endl;
  if (print_transitions) printStateInfo(backoff_timer.getDuration());
}


void MMacUWSR::stateRxBackoff() {
  backoff_timer.freeze();
  refreshState(UWSR_STATE_RX_BACKOFF);

  if (print_transitions) printStateInfo();
}


bool MMacUWSR::prepBeforeTx(int mac_addr, int seq_num) {
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::prepBeforeTx(), is item in tx rounds map " << chkItemInmapTxRounds(mac_addr, seq_num) << endl;
  
  if (chkItemInmapTxRounds(mac_addr, seq_num)) { 
  	if (  getCurrTxRounds(mac_addr, seq_num) < max_tx_tries + 1 ) {
 
	  last_sent_data_id = seq_num;
	  incrCurrTxRounds(mac_addr, seq_num);
	  return true;
    	}
    	else {
	  eraseItemFromPktQueue(mac_addr, seq_num);
	  eraseItemFromTxRounds(mac_addr, seq_num);
	  eraseItemFrommapAckTimer(mac_addr, seq_num);
	  incrDroppedPktsTx();

	  refreshReason(UWSR_REASON_MAX_TX_TRIES);

	  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateTxData() curr_tx_rounds " << curr_tx_rounds
                		<< " > max_tx_tries = " << max_tx_tries << endl;
	  return false;
      	}
  }
  else {
    	listen_timer.resetCounter();
  	backoff_timer.resetCounter(); 
	
	setCurrTxRounds(mac_addr, seq_num);
	return true;
  
    }
}


void MMacUWSR::statePreTxData() {
   
  refreshState(UWSR_STATE_PRE_TX_DATA);
  
   if (uwsr_debug) cout<< NOW << " MMacUWSR("<< addr << ")::Time require to transmit a data packet: "<<computeTxTime(UWSR_DATA_PKT)<<endl;
   if (uwsr_debug) cout<< NOW << " MMacUWSR("<< addr << ")::Time require to transmit an ack packet: "<<computeTxTime(UWSR_ACK_PKT)<<endl;  
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::statePreTxData() " << endl;
  if (print_transitions) printStateInfo();
  
  map< usrPair,Packet*> :: iterator it_p;
  map< usrPair,AckTimer> :: iterator it_a;
  
  int curr_mac_addr;
  int seq_num;

  it_p = mapPacket.begin();
	if (mapPacket.size() == 0) {
	  stateIdle();
	}
	else if (mapAckTimer.size() == 0) {
		curr_data_pkt = (*it_p).second;
   		if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::Packet transmitting: "<<curr_data_pkt<<endl;
    		seq_num = getPktSeqNum(curr_data_pkt);
   		if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::seq_num: "<<seq_num<<endl;
		
		hdr_mac* mach = HDR_MAC(curr_data_pkt);
		curr_mac_addr = mach->macDA();
		
		if (prepBeforeTx(curr_mac_addr, seq_num)) {
		    if (prev_state == UWSR_STATE_LISTEN) {
			stateTxData();
		    }
		    else {
			stateCheckWaitTxExpired();
		    }	
		}
		else stateIdle();  	
	}
	else if (mapPacket.size() > mapAckTimer.size()) {
// 		int seq_num;
		for (it_p = mapPacket.begin(); it_p != mapPacket.end(); it_p++) {
			it_a = mapAckTimer.find((*it_p).first);
			if ((*it_a).first != (*it_p).first) {
	  			//it_p = mapPacket.begin();
				curr_data_pkt = (*it_p).second;
	 			if (uwsr_debug) cout<< NOW << " MMacUWSR("<< addr << ")::Packet transmitting: "<<curr_data_pkt<<endl;
	  			seq_num = getPktSeqNum(curr_data_pkt);
	 			if (uwsr_debug) cout<< NOW << " MMacUWSR("<< addr << ")::seq_num: "<<seq_num<<endl;
				hdr_mac* mach = HDR_MAC(curr_data_pkt);
				curr_mac_addr = mach->macDA();
				if (prev_state == UWSR_STATE_TX_DATA) {
				  if(curr_mac_addr == prv_mac_addr) break;
// 				  else continue;
				}
				else break;
			}
		}
		if (prepBeforeTx(curr_mac_addr, seq_num)) {
		    if (prev_state == UWSR_STATE_LISTEN) {
			stateTxData();
		    }
		    else {
			stateCheckWaitTxExpired();
		    }	
		}
		else stateIdle();  	
	}
	else {
		stateCheckAckExpired();
	}
  
}
 
               
void MMacUWSR::stateWaitAck() {

  refreshState(UWSR_STATE_WAIT_ACK);

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateWaitAck() " << endl;
  if (print_transitions) printStateInfo();

}


void MMacUWSR::stateRxWaitAck() {
  refreshState(UWSR_STATE_RX_WAIT_ACK);

  if (print_transitions) printStateInfo();
}

void MMacUWSR::stateRxinPreTxData() {
  refreshState(UWSR_STATE_RX_IN_PRE_TX_DATA);

  if (print_transitions) printStateInfo();
}

void MMacUWSR::stateTxAck( int dest_addr ) {

  refreshState(UWSR_STATE_TX_ACK);

  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateTxAck() dest addr " << dest_addr << endl;
  if (print_transitions) printStateInfo();
 
  txAck( dest_addr );
}


void MMacUWSR::stateRxData(Packet* data_pkt) {

  refreshState( UWSR_STATE_DATA_RX );
  
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateRxData() " << endl;
  refreshReason( UWSR_REASON_DATA_RX );

  hdr_mac* mach = HDR_MAC(data_pkt);
  int dst_addr = mach->macSA();
  hdr_cmn* ch = hdr_cmn::access(data_pkt);
  ch->size() = ch->size() - HDR_size;
  recv_data_id = ch->uid();
  incrDataPktsRx();
  sendUp(data_pkt); // mando agli strati superiori il pkt
  stateTxAck(dst_addr);
}


void MMacUWSR::stateRxAck(Packet* p) {
  
  refreshState(UWSR_STATE_ACK_RX);
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::stateRxAck() " << endl;
  
  hdr_mac* mach = HDR_MAC(p);
  int curr_mac_addr = mach->macSA();
  int seq_num = getPktSeqNum(p);
  
  map< usrPair,AckTimer> :: iterator it_a;
  map< usrPair,txStartTime> :: iterator it_ca;
  
  it_a = mapAckTimer.find(make_pair(curr_mac_addr, seq_num));
  ((*it_a).second).stop();
  
  it_ca = mapCalcAck.find(make_pair(curr_mac_addr, seq_num));
  double start_tx_time = ((*it_ca).second);
  
  eraseItemFromPktQueue(curr_mac_addr, seq_num);
  eraseItemFromTxRounds(curr_mac_addr, seq_num);
  eraseItemFrommapAckTimer(curr_mac_addr, seq_num);
  eraseItemFrommapCalcAck(curr_mac_addr, seq_num);
  incrAckPktsRx();

  incrAcksRcvIn1RTT();
  updateTxStatus(curr_mac_addr, getAcksRcvIn1RTT());
      
  Packet::free(p);

  refreshReason(UWSR_REASON_ACK_RX);
  
   switch( prev_state ) {
    
    case UWSR_STATE_RX_IDLE:
      stateIdle();
      break;
      
    case UWSR_STATE_RX_LISTEN:
      stateCheckListenExpired();
      break;
      
    case UWSR_STATE_RX_BACKOFF:
      stateCheckBackoffExpired();
      break;
      
    case UWSR_STATE_RX_WAIT_ACK:
      if (mapAckTimer.size() > 0) stateCheckAckExpired();
      else stateIdle();      
      break;
      
    case UWSR_STATE_RX_IN_PRE_TX_DATA: {
      if (mapPacket.size() == 0) stateIdle();
      else stateCheckWaitTxExpired(); 
      }
      break;
 
 
    default: 
      
      cerr << NOW << " MMacUWSR("<< addr << ")::stateRxAck() logical error, prev state = " << status_info[prev_state]
           << endl; 
      exit(1);
  }
}



void MMacUWSR::printStateInfo(double delay)
{
  if (uwsr_debug) cout << NOW << " MMacUWSR("<< addr << ")::printStateInfo() " << "from " << status_info[prev_state] 
                   << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;

  if (curr_state == UWSR_STATE_BACKOFF) {
      fout <<left << setw(10) << NOW << "  MMacUWSR("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] 
           << ". Backoff duration = " << delay << endl;
  }
  else {
      fout << left << setw(10) << NOW << "  MMacUWSR("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;
  }
}

void MMacUWSR::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
} 

