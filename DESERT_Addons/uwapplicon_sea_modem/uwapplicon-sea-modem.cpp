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
#include <uwal.h>
#include <uwapplicon-sea-modem.h>
#include <uwphy-clmsg.h>
#include <uwsocket.h>

#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <iomanip>
#include <chrono>


const std::chrono::milliseconds UwAppliconSeaModem::MODEM_TIMEOUT =
    std::chrono::milliseconds(500);
/**
 * Class to create the Otcl shadow object for an object of the class
 * UwAppliconSeaModem.
 */
static class UwApplicon_TclClass : public TclClass
{

public:
	UwApplicon_TclClass()
		: TclClass("Module/UwModem/Applicon")
	{
	}

	TclObject *
	create(int args, const char *const *argv)
	{
		return (new UwAppliconSeaModem());
	}

} class_evologicss2c;

UwAppliconSeaModem::UwAppliconSeaModem()
	: UwModem()
	, p_connector(new UwSocket())
	, p_interpreter(new UwAppliconInterpr())
	, status(SeaModemState::AVAILABLE)
	, tx_status(TransmissionState::TX_IDLE)
	, status_m()
	, tx_status_m()
	, tx_queue_m()
	, status_cv()
	, tx_status_cv()
	, tx_queue_cv()
	, receiving(false)
	, transmitting(false)
	, rx_thread()
	, tx_thread()

{
	bind("buffer_size", (int *) &DATA_BUFFER_LEN);
	bind("max_read_size", (int *) &MAX_READ_BYTES);
}

UwAppliconSeaModem::~UwAppliconSeaModem()
{
	stop();
}

void
UwAppliconSeaModem::recv(Packet *p)
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
				"APPLICONSEAMODEM",
				"recv::PUSHING_IN_TX_QUEUE");
		tx_queue_cv.notify_one();
	}
}

int
UwAppliconSeaModem::command(int argc, const char *const *argv)
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
	}
	return UwModem::command(argc, argv);
}



void
UwAppliconSeaModem::startTx(Packet *p)
{
	// this method does nothing in ns, so it can be called from an
	// external thread
	// save MAC and AL headers and write payload
	hdr_mac *mach = HDR_MAC(p);
	hdr_uwal *uwalh = HDR_UWAL(p);
	std::string payload;
	payload.assign(uwalh->binPkt(), uwalh->binPktLength());

	// build command to perform a SEND or SENDIM
	std::string cmd_s = p_interpreter->buildSend(payload, mach->macDA());
	

	printOnLog(LogLevel::INFO,
			"APPLICONSEAMODEM",
			"startTx::COMMAND_TX::" + cmd_s);

	// write the obtained command to the device, through the connector
	std::unique_lock<std::mutex> state_lock(status_m);
	if (status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
			return status == SeaModemState::AVAILABLE;
		})) {

		status = SeaModemState::BUSY;

		std::unique_lock<std::mutex> tx_state_lock(tx_status_m);

		if ((p_connector->writeToDevice(cmd_s)) < 0) {
			printOnLog(LogLevel::ERROR,
					"APPLICONSEAMODEM",
					"startTx::FAIL_TO_WRITE_TO_DEVICE=" + cmd_s);
			return;
		}

		
		tx_status = TransmissionState::TX_PENDING;
		if (!status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&]
			 { return status == SeaModemState::AVAILABLE; })) {
			printOnLog(LogLevel::ERROR,
				"APPLICONSEAMODEM",
				"startTx::TIMEOUT_EXPIRED::WAITING_FOR_ACK");
			status = SeaModemState::AVAILABLE;
		}

		// add code if you need to check for modem state
		state_lock.unlock();

		std::function<void(UwModem &, Packet * p)> callback =
				&UwModem::realTxEnded;
		ModemEvent e = {callback, p};
		event_q.push(e);

	} else {
		printOnLog(LogLevel::ERROR,
				"APPLICONSEAMODEM",
				"startTx::TIMEOUT_EXPIRED::FORCING_MODEM_AVAILABILITY");
		status = SeaModemState::AVAILABLE;
	}
}

