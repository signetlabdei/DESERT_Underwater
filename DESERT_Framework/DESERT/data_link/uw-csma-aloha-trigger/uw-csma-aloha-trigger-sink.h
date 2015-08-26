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
 * @file   uw-csma-aloha-trigger-sink.cc
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the declaration of UwCsmaAloha_Trigger_SINK class
 *
 */


#ifndef UW_CSMA_UW_CS_ALOHA_TRIG_SINK_TRIGGER_SINK_H 
#define UW_CSMA_UW_CS_ALOHA_TRIG_SINK_TRIGGER_SINK_H

#include "mmac.h"
#include "mphy.h"
#include "mac.h"


#include <iostream>

#define UW_CS_ALOHA_TRIG_SINK_DROP_REASON_WRONG_RECEIVER "WRCV" /**< The packet is for another node */
#define UW_CS_ALOHA_TRIG_SINK_DROP_REASON_UNKNOWN_TYPE "UPT" /**< The type of the packet is unknown */
#define UW_CS_ALOHA_TRIG_SINK_DROP_REASON_ERROR "ERR" /**< Packet corrupted */
#define UW_CS_ALOHA_TRIG_SINK_DROP_REASON_RECEIVING_NOT_ENABLED "RNE" /**< The sink is not enabled to receive data */


extern packet_t PT_MMAC_TRIGGER;

/**
 * Class that describes a UwCsmaAloha_Trigger_SINK module 
 */
class UwCsmaAloha_Trigger_SINK : public MMac {
public:

    /**
     * Constructor of the UwCsmaAloha_Trigger_SINK class
     */
    UwCsmaAloha_Trigger_SINK();
    /**
     * Destructor of the UwCsmaAloha_Trigger_SINK class
     */
    virtual ~UwCsmaAloha_Trigger_SINK();
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     **/
    virtual int command(int argc, const char*const* argv);
    /**
     * Cross-Layer messages interpreter
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received
     * @return <i>0</i> if successful.
     */
    virtual int crLayCommand(ClMessage* m);


protected:


    /**< Variable that rapresent the status of the protocol machine state */
    enum UW_CS_ALOHA_TRIG_SINK_STATUS {
        UW_CS_ALOHA_TRIG_SINK_STATE_IDLE = 1,
        UW_CS_ALOHA_TRIG_SINK_STATE_NOT_SET,
        UW_CS_ALOHA_TRIG_SINK_STATE_TX_TRIGGER,
        UW_CS_ALOHA_TRIG_SINK_STATE_ENABLE_RX,
        UW_CS_ALOHA_TRIG_SINK_STATE_DISABLE_RX,
        UW_CS_ALOHA_TRIG_SINK_STATE_DATA_RX
    };

    /**< Reason for the changing of the state */
    enum UW_CS_ALOHA_TRIG_SINK_REASON_STATUS {
        UW_CS_ALOHA_TRIG_SINK_REASON_DATA_RX = 1,
        UW_CS_ALOHA_TRIG_SINK_REASON_NOT_SET,
        UW_CS_ALOHA_TRIG_SINK_REASON_START_RX,
        UW_CS_ALOHA_TRIG_SINK_REASON_PKT_NOT_FOR_ME,
        UW_CS_ALOHA_TRIG_SINK_REASON_PKT_ERROR,
        UW_CS_ALOHA_TRIG_SINK_REASON_TX_TRIGGER
    };

    /**< Status of the timer */
    enum UW_CS_ALOHA_TRIG_SINK_TIMER_STATUS {
        UW_CS_ALOHA_TRIG_SINK_IDLE = 1, UW_CS_ALOHA_TRIG_SINK_RUNNING, UW_CS_ALOHA_TRIG_SINK_FROZEN, UW_CS_ALOHA_TRIG_SINK_EXPIRED
    };

    /**
     * Class that describes the timers in the node
     */
    class GenericTimer : public TimerHandler {
    public:

