//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
 * @file   uw-smart-ofdm.h
 * @author Sara Falleni
 * @version 1.0.0
 *
 * @brief
 * Your can find the description of this protocol in the paper, named 
 * "A Reservation-based Adaptive MAC Protocol for OFDM Physical Layers in Underwater Networks”, 
 * In Proceedings of IEEE ICNC, Honolulu, Hawaii, USA, February 20-22, 2023.


 */

#ifndef UWSMARTOFDM_H_
#define UWSMARTOFDM_H_

#include <mmac.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <mutex> 
#include <array>
#include <mphy.h>
#include "uwofdmphy_hdr.h"
#include "uwofdmphy.h"
#include "uwofdmmac_hdr.h"
#include <clmessage.h>
#include <atomic>


#include "msg-display.h"

#define UWSMARTOFDM_DROP_REASON_WRONG_STATE "WST"
#define UWSMARTOFDM_DROP_REASON_WRONG_RECEIVER "WRCV"
#define UWSMARTOFDM_DROP_REASON_UNKNOWN_TYPE "UPT"
#define UWSMARTOFDM_DROP_REASON_BUFFER_FULL "DBF"
#define UWSMARTOFDM_DROP_REASON_ERROR "ERR"

extern packet_t PT_MMAC_ACK;
extern packet_t PT_MMAC_CTS;
extern packet_t PT_MMAC_DATA;
extern packet_t PT_MMAC_RTS; 

// Types of MAC frames that can be used
// enum MacFrameType {
// 	MF_BEACON	= 0x0008, // beaconing
// 	MF_CONTROL	= 0x0010, // used as mask for control frame
// 	MF_SLOTS	= 0x001a, // announce slots open for contention
// 	MF_RTS		= 0x001b, // request to send
// 	MF_CTS		= 0x001c, // clear to send, grant
// 	MF_ACK		= 0x001d, // acknowledgement
// 	MF_CF_END	= 0x001e, // contention free period end
// 	MF_POLL		= 0x001f, // polling
// 	MF_DATA		= 0x0020, // also used as mask for data frame
// 	MF_DATA_ACK	= 0x0021  // ack for data frames
// };

typedef int pktSeqNum;
enum criticalLevel {LOW = 0, HIGH=1};

/**
*@brief This is the base class of UWSmartOFDM protocol, which is a derived class of
*MMac.
*/

class UWSmartOFDM : public MMac
{

public:
	/**
	*Constructor of UWSmartOFDM Class
	*/
	UWSmartOFDM();

	/**
	*Destructor of UWSmartOFDM Class
	*/
	virtual ~UWSmartOFDM();

	/**
	* TCL command interpreter. It implements the following OTcl methods:
	* @param argc number of arguments in <i>argv</i>
	* @param argv array of strings which are the command parameters (Note that
	* argv[0] is the name of the object)
	* @return TCL_OK or TCL_ERROR whether the command has been dispatched
	* succesfully or not
	*/
	virtual int command(int argc, const char *const *argv);

	//Initialize subCarriers parameters inside a node, default all carriers are used
	void init_macofdm_node(int subCarNum, double carSize, int ctrl_subCar, std::string modulation);

protected:
	/**
	*Enumeration class of UWSmartOFDM status. First enumerator is given value 1.
	*/

	enum UWSMARTOFDM_STATUS {
		UWSMARTOFDM_STATE_IDLE = 1,
		UWSMARTOFDM_STATE_BACKOFF,
		UWSMARTOFDM_STATE_TX_DATA,
		UWSMARTOFDM_STATE_TX_ACK,
		UWSMARTOFDM_STATE_WAIT_ACK,
		UWSMARTOFDM_STATE_DATA_RX,
		UWSMARTOFDM_STATE_ACK_RX,
		UWSMARTOFDM_STATE_NOT_SET,
		UWSMARTOFDM_STATE_CHK_ACK_TIMEOUT,
		UWSMARTOFDM_STATE_RX_IDLE,
		UWSMARTOFDM_STATE_RX_WAIT_ACK,
		UWSMARTOFDM_STATE_CHK_BACKOFF_TIMEOUT,
		UWSMARTOFDM_STATE_RX_BACKOFF,
		UWSMARTOFDM_STATE_WRONG_PKT_RX,
		UWSMARTOFDM_STATE_TX_RTS,
		UWSMARTOFDM_STATE_RX_RTS,
		UWSMARTOFDM_STATE_WAIT_CTS,
		UWSMARTOFDM_STATE_CTRL_BACKOFF,
		UWSMARTOFDM_STATE_CHK_CTS_BACKOFF_TIMEOUT,
		UWSMARTOFDM_STATE_TX_CTS,
		UWSMARTOFDM_STATE_RX_CTS,
		UWSMARTOFDM_STATE_RX_ACTIVE,
		UWSMARTOFDM_STATE_TX_ACTIVE,
		UWSMARTOFDM_STATE_WAIT_DATA
	};

