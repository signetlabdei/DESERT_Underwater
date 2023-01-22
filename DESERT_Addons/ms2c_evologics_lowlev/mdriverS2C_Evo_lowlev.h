//
// Copyright (c) 2016 Regents of the SIGNET lab, University of Padova.
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
 * @file mdriverS2C_Evo_lowlev.h
 * @author Roberto Francescon
 * @version 0.0.1
 * @brief Class derived from UWMdriver to interface ns2/NS-Miracle with the
 * S2C EvoLogics acoustic modems through the low level firmware made available,
 * individually, by EvoLogics, Gmbh (www.evologics.de).
 */

#ifndef UWMDRIVERS2CLOWLEVEL_H
#define UWMDRIVERS2CLOWLEVEL_H

#include <msocket.h>
#include "minterpreterTEL.h"
#include <uwmdriver.h>
#include <queue>

enum LL_STATE_TX {
	TX_STATE_IDLE = 0,
	TX_STATE_ON1,
	TX_STATE_ON2,
	TX_STATE_ON3,
	TX_STATE_ON4,
	TX_STATE_OFF1,
	TX_STATE_OFF2,
	TX_STATE_OFF3,
	TX_STATE_DATA,
	TX_STATE_CTRL,
	TX_STATE_BITRATE_CFG,
	TX_STATE_DSP_CFG,
	TX_STATE_ASK_BUSY,
	TX_STATE_STOP_LISTEN,
	TX_STATE_CLEAR_TX,
};

enum LL_STATE_RX { RX_STATE_IDLE = 0, RX_STATE_DATA, LISTENING };

typedef enum LL_STATE_TX ll_tx_state_t;

typedef enum LL_STATE_RX ll_rx_state_t;

/**
 * Class containing the basic functions to drive the S2C EvoLogics acoustic
 * modem transmissions/receptions according to Low Level drivers
 * (this class is a derived class of UWMdriver).
 */
class MdriverS2C_Evo_lowlev : public UWMdriver
{

	MinterpreterTEL mInterpreter; /**< Object that builds/parses TELEGRAMS and
															   GPIO settings. */
	Msocket mConnector; /**< Object that handles the physical host to modem
											 communications via TCP/IP sockets.
						   */

	ll_tx_state_t m_state_tx; /**< TX state to manage transmission methods*/
	ll_rx_state_t m_state_rx; /**< RX state to manage reception methods*/

	std::queue<std::string> queue_tx; /**< Queue used to buffer incoming strings
															  for tx messages.*/
	std::queue<std::string>
			queue_rx; /**< Queue used to buffer incoming strings for
														 rx messages.*/
	int _gain; /**< Variable holding the Gain level of the low level driver*/
	int _SL; /**< Variable holding the Source Level of the low level driver*/
	int _bitrate_i; /**< Variable holding the bitrate index of the low level
																		 firmware*/
	int _chipset; /**< Variable holding the chipset that will be used by the low
																  level
					 firmware*/
	int _th; /**< Variable holding the threshold used by the low level
				firmware*/
	int _mps_th; /**< Variable holding the MPS treshold used by the low level
																		 firmware*/
	double
			_delay; /**< Variable that holds the delay required for doing some
						  operations, e.g., sending a message, stopping
					   listening */
	int _delay_flag; /**< Variable that holds that flag that triggers the
			  waiting
			  time for some operations e.g., sending a packet, stopping
			  listening*/
	int _msg_bitlen; /**< Very very temporary parameter to let the receiver not
					   screw up and read only the, known, number of bytes */

public:
	/**
	 * Class constructor
	 */
	MdriverS2C_Evo_lowlev(UWMPhy_modem *);
	/**
	 * Class destructor
	 */
	~MdriverS2C_Evo_lowlev();
	/**
	 * Method to let the driver start operations and initialize
	 * configurations.
	 */
	void start();
	/**
	 * Method to stop the driver operations. To be called before finishing
	 * the simulation.
	 */
	void stop();
	/**
	 * Method to set the bitrate that will be sent to the modem config
	 */
	void setBitrate(int index);
	/**
	 * Method to set the source level that will be sent to the modem config
	 */
	void setSourceLevel(int level);
	/**
	 * Method to set the msg bitlength that will be received by rx
	 */
	void setPktBitLen(int bitlen);
	/**
	 * Method that notifies the driver that there is a packet to be sent via
	 * the modem.
	 * NOTE: when this function is called (by an UWMPhy_modem object) the
	 * driver's status must be set to MODEM_TX and the packet must be sent
	 * immediately to the modem.
	 */
	void modemTx();
	/**
	 * Method that notifies the driver that there is a packet to be sent via
	 * the modem. On the Low Level firmware nothing as this exists so the call
	 * is passed to modemTx().
	 * NOTE: when this function is called (by an UWMPhy_modem object) the
	 * driver's status must be set to MODEM_TX and the packet must be sent
	 * immediately to the modem.
	 */
	void modemTxBurst();
	/**
	 * Method that notifies the driver that there is a packet to be sent via
	 * the modem. On the Low Level firmware nothing as this exists so the call
	 * is passed to modeTx().
	 * NOTE: when this function is called (by an UWMPhy_modem object) the
	 * driver's status must be set to MODEM_TX and the packet must be sent
	 * immediately to the modem.
	 */
	void modemTxPBM();
	/**
	 * Method that notifies the driver that there is a configuration to be
	 * sent to the modem (DSP).
	 * NOTE: when this function is called (by an UWMPhy_modem object) the
	 * driver's status must be set to MODEM_TX and the configuration must be
	 * sent immediately to the modem.
	 */
	void modemCfgDSP();
	/**
	 * Method that notifies the driver that there is a configuration to be
	 * sent to the modem (Bitrate).
	 * NOTE: when this function is called (by an UWMPhy_modem object) the
	 * driver's status must be set to MODEM_TX and the configuration must be
	 * sent immediately to the modem.
	 */
	void modemCfgBitrate();
	/**
	 * Method to set the ID of the modem. On the Low Level firmware is not
	 * possible to set any ID so the method is left empty.
	 */
	virtual void modemSetID();
	/**
	 * Method to update modem status. This method has to update the modem
	 * status according to the  messages received from the modem/channel
	 * (e.g., after a check of the modem buffer's output).
	 * NOTE: This method may return after an arbitrary period if nothing has
	 * happened, but it must return immediately after a change of
	 * UWMdriver::status.
	 *
	 *  @return UWMdriver::status, the updated modem's status.
	 */
	virtual modem_state_t updateStatus();

protected:
	/**
	 * Method that manages the transmission of TELEGRAMS and configurations via
	 * GPIOs to the modem.
	 */
	void modemTxManager();
	/**
	 * Method for updating the state after a significant change, namely: a
	 * configuration of the firmware parameters, a trasmission of a packet and a
	 * reception of a packet.
	 */
	void updateTxState(ll_tx_state_t);
};
#endif
