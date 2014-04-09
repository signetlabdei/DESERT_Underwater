//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwUFetch_NODE.h
 * @author Loris Brolo
 * @version 1.0.0
 * 
 * @brief Declaration of uwUFetch_NODE class
 */

#ifndef UWUFETCH_NODE_H_
#define UWUFETCH_NODE_H_

#include <mmac.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <clmessage.h>
#include <mphy.h>
#include <string>
#include <fstream>
#include <ostream>
#include <cassert>
#include <queue>
#include <map>
//#include "uwmphy_modem_cmn_hdr.h"

#define UWFETCH_NODE_DROP_REASON_UNKNOWN_TYPE "DUT"                                     /**<Packet dropped: unknown type */
#define UWFETCH_NODE_DROP_REASON_WRONG_RECEIVER "DWR"                                 /**<Packet dropped: Packet is addressed to another node */ 
#define UWFETCH_NODE_DROP_REASON_CBEACON_AND_DATA_ALREADY_TX "DCDAT"   /**<Packet dropped: reject CBEACON because DATA packets are already transmitted in the previous cycle*/
#define UWFETCH_NODE_DROP_REASON_ERROR "DRE"                                                     /**<Packet dropped: packet is corrupted */
#define UWUFETCH_NODE_DROP_REASON_BUFFER_FULL "DBF"                                       /**< Packet dropped: buffer of the node is completely full */
#define UWUFETCH_NODE_DROP_REASON_NOT_ENABLE "DNE"                                        /**< Packet dropped: node is not enabled to receive this type of packet*/
#define UWUFETCH_NODE_DROP_REASON_NOT_MY_TURN "DNMT"                                  /**< Packet dropped: poll is not addressed to me */
#define UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK "CNRP"                                   /**< Packet dropped: node can not receive this type of node */

extern packet_t PT_TRIGGER_UFETCH; /**< TRIGGER packet type for UFetch protocol */
extern packet_t PT_RTS_UFETCH; /**< RTS packet type for UFetch protocol */
extern packet_t PT_CTS_UFETCH; /**< CTS packet type for UFetch protocol */
extern packet_t PT_BEACON_UFETCH; /**< BEACON packet type for UFetch protocol */
extern packet_t PT_PROBE_UFETCH; /**< PROBE packet type for UFetch protocol */
extern packet_t PT_POLL_UFETCH; /**< POLL packet type for UFetch protocol */
extern packet_t PT_CBEACON_UFETCH; /**< CBEACON packet type for UFetch protocol */

/**
 * Internal structure where the HN store the informations about the NODE from 
 * which it received a PROBE
 */
typedef struct probbed_node {
    uint id_node_; // Id of the NODE
    int n_pcks_NODE_want_tx_; // Number of packets that the NODE want to transmit at the HN
    int mac_address_; // Mac Address of the NODE
    double backoff_time_; // Back-off time chosen by the NODE before transmitting the PROBE

    /**
     * Reference to the id_node variable
     */
    inline uint & Id_node() {
        return (id_node_);
    }

    /**
     * Reference to the n_pcks_NODE_want_tx_ variable
     */
    inline int& n_pcks_NODE_want_tx() {
        return n_pcks_NODE_want_tx_;
    }

    /**
     * Reference to the back-off_time_ variable
     */
    inline double& backoff_time() {
        return backoff_time_;
    }

    /**
     * Reference to the mac_address variable
     */
    inline int& mac_address() {
        return mac_address_;
    }
} probbed_node;

/**< uwuFetch_NODE class */

class uwUFetch_NODE : public MMac {
public:

    /**
     * Constructor of uwUFetch_NODE class 
     */
    uwUFetch_NODE();

    /**
     * Destructor of uwUFetch_NODE class
     */
    virtual ~uwUFetch_NODE();

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

    /**< States in which SN or HN may be during Its execution */
    enum UWUFETCH_NODE_STATUS {
        UWUFETCH_NODE_STATUS_IDLE,
        UWUFETCH_NODE_STATUS_BEACON_RECEIVE,
        UWUFETCH_NODE_STATUS_TRANSMIT_PROBE,
        UWUFETCH_NODE_STATUS_WAIT_POLL_PACKET,
        UWUFETCH_NODE_STATUS_POLL_RECEIVE,
        UWUFETCH_NODE_STATUS_TRANSMIT_DATA,
        UWUFETCH_NODE_STATUS_TRIGGER_RECEIVE,
        UWUFETCH_NODE_STATUS_RTS_TRANSMIT,
        UWUFETCH_NODE_STATUS_WAIT_CTS_PACKET,
        UWUFETCH_NODE_STATUS_CTS_RECEIVE,
        UWUFETCH_NODE_STATUS_TRANSMIT_BEACON,
        UWUFETCH_NODE_STATUS_WAIT_PROBE_PACKET,
        UWUFETCH_NODE_STATUS_PROBE_RX,
        UWUFETCH_NODE_STATUS_POLL_TX,
        UWUFETCH_NODE_STATUS_WAIT_DATA_NODE,
        UWUFETCH_NODE_STATUS_DATA_RX,
        UWUFETCH_NODE_STATUS_WAIT_CBEACON_PACKET,
        UWUFETCH_NODE_STATUS_CBEACON_RECEIVE
    };

    /**< Reasons for which the SN or HN moves from one state to another one */
    enum UWUFETCH_NODE_STATUS_CHANGE {
        UWFETCH_NODE_STATUS_CHANGE_STATE_IDLE,
        UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR,
        UWFETCH_NODE_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE,
        UWFETCH_NODE_STATUS_CHANGE_PACKET_UNKNOWN_TYPE,
        UWUFETCH_NODE_STATUS_CHANGE_BEACON_RX,
        UWUFETCH_NODE_STATUS_CHANGE_PROBE_TX,
        UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_POLL_EXPIRED,
        UWUFETCH_NODE_STATUS_CHANGE_POLL_RX,
        UWFETCH_NODE_STATUS_CHANGE_CBEACON_RX_DATA_ALREADY_TX,
        UWUFETCH_NODE_STATUS_CHANGE_MAX_DATA_PCK_TX,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_PCK_TX,
        UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX,
        UWUFETCH_NODE_STATUS_CHANGE_RTS_TX,
        UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_CTS_EXPIRED,
        UWUFETCH_NODE_STATUS_CHANGE_CTS_RX,
        UWUFETCH_NODE_STATUS_CHANGE_BEACON_TX,
        UWUFETCH_NODE_STATUS_CHANGE_CBEACON_TX,
        UWUFETCH_NODE_STATUS_CHANGE_PROBE_RX,
        UWUFETCH_NODE_STATUS_CHANGE_CBEACON_MAX_ALLOWED_TX_0_PROBE_RX,
        UWUFETCH_NODE_STATUS_CHANGE_CBEACON_ALLOWED_TX_0_PROBE_RX,
        UWUFETCH_NODE_STATUS_CHANGE_MAX_PROBE_RX_PROBE_TO_NOT_EXPIRED,
        UWUFETCH_NODE_STATUS_CHANGE_TO_EXPIRED_AT_LEAST_1_PROBE_RX,
        UWUFETCH_NODE_STATUS_CHANGE_POLL_TX,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED_0_NODE_TO_POLL_NO_OTHER_CBEACON,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED_0_NODE_TO_POLL_YES_OTHER_CBEACON,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED_ANOTHER_NODE_TO_POLL,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_RX,
        UWUFETCH_NODE_STATUS_CHANGE_LAST_DATA_PCK_RX,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_NO_OTHER_CBEACON,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_YES_OTHER_CBEACON,
        UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_YES_NODE_TO_POLL,
        UWUFETCH_NODE_STATUS_CHANGE_CBEACON_MAX_ALLOWED_TX,
        UWUFETCH_NODE_STATUS_CHANGE_BEACON_TO_EXPIRED,
        UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX_BUT_HN_NO_DATA_TO_TX,
        UWUFETCH_NODE_STATUS_CHANGE_HN_TX_ALL_DATA_TO_AUV,
        UWUFETCH_NODE_STATUS_CHANGE_HN_RX_TRIGGER_FROM_AUV_CORRUPTED,
        UWUFETCH_NODE_STATUS_CHANGE_HN_FINISH_TX_DATA_PCK_TO_AUV_RETURN_CBEACON_TX
    };

    /**< Type of packets transmitted and received by SN or HN */
    enum UWUFETCH_NODE_PACKET_TYPE {
        UWUFETCH_NODE_PACKET_TYPE_DATA, UWUFETCH_NODE_PACKET_TYPE_PROBE,
        UWUFETCH_NODE_PACKET_TYPE_POLL, UWUFETCH_NODE_PACKET_TYPE_BEACON,
        UWUFETCH_NODE_PACKET_TYPE_TRIGGER, UWUFETCH_NODE_PACKET_TYPE_RTS,
        UWUFETCH_NODE_PACKET_TYPE_CTS
    };

