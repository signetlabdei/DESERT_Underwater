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
 * @file   uwsr.h
 * @author Saiful Azad
 * @version 1.0.0
 * 
 * @brief  UnderWater Selective Repeat (UWSR) this is basically a selective repeat ARQ protocol that coordinates the transmitter and receiver in time. 
 * In more detail, the transmitter sends Data packets and waits for a given time period between subsequent packets; the receiver knows the timing 
 * of the transmitter and sends acknowledgement (ACK) packets so that they arrive at the transmitter when it is listening. The protocol is effective 
 * because the channel propagation delay, underwater, is sufficiently large to accommodate more than one packet transmission within one round-trip 
 * time (RTT). Some parameters of the protocol can be tuned: for example, setting a higher number of packets to be transmitted within one RTT makes 
 * the transmission process more efficient, but also more prone to interference in a multiuser network.
 */


#ifndef UWSR_H 
#define UWSR_H

#include <mac.h>
#include <mmac.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <utility>
#include <fstream>

#include <mphy.h>

#define UWSR_DROP_REASON_WRONG_STATE "WST"
#define UWSR_DROP_REASON_WRONG_RECEIVER "WRCV"
#define UWSR_DROP_REASON_UNKNOWN_TYPE "UPT"
#define UWSR_DROP_REASON_BUFFER_FULL "DBF"
#define UWSR_DROP_REASON_ERROR "ERR"

extern packet_t PT_MMAC_ACK;

typedef int pktSeqNum;
typedef double txStartTime;
typedef int macAddress;
typedef int txRounds;
typedef pair <macAddress, pktSeqNum> usrPair;
typedef pair <double, double> rttPair;
typedef pair <int, int> txStatusPair;


/**
*@brief This is the base class of MMacUWSR protocol, which is a derived class of MMac.
*/

class MMacUWSR : public MMac {

  
  public:
    
  /**
  *Constructor of MMacUWSR Class
  */
  MMacUWSR();
  
  /**
  *Destructor of MMacUWSR Class
  */
  virtual ~MMacUWSR();

  /**
  * TCL command interpreter. It implements the following OTcl methods:
  * @param argc number of arguments in <i>argv</i>
  * @param argv array of strings which are the command parameters (Note that argv[0] is the name of the object)
  * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or not
  */  
  virtual int command(int argc, const char*const* argv);
  
  protected:
  
  /**
  *Enumeration class of MMacUWSR status. First enumerator is given value 1.
  */  
  enum UWSR_STATUS {
    UWSR_STATE_IDLE = 1, UWSR_STATE_BACKOFF, UWSR_STATE_TX_DATA, UWSR_STATE_TX_ACK, UWSR_STATE_WAIT_ACK, 
    UWSR_STATE_DATA_RX, UWSR_STATE_ACK_RX, UWSR_STATE_NOT_SET, UWSR_STATE_LISTEN,
    UWSR_STATE_CHK_ACK_TIMEOUT, UWSR_STATE_CHK_LISTEN_TIMEOUT, UWSR_STATE_CHK_BACKOFF_TIMEOUT, UWSR_STATE_RX_IDLE,
    UWSR_STATE_RX_LISTEN, UWSR_STATE_RX_BACKOFF, UWSR_STATE_RX_WAIT_ACK, UWSR_STATE_WRONG_PKT_RX, UWSR_STATE_WAIT_TX,
    UWSR_STATE_CHK_WAIT_TX_TIMEOUT, UWSR_STATE_WAIT_ACK_WAIT_TX, UWSR_STATE_RX_DATA_TX_DATA, UWSR_STATE_PRE_TX_DATA, UWSR_STATE_RX_IN_PRE_TX_DATA
  };

