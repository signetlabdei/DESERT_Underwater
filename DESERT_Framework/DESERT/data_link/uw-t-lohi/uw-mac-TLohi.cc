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
 * @file   uw-mac-TLohi.cc
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the implementation of the MMacTLOHI class
 *
 */


#include "uw-mac-TLohi.h"
#include "wake-up-pkt-hdr.h"
#include "uw-phy-WakeUp.h"
#include <clmsg-discovery.h>
#include <mac.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <rng.h>

enum {
    NOT_SET = -1, SESSION_DISTANCE_NOT_SET = 0
};



extern packet_t PT_TLOHI;

int hdr_tlohi::offset_;

/**
 * Class the describe the PacketHeader for T-LOHI header
 */
static class TLOHIPktClass : public PacketHeaderClass {
public:
  /**
   * Constructor of the class
   */
  TLOHIPktClass() : PacketHeaderClass("PacketHeader/TLOHI", sizeof(hdr_tlohi)) {
    this->bind();
    bind_offset(&hdr_tlohi::offset_);
  }
} class_tlohi_pkt; 


/**
 * Class that represent the binding with the tcl configuration script 
 */
static class TLOHIModuleClass : public TclClass {
public:
  /**
   * Constructor of the class
   */
  TLOHIModuleClass() : TclClass("Module/UW/TLOHI") {}
  /**
   * Create the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
    return (new MMacTLOHI());
  }
} class_module_tlohi;


void Timer::expire(Event *e)
{
  switch(module->curr_state) {

    case(STATE_WAIT_ACK): { 

        if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << " ACK not received, next state = " 
                         << module->status_info[STATE_WAIT_END_CR] << endl;

        module->last_reason = REASON_ACK_TIMEOUT;
        if (module->curr_tx_rounds < module->max_tx_rounds) module->stateTxData();
        else module->stateIdle(); 
    }
    break;

    case(STATE_WAIT_END_CR): { 

        if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << " CR expired, next state = " 
                         << module->status_info[STATE_IDLE] << endl;

        module->last_reason = REASON_CR_END;
        module->stateIdle(); 
     }
    break;

    case(STATE_BACKOFF): {

        if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << " backoff expired, next state = " 
                         << module->status_info[STATE_START_CONTENTION] << endl;

        module->last_reason = REASON_BACKOFF_TIMEOUT;
        module->stateStartContention();
    }
    break;

    case(STATE_WAIT_XACK): {

        if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << " xACK not received, next state = " 
                         << module->status_info[STATE_RECONTEND_WINDOW] << endl;

        module->last_reason = REASON_XACK_TIMEOUT;
        module->stateRecontendWindow(); 
    }
    break;

    case(STATE_WAIT_END_CONTENTION): {

        if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << " contention ended, next state = " 
                         << module->status_info[STATE_COUNT_CONTENDERS] << endl;

        module->last_reason = REASON_WAIT_CR_END;
        module->stateCountContenders(); 
    }
    break;

    case(STATE_SLEEP): {

        if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << " sleep period expired, next state = " 
                         << module->status_info[STATE_RECONTEND_WINDOW] << endl;

        module->last_reason = REASON_SLEEP_TIMEOUT;
        module->stateRecontendWindow(); 
    }
    break;

    case(STATE_RECONTEND_WINDOW): {

        if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << " recontend window expired, next state = " 
                         << module->status_info[STATE_IDLE] << endl;

        module->last_reason = REASON_RECONTEND_END;
        module->stateIdle(); 
    }
    break;

    default: {
        cout << NOW << "  MMacTLOHI("<< module->addr << ") timer expire() logical error, current state = " 
             << module->status_info[module->curr_state] << endl;
        exit(1);
    }
    break;

  }

}


void DataTimer::expire(Event *e)
{
  if (module->debug_) cout << NOW << "  MMacTLOHI("<< module->addr << ") data timer expire() turning off DATA PHY."
                           << " Current state = " << module->status_info[module->curr_state] << endl;

//   module->Mac2PhyTurnOff(module->data_phy_id);
}


int MMacTLOHI::u_pkt_id;
// int MMacTLOHI::u_data_id;
bool MMacTLOHI::initialized = false;

map< TLOHI_STATUS , string> MMacTLOHI::status_info;
map< TLOHI_REASON_STATUS, string> MMacTLOHI::reason_info;
map< TLOHI_PKT_TYPE, string> MMacTLOHI::pkt_type_info;

MMacTLOHI::MMacTLOHI() 
  : timer(this),
  data_phy_timer(this),
  txsn(1),
  data_phy_id(NOT_SET),
  tone_phy_id(NOT_SET),
  CR_duration(NOT_SET),
  session_distance(SESSION_DISTANCE_NOT_SET),
  curr_dest_addr(NOT_SET),
  backoff_duration(NOT_SET),
  backoff_start_time(NOT_SET),
  backoff_remaining(NOT_SET),
  last_data_id_tx(NOT_SET),
  last_data_id_rx(NOT_SET),
  curr_data_pkt(0),
  curr_contenders(0),
  tone_pkts_tx(0),
  tone_pkts_rx(0),
  curr_tx_tries(0),
  curr_tx_rounds(0),
  u_data_id(0),
  TxActive(false),
  session_active(false),
  backoff_pending(false),
  tone_transmitted(false),
  mphy_ids_initialized(false),
  print_transitions(false),
  has_buffer_queue(false),
  curr_state(STATE_IDLE), 
  prev_state(STATE_IDLE),
  ack_mode(NO_ACK_MODE),
  op_mode(AGGRESSIVE_UNSYNC_MODE)
{ 
  u_pkt_id = 0;

  bind("max_prop_delay", (double*)& max_prop_delay);
  bind("HDR_size", (int*)& HDR_size);
  bind("ACK_size", (int*)& ACK_size);
  bind("max_tx_rounds", (int*)& max_tx_rounds);
  bind("wait_costant", (double*)& wait_costant);
  bind("debug_", (int*)&debug_); //degug mode
  bind("max_payload", (int*)&max_payload);
  bind("recontend_time", (double*)& recontend_time);
  bind("tone_data_delay", (double*)& tone_data_delay);
  bind("max_tx_tries", (double*)& max_tx_tries);
  bind("buffer_pkts", (int*)&buffer_pkts);
  
  if (max_tx_tries <= 0) max_tx_tries = HUGE_VAL;
  if (buffer_pkts > 0) has_buffer_queue = true;

  tcl_modulation.clear();
}

MMacTLOHI::~MMacTLOHI()
{

}

// TCL command interpreter
int MMacTLOHI::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if (argc==3) {
      if(strcasecmp(argv[1], "addTonePhy") == 0) {

        MPhy_WakeUp* tone_phy = dynamic_cast<MPhy_WakeUp*> (TclObject::lookup(argv[2])); 

        if (!tone_phy) return TCL_ERROR;
        else {
           tone_phy_id = tone_phy->getId();

           if (debug_) cout << NOW << "  MMacTLOHI(" << addr << ")::command() addTonePhy called, tone_phy_id = " 
                            << tone_phy_id << endl;

           return TCL_OK;
        }
      }
      else if(strcasecmp(argv[1], "addDataPhy") == 0) {

        MPhy* data_phy = dynamic_cast<MPhy*> (TclObject::lookup(argv[2])); 

        if (!data_phy) return TCL_ERROR;
        else {
          data_phy_id = data_phy->getId();

          if (debug_) cout << NOW << "  MMacTLOHI(" << addr << ")::command() addDataPhy called, data_phy_id = " 
                           << data_phy_id << endl;

          initData();
          return TCL_OK;
        }
      }
      else if(strcasecmp(argv[1], "setDataName") == 0) {
          tcl_modulation = argv[2];
          return TCL_OK;
        }
	 else if(strcasecmp(argv[1],"setMacAddr") == 0) {
			addr = atoi(argv[2]);
			if(debug_) cout << "T-LOHI MAC address of current node is " << addr <<endl;
			return TCL_OK;
		}
  }
  else if(argc==2)
    {
      if(strcasecmp(argv[1], "setAckMode") == 0)
	{
	  ack_mode = ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setNoAckMode") == 0)	
	{
          ack_mode = NO_ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setConservativeUnsyncMode") == 0)	
	{
          checkPhyInit();
          op_mode = CONSERVATIVE_UNSYNC_MODE;
          CR_duration = 2 * max_prop_delay + computeTxTime(TONE_PKT);
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setAggressiveUnsyncMode") == 0)	
	{
          checkPhyInit();
          op_mode = AGGRESSIVE_UNSYNC_MODE;
          CR_duration = max_prop_delay + computeTxTime(TONE_PKT);
	  return TCL_OK;
	}
/*      else if(strcasecmp(argv[1], "setSyncMode") == 0)	
	{
          checkPhyInit();
          op_mode = SYNC_MODE;
          CR_duration = max_prop_delay + computeTxTime(TONE_PKT);
	  return TCL_OK;
	}*/
      else if(strcasecmp(argv[1], "initialize") == 0)	
	{
          if (initialized == false) initInfo();
          initMphyIds();
          if (print_transitions) fout.open("/tmp/TLOHIstateTransitions.txt",ios_base::app);
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "printTransitions") == 0)	
	{
          print_transitions = true;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getCRTime") == 0)	
	{
	  tcl.resultf("%f",getCRduration());
      	  return TCL_OK;
	}
      // stats functions
      else if(strcasecmp(argv[1], "getQueueSize") == 0)	
	{
	  tcl.resultf("%d",Q.size());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getTonePktsTx") == 0)	
	{
	  tcl.resultf("%d",getTonePktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getTonePktsRx") == 0)	
	{
	  tcl.resultf("%d",getTonePktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1],"getUpLayersDataRx") == 0)
      {
          tcl.resultf("%d",getUpLayersDataPktsRx());
          return TCL_OK;
      }
    }
  return MMac::command(argc, argv);
}


int MMacTLOHI::crLayCommand(ClMessage* m)
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

void MMacTLOHI::checkPhyInit()
{
  if ( (mphy_ids_initialized == false) || (initialized == false) ) {
     cout << "MMacTLOHI(" << addr << ")::command() initialized command not called, aborting..." << endl;
     exit(1);
  }
}

void MMacTLOHI::initData()
{
  if (ack_mode == ACK_MODE) sleep_timeout = 3 * max_prop_delay + computeTxTime(ACK_PKT) + computeTxTime(DATA_PKT) 
                                                               + 2*computeTxTime(TONE_PKT) + wait_costant;
  else sleep_timeout = 3 * max_prop_delay + computeTxTime(DATA_PKT) + 2 * computeTxTime(TONE_PKT) + wait_costant;

// sleep_timeout = 3 * max_prop_delay + computeTxTime(ACK_PKT) + computeTxTime(DATA_PKT) 
//                                                                + 2*computeTxTime(TONE_PKT) + wait_costant;

  DATA_listen_timeout = computeTxTime(DATA_PKT) + wait_costant;
  ACK_timeout = 2 * max_prop_delay + computeTxTime(DATA_PKT) + computeTxTime(TONE_PKT) + computeTxTime(ACK_PKT) + wait_costant;

  if (max_tx_tries <= 0) max_tx_tries = HUGE_VAL;
  if (buffer_pkts > 0) has_buffer_queue = true;

  if (debug_) cout << NOW << "  MMacTLOHI("<< addr <<")::initData() sleep_timeout = " << sleep_timeout 
                   << ", DATA_listen_timeout = " << DATA_listen_timeout << endl;
  
  
}

void MMacTLOHI::initInfo()
{

  initialized = true;

  if ( (print_transitions) && (system(NULL)) ) {
      system("rm -f /tmp/TLOHIstateTransitions.txt");
      system("touch /tmp/TLOHIstateTransitions.txt");
  }

  status_info[STATE_IDLE] = "Idle state";
  status_info[STATE_BACKOFF] = "Backoff state"; 
  status_info[STATE_START_CONTENTION] = "Start of Contention Round state"; 
/*  status_info[STATE_END_CR] = "End of Contention Round state"; */
  status_info[STATE_TX_DATA] = "Transmit DATA state"; 
  status_info[STATE_SLEEP] = "Sleep state"; 
  status_info[STATE_WAIT_END_CR] = "Wait a Contention Round state";
  status_info[STATE_TX_ACK] = "Transmit ACK state";
  status_info[STATE_WAIT_ACK] = "Wait for ACK state"; 
  status_info[STATE_DATA_RECEIVED] = "DATA received state"; 
  status_info[STATE_ACK_RECEIVED] = "ACK received state"; 
  status_info[STATE_WAIT_END_CONTENTION] = "Wait for the end of Contention state";
  status_info[STATE_COUNT_CONTENDERS] = "Count contenders state";
  status_info[STATE_RECONTEND_WINDOW] = "Recontend backoff state";
  status_info[STATE_WAIT_XACK] = "Wait for xACK state";

  reason_info[REASON_TONE_TX] = "Tone transmitted"; 
  reason_info[REASON_DATA_PENDING] = "DATA pending from upper layers"; 
  reason_info[REASON_TONE_RX] = "Tone received"; 
  reason_info[REASON_WAIT_CR_END] = "End of Contention Round";
  reason_info[REASON_NO_CONTENDERS] = "No contenders found"; 
  reason_info[REASON_CONTENDERS] = "Contenders found"; 
  reason_info[REASON_DATA_RX] = "DATA received";
  reason_info[REASON_DATA_TX] = "DATA transmitted"; 
  reason_info[REASON_ACK_TX] = "ACK tranmsitted";
  reason_info[REASON_ACK_RX] = "ACK received"; 
  reason_info[REASON_RECONTEND_END] = "Recontend backoff expired"; 
  reason_info[REASON_BACKOFF_TIMEOUT] = "Backoff expired"; 
  reason_info[REASON_ACK_TIMEOUT] = "ACK timeout"; 
  reason_info[REASON_SLEEP_TIMEOUT] = "SLEEP expired";
  reason_info[REASON_CR_END] = "Recontention CR expired";
  reason_info[REASON_DATA_EMPTY] = "DATA queue empty";
  reason_info[REASON_XACK_TIMEOUT] = "xACK timeout";
  reason_info[REASON_XDATA_RX] = "xDATA received";
  reason_info[REASON_XACK_RX] = "xACK received";
  reason_info[REASON_MAX_TX_TRIES] = "DATA dropped due to max tx rounds";

  pkt_type_info[TONE_PKT] = "TONE pkt";
  pkt_type_info[ACK_PKT] = "ACK pkt";
  pkt_type_info[DATA_PKT] = "DATA pkt"; 
  pkt_type_info[DATAMAX_PKT] = "MAX payload DATA pkt";
}

void MMacTLOHI::initMphyIds()
{
  if (tcl_modulation.size() == 0) {
     cout << NOW << "  MMacTLOHI("<< addr <<")::initMphyIds() Error, data phy name not set" << endl;
     exit(1);
  }

  ClMsgDiscovery m;
  m.addSenderData((const PlugIn*) this, getLayer(), getId(), name() , getTag());
  sendSyncClMsgUp(&m);
  sendSyncClMsgDown(&m);
  
//   if (debug_) m.printReplyData();

  DiscoveryStorage lower_layer = m.findLayer( getLayer() - 1);

//   if (debug_) lower_layer.printData();

  DiscoveryStorage tone_phy_db = lower_layer.findTclName("Module/MPhy/Underwater/WKUP");
  DiscoveryStorage data_phy_db = lower_layer.findTclName(tcl_modulation.c_str());

//   if (debug_) {
//      tone_phy_db.printData();
//      data_phy_db.printData();
//   }

  if (tone_phy_db.getSize() == 1) tone_phy_id = (*tone_phy_db.begin()).first;
  if (data_phy_db.getSize() == 1) data_phy_id = (*data_phy_db.begin()).first;

//   cout << "MMacTLOHI(" << addr << ")::initMphyIds() tone_phy id = " << tone_phy_id << endl 
//        << "MMacTLOHI(" << addr << ")::initMphyIds() data_phy id = " << data_phy_id << endl;

  //waitForUser();

  initData();

  mphy_ids_initialized = true;
}

void MMacTLOHI::resetSession()
{   
  session_distance = SESSION_DISTANCE_NOT_SET; 
  curr_dest_addr = NOT_SET; 
  curr_tx_rounds = 0;
  curr_contenders = 0; 
}

double MMacTLOHI::computeTxTime(TLOHI_PKT_TYPE type)
{
  double duration;
  Packet* temp_data_pkt;
  int dest_addr;

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
     dest_addr = data_phy_id;
  }
  else if (type == ACK_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = ACK_size;
        dest_addr = data_phy_id;
  }
  else if (type == TONE_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = 1;
        ch->ptype() = PT_WKUP;
        dest_addr = tone_phy_id;
  }
  duration = Mac2PhyTxDuration(dest_addr, temp_data_pkt );
  Packet::free(temp_data_pkt);
  return(duration);
}


