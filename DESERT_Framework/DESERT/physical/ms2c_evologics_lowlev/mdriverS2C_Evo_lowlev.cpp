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
 * @file mdriverS2C_Evo_lowlev.cc
 * @author Roberto Francescon
 * @version 0.0.1
 * @brief Implementation of the MdriverS2C_Evo_lowlev class.
 */

#include "mdriverS2C_Evo_lowlev.h"
#include <uwmphy_modem.h>

// Utility methods
static void
hexdump(std::string name, std::string str)
{
	int len = str.size();
	const char *data = str.c_str();

	std::cout << name << "[" << len << "]: " << std::hex;
	for (int i = 0; i < len; i++) {
		std::cout.fill('0');
		std::cout.width(2);
		std::cout << std::right << (unsigned int) (unsigned char) data[i];

		if (std::isalnum(data[i]) || std::ispunct(data[i]))
			std::cout << "(" << data[i] << ")";
		std::cout << " ";
	}

	std::cout.width(0);
	std::cout << std::dec << std::endl;
}

static std::string
hexdumplog(std::string str)
{
	int len = str.size();
	const char *data = str.c_str();
	std::string str_out = "";

	for (int i = 0; i < len; i++) {
		if (std::isalnum(data[i]) || std::ispunct(data[i]))
			str_out += data[i];
		else {
			std::string str;
			std::stringstream sstr("");
			sstr << "[" << std::hex << (unsigned int) (unsigned char) data[i]
				 << std::dec << "]";
			sstr >> str;
			str_out += str;
		}
	}
	return str_out;
}

static std::string
hexdumpdata(std::string str)
{
	int len = str.size();
	const char *data = str.c_str();

	std::string str_out = "";

	for (int i = 0; i < len; i++) {
		if (std::isalnum(data[i]) || std::ispunct(data[i]))
			str_out += data[i];
		else {
			std::string str;
			std::stringstream sstr("");
			sstr << std::hex << (unsigned int) (unsigned char) data[i];
			sstr >> str;
			str_out += str;
		}
	}

	return str_out;
}

MdriverS2C_Evo_lowlev::MdriverS2C_Evo_lowlev(UWMPhy_modem *pmModem_)
	: UWMdriver(pmModem_)
	, mInterpreter(this)
	, mConnector(this, pmModem_->getPathToDevice())
	, _gain(0)
	, _SL(3)
	, _bitrate_i(5)
	, _chipset(2)
	, _th(350)
	, _mps_th(0)
	, _delay(0)
	, _delay_flag(0)
	, _msg_bitlen(0)
{
	m_state_tx = TX_STATE_IDLE;
	m_state_rx = RX_STATE_IDLE;
	setConnections(&mInterpreter, &mConnector);
}

MdriverS2C_Evo_lowlev::~MdriverS2C_Evo_lowlev()
{
}

void
MdriverS2C_Evo_lowlev::start()
{
	printOnLog(LOG_LEVEL_DEBUG, "MS2C_LOWLEVELDRIVER", "MODEM_START");
	mConnector.openConnection();

	std::string q_msg;
	q_msg = mConnector.readFromModem();

	while (q_msg != "") {
		q_msg = mConnector.readFromModem();
	}

	status = MODEM_CFG;
	m_state_tx = TX_STATE_ON1;
	modemTxManager();
}

void
MdriverS2C_Evo_lowlev::stop()
{

	status = MODEM_QUIT;
	m_state_tx = TX_STATE_OFF1;
	modemTxManager();

	// empty msg queue and close TCP connection
	std::string q_trash;
	q_trash = mConnector.readFromModem();
	mConnector.closeConnection();
}

void
MdriverS2C_Evo_lowlev::modemTx()
{
	status = MODEM_TX;
	m_state_tx = TX_STATE_DATA;
	modemTxManager();
}

