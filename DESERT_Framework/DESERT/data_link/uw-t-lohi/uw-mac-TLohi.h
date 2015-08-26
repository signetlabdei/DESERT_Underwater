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
 * @file   uw-mac-TLohi.h
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the declaration of uw-mac-TLohi MAC protocol
 *
 */


#ifndef MMAC_UW_TLOHI_H
#define MMAC_UW_TLOHI_H

//#include<module.h>
#include <mmac.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <utility>
#include <fstream>

#include <mphy.h>

#define HDR_TLOHI(P)      (hdr_tlohi::access(P))

#define TLOHI_DROP_REASON_WRONG_STATE "WST"
#define TLOHI_DROP_REASON_WRONG_RECEIVER "WRCV"
#define TLOHI_DROP_REASON_UNKNOWN_TYPE "UPT"
#define TLOHI_DROP_REASON_BUFFER_FULL "DBF"

extern packet_t PT_TLOHI;

static const int prop_speed = 1500; /**< typical sound speed in underwater scenario */

/**
 * Enumeration that indicates the possible state of T-LOHI MAC
 */
enum TLOHI_STATUS {
     STATE_IDLE = 1, STATE_BACKOFF, STATE_START_CONTENTION, STATE_WAIT_END_CONTENTION, STATE_TX_DATA, STATE_SLEEP, 
     STATE_TX_ACK, STATE_WAIT_ACK, STATE_DATA_RECEIVED, STATE_ACK_RECEIVED, STATE_WAIT_END_CR, 
     STATE_COUNT_CONTENDERS, STATE_RECONTEND_WINDOW, STATE_WAIT_XACK, STATE_NOT_SET
};

/**
 * Enumeration that indicates the possibile reason for the change of state in T-LOHI MAC protocol
 */
enum TLOHI_REASON_STATUS {
     REASON_TONE_TX = 1, REASON_DATA_PENDING, REASON_TONE_RX, REASON_WAIT_CR_END, REASON_NO_CONTENDERS, 
     REASON_CONTENDERS, REASON_DATA_RX, REASON_XDATA_RX, REASON_XACK_RX, REASON_DATA_TX, REASON_ACK_TX, 
     REASON_ACK_RX, REASON_BACKOFF_TIMEOUT, REASON_ACK_TIMEOUT, REASON_SLEEP_TIMEOUT, REASON_RECONTEND_END, 
     REASON_CR_END, REASON_DATA_EMPTY, REASON_XACK_TIMEOUT, REASON_NOT_SET, REASON_MAX_TX_TRIES
};

/**
 * Enumeration that indicates the possibile type of packets in T-LOHI MAC protocol 
 */
enum TLOHI_PKT_TYPE {
     TONE_PKT = 1, ACK_PKT, DATA_PKT, DATAMAX_PKT
};

/**
 * Enumeration that indicates the possible type of ACK MODE, namely: ACK_MODE or NO_ACK_MODE 
 */
enum TLOHI_ACK_MODE {
     ACK_MODE = 1, NO_ACK_MODE
};

/**
 * Enumeration that indicates the possible modality of T-LOHI protocol, namely: CONSERVATIVE_UNSYNC_MODE or AGGRESSIVE_UNSYNC_MODE
 */

enum TLOHI_MODE {
     CONSERVATIVE_UNSYNC_MODE = 1, AGGRESSIVE_UNSYNC_MODE, SYNC_MODE
};

/**
 * Enumeration that indicates the possible state of the timers, namely: RUNNING, STOPPED or FREEZED
 */

enum TIMER_STATUS {
     RUNNING = 1, STOPPED, FREEZED
};

/**
 * Struct that defines the header of T-LOHI in the packets 
 */
typedef struct hdr_tlohi {

  double ts;        		/**< packet timestamp, i.e., its generation time) */
  int sn;           		/**< sequence number of this packet */  
  TLOHI_PKT_TYPE pkt_type;      /**< T-LOHI packet type */  
  packet_t orig_type;		/**< Original type of the packet (i.e. the type defined by the upper layers) */
  int data_sn;			/**< DATA packet sequence number */

  static int offset_;		/**< Required by PacketManager Header */
  
  /**
   * Reference to the offset_ variable 
   */
  inline static int& offset() { return offset_; }
  /**
   * Method that permits to access to the header
   * @return Pointer to the hdr_tlohi struct
   */
  inline static struct hdr_tlohi* access(const Packet* p) {
    return (struct hdr_tlohi*)p->access(offset_);
  }
  
} hdr_tlohi;