void MMacTLOHI::exitBackoff()
{
  timer.force_cancel();
  backoff_pending = false;
  backoff_start_time = NOT_SET;
  backoff_duration = NOT_SET;
  backoff_remaining = NOT_SET;
}

void MMacTLOHI::getBackoffTime()
{
  incrTotalBackoffTimes();
  double random = RNG::defaultrng()->uniform_double();

  while (random == 0) { // se random == 0 lo rigenero
     random = RNG::defaultrng()->uniform_double();
  }

  backoff_duration = random * CR_duration * max(1,curr_contenders);
  backoffSumDuration(backoff_duration);
  curr_contenders = 0;

  if (debug_){
       cout << NOW << "  MMacTLOHI("<< addr <<")::getBackoffTime() backoff time = " << backoff_duration << " s" << endl;
       //waitForUser();  
  }

}

void MMacTLOHI::recvFromUpperLayers(Packet* p)
{ 
  if ( ((has_buffer_queue == true) && (Q.size() < buffer_pkts)) || (has_buffer_queue == false) ) {
 
     initPkt(p , DATA_PKT);
     Q.push(p);
     incrUpperDataRx();
     waitStartTime();

     if ( (curr_state == STATE_IDLE) && (session_active == false) ) // se sono libero comincio contention
       {
         last_reason = REASON_DATA_PENDING;
         stateStartContention();

//          if (debug_) 
// 	   cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
// 	        << " mac busy => enqueueing packet" << endl;
       }
     else
       {
//          if (debug_) 
// 	   cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
// 	        << " transmitting packet" << endl;
//       
       }
  }
  else {
     incrDiscardedPktsTx();
     drop(p, 1, TLOHI_DROP_REASON_BUFFER_FULL);
//      Packet::free(p);
  }
}

