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
 * @file   uwUFetch_AUV.h
 * @author Loris Brolo
 * @version 1.0.0
 * 
 * @brief Declaration of class that implement AUV for UFetch protocol 
 */

#ifndef UWUFETCH_AUV_H_
#define UWUFETCH_AUV_H_

#include <mmac.h>
#include <iostream>
#include <clmessage.h>
#include <mphy.h>
#include <string>
#include <fstream>
#include <ostream>
#include <cassert>
#include <map>
#include <queue>

#define UWFETCH_AUV_DROP_REASON_ERROR "DERR"                                  /**<  Packet dropped: Packet corrupted */
#define UWFETCH_AUV_DROP_REASON_UNKNOWN_TYPE "DUT"                    /**< Packet dropped: Packet of unknown type */
#define UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER "DWR"                 /**< Packet dropped: Packet is addressed to another node of the */
#define UWFETCH_AUV_DROP_REASON_WRONG_STATE "DWS"                        /**< Packet dropped: AUV cannot receive this kind of packet in this state */
#define UWFETCH_AUV_DROP_PHY_CAN_NOT_TX "PNT"                                  /**< Packet dropped: Physical can not transmit the packet*/       
#define UWFETCH_AUV_DROP_REASON_ALREADY_ONE_RTS_RX "DAORR"      /**< RTS dropped: just one RTS it was analyzed by the AUV*/
#define UUFETCH_AUV_DROP_REASON_NOT_ENABLE "PDNE"                         /**< Packet dropped: AUV is not enabled to transmit or receive this type of packet*/   
#define UWUFETCH_AUV_DROP_CAN_NOT_RX_THIS_PCK "DCNR"                    /**< Packet dropped: AUV can not receive this particular type of packets */

extern packet_t PT_TRIGGER_UFETCH; /**< Trigger packet type for UFetch protocol */
extern packet_t PT_RTS_UFETCH; /**< RTS packet type for UFetch protocol */
extern packet_t PT_CTS_UFETCH; /**< CTS packet type for UFetch protocol */
extern packet_t PT_BEACON_UFETCH; /**< BEACON packet type for UFetch protocol */
extern packet_t PT_PROBE_UFETCH; /**< PROBE packet type for UFetch protocol */
extern packet_t PT_POLL_UFETCH; /**< POLL packet type for UFetch protocol */
extern packet_t PT_CBEACON_UFETCH; /**< CBEACON packet type for UFetch protocol */

/**
 * Class that represent the UFetch mac layer for AUV node
 */
class uwUFetch_AUV : public MMac {
public:
    /**
     * Constructor of the class UWUFetch       
     */
    uwUFetch_AUV();

    /**
     * Destructor of the class UWUFetch
     */
    virtual ~uwUFetch_AUV();

    /**
     * TCL command interpreter. It implements the following OTCL methods:
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

    /**< States in which the AUV node may be during Its execution */
    enum UWUFETCH_AUV_STATUS {
        UWUFETCH_AUV_STATUS_IDLE,
        UWUFETCH_AUV_STATUS_TRANSMIT_TRIGGER,
        UWUFETCH_AUV_STATUS_WAIT_RTS_PACKET,
        UWUFETCH_AUV_STATUS_RECEIVE_RTS_PACKET,
        UWUFETCH_AUV_STATUS_TRANSMIT_CTS_PACKET,
        UWUFETCH_AUV_STATUS_WAIT_DATA_HN,
        UWUFETCH_AUV_STATUS_RECEIVE_DATA_PACKET
    };