modem_state_t
MdriverS2C_Evo_lowlev::updateStatus()
{

	modem_state_t old_status = status;
	std::stringstream sstr("");
	std::string strlog;
	//  modem_state_t status;
	sstr << "UPDATE_STATUS::STATUS_" << status << "_M_STATUS_TX_" << m_state_tx
		 << "_M_STATUS_RX_" << m_state_rx;
	sstr >> strlog;
	printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);

	std::string rx_msg;
	bool cread = true;

	while (cread) {
		// read what's waiting in the modem
		rx_msg = mConnector.readFromModem();

		if (rx_msg != "") {
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER::RAW", rx_msg);

			std::string pr_msg;
			std::string parser("\n");
			size_t p_offset = 0;
			size_t p_parser = rx_msg.find(parser);

			// Messages in low level can be very long and we need a line
			// termination
			while (!rx_msg.empty() && p_parser == std::string::npos) {
				usleep(1000);
				rx_msg += mConnector.readFromModem();
				p_parser = rx_msg.find(parser);
			}

			while (p_parser != std::string::npos) {
				pr_msg = rx_msg.substr(
						p_offset, p_parser + parser.size() - p_offset);
				if (pr_msg.find("TELEGRAM_ACCEPTED") != string::npos) {
					queue_rx.push(pr_msg);
				}
				if (pr_msg.find("TELEGRAM_SENT") != string::npos ||
						pr_msg.find("GPIO") != string::npos ||
						pr_msg.find("DSP_READY_TO_START") != string::npos ||
						pr_msg.find("LISTENING_EXPIRED") != string::npos ||
						pr_msg.find("CONFIG_DONE") != string::npos ||
						pr_msg.find("BITRATE") != string::npos) {
					queue_tx.push(pr_msg);
				}
				p_offset += pr_msg.size();
				p_parser = rx_msg.find(parser, p_offset + 1);
			} // End while (p_parser!=std::string::npos)
		} else {
			cread = false; // exit loop
		} // if (rx_msg != "")
	} // while (cread)

	cread = true; // next loop

	if (status == MODEM_TX || status == MODEM_CFG || status == MODEM_RESET ||
			status == MODEM_QUIT) // read from queue_tx
	{

		while (cread) {
			if (!queue_tx.empty()) {
				// Read the possible received message
				rx_msg = queue_tx.front();
				queue_tx.pop();

				if (rx_msg.find("TELEGRAM_SENT") != std::string::npos)
					if (rx_msg.find("4114") != std::string::npos) {
						std::stringstream sstr("");
						string strlog;
						sstr << "UPDATE_STATUS::PROCESSING_ERROR::" << status
							 << "_M_STATUS_TX_" << m_state_tx << "_M_STATUS_RX_"
							 << m_state_rx;
						sstr >> strlog;
						printOnLog(
								LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
						status = MODEM_IDLE;
						m_state_tx = TX_STATE_IDLE;
						cread = false;
					} else {
						std::stringstream sstr("");
						string strlog;
						sstr << "UPDATE_STATUS::PROCESSING_OK::" << status
							 << "_M_STATUS_TX_" << m_state_tx << "_M_STATUS_RX_"
							 << m_state_rx;
						sstr >> strlog;
						printOnLog(
								LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
						status = MODEM_IDLE;
						m_state_tx = TX_STATE_IDLE;
						cread = false;
					}
				else if (rx_msg.find("DSP_READY_TO_START") != string::npos) {
					std::stringstream sstr("");
					string strlog;
					sstr << "UPDATE_STATUS::POWERING_ON_OK::" << status
						 << "_M_STATUS_TX_" << m_state_tx << "_M_STATUS_RX_"
						 << m_state_rx;
					sstr >> strlog;
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
					status = MODEM_CFG;
					m_state_tx = TX_STATE_DSP_CFG;
					modemTxManager();
					cread = false;
				} else if (rx_msg.find("BITRATE") != string::npos) {
					std::stringstream sstr("");
					string strlog;
					sstr << "UPDATE_STATUS::SETTING_BITRATE_OK::" << status
						 << "_M_STATUS_TX_" << m_state_tx << "_M_STATUS_RX_"
						 << m_state_rx;
					sstr >> strlog;
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
					status = MODEM_IDLE;
					m_state_tx = TX_STATE_IDLE;
					cread = false;
				} else if (rx_msg.find("LISTENING_EXPIRED") != string::npos) {
					status = MODEM_IDLE;
					m_state_tx = TX_STATE_IDLE;
					cread = false;
				} else if (rx_msg.find("CONFIG_DONE") != string::npos) {
					std::stringstream sstr("");
					string strlog;
					sstr << "UPDATE_STATUS::CONFIGURING_DSP_OK::" << status
						 << "_M_STATUS_TX_" << m_state_tx << "_M_STATUS_RX_"
						 << m_state_rx;
					sstr >> strlog;
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
					status = MODEM_CFG;
					m_state_tx = TX_STATE_BITRATE_CFG;
					modemTxManager();
					cread = false;
				} else if (rx_msg.find("tx_on") != string::npos) {
					std::stringstream sstr("");
					std::string strlog;
					sstr << "UPDATE_STATUS::TX_PIN_CLEARED::" << m_state_tx
						 << "_" << status;
					sstr >> strlog;
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
					status = MODEM_IDLE;
					m_state_tx = TX_STATE_IDLE;
				} else if (rx_msg.find("ERROR") != string::npos) {
					std::stringstream sstr("");
					std::string strlog;
					sstr << "UPDATE_STATUS::ERROR::" << m_state_tx << "_"
						 << status;
					sstr >> strlog;
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
					status = MODEM_IDLE;
					cread = false;
				} else {
					std::stringstream sstr("");
					std::string strlog;
					sstr << "UPDATE_STATUS::OK_WRONG_STATUS_" << rx_msg
						 << m_state_tx << "_" << status;
					sstr >> strlog;
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
				}
			} else {
				cread = false;
			}
		}
	} else if (status == MODEM_RX) {
		std::stringstream sstr("");
		string strlog;
		sstr << "UPDATE_STATUS::END_RX::" << status << "_M_STATUS_TX_"
			 << m_state_tx << "_M_STATUS_RX_" << m_state_rx;
		strlog = sstr.str();
		printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
		m_state_rx = RX_STATE_IDLE;
		status = MODEM_IDLE_RX;
		cread = false;
	} else if (status == MODEM_IDLE_RX) {
		std::stringstream sstr("");
		string strlog;
		sstr << "UPDATE_STATUS::GOING_TO_LISTENING::" << status
			 << "_M_STATUS_TX_" << m_state_tx << "_M_STATUS_RX_" << m_state_rx;
		strlog = sstr.str();
		printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
		m_state_rx = RX_STATE_IDLE;
		status = MODEM_IDLE;
		cread = false;
	} else // read from queue_rx only
	{
		while (cread) {
			if (!queue_rx.empty()) {
				// Read the possible received message
				rx_msg = queue_rx.front();
				queue_rx.pop();

				if (rx_msg.find("TELEGRAM_ACCEPTED") != string::npos) {
					std::stringstream sstr("");
					std::string strlog;
					mInterpreter.parse_TELEGRAM(rx_msg);
					sstr << "UPDATE_STATUS::DETECTED_DATA::" << payload_rx
						 << "::" << m_state_tx << "_" << status;
					strlog = sstr.str();
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
					status = MODEM_RX;
					m_state_rx = RX_STATE_DATA;
					m_state_tx = TX_STATE_IDLE;
					cread = false;
				} else if (rx_msg.find("CONFIG_DONE") != string::npos) {
					std::stringstream sstr("");
					string strlog;
					sstr << "UPDATE_STATUS::CONFIGURING_DSP_OK::" << status
						 << "_M_STATUS_TX_" << m_state_tx << "_M_STATUS_RX_"
						 << m_state_rx;
					strlog = sstr.str();
					printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
					status = MODEM_IDLE;
					m_state_tx = TX_STATE_IDLE;
					cread = false;
				}
			} else {
				cread = false;
			} // if (!queue_rx.empty())
		} // while (cread)

	} // if (status == _TX || status == _CFG)
	modemTxManager();
	return status;
}

void
MdriverS2C_Evo_lowlev::modemTxManager()
{
	std::stringstream sstr("");
	std::string strlog;
	std::string tx_msg;
	std::string rx_msg;
	switch (m_state_tx) {
		case TX_STATE_ON1:
			tx_msg = mInterpreter.build_poweron_DSP(1); // turn on firmware
			break;
		case TX_STATE_ON2:
			tx_msg = mInterpreter.build_poweron_DSP(2);
			break;
		case TX_STATE_ON3:
			sleep(1);
			tx_msg = mInterpreter.build_poweron_DSP(3);
			break;
		case TX_STATE_ON4:
			tx_msg = mInterpreter.build_poweron_DSP(4);
			sstr << "MODEM_TX_MANAGER::MODEM_START_DSP";
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_OFF1:
			tx_msg = mInterpreter.build_poweroff_DSP(1); // turn off firmware
			break;
		case TX_STATE_OFF2:
			tx_msg = mInterpreter.build_poweroff_DSP(2);
			break;
		case TX_STATE_OFF3:
			tx_msg = mInterpreter.build_poweroff_DSP(3);
			sstr << "MODEM_TX_MANAGER::MODEM_STOP_DSP";
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_IDLE:
			tx_msg = mInterpreter.build_recv_data(_msg_bitlen, 0, 0);
			sstr << "MODEM_TX_MANAGER::TELEGRAM_RECV::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_DATA:
			tx_msg = mInterpreter.build_send_data(
					payload_tx, _delay_flag, _delay);
			sstr << "MODEM_TX_MANAGER::TELEGRAM_DATA::MSG::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_CTRL:
			tx_msg = mInterpreter.build_send_ctrl(
					payload_tx, _delay_flag, _delay);
			sstr << "MODEM_TX_MANAGER::TELEGRAM_CTRL::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_DSP_CFG:
			tx_msg = mInterpreter.build_config_DSP(
					_gain, _chipset, _SL, _th, _mps_th);
			sstr << "MODEM_TX_MANAGER::TELEGRAM_DSP_CFG::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_BITRATE_CFG:
			tx_msg = mInterpreter.build_bitrate(_bitrate_i);
			sstr << "MODEM_TX_MANAGER::TELEGRAM_BITRATE_CFG::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_ASK_BUSY:
			tx_msg = mInterpreter.build_busy_FPGA();
			sstr << "MODEM_TX_MANAGER::TELEGRAM_ASK_BUSY::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_STOP_LISTEN:
			tx_msg = mInterpreter.build_stop_listen();
			sstr << "MODEM_TX_MANAGER::TELEGRAM_STOP_LISTEN::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
			break;
		case TX_STATE_CLEAR_TX:
			usleep(100000);
			tx_msg = mInterpreter.build_clear_tx();
			sstr << "MODEM_TX_MANAGER::TELEGRAM_CLEAR_TX::" << tx_msg;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO, "MS2C_LOWLEVELDRIVER", strlog);
	} // switch

	usleep(100000);
	mConnector.writeToModem(tx_msg);
	updateTxState(m_state_tx);
}