void MMacTLOHI::initPkt(Packet* p, TLOHI_PKT_TYPE type)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_tlohi* tlohih = HDR_TLOHI(p);
  hdr_mac* mach = HDR_MAC(p);

  tlohih->sn = txsn;
  txsn++;
  tlohih->ts = NOW;
  tlohih->pkt_type = type;

  int curr_size = ch->size();

  if (type != DATA_PKT) {
    ch->timestamp() = tlohih->ts;
    ch->ptype() = PT_TLOHI;
  }
  else { // incapsulamento dei dati
     tlohih->orig_type = ch->ptype();
     ch->ptype() = PT_TLOHI;
     tlohih->data_sn = u_data_id++;
  }

  switch(type) {
  
    case(TONE_PKT): {
      ch->ptype() = PT_WKUP;
      ch->size() = 1;
      ch->uid() = u_pkt_id++;
      mach->set(MF_CONTROL, addr, curr_dest_addr);
     }
    break;

    case(DATA_PKT): {
      ch->size() = curr_size + HDR_size;
      mach->macSA() = addr;
    } 
    break;

    case(ACK_PKT): {
      ch->size() = ACK_size;
      ch->uid() = u_pkt_id++;
      mach->set(MF_CONTROL, addr, curr_dest_addr);
    }
    break;

  }

}

