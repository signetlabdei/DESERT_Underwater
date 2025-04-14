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
 * @file   uwtwr_NDOE.h
 * @author Yuehan Jiang
 * @version 1.0.0
 *
 * @brief Provide the definition of the class <i>UWTWR</i>
 */

#ifndef UWTWR_NODE_H
#define UWTWR_NODE_H

#include "uwtwr_cmn_hdr.h"
#include "mmac.h"

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <chrono>

#define UWTWR_NODE_DROP_REASON_ERROR "DERR" /**< Packet corrupted */
#define UWTWR_NODE_DROP_REASON_UNKNOWN_TYPE "DUT" /**< Packet type unknown */
#define UWTWR_NODE_DROP_REASON_WRONG_RECEIVER "DWR" /**< The packet is for another node */
#define UWTWR_NODE_DROP_REASON_NOT_POLLED "DNP" /**< The node is not in the polling list */
#define UWTWR_NODE_DROP_REASON_WRONG_STATE "DWS" /**< The node cannot receive this kind of packet in this state */
/**
 * Class that represents the UWTWR MAC layer of a node.
 */
class UWTWR_NODE : public MMac
{
public:
	/**
	 * Constructor of the UWTWR_NODE class
	 */
	UWTWR_NODE();
	/**
	 * Destructor of the UWTWR_NODE class
	 */
	virtual ~UWTWR_NODE();
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 *<i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 *successfully or not.
	 *
	 **/
	virtual int command(int argc, const char *const *argv);
	/**
	 * Cross-Layer messages interpreter
	 *
	 * @param ClMessage* an instance of ClMessage that represent the message
	 * received
	 * @return <i>0</i> if successful.
	 */
	virtual int crLayCommand(ClMessage *m);

protected:
	/**< Variable that represents the status of the protocol machine state */
	enum UWTWR_NODE_STATUS
	{
		UWTWR_NODE_STATUS_IDLE = 1,
		UWTWR_NODE_STATUS_RX_POLL,
		UWTWR_NODE_STATUS_TX_ACK
	}; /* <trx state> */

	/**< Type of the packet */
	enum UWTWR_NODE_PKT_TYPE
	{
		UWTWR_ACK_PKT = 1,
		UWTWR_POLL_PKT,
	};

	/**< Status of the timer */
	enum UWTWR_NODE_TIMER_STATUS
	{
		UWTWR_NODE_IDLE = 1,
		UWTWR_NODE_RUNNING,
		UWTWR_NODE_FROZEN,
		UWTWR_NODE_EXPIRED
	};

	/**
	 * Class that describes the timer in the NODE
	 */
	class UWTWR_NODE_Timer : public TimerHandler
	{
	public:
		/**
		 * Constructor of the UWTWR_NODE_Timer class
		 * @param UWTWR_AUV* a pointer to an object of type UWTWR_AUV
		 */
		UWTWR_NODE_Timer(UWTWR_NODE *m)
			: TimerHandler()
			, start_time(0.0)
			, left_duration(0.0)
			, counter(0)
			, module(m)
			, timer_status(UWTWR_NODE_IDLE)
		{
			assert(m != NULL);
		}

		/**
		 * Destructor of the UWTWR_NODE_Timer class
		 */
		virtual ~UWTWR_NODE_Timer()
		{
		}

		/**
		 * Freeze the timer
		 */
		virtual void freeze()
		{
			assert(timer_status == UWTWR_NODE_RUNNING);
			left_duration -= (NOW - start_time);
			if (left_duration <= 0.0) {
				left_duration = module->mac2phy_delay_;
			}
			force_cancel();
			timer_status = UWTWR_NODE_FROZEN;
		}

		/**
		 * unFreeze is used to resume the timer starting from the point where it
		 * was freezed
		 */
		virtual void unFreeze()
		{
			assert(timer_status == UWTWR_NODE_FROZEN);
			start_time = NOW;
			assert(left_duration > 0);
			sched(left_duration);
			timer_status = UWTWR_NODE_RUNNING;
		}

		/**
		 * stops the timer
		 */
		virtual void stop()
		{
			timer_status = UWTWR_NODE_IDLE;
			force_cancel();
		}

