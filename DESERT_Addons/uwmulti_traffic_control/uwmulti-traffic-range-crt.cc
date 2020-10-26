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
 * @file   uwmulti-traffic-range-crt.cc
 * @author Filippo Campagnaro, Federico Guerra
 * @version 1.0.0
 *
 * \brief Implementation of UwMultiTrafficControl class.
 *
 */

#include "uwmulti-traffic-range-crt.h"
#include "uwip-clmsg.h"
#include <uwcbr-module.h>
#include <iostream>

void UwMultiTrafficRangeCtr::UwCheckRangeTimer::expire(Event *e) 
{ 
  UwCheckRangeTimer::num_expires++; 
  if (UwCheckRangeTimer::num_expires > UwCheckRangeTimer::max_increment)
    UwCheckRangeTimer::num_expires = UwCheckRangeTimer::max_increment;
  UwCheckRangeTimer::module->timerExpired(UwCheckRangeTimer::traffic); 
}

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwMultiTrafficRangeCtrClass : public TclClass 
{
public:
  /**
   * Constructor of the class
   */
  UwMultiTrafficRangeCtrClass() : TclClass("Module/UW/MULTI_TRAFFIC_RANGE_CTR") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) 
  {
    return (new UwMultiTrafficRangeCtr);
  }
} class_tr_range_control;

UwMultiTrafficRangeCtr::UwMultiTrafficRangeCtr() 
: 
  UwMultiTrafficControl(),
  status(),
  check_to_period(0),
  signaling_pktSize(1),
  timers()
{ 
  bind("check_to_period_", &check_to_period);
  bind("signaling_pktSize_", &signaling_pktSize);
}

int UwMultiTrafficRangeCtr::command(int argc, const char*const* argv) 
{
  Tcl &tcl = Tcl::instance();
  if (argc == 3)
  {
    if(strcasecmp(argv[1], "getProbeSent") == 0)
    {
      tcl.resultf("%d", getSignalingCounter(atoi(argv[2]), tx_probe_cnt));
      return TCL_OK;
    } 
    else if((strcasecmp(argv[1], "getProbeRecv") == 0)) 
    {
      tcl.resultf("%d", getSignalingCounter(atoi(argv[2]), rx_probe_cnt));
      return TCL_OK;
    } 
    else if((strcasecmp(argv[1], "getProbeAckSent") == 0)) 
    {
      tcl.resultf("%d", getSignalingCounter(atoi(argv[2]), tx_probe_ack_cnt));
      return TCL_OK;
    } 
    else if((strcasecmp(argv[1], "getProbeAckRecv") == 0)) 
    {
      tcl.resultf("%d", getSignalingCounter(atoi(argv[2]), rx_probe_ack_cnt));
      return TCL_OK;
    }
  }
  else if (argc == 4) 
  {
    if(strcasecmp(argv[1], "addRobustLowLayer") == 0)
    {
      addLowLayerFromTag(atoi(argv[2]),argv[3],ROBUST);
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "addFastLowLayer") == 0)
    {
      addLowLayerFromTag(atoi(argv[2]),argv[3],CHECK_RANGE);
      return TCL_OK;
    }
	}	
  return UwMultiTrafficControl::command(argc, argv);     
} /* UwMultiTrafficRangeCtr::command */