	/**
	*Enumeration class which tells the nodes the reason why it is in this state.
	*First enumerator is given value 1.
	*/
	enum UWSMARTOFDM_REASON_STATUS {
		UWSMARTOFDM_REASON_DATA_PENDING,
		UWSMARTOFDM_REASON_DATA_NOCAR,
		UWSMARTOFDM_REASON_DATA_CARASSIGNED,  
		UWSMARTOFDM_REASON_DATA_RX,
		UWSMARTOFDM_REASON_DATA_TX,
		UWSMARTOFDM_REASON_ACK_TX,
		UWSMARTOFDM_REASON_ACK_RX,
		UWSMARTOFDM_REASON_ACK_TIMEOUT,
		UWSMARTOFDM_REASON_DATA_EMPTY,
		UWSMARTOFDM_REASON_NOT_SET,
		UWSMARTOFDM_REASON_MAX_TX_TRIES,
		UWSMARTOFDM_REASON_START_RX,
		UWSMARTOFDM_REASON_PKT_NOT_FOR_ME,
		UWSMARTOFDM_REASON_WAIT_ACK_PENDING,
		UWSMARTOFDM_REASON_PKT_ERROR,
		UWSMARTOFDM_REASON_BACKOFF_TIMEOUT,
		UWSMARTOFDM_REASON_BACKOFF_PENDING,
		UWSMARTOFDM_REASON_WAIT_CTS_PENDING,
		UWSMARTOFDM_REASON_CTS_TX,
		UWSMARTOFDM_REASON_RTS_TX,
		UWSMARTOFDM_REASON_CTS_RX,
		UWSMARTOFDM_REASON_RTS_RX,
		UWSMARTOFDM_REASON_CTS_BACKOFF_TIMEOUT,
		UWSMARTOFDM_REASON_PHY_LAYER_RECEIVING,
		UWSMARTOFDM_REASON_PHY_LAYER_SENDING,
		UWSMARTOFDM_REASON_MAX_RTS_TRIES,
		UWSMARTOFDM_REASON_WAIT_DATA,
		UWSMARTOFDM_REASON_DATAT_EXPIRED, 
		UWSMARTOFDM_REASON_PREVIOUS_RTS
	};

	/**
	*Enumeration class of UWSmartOFDM packet type. First enumerator is given value
	*1. Three kinds of packets are supported by UWSmartOFDM protocol.
	*/
	enum UWSMARTOFDM_PKT_TYPE {
		UWSMARTOFDM_ACK_PKT = 1,
		UWSMARTOFDM_DATA_PKT,
		UWSMARTOFDM_DATAMAX_PKT, 
		UWSMARTOFDM_RTS_PKT,
		UWSMARTOFDM_CTS_PKT

	};

	/**
	*Enumeration class of UWSmartOFDM acknowledgement mode. First enumerator is
	*given value 1. This protocol supports both acknowledgement and
	* non-acknowledgement technique. If Acknowledgement is set, it uses
	*Stop-And-Wait ARQ technique.
	*/
	enum UWSMARTOFDM_ACK_MODES { UWSMARTOFDM_ACK_MODE = 1, UWSMARTOFDM_NO_ACK_MODE };

	/**
	*Enumeration class of UWSmartOFDM timer status. First enumerator is given value
	*1. It is employed to know the current status of a timer.
	*/
	enum UWSMARTOFDM_TIMER_STATUS {
		UWSMARTOFDM_IDLE = 1,
		UWSMARTOFDM_RUNNING,
		UWSMARTOFDM_FROZEN,
		UWSMARTOFDM_EXPIRED
	};

	/**
	* Base class of all the timer used in this protocol. This is a derived class
	* of TimerHandler.
	*/
	class UWSmartOFDMTimer : public TimerHandler
	{

	public:
		/**
		* Constructor of UWSmartOFDMTimer Class.
		*/
		UWSmartOFDMTimer(UWSmartOFDM *m)
			: TimerHandler()
			, start_time(0.0)
			, left_duration(0.0)
			, counter(0)
			, module(m)
			, timer_status(UWSMARTOFDM_IDLE)
		{
			assert(m != NULL);
		}

		/**
		* Destructor of UWSmartOFDMTimer Class.
		*/
		virtual ~UWSmartOFDMTimer()
		{
		}
		
		virtual double
		startTime()
		{
			return start_time;
		}