  /**
  *Enumeration class which tells the nodes the reason why it is in this state. First enumerator is given value 1.
  */
  enum UWSR_REASON_STATUS {
    UWSR_REASON_DATA_PENDING, UWSR_REASON_DATA_RX, UWSR_REASON_DATA_TX, UWSR_REASON_ACK_TX, 
    UWSR_REASON_ACK_RX, UWSR_REASON_BACKOFF_TIMEOUT, UWSR_REASON_ACK_TIMEOUT, UWSR_REASON_DATA_EMPTY, 
    UWSR_REASON_NOT_SET, UWSR_REASON_MAX_TX_TRIES, UWSR_REASON_BACKOFF_PENDING, UWSR_REASON_LISTEN,
    UWSR_REASON_LISTEN_TIMEOUT, UWSR_REASON_LISTEN_PENDING, UWSR_REASON_START_RX, 
    UWSR_REASON_PKT_NOT_FOR_ME, UWSR_REASON_WAIT_ACK_PENDING, UWSR_REASON_PKT_ERROR, UWSR_REASON_WAIT_TX,
    UWSR_REASON_WAIT_TX_PENDING, UWSR_REASON_WAIT_TX_TIMEOUT
  };

  /**
  *Enumeration class of packet type. First enumerator is given value 1. Three kinds of packets are supported by MMacUWSR protocol.
  */
  enum UWSR_PKT_TYPE {
    UWSR_ACK_PKT = 1, UWSR_DATA_PKT, UWSR_DATAMAX_PKT
  };

  /**
  *Enumeration class of MMacUWSR timer status. First enumerator is given value 1. It is employed to know the current status of a timer.
  */
  enum UWSR_TIMER_STATUS {
    UWSR_IDLE = 1, UWSR_RUNNING, UWSR_FROZEN, UWSR_EXPIRED
  };

  /**
  * This enumeration class is employed to know the current acknowledgement timer status. First enumerator is given value 1.
  */
  enum CHECK_ACK_TIMER {
    CHECK_ACTIVE = 1, CHECK_EXPIRED, CHECK_IDLE
  };

  /**
  * Base class of all the timer used in this protocol. This is a derived class of TimerHandler.
  */
  class UWSRTimer : public TimerHandler {
    
    
    public:

    /**
    * Constructor of UWSRTimer Class.
    */
    UWSRTimer(MMacUWSR *m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(UWSR_IDLE) { 
      assert(m != NULL); }
    
    /**
    * Destructor of UWSRTimer Class.
    */
    virtual ~UWSRTimer() { }
    
    /** It freezes or in another word, it stops the timer for some time. Suppose, for some reason we want to stop 
    * a timer for some period and we want to run this timer from where it was stopped. This function stops the timer and 
    * save the left time duration it must run.
    */
    virtual void freeze() { assert(timer_status == UWSR_RUNNING); left_duration -= (NOW - start_time); 
                            if (left_duration <= 0.0) left_duration = module->mac2phy_delay_; force_cancel();
                            timer_status = UWSR_FROZEN; }
    
    /**
    * It starts the timer from where it was stopped. To run any freeze timer, we can use unfreeze method.   
    */
    virtual void unFreeze() { assert(timer_status == UWSR_FROZEN); start_time = NOW; assert(left_duration > 0);
                              sched(left_duration); timer_status = UWSR_RUNNING; }
    
    /**
    * Stop the timer any way.
    */
    virtual void stop() { timer_status = UWSR_IDLE; force_cancel(); }
    
    /**
    * Schedule the time, i.e., how long a timer is going to run. 
    * @param double time
    */
    virtual void schedule( double val ) { start_time = NOW; left_duration = val; timer_status = UWSR_RUNNING; resched(val); }
    
    /**
    * It tells whether the timer is in Idle state or not.
    * @return 1 if the timer is idle and 0 if it is not.
    */    
    bool isIdle() { return ( timer_status == UWSR_IDLE ); }

    /**
    * This method tells whether the timer is in Running state or not.
    * @return 1 if the timer is running and 0 if it is not.
    */
    bool isRunning() { return (timer_status == UWSR_RUNNING); }

    /**
    * Tells whether the timer is expired or not.
    * @return 1 if the timer expired and 0 if it is not.
    */
    bool isExpired() { return (timer_status == UWSR_EXPIRED); }

    /**
    * It tells whether the timer is in freeze mode or not.
    * @return 1 if the timer is in freeze mode and 0 if it is not.
    */
    bool isFrozen() { return (timer_status == UWSR_FROZEN); }

    /**
    * It tells whether the timer is active or not.
    * @return 1 if the timer is active and 0 if it is not.
    */
    bool isActive() { return (timer_status == UWSR_FROZEN || timer_status == UWSR_RUNNING ); }
    
    /**
    * Reset the timer counter.
    */
    void resetCounter() { counter = 0; }
    
    /**
    * Increment the timer counter. It helps to know the statics of the timer.
    */
    void incrCounter() { ++counter; }
    
    /**
    * It provides, how many times a timer ran.
    * @return number of times a timer ran (int).
    */
    int getCounter() { return counter; }
    
    /**
    * This methods provide the duration of a timer.
    * @return duration of a timer (double).
    */
    double getDuration() { return left_duration; }
    
    /**
    * This methods provide the remaining duration of a timer.
    * @return left time duration of a timer (double).
    */
    double leftDuration() { return NOW - start_time;}
    
    
    protected:

      
    double start_time; /**< Start time of a timer. */
    
    double left_duration; /**< How long a timer is going to run more. */ 
       
    int counter; /**< How many times a timer ran. */
    
    
    MMacUWSR* module; /**< Pointer of MMacUWSR module. */
    
    UWSR_TIMER_STATUS timer_status; /**< Set the status of the timer. */
    
    
  };
  
