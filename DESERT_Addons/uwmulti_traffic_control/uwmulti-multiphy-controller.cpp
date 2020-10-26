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
 * @file   uwmulti-multiphy-controller.cc
 * @author Enrico Lovisotto
 * @version 1.0.0
 *
 * \brief Implementation of UwMultiPhyControl class.
 *
 */

#include "uwmulti-multiphy-controller.h"
#include "uwmulti-cmn-hdr.h"
#include <mphy_pktheader.h>
#include "mac.h"
#include "ip.h"
#include <mmac-clmsg.h>
#include <uwip-clmsg.h>
#include <algorithm>
#include <clmsg-discovery.h>

extern packet_t PT_UWCBR;

PriorityMap::PriorityMap()
{
  this->debug_ = 0;
}

PriorityMap::PriorityMap(int debug_)
{
  this->debug_ = debug_;
}

void PriorityMap::addCustomPriority(LocalConnection c, int priority)
{
  this->customPriorities[c] = priority;
}

void PriorityMap::setDefaultPriority(int macID, int priority)
{
  this->defaultPriorities[macID] = priority;
}

int PriorityMap::getPriority(LocalConnection c)
{
  // if connection is specified in custom settings
  if ( this->customPriorities.find(c) != this->customPriorities.end() ) {
    return this->customPriorities[c];
  }
  // if connection is in default priorities
  if ( this->defaultPriorities.find(c.localMacID) != this->defaultPriorities.end() ) {
    return this->defaultPriorities[c.localMacID];
  }

  // if no priority is available, return the "NULL" priority
  return -1;
}

ostream& operator<< (ostream& os, const PriorityMap& obj) {
  os << "<PriorityMap(debug_=" << obj.debug_ << ", ";

  os << "defaultPriorities={";
  for (std::map<int, int>::const_iterator it = obj.defaultPriorities.begin();
       it != obj.defaultPriorities.end();
       it++)
    {
      os << it->first << ": " << it->second << ", ";
    }
  os << "}, ";

  os << "customPriorities={";
  for (std::map<LocalConnection, int>::const_iterator it = obj.customPriorities.begin();
       it != obj.customPriorities.end();
       it++)
    {
      os << it->first << ": " << it->second << ", ";
    }
  os << "})>";
  return os;
}

/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwMultiPhyControlClass : public TclClass
{
public:
  /**
   * Constructor of the class
   */
  UwMultiPhyControlClass() : TclClass("Module/UW/MULTIPHY_CONTROLLER") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*)
  {
    return (new UwMultiPhyControl);
  }
} class_stack_controller;

UwMultiPhyControl::UwMultiPhyControl()
  :
  Module(),
  debug_(0)
{
	bind("debug_", &debug_);
  this->priorityMap = PriorityMap(debug_);
}

int UwMultiPhyControl::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();

  if (argc == 2) {

    if(strcasecmp(argv[1], "initialize") == 0) {
      this->initialize();
      return TCL_OK;
    }

  } else if (argc == 4) {

    if(strcasecmp(argv[1], "setResilienceTimeout") == 0) {
      // retrieve mac ID given Tcl name
      int tclId = atoi(argv[2]);
      int macID = this->macTclIdLayerId[tclId];

      double timeout = atof(argv[3]);

      this->resilienceTimeouts[macID] = timeout;
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "setProbeTimeout") == 0) {
      // retrieve mac ID given Tcl name
      int tclId = atoi(argv[2]);
      int macID = this->macTclIdLayerId[tclId];

      double timeout = atof(argv[3]);

      this->probeTimeouts[macID] = timeout;
      return TCL_OK;
    }
    
    if(strcasecmp(argv[1], "setMacResilience") == 0)
    {
      // retrieve mac ID given Tcl name
      int tclId = atoi(argv[2]);
      int macID = this->macTclIdLayerId[tclId];

      int score = atoi(argv[3]);

      this->macResilience[macID] = score;
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "setDefaultPriority") == 0) {
      // retrieve mac ID given Tcl name
      int tclId = atoi(argv[2]);
      int macID = this->macTclIdLayerId[tclId];

      int priority = atoi(argv[3]);

      this->priorityMap.setDefaultPriority(macID, priority);
      return TCL_OK;
    }
      
    if(strcasecmp(argv[1], "getBestMac") == 0) {
      int remoteIP = atoi(argv[2]);
      int trafficType = atoi(argv[3]);
      int bestMacID = this->getBestMacID(trafficType, remoteIP);

      tcl.resultf("%d", bestMacID);

      return TCL_OK;
    }
  } else if (argc == 5) {

    if(strcasecmp(argv[1], "addCustomPriority") == 0) {
      // retrieve mac ID given Tcl name
      int tclId = atoi(argv[2]);

      LocalConnection c;
      c.localMacID = this->macTclIdLayerId[tclId];
      c.trafficType = atoi(argv[3]);

      int priority = atoi(argv[4]);

      this->priorityMap.addCustomPriority(c, priority);
      return TCL_OK;
    }
  }

  return Module::command(argc, argv);
}

