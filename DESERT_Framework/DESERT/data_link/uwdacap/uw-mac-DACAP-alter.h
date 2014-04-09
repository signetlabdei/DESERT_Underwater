//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file   uw-mac-DACAP-alter.h
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the DACAP Class header description.
 *
 */

#ifndef MMAC_UW_DACAP_H
#define MMAC_UW_DACAP_H

#include <mmac.h>
#include <queue>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <mphy.h>

#define HDR_DACAP(P)      (hdr_dacap::access(P))

#define DACAP_DROP_REASON_WRONG_STATE "WST"		/**< The protocol cannot receive this kind of packet in this state */
#define DACAP_DROP_REASON_WRONG_RECEIVER "WRCV"	/**< The packet is intended for another receiver */
#define DACAP_DROP_REASON_UNKNOWN_TYPE "UPT"	/**< The type of the packet is unknown */
#define DACAP_DROP_REASON_BUFFER_FULL "DBF"		/**< The buffer of the node is full */

extern packet_t PT_DACAP;

/**
 * Definition of the header of DACAP packets 
 */
typedef struct hdr_dacap {

  double ts;					/**< packet timestamp, i.e., its generation time) */
  int sn;						/**< sequence number of this packet */  
  int dacap_type;				/**< sequence number of this packet */  
  packet_t orig_type;			/**< Original type of the packet */
  int data_sn;					/**< Sequence number of the packet */
  
  int tx_tries;					/**< Number of transmission tries of the packet */
  
  static int offset_;			/**< Required by the PacketHeaderManager. */
  /**
   * Reference to the offset variable
   */
  inline static int& offset() { return offset_; }
  /**
   * Access to this header in the Packet
   * @return Pointer to the hdr_dacap structure
   */
  inline static struct hdr_dacap* access(const Packet* p) {
    return (struct hdr_dacap*)p->access(offset_);
  }
  
} hdr_dacap;


class MMacDACAP;

/**
 * Class that represents the timers in DACAP
 */
class DACAPTimer : public TimerHandler
{
	
public:
  /**
   * Costructor of the class DACAPTimer
   * @param Pointer of a MMacDACAP object
   */
  DACAPTimer(MMacDACAP *m) : TimerHandler() { module = m; }
	
protected:
	/**
	 * Method call when the timer expire
	 * @param Event*  pointer to an object of type Event
	 */
  virtual void expire(Event *e);
  MMacDACAP* module;	/**< Pointer to an object of MMacDACAP class */
  int wait_reason;		/**< Reason for freezing the timer */
};
/**
 * Class that represents the backoff timer in DACAP 
 */
class DACAPBTimer : public TimerHandler
{
public:
	/**
	 * Costructor of the class DACAPBTimer
	 * @param Pointer of a MMacDACAP object
	 */
  DACAPBTimer(MMacDACAP *m) : TimerHandler() { module = m; }
	
protected:
  	/**
	 * Method called when the timer expire
	 * @param Event*  pointer to an object of type Event
	 */
  virtual void expire(Event *e);
  MMacDACAP* module;	/**< Pointer to an object of MMacDACAP class */
  int wait_reason;		/**< Reason for freezing the timer */
};

/**
 * Class that represents a DACAP node
 */