    /**< Reasons for which the AUV node moves from one state to another one */
    enum UWUFETCH_AUV_STATUS_CHANGE {
        UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_TX,
        UWUFETCH_AUV_STATUS_CHANGE_PHY_CAN_NOT_TX,
        UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_RTS_EXPIRED,
        UWUFETCH_AUV_STATUS_CHANGE_PACKET_IS_IN_ERROR,
        UWUFETCH_AUV_STATUS_CHANGE_RTS_RX,
        UWUFETCH_AUV_STATUS_CHANGE_CTS_TX,
        UWUFETCH_AUV_STATUS_CHANGE_DATA_PCK_RX,
        UWFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE,
        UWFETCH_AUV_STATUS_CHANGE_PACKET_ERROR,
        UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_TX_WAIT_RTS,
        UWUFETCH_AUV_STATUS_CHANGE_CTS_TX_WAIT_DATA,
        UWUFETCH_AUV_STATUS_CHANGE_RTS_FINISHED_TO_RX,
        UWUFETCH_AUV_STATUS_CHANGE_DATA_PCK_FINISHED_TO_RX,
        UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE,
        UWUFETCH_AUV_STATUS_CHANGE_LAST_DATA_PCK_RX,
        UWUFETCH_AUV_STATUS_CHANGE_ANOTHER_DATA_PCK_RX,
        UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_TO_EXPIRED,
        UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_INITIALIZE_TX_TRIGGER,
        UWUFETCH_AUV_STATUS_CHANGE_RTS_FINSHED_TO_STORE,
        UWUFETCH_AUV_STATUS_CHANGE_RTS_TO_EXPIRED_AT_LEAST_ONE_RTS_RX,
        UWUFETCH_AUV_STATUS_CHANGE_RTS_TO_EXPIRED_0_RTS_RX,
        UWUFETCH_AUV_STATUS_CHANGE_CTS_INITIALIZED_TX_CTS,
        UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED
    };

    /**< Type of packets transmitted and received by AUV node */
    enum UWUFETCH_AUV_PACKET_TYPE {
        UWUFETCH_AUV_PACKET_TYPE_DATA, UWUFETCH_AUV_PACKET_TYPE_TRIGGER,
        UWUFETCH_AUV_PACKET_TYPE_RTS, UWUFETCH_AUV_PACKET_TYPE_CTS
    };

    /**< Status timer in which node AUV can be found */
    enum UWUFETCH_TIMER_STATUS {
        UWUFETCH_TIMER_STATUS_IDLE, UWUFETCH_TIMER_STATUS_RUNNING,
        UWUFETCH_TIMER_STATUS_FROZEN, UWUFETCH_TIMER_STATUS_EXPIRED
    };

    /**
     * Class that handle the timers of AUV node
     */
    class uwUFetch_timer : public TimerHandler {
    public:

        /**
         *  Constructor of uwUFetch_timer class
         * @param m pointer to uwUFetch_AUV class
         */
        uwUFetch_timer(uwUFetch_AUV *m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(UWUFETCH_TIMER_STATUS_IDLE) {
            assert(m != NULL);
        }

        /**
         * Destructor of uwUFetch_timer class 
         */
        virtual ~uwUFetch_timer() {
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
         * Schedule a timer
         * 
         * @param val duration of the timer
         */
        virtual void schedule(double val) {
            start_time = NOW;
            left_duration = val;
            timer_status = UWUFETCH_TIMER_STATUS_RUNNING;
            resched(val);
        }

        /**
         *  Verify if the timer is IDLE
         * 
         *  @return: bool <i>true</i> IDLE
         */
        bool isIdle() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_IDLE);
        }

