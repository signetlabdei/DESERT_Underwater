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
 * @file   uwtwr_AAUV.h
 * @author Yuehan Jiang
 * @version 1.0.0
 *
 * @brief Provide the definition of the class <i>UWTWR</i>
 */

#ifndef UWTWR_AAUV_H
#define UWTWR_AAUV_H

#include "uwtwr_cmn_hdr.h"

#include <mmac.h>
#include <mphy.h>
#include <clmessage.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <ostream>
#include <chrono>
#include "uwsmposition.h"

#define UWPOLLING_AUV_DROP_REASON_WRONG_RECEIVER \
	"DWR" /**< Packet for another node */

/**
 * Class that represents the UWTWR MAC layer of an Active(polling) AUV.
 */
class UWTWR_AAUV : public MMac
{
public:
	/**
	 * Constructor of the UWTWR_AAUV class
	 */
	UWTWR_AAUV();
	/**
	 * Destructor of the UWTWR_AAUV class
	 */
	virtual ~UWTWR_AAUV();

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
							 (Note that <i>argv[0]</i> is the name of the
	 object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
															  successfully or
	 not.
	 */
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
	/***********************************
	 | Internal variable and functions |
	 ***********************************/

	enum UWTWR_AAUV_STATUS
	{
		UWTWR_AAUV_STATUS_IDLE = 1,
		UWTWR_AAUV_STATUS_TX_POLL,
		UWTWR_AAUV_STATUS_WAIT_ACK,
		UWTWR_AAUV_STATUS_RX_ACK
	}; /* <trx state> */

	/**< Status of the timer */
	enum UWTWR_AAUV_TIMER_STATUS {
		UWTWR_AAUV_IDLE = 1,
		UWTWR_AAUV_RUNNING,
		UWTWR_AAUV_FROZEN,
		UWTWR_AAUV_EXPIRED
	};

	/**
	 * Class that describes the timer in the AUV
	 */
	class UWTWR_AAUV_Timer : public TimerHandler
	{
	public:
		/**
		 * Constructor of the UWTWR_AAUV_Timer class
		 * @param UWTWR_AAUV* a pointer to an object of type Uwpolling_AUV
		 */
		UWTWR_AAUV_Timer(UWTWR_AAUV *m)
			: TimerHandler()
			, start_time(0.0)
			, left_duration(0.0)
			, counter(0)
			, module(m)
			, timer_status(UWTWR_AAUV_IDLE)
		{
			assert(m != NULL);
			// module = m;
		}

		/**
		 * Destructor of the UWTWR_AUV_Timer class
		 */
		virtual ~UWTWR_AAUV_Timer()
		{
		}

		/**
		 * Freeze the timer
		 */
		virtual void freeze()
		{
			assert(timer_status == UWTWR_AAUV_RUNNING);
			left_duration -= (NOW - start_time);
			if (left_duration <= 0.0){
				left_duration = module->mac2phy_delay_;
			}
			force_cancel();
			timer_status = UWTWR_AAUV_FROZEN;
		}

		/**
		 * unFreeze is used to resume the timer starting from the point where it
		 * was freezed
		 */
		virtual void unFreeze()
		{
			assert(timer_status == UWTWR_AAUV_FROZEN);
			start_time = NOW;
			assert(left_duration > 0);
			sched(left_duration);
			timer_status = UWTWR_AAUV_RUNNING;
		}

		/**
		 * stops the timer
		 */
		virtual void stop()
		{
			timer_status = UWTWR_AAUV_IDLE;
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
			timer_status = UWTWR_AAUV_RUNNING;
			resched(val);
		}

		/**
		 * Checks if the timer is IDLE
		 * @return bool <i>true</i> or <i>false</i>
		 */
		bool isIdle()
		{
			return (timer_status == UWTWR_AAUV_IDLE);
		}

		/**
		 * Checks if the timer is RUNNING
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isRunning()
		{
			return (timer_status == UWTWR_AAUV_RUNNING);
		}

		/**
		 * Checks if the timer is EXPIRED
		 * @return <i>true</i> or <i>false</i>
		 */

		bool isExpired()
		{
			return (timer_status == UWTWR_AAUV_EXPIRED);
		}

		/**
		 * Checks if the timer is FROZEN
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isFrozen()
		{
			return (timer_status == UWTWR_AAUV_FROZEN);
		}

		/**
		 * Checks if the timer is ACTIVE
		 * @return <i>true</i> or <i>false</i>
		 */
		bool isActive()
		{
			return (timer_status == UWTWR_AAUV_FROZEN ||
					timer_status == UWTWR_AAUV_RUNNING);
		}

		/**
		 * Resets the counter of the timer
		 */
		void
		resetCounter()
		{
			counter = 0;
		}

		/**
		 * Increments the counter of the timer
		 */
		void
		incrCounter()
		{
			++counter;
		}

		/**
		 * Returns the counter of the timer
		 * @return the value of the counter of the timer
		 */
		int
		getCounter()
		{
			return counter;
		}

		/**
		 * Returns the left duration of the timer
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
		UWTWR_AAUV *module; /* Pointer to an object of type UWTWR_AAUV */
		UWTWR_AAUV_TIMER_STATUS timer_status; /**< Timer status */
	};

	/**
	 * Class (inherited from UWTWR_AAUV_Timer) used to handle the timer of
	 * Ack packet
	 * When the AUV give the POLL packet to a node, he set up the timer in which
	 * the node has to transmit
	 * his packet. The duration of the timer is calculated based on the RTT
	 * between the AUV and the node and
	 * the duration of the transmission of a packet
	 */
	// maybe needed for handling turnaround time?
	class PollTimer : public UWTWR_AAUV_Timer
	{
	public:
		/**
		 * Conscructor of PollTimer class
		 */
		PollTimer(UWTWR_AAUV *m)
			: UWTWR_AAUV_Timer(m)
		{
		}

		/**
		 * Destructor of PollTimer class
		 * @param UWTWR_AAUV* Pointer of an object of type Uwpolling_AUV
		 */
		virtual ~PollTimer()
		{
		}
	
	protected:
		/**
		 * Method call when the timer expire
		 * @param Event* pointer to an object of type Event
		 */
		virtual void expire(Event *e);
	};

	class ACKTimer : public UWTWR_AAUV_Timer
	{
	public:
		/**
		 * Conscructor of ACKTimer class
		 */
		ACKTimer(UWTWR_AAUV *m)
			: UWTWR_AAUV_Timer(m)
		{
		}

		/**
		 * Destructor of ACKTimer class
		 * @param UWTWR_AAUV* Pointer of an object of type Uwpolling_AUV
		 */
		virtual ~ACKTimer()
		{
		}
	
	protected:
		/**
		 * Method call when the timer expire
		 * @param Event* pointer to an object of type Event
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
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param Packet* Pointer to an Packet object that represent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p);
	
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an Packet object that represent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p);
	
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
	 * Increases the number of Ack packets received. Used for statistical
	 * purposes
	 */
	inline void incrAckRx()
	{
		n_ack_rx++;
	}

	/**
	 * Increases the number of wrong ACK packet received. 
	 * Used for statistical purposes
	 */
	inline void incrDroppedAckPkts()
	{
		n_dropped_ack_pkts++;
	}

	/**
	 * Returns the number of ACK received during the simulation
	 * @return int n_probe_rx the number of PROBE received
	 */
	inline int getAckRx()
	{
		return n_ack_rx;
	}
	
	/**
	 * Return the number of ACK packets discarded because of wrong CRC
	 * @return int N_dropped_probe_pkts number of PROBE pkts dropped
	 */
	inline int getDroppedAckPkts()
	{
		return n_dropped_ack_pkts;
	}

	/**
	 * Refresh the state of the protocol
	 * @param UWTWR_AAUV_STATUS current state of the protcol
	 */
	virtual void refreshState(UWTWR_AAUV_STATUS state)
	{
		prev_state = curr_state;
		curr_state = state;
	}

	/**
	 * IDLE state. Each variable is resetted
	 */
	virtual void stateIdle();
	
	/**
	 * State of the protocol in which a POLL packet is initialized
	 */
	virtual void stateTxPoll();

	/**
	 * ACK TIMER is Expired. In this method the reception of ACK is disabled.
	 */
	virtual void AckTOExpired();

	/**
	 * State of the protcol in which the ACK timer is set up
	 */
	virtual void stateWaitAck();

	/**
	 * Handle the recption of an ACK from the node
	 */
	virtual void stateRxAck();

	/**
	 * Initializes the protocol at the beginning of the simulation. This method
	 * is called by
	 * a command in tcl.
	 * @param double delay
	 * @see command method
	 */
	virtual void initInfo();

	/**
	 * Transmisssion of the POLL packet
	 */
	virtual void TxPoll();

	/**
	 * Method for setting the id of node polled
	 */
	virtual void SetNodePoll();

	/**
	 * Incrase the number of POLL transmitted
	 */
	inline void incrPollTx()
	{
		n_poll_tx++;
	}

	/**
	 * Return the number of POLL packets sent during the simulation
	 * @return int n_poll_tx number of POLL packets sent during the simulation
	 */
	inline int getPollSent()
	{
		return n_poll_tx;
	}

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
	// which time to return?
	// inline unsigned long int
	// getEpoch()
	// {
	// 	return time(NULL);
	// }


	// timers
	// PollTimer poll_timer; /**< POLL Timer */
	ACKTimer ack_timer; /**< ACK Timer */

	int polling_index; /**< Index of the node that the AUV is polling */
	
	// pointer to packets
	Packet *curr_poll_packet; /**< Pointer to the current POLL packet >*/
	Packet *curr_ack_packet; /**< Pointer to the current ACK packet*/

	// packet size
	int POLL_size; /**< Size of the POLL pkt */
	int ACK_size; /**< Size of the ACK pkt */

	// input parameters via TCL
	double T_ack_timer; /** Duration of ACK_TIMER 
							T_ack_timer = T_ack + RTT + T_guard */

	// states of protocol
	UWTWR_AAUV_STATUS curr_state,
			prev_state; /* <Variable holding the status enum type> */
	
	// mapping
	static std::map<UWTWR_AAUV_STATUS, std::string> status_info;
	/** Map the UWTWR_AAUV_AUV_STATUS to the
		description of each state */

	static bool initialized; /**< Indicate if the protocol has been initialized or not */
	
	bool TxEnabled; /**< <i>true</i> if the AUV is enabled to receive POLL
						   packets, <i>false</i> otherwise */

	bool RxAckEnabled; /**< True if the ack reception is enabled */

	uint curr_node_id; /**< ID of the node polled */
	
	// statistics
	int n_ack_rx; /**< Number of ack packets received */
	int n_poll_tx; /**< Number of POLL packets sent */
	uint POLL_uid; /**< POLL Unique ID */

	int n_dropped_ack_pkts; /**<Number of ACK dropped pkts */

	int ack_enabled; /**< True if ack is enabled, false if disabled, default true*/

	//Update Range(TOF) to the previous buoy and then send in next POLL
	double diff_time;
};

#endif