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

#include <clmessage.h>
#include <hdr-uwal.h>
#include <ms2c_ClMessage.h>
#include <uwal.h>
#include <uwevologicss2cmodem.h>
#include <uwphy-clmsg.h>
#include <uwsocket.h>

#include <cstring>
#include <iostream>
#include <iterator>
#include <string>

const std::chrono::milliseconds UwEvoLogicsS2CModem::MODEM_TIMEOUT =
    std::chrono::milliseconds(210);

const std::chrono::seconds UwEvoLogicsS2CModem::WAIT_DELIVERY_BURST =
    std::chrono::seconds(5);

const std::chrono::milliseconds UwEvoLogicsS2CModem::WAIT_DELIVERY_IM =
    std::chrono::milliseconds(200);

uint UwEvoLogicsS2CModem::MAX_N_STATUS_QUERIES = 10;

/**
 * Class to create the Otcl shadow object for an object of the class
 * UwEvoLogicsS2CModem.
 */
static class UwEvoLogicsS2C_TclClass : public TclClass
{

public:
	UwEvoLogicsS2C_TclClass()
		: TclClass("Module/UW/UwModem/EvoLogicsS2C")
	{
	}

	TclObject *
	create(int args, const char *const *argv)
	{
		return (new UwEvoLogicsS2CModem());
	}

} class_evologicss2c;

UwEvoLogicsS2CModem::UwEvoLogicsS2CModem()
	: UwModem()
	, p_connector(new UwSocket())
	, p_interpreter(new UwInterpreterS2C())
	, status(ModemState::AVAILABLE)
	, tx_status(TransmissionState::TX_IDLE)
	, status_m()
	, tx_status_m()
	, tx_queue_m()
	, status_cv()
	, tx_status_cv()
	, tx_queue_cv()
	, receiving(false)
	, transmitting(false)
	, im_status_updated(false)
	, rx_thread()
	, tx_thread()
	, tx_mode(TransmissionMode::IM)
	, ack_mode(false)
	, curr_source_level(3)
	, n_rx_failed(0)
	, pend_source_level(3)
	, source_level_change(false)

{
	bind("buffer_size", (int *) &DATA_BUFFER_LEN);
	bind("max_read_size", (int *) &MAX_READ_BYTES);
	bind("max_n_status_queries",
        (uint *) &UwEvoLogicsS2CModem::MAX_N_STATUS_QUERIES);
}

UwEvoLogicsS2CModem::~UwEvoLogicsS2CModem()
{
	stop();
}

void
UwEvoLogicsS2CModem::recv(Packet *p)
{
	// push new incoming pck into tx_queue
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	if (ch->direction() == hdr_cmn::UP) {
		if (isOn == true) {
			startRx(p);
			endRx(p);
		} else {
			Packet::free(p);
		}
	} else { // DOWN
		if (!isOn) {
			return;
		}
		ph->Pr = 0;
		ph->Pn = 0;
		ph->Pi = 0;
		ph->txtime = NOW;
		ph->rxtime = ph->txtime;

		ph->worth_tracing = false;

		ph->srcSpectralMask = getTxSpectralMask(p);
		ph->srcAntenna = getTxAntenna(p);
		ph->srcPosition = getPosition();
		ph->dstSpectralMask = 0;
		ph->dstPosition = 0;
		ph->dstAntenna = 0;
		ph->modulationType = getModulationType(p);
		ph->duration = getTxDuration(p);

		std::unique_lock<std::mutex> tx_lock(tx_queue_m);
		tx_queue.push(p);
		tx_lock.unlock();
		printOnLog(LogLevel::DEBUG,
				"EVOLOGICSS2CMODEM",
				"recv::PUSHING_IN_TX_QUEUE");
		tx_queue_cv.notify_one();
	}
}

int
UwEvoLogicsS2CModem::command(int argc, const char *const *argv)
{
	// Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (!strcmp(argv[1], "start")) {
			start();
			return TCL_OK;
		}
		if (!strcmp(argv[1], "stop")) {
			stop();
			return TCL_OK;
		}
		if (!strcmp(argv[1], "setBurstMode")) {
			tx_mode = TransmissionMode::BURST;
			return TCL_OK;
		}
		if (!strcmp(argv[1], "setIMMode")) {
			tx_mode = TransmissionMode::IM;
			return TCL_OK;
		}
		if (!strcmp(argv[1], "enableIMAck")) {
			ack_mode = true;
			return TCL_OK;
		}
		if (!strcmp(argv[1], "disableIMAck")) {
			ack_mode = false;
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (!strcmp(argv[1], "setSourceLevel")) {
			std::stringstream sl_ss(argv[2]);
			int sl_;
			if (sl_ss >> sl_)
				return TCL_OK;
			else
				return TCL_ERROR;
		}
	}

	return UwModem::command(argc, argv);
}