		/**
		* It freezes or in another word, it stops the timer for some time.
		* Suppose, for some reason we want to stop
		* a timer for some period and we want to run this timer from where it
		* was stopped. This function stops the timer and
		* save the left time duration it must run.
		*/
		virtual void
		freeze()
		{
			assert(timer_status == UWSMARTOFDM_RUNNING);
			left_duration -= (NOW - start_time);
			if (left_duration <= 0.0)
				left_duration = module->mac2phy_delay_;
			force_cancel();
			timer_status = UWSMARTOFDM_FROZEN;
		}
		/**
		* It starts the timer from where it was stopped. To run any freeze
		* timer, we can use unfreeze method.
		*/
		virtual void
		unFreeze()
		{
			assert(timer_status == UWSMARTOFDM_FROZEN);
			start_time = NOW;
			assert(left_duration > 0);
			sched(left_duration);
			timer_status = UWSMARTOFDM_RUNNING;
		}

		/**
		* Stop the timer any way.
		*/
		virtual void
		stop()
		{
			timer_status = UWSMARTOFDM_IDLE;
			force_cancel();
		}

		/**
		* Schedule the time, i.e., how long a timer is going to run.
		* @param double time
		*/
		virtual void
		schedule(double val)
		{
			start_time = NOW;
			left_duration = val;
			timer_status = UWSMARTOFDM_RUNNING;
			resched(val);
		}




		/**
		* It tells whether the timer is in Idle state or not.
		* @return 1 if the timer is idle and 0 if it is not.
		*/
		bool
		isIdle()
		{
			return (timer_status == UWSMARTOFDM_IDLE);
		}

		/**
		* This method tells whether the timer is in Running state or not.
		* @return 1 if the timer is running and 0 if it is not.
		*/
		bool
		isRunning()
		{
			return (timer_status == UWSMARTOFDM_RUNNING);
		}

		/**
		* Tells whether the timer is expired or not.
		* @return 1 if the timer expired and 0 if it is not.
		*/
		bool
		isExpired()
		{
			return (timer_status == UWSMARTOFDM_EXPIRED);
		}

		/**
		* It tells whether the timer is in freeze mode or not.
		* @return 1 if the timer is in freeze mode and 0 if it is not.
		*/
		bool
		isFrozen()
		{
			return (timer_status == UWSMARTOFDM_FROZEN);
		}

		/**
		* It tells whether the timer is active or not.
		* @return 1 if the timer is active and 0 if it is not.
		*/
		bool
		isActive()
		{
			return (timer_status == UWSMARTOFDM_FROZEN ||
					timer_status == UWSMARTOFDM_RUNNING);
		}

		/**
		* Reset the timer counter.
		*/
		void
		resetCounter()
		{
			counter = 0;
		}

		/**
		* Increment the timer counter. It helps to know the statics of the
		* timer.
		*/
		void
		incrCounter()
		{
			++counter;
		}

		/**
		* It provides, how many times a timer ran.
		* @return number of times a timer ran (int).
		*/
		int
		getCounter()
		{
			return counter;
		}

		/**
		* This methods provide the duration of a timer.
		* @return left time duration of a timer (double).
		*/
		double
		getDuration()
		{
			return left_duration;
		}

	protected:
		double start_time; /**< Start time of a timer. */

		double left_duration; /**< How long a timer is going to run more. */

		int counter; /**< How many times a timer ran. */

		UWSmartOFDM *module; /**< Pointer of UWSmartOFDM module. */

		UWSMARTOFDM_TIMER_STATUS timer_status; /**< Set the status of the timer. */
	};

	/**
	* Base class of AckTimer, which is a derived class of UWSmartOFDMTimer.
	*/
	class AckTimer : public UWSmartOFDMTimer
	{

	public:
		/**
		* Constructor of AckTimer Class.
		*/
		AckTimer(UWSmartOFDM *m)
			: UWSmartOFDMTimer(m)
		{
		}

		/**
		* Destructor of AckTimer Class.
		*/
		virtual ~AckTimer()
		{
		}

	protected:
		/**
		* What a node is going to do when a timer expire.
		* @param Event
		*/
		virtual void expire(Event *e);
	};

	/**
	* Base class of BackoffTimer. It is derived class of UWSmartOFDMTimer.
	*/
	class BackOffTimer : public UWSmartOFDMTimer
	{

	public:
		/**
		* Constructor of BackOffTimer Class.
		*/
		BackOffTimer(UWSmartOFDM *m)
			: UWSmartOFDMTimer(m)
		{
		}

		/**
		* Destructor of BackOffTimer.
		*/
		virtual ~BackOffTimer()
		{
		}