void
UwAppliconSeaModem::startRx(Packet *p)
{
	printOnLog(LogLevel::INFO,
			"APPLICONSEAMODEM",
			"startRx::CALL_PHY2MACSTARTRX");
	Phy2MacStartRx(p);
}

void
UwAppliconSeaModem::endRx(Packet *p)
{
	printOnLog(LogLevel::INFO, "APPLICONSEAMODEM", "endRx::CALL_SENDUP");
	sendUp(p, 0.01);
}

void
UwAppliconSeaModem::start()
{
	if (modem_address == "") {
		std::cout << "ERROR: Modem address not set!" << std::endl;
		printOnLog(
				LogLevel::ERROR, "APPLICONSEAMODEM", "start::ADDRESS_NOT_SET");
		return;
	}

	if (!p_connector->openConnection(modem_address)) {
		std::cout << "ERROR: connection to modem failed to open: "
				  << modem_address << std::endl;
		printOnLog(LogLevel::ERROR,
				"APPLICONSEAMODEM",
				"start::CONNECTION_OPEN_FAILED");
		return;
	}

	// set flags to true so loops can start
	receiving.store(true);
	transmitting.store(true);

	// branch off threads
	rx_thread = std::thread(&UwAppliconSeaModem::receivingData, this);

	tx_thread = std::thread(&UwAppliconSeaModem::transmittingData, this);

	checkTimer = new CheckTimer(this);
	checkTimer->resched(period);
}

void
UwAppliconSeaModem::stop()
{

	std::cout << "UwAppliconSeaModem::stop() close conn " << std::endl;
	printOnLog(LogLevel::INFO, "APPLICONSEAMODEM", "stop::CLOSING_CONNECTION");

	receiving.store(false);
	transmitting.store(false);
	tx_queue_cv.notify_one();
	if (tx_thread.joinable())
		tx_thread.join();
	if (p_connector->isConnected() && !p_connector->closeConnection()) {
		printOnLog(LogLevel::ERROR,
				"APPLICONSEAMODEM",
				"stop::CONNECTION_CLOSE_FAIL");
	}
	if (rx_thread.joinable()) {
		rx_thread.join();
	}

	checkTimer->force_cancel();
}

void
UwAppliconSeaModem::transmittingData()
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
				"APPLICONSEAMODEM",
				"transmittingData::BLOCKING_ON_NEXT_PACKET");
	}
}

