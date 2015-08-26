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
 * @file   uw-csma-aloha-triggered-node.h
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the declaration of UwCsmaAloha_Trigger_NODE class
 *
 */


#ifndef UW_CS_ALOHA_TRIG_NODE_H
#define UW_CS_ALOHA_TRIG_NODE_H

#include <mmac.h>
#include <mac.h>
#include <mphy.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <cmath>
#include <climits>
#include <iomanip>
#include <rng.h>


#define UW_CS_ALOHA_TRIG_NODE_DROP_REASON_BUFFER_FULL "DBF"     /**< The Buffer of DATA packets is full */
#define UW_CS_ALOHA_TRIG_NODE_DROP_REASON_ERROR "ERR"           /**< Packet corrupted */

extern packet_t PT_MMAC_TRIGGER;

/**
 * Class that describes a CsmaAloha_TRIGGERED module of the node
 */
class UwCsmaAloha_Trigger_NODE : public MMac {
public:

    /**
     * Constructor of the UwCsmaAloha_Trigger_NODE class
     */
    UwCsmaAloha_Trigger_NODE();
    /**
     * Destructor of the UwCsmaAloha_Trigger_NODE class
     */
    virtual ~UwCsmaAloha_Trigger_NODE();
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


    static const double prop_speed; /**< Typical sound propagation speed in underwater enviroment */

    /**< Variable that rapresent the status of the protocol machine state */
    enum UW_CS_ALOHA_TRIG_NODE_STATUS {
        UW_CS_ALOHA_TRIG_NODE_STATE_IDLE = 1, UW_CS_ALOHA_TRIG_NODE_STATE_TX_DATA,
        UW_CS_ALOHA_TRIG_NODE_STATE_DATA_RX, UW_CS_ALOHA_TRIG_NODE_STATE_NOT_SET, UW_CS_ALOHA_TRIG_NODE_STATE_LISTEN,
        UW_CS_ALOHA_TRIG_NODE_STATE_CHK_LISTEN_TIMEOUT, UW_CS_ALOHA_TRIG_NODE_STATE_RX_IDLE,
        UW_CS_ALOHA_TRIG_NODE_STATE_RX_LISTEN, UW_CS_ALOHA_TRIG_NODE_STATE_WRONG_PKT_RX, UW_CS_ALOHA_TRIG_NODE_STATE_RX_TRIGGER
    };

    /**< Type of the packet */
    enum UW_CS_ALOHA_TRIG_NODE_REASON_STATUS {
        UW_CS_ALOHA_TRIG_NODE_REASON_DATA_PENDING, UW_CS_ALOHA_TRIG_NODE_REASON_DATA_RX, UW_CS_ALOHA_TRIG_NODE_REASON_DATA_TX,
        UW_CS_ALOHA_TRIG_NODE_REASON_DATA_EMPTY, UW_CS_ALOHA_TRIG_NODE_REASON_NOT_SET, UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN, UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_TIMEOUT,
        UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_PENDING, UW_CS_ALOHA_TRIG_NODE_REASON_START_RX, UW_CS_ALOHA_TRIG_NODE_REASON_PKT_NOT_FOR_ME,
        UW_CS_ALOHA_TRIG_NODE_REASON_PKT_ERROR, UW_CS_ALOHA_TRIG_NODE_REASON_LISTEN_AFTER_TRIGGER, UW_CS_ALOHA_TRIG_NODE_REASON_TRIGGER_RX
    };

    /**< Status of the timer */
    enum UW_CS_ALOHA_TRIG_NODE_TIMER_STATUS {
        UW_CS_ALOHA_TRIG_NODE_IDLE = 1, UW_CS_ALOHA_TRIG_NODE_RUNNING, UW_CS_ALOHA_TRIG_NODE_FROZEN, UW_CS_ALOHA_TRIG_NODE_EXPIRED
    };

    /**
     * Class that describes the timers in the node
     */
    class Csma_Aloha_Triggered_Timer : public TimerHandler {
    public:

        /**
         * Constructor of the AlohaTimer class
         * @param CsmaAloha* a pointer to an object of type CsmaAloha*
         */
        Csma_Aloha_Triggered_Timer(UwCsmaAloha_Trigger_NODE *m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(UW_CS_ALOHA_TRIG_NODE_IDLE) {
            assert(m != NULL);
        }

        /**
         * Destructor of the AlohaTimer class
         */
        virtual ~Csma_Aloha_Triggered_Timer() {
        }

        /**
         * Freezes the timer
         */
        virtual void freeze() {
            assert(timer_status == UW_CS_ALOHA_TRIG_NODE_RUNNING);
            left_duration -= (NOW - start_time);
            if (left_duration <= 0.0) left_duration = module->mac2phy_delay_;
            force_cancel();
            timer_status = UW_CS_ALOHA_TRIG_NODE_FROZEN;
        }

        /**
         * unFreezes is used to resume the timer starting from the point where it was freezed
         */
        virtual void unFreeze() {
            assert(timer_status == UW_CS_ALOHA_TRIG_NODE_FROZEN);
            start_time = NOW;
            assert(left_duration > 0);
            sched(left_duration);
            timer_status = UW_CS_ALOHA_TRIG_NODE_RUNNING;
        }

        /**
         * stops the timer
         */
        virtual void stop() {
            timer_status = UW_CS_ALOHA_TRIG_NODE_IDLE;
            force_cancel();
        }

        /**
         * schedule a timer
         * @param double the duration of the timer
         */
        virtual void schedule(double val) {
            start_time = NOW;
            left_duration = val;
            timer_status = UW_CS_ALOHA_TRIG_NODE_RUNNING;
            resched(val);
        }

        /**
         * checks if the timer is IDLE
         * @return bool <i>true</i> or <i>false</i>
         */
        bool isIdle() {
            return ( timer_status == UW_CS_ALOHA_TRIG_NODE_IDLE);
        }

        /**
         * checks if the timer is RUNNING
         * @return <i>true</i> or <i>false</i>
         */
        bool isRunning() {
            return (timer_status == UW_CS_ALOHA_TRIG_NODE_RUNNING);
        }

        /**
         * Checks if the timer is EXPIRED
         * @return <i>true</i> or <i>false</i>
         */
        bool isExpired() {
            return (timer_status == UW_CS_ALOHA_TRIG_NODE_EXPIRED);
        }

        /**
         * Checks if the timer is FROZEN
         * @return <i>true</i> or <i>false</i>
         */
        bool isFrozen() {
            return (timer_status == UW_CS_ALOHA_TRIG_NODE_FROZEN);
        }

        /**
         * Checks if the timer is ACTIVE
         * @return <i>true</i> or <i>false</i>
         */
        bool isActive() {
            return (timer_status == UW_CS_ALOHA_TRIG_NODE_FROZEN || timer_status == UW_CS_ALOHA_TRIG_NODE_RUNNING);
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


        double start_time; /**< Start time of the timer */

        double left_duration; /**< Left duration of  the timer */

        int counter; /**< counter of the timer */


        UwCsmaAloha_Trigger_NODE* module; /**< Pointer of an object of type UwCsmaAloha_Triggered */

        UW_CS_ALOHA_TRIG_NODE_TIMER_STATUS timer_status; /**< Timer status */


    };

    /**
     * Class used to handle the timer to handle the listen time
     */
    class ListenTimer : public Csma_Aloha_Triggered_Timer {
    public:

        /**
         * Constructor of ListenTimer class 
         * @param UwCsmaAloha_Trigger_NODE* pointer to an object of type CsmaAloha
         */
        ListenTimer(UwCsmaAloha_Trigger_NODE* m) : Csma_Aloha_Triggered_Timer(m) {
        }

        /**
         * Destructor of the class ListenTimer
         */
        virtual ~ListenTimer() {
        }


    protected:

        /**
         * Method called when the timer expire
         * @param Eevent*  pointer to an object of type Event
         */
        virtual void expire(Event *e);


    };

    /**
     * Class used to handle the time in which the node is allowed to transmit to the sink
     */
    class TransmissionTimer : public Csma_Aloha_Triggered_Timer {
    public:

        /**
         * Constructor of the TransmissionTimer class
         * @param m pointer to an object of type UwCsmaAloha_Trigger_NODE
         */
        TransmissionTimer(UwCsmaAloha_Trigger_NODE* m) : Csma_Aloha_Triggered_Timer(m) {
        }

        /**
         * Destructor of the class TransmissionTimer
         */
        virtual ~TransmissionTimer() {
        }


    protected:

        /**
         * Method called when the timer expire
         * @param Eevent*  pointer to an object of type Event
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
     * @param Packet* Pointer to an object of type Packet that rapresent the Packet to transmit
     */
    virtual void Mac2PhyStartTx(Packet* p);
    /**
     * Method called when the PHY layer finish to transmit the packet.
     * @param Packet* Pointer to an object of type Packet that rapresent the Packet transmitted
     */
    virtual void Phy2MacEndTx(const Packet* p);
    /**
     * Method called when the Phy Layer start to receive a Packet 
     * @param const Packet* Pointer to an object of type Packet that rapresent the Packet that is in reception
     */
    virtual void Phy2MacStartRx(const Packet* p);
    /**
     * Method called when the Phy Layer finish to receive a Packet 
     * @param Packet* Pointer to an object of type Packet that rapresent the Packet received
     */
    virtual void Phy2MacEndRx(Packet* p);
    /**
     * Init the packet with the MAC address of the receiver and the sender,
     * the size of the packet and the type
     * @param UWPOLLING_PKT_TYPE the type of the packet
     */
    virtual void initPkt(Packet* p);
    /**
     * Transmits the DATA packet (calling Mac2PhyStartTx) and increment the counter of transmitted data packets
     */
    virtual void txData();

    /**
     * IDLE state. Each variable is resetted
     */
    virtual void stateIdle();
    /**
     * A reception is occuring while the protocol is in IDLE state
     */
    virtual void stateRxIdle();
    /**
     * State in which the protocol allows the node to transmit a data packet
     */
    virtual void stateTxData();
    /**
     * State in which the node is listening the channel
     */
    virtual void stateListen();
    /**
     * State in which a reception is occuring while the node is listening the channel
     */
    virtual void stateRxListen();
    /**
     * State of the protocol in which a TRIGGER packet is received
     * @param p Pointer to the Packet received
     */
    virtual void stateRxTrigger(Packet* p);

    /**
     * Checks if the Listen period is expired
     */
    virtual void stateCheckListenExpired();
    /**
     * Checks if the Time period is expired
     */
    virtual void stateCheckTxTimerExpired();
    /**
     * State in which a DATA packet is received
     * @param p pointer to the Packet received
     */
    virtual void stateRxData(Packet* p);
    /**
     * state in which a wrong Packet is received
     */
    virtual void stateRxPacketNotForMe(Packet* p);
    /**
     * Initializes the protocol at the beginning of the simulation. This method is called by
     * a command in tcl. 
     * @param double delay
     * @see command method
     */
    virtual void initInfo();

    /**
     * Refresh the State of the protocol
     */
    virtual void refreshState(UW_CS_ALOHA_TRIG_NODE_STATUS state) {
        prev_prev_state = prev_state;
        prev_state = curr_state;
        curr_state = state;
    }

    /**
     * Refresh the reason for the change of state
     */
    virtual void refreshReason(UW_CS_ALOHA_TRIG_NODE_REASON_STATUS reason) {
        last_reason = reason;
    }

    /**
     * Updates the ID of the last DATA packet received
     * @param id ID of the packet
     */
    virtual void updateLastDataIdRx(int id) {
        last_data_id_rx = id;
    }

    /**
     * Pop the first element of the data packet queue
     */
    virtual void queuePop(bool flag = true) {
        Packet::free(Q.front());
        Q.pop();
        waitEndTime(flag);
        data_sn_queue.pop();
    }
    /**
     * Used for debug purposes. (Permit to have a "step by step" behavior of the protocol)
     */
    virtual void waitForUser();

    // stats functions

    /**
     * Return the number of packets not transmitted (remained in the protocol queue) at the end 
     * of the simulation
     * @return the size of the queue at the end of the simulation
     */
    virtual int getRemainingPkts() {
        return (up_data_pkts_rx - Q.size());
    }

    /**
     * Increase the number of Data packet Received from the Upper layers
     */
    virtual void incrUpperDataRx() {
        up_data_pkts_rx++;
    }

    ListenTimer listen_timer;

    TransmissionTimer tx_timer;

    ///////////// input
    double wait_costant; /**< Adding factor in the calculation of the listen time */
    int max_payload; /**< Maximum dimension of the data payload in bytes */
    int HDR_size; /**< Size (in bytes) of the header added by the protocol */
    int buffer_pkts; /**< Length of the data buffer in number of packets */
    double listen_time; /**< Time in which the node sense the channel */
    int tx_timer_duration; /**< Duration of the time in which the node is allowed to transmit */
    /////////////////////////////

    std::queue<Packet*> Q; /**< Packet queue */
    std::queue<int> data_sn_queue; /**< Queue of the sequence number of the packets */

    static bool initialized; /**< <i>true</i> if the protocol is initialized */

    static int u_pkt_id; /**< simulation-unique packet ID */
    int u_data_id; /**< DATA packete ID */
    int last_sent_data_id; /**< ID of the last sent packet */


    Packet* curr_data_pkt; /**< Pointer to the current data packet */

    int last_data_id_rx; /**< ID of the last DATA packet received */

    bool has_buffer_queue; /**< flag that indicates if a node has a buffer where store DATA packets */
    bool can_transmit; /**< Flag that indicates if the node can transmit data packets to the sink */

    static map< UW_CS_ALOHA_TRIG_NODE_STATUS, string > status_info; /**< Textual description of the protocol states */
    static map< UW_CS_ALOHA_TRIG_NODE_REASON_STATUS, string> reason_info; /**< Textual description of the protocol reason for the change of the state */

    UW_CS_ALOHA_TRIG_NODE_STATUS curr_state; /**< Current state of the protocol */
    UW_CS_ALOHA_TRIG_NODE_STATUS prev_state; /**< Previous state of the protocol */
    UW_CS_ALOHA_TRIG_NODE_STATUS prev_prev_state; /**< Previous previous state of the protocol */
    UW_CS_ALOHA_TRIG_NODE_REASON_STATUS last_reason; /**< Reason for the state transitions */


    ofstream fout; /**< Object that handles the output file where the protocol writes the state transistions */
};

#endif