void UwMultiPhyControl::initialize()
{
  // send discovery ctrl message down
  ClMsgDiscovery m;
  m.addSenderData((const PlugIn*) this, getLayer(), getId(), getStackId(), name() , getTag());
  this->sendSyncClMsgDown(&m);
  DiscoveryStorage a = m.copyStorage();

  if (debug_ >= 2) {
    m.printReplyData();
  }

  for (DBIt it=a.begin(); it!=a.end();  it++) {
    int id = it->first;
    int layerId = it->second.getId();

    // unwanted layer (current one probably)
    if (id == 0) continue;

    // save retrieved information
    this->macIDs.push_back(layerId);

    // TODO check the one used in recv
    this->macTclIdLayerId[id] = layerId;

    if (debug_) {
      std::cout << "UwMultiPhyControl::initialize::tclId(" << id << ")"
                << "::layerId(" << layerId << ")" << std::endl;
    }
  }

  // retrieve and store local IP address
  UWIPClMsgSendAddr msg;
  this->sendSyncClMsg(&msg);
  this->localIP = msg.getAddr();
  if (debug_) {
    std::cout << "UwMultiPhyControl::initialize::localIP(" << this->localIP << ")" << std::endl;
  }
}

void UwMultiPhyControl::recv(Packet* p)
{
  std::cerr << "UwMultiPhyControl: a Packet is received without source module ID" << std::endl;
  Packet::free(p);
}

void UwMultiPhyControl::recv(Packet* p, int idSrc)
{
  hdr_cmn *ch = HDR_CMN(p);

  if (ch->direction() == hdr_cmn::DOWN) {
    // create a timer in preparation for the first packet retransmission
    this->recvFromUpperLayers(p);
  }
  else {
    this->recvFromLowerLayers(p, idSrc);
  }
}

void UwMultiPhyControl::recvFromLowerLayers(Packet *p, int macID)
{
  hdr_uwip* uwip = HDR_UWIP(p);
  hdr_cmn* cmn = HDR_CMN(p);

  int dst_ip = uwip->daddr();

  RemoteConnection c;
  c.remoteIP = uwip->saddr();
  c.localMacID = macID;

  if (debug_) {
    std::cout << NOW
             << "::UwMultiPhyControl::recvFromLowerLayers"
             << "::localIP(" << this->localIP << ")"
             << "::localMAC(" << macID << ")"
             << "::remoteIP(" << c.remoteIP << ")"
             << "::destIP(" << dst_ip << ")"
             << "::pktType(" << printType(cmn->ptype()) << ")"
             // << "::trafficType(" << cbr->trafficType() << ")"
             << std::endl;
  }

  // update availability and trigger its timeout
  // always, even for packets not for this node
  this->updateAvailability(c, LINK_OK);

  // send up only if data
  if (cmn->ptype() == PT_UWMULTIPHY_DATA) {
    // UWIP layer will
    // 1) reject packets sent for others
    // 2) route packets to right application

    // reset proper UWCBR type
    cmn->ptype() = PT_UWCBR;
    this->sendUp(p);
  }

  // link is alive: send corresponding packets in queue, if any
  bool packet_sent = false;
  unsigned int n_stored_packets = this->packet_queue.size();
  for (unsigned int i = 0; i < n_stored_packets; i++) {
    Packet* stored_p = this->packet_queue.front();
    this->packet_queue.pop();
    hdr_uwip* stored_uwip = HDR_UWIP(stored_p);

    if (stored_uwip->daddr() == c.remoteIP) {
      // process them as if they were a new packet
      this->recvFromUpperLayers(stored_p);
      packet_sent = true;
    }
    else {
      // put it again in queue
      this->packet_queue.push(stored_p);
    }
  }

  // answer with PONG only if it is for us and no DATA were sent in previous
  // queue scan
  if (!packet_sent &&
    cmn->ptype() == PT_UWMULTIPHY_PING &&
    uwip->daddr() == this->localIP) {

    Packet* pongPacket = Packet::alloc();

    // set PONG packet type
    hdr_cmn* pongCmn = HDR_CMN(pongPacket);
    pongCmn->ptype() = PT_UWMULTIPHY_PONG;
    pongCmn->size() = 2;

    // set destination and source address (current node one)
    hdr_uwip* pongIp = HDR_UWIP(pongPacket);

    pongIp->saddr() = this->localIP;
    pongIp->daddr() = c.remoteIP;

    Packet::free(p);

    // this triggers errors: lower layer IDs are not the right ones
    this->sendDown(c.localMacID, pongPacket);

    if (debug_) {
      std::cout << NOW
               << "::UwMultiPhyControl::sendPong"
               << "::localIP(" << this->localIP << ")"
               << "::localMAC(" << c.localMacID << ")"
               << "::remoteIP(" << c.remoteIP << ")"
               << "::pktType(" << printType(pongCmn->ptype()) << ")"
               << std::endl;
    }
  }
}