int MMacTLOHI::countContenders(double time_interval)
{
  int contenders_no = 0;
 
//    if ( time_interval < computeTxTime(TONE_PKT) ) time_interval = 0;

   if (time_interval > 0) {
      double contenders = time_interval / computeTxTime(TONE_PKT) ;
      double decimal = contenders - floor(contenders);
      if (decimal < 0.5) contenders_no = floor(contenders);
      else contenders_no = ceil(contenders);
   } 
//    if (contenders_no > 0) contenders_no--;

  if (debug_) cout << NOW << "  MMacTLOHI(" << addr << ")::countContenders() time interval = " << setprecision(12) 
                   << scientific << time_interval << fixed 
                   << " [s]; contenders = " << contenders_no << endl;
   //waitForUser();

   return (contenders_no);
}

void MMacTLOHI::Phy2MacEndTx(const Packet* p)
{ // si occupa dei cambi stato

  TxActive = false;

  switch(curr_state) {

    case(STATE_TX_DATA): {
        // TONE already transmitted ==> DATA tx
        if (tone_transmitted == true) {
            tone_transmitted = false;
/*            timer.resched(tone_data_delay);*/
            txData();
            return;
        }

        // TONE && DATA transmitted
        refreshReason(REASON_DATA_TX);

        if (ack_mode == ACK_MODE) {
 
           if (debug_) cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacEndTx() DATA sent, from " 
                            << status_info[curr_state] << " to " 
                            << status_info[STATE_WAIT_ACK] << endl;

           stateWaitAck(); 
        }
        else{

           if (debug_) cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacEndTx() DATA sent, from " 
                            << status_info[curr_state] << " to " << status_info[STATE_IDLE] << endl;

           stateIdle();
        }
    }
    break;

    case(STATE_TX_ACK): {
        // TONE already transmitted ==> ACK tx
        if (tone_transmitted == true) {
            tone_transmitted = false;
            txAck();
            return;
        }

        refreshReason(REASON_ACK_TX);

        if (debug_ > 0) cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacEndTx() ACK sent, from " 
                             << status_info[curr_state] << " to " << status_info[STATE_RECONTEND_WINDOW] << endl;

        stateRecontendWindow();
    }
    break;

    case(STATE_START_CONTENTION): {
        refreshReason(REASON_TONE_TX);

        if (debug_ > 0) cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacEndTx() TONE sent, from " 
                             << status_info[curr_state] << " to " << status_info[STATE_WAIT_END_CONTENTION] << endl;

        stateWaitEndContention();
    }
    break;

    default: {
        cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacEndTx() logical error, current state = " 
             << status_info[curr_state] << endl;
        exit(1);
    }
    break;

  }

}

