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
 * @file mdriverS2C_EvoLogics.h
 * @author Riccardo Masiero and Federico Favaro
 * \version 2.0.0
 * \brief Header of the class derived from UWMdriver to interface ns2/NS-Miracle
 * with the S2C EvoLogics acoustic modems.
 */

#ifndef UWMDRIVERS2CEVOLOGICS_H
#define UWMDRIVERS2CEVOLOGICS_H

#include "msocket.h"
#include "minterpreterAT.h"
#include <uwmdriver.h>

#include <queue>

#define DROPBUFFER_TYPE 4

enum STATES_RX {
	RX_STATE_IDLE = 0,
	RX_STATE_RX_IM,
	RX_STATE_RX_PBM,
	RX_STATE_RX_BURST
};

enum STATES_TX {
	TX_STATE_IDLE = 0,
	TX_STATE_SET_ID,
	TX_STATE_DROPBUFFER,
	TX_STATE_SEND_IM,
	TX_STATE_SET_KO,
	TX_STATE_SEND_BURST,
	TX_STATE_SEND_PBM,
	TX_STATE_SEND_CLOSE,
	TX_STATE_SEND_ATN,
	TX_STATE_SEND_ATE,
	TX_STATE_SEND_ATA,
	TX_STATE_SEND_ATP,
	TX_STATE_SEND_ATL
};

typedef enum STATES_RX rx_states_t;

typedef enum STATES_TX tx_states_t;

/**
 * Class containing the basic functions to drive the S2C EvoLogics acoustic
 * modem transmissions/receptions (this class is a derived class of UWMdriver).
 */
class MdriverS2C_EvoLogics : public UWMdriver
{
	MinterpreterAT mInterpreter; /** < Object that builds/parses AT messages. */
	Msocket mConnector; /** < Object that handles the physical host to modem
											   communications via TCP/IP
						   sockets. */

	tx_states_t m_status_tx; /**< TX status for the transmission manager
								methods,
												see methods
								MdriverS2C_EvoLogics::modemTxManager
												and
								MdriverS2C_EvoLogics::updateStatus. */

	rx_states_t
			m_status_rx; /**< RX status for the
														MdriverS2C_EvoLogics::updateStatus()
							method. */

	queue<std::string>
			queue_tx; /**< Queue used to buffer incoming strings for tx
						 messages.*/

	queue<std::string>
			queue_rx; /**< Queue used to buffer incoming strings for rx
						 messages.*/

public:
	/**
	 * Class constructor.
	 *
	 * @param pmModem_ pointer to the UWMPhy_modem object to link with this
	 *UWMdriver object.
	 */
	MdriverS2C_EvoLogics(UWMPhy_modem *);

	virtual void modemSetID();

	/**
	 * Class destructor.
	 */
	~MdriverS2C_EvoLogics();

	/**
	 *  Driver starter. This method starts the driver performing all the needed
	 * operations
	 *  to open an host-modem connection.
	 */
	virtual void start();

	/**
	 *  Driver stopper. This method should be used before stopping the
	 * simulation.
	 * It closes and, if needed,
	 *  resets all the opened files and ports.
	 */
	virtual void stop();

	/**
	 *  Method to notify to the driver that there is a packet to be sent via
	 * modem.
	 *  NOTE: when this function is called (by an UWMPhy_modem object), the
	 * driver's status must be set to TX_ and the packet must be sent
	 * immediately
	 * to the modem.
	 */
	virtual void modemTx();

	/**
	 *  Method to update modem status. This method has to update the modem
	 *status
	 *according to the  messages
	 *  received from the modem/channel (e.g., after a check of the modem
	 *buffer's
	 *output). NOTE: This method may return after an arbitrary period if nothing
	 *has happened, but it must return immediately after a change of
	 *UWMdriver::status.
	 *
	 *  @return UWMdriver::status, the updated modem's status.
	 */
	virtual modem_state_t updateStatus();

	virtual void power_level_change_ind(int powerlevel);

	virtual void change_power_level();

	virtual inline int
	get_actual_power_level()
	{
		return actual_power_level;
	}

	void modemTxBurst();

	void modemTxPBM();

	/**
	 *  Method to get the Integrity value of the last received packet. NOTE:
	 *This
	 *method is used by McodecS2C_EvoLogics.
	 *
	 *  @return MinterpreterAT::rx_integrity, the integrity value of the last
	 *received packet.
	 */
	double getIntegrity();
	/**
	 * Method to empty modem queue
	 */
	void emptyModemQueue();

	void enterNoiseState();

	void probeNoise();

	void enterListenState();

	void probeMultipath();

	inline bool
	getKeepOnlineMode()
	{
		return KeepOnline;
	}

	inline void
	setKeepOnlineMode(bool ko)
	{
		KeepOnline = ko;
	}

protected:
	/**
	 * Method to manage modem to host and host to modem communications. This
	 * method has to handle the different transmissions cases and corresponding
	 * protocol messages to be generated according to the tcl-user choices and
	 * modem firmware, respectively.
	 */
	virtual void modemTxManager();

	int tx_packet_counter_noise;
	int tx_packet_counter_multipath;
	int actual_power_level;
	int requested_power_level;
	bool clmsg_c_power_level;

	bool KeepOnline;

private:
	bool is_number(const string s);
};
#endif /* UWMDRIVERS2CEVOLOGICS_H */
