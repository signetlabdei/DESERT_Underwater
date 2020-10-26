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
//

/**
 * @file   uwpolling_SINK.h
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Class that represents a node of UWPOLLING
 *
 *
 */

#ifndef Uwpolling_HDR_SINK_H
#define Uwpolling_HDR_SINK_H

#include "uwpolling_cmn_hdr.h"
#include "mmac.h"

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <list>
#include <chrono>

#define UWPOLLING_SINK_DROP_REASON_BUFFER_FULL \
	"DBF" /**< Buffer of the node is full */
#define UWPOLLING_SINK_DROP_REASON_ERROR "DERR" /**< Packet corrupted */
#define UWPOLLING_SINK_DROP_REASON_UNKNOWN_TYPE \
	"DUT" /**< Packet type unknown */
#define UWPOLLING_SINK_DROP_REASON_WRONG_RECEIVER \
	"DWR" /**< The packet is for another node */
#define UWPOLLING_SINK_DROP_REASON_WRONG_STATE \
	"DWS" /**< The node cannot receive this kind of packet in this state */
#define UWPOLLING_SINK_DROP_REASON_PACKET_NOT_FOR_ME \
	"DPNFM" /**< Packet not for me */
#define UWPOLLING_SINK_DROP_REASON_IMINLIST_NOT_POLLED                    \
	"DPLNP" /**< The node is in the list of polled node but it is not the \
			   first */
#define UWPOLLING_SINK_DROP_REASON_NOT_POLLED \
	"DNP" /**< The node is not in the polling list */

	/**
 * Class used to represents the UWPOLLING MAC layer of a node.
 */
class Uwpolling_SINK : public MMac
{
public:
	/**
	 * Constructor of the Uwpolling_SINK class
	 */
	Uwpolling_SINK();
	/**
	 * Destructor of the Uwpolling_SINK class
	 */
	virtual ~Uwpolling_SINK();
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
	/**< Variable that rapresents the status of the protocol machine state */
	enum UWPOLLING_SINK_STATUS {
		UWPOLLING_SINK_STATUS_IDLE = 1,
		UWPOLLING_SINK_STATUS_RX_TRIGGER,
		UWPOLLING_SINK_STATUS_TX_PROBE,
		UWPOLLING_SINK_STATUS_WAIT_DATA,
		UWPOLLING_SINK_STATUS_RX_DATA,
		UWPOLLING_SINK_STATUS_TX_ACK
	};

	/**< Type of the packet */
	enum UWPOLLING_PKT_TYPE {
		UWPOLLING_DATA_PKT = 1,
		UWPOLLING_POLL_PKT,
		UWPOLLING_TRIGGER_PKT,
		UWPOLLING_PROBE_PKT,
		UWPOLLING_ACK_PKT
	};

	/**< Reason for the changing of the state */
	enum UWPOLLING_SINK_REASON {
		UWPOLLING_SINK_REASON_RX_DATA,
		UWPOLLING_SINK_REASON_RX_TRIGGER,
		UWPOLLING_SINK_REASON_PKT_ERROR,
		UWPOLLING_SINK_REASON_TX_PROBE,
		UWPOLLING_SINK_REASON_TX_ACK,
		UWPOLLING_SINK_REASON_BACKOFF_TIMER_EXPIRED,
		UWPOLLING_SINK_REASON_RX_DATA_TIMER_EXPIRED,
		UWPOLLING_SINK_REASON_NOT_SET,
		UWPOLLING_SINK_REASON_MAX_DATA_RECEIVED,
		UWPOLLING_SINK_REASON_WRONG_TYPE,
		UWPOLLING_SINK_REASON_WRONG_RECEIVER,
		UWPOLLING_SINK_REASON_WRONG_STATE,
	};

	/**< Status of the timer */
	enum UWPOLLING_TIMER_STATUS {
		UWPOLLING_IDLE = 1,
		UWPOLLING_RUNNING,
		UWPOLLING_FROZEN,
		UWPOLLING_EXPIRED
	};

	/**
	 * Class that describes the timer in the SINK
	 */
	class Uwpolling_SINK_Timer : public TimerHandler
	{
	public:
		/**
		 * Constructor of the Uwpolling_SINK_Timer class
		 * @param Uwpolling_SINK* a pointer to an object of type Uwpolling_AUV
		 */
		Uwpolling_SINK_Timer(Uwpolling_SINK *m)
			: TimerHandler()
			, start_time(0.0)
			, left_duration(0.0)
			, counter(0)
			, module(m)
			, timer_status(UWPOLLING_IDLE)
		{
			assert(m != NULL);
		}

		/**
		 * Destructor of the Uwpolling_SINK_Timer class
		 */
		virtual ~Uwpolling_SINK_Timer()
		{
		}

