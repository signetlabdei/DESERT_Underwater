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
 * @file mS2C_ClMessage.h
 * @author Federico Favaro
 * \version 1.0.0
 * \brief Header of ClMessage for MS2CEvologics
 */

#ifndef MS2C_CL_MESSAGE_H
#define MS2C_CL_MESSAGE_H

#include "uwphy-clmsg.h"
#include "packet.h"

extern ClMessage_t CLMSG_S2C_POWER_LEVEL;
extern ClMessage_t CLMSG_S2C_TX_MODE;

class ClSAP;

class ClMsgS2CPowerLevel : public ClMsgUwPhy
{
  public:
	ClMsgS2CPowerLevel();

	ClMsgS2CPowerLevel(int stack_id, int dest_module_id);

	ClMsgS2CPowerLevel(const ClMsgS2CPowerLevel &msg);

	~ClMsgS2CPowerLevel();

	void set_power_level(int level);

	inline int get_power_level()
	{
		return power_level;
	}

  private:
	int power_level;
};

class ClMsgS2CTxMode : public ClMsgUwPhy
{
  public:
	ClMsgS2CTxMode();

	typedef enum tx_mode {
		S2C_TX_MODE_IM = 0,
		S2C_TX_MODE_BURST,
		SBC_TX_MODE_PBM
	} tx_mode_t;

	ClMsgS2CTxMode(int stack_id, int dest_module_id);

	ClMsgS2CTxMode(const ClMsgS2CTxMode &msg);

	~ClMsgS2CTxMode();

	void set_tx_mode(tx_mode_t mode);

	inline tx_mode_t get_tx_mode()
	{
		return tx_mode;
	}

  private:
	tx_mode_t tx_mode;
};

#endif