  /**
  * Base class of BackoffTimer. It is derived class of UWSRTimer.
  */
  class BackOffTimer : public UWSRTimer { 
  
    
    public: 
    
    /**
    * Constructor of BackOffTimer Class.
    */
    BackOffTimer(MMacUWSR* m) : UWSRTimer(m) { }
    
    /**
    * Destructor of BackOffTimer.
    */
    virtual ~BackOffTimer() { }
    
    
    protected:
      
    /**
    * What a node is going to do when a timer expire.
    * @param Event
    */  
    virtual void expire(Event *e);

  
  };
  
 
  /**
  * Base class of AckTimer, which is a derived class of UWSRTimer.
  */   
  class AckTimer : public UWSRTimer { 
  
    
    public:

    /**
    * Constructor of AckTimer Class.
    */
    AckTimer(MMacUWSR* m) : UWSRTimer(m) { }

    /**
    * Destructor of AckTimer Class.
    */
    virtual ~AckTimer() { }
    
      
    protected:

    /**
    * What a node is going to do when a timer expire.
    * @param Event
    */
    virtual void expire(Event *e);
    

  };
  

  /**
  * Base class of ListenTimer, which is a derived class of UWSRTimer.
  */  
  class ListenTimer : public UWSRTimer { 
  
    
    public:
    
    /**
    * Constructor of ListenTimer class
    */
    ListenTimer(MMacUWSR* m) : UWSRTimer(m) { }
    
    /**
    * Destructor of ListenTimer class
    */
    virtual ~ListenTimer() { }
    
      
    protected:

    /**
    * What a node is going to do when a timer expire.
    * @param Event
    */
    virtual void expire(Event *e);

    
  };

  /**
  * Base class of WaitTxTimer, which is a derived class of UWSRTimer.
  */    
  class WaitTxTimer : public UWSRTimer { 
  
    
    public:
    
    /**
    * Constructor of WaitTxTimer class.
    */
    WaitTxTimer(MMacUWSR* m) : UWSRTimer(m) { }
    
    /**
    * Destructor of WaitTxTimer class.
    */
    virtual ~WaitTxTimer() { }
    
      
    protected:

    /**
    * What a node is going to do when a timer expire.
    * @param Event
    */
    virtual void expire(Event *e);

    
  };
  
  /**
  * This function receives the packet from upper layer and save it in the queue.
  * @param Packet pointer
  */
  virtual void recvFromUpperLayers(Packet* p);

  /**
  * It informs that a packet transmission started.
  * @param Packet pointer
  */  
  virtual void Mac2PhyStartTx(Packet* p);
  
  /**
  * It infroms that a packet transmission end.
  * @param Packet pointer (constant)
  */
  virtual void Phy2MacEndTx(const Packet* p);
  
  /**
  * PHY layer informs the MAC layer that it is receiving a packet.
  * @Param Packet pointer (constant)
  */
  virtual void Phy2MacStartRx(const Packet* p);
  
  /**
  * PHY layer informs the MAC layer that the reception of the packet is over.
  * @param Packet pointer.
  */
  virtual void Phy2MacEndRx(Packet* p);