void
UwAppliconSeaModem::receivingData()
{
	data_buffer.resize(DATA_BUFFER_LEN);
	std::fill(data_buffer.begin(), data_buffer.end(), '\0');
	// iterators that keep track of read/write operations
	std::vector<char>::iterator beg_it = data_buffer.begin();
	std::vector<char>::iterator end_it = data_buffer.begin();
	// iterators that keep track of commands research
	std::vector<char>::iterator cmd_b = data_buffer.begin();
	std::vector<char>::iterator cmd_e = data_buffer.begin();
	UwAppliconInterpr::Response cmd = UwAppliconInterpr::Response::NO_COMMAND;
	int r_bytes = 0;
	int offset = 0;

	while (receiving.load()) {

		r_bytes = p_connector->readFromDevice(
				&(*beg_it) + offset, MAX_READ_BYTES - offset);

		if (r_bytes <= 0) {
			// Avoid busy loop if read returns 0 or error
			// std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		end_it = beg_it + offset + r_bytes;

		while (receiving.load() && r_bytes > 0 &&
				(cmd = p_interpreter->findResponse(beg_it, end_it, cmd_b)) !=
						UwAppliconInterpr::Response::NO_COMMAND) {

			bool integrity = true;
			while (receiving.load() &&
					!p_interpreter->parseResponse(
							cmd, end_it, cmd_b, cmd_e, rx_payload, integrity)) {

				int current_len = std::distance(beg_it, end_it);
				int new_r_bytes = p_connector->readFromDevice(
				    &(*end_it), MAX_READ_BYTES - current_len);

				if (new_r_bytes > 0) {
					r_bytes += new_r_bytes;
					end_it += new_r_bytes;
				} else {
					break;
				}
			}

			printOnLog(LogLevel::DEBUG,
					"APPLICONSEAMODEM",
					"receivingData::RX_MSG=" + std::string(cmd_b, cmd_e));

			updateStatus(cmd, integrity);
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
UwAppliconSeaModem::updateStatus(UwAppliconInterpr::Response cmd, bool integrity)
{
	std::unique_lock<std::mutex> state_lock(status_m);
	switch (cmd) {
		case UwAppliconInterpr::Response::AOK: {
			// The modem is available again after receiving the ACK
			status = SeaModemState::AVAILABLE;
			status_cv.notify_all();
			state_lock.unlock();

			// Print a log message with the content of the ACK
			printOnLog(LogLevel::INFO,
					"APPLICONSEAMODEM",
					"updateStatus::AOK_RECEIVED: " + rx_payload);
			
			
			break;
		}
		case UwAppliconInterpr::Response::MSGS:	
		case UwAppliconInterpr::Response::RECV: {
			status = SeaModemState::AVAILABLE;
			status_cv.notify_all();
			state_lock.unlock();

			printOnLog(LogLevel::INFO,
				"APPLICONSEAMODEM",
				"updateStatus::MSGS_RECEIVED PRE_: " + std::to_string(rx_payload.length()));

			// std::stringstream ss_hex;
			// // Set the hexadecimal format once
			// ss_hex << std::hex << std::setfill('0');

			// // 3. Iterate over each character of the input string
			// for (char c : rx_payload)
			// {
			// 	// Set width to 2 for each character and add it to the stream
			// 	ss_hex << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
			// }

			// // 4. Extract the final string from the stringstream
			// std::string hex_string = ss_hex.str();

			// printOnLog(LogLevel::INFO,
			// 	"APPLICONSEAMODEM",
			// 	"updateStatus::MSGS_RECEIVED PRE HEX: " + hex_string);

			Packet *p = Packet::alloc();
			createRxPacket(p, integrity);
			
			if (!integrity) {
				printOnLog(LogLevel::ERROR, "APPLICONSEAMODEM", "updateStatus::DROPPING_CORRUPTED_PACKET");
				Packet::free(p);
			} else {
				std::function<void(UwModem &, Packet * p)> callback =
						&UwModem::recv;
				ModemEvent e = {callback, p};
				event_q.push(e);
			}
			printOnLog(LogLevel::INFO,
				"APPLICONSEAMODEM",
				"updateStatus::MSGS_RECEIVED POST: " + rx_payload);

			break;
		}
		default: {
			status = SeaModemState::AVAILABLE;
			status_cv.notify_all();
		}
	}

	printOnLog(LogLevel::DEBUG, "APPLICONSEAMODEM", "updateStatus::END");
}

void
UwAppliconSeaModem::createRxPacket(Packet *p, bool integrity)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	
	if (rx_payload.size() > MAX_BIN_PKT_ARRAY_LENGTH) {
		printOnLog(LogLevel::ERROR, "APPLICONSEAMODEM", "createRxPacket::PAYLOAD_TOO_LARGE_TRUNCATING");
		uwalh->binPktLength() = MAX_BIN_PKT_ARRAY_LENGTH;
	} else {
		uwalh->binPktLength() = rx_payload.size();
	}

	std::memset(uwalh->binPkt(), 0, uwalh->binPktLength());
	std::copy(rx_payload.begin(), rx_payload.begin() + uwalh->binPktLength(), uwalh->binPkt());
	HDR_CMN(p)->direction() = hdr_cmn::UP;
	
	if (!integrity) {
		HDR_CMN(p)->error() = 1;
		printOnLog(LogLevel::ERROR, "APPLICONSEAMODEM", "createRxPacket::PACKET_CORRUPTED");
	}
}