/**
 * Class that represents the T-LOHI MAC protocol for a node
 */
class MMacTLOHI;

/**
 * Class that handles the timers in T-LOHI nodes
 */
class Timer : public TimerHandler
{
public:
  /**
   * Constructor of the class 
   */
  Timer(MMacTLOHI *m) : TimerHandler() { module = m; }
	
protected:
  /**
   * Event method called when the timer expire 
   * @param Event* Pointer to the event object
   */
  virtual void expire(Event *e);
  MMacTLOHI* module; /**< Pointer to the MMacTLOHI module */
};

/**
 * Class that represents the timer that describe the time needed for the 
 * DATA Phy layer to receive packet. After this duration, the PHY layer 
 * come back to sleep
 */
class DataTimer : public TimerHandler
{
public:
   /**
   * Constructor of the class 
   */
  DataTimer(MMacTLOHI *m) : TimerHandler() { module = m; }
	
protected:
  /**
   * Event method called when the timer expire 
   * @param Event* Pointer to the event object
   */ 
  virtual void expire(Event *e);
  /**< Pointer to the T-LOHI protocol of the node */
  MMacTLOHI* module;
};

/**
 * Class that represents the T-LOHI MAC protocol for a node
 */
class MMacTLOHI : public MMac
{
  /**
   * Timer class
   */
  friend class Timer;
  /**
   * DataTimer class
   */
  friend class DataTimer;

public: 
  /**
   * Constructor of the class
   */
  MMacTLOHI();
  /**
   * Destructor of the class
   */
  virtual ~MMacTLOHI();
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
 /**
  * Receives the packet from the upper layer (e.g. IP)
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
   * Computes the Transmission Time of a Packet using a CrLayer Message to ask the PHY to perform this calculation
   * @param TLOHI_PKT_TYPE type of the packet
   */
  virtual double computeTxTime(TLOHI_PKT_TYPE type);
  /**
   * Init the packet with the MAC address of the receiver and the sender,
   * the size of the packet and the type
   * @param UWPOLLING_PKT_TYPE the type of the packet
   */
  virtual void initPkt(Packet* p, TLOHI_PKT_TYPE pkt_type);
  /**
   * Performs the calculation of the Backoff timer
   */
  virtual void getBackoffTime();
  /**
   * Returns the duration of the Contention Round, where the nodes contend the channel
   * @return CR_duration
   */
  virtual double getCRduration() { return CR_duration; }
  /**
   * Checks if the PHY layer for Data is active. If not, actives it for a fixed amout of time
   */
  virtual void checkDataPhy();
  /**
   * Returns the number of Contenders in this Contention Round based on the number of tones received
   * @param double time interval at which count the contenders
   * @return the number of contenders
   */
  virtual int countContenders(double time);
  /**
   * Send the Tone to the tone PHY layer
   */
  virtual void txTone();
  /**
   * Send a DATA packet to the data PHY layer
   */
  virtual void txData();
  /**
   * Send a ACK packet of the data PHY layer
   */
  virtual void txAck();
  /**
   * Method that receives a Tone
   * @param Packet* pointer to the tone received
   */
  virtual void rxTone(Packet* p);
  /**
   * Method that receive an ACK packet
   * @param Packet* pointer to the ACK packet received
   */
  virtual void rxAck(Packet* p);
  /**
   * Receives other types of packets 
   * @param Packet* pointer to packet received 
   */
  virtual void rxElse(Packet* p);
  /**
   * IDLE state of the protocol
   */
  virtual void stateIdle();
  /**
   * The node is in Backoff state
   */
  virtual void stateBackoff();
  /**
   * The node starts the Contention sending a Tone
   */
  virtual void stateStartContention();
  /**
   * The nodes waits the end of Contention listening the channel for possible contenders
   */
  virtual void stateWaitEndContention();
  /**
   * The node counts the contenders in the CR
   */
  virtual void stateCountContenders();
  /**
   * The node has won the CR. He can now sends a DATA packet
   */
  virtual void stateTxData();
  /**
   * Simulate the sleep modality of the PHY layer of the node, while the node is waiting for the DATA
   */
  virtual void stateSleep();
  /**
   * The node has just sent a DATA packet. Now he's waiting for the ACK (in ACK_MODE modality)
   */
  virtual void stateWaitAck();
  /**
   * The node has just received a DATA packet intended for another node. Now he's waiting for the ACK from the 
   * receiver of the DATA packet (in ACK_MODE modality)
   */
  virtual void stateWaitXAck();
  /**
   * A data packet is received 
   */
  virtual void stateDataReceived(Packet* data_pkt);
  /**
   * The node transmit the ACK packet for a received DATA packet (in ACK_MODE modality)
   */
  virtual void stateTxAck();
  /**
   * Calculate the duration of the RecontendWindow
   */
  virtual void stateRecontendWindow();
  /**
   * Set the duration of the timer of the duration of the Contention Round
   */
  virtual void stateWaitCR();
  /**
   * Initializes the protocol at the beginning of the simulation. This method is called by
   * a command in tcl. 
   * @param double delay
   * @see command method
  */
  virtual void printStateInfo(double delay = 0);
  /**
   * Initializes the map between the protocol states and the textual description of these states
   */
  virtual void initInfo();
  /**
   * Initializes the information of the protocol related to the data PHY layers
   * (i.e. the duration of the timers, the length of the buffers for data packets)
   */
  virtual void initData();
  /**
   * Initializes the ID of the PHY layer for tone and Data
   */
  virtual void initMphyIds();
  /**
   * Refresh the state of the protocol
   */
  virtual void refreshState(TLOHI_STATUS state) { prev_state = curr_state; curr_state = state; }
  /** 
   * Refresh the reason for the change of state of the protocol
   */
  virtual void refreshReason(TLOHI_REASON_STATUS reason) { last_reason = reason; }
  /**
   * Reset the timer for Backoff (i.e. after the reception of an ACK packet
   */
  virtual void exitBackoff();
  /**
   * Store the MAC address of the destination for this session
   */
  virtual void setDestAddr(int mac_addr) { curr_dest_addr = mac_addr; }
  /**
   * Set the distance between transmitter and receiver for this session
   */
  virtual void setSessionDistance(double distance) { session_distance = distance; }
  /**
   * Checks if the PHY layers (for DATA and TONE) are initialized
   */
  virtual void checkPhyInit();
  /**
   * Increments the number of times a packet has been transmitted
   */
  virtual void incrCurrTxRounds() { curr_tx_rounds++; }
  /**
   * Resets the variable related to the Session (i.e. the distance between transmitter and receiver, 
   * the MAC address of the destination
   */
  virtual void resetSession();
  /**
   * Pop the first element of the data packets queue
   */
  virtual void queuePop(bool flag) { Packet::free(Q.front()); Q.pop(); waitEndTime(flag); }
  /**
   * Used for debug purposes (permits a step-by-step behaviour of the protocol
   */
  virtual void waitForUser();