void UwMultiPhyControl::recvFromUpperLayers(Packet *p)
{
  hdr_uwip* uwip = HDR_UWIP(p);
  hdr_cmn* cmn = HDR_CMN(p);
  hdr_uwcbr* uwcbr = HDR_UWCBR(p);

  // set packet as data
  cmn->ptype() = PT_UWMULTIPHY_DATA;

  // find connection to send packet into
  int trafficType = uwcbr->traffic_type();
  int remoteIP = uwip->daddr();

  if (debug_) {
    std::cout << NOW
             << "::UwMultiPhyControl::recvFromUpperLayers"
             << "::pktID(" << HDR_CMN(p)->uid_ << ")"
             << "::localIP(" << this->localIP << ")"
             << "::remoteIP(" << remoteIP << ")"
             << "::pktType(" << printType(cmn->ptype()) << ")"
             << "::trafficType(" << trafficType << ")"
             << std::endl;
  }

  int bestMacID = this->getBestMacID(trafficType, remoteIP);

  // if valid mac was found, proceed
  if (bestMacID != -1) {
    this->sendDown(bestMacID, p);

    if (debug_) {
      std::cout << NOW
                << "::UwMultiPhyControl::sendDown"
                << "::localIP(" << this->localIP << ")"
                << "::localMAC(" << bestMacID << ")"
                << "::remoteIP(" << remoteIP << ")"
                << "::pktType(" << printType(cmn->ptype()) << ")"
                << std::endl;
    }

  } else {
    // send PINGs trying to reach remote node
    for (uint i=0; i < this->macIDs.size(); i++) {
      RemoteConnection c;
      c.localMacID = this->macIDs[i];
      c.remoteIP = remoteIP;

      if (this->linkStatuses[c] == LINK_UNKNOWN) {
        this->sendPing(c);
      }
    }

    // put packet in queue, waiting for a PONG
    this->packet_queue.push(p);
  }
}

void UwMultiPhyControl::sendPing(RemoteConnection c) {
    // this should not be called unless link condition is UNKNOWN
    if (this->linkStatuses[c] != LINK_UNKNOWN) {
      std::cerr << NOW
               << "::UwMultiPhyControl::sendPing"
               << "::localIP(" << this->localIP << ")"
               << "::remoteIP(" << c.remoteIP << ")"
               << "::localMAC(" << c.localMacID << ")"
               << "::(ping to be sent while in " << this->linkStatuses[c] << ")"
               << std::endl;
      return;
    }

    this->linkStatuses[c] = LINK_PROBING;

    // send ping packet
    Packet* pingPacket = Packet::alloc();

    hdr_cmn* ch = HDR_CMN(pingPacket);
    ch->ptype() = PT_UWMULTIPHY_PING;
    // check good value: this one comes from module TCL defaults
    ch->size() = 2;

    // set destination and source address (current node one)
    hdr_uwip* pingIPHeader = HDR_UWIP(pingPacket);

    pingIPHeader->saddr() = this->localIP;
    pingIPHeader->daddr() = c.remoteIP;

    if (debug_) {
      std::cout << NOW
               << "::UwMultiPhyControl::sendPing"
               << "::localIP(" << this->localIP << ")"
               << "::localMAC(" << c.localMacID << ")"
               << "::remoteIP(" << c.remoteIP << ")"
               << "::pktType(" << printType(ch->ptype()) << ")"
               << std::endl;
    }

    this->sendDown(c.localMacID, pingPacket);

    // ensure the requested timer exists
    if ( this->probeTimers.find(c) == this->probeTimers.end() ) {
      this->probeTimers[c] = ProbeTimer(this, c);
    }

    double probeTimeout = this->probeTimeouts[c.localMacID];
    this->probeTimers[c].resched(probeTimeout);
}