void UwMultiTrafficRangeCtr::recv(Packet* p, int idSrc)
{
  hdr_cmn *ch = HDR_CMN(p);
  if (ch->direction() == hdr_cmn::UP) {
    if (ch->ptype() == PT_MULTI_TR_PROBE_ACK) {
      hdr_uwip* iph  = HDR_UWIP(p);
      if (debug_)
        std::cout << NOW << " UwMultiTrafficRangeCtr::recv PT_MULTI_TR_PROBE_ACK from:" 
                  << (int)(iph->saddr()) << " in layer " << idSrc << std::endl;
      incrSignalingCounter(HDR_UWMTR(p)->traffic(), rx_probe_ack_cnt);
      manageCheckedLayer(HDR_UWMTR(p)->traffic() , iph->saddr(), true, idSrc);      
      Packet::free(p);
    }
    else if (ch->ptype() == PT_MULTI_TR_PROBE) {
      UWIPClMsgSendAddr msg;
      sendSyncClMsgUp(&msg);
      hdr_uwip* iph  = HDR_UWIP(p);
      incrSignalingCounter(HDR_UWMTR(p)->traffic(), rx_probe_cnt);
      if (iph->daddr() ==  msg.getAddr() || iph->daddr() == UWIP_BROADCAST) {
        ch->ptype() = PT_MULTI_TR_PROBE_ACK;
        ch->size() = signaling_pktSize;
        if (debug_)
          std::cout << NOW << " UwMultiTrafficRangeCtr::recv PT_MULTI_TR_PROBE from:" 
                   << (int)iph->saddr() << " in layer " << idSrc << std::endl;
        iph->daddr() = iph->saddr();
        iph->saddr() =  msg.getAddr();
        incrSignalingCounter(HDR_UWMTR(p)->traffic(), tx_probe_ack_cnt);
        /*ch->next_hop() = ch->prev_hop_;*/
        sendDown(idSrc, p);
      }
      else {

        if (debug_)
          std::cout << NOW << " UwMultiTrafficRangeCtr::recv PT_MULTI_TR_PROBE from :" << (int)iph->saddr() 
                    << " for:" << (int)iph->daddr() << "and not " << msg.getAddr() << " in layer " 
                    << idSrc << " discarded" << std::endl;
        Packet::free(p);
      }
    }
    else {
      UwMultiTrafficControl::recv(p);
      if(debug_)
        std::cout << "UwMultiTrafficRangeCtr::recv(DIR=UP, no signaling)" << std::endl;
    }
  }
  else {
    UwMultiTrafficControl::recv(p);
    if(debug_)
      std::cout << NOW << " UwMultiTrafficRangeCtr::recv(DIR=DOWN)" << std::endl;
  }
}

void UwMultiTrafficRangeCtr::manageCheckedLayer(int traffic, uint8_t destAdd, bool in_range, int idSrc)
{
  BufferTrafficFeature::iterator it_feat = buffer_feature_map.find(traffic);
  if (it_feat == buffer_feature_map.end()) {
    std::cout << "UwMultiTrafficControl::manageCheckedLayer. ERROR. Buffer not "
              << "configured. Traffic " << traffic << std::endl;
    return;
  }
  std::map<int, UwCheckRangeTimer*>::iterator it_t = timers.find(traffic);
  if (it_t == timers.end()) 
    return;
  UwCheckRangeTimer *to = it_t->second;
  if(to->status() == TIMER_IDLE) {
    return; // nothing to check is pending
  }
  else {
    StatusMap::iterator it_s = status.find(traffic);
    if (it_s == status.end()) 
      return;
    if(status[traffic].status == RANGE_CNF_WAIT){
      if(in_range || status[traffic].robust_id) {
        Packet *p = NULL;
        while(true){
          p = getFromBuffer(traffic);
          if(p != NULL && (HDR_UWIP(p)->daddr() == destAdd || HDR_CMN(p)->next_hop() == destAdd ||
                           HDR_CMN(p)->next_hop() == UWIP_BROADCAST)) {
            to->force_cancel();
            to->num_expires = 0;
            status[traffic].status = IDLE;
            //do {
            if(debug_)
              std::cout << NOW << " UwMultiTrafficRangeCtr::manageCheckedLayer sending packet" << std::endl;
            if(in_range) {
              /*sendDown(status[traffic].module_id,p);*/
              sendDown(idSrc,p, it_feat->second.getUpdatedDelay(NOW));
            }
            else {
              sendDown(status[traffic].robust_id,p,
                  it_feat->second.getUpdatedDelay(NOW));
            }
            removeFromBuffer(traffic);
              //p = removeFromBuffer(traffic);
            //} while (p != NULL && HDR_UWIP(p)->daddr() == destAdd);
          }
          else if(p == NULL) {
            if(debug_)
              std::cout << NOW << " UwMultiTrafficRangeCtr::manageCheckedLayer nothing to send to no one" 
                        << std::endl;
            break;
          }
          else{ //NEVER APPENS IF THERE ARE ONLY 2 NODES
            Packet *p0 = p;
            do {
              removeFromBuffer(traffic);
              insertInBuffer(p,traffic);
              p = getFromBuffer(traffic);
            } while (p != NULL && p != p0 && (HDR_UWIP(p)->daddr() != destAdd || 
                     HDR_CMN(p)->next_hop() == destAdd || HDR_CMN(p)->next_hop() == UWIP_BROADCAST));
            if(!(HDR_UWIP(p)->daddr() == destAdd || HDR_CMN(p)->next_hop() == destAdd || 
                 HDR_CMN(p)->next_hop() == UWIP_BROADCAST)) {
              if(debug_)
                std::cout << NOW << " UwMultiTrafficRangeCtr::manageCheckedLayer wrong daddr " 
                          << (int)HDR_UWIP(p)->daddr() << ", " << (int)destAdd << std::endl;
              break; 
            }
            else {
              if(in_range) {
                /*sendDown(status[traffic].module_id,p);*/ 
                sendDown(idSrc,p,it_feat->second.getUpdatedDelay(NOW)); 
              }
              else {
                sendDown(status[traffic].robust_id,p,
                    it_feat->second.getUpdatedDelay(NOW));
              }
              removeFromBuffer(traffic);
            }
          }
        }
      }
      else {
        to->force_cancel();
        ++to->num_expires;
        /*checkRange(traffic, status[traffic].module_id);*/
        checkRange(traffic, idSrc);
      }
    }
  }
}