	protected:
		/**
		* What a node is going to do when a timer expire.
		* @param Event
		*/
		virtual void expire(Event *e);
	};

	/**
	* Base class of CTSTimer. It is derived class of UWSmartOFDMTimer.
	*/
	class CTSTimer : public UWSmartOFDMTimer
	{

	public:
		/**
		* Constructor of BackOffTimer Class.
		*/
		CTSTimer(UWSmartOFDM *m)
			: UWSmartOFDMTimer(m)
		{
		}

		/**
		* Destructor of BackOffTimer.
		*/
		virtual ~CTSTimer()
		{
		}

	protected:
		/**
		* What a node is going to do when a timer expire.
		* @param Event
		*/
		virtual void expire(Event *e);
	};
	/**
	* Base class of RTSTimer. It is derived class of UWSmartOFDMTimer.
	*/
	class RTSTimer : public UWSmartOFDMTimer
	{

	public:
		/**
		* Constructor of BackOffTimer Class.
		*/
		RTSTimer(UWSmartOFDM *m)
			: UWSmartOFDMTimer(m)
		{
		}

		/**
		* Destructor of BackOffTimer.
		*/
		virtual ~RTSTimer()
		{
		}

	protected:
		/**
		* What a node is going to do when a timer expire.
		* @param Event
		*/
		virtual void expire(Event *e);
	};


	/**
	* Base class of AssignmentTimer. It is derived class of UWSmartOFDMTimer.
	*/
	class AssignmentTimer : public UWSmartOFDMTimer
	{

	public:
		/**
		* Constructor of BackOffTimer Class.
		*/
		AssignmentTimer(UWSmartOFDM *m)
			: UWSmartOFDMTimer(m)
		{
		}

		/**
		* Destructor of BackOffTimer.
		*/
		virtual ~AssignmentTimer()
		{
		}

	protected:
		/**
		* What a node is going to do when a timer expire.
		* @param Event
		*/
		virtual void expire(Event *e);
	};

		/**
	* Base class of AssignmentValidTimer. It is derived class of UWSmartOFDMTimer.
	* changes the value of car_assigned
	*/
	class AssignmentValidTimer : public UWSmartOFDMTimer
	{

	public:
		/**
		* Constructor of BackOffTimer Class.
		*/
		AssignmentValidTimer(UWSmartOFDM *m)
			: UWSmartOFDMTimer(m)
		{
		}

		/**
		* Destructor of BackOffTimer.
		*/
		virtual ~AssignmentValidTimer()
		{
		}

	protected:
		/**
		* What a node is going to do when a timer expire.
		* @param Event
		*/
		virtual void expire(Event *e);
	};
	
	/* Base class of DATATimer. It is derived class of UWSmartOFDMTimer.
	* waits for DATA after CTS
	*/
	class DATATimer : public UWSmartOFDMTimer
	{

	public:
		/**
		* Constructor of BackOffTimer Class.
		*/
		DATATimer(UWSmartOFDM *m)
			: UWSmartOFDMTimer(m)
		{
		}

		/**
		* Destructor of BackOffTimer.
		*/
		virtual ~DATATimer()
		{
		}

	protected:
		/**
		* What a node is going to do when a timer expire.
		* @param Event
		*/
		virtual void expire(Event *e);
	};

	/**
	* This function receives the packet from upper layer and save it in the
	* queue.
	* @param Packet pointer
	*/
	virtual void recvFromUpperLayers(Packet *p);

	/**
	* It informs that a packet transmission started.
	* @param Packet pointer
	*/
	virtual void Mac2PhyStartTx(Packet *p);

	/**
	* It infroms that a packet transmission end.
	* @param Packet pointer
	*/
	virtual void Phy2MacEndTx(const Packet *p);

	/**
	* PHY layer informs the MAC layer that it is receiving a packet.
	* @Param Packet pointer (constant)
	*/
	virtual void Phy2MacStartRx(const Packet *p);

	/**
	* PHY layer informs the MAC layer that the reception of the packet is over.
	* @param Packet pointer.
	*/
	virtual void Phy2MacEndRx(Packet *p);

	/**
	* Compute the transmission time of a packet. It uses a cross-layer message
	* to calculate the duration of that packet.
	* @param type is a UWSMARTOFDM_PKT_TYPE
	* @return tranmission time of a packet which is a double data type.
	*/
	virtual double computeTxTime(UWSMARTOFDM_PKT_TYPE type);