void MMacTLOHI::Phy2MacStartRx(const Packet* p)
{
//     if (debug_) cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ << endl;

    hdr_cmn* ch = HDR_CMN(p);
    hdr_MPhy* ph = HDR_MPHY(p);

    if ( (ch->ptype() == PT_TLOHI)  // if the data PHY signal an incoming pkt
                   && ( ((HDR_TLOHI(p))->pkt_type == DATA_PKT) || ((HDR_TLOHI(p))->pkt_type == ACK_PKT)) ) {
        data_phy_timer.resched(ph->duration + wait_costant);

        if (debug_) cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacStartRx() receiving a DATA or ACK pkt," 
                                << " rescheduling data_phy closure" << endl; 
        //waitForUser();
    }
  
}

void MMacTLOHI::Phy2MacEndRx(Packet* p)
{
//   if (debug_) cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ << endl;  
  hdr_cmn* ch = HDR_CMN(p);
  packet_t rx_pkt_type = ch->ptype();

  if ( (rx_pkt_type != PT_TLOHI) && (rx_pkt_type != PT_WKUP)) {
       drop(p, 1, TLOHI_DROP_REASON_UNKNOWN_TYPE);
       return;
  }

  hdr_mac* mach = HDR_MAC(p);
  hdr_MPhy* ph = HDR_MPHY(p);
  hdr_tlohi* tlohih; 

  TLOHI_PKT_TYPE tlohi_pkt_type;

  int source_mac = mach->macSA();
  int dest_mac = mach->macDA();

  double gen_time = ph->txtime;
  double received_time = ph->rxtime;
  double diff_time = received_time - gen_time;

  double distance = diff_time * prop_speed;
  double tone_time_interval = 0;

  if (rx_pkt_type == PT_WKUP) rxTone(p);
  else if ( ch->error() ) {
//     drop(p, 1, "ERR");
    Packet::free(p);
    return;
//     Mac2PhyTurnOff(data_phy_id); // turn off data PHY
  }
  else {
    tlohih = HDR_TLOHI(p); 
    tlohi_pkt_type = tlohih->pkt_type;

    if (debug_) cout << NOW << "  MMacTLOHI("<< addr << ")::Phy2MacEndRx() " 
                     << status_info[curr_state] << ", received a pkt type = " 
                     << pkt_type_info[tlohi_pkt_type] << ", src addr = " << mach->macSA() 
                     << " dest addr = " << mach->macDA() 
                     << ", estimated distance between nodes = " << distance << " m " << endl;

    switch(curr_state) {

      // fallthrough!! STATE_IDLE == STATE_SLEEP
      case(STATE_IDLE): 

      case(STATE_SLEEP): {
          if ((tlohih->pkt_type == DATA_PKT) && (dest_mac == addr)) { // DATA pkt for me
             setDestAddr(source_mac);
             stateDataReceived(p);
          }
          else if ((tlohih->pkt_type == DATA_PKT) && (dest_mac != addr)) { // xDATA pkt
             incrXDataPktsRx();
             setDestAddr(source_mac); // we lock on this pkt for xACK purposes
             Packet::free(p);
             refreshReason(REASON_XDATA_RX);
             if (ack_mode == ACK_MODE) stateWaitXAck();
             else stateRecontendWindow();
          }
          else if ((tlohih->pkt_type == ACK_PKT) && (dest_mac != addr)) { // xACK pkt
             incrXAckPktsRx();

             Packet::free(p);
             refreshReason(REASON_XACK_RX);
             stateRecontendWindow();
          }
          else {
             Packet::free(p);
          }
      }
      break;

      case(STATE_WAIT_XACK): {
          if ((tlohih->pkt_type == ACK_PKT) && (dest_mac != addr) && (dest_mac == curr_dest_addr) ) { // xACK pkt
             incrXAckPktsRx();

             Packet::free(p);
             refreshReason(REASON_XACK_RX);
             // ACK rx ==> turn data_phy OFF
//              Mac2PhyTurnOff(data_phy_id);
             stateRecontendWindow();
          }
          else if ((tlohih->pkt_type == DATA_PKT) && (dest_mac != curr_dest_addr) ){ // xDATA pkt 
             incrXDataPktsRx();
             setDestAddr(source_mac); // we chain-lock on this pkt for xACK purposes
             Packet::free(p);
             refreshReason(REASON_XDATA_RX);
             stateWaitXAck();
          }
          else if ((tlohih->pkt_type == DATA_PKT) && (dest_mac == addr)) { // DATA pkt for me
             setDestAddr(source_mac);
             stateDataReceived(p);
          }
          else {
             Packet::free(p);
          }
      }
      break;

      case(STATE_WAIT_ACK): {
          if ((tlohih->pkt_type == ACK_PKT) && (dest_mac == addr)) { // ACK pkt
             incrAckPktsRx();

             Packet::free(p);
             refreshReason(REASON_ACK_RX);
             // ACK rx ==> turn data_phy OFF

             queuePop(true);
//              Mac2PhyTurnOff(data_phy_id); // we can turn off data_phy
             stateIdle();
          }
          else {
             Packet::free(p);
          }
      }
      break;

      default: {

          if (debug_) cout << NOW << "  MMacTLOHI("<< addr << ")::Phy2MacEndRx() unkown State, dropping pkt" << endl;

          drop(p, 1 , TLOHI_DROP_REASON_WRONG_STATE);
//           Packet::free(p);
          return;
      }
      break;

    }
//     Mac2PhyTurnOff(data_phy_id); // turn off data PHY
  }

}

