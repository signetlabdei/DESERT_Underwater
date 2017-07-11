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
 * @file mS2C_EvoLogics.cpp
 * @author Riccardo Masiero
 * @author Federico Favaro
 * \version 2.0.0
 * \brief Implementation of the MS2C_EvoLogics class.
 */

#include "mS2C_EvoLogics.h"
#include "uwphy-clmsg.h"

/**
 * Class to create the Otcl shadow object for an object of the class
 * MS2C_EvoLogics.
 */
static class MS2C_EvoLogics_TclClass : public TclClass
{
	// Variable to read the path to the device
	std::string pToDevice_;

public:
	MS2C_EvoLogics_TclClass()
		: TclClass("Module/UW/MPhy_modem/S2C")
	{
	}
	TclObject *
	create(int args, const char *const *argv)
	{
		pToDevice_ = (std::string) argv[4];
		return (new MS2C_EvoLogics(pToDevice_));
	}
} class_module_s2cevologics;

MS2C_EvoLogics::MS2C_EvoLogics(std::string pToDevice_)
	: UWMPhy_modem(pToDevice_)
	, checkTmr(this)
	, mDriver(this)
	, dropTmr(this)
	, NoiseProbeFrequency(0)
	, MultipathProbeFrequency(0)
{
	bind("UseKeepOnline_", &UseKeepOnline);
	bind("NoiseProbeFrequency_", &NoiseProbeFrequency);
	bind("MultipathProbeFrequency_", &MultipathProbeFrequency);
	bind("DeafTime_", &DeafTime);
	setConnections(&checkTmr, &mDriver, &dropTmr);
	if (NoiseProbeFrequency > 0 && NoiseProbeFrequency < MIN_NOISE_PROBE_FREQ)
		NoiseProbeFrequency = MIN_NOISE_PROBE_FREQ;

	if (MultipathProbeFrequency > 0 &&
			MultipathProbeFrequency < MIN_MULTIPATH_PROBE_FREQ)
		MultipathProbeFrequency = MIN_MULTIPATH_PROBE_FREQ;
}

MS2C_EvoLogics::~MS2C_EvoLogics()
{
}

void
MS2C_EvoLogics::start()
{
	if (getKeepOnline() == 1) {
		static_cast<MdriverS2C_EvoLogics *>(pmDriver)->setKeepOnlineMode(true);
	} else {
		static_cast<MdriverS2C_EvoLogics *>(pmDriver)->setKeepOnlineMode(false);
	}
	if (getDeafTime() > 0) {
		static_cast<MdriverS2C_EvoLogics *>(pmDriver)->setResetModemQueue(true);
		std::stringstream sstr("");
		string strlog;
		sstr << "START::MODEM_DEAF_TIME_FOR_" << getDeafTime() << "_SECONDS";
		sstr >> strlog;
		pmDriver->printOnLog(LOG_LEVEL_INFO, "UWMPHY_MODEM", strlog);
		UWMPhy_modem::pDropTimer->resched(getDeafTime());
	}
	UWMPhy_modem::start();
}

void
MS2C_EvoLogics::stop()
{
	if (getKeepOnline()) {
		pmDriver->stop();
		check_modem();
		UWMPhy_modem::pcheckTmr->force_cancel();
	} else {
		UWMPhy_modem::stop();
	}
}

void
MS2C_EvoLogics::startTx(Packet *p)
{
	if (getKeepOnline()) {
		static_cast<MdriverS2C_EvoLogics *>(pmDriver)->modemTxBurst();
	} else {
		UWMPhy_modem::startTx(p);
	}
}