class MMacDACAP : public MMac
{
  friend class DACAPTimer;	/**< DACAP ACK timer */
  friend class DACAPBTimer;	/**< DACAP Backoff timer */

public: 
  /**
   * Costructor of the MMacDACAP class 
   */
  MMacDACAP();
	/**
	 * Destructor of the MMacDACAP class
	 */
  virtual ~MMacDACAP();
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * 
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
	 * 
	 */
  virtual int command(int argc, const char*const* argv);
	/**
	 * Cross-Layer messages interpreter
	 * 
	 * @param ClMessage* an instance of ClMessage that represent the message received
	 * @return <i>0</i> if successful.
	 */
  virtual int crLayCommand(ClMessage* m);
  
protected:
	/***********************************
	 | Internal variable and functions |
	 ***********************************/
	/**
	 * Receive the packet from the upper layer (e.g. IP)
	 * @param Packet* pointer to the packet received
	 *
	 */
  virtual void recvFromUpperLayers(Packet* p);

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
	 * @param const Packet* Pointer to an object of type Packet that rapresent the packet received
	 */
	virtual void Phy2MacEndRx(Packet* p);
	/**
	 * Computes the T_w parameter, that is the minimum time that the sender has to wait between the 
	 * reception of CTS packet and the effective transmission of data packet
	 *
	 * @param mode indicates whether the ACK mode or no_ACK mode is activated
	 * @param distance between the sender and the receiver
	 * @return double the value of T_w
	 *
	 */
  virtual double computeWaitTime(int mode, double distance);
	/**
	 * Computes the time that is needed to transmit a packet, implementing a CrLayMessage for asking 
	 * the PHY layer to perform this calculation
	 *
	 * @param type of the packet
	 * @return double the time needed for transmission in seconds
	 */
  virtual double computeTxTime(int type);
	/**
	 * Initializes the packet with the correct MAC source address and destination address, the correct 
	 * type and size
	 * @param Pointer to the packet that we have to initialize
	 * @param type of the packet that we are going to initialize
	 */
  virtual void initPkt( Packet* p, int pkt_type);
	/**
	 * Calculate the backoff time if the node didn't received the ACK
	 *
	 */
  virtual double getBackoffTime();
	/**
	 * Used in multihop mode
	 *
	 */
  virtual double getRecontendTime();
	/**
	 * Pass an ACK packet to transmit to the PHY layer
	 */
  virtual void txAck();
	/**
	 * Pass a DATA packet to transmit to the PHYl layer
	 *
	 */
  virtual void txData();
	/**
	 * Pass a RTS packet to transmit to the PHY layer
	 *
	 */
  virtual void txRts();
	/**
	 * Pass a CTS packet to transmit to the PHY layer
	 */
  virtual void txCts();
	/**
	 * Pass a WRN packet to transmit to the PHY layer
	 */
  virtual void txWrn();
	/**
	 *
	 * The case of reception while the protocol is in IDLE state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateIdle(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in BACKOFF state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateBackoff(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitCTS state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitCTS(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitACK state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitACK(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitDATA state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitData(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitXCts state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitXCts(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitXData state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitXData(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitXAck state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitXAck(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitXWarning state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitXWarning(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in WaitWarning state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateWaitWarning(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in SendWarning state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateSendWarning(Packet* p);
	/**
	 *
	 * The case of reception while the protocol is in RecontendWindow state is handled
	 * @param Packet* pointer to the packet is reception
	 * 
	 */
  virtual void rxStateRecontendWindow(Packet* p);
	 /**
	  * The protocol is in the IDLE state. All variables and timers are resetted
	  */
  virtual void stateIdle();
	/**
	 * The protocol is in SendRTS state. A RTS packet is prepared to be transmitted 
	 */
  virtual void stateSendRTS();
	/**
	 * The protocol is in Backoff state. The node hasn't received the ACK packet and 
	 * the backoff timer is set up for the re-transmission of the DATA packet 
	 */
  virtual void stateBackoff();
	/**
	 * The protocol is in the WaitCTS state. The node has just sent a RTS packet and now he's
	 * waiting for the answer with a CTS packet 
	 */
  virtual void stateWaitCTS();
	/**
	 * The protocol is in the DeferData state. The node has received a WRN packet or has received
	 * a CTS packet coming from other nodes. The scheduled transmission is deferred 
	 */
  virtual void stateDeferData();
	/**
	 * The protocol is in the SendData state. The handshaking phase is done. The node transmit his data packet
	 */
  virtual void stateSendData();
	/**
	 * The protocol is in the WaitACK state. 
	 */
  virtual void stateWaitACK();
	/**
	 * The protocol is in the CTSReceived state
	 */
  virtual void stateCTSReceived();
	/**
	 * The protocol is in the WaitData state. The exchanging of RTS/CTS packets is done. Now the node is waiting
	 * for the data packet 
	 */
  virtual void stateWaitData();
	/** 
	 * The protocol is in the DataReceived state. a Data Packet is received
	 * @param Packet* pointer to the data packet received 
	 */
  virtual void stateDataReceived(Packet* data_pkt);
	/**
	 * The protocol is in WaitXCts state. The node has received a RTS packet coming from a third 
	 * node that is not involved in the transmission. Now the node is waiting for the CTS coming 
	 * from the node destinatary of the RTS
	 */
  virtual void stateWaitXCts();
	/**
	 * The protocol is in WaitXData state. The node has received a CTS packet coming from a third 
	 * node that is not involved in the transmission. Now the node is waiting for the data packet
	 * coming from the node destinatary of the CTS
	 */
  virtual void stateWaitXData();
	/**
	 * The protocol is in WaitXAck state. The node has received a data packet coming from a third
	 * node that is not involved in the transmission. Now the node is waiting for the ACK packet
	 * coming from the destinatay of the data packet (if the acknowledgement is active)
	 */
  virtual void stateWaitXAck();
	/**
	 * The protocol is in WaitXWarning state. The node is waiting for a Warning coming from a third 
	 * node not involved in the transmission
	 */
  virtual void stateWaitXWarning();
	/**
	 * The protocol is in SendAck state. The node is sending an ACK after the reception of a data packet
	 * (if the acknowledgement is active)
	 */
  virtual void stateSendAck();
	/**
	 * The protocol is in SendCTS state. The node is sending CTS afeter the reception of a RTS packet
	 */
  virtual void stateSendCTS();
	/**
	 * The protocol is in WaitWarning state. The node has received a CTS or RTS packet from a third node 
	 * not involved in the transmission. The node is waiting the WRN packet from the receiver 
	 */
  virtual void stateWaitWarning();
	/**
	 * The protocol is in the SendWarning state. The node has received a CTS or RTS packet from a third node
	 * not involved in the transmission. The node is sending the WRN packet to the sender to warning 
	 * the possibility of a collision is the transmission is occuring
	 */
  virtual void stateSendWarning();
	/**
	 * The protocol is in RecontendWindow state. In multi-hop scenario after a transmission the node enter
	 * in a recontention window before return in IDLE state 
	 */
  virtual void stateRecontendWindow();
	/**
	 * Prints a file with every state change for debug purposes. 
	 * @param delay from the call of the method and the effective write process on the file
	 */
  virtual void printStateInfo(double delay = 0);
	/**
	 * Initializes the protocol at the beginning of the simulation. This method is called by
	 * a command in tcl. 
	 * @see command method
     */
  virtual void initInfo();
	/**
	 * Refresh the state of the protocol
	 * @param int current state of the protcol
	 */
  virtual void refreshState(int state) { prev_state = curr_state; curr_state = state; }
	/**
	 * Saves the MAC address of another couple of nodes that is the handshake procedure
	 * in order to access the channel
	 */
  virtual void setBackoffNodes(double node, double node_b);
	/**
	 * The backoff timer is stopped
	 */
  virtual void exitBackoff();
	/**
	 * This method erase the MAC address of the two nodes that have intiated a handshake procedure
	 * if this handshake is done
	 */
  virtual void exitSleep();
	/**
	 * Freezes the Backoff timer
	 */
  virtual void freezeBackoff();
	/**
	 * @return the size of DATA queue at the end of simulation 
	 */
  virtual int getQueueSize() {return Q.size(); }
	/**
	 * Pop the first element of the queue
	 */
  virtual void queuePop(bool flag = true) { Packet::free(Q.front()); Q.pop(); waitEndTime(flag); }
	/**
	 * Used for debug purposes. (Permit to have a "step by step" behaviour of the protocol)
	 */
  virtual void waitForUser();

