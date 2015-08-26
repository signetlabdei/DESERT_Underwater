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
 * @file   uwinterference.cc
 * @author Federico Favaro
 * @version 1.0.0
 * 
 * \brief Implementation of uwinterference class.
 * 
 */


#include <mac.h>

#include <mphy.h>
#include <list>
#include <stdlib.h>
#include <iostream>

#include "scheduler.h"


#include "uwinterference.h"



#define POWER_PRECISION_THRESHOLD (1e-14)
#define EPSILON_TIME 0.000001
#define DOUBLE_CHECK 0.000001
#define DOUBLE_CHECK_LOW 0.001

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class Interference_Class : public TclClass {
public:
  /**
   * Constructor of the class
   */
  Interference_Class() : TclClass("Module/UW/INTERFERENCE") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
    return (new uwinterference);
  }
} class_interference;

void EndCollisionEventTimer::handle(Event* e)
{
    //CollisionEvent* ce = dynamic_cast<CollisionEvent*>(e);
    CollisionEvent* ce = (CollisionEvent*)e;
    attr->addToCollisionCounters(ce->type,NOW,-1);
    delete ce;
}

uwinterference::uwinterference() 
:       end_collision_timer(this),
        initial_interference_time(0),
        start_rx_time(0),
        end_rx_time(0)
{

}

uwinterference::~uwinterference()
{
    
}

void uwinterference::addToInterference(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);
  hdr_mac *mach = HDR_MAC(p);
  //CollisionEvent* ce;
  PKT_TYPE type_rcv_pkt;
  if (mach->ftype() == MF_CONTROL) 
  {
      //ce->type = CTRL;
      type_rcv_pkt = CTRL;
      addToCollisionCounters(CTRL,NOW,1);
  }
  else 
  {
      //ce->type = DATA;
      type_rcv_pkt = DATA;
      addToCollisionCounters(DATA,NOW,1);
  }
  CollisionEvent* ce = new CollisionEvent(type_rcv_pkt);
  //Scheduler::instance().schedule(&endinterftimer, pe, ph->duration - EPSILON_TIME);
  Scheduler::instance().schedule(&end_collision_timer, ce, ph->duration - EPSILON_TIME);
  MInterferenceMIV::addToInterference(p);
}

void uwinterference::addToCollisionCounters(PKT_TYPE type,double starttime,int incr_value)
{
    CollisionFunction::iterator it;
    for (it = cp.begin(); it!=cp.end();)
    {
        if(it->time < starttime - maxinterval_)
        {
            it = cp.erase(it);
        } 
        else
        {
            break;
        }
           
   }
   
    if(cp.empty())
    {
        if (type == CTRL)
        {
                CollisionType newpoint(incr_value,0,NOW);
                cp.push_back(newpoint);
        }
        else if( type == DATA ) 
        {
            CollisionType newpoint(0,incr_value,NOW);
            cp.push_back(newpoint);
        }
    }
    else
    {
        CollisionType lastcollision(cp.back());
        if ( type == CTRL) 
        {
            CollisionType newcollision(lastcollision.CtrlPkts + incr_value, lastcollision.DataPkts,NOW);
            if ((newcollision.CtrlPkts < 0) || (newcollision.DataPkts < 0))
            {
                if (debug_) cout << "WARNING: cancellation error" << endl;
                if(newcollision.CtrlPkts < 0)
                {
                    newcollision.CtrlPkts = 0;
                }
                else
                {
                    newcollision.DataPkts = 0;
                }
            }
            cp.push_back(newcollision);
            
        }
        else if ( type == DATA)
        {
            CollisionType newcollision(lastcollision.CtrlPkts, lastcollision.DataPkts + incr_value,NOW);
            if ((newcollision.CtrlPkts < 0) || (newcollision.DataPkts < 0))
            {
                if (debug_) cout << "WARNING: cancellation error" << endl;
                if(newcollision.CtrlPkts < 0)
                {
                    newcollision.CtrlPkts = 0;
                }
                else
                {
                    newcollision.DataPkts = 0;
                }
            }
            cp.push_back(newcollision);
        }
        else {
         cerr << "Type of the packet not valid!!!" << endl;
         exit(1);
        }
    }
}

counter uwinterference::getCounters(Packet* p)
{
    hdr_mac* mach = HDR_MAC(p);
    hdr_MPhy* ph = HDR_MPHY(p);
    
    PKT_TYPE recv_pkt_type;
    if(mach->ftype() == MF_CONTROL)
    {
        recv_pkt_type = CTRL;
    }
    else
    {
        recv_pkt_type = DATA;
    }
    return (getCounters(ph->rxtime,recv_pkt_type));
    
}

counter uwinterference::getCounters(double starttime, PKT_TYPE type)
{                                                                                                           
    CollisionFunction::reverse_iterator rit;
    
    int ctrl_pkts = 0;
    int data_pkts = 0;
    
    assert(starttime < NOW);
    
    for(rit=cp.rbegin(); rit!=cp.rend(); ++rit)
    {
        if(starttime < rit->time)
        {
            ctrl_pkts += rit->CtrlPkts;
            data_pkts += rit->DataPkts;
        }
        else
        {
            ctrl_pkts += rit->CtrlPkts;
            data_pkts += rit->DataPkts;
            break;
        }
    }
    if ( type == CTRL)
    {
        ctrl_pkts--;
    }
    else
    {
        data_pkts--;
    }
    return counter(ctrl_pkts,data_pkts);
}

const PowerChunkList& uwinterference::getInterferencePowerChunkList(Packet* p)
{
    return MInterferenceMIV::getInterferencePowerChunkList(p);
}

double uwinterference::getCurrentTotalPower()
{
    return MInterferenceMIV::getCurrentTotalPower();
}