void UwMultiTrafficRangeCtr::manageBuffer(int traffic)
{
  BufferTrafficFeature::iterator it_feat = buffer_feature_map.find(traffic);
  if (it_feat == buffer_feature_map.end()) {
    std::cout << "UwMultiTrafficControl::manageBuffer. ERROR. Buffer not "
              << "configured. Traffic " << traffic << std::endl;
    return;
  }
  Packet *p = getFromBuffer(traffic);
  if (p != NULL) {
    int l_id = getBestLowerLayer(traffic,p);
    StatusMap::iterator it_s = status.find(traffic);
    if (it_s == status.end() || status[traffic].status == IDLE) {
      double delay = it_feat->second.getUpdatedDelay(NOW);
      return l_id ? sendDown(l_id,removeFromBuffer(traffic),delay)
                  : sendDown(removeFromBuffer(traffic),delay); 
    }
  }
}
  
int UwMultiTrafficRangeCtr::getBestLowerLayer(int traffic, Packet *p) 
{
  if (debug_)
    std::cout << NOW << " UwMultiTrafficRangeCtr::getBestLowerLayer(" << traffic << ")" << std::endl;
  DownTrafficMap::iterator it = down_map.find(traffic); 
  if (it != down_map.end()) {
    StatusMap::iterator it_s = status.find(traffic);
    if (it_s == status.end()) {
      initStatus(traffic);
    }
    if (status[traffic].status == RANGE_CNF_WAIT) {// I'm checking the range
      if (debug_)
         std::cout << NOW << " UwMultiTrafficRangeCtr::getBestLowerLayer(" << traffic 
                   << ") status == RANGE_CNF_WAIT" << std::endl;
      return 0;
    }
    BehaviorMap temp = it->second;
    BehaviorMap::iterator it_b = temp.begin();
    for (; it_b!=temp.end(); ++it_b)
    {
      int module_id_tmp = it_b->second.first;
      switch (it_b->second.second)
      {
        case(CHECK_RANGE):
        {
          if (debug_)
            std::cout << NOW << " UwMultiTrafficRangeCtr::getBestLowerLayer(" << traffic << "): CHECK_RANGE" 
                      << "PROBING" << it_b->second.first << std::endl;
          if(status[traffic].module_ids.find(module_id_tmp)==status[traffic].module_ids.end())
            status[traffic].module_ids.insert(module_id_tmp);
          checkRange(traffic, it_b->second.first, HDR_CMN(p)->next_hop());
          break;
        }
        case(ROBUST):
        {
          if (debug_)
            std::cout << NOW << " UwMultiTrafficRangeCtr::getBestLowerLayer(" << traffic << "):  ROBUST" 
                      << std::endl;
          status[traffic].robust_id = module_id_tmp;
          break;
        }
        default:
        {
          std::cout << NOW << " UwMultiTrafficRangeCtr:: DEFAULT NOT ALLOWED" << std::endl;
          break;
        }
      }
    }
    if (status[traffic].status == IDLE && status[traffic].robust_id)
      return status[traffic].robust_id;
  }
  return 0;
}

