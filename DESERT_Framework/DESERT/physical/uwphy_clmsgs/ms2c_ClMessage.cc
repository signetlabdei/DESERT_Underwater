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
 * @file mS2C_ClMessage.cc
 * @author Federico Favaro
 * \version 1.0.0
 * \brief Implementation of ClMessage for MS2CEvologics
 */

#include "ms2c_ClMessage.h"

// ClMessage_t CLMSG_S2C_POWER_LEVEL;
// ClMessage_t CLMSG_S2C_TX_MODE;
// ClMessage_t CLMSG_S2C_RX_FAILED;

ClMsgS2CPowerLevel::ClMsgS2CPowerLevel()
	: ClMsgUwPhy(CLMSG_S2C_POWER_LEVEL), power_level(0)
{
}

ClMsgS2CPowerLevel::ClMsgS2CPowerLevel(int stack_id, int dest_module_id)
	: ClMsgUwPhy(stack_id, dest_module_id, CLMSG_S2C_POWER_LEVEL),
	  power_level(0)
{
}

ClMsgS2CPowerLevel::ClMsgS2CPowerLevel(const ClMsgS2CPowerLevel &msg)
	: ClMsgUwPhy(msg)
{
	power_level = msg.power_level;
}

ClMsgS2CPowerLevel::~ClMsgS2CPowerLevel()
{
}

void
ClMsgS2CPowerLevel::set_power_level(int level)
{
	power_level = level;
}

ClMsgS2CTxMode::ClMsgS2CTxMode()
	: ClMsgUwPhy(CLMSG_S2C_TX_MODE), tx_mode(ClMsgS2CTxMode::S2C_TX_MODE_IM)
{
}

ClMsgS2CTxMode::ClMsgS2CTxMode(int stack_id, int dest_module_id)
	: ClMsgUwPhy(CLMSG_S2C_TX_MODE), tx_mode(ClMsgS2CTxMode::S2C_TX_MODE_IM)
{
}

ClMsgS2CTxMode::ClMsgS2CTxMode(const ClMsgS2CTxMode &msg) : ClMsgUwPhy(msg)
{
	tx_mode = msg.tx_mode;
}

ClMsgS2CTxMode::~ClMsgS2CTxMode()
{
}

void ClMsgS2CTxMode::set_tx_mode(ClMsgS2CTxMode::tx_mode_t mode)
{
	tx_mode = mode;
}


ClMsgS2CRxFailed::ClMsgS2CRxFailed()
	: ClMsgUwPhy(CLMSG_S2C_RX_FAILED), n_rx_failed(0)
{
}

ClMsgS2CRxFailed::ClMsgS2CRxFailed(int stack_id, int dest_module_id)
	:
	ClMsgUwPhy(stack_id, dest_module_id, CLMSG_S2C_RX_FAILED),
	n_rx_failed(0)
{
}

ClMsgS2CRxFailed::ClMsgS2CRxFailed(const ClMsgS2CRxFailed &msg)
	:
	ClMsgUwPhy(msg)
{
	n_rx_failed = msg.n_rx_failed;
}

ClMsgS2CRxFailed::~ClMsgS2CRxFailed()
{
}

void ClMsgS2CRxFailed::set_n_rx_failed(int rx_failed)
{
	n_rx_failed = rx_failed;
}
