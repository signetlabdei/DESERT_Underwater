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
* @file   uwmulti-traffic-range-ctr.h
* @author Filippo Campagnaro, Federico Guerra
* @version 1.0.0
*
* \brief Definition of UwMultiTrafficRangeCtr class.
*
*/

#ifndef UWMULTI_TRAFFIC_CONTROL_H
#define UWMULTI_TRAFFIC_CONTROL_H

#include "uwmulti-traffic-control.h"
#include <uwip-module.h>
#include <set>

#define HDR_UWMTR(P)      (hdr_uwm_tr::access(P))
// DEFINE BEHAVIORS
enum CONTROL_RANGE_BEHAVIORS {
  ROBUST = 2,
  CHECK_RANGE
};

// DEFINE STATES
enum TRAFFIC_STATES {
  IDLE = 1,
  RANGE_CNF_WAIT
};

struct check_status {
  std::set <int> module_ids;
  int status;
  int robust_id;
};
typedef std::map <int, check_status> StatusMap; /** traffic, status */

typedef struct hdr_uwm_tr {
  int tr_id_;    /**< Id of the traffic app layer. */
  static int offset_; /**< Required by the PacketHeaderManager. */

  /**
   * Reference to the offset_ variable.
   */
  inline static int& offset() {
      return offset_;
  }  
  
  inline static hdr_uwm_tr * access(const Packet * p) {
      return (hdr_uwm_tr*) p->access(offset_);
  }
  
  /**
   * Reference to the traffic ID.
   */
  int& traffic() {
      return tr_id_;
  }
  
} hdr_uwm_tr;


/**
 * Class used to represents the UwMultiTrafficRangeCtr layer of a node.
 */
class UwMultiTrafficRangeCtr : public UwMultiTrafficControl {


public:

  /**
   * Constructor of UwMultiPhy class.
   */
  UwMultiTrafficRangeCtr();

  /**
   * Destructor of UwMultiPhy class.
   */
  virtual ~UwMultiTrafficRangeCtr() { }

  /**
   * TCL command interpreter. It implements the following OTcl methods:
   *
   * @param argc Number of arguments in <i>argv</i>.
   * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> 
   *             is the name of the object).
   *
   * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
   */
  virtual int command(int, const char*const*);

  virtual void recv(Packet* p, int idSrc);

  virtual void sendDown(Packet* p) { if(p != NULL ) { Module::sendDown(p); } }

  virtual void sendDown(int moduleId, Packet* p) { if(p != NULL ) { Module::sendDown(moduleId, p); } }

protected:
  StatusMap status; /**< Map of status per traffic types*/
  double check_to_period; /**< Time-Out period*/
  int signaling_pktSize; /**< Signaling packet size*/
  /** 
   * manage to tx a packet of traffic type
   *
   * @param traffic application traffic id
   */
  virtual void manageBuffer(int traffic);

  /** 
   * return the Best Lower Layer id where to forward the packet of <i>traffic</i> type
   * 
   * @param traffic application traffic id
   *
   * @return the layer id
   */
  virtual int getBestLowerLayer(int traffic, Packet *p = NULL);

  /** 
   * procedure to check if a 
   * 
   * @param traffic application traffic id
   */
  virtual void checkRange(int traffic, int module_id, uint8_t destAdd = UWIP_BROADCAST);

  /** 
   * procedure when a CHECKED stack is checked
   * 
   * @param module id
   * @param in_range true if the PHY is in range, false otherwise
   */
  virtual void manageCheckedLayer(int module_id, uint8_t destAdd, bool in_range, int idSrc = 0);

  /** 
   * default status initialization
   * 
   * @param traffic application traffic id
   */
  virtual void initStatus(int traffic);

  /** 
   * handle when a a timer expires
   * 
   * @param traffic application traffic id
   */
  virtual void timerExpired(int traffic);

private:
  /**
  * Definition of timer class. When it expires, the PROBE fails.
  */
  class UwCheckRangeTimer : public TimerHandler {
  public:

    UwCheckRangeTimer(UwMultiTrafficRangeCtr *m, int traff) : 
    TimerHandler(), 
    traffic(traff),
    max_increment(10),
    num_expires(0)
    {
        module = m;
    }
    ~UwCheckRangeTimer() {}
    int traffic; /**<traffic type*/
    int num_expires; /**< expires counter*/
    int const max_increment; /**< max increment of the timeout*/

  protected:
    /**
     * Timer expire procedure: handles the PROBE timeout
     * @param Event *e, pointer to the event that cause the expire
     */
    virtual void expire(Event *e);
    UwMultiTrafficRangeCtr* module; /**< Pointer to the module class 
                                         where the timer is used*/
  };
  //Variables
  std::map <int, UwCheckRangeTimer*> timers; /**< Map of timer per traffic types*/
};

#endif /* UWMULTI_TRAFFIC_CONTROL_H  */