  /*******************
   * stats functions *
   ******************/
  
 inline int getUpLayersDataPktsRx() {return up_data_pkts_rx;}

	/**
	 * Increases the counter of WRN packets transmitted
	 */
  virtual void incrWrnPktsTx() { wrn_pkts_tx++; }
	/**
	 * Increases the counter of WRN packets received
	 */
  virtual void incrWrnPktsRx() { wrn_pkts_rx++; }
	/**
	 * Increases the counter of RTS packets transmitted
	 */
  virtual void incrRtsPktsTx() { rts_pkts_tx++; }
	/**
	 * Increases the counter of RTS packets received
	 */
  virtual void incrRtsPktsRx() { rts_pkts_rx++; }
	/**
	 * Increases the counter of CTS packets transmitted
	 */
  virtual void incrCtsPktsTx() { cts_pkts_tx++; }
	/**
	 * Increases the counter of CTS packets received
	 */
  virtual void incrCtsPktsRx() { cts_pkts_rx++; }
	/**
	 * Increases the number of times a transmission is deferred
	 */
  virtual void incrTotalDeferTimes() {defer_times_no++; }
	/**
	 * Resets all the stats counters
	 */

  virtual void resetStats() { wrn_pkts_tx = 0; wrn_pkts_rx = 0; defer_times_no = 0; 
                              return MMac::resetStats(); }
	/**
	 * @return Number of data packets that are remained in the data queue at the end of simulation
	 */
  virtual int getRemainingPkts() { return(up_data_pkts_rx - Q.size()); }
	/**
	 * @return number of WRN packets transmitted
	 */
  virtual int getWrnPktsTx() { return wrn_pkts_tx; }
	/**
	 * @return number of WRN packets received
	 */
  virtual int getWrnPktsRx() { return wrn_pkts_rx; }
	/**
	 * @return number of RTS packets transmitted
	 */
  virtual int getRtsPktsTx() { return rts_pkts_tx; }
	/**
	 * @return number of RTS packets received
	 */
  virtual int getRtsPktsRx() { return rts_pkts_rx; }
	/**
	 * @return number of CTS packets transmitted
	 */
  virtual int getCtsPktsTx() { return cts_pkts_tx; }
	/**
	 * @return number of CTS packets received
	 */
  virtual int getCtsPktsRx() { return cts_pkts_rx; }
	/**
	 * @return number of defers occured
	 */
  virtual int getTotalDeferTimes() { return defer_times_no; }
	/**
	 * Sets the start time of the backoff timer to NOW 
	 */
  virtual void backoffStartTime() { backoff_first_start = NOW; }
	/**
	 * Sets the end time of the defer summing the duration to the actual duration of defer
	 *
	 * @param double duration of the defer time
	 */
  virtual void deferEndTime(double duration) { sum_defer_time += duration;  }
	/**
	 * calculates the End Time of the backoff timer
	 */
  virtual void backoffEndTime() { double duration = NOW - backoff_first_start; backoffSumDuration(duration); }
	/**
	 * @return Mean of all the defer times
	 */
  virtual double getMeanDeferTime() { if (defer_times_no > 0) 
                                      return( sum_defer_time / defer_times_no); 
                                      else return (0.0); }

