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
 * @file   uwmulti-traffic-control.cc
 * @author Filippo Campagnaro, Federico Guerra
 * @version 1.0.0
 *
 * \brief Implementation of UwMultiTrafficControl class.
 *
 */

#include "uwmulti-traffic-control.h"
#include <clmsg-discovery.h>
#include <uwcbr-module.h>
#include <iostream>

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwMultiTrafficControlClass : public TclClass 
{
public:
  /**
   * Constructor of the class
   */
  UwMultiTrafficControlClass() : TclClass("Module/UW/MULTI_TRAFFIC_CONTROL") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) 
  {
    return (new UwMultiTrafficControl);
  }
} class_traffic_control;

UwMultiTrafficControl::UwMultiTrafficControl() 
: 
  Module(),
  debug_(0),
  up_map(),
  down_map(),
  down_buffer(),
  buffer_feature_map()
{
  bind("debug_", &debug_);
}

int UwMultiTrafficControl::command(int argc, const char*const* argv) 
{
  Tcl &tcl = Tcl::instance();
  if (argc == 3)
  {
    if(strcasecmp(argv[1], "getDiscardedPacket") == 0)
    {
      tcl.resultf("%d", getDiscardedPacket(atoi(argv[2])));
      return TCL_OK;
    }
  }
  else if (argc == 4) 
  {
    if(strcasecmp(argv[1], "addUpLayer") == 0)
    {
      addUpLayerFromTag(atoi(argv[2]),argv[3]);
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "addLowLayer") == 0)
    {
      addLowLayerFromTag(atoi(argv[2]),argv[3],DEFAULT);
      return TCL_OK;
    }
  }
  else if (argc == 5) 
  {
    if(strcasecmp(argv[1], "addLowLayer") == 0)
    {
      addLowLayerFromTag(atoi(argv[2]),argv[3],atoi(argv[4]));
      return TCL_OK;
    } 
    else if (strcasecmp(argv[1], "setBufferFeatures") == 0)
    {
      setBufferFeature(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
      return TCL_OK;    
    }
  }
  else if (argc == 6)
  {
    if (strcasecmp(argv[1], "setBufferFeatures") == 0)
    {
      setBufferFeature(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),atof(argv[5]));
      return TCL_OK;    
    }
  }

  return Module::command(argc, argv);     
} /* UwMultiTrafficControl::command */

void UwMultiTrafficControl::addUpLayerFromTag(int traffic_id, std::string tag) {
  ClMsgDiscovery m;
  m.addSenderData((const PlugIn*) this, getLayer(), getId(), getStackId(), name() , getTag());
  sendSyncClMsgUp(&m);
  DiscoveryStorage up_layer_storage = m.findTag(tag.c_str());
  if (debug_)
    std::cout << "UwMultiTrafficControl::addUpLayerFromTag(" << traffic_id << "," << tag << ")"
              << up_layer_storage.getSize() << std::endl;
  if (up_layer_storage.getSize() == 1) 
  {
    if (debug_) 
      std::cout << "UwMultiTrafficControl::addUpLayerFromTag(" << traffic_id << "," << tag << ")" <<
                (*up_layer_storage.begin()).first << std::endl;
    insertTraffic2UpLayer(traffic_id,(*up_layer_storage.begin()).first);
  }  
}

void UwMultiTrafficControl::addLowLayerFromTag(int traffic_id, std::string tag, int behavior) {
  ClMsgDiscovery m;
  m.addSenderData((const PlugIn*) this, getLayer(), getId(), getStackId(), name() , getTag());
  sendSyncClMsgDown(&m);
  DiscoveryStorage low_layer_storage = m.findTag(tag.c_str());
  DiscoveryData low_layer = (*low_layer_storage.begin()).second;
  if (debug_)
    std::cout << NOW << "UwMultiTrafficControl::addLowLayerFromTag(" << traffic_id << "," 
              << tag << ") disc size " << low_layer_storage.getSize() << endl;
  if (low_layer_storage.getSize() == 1) 
  {
    if (debug_)
      std::cout << NOW << "UwMultiTrafficControl::addLowLayerFromTag(" << traffic_id << "," 
                << tag << ") disc data tr = "<< traffic_id << " m_id = " << low_layer.getId() 
                << " st_id = " << low_layer.getStackId() << " " << behavior << std::endl;
    insertTraffic2LowerLayer(traffic_id,low_layer.getStackId(),low_layer.getId(),behavior);
  }  
}

