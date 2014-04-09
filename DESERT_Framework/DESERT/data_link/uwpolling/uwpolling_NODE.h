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
 * @file   uwpolling_NODE.h
 * @author Federico Favaro
 * @version 1.0.0 
 *
 * \brief Class that represents a node of UWPOLLING 
 *
 *
 */

#ifndef Uwpolling_HDR_NODE_H
#define Uwpolling_HDR_NODE_H


#include "uwpolling_cmn_hdr.h"
#include "mmac.h"

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <fstream>


#define UWPOLLING_NODE_DROP_REASON_BUFFER_FULL          "DBF"		/**< Buffer of the node is full */
#define UWPOLLING_NODE_DROP_REASON_ERROR                "DERR"		/**< Packet corrupted */
#define UWPOLLING_NODE_DROP_REASON_UNKNOWN_TYPE         "DUT"		/**< Packet type unknown */
#define UWPOLLING_NODE_DROP_REASON_WRONG_RECEIVER       "DWR"		/**< The packet is for another node */
#define UWPOLLING_NODE_DROP_REASON_WRONG_STATE          "DWS"		/**< The node cannot receive this kind of packet in this state */
#define UWPOLLING_NODE_DROP_REASON_PACKET_NOT_FOR_ME    "DPNFM"         /**< Packet not for me */
#define UWPOLLING_NODE_DROP_REASON_IMINLIST_NOT_POLLED  "DPLNP"         /**< The node is in the list of polled node but it is not the first */
#define UWPOLLING_NODE_DROP_REASON_NOT_POLLED           "DNP"		/**< The node is not in the polling list */

/**
 * Class used to represents the UWPOLLING MAC layer of a node.
 */
class Uwpolling_NODE : public MMac {
public:
    /**
     * Constructor of the Uwpolling_NODE class
     */
    Uwpolling_NODE();
    /**
     * Destructor of the Uwpolling_NODE class
     */
    virtual ~Uwpolling_NODE();
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

    /**< Variable that rapresents the status of the protocol machine state */
    enum UWPOLLING_NODE_STATUS {
        UWPOLLING_NODE_STATUS_IDLE = 1, UWPOLLING_NODE_STATUS_RX_TRIGGER, UWPOLLING_NODE_STATUS_TX_PROBE,
        UWPOLLING_NODE_STATUS_RX_POLL, UWPOLLING_NODE_STATUS_TX_DATA, UWPOLLING_NODE_STATUS_WAIT_POLL
    };

    /**< Type of the packet */
    enum UWPOLLING_PKT_TYPE {
        UWPOLLING_DATA_PKT = 1, UWPOLLING_POLL_PKT, UWPOLLING_TRIGGER_PKT, UWPOLLING_PROBE_PKT
    };

    /**< Reason for the changing of the state */
    enum UWPOLLING_NODE_REASON {
        UWPOLLING_NODE_REASON_TX_DATA, UWPOLLING_NODE_REASON_RX_TRIGGER, UWPOLLING_NODE_REASON_PKT_ERROR,
        UWPOLLING_NODE_REASON_TX_PROBE, UWPOLLING_NODE_REASON_RX_POLL, UWPOLLING_NODE_REASON_BACKOFF_TIMER_EXPIRED,
        UWPOLLING_NODE_REASON_RX_POLL_TIMER_EXPIRED, UWPOLLING_NODE_REASON_NOT_SET,
        UWPOLLING_NODE_REASON_EMPTY_DATA_QUEUE, UWPOLLING_NODE_REASON_WRONG_TYPE, UWPOLLING_NODE_REASON_WRONG_RECEIVER,
        UWPOLLING_NODE_REASON_WRONG_STATE, UWPOLLING_NODE_REASON_NOT_IN_LIST, UWPOLLING_NODE_REASON_LIST_NOT_POLLED
    };

    /**< Status of the timer */
    enum UWPOLLING_TIMER_STATUS {
        UWPOLLING_IDLE = 1, UWPOLLING_RUNNING, UWPOLLING_FROZEN, UWPOLLING_EXPIRED
    };

    /**
     * Class that describes the timer in the AUV
     */
    class Uwpolling_NODE_Timer : public TimerHandler {
    public:

