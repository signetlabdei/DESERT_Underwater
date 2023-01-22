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
 * @file uwmulti-multiphy-controller.h
 * @author Enrico Lovisotto
 * @version 1.0.0
 *
 * \brief Definition of UwMultiPhyControl class.
 *
 */

#ifndef UWMULTI_MULTIPHY_CONTROLLER_H
#define UWMULTI_MULTIPHY_CONTROLLER_H

#include <rng.h>
#include <uwip-module.h>
#include <uwcbr-module.h>
#include <uwmulti-cmn-hdr.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>
#include <map>
#include <vector>
#include <queue>

#include <iostream>
#include <string.h>
#include <cmath>
#include <climits>
#include <limits>

std::string printType(packet_t ptype) {
  if (ptype == PT_UWMULTIPHY_DATA) {
    return "PT_UWMULTIPHY_DATA";
  }
  if (ptype == PT_UWMULTIPHY_PING) {
    return "PT_UWMULTIPHY_PING";
  }
  if (ptype == PT_UWMULTIPHY_PONG) {
    return "PT_UWMULTIPHY_PONG";
  }
  return "Unknown packet";
}

enum LinkStatus { LINK_UNKNOWN, LINK_OK, LINK_PROBING };

std::ostream& operator<<(std::ostream &o, LinkStatus s)
{
  switch (s) {
  case LINK_UNKNOWN: return o << "LINK_UNKNOWN";
  case LINK_OK:      return o << "LINK_OK";
  case LINK_PROBING: return o << "LINK_PROBING";
  default: return o<<"(invalid value)";
  }
}

// connection between local application and local MAC layer
struct LocalConnection {
  int trafficType;
  int localMacID;

  bool operator==(const LocalConnection &o) const
  {
    return trafficType == o.trafficType && localMacID == o.localMacID;
  }

  bool operator<(const LocalConnection &o) const
  {
    // this total order relationship has no actual meaning, only needed
    // to use struct as key in a map
    return trafficType < o.trafficType || localMacID < o.localMacID;
  }

  friend std::ostream& operator<<(std::ostream& os, const LocalConnection &c)
  {
    os << "<LocalConnection(trafficType=" << c.trafficType << "," << "localMacID=" << c.localMacID << ")>";
    return os;
  }
};

// connection between local MAC layer and remote destination IP
struct RemoteConnection {
  int localMacID;
  int remoteIP;

  bool operator==(const RemoteConnection &o) const
  {
    return remoteIP == o.remoteIP && localMacID == o.localMacID;
  }

  bool operator<(const RemoteConnection &o) const
  {
    // this total order relationship has no actual meaning, only needed
    // to use struct as key in a map
    return remoteIP < o.remoteIP || localMacID < o.localMacID;
  }

  friend std::ostream& operator<<(std::ostream& os, const RemoteConnection &c)
  {
    os << "<RemoteConnection(localMacID=" << c.localMacID << "," << "remoteIP=" << c.remoteIP << ")>";
    return os;
  }
};

class PriorityMap {
private:
  int debug_;

  //* default priority for each MAC
  std::map<int, int> defaultPriorities;

  //* custom priority for a given (local) traffic on each available MAC module
  std::map<LocalConnection, int > customPriorities;

public:
  PriorityMap();

  PriorityMap(int debug_);

  /**
   * Set MAC priority for packets coming from a specific traffic type
   *
   * @param c        local APP -> local MAC connection
   * @param priority priority, integer from -1 (minimum) to MAX_INT (maximum)
   */
  void addCustomPriority(LocalConnection c, int priority);

  /**
   * Set MAC default priority regardless of origin traffic type
   *
   * @param macID MAC module ID
   * @param priority priority, integer from -1 (minimum) to MAX_INT (maximum)
   */
  void setDefaultPriority(int macID, int priority);

  /**
   * Get priority of a certain MAC for packets of given traffic type
   * (using default priority if a custom one is not set)
   *
   * @param c  local APP -> local MAC connection
   * @returns  number, bigger as more prioritary
   */
  int getPriority(LocalConnection c);

  friend std::ostream& operator<< (std::ostream&, const PriorityMap&);
};

/**
 * Class used to represents the UwMultiPhyControl layer of a node.
 */
class UwMultiPhyControl : public Module {
private:
  int debug_;

  // characterize current node
  int localIP;
  std::vector<int> macIDs;