  /*****************
   * stats methods *
   ****************/
  /**
   * Increments the number of the TONE sended
   */
  virtual void incrTonePktsTx() { tone_pkts_tx++; }
  /**
   * Increments the number of the TONE received
   */
  virtual void incrTonePktsRx() { tone_pkts_rx++; }
  /**
   * Increments the number of tries for the transmission of a packet
   */
  virtual void incrCurrTxTries() { curr_tx_tries++; }
  /**
   * Resets the variables related to the Statiscal computation
   */
  virtual void resetStats() { tone_pkts_tx = 0; tone_pkts_rx = 0; return MMac::resetStats(); }
  /**
   * returns the number of Packet remained in the queue at the end of the simulation
   * @return number of packet remained in the queue
   */
  virtual int getRemainingPkts() { return(up_data_pkts_rx - Q.size()); }
  /**
   * returns the number of tones transmitted in the simulation
   * @return number of tones transmitted
   */
  virtual int getTonePktsTx() { return tone_pkts_tx; }
  /**
   * returns the number of tones received in the simulation
   * @return number of tones received
   */  
  virtual int getTonePktsRx() { return tone_pkts_rx; }
  
  virtual int getUpLayersDataPktsRx() {return up_data_pkts_rx;}


  // stats //
  int tone_pkts_tx;			/**< Number of tone transmitted */
  int tone_pkts_rx;			/**< Number of tone received */
  /////