void UwMultiTrafficRangeCtr::checkRange(int traffic, int module_id, uint8_t destAdd) 
{
  StatusMap::iterator it_s = status.find(traffic);
  if (it_s == status.end()) {
    initStatus(traffic);
  }
  if(status[traffic].status == RANGE_CNF_WAIT){//already checking 
    if (debug_)
      std::cout << NOW << " UwMultiTrafficRangeCtr::checkRange ALREADY CHECKING" << endl;
  }
  else {
    status[traffic].status = RANGE_CNF_WAIT;
    std::map<int, UwCheckRangeTimer*>::iterator it_t = timers.find(traffic);
    if (it_t == timers.end()) {
      UwCheckRangeTimer *t_o = new UwCheckRangeTimer(this,traffic);
      t_o->resched(check_to_period * (t_o->num_expires + 1));
      timers.insert(std::pair<int,UwCheckRangeTimer*>(traffic, t_o));
    }
    else{
      it_t->second->resched(check_to_period * (it_t->second->num_expires + 1));
    }
  }
  Packet* p = Packet::alloc();
  hdr_cmn* ch = hdr_cmn::access(p);
  ch->ptype() = PT_MULTI_TR_PROBE;
  ch->size() = signaling_pktSize;
  hdr_uwm_tr* tr = HDR_UWMTR(p);
  tr->traffic() = traffic;
  hdr_uwip* iph  = HDR_UWIP(p);
  UWIPClMsgSendAddr msg;
  sendSyncClMsgUp(&msg);
  iph->saddr() =  msg.getAddr();
  iph->daddr() = destAdd;
  hdr_uwcbr *ah = HDR_UWCBR(p);
  ah->priority() = 1;
  incrSignalingCounter(traffic, tx_probe_cnt);
  sendDown(module_id, p);
  if (debug_)
  std::cout << NOW << " UwMultiTrafficRangeCtr(" << (int)iph->saddr() << ")::checkRange " 
            << "sending PT_MULTI_TR_PROBE to " << (int) iph->saddr() << std::endl;
}

void UwMultiTrafficRangeCtr::timerExpired(int traffic) 
{
  if(debug_)
    std::cout << NOW << " UwMultiTrafficRangeCtr::timerExpired(" << traffic << ")" << std::endl;
  StatusMap::iterator it_s = status.find(traffic);
  if (it_s == status.end())
    return;
  if (status[traffic].status != RANGE_CNF_WAIT)
    return;
  if (status[traffic].robust_id)
  {
    std::map<int, UwCheckRangeTimer*>::iterator it_t = timers.find(traffic);
    if (it_t != timers.end()) {
      it_t->second->num_expires = 0;
    }
    Packet *p = getFromBuffer(traffic);
    if(p!=NULL)
      manageCheckedLayer(traffic, HDR_UWIP(p)->daddr(), false);
    status[traffic].status = IDLE;
  }
  else 
  {
    status[traffic].status = IDLE;
    for (std::set<int>::iterator it = status[traffic].module_ids.begin(); 
         it!= status[traffic].module_ids.end(); ++it){
      if(*it > 0) {
        checkRange(traffic, *it, UWIP_BROADCAST);
      }
    }
  }
}

void UwMultiTrafficRangeCtr::initStatus(int traffic)
{
  check_status default_st;
  default_st.status = IDLE;
  default_st.module_ids = set<int>();
  default_st.robust_id = 0;
  status[traffic] = default_st;
}

void UwMultiTrafficRangeCtr::incrSignalingCounter(int traffic, 
    CounterMap& map_cnt)
{
  CounterMap::iterator it = map_cnt.find(traffic);
  if (it != map_cnt.end()) {
    it->second++;
  } else {
    map_cnt[traffic] = 1;
  }
}

uint UwMultiTrafficRangeCtr::getSignalingCounter(int traffic, 
    const CounterMap& map_cnt) const
{
  CounterMap::const_iterator it = map_cnt.find(traffic);
  if (it == map_cnt.end()) {
    return 0;
  }	else {
    return it->second;
  }
}