    /**< Status timer in which SN or HN can be found */
    enum UWUFETCH_TIMER_STATUS {
        UWUFETCH_TIMER_STATUS_IDLE, UWUFETCH_TIMER_STATUS_RUNNING,
        UWUFETCH_TIMER_STATUS_FROZEN, UWUFETCH_TIMER_STATUS_EXPIRED
    };

    /**
     * Class that handle the timers of SN or HN
     */
    class uwUFetch_NODE_timer : public TimerHandler {
    public:

        /**
         * Constructor of the class uwUFetch_NODE_timer
         * @param   m pointer to uwUFetch_NODE class
         */
        uwUFetch_NODE_timer(uwUFetch_NODE *m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(UWUFETCH_TIMER_STATUS_IDLE) {
            assert(m != NULL);
        }

        /**
         *  Destructor of the class uwUFetch_NODE_timer
         */
        virtual ~uwUFetch_NODE_timer() {
        };

        /**
         *  Freeze the timer
         */
        virtual void freeze() {
            assert(timer_status == UWUFETCH_TIMER_STATUS_RUNNING);
            left_duration = NOW - start_time;
            if (left_duration <= 0.0) {
                left_duration = module->mac2phy_delay_;
            }
            force_cancel();
            timer_status = UWUFETCH_TIMER_STATUS_FROZEN;
        }

        /**
         *  unFreeze are used after a freezing of the timer. In this case 
         *  the timer restart from the timer where was freezed 
         */
        virtual void unFreeze() {
            assert(timer_status == UWUFETCH_TIMER_STATUS_FROZEN);
            start_time = NOW;
            assert(left_duration > 0);
            sched(left_duration); /**check the state of the timer before 
                                         * sheduling the event. If the timer is 
                                         * set, then the call are aborted
                                         */
            timer_status = UWUFETCH_TIMER_STATUS_RUNNING;
        }

        /**
         *  Stop the timer
         */
        virtual void stop() {
            timer_status = UWUFETCH_TIMER_STATUS_IDLE;
            force_cancel();
        }

        /**
         *  schedule a timer
         * 
         * @param double the duration of the timer
         */
        virtual void schedule(double val) {
            start_time = NOW;
            left_duration = val;
            timer_status = UWUFETCH_TIMER_STATUS_RUNNING;
            resched(val);
        }

        /**
         *  verify whether the timer is IDLE
         * 
         *  @return: bool <i>true</i>-->IDLE
         */
        bool isIdle() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_IDLE);
        }