	/**
	* This method, initialize the packet. If the packet is received from the
	* upper layer, it adds the header (if any). In case of UWSmartOFDM with ARQ
	* technique, it set the fields of ACK packet.
	* @param Packet pointer P. The packet can be <i>Data</i> packet or
	* <i>ACK</i> packet.
	* @param pkt_type is an UWSMARTOFDM_PKT_TYPE. Packet can be either <i>Data</i>
	* packet or <i>ACK</i> packet.
	* @param dest_addr is a integer data type. It is initialized as 0.
	*/
	virtual void initPkt(
			Packet *p, UWSMARTOFDM_PKT_TYPE pkt_type, int dest_addr = 0);

	/**
	* This function calculates the backoff duration and return the backoff
	* time.It employs the exponential backoff algorithm.
	* @return backoff duration which is a double data type.
	*/
	virtual double getBackoffTime();

	/**
	* This method transmits <i>Data</i> packets from MAC layer to PHY layer.
	*/
	virtual void txData();

	/**
	* This methods transmits <i>ACK</i> packet from MAC layer to PHY layer.
	* @param dest_addr which is an integer data type.
	*/
	virtual void txAck(int dest_addr);

	/**
	* This methods transmits <i>RTS</i> packet from MAC layer to PHY layer.
	* @param dest_addr which is an integer data type.
	*/
	virtual void txRTS();

	/**
	* This methods transmits <i>CTS</i> packet from MAC layer to PHY layer.
	* @param dest_addr which is an integer data type.
	* @param rcv_car carriers received in the RTS
	* @param bytesToSend bytes requested by RTS
	*/
	virtual void txCTS(int dest_addr, int* rcv_car, int bytesToSend);

	/**
	* Node is in Idle state. It only changes its state if it has packet(s) to
	* transmit or it receives a packet.
	*/
	virtual void stateIdle();

	/**
	* Node is in sendRTS state. If a packet needs to be send and the carriers 
	* are not yet assigned to that node, an RTS is sent. 
	*/
	virtual void stateSendRTS();

	/**
	* Node is in sendRTS state. It has sent a CTS to the node. 
	*/
	virtual void stateSendCTS(Packet* p);

	/**
	* Node is in waitCTS state. Waits for a CTS from coordinator. 
	*/
	virtual void stateWaitCTS();

	/**
	* Node is in BackoffCTS state. Waits for a CTS from coordinator. 
	*/
	virtual void stateBackoffCTS();

	/**
	* If a node start receiving a packet in Idle state.
	*/
	virtual void stateRxIdle();

	/**
	* If a node has packet to transmits. In such case, it moves from Idle state
	* to data transmits state.
	*/
	virtual void stateTxData();

	/**
	* If the protocl uses ARQ technique, in that case, after receiving a
	* <i>Data</i> packet the node sends an <i>ACK</i> packet.
	*/
	virtual void stateTxAck(int dest_addr);

	/**
	* After transmitting a <i>Data</i> packet, a node waits for the <i>ACK</i>
	* packet.
	*/
	virtual void stateWaitAck();

	/**
	* If a node receives any packet while it was waiting for <i>ACK</i> packet,
	* it moves to this state. The packet it is receiving can be a <i>Data</i>
	* packet
	* from another node or <i>ACK</i> packet.
	*/
	virtual void stateRxWaitAck();

	/**
	* If <i>ACK</i> packet is not received within the acknowledgement expire
	* time.
	*/
	virtual void stateBackoff(double bt=0);

	/**
	* If a node start receiving a packet when it is in backoff state. The node
	* first freeze (or another word, hold) the backoff timer and start receiving
	* the packet.
	*/
	virtual void stateRxBackoff();

	/**
	* It checks whether the ack timer is already expired while it was busy with
	* other activities.
	*/
	virtual void stateCheckAckExpired();

	/**
	* It checks whether the backoff timer is already expired while it was busy
	* with other activities.
	*/
	virtual void stateCheckBackoffExpired();

	/**
	* It checks whether the CTS backoff timer is already expired while it was busy
	* with other activities.
	*/
	virtual void stateCheckCTSBackoffExpired();

	/**
	* It process the packet which is received. After receiving a packet it
	* changes it states according to the previously stored status information.
	* @param <i>Data</i> packet pointer
	*/
	virtual void stateRxData(Packet *p);

	/**
	* The node comes to this state if it receives an <i>ACK</i> packet. After
	* receiving an <i>ACK</i> packet it changes it states according
	* to the previously stored status information.
	*/
	virtual void stateRxAck(Packet *p);

	/* The node comes to this state if it receives an <i>RTS</i> packet */
	virtual void stateRxRTS(Packet *p);

	/* The node comes to this state if it receives an <i>CTS</i> packet */
	virtual void stateRxCTS(Packet *p);

	/* Node comes here after sending a CTS. Waits for data before doing other stuff */
	virtual void stateWaitData(double t);