        /**
         *  Verify if the timer is RUNNING
         * 
         *  @return: bool <i>true</i> RUNNING         
         */
        bool isRunning() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_RUNNING);
        }

        /**
         *  Verify if the timer is FROZEN
         * 
         *  @return: bool <i>true</i> FROZEN          
         */
        bool isFroozen() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_FROZEN);
        }

        /**
         *  Verify if the timer is EXPIRED
         * 
         *  @return: bool <i>true</i> EXPIRED          
         */
        bool isExpired() {
            return ( timer_status == UWUFETCH_TIMER_STATUS_EXPIRED);
        }

        /**
         *  Verify if the timer is ACTIVE
         * 
         *  @return: bool <i>true</i> ACTIVE          
         */
        bool isActive() {
            return ( timer_status == isRunning() || timer_status == isFroozen());
        }

        /**
         *  Reset the counter that scan the timer   
         */
        void resetCounter() {
            counter = 0;
        }

        /**
         *  Increment the counter by one value
         */
        void incrCounter() {
            ++counter;
        }

        /**
         *  Counter value
         *  
         * @return int counter 
         */
        int getCounter() {
            return counter;
        }

        /**
         *  Left duration of the timer
         * 
         * @return double left_duration  
         */
        double getDuration() {
            return left_duration;
        }

    protected:
        double start_time; /**< Time to start the timer */
        double left_duration; /**<  Left duration of the timer */
        int counter; /**< Counter of the timer */
        uwUFetch_AUV* module; /**< Pointer to an object of type uwUFetch_AUV */
        UWUFETCH_TIMER_STATUS timer_status; /**< Timer status */

    }; //END class uwUFetch_timer

    /** 
     * Class inherited the method and variable of uwUFetch_timer that handle 
     * the timer of TRIGGER packets. 
     * Before to transmit in broadcast to the HNs a TRIGGER packet, the AUV node 
     * wait a finite interval time, after that start with the creation and transmission of TRIGGER packet.
     */
    class uwUFetch_TRIGGER_timer : public uwUFetch_timer {
    public:

        /**
         * Constructor of uwUFetch_TRIGGER_timer class
         * 
         * @param m a pointer to an object of type uwUFetch_AUV
         */
        uwUFetch_TRIGGER_timer(uwUFetch_AUV *m) : uwUFetch_timer(m) {
        }

        /*
         * Destructor of uwUFetch_TRIGGER_timer
         */
        virtual ~uwUFetch_TRIGGER_timer() {
        }

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_TRIGGER_timer 

    /** 
     * Class inherited the method and variable of  uwUFetch_timer that handle 
     * the timer of RTS packets. 
     * When the TRIGGGER packet is transmitted, the AUV node wait for a finite 
     * interval time a RTS packet from the HN. 
     * The uwUFetch_RTS_timer handle this waiting interval time.
     */
    class uwUFetch_RTS_timer : public uwUFetch_timer {
    public:

        /**
         * Constructor of uwUFetch_RTS_timer class
         * 
         * @param m pointer to an object of type UWUFetch_AUV
         */
        uwUFetch_RTS_timer(uwUFetch_AUV *m) : uwUFetch_timer(m) {
        }

        /*
         * Destructor of  uwUFetch_RTS_timer
         */
        virtual ~uwUFetch_RTS_timer() {
        }

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class UWUFetch_RTS_timer 

    /** 
     * Class inherited the method and variable of uwUFetch_timer that handle 
     * the timer of DATA packets.
     * After the transmission of CTS packet, the AUV node enter in a waiting 
     * state in which It wait a sequence of DATA packets requests from HN.
     * The uwUFetch_DATA_timer schedule this waiting interval time     
     */
    class uwUFetch_DATA_timer : public uwUFetch_timer {
    public:

        /**
         *  Constructor of uwUFetch_DATA_timer class
         *
         * @param m a pointer to an object of type uwUFetch_AUV
         */
        uwUFetch_DATA_timer(uwUFetch_AUV *m) : uwUFetch_timer(m) {
        }

        /*
         * Destructor of uwUFetch_DATA_timer
         */
        virtual ~uwUFetch_DATA_timer() {
        }

    protected:
        /**
         * Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class UWUFetch_DATA_timer   uwUFetch_FIRST_DATA_timer

    /** 
     * Class inherited the method and variable of uwUFetch_timer that handle 
     * the timer of DATA packets.
     * After the transmission of CTS or TRIGGER packet, the AUV node enter in a waiting 
     * state in which It wait a sequence of DATA packets requests from HN. This timer
     * wait the reception of first DATA packet from the specific HN. It is used only
     * if the communication come without RTS and CTS
     * The uwUFetch_DATA_timer schedule this waiting interval time     
     */
    class uwUFetch_FIRST_DATA_timer : public uwUFetch_timer {
    public:

        /**
         *  Constructor of uwUFetch_FIRST_DATA_timer class
         * @param m a pointer to an object of type uwUFetch_AUV
         */
        uwUFetch_FIRST_DATA_timer(uwUFetch_AUV *m) : uwUFetch_timer(m) {
        }

        /*
         * Destructor of uwUFetch_FIRST_DATA_timer
         */
        virtual ~uwUFetch_FIRST_DATA_timer() {
        }

    protected:
        /**
         *  Method called when the timer expire
         * 
         * @param Event*  pointer to an object of type Event
         */
        virtual void expire(Event *e);
    }; //END class uwUFetch_FIRST_DATA_timer

    /****************************************************************************
     *                        GENERAL METHODS                                   *
     ****************************************************************************/
    /**
     *  Handle the detected-start-of-PHY-reception event
     * 
     * @param p pointer to the packet whose reception has begun.
     */
    virtual void Phy2MacStartRx(const Packet *p);

    /**
     * Handle the detected-start-of-PHY-reception event in the case protocol doesn't use RTS and CTS packets
     * 
     * @param p pointer to the packet whose reception has begun
     */
    virtual void Phy2MacStartRx_without(const Packet *p);

    /**
     *  Handle the end-of-PHY-reception event
     *   
     * @param p pointer to the packet whose reception has ended.
     */
    virtual void Phy2MacEndRx(Packet *p);

    /**
     *  Handle the end-of-PHY-reception event in the case protocol doesn't use RTS and CTS packets
     
     * @param p pointer to the packet whose reception has ended.
     */
    virtual void Phy2MacEndRx_without(Packet *p);

    /**
     * This method must be called by the MAC to instruct the PHY to start the transmission of a packet
     * 
     * @param p pointer to the packet to be transmitted
     */
    virtual void Mac2PhyStartTx(Packet *p);

    /**
     * This method must be called by the MAC to instruct the PHY to start the transmission of a packet in the case 
     * protocol doesn't use RTS and CTS packets
     * 
     * @param p pointer to the packet to be transmitted
     */
    virtual void Mac2PhyStartTx_without(Packet *p);

    /**
     * Handle the end-of-PHY-transmission event
     * 
     * @param p pointer to the packet whose transmission has ended. Note that the Packet is not any more under 
     *  control of the MAC at the time this event occurs, hence the 'const' declaration.
     */
    virtual void Phy2MacEndTx(const Packet *p);

    /**
     * Handle the end-of-PHY-transmission event in the case protocol doesn't use RTS and CTS packets
     * 
     * @param p pointer to the packet whose transmission has ended. Note that the Packet is not any more under 
     *  control of the MAC at the time this event occurs, hence the 'const' declaration.
     */
    virtual void Phy2MacEndTx_without(const Packet *p);

    /**
     *  Prints a file with every state change for debug purposes.
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
     * @param reason The reason of the change of the state of the protocol
     */
    virtual void refreshReason(UWUFETCH_AUV_STATUS_CHANGE reason) {
        last_reason = reason;
    }

    /**
     *  Refresh the state of the protocol
     * 
     * @param state current state of the protocol
     */
    virtual void refreshState(UWUFETCH_AUV_STATUS state) {
        prev_state = curr_state;
        curr_state = state;
    }

    /*****************************************************************************
     *                             AUV METHODS                                                           *  
     *****************************************************************************/
    /**
     *  Idle state. Each variables are resetted
     */
    virtual void stateIdle_AUV();

    /**
     * Timeout trigger is expired. AUV start the procedure to communicate with HNs
     */
    virtual void TriggerTOExpired();

    /**
     * Initialization of TRIGGER packet that will be forwarded in broadcast to the HNs. Case in which UFetch protocol use RTS and CTS packets
     */
    virtual void state_TRIGGER_tx();

    /**
     * Initialization of TRIGGER packet that will be forwarded in broadcast to the HNs. Case in which UFetch protocol doesn't use RTS and CTS packets
     */
    virtual void state_TRIGGER_tx_without();

    /**
     * Trasmission of TRIGGER packet
     */
    virtual void TRIGGER_tx();

    /**
     * AUV is wait a RTS packet from HNs
     */
    virtual void state_wait_RTS();

    /**
     * AUV has received RTS packet. Extract all information cointed into It
     */
    virtual void RTS_rx();

    /**
     * Timeout within AUV is enabled to receive RTS packet is expired
     */
    virtual void RtsTOExpired();

    /**
     * Initialization of CTS packet that will be forwarded to the specifical HN.
     */
    virtual void state_CTS_tx();

    /**
     * Transmission of CTS packet
     */
    virtual void CTS_tx();

    /**
     * AUV is waiting the FIRST DATA packet from the HN to whom It has sent a RTS and CTS packet
     */
    virtual void state_wait_first_DATA();

    /**
     * AUV is waiting the FIRST DATA packet from the HN to whom It has sent only RTS packet  
     */
    virtual void state_wait_first_DATA_without();

    /**
     * AUV has received a DATA packet from HN. Extract all information contained into It.
     */
    virtual void DATA_rx();

    /**
     * AUV is waiting the SECOND and successive DATA packets from HN
     */
    virtual void state_wait_DATA();

    /**
     * Timeout within received DATA packets is expired. AUV is not enabled to receive other DATA packets from HN.
     */
    virtual void DataTOExpired();

    /**
     * Timeout within received the FIRST DATA packet from HN that it has sent a CTS packet is expired. Timeout used only
     * in the case It's not used the RTS and CTS packet for establish a connetion between HN and AUV
     */
    virtual void DataTOExpired_first();

    /**
     * Remove the information relative to the HN from which AUV wanted and it has  finished to receive DATA packets
     */
    virtual void updateQueueRTS();

    /*****************************************************************************
     *                             METODI AUSILIARI                              * 
     *****************************************************************************/
    /**
     * Establish whether the communication between SN and AUV take place with RTS and CTS or only using TRIGGER packet  
     * 
     * @return <i>true</i> if the connection between HN and AUV take place without RTS and CTS
     */
    virtual bool typeCommunication();

    /**
     * Compute the length of time interval within AUV want to receive all DATA packet from specifical HN
     * 
     * @return  time interval length 
     */
    virtual double getDataTimerValue();

    /**
     * Compute the transmission time for a specifical type of packet
     * 
     * @param tp type of packet which is necessary to calculate the transmission time including propagation delay
     */
    virtual void computeTxTime(UWUFETCH_AUV_PACKET_TYPE tp);

    /**
     * Compute the Round Trip Time
     * 
     * @return Round Trip Time 
     */
    virtual double getRTT();

    /**
     * Verify whether AUV must to receive another DATA packet from the HN to whome It has transmitted the 
     * TRIGGER or RTS and CTS paket
     */
    virtual void another_DATA_received();

    /*****************************************************************************
     *                           Metodi get & incr                               *
     *****************************************************************************/

    /**
     * Increment by 1 the number of TRIGGER packets transmitted by AUV during a single cycle TRIGGER-RTS-CTS-DATA 
     */
    void incrTrigger_Tx_by_AUV() {
        n_TRIGGER_tx_by_AUV++;
    }

    /**
     * Number of TRIGGER packets transmitted during the simulation during the single cycle
     * 
     * @return int n_TRIGGER_tx_by_AUV 
     */
    int getTrigger_Tx_by_AUV() {
        return n_TRIGGER_tx_by_AUV;
    }

    /**
     * Increase of 1 the number of TRIGGER packets transmitted by AUV. 
     */
    void incrTotalTrigger_Tx_by_AUV() {
        n_tot_TRIGGER_tx_by_AUV++;
    }

    /**
     * Total number of TRIGGER packets transmitted by the AUV during the simulation
     * 
     * @return int n_tot_TRIGGER_tx_by_AUV
     */
    int getTotalTrigger_Tx_by_AUV() {
        return n_tot_TRIGGER_tx_by_AUV;
    }

    /**
     * Increase of 1 the number of RTS packets received by AUV.
     */
    void incrRts_Rx_by_AUV() {
        n_RTS_rx_by_AUV++;
    }

    /**
     * Number of RTS packets received by AUV after the transmission of TRIGGER packet
     * 
     * @return int n_RTS_rx_by_AUV
     */
    int getRts_Rx_by_AUV() {
        return n_RTS_rx_by_AUV;
    }

    /**
     * Increase of 1 the number of RTS packets received by AUV.
     */
    void incrTotalRts_Rx_by_AUV() {
        n_tot_RTS_rx_by_AUV++;
    }

    /**
     * Total number of RTS received by AUV during the simulation from all HNs of the network
     * 
     * @return int n_tot_RTS_rx_by_AUV
     */
    int getTotalRts_Rx_by_AUV() {
        return n_tot_RTS_rx_by_AUV;
    }

    /**
     * Increase of 1 the number of corrupted RTS packets received by AUV.
     */
    void incrTotalRts_Rx_corrupted_by_AUV() {
        n_tot_RTS_rx_corr_by_AUV++;
    }

    /**
     * Total number of corrupted RTS packets received by AUV from all HNs of the network
     * 
     * @return int n_tot_RTS_rx_corr_by_AUV
     */
    int getTotalRts_Rx_corrupted_by_AUV() {
        return n_tot_RTS_rx_corr_by_AUV;
    }

    /**
     * Increase of 1 the number of CTS packets transmitted by AUV.
     */
    void incrCts_Tx_by_AUV() {
        n_CTS_tx_by_AUV++;
    }

    /**
     * Number of CTS packets transmitted by AUV to the HN
     * 
     * @return int n_CTS_tx_by_AUV
     */
    int getCts_Tx_by_AUV() {
        return n_CTS_tx_by_AUV;
    }

    /**
     * Increase of 1 the number of CTS packets transmitted by AUV.
     */
    void incrTotalCts_Tx_by_AUV() {
        n_tot_CTS_tx_by_AUV++;
    }

    /**
     * Total number of CTS packets transmitted by AUV during the simulation
     * 
     * @return int n_tot_CTS_tx_by_AUV
     */
    int getTotalCts_Tx_by_AUV() {
        return n_tot_CTS_tx_by_AUV;
    }

    /**
     * Increase of 1 the number of DATA packets received by AUV.
     */
    void incrData_Rx_by_AUV() {
        n_DATA_rx_by_AUV++;
    }

    /**
     * Number of DATA packets received by AUV after the transmission of TRIGGER-RTS-CTS packets from a specifical HN
     * 
     * @return int n_DATA_rx_by_AUV
     */
    int getData_Rx_by_AUV() {
        return n_DATA_rx_by_AUV;
    }

    /**
     * Increase of 1 the number of DATA packets received by AUV.
     */
    void incrTotal_Data_Rx_by_AUV() {
        n_tot_DATA_rx_by_AUV++;
    }

    /**
     * Total number of DATA packets received by AUV from all HNs of the netwrok during the simulation
     * 
     * @return int n_tot_DATA_rx_by_AUV 
     */
    int getTotal_Data_Rx_by_AUV() {
        return n_tot_DATA_rx_by_AUV;
    }

    /**
     * Increase of 1 the number of corrupted DATA packets received by AUV.
     */
    void incrTotal_Data_Rx_corrupted_by_AUV() {
        n_tot_DATA_rx_corr_by_AUV++;
    }

    /**
     * Total number of corrupted DATA packets received by the AUV from all HNs of the network
     * 
     * @return int n_tot_DATA_rx_corr_by_AUV
     */
    int getTotal_Data_Rx_corrupted_by_AUV() {
        return n_tot_DATA_rx_corr_by_AUV;
    }

    /*****************************************************************************
     *                         VARIABLES OF AUV NODE                             * 
     *****************************************************************************/
    // TCL VARIABLES
    double T_MIN_RTS; /**< Lower bound of the interval in which HN choice the back-off time to tx RTS pck */
    double T_MAX_RTS; /**< Upper bound of the interval in which HN choice the back-off time to tx RTS pck */
    double T_GUARD; /**< Guard time interval used between two consecutive transmissions of data packets */
    double T_RTS; /**<Interval time in which the AUV want to receive an RTS packet in answer to the trigger */
    int mode_comm_hn_auv; /**< Indicate how the communication takes place with or without RTS-CTS packets */
    int NUM_HN_NET; /**< Number of Head Nodes in the network */

    // TIMERS
    uwUFetch_TRIGGER_timer Trigger_timer; /**< Interval time. After that the AUV start the transmission of the TRIGGER packet */
    uwUFetch_RTS_timer RTS_timer; /**< Interval time in which the AUV wait a RTS packets from the HNs */
    uwUFetch_DATA_timer DATA_timer; /**< Interval time in which the AUV want to receive all DATA packets from the HN */
    uwUFetch_FIRST_DATA_timer DATA_timer_first_pck; /**< Interval time in which the AUV want to receive the first DATA packet from the HN */

    // GLOBAL VARIABLES THAT SHOULD BE NEVER RESET
    double MAX_PAYLOAD; /**< Maximum size of DATA PAYLOAD packet */
    int NUM_MAX_DATA_AUV_WANT_RX; /**< Maximum number of data packet that AUV want to receive from the HN in a single cycle of TRIGGER-RTS-CTS-DATA*/
    double T_START_PROC_TRIGGER; /**< Time before that the AUV start the procedure to transmit a TRIGGER packet */
    int debugMio_; /**< Used if we want to create the logging file */
    int N_RUN; /**< Number of run in execution  */
    int HEAD_NODE_1; /**< Id number of HN 1 */
    int HEAD_NODE_2; /**< Id number of HN 2 */
    int HEAD_NODE_3; /**< Id number of HN 3 */
    int HEAD_NODE_4; /**< Id number of HN 4 */
    double tx_TRIGGER_start_time; /**< Indicates when AUV start a transmission of TRIGGER packet */
    double tx_TRIGGER_finish_time; /**< Indicates when AUV end the transmission of TRIGGER packet */
    double rx_RTS_start_time; /**< Indicates when AUV start the reception of RTS packet */
    double rx_RTS_finish_time; /**< Indicates when AUV end the reception of RTS packet */
    double tx_CTS_start_time; /**< Indicates when AUV start a transmission of CTS packet */
    double tx_CTS_finish_time; /**< Indicates when AUV end the transmission of CTS packet */
    double rx_DATA_start_time; /**< Indicates when AUV start the reception of DATA packet */
    double rx_DATA_finish_time; /**< Indicates when AUV end the reception of DATA packet */
    int n_tot_TRIGGER_tx_by_AUV; /**< Counter of the number of TRIGGER packets transmitted by AUV during an entire duration of simulation */
    int n_tot_RTS_rx_by_AUV; /**< Counter of the number of RTS packets received by AUV during an entire duration of simulation */
    int n_tot_RTS_rx_corr_by_AUV; /**< Counter of the number of  RTS packets corrupted received by AUV during an entire duration of simulation */
    int n_tot_CTS_tx_by_AUV; /**< Counter of the number of CTS packets transmitted by AUV during an entire duration of simulation */
    int n_tot_DATA_rx_by_AUV; /**< Counter of the number of DATA packets received by AUV during an entire duration of simulation */
    int n_tot_DATA_rx_corr_by_AUV; /**< Counter of the number of DATA packets corrupted received by AUV during an entire duration of simulation */
    int number_data_pck_AUV_rx_exact; /**< Number of DATA packets that AUV want exactly received from the HN */
    int mac_addr_HN_ctsed; /**< Mac address of the HN from which the AUV want to receive the DATA packets */
    double data_timeout; /**< Interval time within AUV want to receive all DATA packets from the HN */
    double T_tx_TRIGGER; /**< Time required to transimt a single TRIGGER packet */
    double Trts; /**< Time needed to transmit a RTS packet. It contain also the propagation delay */
    double Tcts; /**< Time needed to transmit a CTS packet. It contain also the propagation delay */
    double Tdata; /**<Time needed to transmit a DATA packet. It contain also the propagation delay */
    double RTT; /**< Round Trip Time*/
    int mac_addr_HN_in_data; /**< MAC address of the HN from which the AUV has received the DATA packet  */
    int last_hn_triggered; /**< Index of the HN that was triggered in the previous cycle by the AUV */
    int hn_trigg; /**< Index of the HN that will be triggered with the next cycle of TRIGGER-RTS-CTS */
    int index; /**< */
    int mac_addr_hn_triggered; /**< MAC address of the HN triggered by the AUV.  */
    int num_pck_hn_1; /**< Counter of the correct DATA packets received by HN 1 from SNs */
    int num_pck_hn_2; /**< Counter of the correct DATA packets received by HN 2 from SNs */
    int num_pck_hn_3; /**< Counter of the correct DATA packets received by HN 3 from SNs */
    int num_pck_hn_4; /**< Counter of the correct DATA packets received by HN 4 from SNs */

    // VARIABLES THAT ENABLES OR NOT AN OPERATION
    bool txTRIGGEREnabled; /**< <i>true</i> if AUV is enabled to transmit a TRIGGER packet */
    bool rxRTSEnabled; /**< <i>true</i> if AUV is enabled to receive a RTS packet */
    bool txCTSEnabled; /**< <i>true</i> if AUV is enabled to transmit a CTS packet */
    bool rxDATAEnabled; /**< <i>true</i> if AUV is enabled to receive a DATA packet */

    // VARIABLES THAT SHOULD BE RESETED WHEN A CYCLE IS ENDED
    int n_TRIGGER_tx_by_AUV; /**< Counter of the number of TRIGGER packets transmitted by AUV during a single cycle of simulation */
    int n_RTS_rx_by_AUV; /**< Counter of the number of RTS packets received by AUV during a single cycle of simulation */
    int n_CTS_tx_by_AUV; /**< Counter of the number of CTS packets transmitted by AUV during a single cycle of simulation */
    int n_DATA_rx_by_AUV; /**< Counter of the number of DATA packets transmitted by AUV during a single cycle of simulation */

    // PACKETS CREATED
    Packet* curr_TRIGGER_pck_tx; /**< Pointer to the TRIGGER packet that is being transmitted by AUV */
    Packet* curr_RTS_pck_rx; /**< Pointer to the RTS packet that is being received by AUV*/
    Packet* curr_CTS_pck_tx; /**< Pointer to the CTS packet that is being transmitted by AUV*/
    Packet* curr_DATA_pck_rx; /**< Pointer to the DATA packet that is being received by AUV*/

    //STRUCTURES USED
    std::queue<Packet*> Q_data_AUV; /**< Queue of DATA packets stored by the AUV and received from HNs */
    std::queue<int> Q_rts_mac_HN; /**< Queue of HN MAC address from which AUV has received correctly the RTS packet */
    std::queue<int> Q_rts_n_pcks_HN_want_tx_AUV; /**< Queue that store the number of DATA packets that the single HN want to tx to the AUV. */
    std::queue<double> Q_rts_backoff_time; /**< Queue that stored the backoff time choice by the single HN before to transmit RTS packet */

    //VARIABLES THAT INDICATE IN WHICH STATE THE NODE IS IN THAT MOMENT AND THE REASON BECAUSE THE NODE PASS FROM A STATE TO ANOTHER ONE
    UWUFETCH_AUV_STATUS_CHANGE last_reason; /**< Last reason because the NODE change his state */
    UWUFETCH_AUV_STATUS curr_state; /**< Current state in which the node is located */
    UWUFETCH_AUV_STATUS prev_state; /**< Previous state in which the node it was located */

    static std::map< UWUFETCH_AUV_STATUS, std::string > statusInfo; /**< Map the UWUFETCH_AUV_STATUS to the description of each state */
    static std::map< UWUFETCH_AUV_STATUS_CHANGE, std::string > statusChange; /**< Map the UWUFETCH_AUV_STATUS_CHANGE to the description the reason of changing state */
    static std::map< UWUFETCH_AUV_PACKET_TYPE, std::string > packetType; /**< Map the UWUFETCH_AUV_PACKET_TYPE to the description of  packet type*/
    static std::map< UWUFETCH_TIMER_STATUS, std::string > statusTimer; /**< Map the UWUFETCH_TIMER_STATUS to the description of the timers*/

    //VARIABLES FOR DEBUG
    std::ofstream fout; /**< Variable that handle the file in which the protocol write the state transition for debug purposes */
    std::ofstream out_file_logging; /**< Variable that handle the file in which the protocol write the statistics */
    static bool initialized; /**< Indicate if the protocol has been initialized or not */
    bool print_transitions; /**< <i>true</i> if the writing of state transitions in the file is enabled. */

    static const int MAX_RTS_RX = 100; /**< Maximum number of RTS packets that AUV can receive */

}; //END class UWUFetch_AUV


#endif