modem_state_t
MS2C_EvoLogics::check_modem()
{

	modem_state_t modemStatus_old = pmDriver->getStatus();
	modem_state_t modemStatus = pmDriver->updateStatus();
	std::stringstream sstr("");
	string strlog;
	sstr << "CHECK_MODEM::TRANSITION_FROM_" << modemStatus_old << "_TO_"
		 << modemStatus;
	sstr >> strlog;
	pmDriver->printOnLog(LOG_LEVEL_INFO, "UWMPHY_MODEM", strlog);

	if (modemStatus == MODEM_CHANGE_POWER_LEVEL) {
		if (modemStatus_old == MODEM_TX) {
			endTx(popTxBuff());
		}
		static_cast<MdriverS2C_EvoLogics *>(pmDriver)->change_power_level();
		return modemStatus;
	}

	if (modemStatus == MODEM_RX && modemStatus_old == MODEM_IDLE) {
		startRx(PktRx);
		return modemStatus;
	}

	if (modemStatus == MODEM_IDLE_RX && modemStatus_old == MODEM_RX) {
		if (modemStatus_old == MODEM_RX) {
			endRx(PktRx);
			return modemStatus;
		}
		if (modemStatus_old == MODEM_TX_RX) {
			endTx(popTxBuff());
			startRx(PktRx);
			endRx(PktRx);
			return modemStatus;
		}
	}

	if (modemStatus == MODEM_CFG && modemStatus_old == MODEM_CFG) {
		pmDriver->modemSetID();
		return modemStatus;
	}

	if (modemStatus == MODEM_NOISE) {
		if (modemStatus_old == MODEM_NOISE) {
			static_cast<MdriverS2C_EvoLogics *>(pmDriver)->probeNoise();
			return modemStatus;
		} else {
			if (modemStatus_old == MODEM_TX) {
				endTx(popTxBuff());
			}
			static_cast<MdriverS2C_EvoLogics *>(pmDriver)->enterNoiseState();
			return modemStatus;
		}
	}

	if (modemStatus == MODEM_IDLE) {
		if (modemStatus_old == MODEM_NOISE) {
			static_cast<MdriverS2C_EvoLogics *>(pmDriver)->enterListenState();
			return modemStatus;
		}
		if (modemStatus_old == MODEM_TX) {
			endTx(popTxBuff());
			return modemStatus;
		}
		if (modemStatus_old == MODEM_MULTIPATH ||
				modemStatus_old == MODEM_IDLE ||
				modemStatus_old == MODEM_CHANGE_POWER_LEVEL) {
			return modemStatus;
		}
	}

	if (modemStatus == MODEM_MULTIPATH) {
		if (modemStatus_old == MODEM_TX) {
			endTx(popTxBuff());
		}
		static_cast<MdriverS2C_EvoLogics *>(pmDriver)->probeMultipath();
		return modemStatus;
	}

	pmDriver->printOnLog(
			LOG_LEVEL_INFO, "UWMPHY_MODEM", "STATE_CHANGE_NOT_HANDLED");
	return modemStatus;
}

int
MS2C_EvoLogics::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_S2C_POWER_LEVEL) {
		ClMsgS2CPowerLevel *msg = static_cast<ClMsgS2CPowerLevel *>(m);
		if (msg->getReqType() == ClMsgUwPhy::SET_REQ) {
			pmDriver->printOnLog(LOG_LEVEL_INFO,
					"UWPHY_MODEM",
					"RECEIVED_CLMSG_FOR_POWER_CHANGE");
			static_cast<MdriverS2C_EvoLogics *>(pmDriver)
					->power_level_change_ind(msg->get_power_level());
			msg->setReqType(ClMsgUwPhy::SET_REPLY);
			return (0);
		} else if (msg->getReqType() == ClMsgUwPhy::GET_REQ) {
			int pwl;
			pwl = static_cast<MdriverS2C_EvoLogics *>(pmDriver)
						  ->get_actual_power_level();
			msg->set_power_level(pwl);
			msg->setReqType(ClMsgUwPhy::GET_REPLY);
			return (0);
		}
	} else if (m->type() == CLMSG_S2C_TX_MODE) {
		ClMsgS2CTxMode *msg = static_cast<ClMsgS2CTxMode *>(m);
		if (msg->getReqType() == ClMsgUwPhy::SET_REQ) {
			/*TODO: Method to handle change of transmission method */
			msg->setReqType(ClMsgUwPhy::SET_REPLY);
			return (0);
		} else if (msg->getReqType() == ClMsgUwPhy::GET_REQ) {
			/*TODO: Global variable to store current tx type */
			msg->set_tx_mode(ClMsgS2CTxMode::S2C_TX_MODE_IM);
			msg->setReqType(ClMsgUwPhy::GET_REPLY);
			return (0);
		}
	}

	return MPhy::recvSyncClMsg(m);
}

void
DropTimer::expire(Event *e)
{
	pModem->pmDriver->setResetModemQueue(false);
}