void MMacTLOHI::rxTone(Packet* p) 
{
  hdr_cmn* ch = HDR_CMN(p);

  if (debug_) cout << NOW << "  MMacTLOHI("<< addr << ")::Phy2MacEndRx() " 
                   << status_info[curr_state] << ", received a TONE pkt: error = " << ch->error() << endl;

  hdr_wkup* wkuph = HDR_WKUP(p);

  // if an error occurs in the pkt received we must subtract a tone duration
  double tone_time_interval = 0;

  if ( ch->error() ) tone_time_interval = wkuph->endRx_time - wkuph->startRx_time - computeTxTime(TONE_PKT);
  else tone_time_interval = wkuph->endRx_time - wkuph->startRx_time;

  refreshReason(REASON_TONE_RX);
  incrTonePktsRx();

  Packet::free(p);

  switch(curr_state) {

     // fallthrough
     case(STATE_BACKOFF):

     case(STATE_WAIT_END_CR):

     case(STATE_IDLE): {

        if (debug_ > 0)  cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacEndRx() TONE rx. From "   
                              << status_info[curr_state] << " to " << status_info[STATE_SLEEP] << endl;

        checkDataPhy();
        stateSleep(); 
     }
     break;

     case(STATE_START_CONTENTION): 

     case(STATE_WAIT_END_CONTENTION): {

        int contenders = countContenders(tone_time_interval);
        curr_contenders += contenders;

        if (debug_ > 0) cout << NOW << "  MMacTLOHI("<< addr <<")::Phy2MacEndRx() contenders number = "
                             << contenders << " ; current contenters = " << curr_contenders << endl;

     }
     break;

     case(STATE_WAIT_XACK):

     case(STATE_SLEEP): {
         checkDataPhy();
//          refreshReason(REASON_TONE_RX);
         stateSleep();
     }
     break;

     case(STATE_WAIT_ACK): {
         checkDataPhy();
     }
     break;

     default: {

        if (debug_) cout << NOW << "  MMacTLOHI("<< addr << ")::Phy2MacEndRx() " 
                         << "wrong state " << status_info[curr_state] << endl;

//         drop(p,1,TLOHI_DROP_REASON_WRONG_STATE);
//         Packet::free(p);
//         return;

     }
     break;
  }
}