void UwMultiTrafficControl::recv(Packet* p)
{
  hdr_cmn *ch = HDR_CMN(p);
  if(ch->direction() == hdr_cmn::UP)
  {   
    hdr_uwcbr *ah = HDR_UWCBR(p);
    int traf_type = ah->traffic_type();
    if(debug_)
      std::cout << NOW << " UwMultiTrafficControl::recv type = " << traf_type <<std::endl;
    sendUp(getUpperLayer(traf_type), p);
  }
  else //direction DOWN: packet is coming from upper layers
  {
    recvFromUpperLayers(p);
  }
}

void UwMultiTrafficControl::recvFromUpperLayers(Packet *p)
{
  hdr_cmn *ch = HDR_CMN(p);
  assert(ch->direction() == hdr_cmn::DOWN);

  hdr_uwcbr *ah = HDR_UWCBR(p);
  int traf_type = ah->traffic_type();
  if(debug_)
    std::cout << NOW << " UwMultiTrafficControl::recvFromUpperLayers" << std::endl;
  insertInBuffer(p,traf_type);
  manageBuffer(traf_type);
}

void UwMultiTrafficControl::insertInBuffer(Packet *p, int traffic) 
{
  BufferTrafficFeature::iterator it_feat = buffer_feature_map.find(traffic);
  if (it_feat == buffer_feature_map.end()) {
    std::cout << "UwMultiTrafficControl::insertInBuffer. ERROR. Buffer not "
              << "configured. Traffic " << traffic << std::endl;
    Packet::free(p);
    return;
  }

  DownTrafficBuffer::iterator it = down_buffer.find(traffic);
  if (it != down_buffer.end()) {
    uint n_elem = it->second->size();
    if (n_elem < it_feat->second.max_size) {
      it->second->push(p);
      if(debug_)
        std::cout << NOW <<" UwMultiTrafficControl::insertInBuffer, traffic = "
                  << traffic << ", buffer size =" << it->second->size() 
                  << std::endl;
    } else { 
      incrPktLoss(traffic);
      if (it_feat->second.behavior_buff == BufferType::CIRCULAR) { //circular buffer
        if (debug_)
          std::cout << NOW << "UwMultiTrafficControl::insertInBuffer, traffic = "
                    << traffic << ", circular buffer full. Discard first element"
                    << std::endl;
        it->second->pop();
        it->second->push(p);
      } else {
        if (debug_)
          std::cout << NOW << "UwMultiTrafficControl::insertInBuffer, traffic = "
                    << traffic << ", buffer full. Discard incoming packet "
                    << std::endl;
        Packet::free(p);          
      }
    }    
  }
  else {
    std::queue<Packet*> *q = new std::queue<Packet*>;
    q->push(p);
    down_buffer[traffic] = q;
    if(debug_)
      std::cout << NOW <<" UwMultiTrafficControl::insertInBuffer, traffic = "
                << traffic << ", buffer size =" << 1 << std::endl;
  }
}

void UwMultiTrafficControl::manageBuffer(int traffic)
{
  BufferTrafficFeature::iterator it_feat = buffer_feature_map.find(traffic);
  if (it_feat == buffer_feature_map.end()) {
    std::cout << "UwMultiTrafficControl::insertInBuffer. ERROR. Buffer not "
              << "configured. Traffic " << traffic << std::endl;
    return;
  }
  DownTrafficBuffer::iterator it = down_buffer.find(traffic);
  if (it != down_buffer.end()) {
    sendDown(getBestLowerLayer(traffic),removeFromBuffer(traffic),
        it_feat->second.getUpdatedDelay(NOW));
    if(debug_)
      std::cout << NOW << "UwMultiTrafficControl::manageBuffer(" << traffic << ")" << std::endl;
  }
}

Packet * UwMultiTrafficControl::removeFromBuffer(int traffic) 
{
  Packet * p = NULL;
  DownTrafficBuffer::iterator it = down_buffer.find(traffic);
  if (it != down_buffer.end() && ! it->second->empty()) {
    p = it->second->front();
    it->second->pop();
    if (debug_)
      std::cout << NOW << " UwMultiTrafficControl::removeFromBuffer(" << traffic 
                << "), packet in buffer = " << it->second->size() << std::endl;
  }
  return p;
}