		/**
		 * Freeze the timer
		 */
		virtual void
		freeze()
		{
			assert(timer_status == UWPOLLING_RUNNING);
			left_duration -= (NOW - start_time);
			if (left_duration <= 0.0) {
				left_duration = module->mac2phy_delay_;
			}
			force_cancel();
			timer_status = UWPOLLING_FROZEN;
		}

		/**
		 * unFreeze is used to resume the timer starting from the point where it
		 * was freezed
		 */
		virtual void
		unFreeze()
		{
			assert(timer_status == UWPOLLING_FROZEN);
			start_time = NOW;
			assert(left_duration > 0);
			sched(left_duration);
			timer_status = UWPOLLING_RUNNING;
		}

		/**
		 * stops the timer
		 */
		virtual void
		stop()
		{
			timer_status = UWPOLLING_IDLE;
			force_cancel();
		}

		/**
		 * Schedules a timer
		 * @param double the duration of the timer
		 */
		virtual void
		schedule(double val)
		{
			start_time = NOW;
			left_duration = val;
			timer_status = UWPOLLING_RUNNING;
			resched(val);
		}

		/**
		 * Checks if the timer is IDLE
		 * @return <i>true</i> or <i>false</i>
		 */
		bool
		isIdle()
		{
			return (timer_status == UWPOLLING_IDLE);
		}

		/**
		 * Checks if the timer is RUNNING
		 * @return <i>true</i> or <i>false</i>
		 */
		bool
		isRunning()
		{
			return (timer_status == UWPOLLING_RUNNING);
		}

		/**
		 * Checks if the timer is EXPIRED
		 * @return <i>true</i> or <i>false</i>
		 */
		bool
		isExpired()
		{
			return (timer_status == UWPOLLING_EXPIRED);
		}

		/**
		 * Checks if the timer is FROZEN
		 * @return <i>true</i> or <i>false</i>
		 */
		bool
		isFrozen()
		{
			return (timer_status == UWPOLLING_FROZEN);
		}

		/**
		 * Checks if the timer is ACTIVE
		 * @return <i>true</i> or <i>false</i>
		 */
		bool
		isActive()
		{
			return (timer_status == UWPOLLING_FROZEN ||
					timer_status == UWPOLLING_RUNNING);
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
		 * Returns the counter of the timer
		 * @return the value of the counter of the timer
		 */
		double
		getDuration()
		{
			return left_duration;
		}

	protected:
		double start_time; /**< Start Time of the timer */
		double left_duration; /**< Left duration of the timer */
		int counter; /**< counter of the timer */
		Uwpolling_SINK
				*module; /**< Pointer to an object of type Uwpolling_SINK */
		UWPOLLING_TIMER_STATUS timer_status; /**< Timer status */
	};

	/**
	 * Class (inherited from Uwpolling_SINK_Timer) used to handle the time of
	 * backoff of the node before transmitting the PROBE packet. After receiving
	 * a PROBE
	 * the node set this timer. When the timer expire, the node transmit the
	 * PROBE
	 */
	class BackOffTimer : public Uwpolling_SINK_Timer
	{
	public:
		/**
		 * Conscructor of ProbeTimer class
		 * @param Uwpolling_SINK* pointer to an object of type Uwpolling_SINK
		 */
		BackOffTimer(Uwpolling_SINK *m)
			: Uwpolling_SINK_Timer(m)
		{
		}

		/**
		 * Destructor of DataTimer class
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

	/**
	 * Class (inherited from Uwpolling_SINK_Timer) used to handle the time in
	 * which the NODE wait for the DATA Timer.
	 */
	class Rx_Data_Timer : public Uwpolling_SINK_Timer
	{
	public:
		/**
		 * Conscructor of Rx_Data_Timer class
		 * @param Uwpolling_SINK* pointer to an object of type Uwpolling_SINK
		 */
		Rx_Data_Timer(Uwpolling_SINK *m)
			: Uwpolling_SINK_Timer(m)
		{
		}

		/**
		 * Destructor of ProbeTimer class
		 */
		virtual ~Rx_Data_Timer()
		{
		}

	protected:
		/**
		 * Method call when the timer expire
		 * @param Eevent*  pointer to an object of type Event
		 */
		virtual void expire(Event *e);
	};

	/**
	 * Pass the packet to the PHY layer
	 * @param Packet* Pointer to an object of type Packet that rapresent the
	 * Packet to transmit
	 */
	virtual void Mac2PhyStartTx(Packet *p);