  /**
  * Compute the transmission time of a packet. It uses a cross-layer message to calculate the duration of that packet. 
  * @param type is a UWSR_PKT_TYPE
  * @return tranmission time of a packet which is a double data type.
  */  
  virtual double computeTxTime(UWSR_PKT_TYPE type);

  /**
  * This method, initialize the packet. If the packet is received from the upper layer, it adds the header (if any). It also sets the fields of ACK packet.
  * @param Packet pointer P. The packet can be <i>Data</i> packet or <i>ACK</i> packet.
  * @param pkt_type is an UWSR_PKT_TYPE. Packet can be either <i>Data</i> packet or <i>ACK</i> packet.
  * @param dest_addr is a integer data type. It is initialized as 0.
  */
  virtual void initPkt( Packet* p, UWSR_PKT_TYPE pkt_type, int dest_addr = 0);
  
  /**
  * This function calculates the backoff duration and return the backoff time.It employs the exponential backoff algorithm.
  * @return backoff duration which is a double data type.
  */
  virtual double getBackoffTime();
  
  /**
  * It sets the number of times backoff cpounter is run.
  */
  inline void setBackoffCount() { backoff_count++; } 
  
  /**
  * It returns the number of time backoff counter is run in the whole communication period.
  * @return backoff_count which is an integer data type.
  */
  inline int getBackoffCount() { return backoff_count; }

  /**
  * This method finds out whether there is any packet to be transmitted to the destination. 
  */
  virtual void statePreTxData();
  
  /**
  * This methods transmits <i>ACK</i> packet from MAC layer to PHY layer.
  * @param dest_addr which is an integer data type.
  */
  virtual void txAck(int dest_addr);

  /**
  * Node is in Idle state. It only changes its state if it has packet(s) to transmit or it receives a packet.
  */
  virtual void stateIdle();
  
  /**
  * If a node starts receiving a packet in Idle state.
  */
  virtual void stateRxIdle();
  
  /**
  * If a node has packet to transmits. In such case, it moves from Idle state to data transmits state.
  */
  virtual void stateTxData();
  
  /**
  * After receiving a <i>Data</i> packet the node sends an <i>ACK</i> packet.
  */
  virtual void stateTxAck(int dest_addr);
  
  /**
  * If <i>ACK</i> packet is not received within the acknowledgement expire time.
  */
  virtual void stateBackoff();
  
  /**
  * If a node starts receiving a packet when it is in backoff state. The node first freeze (or another word, hold) the backoff timer and start receiving 
  * the packet. 
  */
  virtual void stateRxBackoff();
  
  /**
  * After transmitting a <i>Data</i> packet, a node waits for the <i>ACK</i> packet.
  */
  virtual void stateWaitAck();
  
  /**
  * If a node starts receiving a packet when it was waiting for transmitting another packet in a singlet RTT.
  */
  virtual void stateRxinPreTxData();
  
  /**
  * If a node receives any packet while it was waiting for <i>ACK</i> packet, it moves to this state. The packet it is receiving can be a <i>Data</i> packet
  * from another node or <i>ACK</i> packet.
  */
  virtual void stateRxWaitAck();
  
  /**
  * A short time which is used to sense the channel condision.
  */
  virtual void stateListen();
  
  /**
  * If a node starts receiving a packet when it was listening the channel.
  */
  virtual void stateRxListen();

  /**
  * It checks whether the listen timer is already expired while it was busy with other activities.
  */
  virtual void stateCheckListenExpired();
  
  /**
  * It checks whether the ack timer is already expired while it was busy with other activities.
  */
  virtual void stateCheckAckExpired();

  /**
  * It checks whether the backoff timer is already expired while it was busy with other activities.
  */
  virtual void stateCheckBackoffExpired();
  
  /**
  * It checks whether the wait transmit timer is already expired while it was busy with other activities.
  */
  virtual void stateCheckWaitTxExpired();
  
  /**
  * It process the packet which is received. After receiving a packet it changes it states according to the previously stored status information.
  * @param <i>Data</i> packet pointer
  */
  virtual void stateRxData(Packet* p);
  
  /**
  * The node comes to this state if it receives an <i>ACK</i> packet. After receiving an <i>ACK</i> packet it changes it states according 
  * to the previously stored status information.
  */
  virtual void stateRxAck(Packet* p);
  
