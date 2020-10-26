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
 * @file   uw-csma-ca.h
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the description of CsmaCa Class
 *
 */

#ifndef CSMA_CA_H
#define CSMA_CA_H

#include <mmac.h>
#include <queue>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <time.h>

#include "uw-csma-ca-hdrs.h"

#define DROP_REASON_SNR "SNR"
#define DROP_REASON_NOTFORME "NFM"
#define DROP_REASON_NOTRIGHTSTATE "NRS"
#define DROP_REASON_GENERICERROR "ERR"

typedef enum CSMA_CA_TIMERS {
	CSMA_CA_DATA_TIMER,
	CSMA_CA_BACKOFF_TIMER,
	CSMA_CA_CTS_TIMER,
	CSMA_CA_ACK_TIMER
} csma_ca_timers_t;

enum CSMA_CA_ACK_MODES { CSMA_CA_NO_ACK_MODE = 0, CSMA_CA_ACK_MODE };

typedef enum CSMA_CA_ACK_MODES ack_modes_t;

enum CSMA_CA_STATES {
	CSMA_CA_IDLE = 0,
	CSMA_CA_BACKOFF,
	CSMA_CA_TX_RTS,
	CSMA_CA_TX_CTS,
	CSMA_CA_TX_DATA,
	CSMA_CA_WAIT_CTS,
	CSMA_CA_WAIT_DATA,
	CSMA_CA_WAIT_ACK,
	CSMA_CA_TX_ACK,
};

enum CSMA_CA_PKT_TYPE { CSMA_CA_RTS, CSMA_CA_CTS, CSMA_CA_ACK, CSMA_CA_DATA };

typedef enum CSMA_CA_PKT_TYPE csma_ca_pkt_type_t;

typedef enum CSMA_CA_STATES csma_ca_states_t;

enum log_level { CSMA_CA_ERROR = 0, CSMA_CA_WARN, CSMA_CA_INFO, CSMA_CA_DEBUG };

typedef enum log_level csma_ca_log_level_t;



string log_level_string[] = {"ERROR", "WARNING", "INFO", "DEBUG"};
/**
 * Class that describes a CsmaAloha module
 */
class CsmaCa : public MMac
{
public:
	class CsmaCaTimer : public TimerHandler
	{
	public:
		CsmaCaTimer(CsmaCa *m, csma_ca_timers_t tt)
			: TimerHandler()
		{
			module = m;
			timer_type = tt;
		}

	protected:
		virtual void expire(Event *e);
		CsmaCa *module;
		csma_ca_timers_t timer_type;
	};

	CsmaCa();
	virtual ~CsmaCa();
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
	 * (Note that <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has
	 * been dispatched successfully or not.
	 *
	 **/
	virtual int command(int argc, const char *const *argv);

protected:
	/**
	 * Receives the packet from the upper layer (e.g. IP)
	 * @param Packet* pointer to the packet received
	 *
	 */
	virtual void recvFromUpperLayers(Packet *p);
	/**
	 * Pass the packet to the PHY layer
	 * @param Packet* Pointer to an object of type Packet that represent the
	 * Packet to transmit
	 */
	virtual void Mac2PhyStartTx(Packet *p);
	/**
	 * Method called when the PHY layer finish to transmit the packet.
	 * @param Packet* Pointer to an object of type Packet that represent the
	 * Packet transmitted
	 */
	virtual void Phy2MacEndTx(const Packet *p);
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an object of type Packet that represent
	 * the Packet that is in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p);
	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param Packet* Pointer to an object of type Packet that represent
	 * the Packet received
	 */
	virtual void Phy2MacEndRx(Packet *p);
	/**
	 * Drop the packet logging the reason and incrementing the counters
	 * @param Packet* Pointer to packet to be ack_pkt_dropped
	 * @param csma_ca_pkt_type_t type of the packet dropped
	 * @param char* Reason for drop
	 */
	virtual void dropPacket(Packet *p, csma_ca_pkt_type_t type, char *reason);
	/**
	 * Get the state of the protocol
	 * @return csma_ca_state_t Current state
	 */
	inline csma_ca_states_t
	getState()
	{
		return state;
	}
	/**
	 * Update the state of the protocol
	 * @param csma_ca_state_t new state
	 */
	inline void
	updateState(csma_ca_states_t s)
	{
		previous_state = state;
		state = s;
	}
	/**
	 * Return the current log level of the protocol
	 * @return csma_ca_log_t log level
	 */
	inline csma_ca_log_level_t
	getLogLevel()
	{
		return log_level;
	}
	/**
	 * Return the size of the data packet queue
	 * @return int queue size
	 */
	inline int
	getQueueSize()
	{
		return data_q.size();
	}
	/**
	 * Return the system epoch
	 * @return time_t epoch
	 */
	inline time_t
	getEpoch()
	{
		return time(NULL);
	}
	/**
	 * Return name of the log file
	 * @return string file name
	 */
	inline string
	getLogFile()
	{
		return logfile;
	}
	/**
	 * Waiting for CTS packet
	 */
	virtual void state_Wait_CTS();
	/**
	 * Waiting for DATA packet
	 */
	virtual void state_Wait_Data();
	/**
	 * Protocol in IDLE state
	 */
	virtual void state_Idle();
	/**
	 * Backoff state
	 * @param int Transmission time sent on over-heared RTS
	 */
	virtual void state_Backoff(int tx_time);
	/**
	 * Transmit a data packet
	 * @param Packet* Pointer to data packet to be transmitted
	 */
	virtual int stateRxData(Packet *p);
	/**
	 * Reception of an RTS packet
	 * @param hdr_ca_RTS* pointer to RTS header in packet
	 * @param int source MAC of packet
	 * @param int destination MAC of the packet
	 */
	virtual int stateRxRTS(hdr_ca_RTS *rts, int mac_src, int mac_dst);
	/**
	 * Reception of an CTS packet
	 * @param hdr_ca_CTS* pointer to RTS header in packet
	 * @param int source MAC of packet
	 * @param int destination MAC of the packet
	 */
	virtual int stateRxCTS(hdr_ca_CTS *cts, int mac_src, int mac_dst);
	/**
	 * Reception of an ACK packet
	 * @param hdr_ca_ACK* pointer to RTS header in packet
	 * @param int source MAC of packet
	 * @param int destination MAC of the packet
	 */
	virtual int stateRxACK(Packet *ack);
	/**
	 * Transmission of a CTS packet
	 */
	virtual void stateTxCTS();
	/**
	 * Transmission of a DATA packet
	 */
	virtual int stateTxData();
	/**
	 * Wait for an ACK after a data transmission
	 */
	virtual void state_Wait_ACK();
	/**
	 * Tranmission of an ACK
	 * @param int destination MAC address
	 */
	virtual void stateTxAck(int mac_dst);
	/**
   * Initializes the protocol at the beginning of the simulation. This method is
   * called by
   * a command in tcl.
   */
	virtual void initializeLog();