  int wrn_pkts_tx;			/**< Counter of WRN packets TX */
  int wrn_pkts_rx;			/**< Counter of WRN packets RX */
  int rts_pkts_tx;			/**< Counter of RTS packets TX */
  int rts_pkts_rx;			/**< Counter of RTS packets RX */
  int cts_pkts_tx;			/**< Counter of CTS packets TX */
  int cts_pkts_rx;			/**< Counter of CTS packets RX */
  double defer_times_no;	/**< Counter of defer times */
  double sum_defer_time;	/**< Sum of the defer timers */

  double start_tx_time;		/**< Timestamp at which a packet is transmitted */
  double srtt;				/**< Smoothed Round Trip Time, calculated as for TCP */
  double sumrtt;			/**< sum of RTT samples */
  double sumrtt2;			/**< sum of (RTT^2) */
  int rttsamples;			/**< num of RTT samples */  


  std::queue<Packet*> Q;	/**< MAC queue used for packet scheduling */

  bool TxActive;			/**< <i> true </i> if a transmission process is occuring */
  bool RxActive;			/**< <i> true </i> if a reception process is occuring */
  bool defer_data;			/**< <i> true </i> if a node has to defer the transmission of a data packet */
  bool session_active;		/**< <i> true </i> if a session is active (i.e. an hand-shaking process is occurring */
  bool backoff_pending;		/**< <i> true </i> if the backoff-timer is active (i.e. there's a backoff process) */
  bool warning_sent;		/**< <i> true </i> if a WRN packet has been just sent */
  bool backoff_freeze_mode;	/**< <i> true </i> if the backoff-timer is freezed */
  bool print_transitions;	/**< <i> true </i> if the state-transitions of the protocol is printed on a file */
  bool has_buffer_queue;	/**< <i> true </i> if the node has a buffer queue to store data packets */
  bool multihop_mode;		/**< <i> true </i> if the multihop mode is active to simulate a multihop scenario with routing */