		/**
		 * Schedules a timer
		 * @param double the duration of the timer
		 */
		virtual void schedule(double val)
		{
			start_time = NOW;
			left_duration = val;
			timer_status = UWTWR_NODE_RUNNING;
			resched(val);
		}

		/**
		 * Checks if the timer is IDLE
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isIdle()
		{
			return (timer_status == UWTWR_NODE_IDLE);
		}

		/**
		 * Checks if the timer is RUNNING
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isRunning()
		{
			return (timer_status == UWTWR_NODE_RUNNING);
		}

		/**
		 * Checks if the timer is EXPIRED
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isExpired()
		{
			return (timer_status == UWTWR_NODE_EXPIRED);
		}

		/**
		 * Checks if the timer is FROZEN
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isFrozen()
		{
			return (timer_status == UWTWR_NODE_FROZEN);
		}

		/**
		 * Checks if the timer is ACTIVE
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isActive()
		{
			return (timer_status == UWTWR_NODE_FROZEN ||
					timer_status == UWTWR_NODE_RUNNING);
		}

		/**
		 * Resets the counter of the timer
		 */
		void resetCounter()
		{
			counter = 0;
		}

		/**
		 * Increments the counter of the timer
		 */
		void incrCounter()
		{
			++counter;
		}

		/**
		 * Returns the counter of the timer
		 * @return the value of the counter of the timer
		 */
		int getCounter()
		{
			return counter;
		}

		/**
		 * Returns the counter of the timer
		 * @return the value of the counter of the timer
		 */
		double getDuration()
		{
			return left_duration;
		}

	protected:
		double start_time; /**< Start Time of the timer */
		double left_duration; /**< Left duration of the timer */
		int counter; /**< counter of the timer */
		UWTWR_NODE *module; /**< Pointer to an object of type UWTWR_AUV */
		UWTWR_NODE_TIMER_STATUS timer_status; /**< Timer status */
	};

	/**
	 * Class (inherited from UWTWR_NODE_Timer) used to handle the time of
	 * backoff of the node before transmitting the ACK packet. After receiving
	 * a POLL
	 * the node set this timer. When the timer expire, the node transmit the
	 * ACK
	 */
	class BackOffTimer : public UWTWR_NODE_Timer
	{
	public:
		/**
		 * Conscructor of BackOffTimer class
		 * @param UWTWR_NODE* pointer to an object of type UWTWR_NODE
		 */
		BackOffTimer(UWTWR_NODE *m)
			: UWTWR_NODE_Timer(m)
		{
		}

		/**
		 * Destructor of BackOffTimer class
		 */
		virtual ~BackOffTimer()
		{
		}

	protected:
		/**
		 * Method call when the timer expire
		 * @param Eevent*  pointer to an object of type Event
		 */
		virtual void expire(Event *e);
	};

	// /**
	//  * Receive the packet from the upper layer (e.g. IP)
	//  * @param Packet* pointer to the packet received
	//  *
	//  */
	// virtual void recvFromUpperLayers(Packet *p);

	/**
	 * Method called when the Mac Layer start to transmit a Packet
	 * @param Packet* Pointer to an Packet object that represent the
	 * Packet in transmission
	 */
	virtual void Mac2PhyStartTx(Packet *p);