	CsmaCaTimer backoff_timer; /**< Backoff timer */
	CsmaCaTimer cts_timer; /**< CTS timer */
	CsmaCaTimer data_timer; /**< Data timer */
	CsmaCaTimer ack_timer; /**< ACK timer */

private:
/* Log Macros */
#define LOGERR(log) printonLog(CSMA_CA_ERROR, log);
#define LOGDBG(log) printonLog(CSMA_CA_DEBUG, log);
#define LOGWRN(log) printonLog(CSMA_CA_WARN, log);
#define LOGINFO(log) printonLog(CSMA_CA_INFO, log);
	/**
	 * Print a message on log file
	 * @param csma_ca_log_level_t log level of the message
	 * @param string Actual message
	 */
	void printonLog(csma_ca_log_level_t level, string log);

	/* Private Methods */
	/**
	 * Extract data packet from queue
	 */
	void extractDataPacket();
	/**
	 * Actually transmit a DATA packet
	 */
	int txData();
	/**
	 * Actually transmit a RTS packet
	 * @param int destination MAC
	 */
	int txRTS(int mac_dest);
	/**
	 * Actually transmit a CTS packet
	 * @param int destination MAC
	 */
	int txCTS(int mac_dest);
	/**
	 * Actually transmit an ACK
	 */
	int txAck();
	/**
	 * data timer is expired
	 */
	void data_timer_fired();
	/**
	 * backoff timer is expired
	 */
	void backoff_timer_fired();
	/**
	 * CTS timer is expired
	 */
	void cts_timer_fired();
	/**
	 * ACK timer is expired
	 */
	void ack_timer_fired();
	/**
	 * Compute transmission time of a packet using known bitrate
	 * @return int tranmission time
	 */
	inline int
	computeTxTime()
	{
		return (int) ((data_size * 8) / bitrate);
	}

	/* Header build Methods */
	/**
	 * Build an RTS header
	 * @param hdr_ca_RTS** pointer to RTS header to be built
	 * @param uint8_t expected tx time of DATA packet
	 */
	void buildRTShdr(hdr_ca_RTS **rts, uint8_t tx_time);
	/**
	 * Build an CTS header
	 * @param hdr_ca_CTS** pointer to CTS header to be built
	 * @param uint8_t expected tx time of DATA packet
	 */
	void buildCTShdr(hdr_ca_CTS **cts, uint8_t tx_time);
	/**
	 * Build a generic packet
	 * @param int MAC destination
	 * @param csma_ca_pkt_type_t type of packet to build
	 * @param uint8_t expected tx time of DATA packet
	 */
	Packet *buildPacket(int mac_dest, csma_ca_pkt_type_t type, uint8_t tx_time);

	/* config from tcl */
	int max_queue_size; /**< Maximum dimension of Queue */
	int data_size; /**< Size of DATA packet */
	int bitrate; /**< Bit rate adopted */
	int backoff_delta; /**< Delta value (configurable) to be added to backoff*/
	int backoff_max; /**< Maximum value in range of backoff */
	int cts_wait_val; /**< Timer duration of CTS */
	int data_wait_val; /**< Timer duration of DATA */
	int ack_wait_val; /**< Timer duration of ACK */

	/* status variables */
	int actual_mac_data_src; /**< Source MAC of DATA packet we are handling */
	int actual_expected_tx_time; /**< Tx time of DATA packet we are handling */
	Packet *actual_data_packet; /**< Pointer to DATA packet we are handling */
	std::queue<Packet *> data_q; /**< Size of DATA packet */
	ack_modes_t ack_mode; /**< ACK mode (configurable */
	csma_ca_states_t state; /**< Current state of the protocol */
	csma_ca_states_t previous_state; /**< Previous state of the protocol */

	/* log */
	std::ofstream outLog; /**< Stdout stream of log */
	csma_ca_log_level_t log_level; /**< Current log level chosen for protocol */
	string logfile; /**< File name of log */

	/* statistics*/
	int n_rts_rx; /**< RTS received */
	int n_cts_rx; /**< CTS received */
	int data_pkt_dropped; /**< DATA packet dropped */
	int cts_pkt_dropped; /**< CTS packet dropped */
	int rts_pkt_dropped; /**< RTS packet dropped */
	int ack_pkt_dropped; /**< ACK packet dropped */
};

#endif /* CSMA_CA_H */
