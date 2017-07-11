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
 * @file mS2C_Evo_lowlev.cpp
 * @author Roberto Francescon
 * \version 0.0.1
 * \brief Implementation of the MS2C_Evo_lowlev class.
 */

#include "mS2C_Evo_lowlev.h"

/**
 * Class to create the Otcl shadow object for an object of the class
 * MS2C_Evo_lowlev. The OTcl object is called Module/UW/MPhy_modem/S2CLowLevel.
 */
static class MS2C_Evo_lowlev_TclClass : public TclClass
{
	// Variable to read the path to the device
	std::string pToDevice_;

public:
	MS2C_Evo_lowlev_TclClass()
		: TclClass("Module/UW/MPhy_modem/S2CLowLevel")
	{
	}

	TclObject *
	create(int args, const char *const *argv)
	{
		pToDevice_ = (std::string) argv[4];
		return (new MS2C_Evo_lowlev(pToDevice_));
	}

} class_module_s2cevologics;

MS2C_Evo_lowlev::MS2C_Evo_lowlev(std::string pToDevice_)
	: UWMPhy_modem(pToDevice_)
	, checkTmr(this)
	, mDriver(this)
	, dropTmr(this)
{

	setConnections(&checkTmr, &mDriver, &dropTmr);
	// Two steps configurations
	bind("bitrate_index", (int *) &bitrate_i);
	bind("SL", (int *) &SL);
	bind("msg_bitlength", (int *) &msg_bitlen);
	mDriver.setBitrate(bitrate_i);
	mDriver.setSourceLevel(SL);
	mDriver.setPktBitLen(msg_bitlen);
}

MS2C_Evo_lowlev::~MS2C_Evo_lowlev()
{
}

int
MS2C_Evo_lowlev::recvSyncClMsg(ClMessage *m)
{
	return MPhy::recvSyncClMsg(m);
}

modem_state_t
MS2C_Evo_lowlev::check_modem()
{
	modem_state_t modemStatus_old = pmDriver->getStatus();
	modem_state_t modemStatus = pmDriver->updateStatus();
	std::stringstream sstr("");
	string strlog;
	sstr << "CHECK_MODEM::TRANSITION_FROM_" << modemStatus_old << "_TO_"
		 << modemStatus;
	sstr >> strlog;
	pmDriver->printOnLog(LOG_LEVEL_INFO, "UWMPHY_MODEM", strlog);

	if (modemStatus == MODEM_IDLE && modemStatus_old == MODEM_TX) {
		endTx(popTxBuff());
	} else if (modemStatus == MODEM_RX && modemStatus_old == MODEM_IDLE)
		startRx(PktRx);

	else if (modemStatus == MODEM_IDLE_RX && modemStatus_old == MODEM_RX)
		endRx(PktRx);

	else if (modemStatus == MODEM_IDLE_RX && modemStatus_old == MODEM_TX_RX) {
		endTx(popTxBuff());
		startRx(PktRx);
		endRx(PktRx);
	}

	return modemStatus;
}