	/**
	 * Method called when the PHY layer finish to transmit the packet.
	 * @param Packet* Pointer to an object of type Packet that rapresent the
	 * Packet transmitted
	 */
	virtual void Phy2MacEndTx(const Packet *p);

	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an object of type Packet that rapresent
	 * the Packet that is in reception
	 */
	virtual void
	Phy2MacStartRx(const Packet *p)
	{
	}

	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param const Packet* Pointer to an object of type Packet that rapresent
	 * the packet received
	 */
	virtual void Phy2MacEndRx(Packet *p);
	/**
	 * Inits the packet with the MAC address of the receiver and the sender,
	 * the size of the packet and the type
	 * @param UWPOLLING_PKT_TYPE the type of the packet
	 */
	virtual void initPkt(UWPOLLING_PKT_TYPE pkt_type);
	/**
	 * Initializes the protocol at the beginning of the simulation. This method
	 * is called by
	 * a command in tcl.
	 * @param double delay
	 * @see command method
	 */
	virtual void initInfo();

	/**
	 * Refresh the reason for the changing of the state
	 * @param UWPOLLING_SINK_REASON The reason of the change of the state
	 */
	virtual void
	refreshReason(UWPOLLING_SINK_REASON reason)
	{
		last_reason = reason;
	}

	/**
	 * Refresh the state of the protocol
	 * @param UWPOLLING_AUV_STATUS current state of the protcol
	 */
	virtual void
	refreshState(UWPOLLING_SINK_STATUS state)
	{
		prev_state = curr_state;
		curr_state = state;
	}

	/**
	 * Increment the number of sent PROBE packets
	 */
	virtual void
	incrProbeSent()
	{
		n_probe_sent++;
	}

	/**
	 * Increment the number of sent PROBE packets
	 */
	virtual void
	incrAckSent()
	{
		n_ack_sent++;
	}

	/**
	 * Increments the number of TRIGGER packets received
	 */
	inline void
	incrTriggerReceived()
	{
		n_trigger_received++;
	}

	/**
	 * Increments the number of TRIGGER packets dropped because of erroneous CRC
	 */
	inline void
	incrTriggerDropped()
	{
		n_trigger_dropped++;
	}

	/**
	 * Returns the number of PROBE packets sent during the simulation
	 * @return int n_probe_sent the number of PROBE packets sent
	 */
	inline int
	getProbeSent()
	{
		return n_probe_sent;
	}

	/**
	 * Returns the number of ACK packets sent during the simulation
	 * @return int n_ack_sent the number of ACK packets sent
	 */
	inline int
	getAckSent()
	{
		return n_ack_sent;
	}

	/**
	 * Return the number of TRIGGER packets received by the NODE
	 * @return int n_trigger_received number of TRIGGER packets received
	 */
	inline int
	getTriggerReceived()
	{
		return n_trigger_received;
	}

	/**
	 * Return the number of TRIGGER dropped by the node because of erroneous CRC
	 * @return int n_trigger_dropped number of TRIGGER dropped by the AUV
	 */
	inline int
	getTriggerDropped()
	{
		return n_trigger_dropped;
	}

	inline uint
	getDuplicatedPkt()
	{
		return duplicate_pkts;
	}
	
