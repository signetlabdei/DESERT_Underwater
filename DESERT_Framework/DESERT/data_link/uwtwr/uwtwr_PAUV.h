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
 * @file   uwtwr_PAUV.h
 * @author Yuehan Jiang
 * @version 1.0.0
 *
 * @brief Provide the definition of the class <i>UWTWR</i>
 */

#ifndef UWTWR_PAUV_H
#define UWTWR_PAUV_H

#include "uwtwr_cmn_hdr.h"
#include "mmac.h"

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <list>
#include <chrono>

/**
 * Class that represents the UWTWR MAC layer of a node.
 */
class UWTWR_PAUV : public MMac
{
public:
	/**
	 * Constructor of the UWTWR_PAUV class
	 */
	UWTWR_PAUV();
	/**
	 * Destructor of the UWTWR_PAUV class
	 */
	virtual ~UWTWR_PAUV();
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
	virtual int crLayCommand(ClMessage *M);

protected:
	/**< Variable that represents the status of the protocol machine state */
	enum UWTWR_PAUV_STATUS
	{
		UWTWR_PAUV_STATUS_IDLE = 1,
		UWTWR_PAUV_STATUS_RX_POLL,
		UWTWR_PAUV_STATUS_RX_ACK
	};

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
	 * Initializes the protocol at the beginning of the simulation. This method
	 * is called by
	 * a command in tcl.
	 * @param double delay
	 * @see command method
	 */
	virtual void initInfo();

	/**
	 * Refresh the state of the protocol
	 * @param UWTWR_PAUV_STATUS current state of the protcol
	 */
	virtual void refreshState(UWTWR_PAUV_STATUS state)
	{
		prev_state = curr_state;
		curr_state = state;
	}

	/**
	 * Increases the number of Poll packets received. Used for statistical
	 * purposes
	 */
	inline void incrPollRx()
	{
		n_poll_rx++;
	}

	/**
	 * Increases the number of Ack packets received. Used for statistical
	 * purposes
	 */
	inline void incrAckRx()
	{
		n_ack_rx++;
	}

	/**
	 * Returns the number of Poll received during the simulation
	 * @return int n_ppll_rx the number of PROBE received
	 */
	inline int getPollRx()
	{
		return n_poll_rx;
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
	 * Increments the number of POLL packets dropped because of erroneous CRC
	 */
	inline void incrPollDropped()
	{
		n_poll_dropped++;
	}

	/**
	 * Increases the number of wrong ACK packet received. 
	 * Used for statistical purposes
	 */
	inline void incrDroppedAckPkts()
	{
		n_ack_dropped++;
	}

	/**
	 * Return the number of POLL dropped by the PAUV because of erroneous CRC
	 * @return itn n_poll_dropped number of POLL dropped
	 */
	inline int getPollDropped()
	{
		return n_poll_dropped;
	}

	/**
	 * Return the number of ACK packets discarded because of wrong CRC
	 * @return int N_dropped_probe_pkts number of PROBE pkts dropped
	 */
	inline int getDroppedAckPkts()
	{
		return n_ack_dropped;
	}

	/**
	 * IDLE state. Each variable is resetted
	 */
	virtual void stateIdle();

	/**
	 * Handle the recption of an POLL from the PAUV
	 */
	virtual void stateRxPoll();

	/**
	 * Handle the recption of an ack from the PAUV
	 */
	virtual void stateRxAck();

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
	static bool initialized; /**< <i>true</i> if the protocol is initialized,
								<i>false</i> otherwise */

	UWTWR_PAUV_STATUS curr_state; /**< Current state of the protocol */
	UWTWR_PAUV_STATUS prev_state; /**< Previous state of the protocol */

	static std::map<UWTWR_PAUV::UWTWR_PAUV_STATUS, std::string> status_info; /**< Textual info of the state */

	uint ACK_uid; /**< ACK Unique ID */

	int n_ack_rx; /**< Number of ack packets received */
	int n_poll_rx; /**< Number of POLL packets received */
	int n_poll_dropped; /**< Number of POLL packet dropped */
	int n_ack_dropped; /**<Number of ACK dropped pkts */
};

#endif