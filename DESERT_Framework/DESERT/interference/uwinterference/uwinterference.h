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
 * @file   uw-interference-miv-plain.h
 * @author Federico Favaro
 * @version 1.0.0
 * 
 * \brief Implementation of MinterferenceMIV_plain class.
 * 
 */



#ifndef UW_INTERFERENCE__H
#define	UW_INTERFERENCE__H

#include "interference_miv.h"

#include <scheduler.h>

enum PKT_TYPE {CTRL, DATA}; /**< Pkt type: CONTROL or DATA packet */

typedef std::pair<int,int> counter; /**< counter of collisions */
/**
 * Class that describes the type of the collision occured
 */
class CollisionType {
public:
    /**
     * Consstructor of the class CollisionType
     */
    CollisionType() {}
    /**
     * Contructor of the class CollisionType
     * @param ctrl 1 if the packet collided is a control packet
     * @param data 1 if the packet collided is a data packet
     * @param time_ Time at which the packet collided
     */
    CollisionType(int ctrl, int data, double time_) {CtrlPkts = ctrl; DataPkts = data; time = time_;}
    /**
     * Contructor of the class CollisionType
     * @param CollisionType Object of the class CollisionType
     */
    CollisionType(const CollisionType &c) {CtrlPkts = c.CtrlPkts; DataPkts = c.DataPkts; time = c.time;}
    double time; /**< Time at which the packet collided */
    int CtrlPkts; /**< 1 if the packet is a CTRL packet */
    int DataPkts; /**< 1 if the packet is a DATA packet */
};

typedef std::list<CollisionType> CollisionFunction;     /**< List of the collisions over time */
typedef std::list<CollisionType>::iterator CollisionFunctionIterator; /**< Iterator over the collision function */




/**
 * Class that represent a Collision Event 
 */
class CollisionEvent : public Event {
public:
    /**
     * Constructor of the class CollisionEvent
     */
    CollisionEvent() {}
    /**
     * Constructor of the class CollisionEvent
     * @param pkt type of the packet collided
     */
    CollisionEvent(PKT_TYPE pkt) : type(pkt) {} 
    /**
     * Destructor of the class CollisionEvent
     */
    virtual ~CollisionEvent() {}
    PKT_TYPE type; /**< Type of the packet collided */
};

class uwinterference;
/**
 * Class that represent the duration of a collision event
 */
class EndCollisionEventTimer : public Handler {
public:
    /**
     * 
     * @param ptr pointer to an object of type uwinterference
     */
    EndCollisionEventTimer(uwinterference* ptr) : attr(ptr) { }
    /**
     * Handle the Event represented by the timer
     * @param e Event
     */
    virtual void handle(Event *e);
protected:
    uwinterference* attr; /**< Pointer to an object of type uwinterference */
};



/**
 * Class that represent the interference model used in simulation
 */
class uwinterference : public MInterferenceMIV
{
    public:
        /**
         * Constructor of the class uwinterference
         */
        uwinterference();
        /**
         * Destructor of the class uwinterference
         */
        virtual ~uwinterference();
        /**
         * Returns the power of interference that a certain packet experienced
         * @param p The Packet for which the calculation of the interference experienced is needed
         * @return Amount of interference power
         */
        virtual double getInterferencePower(Packet* p);
        /**
         * Returns the power of interference that a certain packet experienced
         * @param power Reception power of the desired packet
         * @param starttime timestamp of the begin of the reception of the desired packet
         * @param endtime timestamp of the end of the reception of the desired packet
         * @return Amount of interference power
         */
        virtual double getInterferencePower(double power, double starttime, double endtime);
        /**
         * Add a packet to the interference calculation
         * @param p Pointer to the interferer packet
         */
        virtual void addToInterference(Packet* p);
        /**
         * Add a packet to the counter of collision
         * @param type type of the packet (DATA or CTRL)
         * @param starttime timestamp of the begin of the reception of the interferer packet
         * @param incr_value Increment value (+1 if the node is receiving the packet, -1 if reception of the packet is terminated)
         */
        virtual void addToCollisionCounters(PKT_TYPE type,double starttime, int incr_value);
        /**
         * Returns the counters of collisions
         * @param p Pointer of the packet for which the counters are needed
         * @return counter variable that represent the counters of the interference
         */
        virtual counter getCounters(Packet* p);
        /**
         * Returns the counters of collisions
         * @param starttime timestamp of the start of reception phase
         * @param type type of the packet (DATA or CTRL)
         * @return counter variable that represent the counters of the interference
         */
        virtual counter getCounters(double starttime, PKT_TYPE type);
        /**
         * return a list of Chunks (part in which a packet is divided )
         * @param p Pointer to the packet for the the chunk list is needed
         * @return list of the chunks with relative interference power experienced
         */
        virtual const PowerChunkList& getInterferencePowerChunkList(Packet* p);
        /**
         * Get the timestamp of the start of reception phase
         * @return timestamp of the start of reception phase
         */
        inline double getStartRxTime() {return start_rx_time;}
        /**
         * Get the timestamp of the end of reception phase
         * @return timestamp of the start of reception phase
         */
        inline double getEndRxTime() {return end_rx_time;}
        /**
         * Get the timestamp of the begin of reception of the first interferer packet 
         * @return timestamp of the begin of reception of the first interferer packet
         */
        inline double getInitialInterferenceTime() {return initial_interference_time; }
        /**
         * Get the total power of interference at this moment
         * @return amount of the interference power at this moment
         */
        virtual double getCurrentTotalPower();
        
   protected: 
       
       
       double initial_interference_time; /**< timestamp of the begin of reception of the first interferer packet */
       double start_rx_time; /**< timestamp of the start of reception phase */
       double end_rx_time; /**< timetamp of the end of reception phase */
       
       CollisionFunction cp; /**< Object of a type CollisionFunction */
       
       EndCollisionEventTimer end_collision_timer; /**< Timer for the event of finsh of collision between two concurrent packets */
};





#endif	/* UW_INTERFERENCE_COLLISION_COUNTER_H */