int
UwEvoLogicsS2CModem::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_S2C_POWER_LEVEL) {
		ClMsgS2CPowerLevel *msg = static_cast<ClMsgS2CPowerLevel *>(m);
		if (msg->getReqType() == ClMsgUwPhy::SET_REQ) {
			printOnLog(LogLevel::INFO,
					"UWEVOLOGICSS2CMODEM",
					"RECEIVED_CLMSG_FOR_POWER_CHANGE");
			pend_source_level = msg->get_power_level();
			source_level_change = true;
			msg->setReqType(ClMsgUwPhy::SET_REPLY);
			return 0;
		} else if (msg->getReqType() == ClMsgUwPhy::GET_REQ) {
			msg->set_power_level(curr_source_level);
			msg->setReqType(ClMsgUwPhy::GET_REPLY);
			return 0;
		}
	} else if (m->type() == CLMSG_S2C_TX_MODE) {
		ClMsgS2CTxMode *msg = static_cast<ClMsgS2CTxMode *>(m);
		if (msg->getReqType() == ClMsgUwPhy::SET_REQ) {
			if (msg->get_tx_mode() == ClMsgS2CTxMode::S2C_TX_MODE_IM) {
				tx_mode = TransmissionMode::BURST;
			} else {
				tx_mode = TransmissionMode::IM;
			}
			msg->setReqType(ClMsgUwPhy::SET_REPLY);
			return 0;
		} else if (msg->getReqType() == ClMsgUwPhy::GET_REQ) {
			if (tx_mode == TransmissionMode::IM) {
				msg->set_tx_mode(ClMsgS2CTxMode::S2C_TX_MODE_IM);
			} else {
				msg->set_tx_mode(ClMsgS2CTxMode::S2C_TX_MODE_BURST);
			}
			msg->setReqType(ClMsgUwPhy::GET_REPLY);
			return 0;
		}
	} else if (m->type() == CLMSG_S2C_RX_FAILED) {
		ClMsgS2CRxFailed *msg = static_cast<ClMsgS2CRxFailed *>(m);
		if (msg->getReqType() == ClMsgUwPhy::GET_REQ) {
			msg->set_n_rx_failed(n_rx_failed);
			msg->setReqType(ClMsgUwPhy::GET_REPLY);
			return 0;
		} else {
			return 1;
		}
	}

	return MPhy::recvSyncClMsg(m);
}

bool
UwEvoLogicsS2CModem::configure(Config cmd)
{
	std::string config_cmd{""};

	switch(cmd) {
	case Config::ATL_GET: {
		std::string command = p_interpreter->buildGetATL();
		break;
	}
	case Config::ATL_SET: {
		std::string command = p_interpreter->buildSetATL(pend_source_level);
		break;
	}
	case Config::ATAL_GET: {
		std::string command = p_interpreter->buildGetATAL();
		break;
	}
	case Config::ATAL_SET: {
		std::string command = p_interpreter->buildSetATAL(modemID);
		break;
	}
	case Config::MODEM_STATUS: {
		std::string command = p_interpreter->buildATS();
		break;
	}
	case Config::CURR_SETTINGS: {
		std::string command = p_interpreter->buildATV();
		break;
	}
	default:
		return false;
	}

	std::unique_lock<std::mutex> state_lock(status_m);

	if (status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
				return status == ModemState::AVAILABLE;
			})) {

		status = ModemState::BUSY;

		std::lock_guard<std::mutex> tx_state_lock(tx_status_m);

		if (!p_connector->writeToDevice(config_cmd)) {
			printOnLog(LogLevel::ERROR,
					   "EVOLOGICSS2CMODEM",
					   "configure::FAIL_TO_WRITE_TO_DEVICE=" + config_cmd);
			return false;
		}

		state_lock.unlock();
		tx_status = TransmissionState::TX_PENDING;

	} else {

		return false;

	}

	return true;
}