   /***************
   * input values *
   ****************/
  double max_prop_delay;		/**< Maximum propagation delay in the network */
  int max_tx_rounds;			/**< Maximum transmission round for one packet */
  double max_tx_tries;			/**< Maximum transmission tries for one packet */
  double backoff_tuner;			/**< Multiplier factor in the calculation of the backoff */
  double wait_costant;			/**< Additive factor in the calculation of ACK timer */
  int max_payload; 			/**< Maximum Data payload dimension in number of bytes */
  int HDR_size;				/**< Dimension of the header added by T-LOHI in bytes */
  int ACK_size;				/**< Size of the ACK packet in bytes */
  double sleep_timeout;			/**< Duration of the sleep phase for the PHY layer */
  double DATA_listen_timeout;		/**< Time needed for the reception of a DATA packet */
  double ACK_timeout;			/**< Time needed for the reception of a ACK packet */
  double recontend_time;		/**< Time needed for the recontention */
  double tone_data_delay;		/**< Not used anymore */
  int buffer_pkts;			/**< Buffer capacity in number of packets */

  queue<Packet*> Q;   			/**< MAC queue used for packet scheduling */

  bool TxActive;			/**< Flag that indicates if a transmission is occuring */
  bool session_active;			/**< Flag that indicates if a Session is active */
  bool backoff_pending;			/**< Flag that indicates if a backoff timer is pending */
  bool tone_transmitted;		/**< Flag that indicates if a tone has been transmitted */
  bool print_transitions;		/**< Flat that indicates if the modality in which the protocol write in a file the state transitions is active */
  static bool initialized;		/**< Flag that indicates if the protocol is initialized */
  bool mphy_ids_initialized;		/**< Flag that indicates if the IDs of the PHY layers is stored */
  bool has_buffer_queue;		/**< Flag that indicates whether the node has a buffer queue */

  TLOHI_ACK_MODE ack_mode;		/**< ACK mode of the protocol */
  TLOHI_MODE op_mode; 			/**< Operational mode of the protocol */

  int tone_phy_id; 			/**< ID of the tone PHY layer */
  int data_phy_id;			/**< ID of the DATA PHY layer */
  double CR_duration;			/**< Contention Round Duration */

  int curr_dest_addr;  			/**< Current destination MAC address */
  int last_data_id_tx;			/**< Last data packet received ID */
  int last_data_id_rx;			/**< Last data packet transmitted ID */
  Packet* curr_data_pkt;		/**< Pointer to the current data packet */
  int curr_contenders;			/**< Number of contenders in current Contention Round */
  int curr_tx_rounds;			/**< Number of current transmission round */
  int curr_tx_tries;			/**< Number of current transmission tries */

  TLOHI_REASON_STATUS last_reason;	/**< Last reason for state transition of the protocol */
  TLOHI_STATUS curr_state;		/**< Current state of the protocol */
  TLOHI_STATUS prev_state;		/**< Previous state of the protocol */

  double session_distance;		/**< Distance between transmitter and receiver in this session */

  double backoff_start_time;		/**< Timestamp of the backoff start time */
  double backoff_duration;		/**< Duration of the backoff timer */
  double backoff_remaining;		/**< Remaining time of the backoff (if the timer has been freezed) */

  static int u_pkt_id; 			/**< simulation-unique packet ID */
  int u_data_id; 			/**< simulation-unique data packet ID */
  int txsn;				/**< Transmitted data serial number */

  string tcl_modulation;		/**< Type of modulation adopted for data PHY layer */

  Timer timer; 				/**< timer for Contention Round */
  DataTimer data_phy_timer;		/**< Timer that describe the time needed to receive the packet (i.e. the data PHY layer wake up for the duration of the reception, then go to sleep again */
 

  static map< TLOHI_STATUS , string > status_info;	/**< Map between the state and the textual description of the state */
  static map< TLOHI_REASON_STATUS, string> reason_info;	/**< Map between the reason for state transitions and the description of this reason */
  static map< TLOHI_PKT_TYPE, string> pkt_type_info;	/**< Map between the pkt-type and the description of the packet */

  ofstream fout;					/**< Object that handle the output file for the state transition of the protocol */
};


#endif /* MMAC_UW_TLOHI_H */