	/**
	* It stops the backoff timer.
	*/
	virtual void exitBackoff();

	/**
	* It stops the CTS backoff timer.
	*/
	virtual void exitCTSBackoff();

	/**
	* This methods print the state information of the nodes.
	* @param delay is a double data type.
	*/
	virtual void printStateInfo(double delay = 0);

	/**
	* This function is used to initialize the UWSmartOFDM protocol.
	*/
	virtual void initInfo();

	/**
	* Refreshes the states of the node. The node save the information of three
	* states, they are: previous to previous state, previous state and
	* current state of the node.
	* @param state which is an UWSMARTOFDM_STATUS type.
	*/
	virtual void
	refreshState(UWSMARTOFDM_STATUS state)
	{
		prev_prev_state = prev_state;
		prev_state = curr_state;
		curr_state = state;
	}

	/**
	* To know the reason why a node is in this current state.
	* @param reason is an UWSMARTOFDM_REASON_STATUS type.
	*/
	virtual void
	refreshReason(UWSMARTOFDM_REASON_STATUS reason)
	{
		last_reason = reason;
	}

	/**
	* Increments the current transmission round of a packet. It keeps track of
	* the number of retransmition of a packet.
	*/
	virtual void
	incrCurrTxRounds()
	{
		curr_tx_rounds++;
	}

	/**
	* If a node is going to transmit a new packet, it resets the tx counter.
	*/
	virtual void
	resetCurrTxRounds()
	{
		curr_tx_rounds = 0;
	}

	/**
	* Update the Round Trip Time (RTT) which is necessary to compute the
	* acknowledgement duration as well as backoff duration.
	* @param rtt is a double data type.
	*/
	virtual void updateRTT(double rtt);

	/**
	* This method is used to get the average RTT over all the receives RTT.
	* @return average RTT time which is a double data type.
	*/
	virtual double
	getRTT()
	{
		return (rttsamples > 0) ? sumrtt / rttsamples : 0;
	}

	/**
	* This method is used to get the total number of sent RTS messages.
	*/
	virtual double
	getRTSsent()
	{
		return RTSsent;
	}

	/**
	* This method is used to get the total number of sent CTS messages.
	*/
	virtual double
	getCTSsent()
	{
		return CTSsent;
	}

	/**
	* Like updateRTT() function.
	*/
	virtual void updateAckTimeout(double rtt);

	/**
	* It updates the sequence number of the last data packet rx.
	* @param id is an integer data type.
	*/
	virtual void
	updateLastDataIdRx(int id)
	{
		last_data_id_rx = id;
	}
	virtual void waitForUser();

	/**
	* This method is used to get the sequence number from a packet.
	* @param packet pointer
	* @return it returns sequence number which is an integer data type.
	*/
	inline int
	getPktSeqNum(Packet *p)
	{
		int seq_num;
		hdr_cmn *ch = hdr_cmn::access(p);
		seq_num = ch->uid();
		return seq_num;
	}

	/**
	* A node receives packet(s) from upper layer and store them in the
	* container.
	* @param packet pointer
	*/
	inline void
	putPktInQueue(Packet *p)
	{
		mapPacket.insert(pair<pktSeqNum, Packet *>(getPktSeqNum(p), p));
	}

	/**
	* It erases the packet from the container.
	* @param seq_num which is an integer data type.
	*/
	inline void
	eraseItemFromPktQueue(int seq_num)
	{
		map<pktSeqNum, Packet *>::iterator it_p;
		it_p = mapPacket.find(seq_num);
		if (it_p != mapPacket.end()){
		Packet::free((*it_p).second);
		mapPacket.erase((*it_p).first);

		}
		else 
			std::cout << "ATTENTION PACKET NOT FOUND IN PKTQUEUE" << std::endl;
	}

	/**
	* Put acknowledgement timer in the container.
	* @param seq_num which is an integer data type.
	*/
	inline void
	putAckTimerInMap(int seq_num)
	{
		mapAckTimer.insert(pair<pktSeqNum, AckTimer>(seq_num, ack_timer));
	}

	/**
	* Erase an item from acknowledgement stored container.
	* @param seq_num which is an integer data type.
	*/
	inline void
	eraseItemFrommapAckTimer(int seq_num)
	{
		map<pktSeqNum, AckTimer>::iterator it_a;
		it_a = mapAckTimer.find(seq_num);
		mapAckTimer.erase((*it_a).first);
	}

	/**
	* Number of packets which MAC layer receives form upper layer(s) but were
	* not transmitted.
	* @return an integer value.
	*/
	virtual int
	getRemainingPkts()
	{
		return (up_data_pkts_rx - mapPacket.size());
	}