void
UwEvoLogicsS2CModem::startTx(Packet *p)
{
	// this method does nothing in ns, so it can be called from an
	// external thread
	// save MAC and AL headers and write payload
	hdr_mac *mach = HDR_MAC(p);
	hdr_uwal *uwalh = HDR_UWAL(p);
	std::string payload;
	payload.assign(uwalh->binPkt(), uwalh->binPktLength());

	// build command to perform a SEND or SENDIM
	std::string cmd_s;
	if (tx_mode == TransmissionMode::IM) {
		cmd_s = p_interpreter->buildSendIM(payload, mach->macDA(), ack_mode);
	} else {
		cmd_s = p_interpreter->buildSend(payload, mach->macDA());
	}

	printOnLog(LogLevel::INFO,
			"EVOLOGICSS2CMODEM",
			"startTx::COMMAND_TX::" + cmd_s);

	// write the obtained command to the device, through the connector
	std::unique_lock<std::mutex> state_lock(status_m);
	if (status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
			return status == ModemState::AVAILABLE;
		})) {

		status = ModemState::BUSY;

		std::unique_lock<std::mutex> tx_state_lock(tx_status_m);

		if ((p_connector->writeToDevice(cmd_s)) < 0) {
			printOnLog(LogLevel::ERROR,
					"EVOLOGICSS2CMODEM",
					"startTx::FAIL_TO_WRITE_TO_DEVICE=" + cmd_s);
			return;
		}

		
		tx_status = TransmissionState::TX_PENDING;
		status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&]
			 { return status == ModemState::AVAILABLE; });
		state_lock.unlock();

		if (tx_mode == TransmissionMode::IM) {

			size_t status_polling_counter = 0;
			im_status_updated.store(false);
			while (status_polling_counter < MAX_N_STATUS_QUERIES &&
				   tx_status != TransmissionState::TX_IDLE) {
			
				cmd_s = p_interpreter->buildATDI();

				printOnLog(LogLevel::INFO,
						   "EVOLOGICSS2CMODEM",
						   "startTx::SENDING=" + cmd_s);

				if ((p_connector->writeToDevice(cmd_s)) < 0) {
					printOnLog(LogLevel::ERROR,
							   "EVOLOGICSS2CMODEM",
							   "startTx::FAIL_TO_WRITE_TO_DEVICE=" + cmd_s);
				}

				tx_status_cv.wait_for(tx_state_lock, WAIT_DELIVERY_IM, [&]
				    { return (tx_status == TransmissionState::TX_IDLE); });

				if (tx_status == TransmissionState::TX_IDLE) {
					printOnLog(LogLevel::DEBUG,
							   "EVOLOGICSS2CMODEM",
							   "startTx::TX_IDLE");
					break;
				} else {
					im_status_updated.store(false);
					printOnLog(LogLevel::DEBUG,
							   "EVOLOGICSS2CMODEM",
							   "startTx::TX_PENDING");
				}
			
				status_polling_counter++;
			}

			//In case the attempts reached MAX_N_STATUS_QUERIES, force tx_status
			if (status_polling_counter == MAX_N_STATUS_QUERIES &&
					tx_status != TransmissionState::TX_IDLE) {
				printOnLog(LogLevel::ERROR,
						"EVOLOGICSS2CMODEM",
						"startTx::MAX_N_STATUS_QUERIES_REACHED");
			}

		} else { // tx_mode == TransmissionMode::BURST
			tx_status_cv.wait_for(tx_state_lock, WAIT_DELIVERY_BURST, [&]
				    { return (tx_status == TransmissionState::TX_IDLE); });
		}


		std::function<void(UwModem &, Packet * p)> callback =
				&UwModem::realTxEnded;
		ModemEvent e = {callback, p};
		event_q.push(e);

	} else {
		printOnLog(LogLevel::ERROR,
				"EVOLOGICSS2CMODEM",
				"startTx::TIMEOUT_EXPIRED::FORCING_MODEM_AVAILABILITY");
		status = ModemState::AVAILABLE;
	}
}

void
UwEvoLogicsS2CModem::startRx(Packet *p)
{
	printOnLog(LogLevel::INFO,
			"EVOLOGICSS2CMODEM",
			"startRx::CALL_PHY2MACSTARTRX");
	Phy2MacStartRx(p);
}

