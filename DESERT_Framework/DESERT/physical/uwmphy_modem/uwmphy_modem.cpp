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
 * @file uwmphy_modem.cpp
 * \author Riccardo Masiero
 * \author Federico Favaro
 * \version 2.0.0
 * \brief Implementation of the UWMPhy_modem class.
 */

#include "uwmphy_modem.h"

UWMPhy_modem::UWMPhy_modem(std::string pToDevice_)
{

	pToDevice = pToDevice_;

	bind("ID_", &ID);
	bind("period_", &period);
	bind("debug_", &debug_);
	bind("loglevel_", &loglevel_);
	bind("SetModemID_", &SetModemID);
	bind("ModemQueueLength_", &queue_length);
	t = -1;
	PktRx = NULL;
	pcheckTmr = NULL;
	pDropTimer = NULL;
	pmDriver = NULL;

	for (int i = 0; i < _MTBL; i++) {
		modemTxBuff[i] = NULL;
	}

	if (loglevel_ > MAX_LOG_LEVEL)
		loglevel_ = MAX_LOG_LEVEL;

	if (queue_length < 0)
		queue_length = MIN_MODEM_QUEUE_LENGTH;
	if (queue_length > MAX_MODEM_QUEUE_LENGTH)
		queue_length = MAX_MODEM_QUEUE_LENGTH;
}

UWMPhy_modem::~UWMPhy_modem()
{
}

int
UWMPhy_modem::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (!strcmp(argv[1], "start")) {
			start();
			return TCL_OK;
		}
		if (!strcmp(argv[1], "stop")) {
			stop();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (!strcmp(argv[1], "setLogSuffix")) {
			string tmp_ = (char *) argv[2];
			// tmp_.push_back('\n');
			log_suffix = new char[tmp_.size()];
			strcpy(log_suffix, tmp_.c_str());
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

void
UWMPhy_modem::updatePktRx(Packet *p)
{
	PktRx = p;
}

void
UWMPhy_modem::recv(Packet *p)
{

	pmDriver->printOnLog(
			LOG_LEVEL_INFO, "UWMPHY_MODEM", "RECV::RECV_PKT_FROM_UPPER_LAYER");

	int modemStatus = pmDriver->getStatus();
	if (t >= -1 && t < _MTBL) {
		if (t == _MTBL - 1) {
			free(p);
			pmDriver->printOnLog(LOG_LEVEL_INFO,
					"UWMPHY_MODEM",
					"NO_SPACE_LEFT_ON_MODEM_QUEUE");
		} else {
			modemTxBuff[++t] = p;
		}
	} else {
		pmDriver->printOnLog(LOG_LEVEL_ERROR,
				"UWMPHY_MODEM",
				"MODEM_QUEUE_INDEX_OUT_OF_BOUNDS");
	}

	while (modemStatus == MODEM_CFG) {
		modemStatus = check_modem();
	}

	while (modemStatus == MODEM_RESET) {
		modemStatus = check_modem();
	}
	if (modemStatus == MODEM_RX || modemStatus == MODEM_IDLE_RX) {
		pmDriver->resetModemStatus();
		free(PktRx);
		PktRx = NULL;
		modemStatus = MODEM_IDLE;
		pmDriver->printOnLog(LOG_LEVEL_ERROR,
				"UWMPHY_MODEM",
				"RX_PACKET_NOT_DELIVERED_CONCURRENT_PACKET");
	}
	if (modemStatus == MODEM_IDLE) {
		hdr_mac *mach = HDR_MAC(modemTxBuff[0]);
		hdr_uwal *uwalh = HDR_UWAL(modemTxBuff[0]);
		std::string payload_string;
		payload_string.assign(uwalh->binPkt(), uwalh->binPktLength());
		pmDriver->updateTx(mach->macDA(), payload_string);
		startTx(modemTxBuff[0]);
		if (pmDriver->getStatus() != MODEM_TX) {
			endTx(popTxBuff());
		}
	}
}

void
UWMPhy_modem::setConnections(
		CheckTimer *pcheckTmr_, UWMdriver *pmDriver_, DropTimer *pDropTimer_)
{
	pcheckTmr = pcheckTmr_;
	pmDriver = pmDriver_;
	pDropTimer = pDropTimer_;
}

void
UWMPhy_modem::start()
{
	std::stringstream str("");
	if (log_suffix) {
		str << "MODEM_log_" << ID << "_" << log_suffix << "_" << getEpoch();
	} else {
		str << "MODEM_log_" << ID << getEpoch();
	}
	str >> logFile;

	outLog.open(logFile.c_str());
	if (!outLog) {
		std::cout << "WARNING: Opening error ( " << strerror(errno)
				  << " ). It was not possible to create " << logFile.c_str()
				  << "\n";
	}
	outLog.close();
	pmDriver->setID(ID);
	if (SetModemID == 1) {
		pmDriver->setModemID(true);
	} else {
		pmDriver->setModemID(false);
	}
	pmDriver->start();
	pcheckTmr->resched(period);
}

void
UWMPhy_modem::stop()
{
	pmDriver->stop();
	pcheckTmr->force_cancel();
}

void
UWMPhy_modem::startTx(Packet *p)
{
	pmDriver->printOnLog(
			LOG_LEVEL_DEBUG, "UWMPHY_MODEM", "CHECK_MODEM::START_TX");

	pmDriver->modemTx();
}

void
UWMPhy_modem::endTx(Packet *p)
{
	Phy2MacEndTx(p);
	pmDriver->printOnLog(
			LOG_LEVEL_DEBUG, "UWMPHY_MODEM", "CHECK_MODEM::END_TX");
	free(p);
}

void
UWMPhy_modem::startRx(Packet *p)
{

	pmDriver->printOnLog(
			LOG_LEVEL_DEBUG, "UWMPHY_MODEM", "CHECK_MODEM::START_RX");
	Phy2MacStartRx(p);
}

void
UWMPhy_modem::endRx(Packet *p)
{

	std::string str = pmDriver->getRxPayload();
	unsigned char *buf = (unsigned char *) str.c_str();
	Packet *p_rx = Packet::alloc();
	hdr_uwal *uwalh = HDR_UWAL(p_rx);
	uwalh->binPktLength() = str.length();
	memset(uwalh->binPkt(), 0, sizeof(uwalh->binPkt()));
	memcpy(uwalh->binPkt(), buf, uwalh->binPktLength());
	this->updatePktRx(p_rx);
	pmDriver->printOnLog(
			LOG_LEVEL_DEBUG, "UWMPHY_MODEM", "CHECK_MODEM::END_RX");
	sendUp(PktRx);
	PktRx = NULL;
	pmDriver->resetModemStatus();
}

Packet *
UWMPhy_modem::popTxBuff()
{

	Packet *temp = modemTxBuff[0];
	for (int i = 0; i < t; i++) {
		modemTxBuff[i] = modemTxBuff[i + 1];
	}

	t--;
	return temp;
}

void
CheckTimer::expire(Event *e)
{
	pmModem->check_modem();

	resched(pmModem->getPeriod());
}

void
DropTimer::expire(Event *e)
{
	pModem->pmDriver->setResetModemQueue(false);
}