void UwMultiPhyControl::updateAvailability(RemoteConnection c, LinkStatus status)
{
  if (debug_) {
    std::cout << NOW << "::"
             <<"UwMultiPhyControl::updateAvailability"
             << "::localIP(" << this->localIP << ")"
             << "::localMAC(" << c.localMacID << ")"
             << "::remoteIP(" << c.remoteIP << ")"
             << "::new status " << status << std::endl;
  }

  if (status == LINK_OK) {
    //Reset PROBE timers if a packet is received from that connection
    if (this->linkStatuses[c] == LINK_PROBING) {
      if (this->probeTimers.find(c) != this->probeTimers.end()) {
        this->probeTimers[c].force_cancel();
        if (debug_) {
          std::cout << NOW << "::"
             << "UwMultiPhyControl::updateAvailability"
             << "::localIP(" << this->localIP << ")"
             << "::localMAC(" << c.localMacID << ")"
             << "::remoteIP(" << c.remoteIP << ")"
             << "::Reset probe timer" << std::endl;
        }
      }
    }

    this->linkStatuses[c] = LINK_OK;

    if ( this->resilienceTimers.find(c) == this->resilienceTimers.end() ) {
      this->resilienceTimers[c] = ResilienceTimer(this, c);
    }

    double timeout = resilienceTimeouts[c.localMacID];
    this->resilienceTimers[c].resched(timeout);
  }
  else {
    this->linkStatuses[c] = status;
  }
}

int UwMultiPhyControl::getBestMacID(int trafficType, int remoteIP)
{
  int bestMAC = -1;
  int bestPriority = -1;

  for (uint i=0; i < this->macIDs.size(); i++) {
    int macID = macIDs[i];

    // build the two connections
    LocalConnection localConn;
    localConn.localMacID = macID;
    localConn.trafficType = trafficType;

    RemoteConnection remoteConn;
    remoteConn.localMacID = macID;
    remoteConn.remoteIP = remoteIP;

    // pick the most prioritary MAC that can reach the destination
    if (this->linkStatuses[remoteConn] == LINK_OK) {
      int currentPriority = this->priorityMap.getPriority(localConn);
      if (currentPriority > bestPriority) {
        bestMAC = macID;
        bestPriority = currentPriority;
      }
    }
  }

  return bestMAC;
}

void UwMultiPhyControl::ResilienceTimer::expire(Event *e)
{
  if (this->module == NULL) {
    return;
  }

  if (this->module->debug_) {
    std::cout << NOW
              << "::UwMultiPhyControl::ResilienceTimer::Expire"
              << "::localIP(" << this->module->localIP << ")"
             << "::localMAC(" << this->conn.localMacID << ")"
             << "::remoteIP(" << this->conn.remoteIP << ")"
              << std::endl;
  }
  this->module->updateAvailability(this->conn, LINK_UNKNOWN);
}

void UwMultiPhyControl::ProbeTimer::expire(Event *e)
{
  if (this->module == NULL) {
    return;
  }

  if (this->module->debug_) {
    std::cout << NOW
              << "::UwMultiPhyControl::ProbeTimer::Expire"
              << "::localIP(" << this->module->localIP << ")"
              << "::localMAC(" << this->conn.localMacID << ")"
              << "::remoteIP(" << this->conn.remoteIP << ")"
              << std::endl;
  }

  this->module->linkStatuses[this->conn] = LINK_UNKNOWN;
  this->module->sendPing(this->conn);
}