  /**
  * The receive packet is not intended for this node. The node returns back to its own state.
  * @param packet pointer
  */
  virtual void stateRxPacketNotForMe(Packet* p);

  /**
  * This methods print the state information of the nodes.
  * @param delay is a double data type.
  */
  virtual void printStateInfo(double delay = 0);
  
  /**
  * This function is used to initialize the UWAloha protocol.
  */
  virtual void initInfo();
  
  /**
  * Refreshes the states of the node. The node save the information of three states, they are: previous to previous state, previous state and 
  * current state of the node.
  * @param state which is an UWSR_STATUS type.
  */
  virtual void refreshState(UWSR_STATUS state) { prev_prev_state = prev_state; prev_state = curr_state; curr_state = state; }
  
  /**
  * To know the reason why a node in this current state. 
  * @param reason is an UWSR_REASON_STATUS type.
  */
  virtual void refreshReason(UWSR_REASON_STATUS reason) { last_reason = reason; }

  /**
  * It stops the backoff timer.
  */
  virtual void exitBackoff();

  /**
  * This method is used to get the average RTT over all the receives RTT.
  * @return average RTT time which is a double data type.
  */
  virtual double getRTT() { return (rttsamples>0) ? sumrtt/rttsamples : 0 ; }

  /**
  * It updates the sequence number of the last data packet rx.
  * @param id is an integer data type.
  */
  virtual void updateLastDataIdRx(int id) { last_data_id_rx = id; }
  virtual void waitForUser();

  // stats functions
  /**
  * Number of packets which MAC layer receives form upper layer(s) but were not transmitted.
  * @return an integer value. 
  */
  virtual int getRemainingPkts() { return(up_data_pkts_rx - mapPacket.size()); }
  
  /**
  * Increment the number of <i>Data</i> packet receive for the upper layer.
  */
  virtual void incrUpperDataRx() {up_data_pkts_rx++;}
  
  /**
  * This method is used to get the sequence number from a packet. 
  * @param packet pointer
  * @return it returns sequence number which is an integer data type.
  */  
  virtual int getPktSeqNum(Packet* p) { hdr_cmn* ch = hdr_cmn::access(p); return ch->uid(); }
  
  /**
  * This method returns the destination mac address of the node which transmits the packet.
  * @param packet pointer
  * @return it returns the mac address of the node 
  */
  virtual int getMacAddress(Packet* p) { hdr_mac* mach = HDR_MAC(p); return mach->macDA(); }
  
  ///handling transmission round
  /**
  * It checks whether the same packet is transmitted before.
  * @param mac address of the node 
  * @param sequence number of the packet.
  * @return boolean data type. Returns <i>TRUE</i> when this packet was not transmitted before and <i>FALSE</i> when it was transmitted.
  */
  inline bool chkItemInmapTxRounds(int mac_addr, int seq_num);
  
  /**
  * It sets the current tranmission round of a packet. 
  * @param mac address of the node 
  * @param sequence number of the packet.
  */
  inline void setCurrTxRounds(int mac_addr, int seq_num) { curr_tx_rounds = 1; mapTxRounds.insert (make_pair (make_pair (mac_addr, seq_num),curr_tx_rounds)); }

  /**
  * Increment the number of times a packet is transmitted.
  * @param mac address of the node 
  * @param sequence number of the packet.
  */
  inline void incrCurrTxRounds(int mac_addr, int seq_num) { map< usrPair,txRounds> :: iterator it_t; it_t = mapTxRounds.find(make_pair(mac_addr,seq_num)); (*it_t).second += 1; }
  
  /**
  * Returns the current transmission round of a <i>Data</i> packet.
  * @param mac address of the node 
  * @param sequence number of the packet.
  * @return an integer data type. 
  */
  inline int getCurrTxRounds(int mac_addr, int seq_num) { map< usrPair,txRounds> :: iterator it_t; it_t = mapTxRounds.find(make_pair(mac_addr,seq_num)); return ((*it_t).second); }
  
  /**
  * Erase an Item form the container where transmission round of various transmit packet are stored. 
  * @param mac address of the node 
  * @param sequence number of the packet.
  */
  inline void eraseItemFromTxRounds(int mac_addr, int seq_num) { map< usrPair,txRounds> :: iterator it_t; it_t = mapTxRounds.find(make_pair(mac_addr,seq_num)); mapTxRounds.erase((*it_t).first); }
  