  std::queue<Packet*> packet_queue;

  //* each MAC state gets LINK_UNKNOWN after a timeout
  class ResilienceTimer;
  std::map<int, double> resilienceTimeouts;
  std::map<RemoteConnection, ResilienceTimer> resilienceTimers;

  //* each MAC state gets probed again after a timeout
  class ProbeTimer;

  std::map<int, double> probeTimeouts;
  std::map<RemoteConnection, ProbeTimer> probeTimers;

  // encode here how to choose the MAC for an application that
  // wants to communicate to a remote destination
  PriorityMap priorityMap;

  std::map<RemoteConnection, int> linkStatuses;

  /** <macID, resilience> where resilience score enstablish an order
    across local MAC layers: if a less robust one is LINK_OK all higher
    ones will be as well */
  std::map<int, int> macResilience;

  std::map<int, int> macTclIdLayerId;

  void initialize();

  int getBestMacID(int trafficType, int remoteIP);

  void updateAvailability(RemoteConnection c, LinkStatus status);

  void sendPing(RemoteConnection c);
  void recvFromUpperLayers(Packet *p);

  void recvFromLowerLayers(Packet *p, int macID);

public:
  /**
   * Constructor of UwMultiPhy class.
   */
  UwMultiPhyControl();

  /**
   * Destructor of UwMultiPhy class.
   */
  virtual ~UwMultiPhyControl() { }

  virtual int command(int, const char*const*);

  void recv(Packet *p);

  void recv(Packet *p, int idSrc);

  // timer classes
private:

  class ResilienceTimer : public TimerHandler {
  private:
    UwMultiPhyControl* module; /**< Pointer to the module class
                                where the timer is used*/
    RemoteConnection conn;

    /**
     * Timer expire procedure: handles the PROBE timeout
     * @param Event *e, pointer to the event that cause the expire
     */
    virtual void expire(Event *e);

  public:
    // default constructor is needed to be value of a map
    // set meaningless values then
    ResilienceTimer() : TimerHandler()
    {
      this->module = NULL;

      RemoteConnection c;
      c.localMacID = 0;
      c.remoteIP = 0;

      this->conn = c;
    };

    ResilienceTimer(UwMultiPhyControl *m, RemoteConnection c) :
      TimerHandler()
    {
      this->module = m;
      this->conn = c;
      if (this->module != NULL && this->module->debug_ >= 3) {
        std::cout << NOW
                  << "::UwMultiPhyControl::ResilienceTimer::constructor"
                  << "::localIP(" << this->module->localIP << ")"
                  << "::localMAC(" << this->conn.localMacID << ")"
                  << "::remoteIP(" << this->conn.remoteIP << ")"
                  << std::endl;
      }
    }
    ~ResilienceTimer() {
      if (this->module != NULL && this->module->debug_ >= 3) {
        std::cout << NOW
                 << "::UwMultiPhyControl::ResilienceTimer::destructor"
                 << "::localMacID(" << this->conn.localMacID << ")"
                 << "::remoteIP(" << this->conn.remoteIP << ")"
                 << "::module(" << this->module << ")"
                 << "::this(" << this << ")"
                 << std::endl;
      }
    }
  };

  class ProbeTimer : public TimerHandler {
  private:
    UwMultiPhyControl* module;
    RemoteConnection conn;

    /**
     * Timer expire procedure: handles the PROBE timeout
     * @param Event *e, pointer to the event that cause the expire
     */
    virtual void expire(Event *e);

  public:
    // default constructor is needed to be value of a map
    // set meaningless values then
    ProbeTimer() : TimerHandler()
    {
      this->module = NULL;

      RemoteConnection c;
      c.localMacID = 0;
      c.remoteIP = 0;

      this->conn = c;
    };

    ProbeTimer(UwMultiPhyControl *m, RemoteConnection c) : TimerHandler()
    {
      this->module = m;
      this->conn = c;
    };

    ~ProbeTimer() {
      if (this->module != NULL && this->module->debug_ >= 3) {
        std::cout << NOW
                 << "::UwMultiPhyControl::ProbeTimer::destructor"
                 << "::localMacID(" << this->conn.localMacID << ")"
                 << "::remoteIP(" << this->conn.remoteIP << ")"
                 << "::module(" << this->module << ")"
                 << "::this(" << this << ")"
                 << std::endl;
      }
    }
  };
};

#endif