void
MdriverS2C_Evo_lowlev::updateTxState(ll_tx_state_t state)
{
	switch (state) {
		case TX_STATE_ON1:
			m_state_tx = TX_STATE_ON2;
			modemTxManager();
			break;
		case TX_STATE_ON2:
			m_state_tx = TX_STATE_ON3;
			modemTxManager();
			break;
		case TX_STATE_ON3:
			m_state_tx = TX_STATE_ON4;
			modemTxManager();
			break;
		case TX_STATE_ON4:
			sleep(0.5);
			m_state_tx = TX_STATE_IDLE;
			modemTxManager();
			break;
		case TX_STATE_OFF1:
			m_state_tx = TX_STATE_OFF2;
			modemTxManager();
			break;
		case TX_STATE_OFF2:
			m_state_tx = TX_STATE_OFF3;
			modemTxManager();
			break;
		case TX_STATE_DATA:
			m_state_tx = TX_STATE_CLEAR_TX;
			modemTxManager();
			break;
		case TX_STATE_CTRL:
			m_state_tx = TX_STATE_CLEAR_TX;
			modemTxManager();
			break;
		case TX_STATE_BITRATE_CFG:
			m_state_tx = TX_STATE_IDLE;
			modemTxManager();
			break;
		case TX_STATE_DSP_CFG:
			m_state_tx = TX_STATE_IDLE;
			modemTxManager();
			break;
		case TX_STATE_ASK_BUSY:
			m_state_tx = TX_STATE_IDLE;
			modemTxManager();
			break;
		case TX_STATE_STOP_LISTEN:
			m_state_tx = TX_STATE_IDLE;
			modemTxManager();
			break;
		case TX_STATE_CLEAR_TX:
			m_state_tx = TX_STATE_IDLE;
			modemTxManager();
			break;
		default:
			return;
	}
}

void
MdriverS2C_Evo_lowlev::setBitrate(int index)
{
	_bitrate_i = index;
}

void
MdriverS2C_Evo_lowlev::setSourceLevel(int level)
{
	_SL = level;
}

void
MdriverS2C_Evo_lowlev::setPktBitLen(int bitlen)
{
	_msg_bitlen = bitlen;
}

void
MdriverS2C_Evo_lowlev::modemSetID()
{
	return;
}