  ///handling packets 
  /**
  * Store the packet in a container.
  * @param packet pointer
  */
  inline void putPktInQueue(Packet *p) { mapPacket.insert(make_pair (make_pair (getMacAddress(p), getPktSeqNum(p)), p)); }
  
  /**
  * Erase the packet which is delivered to the destination correctly or other reasons
  */
  inline void eraseItemFromPktQueue(int mac_addr, int seq_num){ map< usrPair,Packet*> :: iterator it_p; it_p = mapPacket.find(make_pair(mac_addr, seq_num)); Packet::free((*it_p).second); mapPacket.erase((*it_p).first); }
  
  ///managing ack
  /**
  * Put AckTimer Object in the container. For every transmitted packet an AckTimer object is created.
  * @param mac address of the node 
  * @param sequence number of the packet.
  */
  inline void putAckTimerInMap(int mac_addr, int seq_num) { mapAckTimer.insert(make_pair (make_pair (mac_addr, seq_num),ack_timer)); }
  
  /**
  * If a node receives an <i>ACK</i> for its transmitted packet, it erases the related AckTimer object of that packet.
  * @param mac address of the node 
  * @param sequence number of the packet.
  */
  inline void eraseItemFrommapAckTimer(int mac_addr, int seq_num) { map< usrPair,AckTimer> :: iterator it_a; it_a = mapAckTimer.find(make_pair (mac_addr, seq_num)); mapAckTimer.erase((*it_a).first); }
  
  /**
  * It stores the initial transmission start time in a container.
  * @param mac address of the node 
  * @param sequence number of the packet.
  */
  inline void putStartTxTimeInMap(int mac_addr, int seq_num, double start_tx_time) { mapCalcAck.insert(make_pair (make_pair (mac_addr, seq_num),start_tx_time)); } 
  
  /**
  * Erases the transmission start time of a packet when an <i>ACK</i> is received correctly.
  * @param mac address of the node 
  * @param sequence number of the packet.
  */
  inline void eraseItemFrommapCalcAck(int mac_addr, int seq_num) { map< usrPair,txStartTime> :: iterator it_ca; it_ca = mapCalcAck.find(make_pair(mac_addr, seq_num)); mapCalcAck.erase((*it_ca).first); }

  /**
  * It checks whether any acknowledgement timer expire. Since, in UWSR, we are transmitting multiple packet in a single RTT, we have to check AckTimer
  * container whether any acknowledgement timer expire or not.
  * @param type which is a CHECK_ACK_TIMER data type.
  * @return number of acknowledgements expire in the container.
  */
  virtual int checkAckTimer(CHECK_ACK_TIMER type);
  
  /**
  * Erases those items whose acknowledgement timer expire from AckTimer containter and also from start trasmit time contaioner.
  */
  virtual void eraseExpiredItemsFrommapAckandCalc();
  
  /**
  * This method checks whether the node is capable of sending multiple packets in a single RTT.
  * @param mac address of the receiving node.
  * @return Returns <i>TRUE</i> if multiple packet transmission is possible, <i>FALSE</i> otherwise.
  */
  virtual bool checkMultipleTx(int rcv_mac_addr); 
  
  /**
  * Number of packets a node can transmits to a receiving node in a single RTT.
  * @param mac address of the receiving node.
  * @return number of packets can transmit in a single RTT (integer).
  */
  virtual int getPktsCanSendIn1RTT(int mac_addr); 
  
  /**
  * Put RTT of all the nodes which are whithin the transmission range of a node. Whever a node receives a packet from another node, it put the RTT
  * information of that node in a container.
  * @param mac address of the node which transmits the packet
  * @param RTT of the sender and receiver.
  */
  virtual void putRTTInMap (int mac_addr, double rtt);
  
  /**
  * Returns the RTT between a sender-receiver pair.
  * @param mac address of the receiving node.
  * @return RTT which is a double data type.
  */
  inline double getRTTInMap(int mac_addr) { map< macAddress, rttPair> :: iterator it_d; it_d = mapRTT.find(mac_addr); return (it_d->second).first;}
  