        /**
         * Constructor of the Uwpolling_NODE_Timer class
         * @param Uwpolling_AUV* a pointer to an object of type Uwpolling_AUV
         */
        Uwpolling_NODE_Timer(Uwpolling_NODE *m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(UWPOLLING_IDLE) {
            assert(m != NULL);
        }

        /**
         * Destructor of the Uwpolling_NODE_Timer class
         */
        virtual ~Uwpolling_NODE_Timer() {
        }

        /**
         * Freeze the timer
         */
        virtual void freeze() {
            assert(timer_status == UWPOLLING_RUNNING);
            left_duration -= (NOW - start_time);
            if (left_duration <= 0.0) {
                left_duration = module->mac2phy_delay_;
            }
            force_cancel();
            timer_status = UWPOLLING_FROZEN;
        }

        /**
         * unFreeze is used to resume the timer starting from the point where it was freezed
         */
        virtual void unFreeze() {
            assert(timer_status == UWPOLLING_FROZEN);
            start_time = NOW;
            assert(left_duration > 0);
            sched(left_duration);
            timer_status = UWPOLLING_RUNNING;
        }

        /**
         * stops the timer
         */
        virtual void stop() {
            timer_status = UWPOLLING_IDLE;
            force_cancel();
        }

        /**
         * Schedules a timer
         * @param double the duration of the timer
         */
        virtual void schedule(double val) {
            start_time = NOW;
            left_duration = val;
            timer_status = UWPOLLING_RUNNING;
            resched(val);
        }

        /**
         * Checks if the timer is IDLE
         * @return <i>true</i> or <i>false</i>
         */
        bool isIdle() {
            return ( timer_status == UWPOLLING_IDLE);
        }

        /**
         * Checks if the timer is RUNNING
         * @return <i>true</i> or <i>false</i>
         */
        bool isRunning() {
            return (timer_status == UWPOLLING_RUNNING);
        }

        /**
         * Checks if the timer is EXPIRED
         * @return <i>true</i> or <i>false</i>
         */
        bool isExpired() {
            return (timer_status == UWPOLLING_EXPIRED);
        }

        /**
         * Checks if the timer is FROZEN
         * @return <i>true</i> or <i>false</i>
         */
        bool isFrozen() {
            return (timer_status == UWPOLLING_FROZEN);
        }

        /**
         * Checks if the timer is ACTIVE
         * @return <i>true</i> or <i>false</i>
         */
        bool isActive() {
            return (timer_status == UWPOLLING_FROZEN || timer_status == UWPOLLING_RUNNING);
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
         * Returns the counter of the timer
         * @return the value of the counter of the timer
         */
        double getDuration() {
            return left_duration;
        }
    protected:
        double start_time; /**< Start Time of the timer */
        double left_duration; /**< Left duration of the timer */
        int counter; /**< counter of the timer */
        Uwpolling_NODE* module; /**< Pointer to an object of type Uwpolling_AUV */
        UWPOLLING_TIMER_STATUS timer_status; /**< Timer status */
    };

    /**
     * Class (inherited from Uwpolling_NODE_Timer) used to handle the time of
     * backoff of the node before transmitting the PROBE packet. After receiving a PROBE 
     * the node set this timer. When the timer expire, the node transmit the PROBE
     */
    class BackOffTimer : public Uwpolling_NODE_Timer {
    public:

        /**
         * Conscructor of ProbeTimer class 
         * @param Uwpolling_NODE* pointer to an object of type Uwpolling_NODE
         */
        BackOffTimer(Uwpolling_NODE* m) : Uwpolling_NODE_Timer(m) {
        }

        /**
         * Destructor of DataTimer class 
         */
        virtual ~BackOffTimer() {
        }

    protected:
        /**
         * Method call when the timer expire
         * @param Eevent*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    };

    /**
     * Class (inherited from Uwpolling_NODE_Timer) used to handle the time in 
     * which the NODE wait for the POLL Timer.
     */
    class Rx_Poll_Timer : public Uwpolling_NODE_Timer {
    public:

        /**
         * Conscructor of Rx_Poll_Timer class 
         * @param Uwpolling_NODE* pointer to an object of type Uwpolling_NODE
         */
        Rx_Poll_Timer(Uwpolling_NODE* m) : Uwpolling_NODE_Timer(m) {
        }

        /**
         * Destructor of ProbeTimer class 
         */
        virtual ~Rx_Poll_Timer() {
        }
    protected:
        /**
         * Method call when the timer expire
         * @param Eevent*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    };

    /**
     * Class (inherited from Uwpolling_NODE_Timer) used to handle the time 
     * between the transmission of the DATA packets when the node is polled
     * It is useful to let the AUV receive and elaborate the packet 
     * before transmit the new one. (Otherwise the new one would be dropped by 
     * the AUV
     */
    class Tx_Data_Timer : public Uwpolling_NODE_Timer {
    public:

        /**
         * Conscructor of Tx_Data_Timer class 
         * @param Uwpolling_NODE* pointer to an object of type Uwpolling_NODE
         */
        Tx_Data_Timer(Uwpolling_NODE* m) : Uwpolling_NODE_Timer(m) {
        }

        /**
         * Destructor of Tx_Data_Timer class 
         */
        virtual ~Tx_Data_Timer() {
        }
    protected:
        /**
         * Method call when the timer expire
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
    virtual void Phy2MacStartRx(const Packet* p) { }
    /**
     * Method called when the Phy Layer finish to receive a Packet 
     * @param const Packet* Pointer to an object of type Packet that rapresent the packet received
     */
    virtual void Phy2MacEndRx(Packet* p);
    /**
     * Inits the packet with the MAC address of the receiver and the sender,
     * the size of the packet and the type
     * @param UWPOLLING_PKT_TYPE the type of the packet
     */
    virtual void initPkt(UWPOLLING_PKT_TYPE pkt_type);
    /**
     * Initializes the protocol at the beginning of the simulation. This method is called by
     * a command in tcl. 
     * @param double delay
     * @see command method
     */
    virtual void initInfo();

    /**
     * Refresh the reason for the changing of the state
     * @param UWPOLLING_NODE_REASON The reason of the change of the state
     */
    virtual void refreshReason(UWPOLLING_NODE_REASON reason) {
        last_reason = reason;
    }

    /**
     * Refresh the state of the protocol
     * @param UWPOLLING_AUV_STATUS current state of the protcol
     */
    virtual void refreshState(UWPOLLING_NODE_STATUS state) {
        prev_state = curr_state;
        curr_state = state;
    }

    /**
     * Increment the number of sent PROBE packets
     */
    virtual void incrProbeSent() {
        n_probe_sent++;
    }

    /**
     * Increments the number of times that the node has been polled by the AUV
     */
    virtual void incrTimesPolled() {
        n_times_polled++;
    }

    /**
     * Increments the number of TRIGGER packets received
     */
    inline void incrTriggerReceived() {
        n_trigger_received++;
    }

    /**
     * Increments the number of TRIGGER packets dropped because of erroneous CRC
     */
    inline void incrTriggerDropped() {
        n_trigger_dropped++;
    }

    /**
     * Increments the number of POLL packets dropped because of erroneous CRC
     */
    inline void incrPollDropped() {
        n_poll_dropped++;
    }

    /**
     * Returns the number of PROBE packets sent during the simulation
     * @return int n_probe_sent the number of PROBE packets sent
     */
    inline int getProbeSent() {
        return n_probe_sent;
    }

    /**
     * Return the number of times the node are polled by the AUV
     * @return int n_times_polled number of times polled
     */
    inline int getTimesPolled() {
        return n_times_polled;
    }

    /**
     * Return the number of TRIGGER packets received by the NODE
     * @return int n_trigger_received number of TRIGGER packets received
     */
    inline int getTriggerReceived() {
        return n_trigger_received;
    }

    /**
     * Return the number of TRIGGER dropped by the node because of erroneous CRC
     * @return int n_trigger_dropped number of TRIGGER dropped by the AUV
     */
    inline int getTriggerDropped() {
        return n_trigger_dropped;
    }

    /**
     * Return the number of POLL dropped by the node because of erroneous CRC
     * @return itn n_poll_dropped number of POLL dropped
     */
    inline int getPollDropped() {
        return n_poll_dropped;
    }
    /**
     * Used for debug purposes. (Permit to have a "step by step" behaviour of the protocol)
     */
    virtual void waitForUser();
    /**
     * IDLE state. Each variable is resetted
     */
    virtual void stateIdle();
    /**
     * State of the protocol in which a DATA packet is sent. Here we compute some statistics
     * related to the transmission of the packet (i.e. the number of data packets transmitted 
     * is incremented and we check the number of data packet to transmit
     */
    virtual void stateTxData();
    /**
     * State of the protocol in which a TRIGGER packet is received. Here we check the header of
     * the TRIGGER and we store the information into it (i.e. the range of values into which choose
     * randomly the backoff time
     */
    virtual void stateRxTrigger();
    /**
     * State of the protocol in which a PROBE packet is sent. Here we store into the PROBE header
     * all the information needed, such as the timestamp of the most recent data packet, the number
     * of the packets that the node wish to transmit
     */
    virtual void stateTxProbe();
    /**
     * State of the protocol in which a POLL packet is received. Here the node check if he's polled
     * or, at least, he is in the list of polled node. If he's the polled node, he goes into the 
     * stateTxData state. Otherwise he drops the packet
     */
    virtual void stateRxPoll();
    /**
     * Method called by the Expire event of the timer. Here we call the stateTxProbe method
     */
    virtual void BackOffTimerExpired();
    /**
     * Methods called by the Expire event of the timer. Here we disable the possibility of the node
     * to receive POLL packets. 
     */
    virtual void RxPollTimerExpired();
    /**
     * The backoff timer is calculated choosing randomly using a Uniform random variable and multiplying 
     * it by a backoff_tuner chosen by the user via tcl
     */
    virtual double getBackOffTime();
    /**
     * The DATA Packet is sended down to the PHY layer
     */
    virtual void TxData();
    /**
     * The PROBE Packet is sended down to the PHY layer
     */
    virtual void TxPRobe();
    /**
     * State in which the protocol set up the timer to wait the reception of the POLL packet
     */
    virtual void stateWaitPoll();

    /**
     * Calculate the epoch of the event. Used in sea-trial mode
     * @return the epoch of the system
     */
    inline unsigned long int getEpoch() {
        return time(NULL);
    }



    /*************************
     * input values from TCL *
     *************************/
    double T_poll; /**< Duration of RxPOLLTimer */
    double backoff_tuner; /**< Multiplying value to the backoff value */
    int max_payload; /**< Payload of Application Layer in bytes */
    int buffer_data_pkts; /**< Length of buffer of DATA pkts in number of pkts */
    int max_data_pkt_tx; /**< Max number of DATA packets to transmit each cycle */
    uint node_id; /**< Unique Node ID */
    int sea_trial; /**< Sea Trial flag: To activate if the protocol is going to be tested at the sea */
    int print_stats; /**< Print protocol's statistics of the protocol */
    int n_run; /*< ID of the experiments (used during sea trial) */
    int Intra_data_Guard_Time; /**< Guard Time between one data packet and the following */


    std::queue<Packet*> Q_data; /**< Queue of DATA in number of packets */




    static bool initialized; /**< <i>true</i> if the protocol is initialized, <i>false</i> otherwise */

    bool polled; /**< <i>true</i> if the node is polled, <i>false</i> otherwise */
    bool RxPollEnabled; /**< <i>true</i> if the node is enabled to receive the POLL, <i>false</i> otherwise */
    bool Triggered; /**< <i>true</i> if the node has correctly received a TRIGGER, <i>false</i> otherwise */
    bool LastPacket; /**< <i>true</i> if the node has just sent the last packet of the queue and has to exit from the TxData state, <i>false</i> otherwise */



    double MaxTimeStamp; /**< Timestamp of the most recent data packet generated */
    double T_in; /**< Lower bound of the range in which choose randomly the backoff time (sent by the AUV in the TRIGGER message) */
    double T_fin; /**< Upper bound of the range in which choose randomly the backoff time (sent by the AUV in the TRIGGER message) */
    double BOffTime; /**< Backoff time chosen */
    int AUV_mac_addr; /**< MAC address of the AUV */
    int N_data_pkt_2_TX; /**< Number of DATA packets to transmit to the AUV */
    int packet_index; /**< Index of the packet just sent to the AUV */
    int n_probe_sent; /**< Number of PROBE packets sent to the AUV */
    int n_times_polled; /**< Number of times that the node has been polled by the AUV */
    int n_trigger_received; /**< Number of TRIGGER packets received */
    uint N_polled_node; /**< Number of node polled in this POLL message */
    uint PROBE_uid; /**< PROBE Unique ID */

    Packet* curr_data_pkt; /**< Pointer to the current DATA packet */
    Packet* curr_probe_pkt; /**< Pointer to the current PROBE packet */
    Packet* curr_poll_pkt; /**< Pointer ot the current POLL packet */
    Packet* curr_trigger_pkt; /**< Pointer to the current TRIGGER packet */


    UWPOLLING_NODE_REASON last_reason; /**< Last reason to the change of state */
    UWPOLLING_NODE_STATUS curr_state; /**< Current state of the protocol */
    UWPOLLING_NODE_STATUS prev_state; /**< Previous state of the protocol */

    static map< Uwpolling_NODE::UWPOLLING_NODE_STATUS, string > status_info; /**< Textual info of the state */
    static map< Uwpolling_NODE::UWPOLLING_NODE_REASON, string> reason_info; /**< Textual info of the reason */
    static map< Uwpolling_NODE::UWPOLLING_PKT_TYPE, string> pkt_type_info; /**< Textual info of the type of the packet */

    BackOffTimer backoff_timer; /**< Backoff timer */
    Rx_Poll_Timer rx_poll_timer; /**< Receiving POLL Timer */
    Tx_Data_Timer tx_data_timer; /**< Timer between two consequent DATA packet transmission */

    std::ofstream fout; /**< Output stream for the textual file of debug */
    std::ofstream out_file_stats;

    int n_trigger_dropped; /**< Number of TRIGGER packet dropped */
    int n_poll_dropped; /**< Number of POLL packet dropped */

};
#endif