	/**
	* Increment the number of <i>Data</i> packet receive for the upper layer.
	*/
	virtual void
	incrUpperDataRx()
	{
		up_data_pkts_rx++;
	}

	inline int
	getUpLayersDataPktsRx()
	{
		return up_data_pkts_rx;
	}

	//returns the number of high priority packets sent 
	inline int
	getHighPrioPktsSent()
	{
		return high_prio_pkt_sent;
	}

	//returns the number of high priority packets received
	inline int
	getHighPrioPktsRecv()
	{
		return high_prio_pkt_recv;
	}

	//returns the current free carriers that can be given to a node
	/**
	* returns the current free carriers that can be given to a node
	* top and bottom give the range 
	* avoid_top and avoid_bottom if a subrange must be avoided, 0 otherwise
	*/
	void carToBeUsed(criticalLevel c, int& top, int& bottom, int& avoid_top, int& avoid_bottom);

	/**
	* Returns max 5 carriers that are free in the next n time slots from occupancy table
	* @param n is for how many timeslots I want them free, 
	* @param freeC returned carriers 
	* @return number of free carriers
	*/
	int pickFreeCarriers(int* freeC);

	/**
	* Returns free Carriers matching between itself and the sender
	* to be used when an RTS is received to find carriers to include into CTS  
	* @param myFree is node's free carriers 
	* @param otherFree is other node's free carriers 
	* @param matching is the match between myFree and otherFree 
	* @return the number of matching carriers
	*/ 
	int matchCarriers(int* myFree, int* otherFree, int* matching);

	/**
	* updates occupancy table  
	* @param busyCar carriers to update in occupancy table  
	* @param ntslots number of timeslots that will be reserved   
	*/ 
	void updateOccupancy(int* busyCar, int ntslots);

	/**
	* clears the carriers used in the past
	* moves the index for the current time to the next slot    
	*/ 
	void clearOccTable();

	/**
	* resets value of car_assigned to FALSE
	*/ 
	void resetAssignment();

	/**
	* prints on terminal the table   
	*/ 
	void printOccTable();

	// Sends a msg to Mac to tell if tx is happening 
	void Mac2PhySetTxBusy(int, int get=0);

	//True if the conditions for a batch sending are verified, false otherwise
	bool batchSending();

	//add to the list of carriers that shouldn't be used
	inline void 
	addInvalidCarriers(int c){
		nouse_carriers.push_back(c);
	}

	//Remove Invalid Carrier c from nouse_carriers - if it's back to valid
	void removeInvalidCarrier(int c);

	//update InterfTable with new unrecognized packet
	//also cleans the old samples 
	void updateInterfTable(Packet* p);

	///////////// input
	int max_tx_tries; /**< Maximum number of retransmissions attempt. */
	double wait_constant; /**< This fixed time is used to componsate different
							 time variations. */
	double backoff_tuner; /**< Tunes the backoff duration. */
	int max_payload; /**< Maximum number of payload in a packet. */
	int HDR_size; /**< Size of the HDR if any */
	int ACK_size; /**< Size of the ACK, if the node uses ARQ technique */
	int RTS_size; /**< Size of the ACK, if the node uses SMARTOFDM */
	int CTS_size; /**< Size of the ACK, if the node uses SMARTOFDM */
	int DATA_size; /**< Size of the DATA packet, if the node uses SMARTOFDM */
	double ACK_timeout; /**< ACK timeout for the initial packet */
	double bitrateCar; // Bitrate per carrier
	int buffer_pkts; /**< Number of packets a node can store in the container */
	double alpha_; /**< This variable is used to tune the RTT */
	double max_backoff_counter; /**< Maximum number of backoff it will consider
								   while it increases the backoff exponentially
								   */
	int uwsmartofdm_debug; /**< Debuging Flag */
	/////////////////////////////

	///////////// OFDM PARAMS /////
	std::vector<string> mac_carMod; // Vector with carriers modulations
	std::vector<int> mac_carVec; // Vector with carriers used/not used 
	std::vector<char> mac_prioVec; // Vector with node's priorities H/L 
	int mac_ncarriers; 				// number of subcarriers
	double mac_carrierSize; 
	int ctrl_car; 					//number of control subcarriers
	int data_car; 					// number of carriers used for data 
	bool coordinator; 				// if this node is a coordinator
	criticalLevel cLevel = LOW;
	bool car_assigned = 0;
	int nodeNum;					//number of nodes in the network
	std::atomic<int> current_rcvs;	// Number of current Receptions
	int current_macDA;

	int oTableIndex; 		// index in the occupancy table of the current point
	int max_aval_car; //number of free carriers that tx can tell rx 