  /**
  * Increments the number of <i>Data</i> packets sent in single RTT.
  */
  inline void incrPktsSentIn1RTT () { pkts_sent_1RTT++; }
  
  /**
  * Resets the number of <i>Data</i> packets sent in single RTT.
  */
  inline void rstPktsSentIn1RTT () { pkts_sent_1RTT = 0; }
  
  /**
  * Returns number of <i>Data</i> packets transmit in single RTT.
  * @return packets sent in single RTT (integer).
  */
  inline int getPktsSentIn1RTT () { return pkts_sent_1RTT; }

  /**
  * Increments the number of <i>ACK</i> packets receive by a node in single RTT.
  */
  inline void incrAcksRcvIn1RTT () { acks_rcv_1RTT++; }
  
  /**
  * Resets the number of <i>ACK</i> packets receive by a node in single RTT.
  */
  inline void rstAcksRcvIn1RTT () { acks_rcv_1RTT = 0; }
  
  /**
  * Returns number of <i>ACK</i> packets receive by a node in single RTT.
  * @return number of <i>ACK</i> receive in single RTT.
  */
  inline int getAcksRcvIn1RTT () { return acks_rcv_1RTT; }
 
  /**
  * Increments the number of packets lost in a single RTT.
  */
  inline void incrPktsLostCount () { pkts_lost_counter++; }
  
  /**
  * Resets the number of packets lost in single RTT
  */
  inline void rstPktsLostCount () { pkts_lost_counter = 0; }
  
  /**
  * Returns number of packets lost in single RTT.
  * @return number of packets lost in single RTT (integer).
  */
  inline int getPktsLostCount () { return pkts_lost_counter; }
  
  /**
  * It checks how many times a packet is transmitted. If it reaches the threshold of retransmission, the packet is deleted from the all containers.
  * @param mac address of the node 
  * @param sequence number of the packet.
  * @return whether this packet can be transmitted or not. <i>TRUE</i> means its retransmission does not exit the threshold and <i>FALSE</i> otherwise.
  */
  virtual bool prepBeforeTx(int mac_addr, int seq_num);
  
  /**
  * Calcultes the time a node has to wait before transmitting another packet in a single RTT. 
  * @param mac address of the receiving node.
  * @return waiting time (double)
  */
  virtual double calcWaitTxTime(int mac_addr);
  
  /**
  * How many acknowledgements receive for the transmitting packets in a single RTT. It stores this information to calculte the next window size (i.e. number
  * of packets can be exchanged between a sender-receiver pair.
  * @param mac address of the receiving node.
  * @param number of receive acknowledgement packets in a single RTT
  */
  inline void updateTxStatus (macAddress mac_addr, int rcv_acks);
  
  /**
  * Number of packets a node can transmits to a receving node in a single RTT.
  * @return maximum number of packets a node can transmits to a receiving node (integer).
  */
  virtual int calWindowSize(macAddress mac_addr);

  /**
  * Total number of packets transmitted by a node.
  */
  inline void calTotalPktsTx() { hit_count++; total_pkts_tx = total_pkts_tx + getPktsSentIn1RTT(); }
  
  /**
  * Average number of packets transmitted in a single RTT.
  */
  inline double getAvgPktsTxIn1RTT() { double x = (double) total_pkts_tx / (double) hit_count; return x; } 
  
  ///////////// input
  int max_tx_tries; /**< Maximum number of retransmissions attempt. */
  double backoff_tuner;  /**< Tunes the backoff duration. */
  double wait_constant; /**< This fixed time is employed to componsate different time variations. */
  int max_payload;  /**< Maximum number of payload in a packet. */
  int HDR_size; /**< Size of the HDR if any */
  int ACK_size;  /**< Size of the ACK. */
  double ACK_timeout;  /**< ACK timeout for the initial packet */
  int buffer_pkts; /**< Number of packets a node can store in the container */
  double alpha_;  /**< This variable is used to tune the RTT */    
  double max_backoff_counter; /**< Maximum number of backoff it will consider while it increases the backoff exponentially */
  double listen_time; /**< A short channel sensing time */
  double guard_time; /**< A time which is used to componsate variating in timing */
  double node_speed; /**< Speed of the mobile node [m/s] */
  double var_k; /**< It is employed to decrease the window size. */
  int uwsr_debug; /**< Debuging flag. */
 /////////////////////////////