  int sleep_node_1;			/**< MAC address of sleep_node 1 */
  int sleep_node_2;			/**< MAC address of sleep_node 2 */
  
  int last_reason;			/**< last reason for the change of state */

  double t_min;				/**< Minimum time needed to do an hand-shaking */
  double T_W_min;			/**< Minimum Warning Time in sencods */
  double delta_D;			/**< Value (in m) that indicates how far we want the CTS propagates over the sender before initiate the data transmission process (it determines the T_w value) */
  double delta_data;		/**< Dimension difference (in bytes) among data packets (<i> 0 </i> if the packets have always the same dimension)  */
  double max_prop_delay;	/**< One way maximum propagation delay (in seconds) in the network */
  int op_mode;				/**< Indicates if the ACK_mode or no_ACK_mode is activated */
  double max_tx_tries;		/**< Maximum transmission tries for one packet before discarding the packet */
  int buffer_pkts;			/**< Dimension (in packets) of the buffer queue */
  double backoff_tuner;		/**< Multiplicative factor in the calculation of backoff */
  double wait_costant;		/**< Additive factor in the calculation of ACK timer */
  int max_payload;			/**< Dimension of the maximum allowed data paylaod in bytes */
  double max_backoff_counter;	/**< Number of times a backoff is calculated */
  double alpha_;			/**< smoothing factor used for statistics using first order IIR filter */
 
  int curr_dest_addr;		/**< MAC address of the current node involved in this session */

  int CTS_size;				/**< Size (in bytes) of the CTS packet */
  int RTS_size;				/**< Size (in bytes) of the RTS packet */
  int WRN_size;				/**< Size (in bytes) of the WRN packet */
  int HDR_size;				/**< Size (in bytes) of the header added by DACAP */
  int ACK_size;				/**< Size (in bytes) of the ACK packet */

  int curr_state;			/**< Current state of the protocol */
  int prev_state;			/**< Previous state of the protocol */

  double RTS_sent_time;		/**< Timestamp at which the RTS packet is sent */
  double session_distance;	/**< Distance between sender and receiver in the current session */

  double backoff_start_time;	/**< Backoff starting time */
  double backoff_duration;		/**< Backoff duration */
  double backoff_remaining;		/**< Backoff time remaining (if the timer is freezed) */
  double backoff_first_start;	/**< Timestamp at which the backoff timer is started for the first time (if the timer has been freezed many times */
   
  Packet* curr_data_pkt;	/**< Pointer to the current data packet */

  int last_data_id_tx;		/**< Unique ID of the last data packet transmitted */
  int last_data_id_rx;		/**< Unique ID of the last data packet received */

  static int u_pkt_id;		/**< simulation-unique packet ID */
  int u_data_id;			/**< simulation-unique DATA packet ID */

  DACAPTimer timer;			/**< ACK timer */
  DACAPBTimer backoff_timer;	/**< backoff timer */

  int txsn;					/**< serial number of the DATA packet transmitted */
  int backoff_counter;		/**< Number of times a backoff timer is set up */

  static map< int , string > info;	/**< Relationship between the state and its textual description */

  ofstream fout;			/**< Object that handle the file where the state transitions is written (if the print_transitions option is active) */
};




#endif /* MMAC_UW_DACAP_H */