	static bool initialized; /**< It checks whether UWSmartOFDM protocol is
				 * initialized or not. If <i>FALSE</i> means, not initialized
				 * and if <i>TRUE</i>
				 * means it is initialized */

	int last_sent_data_id; /**< sequence number of the last sent packet */

	bool print_transitions; /**< Whether to print the state of the nodes */
	bool
			has_buffer_queue; /**< Whether the node has buffer to store data or
								 not */

	double start_tx_time; /**< Time when a packet start transmitting */
	double srtt; /**< Smoothed Round Trip Time, calculated as for TCP */
	double sumrtt; /**< Sum of RTT samples */
	double sumrtt2; /**< Sum of (RTT^2) */
	int rttsamples; /**< Number of RTT samples */
	int RTSsent = 0;
	int CTSsent = 0;

	int curr_tx_rounds; /**< How many times a packet is transmitted */
	int last_data_id_rx; /**< The sequence number of last received packet */
	int recv_data_id; /**< The sequence number of the packet which is received
						 */
	int high_prio_pkt_sent = 0;
	int high_prio_pkt_recv = 0;

	double nextAssignment;

	Packet *curr_data_pkt; /**< Pointer of the latest selected data packet. */

	int txsn; /**< Sequence number of the packet which is transmitted */

	static const double prop_speed; /**< Speed of the sound signal*/

	AckTimer ack_timer; /**< An object of the AckTimer class */
	BackOffTimer backoff_timer; /**< An object of the backoff timer class */
	CTSTimer CTS_timer; /**< An object of the CTS timer class */
	RTSTimer RTS_timer; /**< An object of the RTS timer class */
	DATATimer DATA_timer; /**< An object of the DATA timer class */

	AssignmentTimer assignment_timer; /**< An object of the assignment timer class */

	AssignmentValidTimer assignment_valid_timer; /**< An object of the assignment valid timer class */

	UWSMARTOFDM_REASON_STATUS
			last_reason; /**< Enum variable which stores the last reason why a
							node changes its state */
	UWSMARTOFDM_STATUS curr_state; /**< Enum variable. It stores the current state
								  of a node */
	UWSMARTOFDM_STATUS
			prev_state; /**< Enum variable. It stores the previous state of a
						   node */
	UWSMARTOFDM_STATUS prev_prev_state; /**< Enum variable. It stores the previous
									   to previous state of a node */

	UWSMARTOFDM_ACK_MODES
			ack_mode; /**< Enum variable. It tells the node whether to use ARQ
						 technique or not. */
	static map<UWSMARTOFDM_STATUS, string> status_info; /**< Container which stores
													   all the status
													   information */
	static map<UWSMARTOFDM_REASON_STATUS, string>
			reason_info; /**< Container which stores all the reason information
							*/
	static map<UWSMARTOFDM_PKT_TYPE, string>
			pkt_type_info; /**< Container which stores all the packet type
							  information of UWSmartOFDM*/
	static map<criticalLevel, string>
			clevel_info; /**< Container which stores all the packet type
							  information of UWSmartOFDM*/

	map<pktSeqNum, Packet *>
			mapPacket; /**< Container where <i>Data</i> packets are stored */
	map<pktSeqNum, AckTimer> mapAckTimer; /**< Container where acknowledgement
											 timer(s) is stored */

	ofstream fout; /**< An object of ofstream class */
	MsgDisplayer msgDisp;

	std::vector<std::vector<int>> occupancy_table; //table with future usage of subcarriers
	std::mutex otabmtx; // mutex for occupancy table 
	int timeslots; // how many timeslots will be kept 
	double timeslot_length; // length in seconds of each timeslot
	int max_car_reserved; // max num of carriers reserved for each exchange
	int req_tslots; // how many tslots reserved for an interaction

	int current_timeslot; //index in the occupancy table of the current timeslot

	int max_rts_tries; 	// max num of times that rts will be sent for a specific packet
	int curr_rts_tries; // current num of rts sent for a specific packet
	int max_burst_size;		// number of packets that will be sent consecutively
	int curr_pkt_batch;
	bool batch_sending; // true if sending a batch, false otherwise

	bool RTSvalid; //true if allowed to send an RTS

	double nextFreeTime; //When in time the bw will become free
	bool ackToSend; //Packets have been received, need to send ack
	Packet* pkt_to_ack; 
	int waitPkt;
	//variables to save heard RTS
	Packet* nextRTS;
	double nextRTSts;
	bool fullBand;
	std::vector<int> nouse_carriers;
	std::vector<std::vector<double>> interf_table; //table with future usage of subcarriers
};

#endif /* UWUWSMARTOFDM_H_ */