  static bool initialized; /**< It checks whether MMacUWSR protocol is initialized or not. If <i>FALSE</i> means, not initialized and if <i>TRUE</i>
			   * means it is initialized */
  static const double prop_speed; /**< Speed of the sound signal*/
  int last_sent_data_id; /**< sequence number of the latest sent packet */

  bool print_transitions; /**< Whether to print the state of the nodes */
  bool has_buffer_queue; /**< Whether the node has buffer to store data or not */

  double start_tx_time; /**< Time when a packet start transmitting */          
  double srtt;       /**< Smoothed Round Trip Time, calculated as for TCP */
  double sumrtt;       /**< Sum of RTT samples */
  double sumrtt2;      /**< Sum of (RTT^2) */
  int rttsamples;      /**< Number of RTT samples */  

  int curr_tx_rounds; /**< How many times the same packet is (re)transmitted */
  int last_data_id_rx; /**< The sequence number of last received packet */
  int recv_data_id; /**< The sequence number of the packet which is received */
  int backoff_count; /**< This variable counts how many times a node move to the backoff state */
  int pkt_tx_count; /**< Number of packets transmitted */

  Packet* curr_data_pkt; /**< Pointer of the latest selected data packet. */

  double round_trip_time; /**< RTT time */
  double wait_tx_time; /**< Waiting time before transmitting a packet in a single RTT. */
  double sumwtt; /**< Summation of total waiting time */
  int wttsamples; /**< Number of waiting time samples */
  
  int pkts_sent_1RTT; /**< Packets already sent in single RTT */
  int acks_rcv_1RTT;  /**< Number of <i>ACK</i> packets receive in single RTT. */
  int pkts_lost_counter; /**< Number of packets lost in a single RTT. */
  int window_size; /**< Maximum number of packets a node can transmit in a single RTT. */

  int txsn; /**< Sequence number of the transmitted packet. */
  
  double latest_ack_timeout; /**< Latest timeout of an acknowledgement packet. */
  int prv_mac_addr; /**< Previous mac address */

  int hit_count; /**< This is a counter which is employed to calculate average number of packets transmit in a sigle RTT */
  int total_pkts_tx; /**< Total number of packets transmit. */

  AckTimer ack_timer; /**< An object of the AckTimer class */
  BackOffTimer backoff_timer; /**< An object of the BackOffTimer class */
  ListenTimer listen_timer; /**< An object of the ListenTimer class */
  WaitTxTimer wait_tx_timer; /**< An object of the WaitTxTimer class */
  
  UWSR_REASON_STATUS last_reason; /**< Enum variable which stores the last reason why a node changes its state */
  UWSR_STATUS curr_state; /**< Enum variable. It stores the current state of a node */
  UWSR_STATUS prev_state; /**< Enum variable. It stores the previous state of a node */
  UWSR_STATUS prev_prev_state; /**< Enum variable. It stores the previous to previous state of a node */
	
  static map< UWSR_STATUS , string > status_info; /**< Container which stores all the status information */
  static map< UWSR_REASON_STATUS, string> reason_info;  /**< Container which stores all the reason information */
  static map< UWSR_PKT_TYPE, string> pkt_type_info; /**< Container which stores all the packet type information of MMacUWSR*/
   
  map< usrPair,Packet*> mapPacket; /**< Container where <i>Data</i> packets are stored */
  map< usrPair,txRounds> mapTxRounds; /**< Container where retransmission information of various packets are stored */
  map< usrPair,AckTimer> mapAckTimer; /**< Container where AckTimer objects for variuous packets are stored */
  map< usrPair,txStartTime> mapCalcAck; /**< Container where starting transmission time of various packets are stored */
  map< macAddress, rttPair> mapRTT; /**< Container where RTT between various sender-receiver pairs are stored */
  map< macAddress, txStatusPair> mapTxStatus; /**< Container which stores the number of packets transmitted in a single RTT to a receiving node and number 
					      * of acknowledgement receive among them. */

  ofstream fout; /**< An object of ofstream class */
};

#endif /* UWSR_H */