void
UwEvoLogicsS2CModem::endRx(Packet *p)
{
	printOnLog(LogLevel::INFO, "EVOLOGICSS2CMODEM", "endRx::CALL_SENDUP");
	sendUp(p, 0.01);
}

void
UwEvoLogicsS2CModem::start()
{
	if (modem_address == "") {
		std::cout << "ERROR: Modem address not set!" << std::endl;
		printOnLog(
				LogLevel::ERROR, "EVOLOGICSS2CMODEM", "start::ADDRESS_NOT_SET");
		return;
	}

	if (!p_connector->openConnection(modem_address)) {
		std::cout << "ERROR: connection to modem failed to open: "
				  << modem_address << std::endl;
		printOnLog(LogLevel::ERROR,
				"EVOLOGICSS2CMODEM",
				"start::CONNECTION_OPEN_FAILED");
		return;
	}

	// set flags to true so loops can start
	receiving.store(true);
	transmitting.store(true);

	// branch off threads
	rx_thread = std::thread(&UwEvoLogicsS2CModem::receivingData, this);

	tx_thread = std::thread(&UwEvoLogicsS2CModem::transmittingData, this);

	checkTimer = new CheckTimer(this);
	checkTimer->resched(period);
}

void
UwEvoLogicsS2CModem::stop()
{

	std::cout << "UwEvoLogicsS2CModem::stop() close conn " << std::endl;
	printOnLog(LogLevel::INFO, "EVOLOGICSS2CMODEM", "stop::CLOSING_CONNECTION");

	receiving.store(false);
	transmitting.store(false);
	tx_queue_cv.notify_one();
	if (tx_thread.joinable())
		tx_thread.join();
	if (p_connector->isConnected() && !p_connector->closeConnection()) {
		printOnLog(LogLevel::ERROR,
				"EVOLOGICSS2CMODEM",
				"stop::CONNECTION_CLOSE_FAIL");
	}
	if (rx_thread.joinable()) {
		rx_thread.join();
	}

	checkTimer->force_cancel();
}

void
UwEvoLogicsS2CModem::transmittingData()
{
	while (transmitting.load()) {

		std::unique_lock<std::mutex> tx_lock(tx_queue_m);
		tx_queue_cv.wait(
				tx_lock, [&] { return !tx_queue.empty() || !transmitting; });
		if (!transmitting) {
			break;
		}

		Packet *pck = tx_queue.front();
		tx_queue.pop();
		tx_lock.unlock();
		if (pck) {
			startTx(pck);
		}

		printOnLog(LogLevel::DEBUG,
				"EVOLOGICSS2CMODEM",
				"transmittingData::BLOCKING_ON_NEXT_PACKET");
	}
}

void
UwEvoLogicsS2CModem::receivingData()
{
	data_buffer.resize(DATA_BUFFER_LEN);
	std::fill(data_buffer.begin(), data_buffer.end(), '\0');
	// iterators that keep track of read/write operations
	std::vector<char>::iterator beg_it = data_buffer.begin();
	std::vector<char>::iterator end_it = data_buffer.begin();
	// iterators that keep track of commands research
	std::vector<char>::iterator cmd_b = data_buffer.begin();
	std::vector<char>::iterator cmd_e = data_buffer.begin();
	UwInterpreterS2C::Response cmd = UwInterpreterS2C::Response::NO_COMMAND;
	int r_bytes = 0;
	int offset = 0;

	while (receiving.load()) {

		r_bytes = p_connector->readFromDevice(
				&(*beg_it) + offset, MAX_READ_BYTES - r_bytes);

		end_it = beg_it + r_bytes + offset;

		while (receiving.load() && r_bytes > 0 &&
				(cmd = p_interpreter->findResponse(beg_it, end_it, cmd_b)) !=
						UwInterpreterS2C::Response::NO_COMMAND) {

			while (receiving.load() &&
					!p_interpreter->parseResponse(
							cmd, end_it, cmd_b, cmd_e, rx_payload)) {

				int new_r_bytes = p_connector->readFromDevice(
				    &(*end_it), MAX_READ_BYTES - r_bytes);

				r_bytes += new_r_bytes;
				end_it = beg_it + r_bytes;

			}

			printOnLog(LogLevel::DEBUG,
					"EVOLOGICSS2CMODEM",
					"receivingData::RX_MSG=" + std::string(cmd_b, cmd_e));

			updateStatus(cmd);
			std::fill(cmd_b, cmd_e, '\0');
			cmd_b = cmd_e + 1;
		}

		// check if bytes are left after parsing and move them to beginning
		if ((int) (cmd_e - beg_it) < r_bytes) {
			offset = r_bytes - (int) (cmd_e - beg_it);
			std::copy(cmd_e, cmd_e + offset, data_buffer.begin());
		} else {
			offset = 0;
		}
	}
}