void MMacTLOHI::rxAck(Packet* p)
{

}


void MMacTLOHI::rxElse(Packet* p)
{

}

void MMacTLOHI::txTone()
{
  Packet* tone_pkt = Packet::alloc();
  initPkt(tone_pkt , TONE_PKT);

  TxActive = true;
  tone_transmitted = true;
  incrTonePktsTx();
  Mac2PhyStartTx(tone_phy_id, tone_pkt); 
}

void MMacTLOHI::txData()
{
  Packet* data_pkt = curr_data_pkt->copy();      // copio pkt 
 
  if ( (ack_mode == NO_ACK_MODE) ) queuePop(true);
                                       
//   initPkt(data_pkt , DATA_PKT);
 
  TxActive = true;
  incrDataPktsTx();
  incrCurrTxRounds();
  incrCurrTxTries();
  Mac2PhyTurnOn(data_phy_id);
  Mac2PhyStartTx(data_phy_id, data_pkt); 
}

void MMacTLOHI::txAck()
{
  Packet* ack_pkt = Packet::alloc();
  initPkt(ack_pkt , ACK_PKT);

  TxActive = true;
  incrAckPktsTx();

  Mac2PhyTurnOn(data_phy_id);
  Mac2PhyStartTx(data_phy_id, ack_pkt);
}

void MMacTLOHI::checkDataPhy()
{
  Mac2PhyTurnOn(data_phy_id);
  data_phy_timer.resched(DATA_listen_timeout + wait_costant);
//   data_phy_timer.resched(sleep_timeout + wait_costant);
}

void MMacTLOHI::stateIdle()
{
  timer.force_cancel();
  refreshState(STATE_IDLE);

  if (print_transitions) printStateInfo();

  session_active = false;
  resetSession();

  if ( !Q.empty() ) {
     if ( HDR_TLOHI(Q.front())->data_sn != last_data_id_tx) {
        session_distance = SESSION_DISTANCE_NOT_SET;
        curr_tx_tries = 0;
        last_data_id_tx = HDR_TLOHI(Q.front())->data_sn;
     }
     if ( curr_tx_tries < max_tx_tries) {
        refreshReason(REASON_DATA_PENDING);
        stateStartContention();
     }
     else {
       queuePop(false); // FALSE
       incrDroppedPktsTx();

       refreshReason(REASON_MAX_TX_TRIES);

       if (debug_) cout << NOW << "  MMacTLOHI("<< addr << ")::stateIdle() curr_tx_tries " << curr_tx_tries
                        << " > max_tx_tries = " << max_tx_tries << endl;
//        waitForUser();

       stateIdle();
     }
  }
}