	/**
	 * Method called when the PHY Layer finish to transmit a Packet
	 * @param Packet* Pointer to an Packet object that represent the
	 * Packet in transmission
	 */
	virtual void Phy2MacEndTx(const Packet *p);

	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an Packet object that represent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p);

	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param Packet* Pointer to an Packet object that represent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p);

	/**
	 * Inits the packet with the MAC address of the receiver and the sender,
	 * the size of the packet and the type
	 * @param UWTWR_NODE_PKT_TYPE the type of the packet
	 */
	virtual void initPkt(UWTWR_NODE_PKT_TYPE pkt_type);

	/**
	 * Initializes the protocol at the beginning of the simulation. This method
	 * is called by
	 * a command in tcl.
	 * @param double delay
	 * @see command method
	 */
	virtual void initInfo();

	/**
	 * Refresh the state of the protocol
	 * @param UWTWR_NODE_STATUS current state of the protcol
	 */
	virtual void refreshState(UWTWR_NODE_STATUS state)
	{
		prev_state = curr_state;
		curr_state = state;
	}

	/**
	 * Increment the number of sent ACK packets
	 */
	virtual void incrAckSent()
	{
		n_ack_sent++;
	}

	/**
	 * Increments the number of times that the node has been polled by the AUV
	 */
	virtual void incrTimesPolled()
	{
		n_times_polled++;
	}

	/**
	 * Increments the number of POLL packets dropped because of erroneous CRC
	 */
	inline void incrPollDropped()
	{
		n_poll_dropped++;
	}

	/**
	 * Return the number of ACK packets sent during the simulation
	 * @return int n_ack_tx number of ACK packets sent during the simulation
	 */
	inline int getAckSent()
	{
		return n_ack_sent;
	}

	/**
	 * Return the number of times the node are polled by the AUV
	 * @return int n_times_polled number of times polled
	 */
	inline int getTimesPolled()
	{
		return n_times_polled;
	}

	/**
	 * Return the number of POLL dropped by the node because of erroneous CRC
	 * @return int n_poll_dropped number of POLL dropped by the NODE
	 */
	inline int getPollDropped()
	{
		return n_poll_dropped;
	}
	
	/**
	 * Used for debug purposes. (Permit to have a "step by step" behaviour of
	 * the protocol)
	 */
	virtual void waitForUser();

	/**
	 * IDLE state. Each variable is resetted
	 */
	virtual void stateIdle();

	/**
	 * Handle the recption of an POLL from the node
	 */
	virtual void stateRxPoll();

	/**
	 * Handle the transmission of an ACK from the node
	 */
	virtual void stateTxAck();

	/**
	 * Method called by the Expire event of the timer. Here we call the
	 * stateTxAck method
	 */
	virtual void BackOffTimerExpired();

	/**
	 * The ACK Packet is sended down to the PHY layer
	 */
	virtual void TxAck();

	/**
	 * Calculate the epoch of the event. Used in sea-trial mode
	 * @return the epoch of the system
	 */
	inline unsigned long int getEpoch()
	{
	  unsigned long int timestamp =
		  (unsigned long int) (std::chrono::duration_cast<std::chrono::milliseconds>(
			  std::chrono::system_clock::now().time_since_epoch()).count() );
	  return timestamp;
	}

	/*************************
	 * input values from TCL *
	 *************************/
	uint node_id; /**< Unique Node ID */

	double T_backoff; /**< Backoff time chosen */
	
	int AUV_mac_addr; /**< MAC address of the AUV from which it receives poll*/
	uint n_ack_sent; /**< Number of ACK packets sent to the AUV */
	int n_times_polled; /**< Number of times that the node has been polled by
						   the AUV */
	int n_poll_dropped; /**< Number of POLL packet dropped */

	static bool initialized; /**< <i>true</i> if the protocol is initialized,
								<i>false</i> otherwise */
	
	bool polled; /**< <i>true</i> if the node is polled, <i>false</i> otherwise */
	
	bool RxPollEnabled; /**< <i>true</i> if the node is enabled to receive the
						   POLL, <i>false</i> otherwise */
	
	uint ACK_uid; /**< ACK Unique ID */

	// packet size
	int POLL_size; /**< Size of the POLL pkt */
	int ACK_size; /**< Size of the ACK pkt */

	Packet *curr_poll_pkt; /**< Pointer ot the current POLL packet */
	Packet *curr_ack_pkt; /**< Pointer to the current ACK packet */

	UWTWR_NODE_STATUS curr_state; /**< Current state of the protocol */
	UWTWR_NODE_STATUS prev_state; /**< Previous state of the protocol */
	
	static std::map<UWTWR_NODE::UWTWR_NODE_STATUS, std::string> status_info; /**< Textual info of the state */
	static std::map<UWTWR_NODE::UWTWR_NODE_PKT_TYPE, std::string> pkt_type_info; /**< Textual info of the type of the packet */
	
	BackOffTimer backoff_timer; /**< Backoff timer */
};

#endif