void
UwEvoLogicsS2CModem::updateStatus(UwInterpreterS2C::Response cmd)
{
	std::unique_lock<std::mutex> state_lock(status_m);
	switch (cmd) {

		case UwInterpreterS2C::Response::RECVIM: {
			
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			state_lock.unlock();

			Packet *p = Packet::alloc();
			createRxPacket(p);
			std::function<void(UwModem &, Packet * p)> callback =
					&UwModem::recv;
			ModemEvent e = {callback, p};
			event_q.push(e);
			break;
		}
		case UwInterpreterS2C::Response::RECV: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			state_lock.unlock();

			Packet *p = Packet::alloc();
			createRxPacket(p);
			std::function<void(UwModem &, Packet * p)> callback =
					&UwModem::recv;
			ModemEvent e = {callback, p};
			event_q.push(e);
			break;
		}
		case UwInterpreterS2C::Response::OK: {
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::EMPTY: {
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			tx_status = TransmissionState::TX_IDLE;
			im_status_updated.store(true);
			tx_status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::BUSY: {
			status = ModemState::BUSY;
			state_lock.unlock();
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::DELIVERING: {
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			status = ModemState::AVAILABLE;
			im_status_updated.store(true);
			tx_status_cv.notify_all();
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::DELIVERED: {			
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			tx_status = TransmissionState::TX_IDLE;
			im_status_updated.store(true);
			tx_status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::DROPCNT: {
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			tx_status = TransmissionState::TX_IDLE;
			tx_status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::PHYOFF: {
			status = ModemState::QUIT;
			state_lock.unlock();
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::NOT_ACCEPTED: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::WRONG_ADDR: {
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			tx_status = TransmissionState::TX_IDLE;
			im_status_updated.store(true);
			tx_status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::CONN_CLOSED: {
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			tx_status = TransmissionState::TX_IDLE;
			im_status_updated.store(true);			
			tx_status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::BUFF_NOT_EMPTY: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::OUT_OF_RANGE: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::PROTOCOL_ID: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::INTERNAL: {
			status = ModemState::RESET;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::BUFFER_FULL: {
			status = ModemState::BUSY;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::FAIL: {
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			tx_status = TransmissionState::TX_IDLE;
			im_status_updated.store(true);			
			tx_status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::INIT_NOISE: {
			status = ModemState::NOISE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::INIT_DEAF: {
			status = ModemState::DEAF;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::INIT_LISTEN: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::RECVSTART: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
	    }
		case UwInterpreterS2C::Response::RECVEND: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::RECVFAIL: {
			n_rx_failed++;
			printOnLog(LogLevel::INFO,
						"EVOLOGICSS2CMODEM",
						"updateStatus::FAILED_RX");
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::SENDSTART: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
	    }
		case UwInterpreterS2C::Response::SENDEND: {
			status = ModemState::AVAILABLE;
			state_lock.unlock();
			status_cv.notify_all();
			std::unique_lock<std::mutex> tx_state_lock(tx_status_m);
			tx_status = TransmissionState::TX_IDLE;
			im_status_updated.store(true);			
			tx_status_cv.notify_all();
			break;
		}
		case UwInterpreterS2C::Response::BITRATE: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
	    }
		case UwInterpreterS2C::Response::UNKNOWN: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			break;
		}
		default: {
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
		}
	}

	printOnLog(LogLevel::DEBUG, "EVOLOGICSS2CMODEM", "updateStatus::END");
}

void
UwEvoLogicsS2CModem::setFailedTx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	ch->error_ = 1;
}

void
UwEvoLogicsS2CModem::createRxPacket(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	uwalh->binPktLength() = rx_payload.size();
	std::memset(uwalh->binPkt(), 0, uwalh->binPktLength());
	std::copy(rx_payload.begin(), rx_payload.end(), uwalh->binPkt());
	HDR_CMN(p)->direction() = hdr_cmn::UP;
}