Packet * UwMultiTrafficControl::getFromBuffer(int traffic) 
{
  Packet * p = NULL;
  DownTrafficBuffer::iterator it = down_buffer.find(traffic);
  if (it != down_buffer.end() && ! it->second->empty()) {
    if (debug_)
      std::cout << NOW << " UwMultiTrafficControl::getFromBuffer(" << traffic 
                << "), packet in buffer = " << it->second->size() << std::endl;
    p = it->second->front();
  }
  return p;
}
  
int UwMultiTrafficControl::getBestLowerLayer(int traffic, Packet *p) 
{
  DownTrafficMap::iterator it = down_map.find(traffic); 
  if (it != down_map.end()) {
    BehaviorMap temp = it->second;
    BehaviorMap::iterator it_b = temp.begin();
    for (; it_b!=temp.end(); ++it_b)
    {
      if (it_b->second.second == DEFAULT)
      {
        if (debug_)
          std::cout << NOW << " UwMultiTrafficControl::getBestLowerLayer(" 
                    << traffic << "), id = " 
                    << it_b->second.first << std::endl;
        return it_b->second.first;
      }
    }
  }
  return 0;
}

int UwMultiTrafficControl::getUpperLayer(int traffic) 
{
  UpTrafficMap::iterator it = up_map.find(traffic); 
  if (it != up_map.end()) {
    if(debug_)
      std::cout << NOW << " UwMultiTrafficControl::getUpperLayer(" <<traffic<<") = " 
                << it->second << std::endl;
    return it->second;
  }
  return 0;
}

void UwMultiTrafficControl::eraseTraffic2LowerLayer(int traffic, int lower_layer_stack)
{
  DownTrafficMap::iterator it = down_map.find(traffic); 
  if (it != down_map.end()) {
    BehaviorMap behav = it->second;
    BehaviorMap::iterator it_layer = behav.find(lower_layer_stack);
    if(it_layer != behav.end())
      behav.erase(lower_layer_stack);
    if(behav.size() == 0)
      down_map.erase(traffic);
  }
}

void UwMultiTrafficControl::eraseTraffic2Low(int traffic)
{
  UpTrafficMap::iterator it = up_map.find(traffic); 
  if (it != up_map.end()) {
    up_map.erase(traffic);
  }
}

void UwMultiTrafficControl::eraseTraffic2Up(int traffic)
{
  UpTrafficMap::iterator it = up_map.find(traffic); 
  if (it != up_map.end()) {
    up_map.erase(traffic);
  }
}

void UwMultiTrafficControl::setBufferFeature(int traffic_id, int max_size, 
    bool is_circular, double send_down_delay) 
{
  BufferType::BufferBehavior behav = is_circular ? BufferType::CIRCULAR :
      BufferType::DISCARD_INCOMING;
	BufferType buff_type = BufferType(max_size, behav, send_down_delay);
  buffer_feature_map.insert(std::make_pair(traffic_id, buff_type));
  
  if (debug_)
    std::cout << "Inserted buffer features for traffic " << traffic_id 
              << " with max size " << max_size << " Is circular " 
              << is_circular << std::endl;
}

void UwMultiTrafficControl::incrPktLoss(int traffic_id)
{
  BufferTrafficFeature::iterator it = buffer_feature_map.find(traffic_id);
  if (it != buffer_feature_map.end()) {
    it->second.pkts_lost++;
  } else {
    std::cout << "UwMultiTrafficControl::incrPktLoss. ERROR. Buffer not "
              << "configured. Traffic " << traffic_id << std::endl;
    return;
  }
}

uint UwMultiTrafficControl::getDiscardedPacket(int traffic_id) const
{
	BufferTrafficFeature::const_iterator it = buffer_feature_map.find(traffic_id);
  if (it == buffer_feature_map.end()) {
    std::cout << "UwMultiTrafficControl::incrPktLoss. ERROR. Buffer not "
              << "configured. Traffic " << traffic_id << std::endl;
    return 0;
  }	else {
    return it->second.pkts_lost;
  }
}