        /**
         * Constructor of the GenericTimer class
         * @param CsmaAloha* a pointer to an object of type CsmaAloha*
         */
        GenericTimer(UwCsmaAloha_Trigger_SINK *m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(UW_CS_ALOHA_TRIG_SINK_IDLE) {
            assert(m != NULL);
        }

        /**
         * Destructor of the GenericTimer class
         */
        virtual ~GenericTimer() {
        }

        /**
         * Freezes the timer
         */
        virtual void freeze() {
            assert(timer_status == UW_CS_ALOHA_TRIG_SINK_RUNNING);
            left_duration -= (NOW - start_time);
            if (left_duration <= 0.0) left_duration = module->mac2phy_delay_;
            force_cancel();
            timer_status = UW_CS_ALOHA_TRIG_SINK_FROZEN;
        }

        /**
         * unFreezes is used to resume the timer starting from the point where it was freezed
         */
        virtual void unFreeze() {
            assert(timer_status == UW_CS_ALOHA_TRIG_SINK_FROZEN);
            start_time = NOW;
            assert(left_duration > 0);
            sched(left_duration);
            timer_status = UW_CS_ALOHA_TRIG_SINK_RUNNING;
        }

        /**
         * stops the timer
         */
        virtual void stop() {
            timer_status = UW_CS_ALOHA_TRIG_SINK_IDLE;
            force_cancel();
        }

        /**
         * schedule a timer
         * @param double the duration of the timer
         */
        virtual void schedule(double val) {
            start_time = NOW;
            left_duration = val;
            timer_status = UW_CS_ALOHA_TRIG_SINK_RUNNING;
            resched(val);
        }

        /**
         * checks if the timer is IDLE
         * @return bool <i>true</i> or <i>false</i>
         */
        bool isIdle() {
            return ( timer_status == UW_CS_ALOHA_TRIG_SINK_IDLE);
        }

        /**
         * checks if the timer is RUNNING
         * @return <i>true</i> or <i>false</i>
         */
        bool isRunning() {
            return (timer_status == UW_CS_ALOHA_TRIG_SINK_RUNNING);
        }

        /**
         * Checks if the timer is EXPIRED
         * @return <i>true</i> or <i>false</i>
         */
        bool isExpired() {
            return (timer_status == UW_CS_ALOHA_TRIG_SINK_EXPIRED);
        }

        /**
         * Checks if the timer is FROZEN
         * @return <i>true</i> or <i>false</i>
         */
        bool isFrozen() {
            return (timer_status == UW_CS_ALOHA_TRIG_SINK_FROZEN);
        }

        /**
         * Checks if the timer is ACTIVE
         * @return <i>true</i> or <i>false</i>
         */
        bool isActive() {
            return (timer_status == UW_CS_ALOHA_TRIG_SINK_FROZEN || timer_status == UW_CS_ALOHA_TRIG_SINK_RUNNING);
        }

        /**
         * Resets the counter of the timer
         */
        void resetCounter() {
            counter = 0;
        }

        /**
         * Increments the counter of the timer
         */
        void incrCounter() {
            ++counter;
        }

        /**
         * Returns the counter of the timer
         * @return the value of the counter of the timer
         */
        int getCounter() {
            return counter;
        }

        /**
         * Returns the left duration of the timer
         * @return left duration of the timer
         */
        double getDuration() {
            return left_duration;
        }


    protected:


        double start_time; /**< Start Time of the timer */

        double left_duration; /**< Left duration of the timer */

        int counter; /**< counter of the timer */


        UwCsmaAloha_Trigger_SINK* module; /**< Pointer to an object of type CsmaAloha */

        UW_CS_ALOHA_TRIG_SINK_TIMER_STATUS timer_status; /**< Timer status */


    };

    /**
     * Class used to handle the timer of the reception period
     */
    class ReceiveTimer : public GenericTimer {
    public:

        /**
         * Conscructor of ReceiveTimer class 
         * @param UwCsmaAloha_Trigger_SINK* pointer to an object of type UwCsmaAloha_Trigger_SINK
         */
        ReceiveTimer(UwCsmaAloha_Trigger_SINK* m) : GenericTimer(m) {
        }