        /**
         *  verify if the timer is RUNNING
         *
         *  @return: bool <i>true</i> RUNNING
         */
        bool isRunning() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_RUNNING);
        }

        /**
         *  verify if the timer is FROZEN
         * 
         *  @return: bool <i>true</i> FROZEN          
         */
        bool isFroozen() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_FROZEN);
        }

        /**
         *  verify whether the timer is EXPIRED
         * 
         *  @return: bool <i>true</i> EXPIRED         
         */
        bool isExpired() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_EXPIRED);
        }

        /**
         *  verify if the timer is ACTIVE
         * 
         *  @return: bool <i>true</i> ACTIVE          
         */
        bool isActive() {
            return ( timer_status == isRunning() || timer_status == isFroozen());
        }

        /**
         *  reset the counter that scan the timer   
         */
        void resetCounter() {
            counter = 0;
        }

        /**
         *  increment the counter by one value
         */
        void incrCounter() {
            ++counter;
        }

        /**
         * value of the counter 
         * 
         * @return int counter 
         */
        int getCounter() {
            return counter;
        }

        /**
         *  left duration of the timer
         * 
         * @return double left_duration 
         */
        double getDuration() {
            return left_duration;
        }

    protected:
        double start_time; /**< Time to start the timer */
        double left_duration; /**< Left duration of the timer     */
        int counter; /**< Counter of the timer */
        uwUFetch_NODE* module; /**< Pointer to an object of type uwUFetch_NODE */
        UWUFETCH_TIMER_STATUS timer_status; /**< Timer status */

    }; //END class uwUFetch_NODE_timer 

    /**
     * Timer associated to the HN. 
     * When the HN is in Idle state, before to start the procedure to transmit the BEACON packet, It wait a TRIGGER 
     * packet from the AUV for an interval time. 
     * In the case of reception of TRIGGGER packet, the communication between HN and AUV start, otherwise HN
     * begin the communication with SN
     */
    class uwUFetch_BEACON_timer : public uwUFetch_NODE_timer {
    public:

        /**
         *  Constructor of uwUFetch_BEACON_timer class
         * 
         * @param m a pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_BEACON_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of uwUFetch_BEACON_timer
         */
        virtual ~uwUFetch_BEACON_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);

    }; //END class uwUFetch_BEACON_timer 

    /**
     * Class (inherited from uwUFetch_NODE_Timer) used to handle the time of back-off of the node before 
     * transmitting the PROBE packet. After the reception of a PROBE packet the node set this timer. 
     * When the timer expire, the node transmit the PROBE.
     */
    class uwUFetch_BackOffTimer : public uwUFetch_NODE_timer {
    public:

        /**
         * Constructor of uwUFetch_BackOffTimer class 
         * 
         * @param uwUFetch_NODE* pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_BackOffTimer(uwUFetch_NODE* m) : uwUFetch_NODE_timer(m) {
        }

        /**
         * Destructor of uwUFetch_BackOffTimer class 
         */
        virtual ~uwUFetch_BackOffTimer() {
        }

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Eevent*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //end class uwUFetch_BackOffTimer

    /** 
     * Class inherited the method and variable of uwUFetch_NODE_timer that handle the timer of PROBE packets.
     * HN set up a time interval within It's enabled to receive PROBE packets from SN.
     */
    class uwUFetch_PROBE_timer : public uwUFetch_NODE_timer {
    public:

        /**
         *  Constructor of uwUFetch_PROBE_timer class
         * 
         * @param m a pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_PROBE_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of uwUFetch_PROBE_timer
         */
        virtual ~uwUFetch_PROBE_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_PROBE_timer    

    /** 
     * Class inherited the method and variable of uwUFetch_NODE_timer that handle the timer of POLL packets.
     * SN set up a time interval within It's enabled to receive a POLL packet from a specific HN. 
     * If in this time interval the HN node doesn't transmit the POLL packet the SN return in IDLE state or It start 
     * to wait the successive CBEACON packet from that HN.
     */
    class uwUFetch_POLL_timer : public uwUFetch_NODE_timer {
    public:

        /**
         *  Constructor of uwUFetch_POLL_timer class
         * 
         * @param m a pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_POLL_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of uwUFetch_POLL_timer
         */
        virtual ~uwUFetch_POLL_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_POLL_timer 

    /**
     * Class inherited the method and variable of uwUFetch_NODE_timer that handle the timer of DATA packets.
     * Sn between to successive transmission of DATA packets wait an interval time. Only at the end of these interval 
     * time SN transmit the DATA packet.
     */
    class uwUFetch_DATA_BEFORE_TX_timer : public uwUFetch_NODE_timer {
    public:

        /**
         *  Constructor of uwUFetch_DATA_BEFORE_TX_timer class
         * 
         * @param m a pointer to an object of type UWUFetch_NODE
         */
        uwUFetch_DATA_BEFORE_TX_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of uwUFetch_DATA_BEFORE_TX_timer class
         */
        virtual ~uwUFetch_DATA_BEFORE_TX_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_DATA_BEFORE_TX_timer 

    /** 
     * Class inherited the method and variable of uwUFetch_NODE_timer that handle the timer of DATA packets. 
     * HN set up a time interval within It want to receive all DATA packets from  a specific SN. After the end of this 
     * time interal HN start a new transmission of POLL or CBEACON or wait a TRIGGER packet from AUV. 
     */
    class uwUFetch_DATA_timer : public uwUFetch_NODE_timer {
    public:

        /**
         *  Constructor of uwUFetch_DATA_timer class
         * 
         * @param m a pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_DATA_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of uwUFetch_DATA_timer class
         */
        virtual ~uwUFetch_DATA_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_DATA_timer 

    /**
     *  Class inherited the method and variable of uwUFetch_NODE_timer that handle the timer of CBEACON packets.
     *  SN set a time interval within it want to receive a CBEACON packet from a specific HN.
     *  If this timer expired the SN return in Idle state and wait a new BEACON packet from any HN
     */
    class uwUFetch_CBeacon_timer : public uwUFetch_NODE_timer {
    public:

        /**
         *  Constructor of uwUFetch_CBeacon_timer class
         * 
         * @param m a pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_CBeacon_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of uwUFetch_Beacon_timer class
         */
        virtual ~uwUFetch_CBeacon_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);

    }; //END class uwUFetch_CBeacon_timer

    /** 
     * Class inherited the method and variable of uwUFetch_NODE_timer that handle the timer of CTS packets. 
     * HN set an interval time within It want to receive a CTS packet from AUV.
     * When this interval time expire, HN return in Idle state and interrupt the communication with AUV.  
     */
    class uwUFetch_CTS_timer : public uwUFetch_NODE_timer {
    public:

        /**
         * Constructor of uwUFetch_CTS_timer class
         * 
         * @param m a pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_CTS_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of UWUFetch_CTS_timer class
         */
        virtual ~uwUFetch_CTS_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_CTS_timer 

    /** 
     * Class inherited the method and variable of uwUFetch_NODE_timer that handle the timer of TRIGGER packets. 
     * HN set an interval time within It wait a TRIGGER packet from AUV.
     * When this interval time expire, if HN has not received any TRIGGER It start the communication with SN, 
     * otherwhise It start the communication with AUV.  
     */
    class uwUFetch_TRIGGER_timer : public uwUFetch_NODE_timer {
    public:

        /**
         * Constructor of uwUFetch_TRIGGER_time class
         * 
         * @param m a pointer to an object of type uwUFetch_NODE
         */
        uwUFetch_TRIGGER_timer(uwUFetch_NODE *m) : uwUFetch_NODE_timer(m) {
        }

        /*
         * Destructor of uwUFetch_TRIGGER_time class
         */
        virtual ~uwUFetch_TRIGGER_timer() {
        };

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_TRIGGER_timer

    /******************************************************************************
     *                           GENERAL METHODS                                  *  
     ******************************************************************************/

    /**
     *  Handle the detected-start-of-PHY-reception event for SENSOR NODE
     * 
     * @param p pointer to the packet whose reception has begun.
     */
    virtual void Phy2MacStartRx(const Packet *p);

    /**
     * Handle the detected-start-of-PHY-reception event for HEAD NODE
     * 
     * @param p pointer to the packet whose reception has begun.
     */
    virtual void Phy2MacStartRx_HN(const Packet *p);

    /**
     *  Handle the end-of-PHY-reception event
     *   
     * @param p pointer to the packet whose reception has ended.
     */
    virtual void Phy2MacEndRx(Packet *p);

    /**
     *  Handle the end-of-PHY-reception event for HEAD NODE in the case that the communication don't use RTS-CTS packets
     *   
     * @param p pointer to the packet whose reception has ended.
     */
    virtual void Phy2MacEndRx_HN_without(Packet *p);

    /**
     * Handle the end-of-PHY-reception event for HEAD NODE
     *   
     * @param p pointer to the packet whose reception has ended.
     */
    virtual void Phy2MacEndRx_HN(Packet *p);

    /**
     *  This method must be called by the SN-MAC to instruct the PHY to start the transmission of a packet
     * 
     * @param p pointer to the packet to be transmitted
     */
    virtual void Mac2PhyStartTx(Packet *p);

    /**
     * This method must be called by the HN-MAC to instruct the PHY to start the transmission of a packet.
     * 
     * @param p pointer to the packet to be transmitted
     */
    virtual void Mac2PhyStartTx_HN(Packet *p);

    /**
     * Handle the end-of-PHY-transmission event
     * 
     * @param p pointer to the packet whose transmission has ended. Note that the Packet is not any more under 
     *  control of the SN-MAC at the time this event occurs, hence the 'const' declaration.
     */
    virtual void Phy2MacEndTx(const Packet *p);

    /**
     * Handle the end-of-PHY-transmission event for HEAD NODE
     * 
     * @param p pointer to the packet whose transmission has ended. Note that the Packet is not any more under
     *  control of the HN-MAC at the time this event occurs, hence the 'const' declaration.
     */
    virtual void Phy2MacEndTx_HN(const Packet *p);

    /**
     * Handle a packet coming from upper layers of SN
     * 
     * @param p packet that node receiving from upper layer. DATA packet created by application layer
     */
    virtual void recvFromUpperLayers(Packet* p);

    /**
     * Handle a packet coming from upper layers of HN
     * 
     * @param p packet that node receiving from upper layer. DATA packet created by application layer
     */
    virtual void recvFromUpperLayers_HN(Packet* p);

    /**
     * Prints a file with every state change for debug purposes. 
     * 
     * @param double delay
     */
    virtual void printStateInfo(double delay = 0);

    /**
     * Initialize the protocol at the beginning of the simulation. This method is called by a command in TCL.
     */
    virtual void initInfo();

    /**
     * Method used for debug
     */
    virtual void waitForUser();

    /**
     *  Refresh the reason for the changing of the state
     * 
     * @param UWPOLLING_AUV_REASON The reason of the change of the state of the protocol
     */
    virtual void refreshReason(UWUFETCH_NODE_STATUS_CHANGE reason) {
        last_reason = reason;
    }

    /*
     *  Refresh the state of the protocol
     * 
     * @param UWUFETCH_NODE_STATUS current state of the protocol
     */
    virtual void refreshState(UWUFETCH_NODE_STATUS state) {
        prev_state = curr_state;
        curr_state = state;
    }

    /******************************************************************************    
     *                        SENSOR NODE METHODS                                 *    
     ******************************************************************************/

    /**
     * Idle state for SN. Each variables are resetted  
     */
    virtual void stateIdle_NODE();

    /**
     * SN received a BEACON packet. Extract  all information contained into It.
     */
    virtual void BEACON_rx();

    /**
     * Handle the 
     */
    virtual void BCKTOExpired();

    /**
     * SN initialize the PROBE packet that will be forwarded to a specific HN.
     */
    virtual void state_PROBE_tx();

    /**
     * Transmission of PROBE packet by SN to the specific HN.
     */
    virtual void PROBE_tx();

    /**
     * SN wait a POLL packet from a specific HN. For an interval time Sn is enabled to receive POLL packets.
     */
    virtual void state_wait_POLL();

    /**
     * POLL timeout is expired. SN return in Idle state and wait a BEACON packet, or start to wait  a CBEACON packet 
     * from HN. 
     */
    virtual void PollTOExpired();

    /**
     * SN has received a POLL packet from a specific HN. Extract  all information contained into It.
     */
    virtual void POLL_rx();

    /**
     * SN initialize the FIRST DATA packet that will be forwarded to a specific HN.
     */
    virtual void state_DATA_NODE_first_tx();

    /**
     * Data timeout. After this timeout is expired the SN pass the DATA packet to its phy level that will then transmit
     * to a specific HN 
     */
    virtual void DataBeforeTxTOExpired();

    /**
     * SN initialize the second or successive DATA packet that will be forwarded to a specific HN.
     */
    virtual void state_DATA_NODE_tx();

    /**
     * SN has finished to transmit the last DATA packet promises with a PROBE packet to a specif HN.
     */
    virtual void state_DATA_NODE_finish_tx();

    /**
     * SN transmit a DATA packet to a specific HN
     */
    virtual void DATA_NODE_tx();

    /**
     * SN wait a CBEACON packet for a pre-established interval time. During this interval time SN is enabled to received 
     * a CBEACON packet from HN 
     */
    virtual void state_wait_CBEACON();

    /**
     * CBEACON timeout is expired. In this case SN return in Idle state and wait another BEACON packet from an 
     * another or the same HN packet
     */
    virtual void CBeaconTOExpired();

    /**
     * SN has received a CBEACON packet from HN. Extract  all information contained into It.
     */
    virtual void CBEACON_rx();


    /****************************************************************************
     *                          HEAD NODE METHODS                               *
     ****************************************************************************/

    /**
     * Idle state for HN. Each variables are resetted  
     */
    virtual void stateIdle_HN();

    /**
     * BEACON timeout is expired. In this case the HN has not received any TRIGGER packet, so start the communication
     * with the SNs of the network. 
     */
    virtual void BeaconTxTOExpire();

    /**
     * HN initialize the BEACON packet that will be forwarded in broadcast at all SNs of the network.
     */
    virtual void state_BEACON_tx();

    /**
     * HN transmit the BEACON packet
     */
    virtual void BEACON_tx();

    /**
     * HN start to wait the FIRST PROBE packet from some SNs. During this interval time the HN is enabled to 
     * received more than one PROBE packets.
     */
    virtual void state_wait_PROBE();

    /**
     * HN has received a PROBE packet from SN. Extract  all information contained into It.
     */
    virtual void PROBE_rx();

    /**
     * HN start to wait the second or successive PROBE packet from some SNs. During this interval time the HN is
     *  enabled to received more than one PROBE packets.
     */
    virtual void state_wait_other_PROBE();

    /**
     * Probe timeout is expired. The HN start to transmit a POLL packet in unicast to a specific SN from which It has 
     * received a POLL packet. The order of the transmission of POLL packet is FIFO type.
     */
    virtual void ProbeTOExpired();

    /**
     * HN initialize the POLL packet that will be forwarded to a specific SN of the network
     */
    virtual void state_POLL_tx();

    /**
     * HN transmit POLL packet to a specific SN of th network
     */
    virtual void POLL_tx();

    /**
     * HN start to wait the FIRST DATA packet from SN. During this interval time the HN is enabled to received DATA 
     * packets from a specifical SN.
     */
    virtual void state_wait_first_DATA();

    /**
     * HN start to wait the second or successive DATA packet from SN. During this interval time the HN is enabled 
     * to received DATA packets from a specifical SN.
     */
    virtual void state_wait_other_DATA();

    /**
     * DATA timeout is expired. HN is not enabled to receive other DATA packets and return in IDLE state or start the 
     * initialization and start transmission of new POLL packet.
     */
    virtual void DataTOExpired();

    /**
     * HN has received a BEACON packet from SN. Extract  all information contained into It.
     */
    virtual void DATA_rx();

    /**
     * HN initialize the CBEACON packet that will be forwarded in broadcast at all SNs of the network.
     */
    virtual void state_CBEACON_tx();

    /**
     * HN transmit the BEACON packet
     */
    virtual void CBEACON_tx();

    /**
     * HN has received a TRIGGER packet from AUV. Extract  all information contained into It. In this case the communication
     * with take place with RTS and CTS packets 
     */
    virtual void TRIGGER_rx();

    /**
     * HN has received a TRIGGER packet from AUV. Extract  all information contained into It. In this case the communication
     * with AUV take place without RTS and CTS packets
     */
    virtual void TRIGGER_rx_without();

    /**
     * HN initialize the RTS packet that will be forwarded to the AUV.
     */
    virtual void state_RTS_tx();

    /**
     * HN transmit a RTS packet to the AUV
     */
    virtual void RTS_tx();

    /**
     * HN start to wait a CTS packet for a pre-established time interval. During this interval time Hn is enabled to 
     * receive a CTS packet from AUV
     */
    virtual void state_wait_CTS();

    /**
     * CTS timeout is expired. HN in this case return in idle state and interrupt the communication with AUV
     */
    virtual void CtsTOExpired();

    /**
     * HN has received a CTS packet from AUV. Extract  all information contained into It.
     */
    virtual void CTS_rx();

    /**
     * HN initialize the FIRST DATA packet that will be forwarded to the AUV node. In this case the communication 
     * between HN and AUV take place with RTS and CTS packets
     */
    virtual void state_DATA_HN_first_tx();

    /**
     * HN initialize the FIRST DATA packet that will be forwarded to the AUV node. In this case the communication 
     * between HN and AUV take place without RTS and CTS packets 
     */
    virtual void state_DATA_HN_first_tx_without();

    /**
     * HN initialize the second and successive DATA packets that will be forwarded to the AUV node. In this case the
     *  communication between HN and AUV take place with RTS and CTS packets 
     */
    virtual void state_DATA_HN_tx();

    /**
     * HN initialize the second and successive DATA packets that will be forwarded to the AUV node. In this case the
     * communication between HN and AUV take place without RTS and CTS packets 
     */
    virtual void state_DATA_HN_tx_without();

    /**
     * HN has finished to transmit the sequence of DATA packets promises to the AUV with the exchange of previous 
     * RTS and CTS packets
     */
    virtual void state_DATA_HN_finish_tx();

    /**
     * HN has finished to transmit the sequence of DATA packets to the AUV. In this case the communication 
     * between HN and AUV take place without RTS and CTS packets
     */
    virtual void state_DATA_HN_finish_tx_without();

    /**
     * HN transmit a DATA packet to the AUV. In this case the communication between HN and AUV take place 
     * with RTS and CTS packets 
     */
    virtual void DATA_HN_tx();

    /**
     * HN transmit a DATA packet to the AUV. In this case the communication between HN and AUV take place 
     * without RTS and CTS packets 
     */
    virtual void DATA_HN_tx_without();

    /**
     * Backoff timeout is expired. After the expiration HN can pass the successive DATA packet that will be transmitted
     * to Its physical layer and then forwarded to the AUV. In this case the communication between HN and AUV take 
     * place with RTS and CTS packets 
     */
    virtual void BCKTOExpired_HN();

    /**
     * Backoff timeout is expired. After the expiration HN can pass the successive DATA packet that will be transmitted
     * to Its physical layer and then forwarded to the AUV. In this case the communication between HN and AUV take
     *  place without RTS and CTS packets 
     */
    virtual void BCKTOExpired_HN_without();

    /**
     * Backoff timeout is expired. After the expiration HN can pass the successive DATA packet that will be transmitted
     * to Its physical layer and then forwarded to the AUV. In this case the communication between HN and AUV take 
     * place without RTS and CTS packets 
     */
    virtual void DataBeforeTxTOExpired_HN();

    /******************************************************************************
     *                          METODI AUSILIARI                                  *
     ******************************************************************************/
    /**
     * Indicate if the node created work as head node or sensor node
     * 
     * @return <i> true</i> The node work as HN
     */
    virtual bool isHeadNode();

    /**
     * Indicate if the communiation between HN and AUV take place with or without RTS and CTS packets
     * 
     * @return <i> true</i> Communication between HN-AUV take place without RTS and CTS 
     */
    virtual bool typeCommunication();

    /**
     * Indicate whether the transmission of DATA packets to the AUV take place with or without burst data 
     * 
     * @return <i> true </i> Transmission take place with burst data 
     */
    virtual bool burstDATA();

    /**
     * Choose the backoff timeout used by SN before to transmit a PROBE packet
     * 
     * @return double backoff timeout choosen by SN
     */
    virtual double choiceBackOffTimer();

    /**
     * Choose the backoff timeout used by HN before to transmit a RTS packet
     * 
     * @return double backoff timeout choosen by HN
     */
    virtual double choiceBackOffTimer_HN();

    /**
     * Compute the interval time within HN want to receive all DATA packets from a specific SN 
     * 
     * @return double Timeout within the HN want to receive all DATA packets from a specific SN
     */
    virtual double getDataTimerValue();

    /**
     * Compute the transmission time for the packets transmitted by the SNs
     * 
     * @param tp type of packet which should be transmitted
     */
    virtual void computeTxTime(UWUFETCH_NODE_PACKET_TYPE tp);

    /**
     * Compute the transmission time for the packets transmitted by the HNs
     * 
     * @param tp type of packet which should be transmitted
     */
    virtual void computeTxTime_HN(UWUFETCH_NODE_PACKET_TYPE tp);

    /**
     * Compute the round trip time
     * 
     * @return double round trip time 
     */
    virtual double getRTT();

    /**
     * Update the list of the SN from which HN has received the PROBE packets. This update is executed after the 
     * transmission of POLL packet to the SN
     *  
     */
    virtual void updateListProbbedNode();

    /******************************************************************************
     *                                          METODI get E incr                                     *
     * ****************************************************************************/

    /**************************
     *          HEAD NODE     *
     **************************/

    /**
     * Increase the number of BEACON packet transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrBeaconPckTx_by_HN() {
        n_BEACON_pck_tx_by_HN++;
    };

    /**
     *  Number of BEACON packet transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_BEACON_pck_tx_by_HN
     */
    int getBeaconTx_by_HN() {
        return n_BEACON_pck_tx_by_HN;
    };

    /**
     * Increase the total number of BEACON packets transmitted by the HN during an entire simulation
     */
    void incrTotalBeaconPckTx_by_HN() {
        n_tot_BEACON_pck_tx_by_HN++;
    };

    /**
     *  Total number of BEACON packets transmitted by the HN during an entire simulation
     * 
     * @return int n_tot_BEACON_pck_tx_by_HN
     */
    int getTotalBeaconTx_by_HN() {
        return n_tot_BEACON_pck_tx_by_HN;
    };

    /**
     * Increase number of PROBE packets received by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrProbePckRx_HN() {
        n_PROBE_pck_rx_by_HN++;
    };

    /**
     *  Number of PROBE packets received by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_PROBE_pck_rx_by_HN
     */
    int getProbePckRx_HN() {
        return n_PROBE_pck_rx_by_HN;
    };

    /**
     * Increase total number of PROBE packets correctly or not received by the HN during an entire simulation
     */
    void incrTotalProbePckRx_HN() {
        n_tot_PROBE_pck_rx_by_HN++;
    };

    /**
     *  Total number of PROBE packets correctly or not received by the HN during an entire simulation
     * 
     * @return int n_tot_PROBE_pck_rx_by_HN
     */
    int getTotalProbePckRx_HN() {
        return n_tot_PROBE_pck_rx_by_HN;
    };

    /**
     * Increase the total number of corrupted PROBE packets received by the HN during an entire simulation
     */
    void incrTotalProbePckRx_corrupted_HN() {
        n_tot_PROBE_pck_corr_rx_by_HN++;
    };

    /**
     * Total number of corrupted PROBE packets received by the HN during an entire simulation
     * 
     * @return int n_tot_PROBE_pck_corr_rx_by_HN
     */
    int getTotalProbePckRx_corrupted_HN() {
        return n_tot_PROBE_pck_corr_rx_by_HN;
    };

    /**
     * Increase the number of POLL packets transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrPollPckTx_by_HN() {
        n_POLL_pck_tx_by_HN++;
    };

    /**
     *  Number of POLL packet transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_POLL_pck_tx_by_HN
     */
    int getPollPckTx_by_HN() {
        return n_POLL_pck_tx_by_HN;
    };

    /**
     *  Increase the total number of POLL packets transmitted by the HN during an entire simulation
     */
    void incrTotalPollPckTx_by_HN() {
        n_tot_POLL_pck_tx_by_HN++;
    };

    /**
     *  Total number of POLL packets transmitted by the HN during an entire simulation
     * 
     * @return int n_tot_POLL_pck_tx_by_HN
     */
    int getTotalPollPckTx_by_HN() {
        return n_tot_POLL_pck_tx_by_HN;
    };

    /**
     * Increase the number of DATA packets transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrDataPckRx_by_HN() {
        n_DATA_pck_rx_by_HN++;
    };

    /**
     *  Number of DATA packets transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_DATA_pck_rx_by_HN
     */
    int getDataPckRx_by_HN() {
        return n_DATA_pck_rx_by_HN;
    };

    /**
     * Increase total number of DATA packets correctly or not received by the HN during an entire simulation
     */
    void incrTotalDataPckRx_by_HN() {
        n_tot_DATA_pck_rx_by_HN++;
    };

    /**
     * Total number of DATA packets correctly or not received by the HN during an entire simulation
     * 
     * @return int n_tot_DATA_pck_rx_by_HN
     */
    int getTotalDataPckRx_by_HN() {
        return n_tot_DATA_pck_rx_by_HN;
    };

    /**
     * Increase total number of corrupted DATA packets received by the HN during an entire simulation
     */
    void incrTotalDataPckRx_corrupted_by_HN() {
        n_tot_DATA_pck_rx_corr_by_HN++;
    };

    /**
     *  Total number of corrupted DATA packets received by the HN during an entire simulation
     * 
     * @return int n_tot_DATA_pck_rx_corr_by_HN
     */
    int getTotalDataPckRx_corrupted_by_HN() {
        return n_tot_DATA_pck_rx_corr_by_HN;
    };

    /**
     * Increase the number of CBEACON packets transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrCBeaconPckTx_by_HN() {
        n_CBEACON_pck_tx_by_HN++;
    };

    /**
     *  Number of CBEACON packets transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_CBEACON_pck_tx_by_HN
     */
    int getCBeaconPckTx_by_HN() {
        return n_CBEACON_pck_tx_by_HN;
    };

    /**
     * Increase the total number of CBEACON packets transmitted by the HN during an entire simulation
     */
    void incrTotalCBeaconPckTx_by_HN() {
        n_tot_CBEACON_pck_tx_by_HN++;
    };

    /**
     *  Total number of CBEACON packets transmitted by the HN during an entire simulation 
     * 
     * @return int n_tot_CBEACON_pck_tx_by_HN
     */
    int getTotalCBeaconPckTx_by_HN() {
        return n_tot_CBEACON_pck_tx_by_HN;
    };

    /**
     * Increase the number of TRIGGER packets received by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     */
    void incrTriggerPckRx_HN() {
        n_TRIGGER_pck_rx_by_HN++;
    };

    /**
     * Number of TRIGGER packets received by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     * 
     * @return int n_TRIGGER_pck_rx_by_HN
     */
    int getTriggerPckRx_HN() {
        return n_TRIGGER_pck_rx_by_HN;
    };

    /**
     * Increase the total number of TRIGGER packets correctly or not received by the HN during an entire simulation
     */
    void incrTotalTriggerPckRx_HN() {
        n_tot_TRIGGER_pck_rx_by_HN++;
    };

    /**
     *  Total number of TRIGGER packets correctly or not received by the HN during an entire simulation
     * 
     * @return int n_tot_TRIGGER_pck_rx_by_HN
     */
    int getTotalTriggerPckRx_HN() {
        return n_tot_TRIGGER_pck_rx_by_HN;
    };

    /**
     * Increase the total number of corrupted TRIGGER packets received by the HN during an entire simulation
     */
    void incrTotalTriggerPckRx_corrupted_HN() {
        n_tot_TRIGGER_pck_rx_corr_by_HN++;
    };

    /**
     *  Total number of corrupted TRIGGER packets received by the HN during an entire simulation
     * 
     * @return int n_tot_TRIGGER_pck_rx_corr_by_HN
     */
    int getTotalTriggerPckRx_corrupted_HN() {
        return n_tot_TRIGGER_pck_rx_corr_by_HN;
    };

    /**
     * Increase the number of RTS packets transmitted by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     */
    void incrRtsPckTx_by_HN() {
        n_RTS_pck_tx_by_HN++;
    };

    /**
     *  Number of RTS packets transmitted by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     * 
     * @return int n_RTS_pck_tx_by_HN
     */
    int getRtsPckTx_by_HN() {
        return n_RTS_pck_tx_by_HN;
    };

    /**
     * Increase the total number of RTS packets transmitted by the HN during an entire simulation
     */
    void incrTotalRtsPckTx_by_HN() {
        n_tot_RTS_pck_tx_by_HN++;
    };

    /**
     *  Total number of RTS packets transmitted by the HN during an entire simulation
     * 
     * @return int n_tot_RTS_pck_tx_by_HN
     */
    int getTotalRtsPckTx_by_HN() {
        return n_tot_RTS_pck_tx_by_HN;
    };

    /**
     * Increase the number of CTS packets received by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     */
    void incrCtsPckRx_HN() {
        n_CTS_pck_rx_by_HN++;
    };

    /**
     *  Number of CTS packets received by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     * 
     * @return int n_CTS_pck_rx_by_HN
     */
    int getCtsPckRx_HN() {
        return n_CTS_pck_rx_by_HN;
    };

    /**
     * Increase the total number of CTS packets correctly or not received by the HN during an entire simulation
     */
    void incrTotalCtsPckRx_HN() {
        n_tot_CTS_pck_rx_by_HN++;
    };

    /**
     *  Total number of CTS packets correctly or not received by the HN during an entire simulation
     * 
     * @return int n_tot_CTS_pck_rx_by_HN
     */
    int getTotalCtsPckRx_HN() {
        return n_tot_CTS_pck_rx_by_HN;
    };

    /**
     * Increase the total number of corrupted CTS packets received by the HN during an entire simulation
     */
    void incrTotalCtsPckRx_corrupted_HN() {
        n_tot_CTS_pck_rx_corr_by_HN++;
    };

    /**
     *  Total number of corrupted CTS packets received by the HN during an entire simulation
     * 
     * @return int n_tot_CTS_pck_rx_corr_by_HN
     */
    int getTotalCtsPckRx_corrupted_HN() {
        return n_tot_CTS_pck_rx_corr_by_HN;
    };

    /**
     * Increase the number of DATA packets transmitted by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     */
    void incrDataPckTx_by_HN() {
        n_DATA_pck_tx_by_HN++;
    };

    /**
     *  Number of DATA packets transmitted by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation
     * 
     * @return int n_DATA_pck_tx_by_HN
     */
    int getDataPckTx_by_HN() {
        return n_DATA_pck_tx_by_HN;
    };

    /**
     * Increase the total number of DATA packets transmitted by the HN during an entire simulation
     */
    void incrTotalDataPckTx_by_HN() {
        n_tot_DATA_pck_tx_by_HN++;
    }

    /**
     *  Total number of DATA packets transmitted by the HN during an entire simulation
     * 
     * @return int n_tot_DATA_pck_tx_by_HN
     */
    int getTotalDataPckTx_by_HN() {
        return n_tot_DATA_pck_tx_by_HN;
    }

    /*********************************
     *          SENSOR NODE            *  
     *********************************/

    /**
     * Increase the number of BEACON packets received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrBeaconPckRx_by_NODE() {
        n_BEACON_pck_rx_by_NODE++;
    };

    /**
     *  Number of BEACON packets received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_BEACON_pck_rx_by_NODE
     */
    int getBeaconPckRx_by_NODE() {
        return n_BEACON_pck_rx_by_NODE;
    };

    /**
     * Increase the total number of BEACON packets received correctly or not by SN during an entire duration of simulation
     */
    void incrTotalBeaconPckRx_by_NODE() {
        n_tot_BEACON_pck_rx_by_NODE++;
    };

    /**
     *  Total number of BEACON packets received correctly or not by SN during an entire duration of simulation
     * 
     * @return int n_tot_BEACON_pck_rx_by_NODE
     */
    int getTotalBeaconPckRx_by_NODE() {
        return n_tot_BEACON_pck_rx_by_NODE;
    };

    /**
     * Increase the total number of corrupted BEACON packets received by SN during an entire duration of simulation
     */
    void incrTotalBeaconPckRx_corrupted_by_NODE() {
        n_tot_BEACON_pck_rx_corr_by_NODE++;
    };

    /**
     *  Total number of corrupted BEACON packets received by SN during an entire duration of simulation
     * 
     * @return int n_tot_BEACON_pck_rx_corr_by_NODE
     */
    int getTotalBeaconPckRx_corrupted_by_NODE() {
        return n_tot_BEACON_pck_rx_corr_by_NODE;
    };

    /**
     * Increase the number of PROBE packets transmitted by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrProbePckTx_by_NODE() {
        n_PROBE_pck_tx_by_NODE++;
    };

    /**
     *  Number of PROBE packets transmitted by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_PROBE_pck_tx_by_NODE
     */
    int getProbePckTx_by_NODE() {
        return n_PROBE_pck_tx_by_NODE;
    };

    /**
     * Increase the total number of PROBE packets transmitted by SN during an entire duration of simulation
     */
    void incrTotalProbePckTx_by_NODE() {
        n_tot_PROBE_pck_tx_by_NODE++;
    };

    /**
     *  Total number of PROBE packets transmitted by SN during an entire duration of simulation
     * 
     * @return int n_tot_PROBE_pck_tx_by_NODE
     */
    int getTotalProbePckTx_by_NODE() {
        return n_tot_PROBE_pck_tx_by_NODE;
    };

    /**
     * Increase the number of POLL packets received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrPollPckRx_by_NODE() {
        n_POLL_pck_rx_by_NODE++;
    };

    /**
     *  Number of POLL packets received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_POLL_pck_rx_by_NODE
     */
    int getPollPckRx_by_NODE() {
        return n_POLL_pck_rx_by_NODE;
    };

    /**
     * Increase the total number of POLL packets received correctly or not by SN during an entire duration of simulation
     */
    void incrTotalPollPckRx_by_NODE() {
        n_tot_POLL_pck_rx_by_NODE++;
    };

    /**
     *  Total number of POLL packets received correctly or not by SN during an entire duration of simulation
     * 
     * @return int n_tot_POLL_pck_rx_by_NODE
     */
    int getTotalPollPckRx_by_NODE() {
        return n_tot_POLL_pck_rx_by_NODE;
    };

    /**
     * Increase the total number of corrupted POLL packets received by SN during an entire duration of simulation
     */
    void incrTotalPollPckRx_corrupted_by_NODE() {
        n_tot_POLL_pck_rx_corr_by_NODE++;
    };

    /**
     *  Total number of corrupted POLL packets received by SN during an entire duration of simulation
     * 
     * @return int n_tot_POLL_pck_rx_corr_by_NODE
     */
    int getTotalPollPckRx_corrupted_by_NODE() {
        return n_tot_POLL_pck_rx_corr_by_NODE;
    };

    /**
     * Increase the number of DATA packets transmitted by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrDataPckTx_by_NODE() {
        n_DATA_pck_tx_by_NODE++;
    };

    /**
     *  Number of DATA packets transmitted by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_DATA_pck_tx_by_NODE
     */
    int getDataPckTx_by_NODE() {
        return n_DATA_pck_tx_by_NODE;
    };

    /**
     * Increase the total number of DATA packets transmitted by SN during an entire duration of simulation
     */
    void incrTotalDataPckTx_by_NODE() {
        n_tot_DATA_pck_tx_by_NODE++;
    };

    /**
     *  Total number of DATA packets transmitted by SN during an entire duration of simulation
     * 
     * @return int n_tot_DATA_pck_tx_by_NODE
     */
    int getTotalDataPckTx_by_NODE() {
        return n_tot_DATA_pck_tx_by_NODE;
    };

    /**
     * Increase the number of CBEACON packets received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     */
    void incrCBeaconPckRx_by_NODE() {
        n_CBEACON_pck_rx_by_NODE++;
    };

    /**
     *  Number of CBEACON packets received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation
     * 
     * @return int n_CBEACON_pck_rx_by_NODE
     */
    int getCBeaconPckRx_by_NODE() {
        return n_CBEACON_pck_rx_by_NODE;
    };

    /**
     * Increase the total number of CBEACON packets received correctly or not by SN during an entire duration of simulation
     */
    void incrTotalCBeaconPckRx_by_NODE() {
        n_tot_CBEACON_pck_rx_by_NODE++;
    };

    /**
     *  Total number of CBEACON packets received correctly or not by SN during an entire duration of simulation
     * 
     * @return int n_tot_CBEACON_pck_rx_by_NODE
     */
    int getTotalCBeaconPckRx_by_NODE() {
        return n_tot_CBEACON_pck_rx_by_NODE;
    };

    /**
     * Increase the total number of corrupted CBEACON packets received by SN during an entire duration of simulation
     */
    void incrTotalCBeaconPckRx_corrupted_by_NODE() {
        n_tot_CBEACON_pck_rx_corr_by_NODE++;
    };

    /**
     *  Total number of corrupted CBEACON packets received by SN during an entire duration of simulation
     * 
     * @return int n_tot_CBEACON_pck_rx_corr_by_NODE
     */
    int getTotalCBeaconPckRx_corrupted_by_NODE() {
        return n_tot_CBEACON_pck_rx_corr_by_NODE;
    };

    /****************************************************************************
     *                   SENSOR NODE VARIABLES                                  *
     ****************************************************************************/
    // TCL variables
    int MAX_PAYLOAD; /**< Maximum size of payload DATA packet */
    double T_GUARD; /**< Guard interval used for successive transmission of DATA packets */
    int MAXIMUM_BUFFER_DATA_PCK_NODE; /**< Maximum number of DATA packets that the SN can store in Its queue */
    double T_POLL; /**< Interval time in which the SN wait a POLL packet from the HN */
    int debugMio_; /**< Used if we want to create the logging file */
    int N_RUN; /**< Indicate the number of the run in execution */
    double TIME_BETWEEN_2_TX_DATA_NODE_HN; /**< Interval time used by the SN before to transmit the next DATA packet to the HN */
    int MODE_COMM_HN_AUV; /**< Indicate the type of communication between HN and AUV, 0 = communication with RTS-CTS, 1 = communication without RTS-CTS */
    int MODE_BURST_DATA; /**< Indicate if it's used or not the burst data. 0=not use burst date, 1=use burst data. */

    //Timers
    uwUFetch_BackOffTimer BCK_timer_probe; /**< Interval time in which HN is enabled to receive PROBE packets from SNs */
    uwUFetch_POLL_timer POLL_timer; /**< Interval time in which SN is enabled to receive POLL packet from the specific HN */
    uwUFetch_DATA_BEFORE_TX_timer DATA_BEFORE_TX_timer; //Interval time after that the SN transmit the successive DATA pck to the HN */
    uwUFetch_CBeacon_timer CBEACON_timer; //Interval time in which SN is enabled to receive CBEACON packet from HN */

    // Global Variables that should be never reset
    double rx_BEACON_start_time; /**< Indicates when SN started the reception of BEACON packet from HN */
    double rx_BEACON_finish_time; /**< Indicates when SN finished the reception of BEACON packet from HN*/
    double tx_PROBE_start_time; /**< Indicates when SN start a transmission of PROBE packet */
    double tx_PROBE_finish_time; /**< Indicates when SN finished a transmission of PROBE packet */
    double rx_POLL_start_time; /**< Indicates when SN started the reception of POLL packet from HN */
    double rx_POLL_finish_time; /**< Indicates when SN finished the reception of POLL packet from HN */
    double tx_DATA_start_time; /**< Indicates when SN start a transmission of DATA packet to the HN */
    double tx_DATA_finish_time; /**< Indicates when SN finished a transmission of DATA packet to the HN */
    double rx_CBEACON_start_time; /**< Indicates when SN started the reception of CBEACON packet from HN */
    double rx_CBEACON_finish_time; /**< Indicates when SN finished the reception of BEACON packet from HN */
    int n_tot_BEACON_pck_rx_by_NODE; /**< Total number of BEACON packets received correctly or not by SN during an entire duration of simulation */
    int n_tot_BEACON_pck_rx_corr_by_NODE; /**< Total number of BEACON packets received corrupted by SN during an entire duration of simulation */
    int n_tot_PROBE_pck_tx_by_NODE; /**< Total number of PROBE packets transmitted by SN during an entire duration of simulation */
    int n_tot_POLL_pck_rx_by_NODE; /**< Total number of POLL packets received correctly or not by SN during an entire duration of simulation */
    int n_tot_POLL_pck_rx_corr_by_NODE; /**< Total number of POLL packets received corrupted by SN during an entire duration of simulation */
    int n_tot_DATA_pck_tx_by_NODE; /**< Total number of DATA packets transmitted by the SN during an entire duration of simulation */
    int n_tot_CBEACON_pck_rx_by_NODE; /**< Total number of CBEACON packets received correctly or not by SN during an entire duration of simulation */
    int n_tot_CBEACON_pck_rx_corr_by_NODE; /**< Total number of CBEACON packets received corrupted by the SN during an entire duration of simulation */
    double T_min_bck_probe_node; /**< Lower bound timer interval used by the SN to choice the interval time before to transmit a PROBE packet to the HN */
    double T_max_bck_probe_node; /**< Upper bound timer interval used by the SN to choice the interval time before to transmit a PROBE packet to the HN */
    int num_cbeacon_node_rx; /**< Number of CBEACON that the node have received from the specific HN before to receive an another BEACON packet */
    int mac_addr_HN_in_beacon; /**< HN MAC address from which the SN has received PROBE packet */
    double bck_before_tx_probe; /**< Backoff time choiced by SN before to transmit a PROBE packet to the HN */
    double Tprobe; /**< Time duration needed for SN to transmit a PROBE packet to the HN. It include the time delay propagation. */
    double Tdata_NODE; /**< Time duration needed for SN to transmit a DATA packet to the HN. It include the time delay propagation. */
    double RTT; /**< Round trip time value */
    int mac_addr_HN_in_poll; /**< HN MAC address from which the SN has received the POLL packet */
    int num_pck_to_tx_by_NODE; /**< Number of DATA packets that the SN at the instant time of BEACON reception has available to transmit to the HN */
    double Tdata_NODE_pck; /**< Time duration need for SN for transmit a DATA packet to the HN */
    int num_cbeacon_at_now_HN_tx; /**< NUmber of CBEACON packets transmitted by the HN during the single cycle BEACON-PROBE-POLL-DATA-CBEACON */
    int mac_addr_HN_in_cbeacon; /**< HN MAC address from which the SN has received the CBEACON packet */
    double T_min_bck_DATA; /**< Lower bound time interval from which the HN choice its backoff timer before to transmit a DATA packet to the AUV */
    double T_max_bck_DATA; /**< Upper bound time interval from which the HN choice its backoff timer before to transmit a DATA packet to the AUV */
    double bck_before_tx_data; /**< Time value chosen by the HN and used by It for transmit a DATA packet to the AUV */
    // Variables that should be reseted when a cycle is end
    int n_BEACON_pck_rx_by_NODE; /**< Number of BEACON receive by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_PROBE_pck_tx_by_NODE; /**< Number of PROBE transmitted by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_POLL_pck_rx_by_NODE; /**< Number of POLL received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_DATA_pck_tx_by_NODE; /**< Number of DATA transmitted by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_CBEACON_pck_rx_by_NODE; /**< Number of CBEACON received by the SN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    bool dataAlreadyTransmitted; /**< Indicate whether the SN has transmit at least one DATA packets during the cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    // Variables that enable or not an operation
    bool rxBEACONEnabled; /**< <i> true </i> if SN is enabled to receive a BEACON packet from the HN */
    bool txPROBEEnabled; /**< <i> true </i> if SN is enabled to transmit a PROBE packet to the HN */
    bool rxPOLLEnabled; /**< <i> true </i> if SN is enabled to receive a POLL packet from the HN */
    bool txDATAEnabled; /**< <i> true </i> if SN is enabled to transmit a DATA packet to the HN */
    bool rxCBEACONEnabled; /**< <i> true </i> if SN is enabled to receive a CBEACON packet from the HN */

    // PACKETS create
    Packet* curr_BEACON_NODE_pck_rx; /**< Pointer to the BEACON packet that is being received by SN */
    Packet* curr_PROBE_NODE_pck_tx; /**< Pointer to the PROBE packet that is being transmitted by SN */
    Packet* curr_POLL_NODE_pck_rx; /**< Pointer to the POLL packet that is being received by SN */
    Packet* curr_DATA_NODE_pck_tx; /**< Pointer to the DATA packet that is being transmitted by SN */
    Packet* curr_CBEACON_NODE_pck_rx; /**< Pointer to the CBEACON packet that is being received by SN  */

    //Queue
    std::queue<Packet*> Q_data; /**< Queue of DATA packets stored by the SN */

    /******************************************************************************
     *                          HEAD NODE VARIABLES                               *
     ******************************************************************************/
    //TCL variables
    double T_START_PROCEDURE_HN_NODE; /**< Time within HN is enabled to received a TRIGGER packet from AUV. If  in this time the AUV never receive a TRIGGER packet start the communication with the SN */
    double T_MAX_BACKOFF_PROBE; /**< Upper bound timer interval of back-off value used by the SN to choice its back-off time before to transmit a PROBE packet */
    double T_MIN_BACKOFF_PROBE; /**< Lower bound timer interval of back-off value used by the SN to choice its back-off time before to transmit a PROBE packet*/
    int PRINT_TRANSITIONS_INT; /**< <i> 0 </i> reason because the SN or HN is passed from a state to another state is not logged in a file*/
    double T_PROBE; /**< Interval time in which HN is enabled to received PROBE packets from SNs after the transmission of TRIGGER packet */
    int MAX_ALLOWED_CBEACON_TX; //Maximum number of CBEACON cycles that HN can start after the transmission of a BEACON packet. */
    int MAX_PCK_HN_WANT_RX_FROM_NODE; //Maximum number of DATA packets that HN want to receive from a single SN during a cycle BEACON-PROBE-POLL-DATA-CBEACON */
    double T_CTS; //Interval time in which HN is enabled to receive a CTS packet from AUV */
    int MAX_POLLED_NODE; /**< Maximum number of PROBE packets that the HN can receive from the SN after the transmission of a BEACON or CBEACON */
    double TIME_BETWEEN_2_TX_DATA_HN_AUV; /**< Interval time used by  HN before to transmit the next DATA packet to the AUV */

    //Timers
    uwUFetch_BEACON_timer BeaconBeforeTx_timer; /**< Schedule the BEACON timeout timer */
    uwUFetch_PROBE_timer PROBE_timer; /**< Schedule the PROBE timeout timer */
    uwUFetch_DATA_timer DATA_timer; /**< Schedule the DATA timeout timer */
    uwUFetch_BackOffTimer BCK_timer_rts; /**< Schedule the backoff timeout timer */
    uwUFetch_CTS_timer CTS_timer; /**< Schedule the CTS timeout timer */
    uwUFetch_BackOffTimer BCK_timer_data; /**< Interval time before that the HN transmit his DATA packet to the AUV */

    // Global Variables that should be never reset
    double tx_BEACON_start_HN_time; /**< Indicates when HN started the transmission of BEACON packet to the SNs */
    double tx_BEACON_finish_HN_time; /**< Indicates when HN finished the transmission of BEACON packet to the SNs */
    double rx_PROBE_start_HN_time; /**< Indicates when HN started the reception of PROBE packet from the SNs */
    double rx_PROBE_finish_HN_time; /**< Indicates when HN finished the reception of PROBE packet from the SNs */
    double tx_POLL_start_HN_time; /**< Indicates when HN started the transmission of POLL packet to a specific SN */
    double tx_POLL_finish_HN_time; /**< Indicates when HN finished the transmission of POLL packet to a specific SN */
    double rx_DATA_start_HN_time; /**< Indicates when HN started the reception of DATA packet from the SNs */
    double rx_DATA_finish_HN_time; /**< Indicates when HN finished the reception of DATA packet from the SNs */
    double tx_CBEACON_start_HN_time; /**< Indicates when HN started the transmission of CBEACON packet to the SNs */
    double tx_CBEACON_finish_HN_time; /**< Indicates when HN finished the transmission of CBEACON packet to the SNs */
    double rx_TRIGGER_start_HN_time; /**< Indicates when HN started the reception of TRIGGER packet from the AUV */
    double rx_TRIGGER_finish_HN_time; /**< Indicates when HN finished the reception of TRIGGER packet from the AUV */
    double tx_RTS_start_HN_time; /**< Indicates when HN started the transmission of RTS packet to the AUV */
    double tx_RTS_finish_HN_time; /**< Indicates when HN finished the transmission of RTS packet to the AUV */
    double rx_CTS_start_HN_time; /**< Indicates when HN started the reception of CTS packet from the AUV */
    double rx_CTS_finish_HN_time; /**< Indicates when HN finished the reception of CTS packet from the SNs */
    double tx_DATA_start_HN_time; /**< Indicates when HN started the transmission of DATA packet to the AUV */
    double tx_DATA_finish_HN_time; /**< Indicates when HN finished the transmission of DATA packet to the AUV */
    int n_tot_BEACON_pck_tx_by_HN; /**< Total number of BEACON packets transmitted by the HN during an entire simulation */
    int n_tot_PROBE_pck_rx_by_HN; /**< Total number of PROBE packets correctly or not received by the HN during an entire simulation */
    int n_tot_PROBE_pck_corr_rx_by_HN; /**< Total number of corrupted PROBE packets received by the HN during an entire simulation */
    int n_tot_POLL_pck_tx_by_HN; /**< Total number of POLL packets transmitted by the HN during an entire simulation */
    int n_tot_DATA_pck_rx_by_HN; /**< Total number of DATA packets correctly or not received by the HN during an entire simulation */
    int n_tot_DATA_pck_rx_corr_by_HN; /**< Total number of corrupted DATA packets received by the HN during an entire simulation */
    int n_tot_CBEACON_pck_tx_by_HN; /**< Total number of CBEACON packets transmitted by the HN during an entire simulation */
    int n_tot_TRIGGER_pck_rx_by_HN; /**< Total number of TRIGGER packets correctly or not received by the HN during an entire simulation */
    int n_tot_TRIGGER_pck_rx_corr_by_HN; /**< Total number of corrupted TRIGGER packets received by the HN during an entire simulation */
    int n_tot_RTS_pck_tx_by_HN; /**< Total number of RTS packets transmitted by the HN during an entire simulation */
    int n_tot_CTS_pck_rx_by_HN; /**< Total number of CTS packets correctly or not received by the HN during an entire simulation */
    int n_tot_CTS_pck_rx_corr_by_HN; /**< Total number of corrupted CTS packets received by the HN during an entire simulation */
    int n_tot_DATA_pck_tx_by_HN; /**< Total number of DATA packets transmitted by the HN during an entire simulation */
    int mac_addr_NODE_polled; /**< SN-MAC address of the node that the HN is going to be poll and from which It want to receive DATA packets */
    int number_data_pck_HN_rx_exact; /**< Exact number of DATA packets that the HN want to receive from the SN that have polled */
    int pck_number_id; /**< Unique identifier of the DATA packet received*/
    double data_timeout; /**< Interval time in which the HN want to receive all DATA packets from the node that have polled */
    double Tbeacon; /**< Time needed to transmit a BEACON packet. It consider also a propagation delay. */
    double Tpoll; /**< Time needed to transmit a POLL packet. It consider also a propagation delay. */
    double Trts; /**< Time needed to transmit a RTS packet. It consider also a propagation delay. */
    double Tdata_HN; /**< Time needed to transmit a DATA packet. It consider also a propagation delay. */
    int mac_addr_NODE_in_data; /**< Mac address of the node from which the HN has received a DATA packet */
    double bck_before_tx_RTS; /**< Time value choice by the HN and used for transmit a RTS packet to the AUV */
    double T_min_bck_RTS; /**< Lower bound interval time from which the HN choice its backoff timer before to transmit a RTS packet to the AUV */
    double T_max_bck_RTS; /**< Upper bound interval time from which the HN choice his backoff timer before to transmit a RTS packet to the AUV */
    int mac_addr_AUV_in_trigger; /**< AUV-MAC address contained in the TRIGGER packet received by the HN*/
    int mac_addr_AUV_in_CTS; /**< AUV-MAC address contained int the CTS packet received by HN */
    int max_data_HN_can_tx; /**< Maximum number of DATA packets that the HN must transmit to the AUV */
    int max_pck_HN_can_tx; /**< Maximum number of DATA packets that AUV want to receive from HN */
    double Tdata_HN_pck; /**< Time to transmit a DATA packet by the HN */
    bool CTSrx; /**< <i> true</i> HN has received a CTS packet after the transmission of RTS */
    bool txRTSbeforeCBEACON; /**< <i> true</i> HN block the transmission of CBEACON because It try to communicate with the AUV. */
    bool sectionCBeacon; /**< <i> true</i> HN has started the transmission of RTS packet when It was in the CBEACON section. */

    // Variables that should be reseted when a cycle is end
    int n_BEACON_pck_tx_by_HN; /**< Number of BEACON packet transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_PROBE_pck_rx_by_HN; /**< Number of PROBE packet received by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_POLL_pck_tx_by_HN; /**< Number of POLL packet transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_DATA_pck_rx_by_HN; /**< Number of DATA packet received by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_CBEACON_pck_tx_by_HN; /**< Number of CBEACON packet transmitted by the HN during a single cycle BEACON-PROBE-POLL-DATA-CBEACON of the simulation */
    int n_TRIGGER_pck_rx_by_HN; /**< Number of TRIGGER packet received by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation */
    int n_RTS_pck_tx_by_HN; /**< Number of RTS packet transmitted by the HN during a  single cycle TRIGGER-RTS-CTS-DATA of the simulation */
    int n_CTS_pck_rx_by_HN; /**< Number of CTS packet received by the HN during a single cycle TRIGGER-RTS-CTS-DATA of the simulation */
    int n_DATA_pck_tx_by_HN; /**< Number of DATA packet transmitted by the HN during a single cycle TRIGGER-RTS-CTS-DATA  of the simulation */

    // Variables that enable or not an operation
    bool txBEACONEnabled; /**< <i> true </i> if HN is enabled to transmit a BEACON packet to the SN */
    bool rxPROBEEnabled; /**< <i> true </i> if HN is enabled to receive a PROBE packet from the SN */
    bool txPOLLEnabled; /**< <i> true </i> if HN is enabled to transmit a POLL packet to the SN */
    bool rxDATAEnabled; /**< <i> true </i> if HN is enabled to receive a DATA packet from teh SN */
    bool txCBEACONEnabled; /**< <i> true </i> if HN is enabled to transmit a CBEACON packet to the SN */
    bool rxTRIGGEREnabled; /**< <i> true </i> if HN is enabled to receive a TRIGGER packet from the AUV */
    bool txRTSEnabled; /**< <i> true </i> if HN is enabled to transmit a RTS packet to the AUV */
    bool rxCTSEnabled; /**< <i> true </i> if HN is enabled to receive a CTS packet from the AUV */
    bool txDATAEnabledHN; /**< <i> true </i> if HN is enabled to transmit a DATA packet to the AUV */

    // PACKETS create
    Packet* curr_BEACON_HN_pck_tx; /**< Pointer to the BEACON packet that is being transmitted by HN */
    Packet* curr_PROBE_HN_pck_rx; /**< Pointer to the PROBE packet that is being received by HN */
    Packet* curr_POLL_HN_pck_tx; /**< Pointer to the POLL packet that is being transmitted by HN */
    Packet* curr_DATA_HN_pck_rx; /**< Pointer to the DATA packet that is being received by HN from SN */
    Packet* curr_CBEACON_HN_pck_tx; /**< Pointer to the CBEACON packet that is being transmitted by HN */
    Packet* curr_TRIGGER_HN_pck_rx; /**< Pointer to the TRIGGER packet that is being received by HN */
    Packet* curr_RTS_HN_pck_tx; /**< Pointer to the RTS packet that is being transmitted by HN */
    Packet* curr_CTS_HN_pck_rx; /**< Pointer to the CTS packet that is being received by HN */
    Packet* curr_DATA_HN_pck_tx; /**< Pointer to the DATA packet that is being transmitted by HN to the AUV*/
    Packet* curr_DATA_NODE_pck_tx_HN; /**< Pointer to the DATA packet that is being transmitted by HN */

    //QUEUE
    std::queue<Packet*> Q_data_HN; /**< Queue of DATA packets stored by the HNs and received from SN */
    std::queue<int>Q_data_source_SN; /**< Queue that contain the MAC address from which the HN has received the DATA packet */
    std::queue<int> Q_probbed_mac_HN; /**< Queue that store the MAC address of the SN from which the HN has received correctly the PROBE packets */
    std::queue<int> Q_probbed_n_pcks_NODE_want_tx_HN; /**< Queue that store the number of DATA packets that the single SN want to tx to the HN. */
    std::queue<double> Q_probbed_backoff_time; /**< Queue that stored the backoff time choice by the single nodes before to transmit the PROBE packet to the HN */

    //EXTRA
    UWUFETCH_NODE_STATUS_CHANGE last_reason; /**< Last reason because the SN or HN change its state */
    UWUFETCH_NODE_STATUS curr_state; /**< Current state in which the SN or HN is located */
    UWUFETCH_NODE_STATUS prev_state; /**< Previous state in which the SN or HN it was located */

    static bool initialized; /**< Indicate if the protocol has been initialized or not */
    bool print_transitions; /**< <i>true</i> if the writing of state transitions in the file is enabled. */
    int HEADNODE; /**< Indicate if the node work as HEAD NODE or SENSOR NODE */

    //Mapping   
    static std::map< UWUFETCH_NODE_STATUS, std::string > statusInfo; /**< Map the UWUFETCH_NODE_STATUS to the description of each state */
    static std::map< UWUFETCH_NODE_STATUS_CHANGE, std::string > statusChange; /**< Map the UWUFETCH_NODE_STATUS_CHANGE to the description the reason of changing state */
    static std::map< UWUFETCH_NODE_PACKET_TYPE, std::string > packetType; /**< Map the UWUFETCH_NODE_PACKET_TYPE to the description of  packet type */
    static std::map< UWUFETCH_TIMER_STATUS, std::string > statusTimer; /**< Map the UWUFETCH_TIMER_STATUS to the description of the timers */

    std::ofstream fout; /**< Variable that handle the file in which the protocol write the state transition for debug purposes */
    std::ofstream out_file_logging; /**< Variable that handle the file in which the protocol write the statistics */
};
#endif

