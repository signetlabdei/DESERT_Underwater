//
// Copyright (c) 2019 Regents of the SIGNET lab, University of Padova.
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
 * @file mS2C_ClMessage.h
 * @author Federico Favaro
 * @version 1.0.0
 * @brief Header of ClMessage for MS2CEvologics
 */

#ifndef MS2C_CL_MESSAGE_H
#define MS2C_CL_MESSAGE_H

#include "uwphy-clmsg.h"
#include "packet.h"

extern ClMessage_t CLMSG_S2C_POWER_LEVEL;
extern ClMessage_t CLMSG_S2C_TX_MODE;
extern ClMessage_t CLMSG_S2C_RX_FAILED;

class ClSAP;

/**
 * Class representing the message for changing or retrieving the power level
 * (source level)
 */
class ClMsgS2CPowerLevel : public ClMsgUwPhy
{
  public:
	/**
	 * Class constructor
	 */
	ClMsgS2CPowerLevel();

	/**
	 * Class constructor, with parameters
	 * @param int stack_id: id of the stack
     * @param dest_mod_id: id of the destination module
	 */
	ClMsgS2CPowerLevel(int stack_id, int dest_module_id);

	/**
	 * Class copy constructor
	 */
	ClMsgS2CPowerLevel(const ClMsgS2CPowerLevel &msg);

	/**
	 * Class destructor
	 */
	virtual ~ClMsgS2CPowerLevel();

	/**
	 * Set the poer level in the selected Cl message
	 * @param level the power level to set in the Cl Message
	 */
	void set_power_level(int level);

	/**
	 * Retrieve the power level specified in the Cl message
	 * @return the power level value in the message
	 */
	int get_power_level() const
	{
		return power_level;
	}


  private:
	int power_level; /**< Power level (source level) value */
};

/**
 * Cl Message type for setting the Tx Mode: Instant Message, Burst or Piggyback
 */
class ClMsgS2CTxMode : public ClMsgUwPhy
{
  public:

	typedef enum tx_mode {
		S2C_TX_MODE_IM = 0,
		S2C_TX_MODE_BURST,
		SBC_TX_MODE_PBM
	} tx_mode_t;

	/**
	 * Class constructor: no parameters
	 */
	ClMsgS2CTxMode();

	/**
	 * Class constructor with parameters
	 * @param int stack_id: id of the stack
     * @param dest_mod_id: id of the destination module
	 */
	ClMsgS2CTxMode(int stack_id, int dest_module_id);

	/**
	 * Class copy constructor
	 */
	ClMsgS2CTxMode(const ClMsgS2CTxMode &msg);

	/**
	 * Class destructor
	 */
	virtual ~ClMsgS2CTxMode();

	/**
	 * Method that sets the TX mode in the message to the specified value
	 * @param mode TX mode to set in the message
	 */
	void set_tx_mode(tx_mode_t mode);

	/**
	 * Method used to retrieve the TX mode value in the message
	 * @return Tx mode value in the message
	 */
	tx_mode_t get_tx_mode() const
	{
		return tx_mode;
	}

  private:
	tx_mode_t tx_mode; /**< Tx mode set in the message: IM, Burst or PBM */
};

/**
 * Class representing the Cl message type used for retrieving the failed
 * receptions counter of S2C devices
 */
class ClMsgS2CRxFailed : public ClMsgUwPhy
{
  public:
	/**
	 * Class constructor: no parameters
	 */
	ClMsgS2CRxFailed();

	/**
	 * Class constructor with parameters
	 * @param int stack_id: id of the stack
     * @param dest_mod_id: id of the destination module
	 */
	ClMsgS2CRxFailed(int stack_id, int dest_module_id);

	/**
	 * Class copy constrcutor
	 */
	ClMsgS2CRxFailed(const ClMsgS2CRxFailed &msg);

	/**
	 * Class destructor
	 */
	virtual ~ClMsgS2CRxFailed();

	/**
	 * Method used to set the number of reception failures in the message
	 * @param n_failed number of failed receptions to set
	 */
	void set_n_rx_failed(int n_failed);

	/**
	 * Method used to retrieve the number of reception failures
	 * from the message
	 * @return number of failed receptions set in the message
	 */
	int get_n_rx_failed() const
	{
		return n_rx_failed;
	}

  private:
	int n_rx_failed; /**< Number of failed receptions */
};

#endif