void MMacTLOHI::stateStartContention()
{
  session_active = true;

//   Mac2PhyTurnOff(data_phy_id);

  timer.force_cancel();
  refreshState(STATE_START_CONTENTION);

  if (print_transitions) printStateInfo();

  curr_data_pkt = Q.front();
  hdr_mac* mach = HDR_MAC(curr_data_pkt);
  curr_dest_addr = mach->macDA(); // indirizzo destinazione

  txTone(); 
}


void MMacTLOHI::stateBackoff()
{
  timer.force_cancel();
  refreshState(STATE_BACKOFF);

  backoff_pending = true;

  getBackoffTime();

  backoff_start_time = NOW;
  timer.resched(backoff_duration);

  if (print_transitions) printStateInfo(backoff_duration);
}

void MMacTLOHI::stateWaitEndContention()
{
  timer.resched(CR_duration);
 
//   Mac2PhyTurnOff(data_phy_id);
 
  refreshState(STATE_WAIT_END_CONTENTION);

  if (print_transitions) printStateInfo();
}

void MMacTLOHI::stateCountContenders()
{
  timer.force_cancel();
 
//   Mac2PhyTurnOff(data_phy_id);

  refreshState(STATE_COUNT_CONTENDERS);

  if (print_transitions) printStateInfo();

  if (curr_contenders == 0) {
     refreshReason(REASON_NO_CONTENDERS);
     stateTxData();
  }
  else {
     refreshReason(REASON_CONTENDERS);
     stateBackoff();
  }
}

void MMacTLOHI::stateWaitCR()
{
  timer.resched(CR_duration);

  refreshState(STATE_WAIT_END_CR);

  if (print_transitions) printStateInfo();
}

void MMacTLOHI::stateSleep()
{
  timer.resched(sleep_timeout);
  refreshState(STATE_SLEEP);

  if (print_transitions) printStateInfo();
}

void MMacTLOHI::stateTxData()
{
  timer.force_cancel();
  refreshState(STATE_TX_DATA);

  if (print_transitions) printStateInfo();

  txTone();
//   tone_transmitted = false;
//   txData();
}


void MMacTLOHI::stateWaitAck()
{
  timer.force_cancel();
  refreshState(STATE_WAIT_ACK);

  if (print_transitions) printStateInfo();

//   timer.resched(2*max_prop_delay + computeTxTime(TONE_PKT));
  timer.resched(ACK_timeout + wait_costant); 
 
}


void MMacTLOHI::stateWaitXAck()
{
  timer.force_cancel();
  refreshState(STATE_WAIT_XACK);
  
  if (print_transitions) printStateInfo();

  timer.resched(2*max_prop_delay + computeTxTime(TONE_PKT)+ computeTxTime(ACK_PKT) + wait_costant);
//   timer.resched(ACK_timeout); 
}


void MMacTLOHI::stateTxAck()
{
  timer.force_cancel();
  refreshState(STATE_TX_ACK);

  if (print_transitions) printStateInfo();

//   txTone();
  tone_transmitted = false; // seems awkward but if put to true tlohi would send two ack pkts. check EndTx
  txAck();
}

void MMacTLOHI::stateRecontendWindow()
{
  timer.force_cancel();
  refreshState(STATE_RECONTEND_WINDOW);

  if (print_transitions) printStateInfo();

  double random = RNG::defaultrng()->uniform_double();

  while (random == 0) random = RNG::defaultrng()->uniform_double();

  double recontend_duration = recontend_time * random;

  if (!Q.empty()) timer.resched(recontend_duration);
  else {
    refreshReason(REASON_DATA_EMPTY);
    stateIdle();
  }
}

void MMacTLOHI::stateDataReceived(Packet* data_pkt)
{
  timer.force_cancel();
  refreshState(STATE_DATA_RECEIVED);
  refreshReason(REASON_DATA_RX);

  if (print_transitions) printStateInfo();

  // tolgo type tlohi e metto quello originale
  incrDataPktsRx();
  hdr_cmn* ch = hdr_cmn::access(data_pkt);
  hdr_tlohi* tlohih = HDR_TLOHI(data_pkt);
  ch->ptype() = tlohih->orig_type;
  ch->size() = ch->size() - HDR_size;

  sendUp(data_pkt); // mando agli strati superiori il pkt

  if (ack_mode == ACK_MODE) stateTxAck();
  else stateRecontendWindow();
}

void MMacTLOHI::printStateInfo(double delay)
{
  if (debug_) cout << NOW << "  MMacTLOHI("<< addr << ")::printStateInfo() " << "from " << status_info[prev_state] 
                   << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;

  if (curr_state == STATE_BACKOFF) {
      fout <<left << setw(10) << NOW << "  MMacTLOHI("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] 
           << ". Backoff duration = " << delay << endl;
  }
  else {
      fout << left << setw(10) << NOW << "  MMacTLOHI("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;
  }
}

void MMacTLOHI::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
} 


    
	