	inline void
	incrDuplicatedPkt()
	{
		duplicate_pkts++;
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
	 * State of the protocol in which a TRIGGER packet is received. Here we
	 * check the header of
	 * the TRIGGER and we store the information into it (i.e. the range of
	 * values into which choose
	 * randomly the backoff time
	 */
	virtual void stateRxTrigger();

	/**
	 * State of the protocol in which a PROBE packet is sent. Here we store into
	 * the PROBE header
	 * all the information needed, such as the timestamp of the most recent data
	 * packet, the number
	 * of the packets that the node wish to transmit
	 */

	virtual void stateTxProbe();
	
	/**
	 * Method called by the Expire event of the timer. Here we call the
	 * stateTxProbe method
	 */
	virtual void BackOffTimerExpired();

	/**
	 * Methods called by the Expire event of the timer. Here we disable the
	 * possibility of the node to receive DATA packets.
	 */
	virtual void RxDataTimerExpired();

	/**
	 * The backoff timer is calculated choosing randomly using a Uniform random
	 * variable and multiplying
	 * it by a backoff_tuner chosen by the user via tcl
	 */
	virtual double getBackOffTime();

	/**
	 * The ACK Packet is sent down to the PHY layer
	 */
	virtual void txAck();

	/**
	 * The PROBE Packet is sended down to the PHY layer
	 */
	virtual void TxProbe();

	/**
	 * State in which the protocol set up the timer to wait the reception of the
	 * DATA packet
	 */
	virtual void stateWaitData();

	/**
	 * State in which the DATA packet is received
	 */
	virtual void stateRxData();

	/**
	 * Add packet uids in the missing packet list
	 * @param number of missing packets to insert inthe list
	 */
	virtual void addMissPkt2List(uint16_t n_pkts);

	/**
	 * State in which the ACK is sent
	 */
	virtual void stateTxAck();

	/**
	 * Calculate the epoch of the event. Used in sea-trial mode
	 * @return the epoch of the system
	 */
	inline unsigned long int
	getEpoch()
	{
	  unsigned long int timestamp =
		  (unsigned long int) (std::chrono::duration_cast<std::chrono::milliseconds>(
			  std::chrono::system_clock::now().time_since_epoch()).count() );
	  return timestamp;
	}

	/*************************
	 * input values from TCL *
	 *************************/
	double T_data; /**< Duration of RxDataTimer */
	double T_data_gurad; /**< Guard time for RxDataTimer */
	double backoff_tuner; /**< Multiplying value to the backoff value */
	
	uint sink_id; /**< Unique Node ID */

	static bool initialized; /**< <i>true</i> if the protocol is initialized,
								<i>false</i> otherwise */

	bool RxDataEnabled; /**< <i>true</i> if the node is enabled to receive the
						   DATA, <i>false</i> otherwise */
	bool Triggered; /**< <i>true</i> if the node has correctly received a
					   TRIGGER, <i>false</i> otherwise */

	bool triggerEnabled; /**True if rx trigger is enabled.*/

	double T_in; /**< Lower bound of the range in which choose randomly the
					backoff time (sent by the AUV in the TRIGGER message) */
	double T_fin; /**< Upper bound of the range in which choose randomly the
					 backoff time (sent by the AUV in the TRIGGER message) */
	double BOffTime; /**< Backoff time chosen */

	int AUV_mac_addr; /**< MAC address of the AUV from which it receives data*/
	
	int n_probe_sent; /**< Number of PROBE packets sent to the AUV */

	int n_trigger_received; /**< Number of TRIGGER packets received */
	int n_trigger_dropped; /**< Number of TRIGGER packet dropped */

	int n_ack_sent;

	uint n_curr_rx_pkts; /**< Number of current received packets after the
								probe transmission. */
	uint16_t expected_id; /**< Expected ID of the next data packet received*/
	uint16_t last_rx; /**< ID of the last received packet*/
	bool send_ACK; /**< True if an ACK has to be sent, false otherwise*/

	std::list<uint16_t> missing_id_list; /**< List with missing packet ID */
	uint16_t expected_last_id; /**Expected Unique ID of the last packet 
							in the round*/
	uint16_t prev_expect_last_id; /*Expected last id of the previous round. 
							Needed to find which are the retx packets. */
	uint duplicate_pkts; /** Number ot duplicated packets received.*/

	bool first_rx_pkt; /** True if the packet received is the first packet in 
						the round*/

	uint PROBE_uid; /**< PROBE Unique ID */

	Packet *curr_data_pkt; /**< Pointer to the current DATA packet */
	Packet *curr_probe_pkt; /**< Pointer to the current PROBE packet */
	Packet *curr_trigger_pkt; /**< Pointer to the current TRIGGER packet */
	Packet *curr_ack_pkt; /**< Pointer to the current ACK packet */

	UWPOLLING_SINK_REASON
			last_reason; /**< Last reason to the change of state */
	UWPOLLING_SINK_STATUS curr_state; /**< Current state of the protocol */
	UWPOLLING_SINK_STATUS prev_state; /**< Previous state of the protocol */

	static map<Uwpolling_SINK::UWPOLLING_SINK_STATUS, string>
			status_info; /**< Textual info of the state */
	static map<Uwpolling_SINK::UWPOLLING_SINK_REASON, string>
			reason_info; /**< Textual info of the reason */
	static map<Uwpolling_SINK::UWPOLLING_PKT_TYPE, string>
			pkt_type_info; /**< Textual info of the type of the packet */

	BackOffTimer backoff_timer; /**< Backoff timer */
	Rx_Data_Timer rx_data_timer; /**< Receiving DATA Timer */


	std::ofstream fout; /**< Output stream for the textual file of debug */
	std::ofstream out_file_stats;
	int sea_trial; /**< Sea Trial flag: To activate if the protocol is going to
					  be tested at the sea */
	int print_stats; /**< Print protocol's statistics of the protocol */

	int n_run; /*< ID of the experiments (used during sea trial) */

	int useAdaptiveTdata; /**< True if an adaptive T_poll is used*/
	int ack_enabled; /**< True if ack is enabled, false if disabled, default true*/
	uint max_n_ack; /**< Max number of ACK that can be sent in a single round. 
					The same value has to be used in packer, if needed. */
	double T_guard; /**< Guard time added to the calculation of the data TO*/
	int max_payload; /**< Dimension of the DATA payload */
	int modem_data_bit_rate; /**< Bit rate of the modem used */
};
#endif /** Uwpolling_HDR_SINK_H */