        /**
         * Descructor of ReceiveTimer class 
         */
        virtual ~ReceiveTimer() {
        }

    protected:
        /**
         * Method called when the timer expire 
         * @param Event* pointer to an object of type Event
         */
        virtual void expire(Event *e);
    };
    /**
     * Receives the packet from the upper layer (e.g. IP)
     * @param Packet* pointer to the packet received
     *
     */
    virtual void recvFromUpperLayers(Packet* p);
    /**
     * Pass the packet to the PHY layer
     * @param Packet* pointer to the packet received
     */
    virtual void Mac2PhyStartTx(Packet* p);
    /**
     * Method called when the PHY layers finishes to transmit packet 
     * @param Packet* pointer to the Packet just transmitted
     */
    virtual void Phy2MacEndTx(const Packet* p);
    /**
     * Method called when the PHY layer begins to receive a packet 
     * @param Packet* pointer to the packet in reception
     */
    virtual void Phy2MacStartRx(const Packet* p);
    /**
     * Method called when the PHY layer finishes to receive a packet
     * @param Packet* packet just received
     */
    virtual void Phy2MacEndRx(Packet* p);
    /**
     * Init the headers of the packet in transmission
     * @param Packet* packet to transmit
     * @param dest_adrr destination address of the packet 
     */
    virtual void initPkt(Packet* p, int dest_addr = 0);


    /**
     * State IDLE of the protocol. All variables resetted
     */
    virtual void stateIdle();
    /**
     * State in which the sink is enabled to receive packets
     */
    virtual void stateEnableRx();
    /**
     * The sink is not allowed to receive the packet from a certain node anymore
     */
    virtual void stateDisableRx();

    /**
     * The sink transmit the TRIGGER packet to begin the neighbour discovery phase
     */
    virtual void stateTxTRIGGER();
    /**
     * Transmit the TRIGGER packet
     */
    virtual void txTRIGGER(Packet* p);
    /**
     * The sink receives a DATA packet from a node
     */
    virtual void stateRxData(Packet* p);
    /**
     * The destination address of the packet is not equal to the sink's address
     */
    virtual void stateRxPacketNotForMe(Packet* p);
    /**
     * Refresh the state of the protocol 
     * @param UW_CS_ALOHA_TRIG_SINK_STATUS next state of the protocol
     */
    inline void refreshState(UW_CS_ALOHA_TRIG_SINK_STATUS state) {
        curr_state = state;
    }

    /**
     * Refresh the reason for the change of state
     */
    inline void refreshReason(UW_CS_ALOHA_TRIG_SINK_REASON_STATUS reason) {
        last_reason = reason;
    }

    /**
     * Return the number of TRIGGER packets sent over the simulation
     * @return number of trigger packets over the simulation
     */
    inline int getTriggerMsgSent() {
        return trigger_pkts_tx;
    }


    /**
     * Used for debug purposes. Wait until the user press a key
     */
    virtual void waitForUser();

    /**
     * Increment the number of Data received from upper layers
     */
    virtual void incrUpperDataRx() { up_data_pkts_rx++; }

    /**
     * Increment the number of TRIGGER packets transmitted
     */
    inline void incrTRIGGERPacketTx() { trigger_pkts_tx++;}

    int TRIGGER_size; /**< Size of the TRIGGER packet */
    int buffer_pkts; /**< Length of data packet queue */

    double tx_timer_duration; /**< Duration of the timer in which one node is allowed to transmit */


    bool receiving_state_active; /**< True if the sink is allowed to receive data packet */


    UW_CS_ALOHA_TRIG_SINK_STATUS curr_state; /**< Current state of the protocol */
    UW_CS_ALOHA_TRIG_SINK_STATUS prev_state; /**< Previous state of the protocol */
    UW_CS_ALOHA_TRIG_SINK_REASON_STATUS last_reason; /**< Last reason of the state change of the protocol */


    ReceiveTimer receive_timer; /**< timer of receive state */

    int trigger_pkts_tx; /**< Number of TRIGGER packet received */
};

#endif 
