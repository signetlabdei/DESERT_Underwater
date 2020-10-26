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
#include <uwmodemcsa.h>
#include <uwphy-clmsg.h>
#include <uwsocket.h>

#include <cstring>
#include <iostream>
#include <iterator>
#include <algorithm>

const std::chrono::milliseconds UwModemCSA::MODEM_TIMEOUT =
		std::chrono::milliseconds(210);

const double UwModemCSA::EPSILON_S = 0.01; // milliseconds

const size_t UwModemCSA::MAX_TX_STATUS_POLL = 20;

/**
 * Class to create the Otcl shadow object for an object of the class
 * UwModemCSA.
 */
static class UwModemCSA_TclClass : public TclClass
{

public:
	UwModemCSA_TclClass()
		: TclClass("Module/UW/UwModem/ModemCSA")
	{
	}

	TclObject *
	create(int args, const char *const *argv)
	{
		return (new UwModemCSA());
	}

} class_modemcsa;

UwModemCSA::UwModemCSA()
	: UwModem()
	, p_connector(new UwSocket())
	, status(ModemState::AVAILABLE)
	, status_m()
	, tx_queue_m()
	, status_cv()
	, tx_queue_cv()
	, receiving(false)
	, transmitting(false)
	, rx_thread()
	, tx_thread()
	, rx_payload("")
	, del_b("PACKET")
	, del_e("EPCK")

{
	bind("buffer_size", (int *) &DATA_BUFFER_LEN);
	bind("max_read_size", (int *) &MAX_READ_BYTES);
}


UwModemCSA::~UwModemCSA()
{
	stop();
}


void
UwModemCSA::recv(Packet *p)
{
	// push new incoming pck into tx_queue
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	if (ch->direction() == hdr_cmn::UP) {
		if (isOn == true) {
			startRx(p);
			endRx(p);

			printOnLog(LogLevel::DEBUG,
				"MODEMCSA",
				"recv::PACKET_SENT_UP");

		} else {
			Packet::free(p);

			printOnLog(LogLevel::DEBUG,
				"MODEMCSA",
				"recv::PACKET_DROPPED");

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
				"MODEMCSA",
				"recv::PUSHING_IN_TX_QUEUE");
		tx_queue_cv.notify_one();
	}
}


int
UwModemCSA::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (!strcmp(argv[1], "setServer")) {
			p_connector->setServer();
			return TCL_OK;
		}

		if (!strcmp(argv[1], "setTCP")) {
			p_connector->setTCP();
			return TCL_OK;
		}

		if (!strcmp(argv[1], "setUDP")) {
			p_connector->setUDP();
			return TCL_OK;
		}
	}
	return UwModem::command(argc, argv);
}

int
UwModemCSA::recvSyncClMsg(ClMessage *m)
{
	return 0;
}

void
UwModemCSA::startTx(Packet *p)
{
	// this method does not do nothing in ns, so it can be called from an
	// extrenal thread
	// save MAC and AL headers and write payload
	hdr_mac *mach = HDR_MAC(p);
	hdr_uwal *uwalh = HDR_UWAL(p);
	std::string payload;
	payload.assign(uwalh->binPkt(), uwalh->binPktLength());

	// build command to perform a SEND
	std::string cmd_s;
	cmd_s = buildSend(payload, mach->macDA());

	printOnLog(LogLevel::INFO,
			"MODEMCSA",
			"startTx::COMMAND_TX::" + cmd_s);

	// write the obtained command to the device, through the connector
	std::unique_lock<std::mutex> state_lock(status_m);
	if (status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
			return status == ModemState::AVAILABLE;
		})) {

		status = ModemState::BUSY;

		if ((p_connector->writeToDevice(cmd_s)) < 0) {
			printOnLog(LogLevel::ERROR,
					"MODEMCSA",
					"startTx::FAIL_TO_WRITE_TO_DEVICE=" + cmd_s);
			return;
		}

		status = ModemState::AVAILABLE;
		state_lock.unlock();
		std::function<void(UwModem &, Packet * p)> callback =
				&UwModem::realTxEnded;
		ModemEvent e = {callback, p};
		event_q.push(e);
	}

	return;
}


std::string
UwModemCSA::buildSend(const std::string &payload, int dest)
{
	std::string cmd_s("");
	cmd_s.append("PACKET,");
	cmd_s += std::to_string(payload.size());
	cmd_s.append(",");
	cmd_s += payload;
	cmd_s.append(",EPCK");

	return cmd_s;
}


void
UwModemCSA::startRx(Packet *p)
{
	printOnLog(LogLevel::INFO,
			"MODEMCSA",
			"startRx::CALL_PHY2MACSTARTRX");
	Phy2MacStartRx(p);
	return;
}


void
UwModemCSA::endRx(Packet *p)
{
	printOnLog(LogLevel::INFO, "MODEMCSA", "endRx::CALL_SENDUP");
	sendUp(p, 0.01);
	return;
}


void
UwModemCSA::start()
{
	if (modem_address == "") {
		std::cout << "ERROR: Modem address not set!" << std::endl;
		printOnLog(
				LogLevel::ERROR, "MODEMCSA", "start::ADDRESS_NOT_SET");
		return;
	}

	if (!p_connector->openConnection(modem_address)) {
		std::cout << "ERROR: connection to modem failed to open: "
				  << modem_address << std::endl;
		printOnLog(LogLevel::ERROR,
				"MODEMCSA",
				"start::CONNECTION_OPEN_FAILED");
		return;
	}

	// set flags to true so loops can start
	receiving.store(true);
	transmitting.store(true);

	// branch off threads
	rx_thread = std::thread(&UwModemCSA::receivingData, this);

	tx_thread = std::thread(&UwModemCSA::transmittingData, this);

	checkTimer = new CheckTimer(this);
	checkTimer->resched(period);
}


void
UwModemCSA::stop()
{
	std::cout << "UwModemCSA::stop() close conn " << std::endl;
	printOnLog(LogLevel::INFO, "MODEMCSA", "stop::CLOSING_CONNECTION");

	receiving.store(false);
	transmitting.store(false);
	tx_queue_cv.notify_one();
	if (tx_thread.joinable())
		tx_thread.join();
	if (p_connector->isConnected() && !p_connector->closeConnection()) {
		printOnLog(LogLevel::ERROR,
				"MODEMCSA",
				"stop::CONNECTION_CLOSE_FAIL");
	}
	if (rx_thread.joinable()) {
		rx_thread.join();
	}

	checkTimer->force_cancel();
}


void
UwModemCSA::transmittingData()
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
				"MODEMCSA",
				"transmittingData::BLOCKING_ON_NEXT_PACKET");
	}
}


void
UwModemCSA::receivingData()
{
	data_buffer.resize(DATA_BUFFER_LEN);
	std::fill(data_buffer.begin(), data_buffer.end(), '\0');
	// iterators that keep track of read/write operations
	std::vector<char>::iterator beg_it = data_buffer.begin();
	std::vector<char>::iterator end_it = data_buffer.begin();
	// iterators that keep track of commands research
	std::vector<char>::iterator cmd_b = data_buffer.begin();
	std::vector<char>::iterator cmd_e = data_buffer.begin();
	std::string cmd("");
	int r_bytes = 0;
	int offset = 0;

	while (receiving.load()) {

		r_bytes = p_connector->readFromDevice(
				&(*beg_it) + offset, MAX_READ_BYTES - r_bytes);

		end_it += r_bytes;

		if (receiving.load() && r_bytes > 0 &&
				(cmd = findCommand(beg_it, end_it, cmd_b, cmd_e)) != "") {

			offset = std::distance(cmd_e, end_it);
			if (parseCommand(cmd_b, cmd_e, rx_payload)) {

				startRealRx(cmd);
			}

			offset = std::distance(cmd_e, end_it);
			std::copy(cmd_e, end_it, beg_it);
			std::fill(beg_it + offset, end_it, '\0');
			end_it = beg_it + offset;

			cmd = "";
		}
	}
}

std::string
UwModemCSA::findCommand(std::vector<char>::iterator beg_it,
						std::vector<char>::iterator end_it,
						std::vector<char>::iterator &cmd_b,
						std::vector<char>::iterator &cmd_e)
{
	std::string cmd = "";
	cmd_b = beg_it;
	cmd_e = beg_it;

	if (cmd_b == end_it)
		return "";

	cmd_b = std::search(beg_it, end_it, del_b.begin(), del_b.end());

	if((cmd_e = std::search(cmd_b, end_it, del_e.begin(), del_e.end())) != end_it) {
		cmd_e += del_e.size();
		return std::string(cmd_b, cmd_e);
	}

	return "";

}

bool
UwModemCSA::parseCommand(std::vector<char>::iterator cmd_b,
						std::vector<char>::iterator cmd_e,
						std::string &rx_payload)
{
	//check only packet correctness
	auto curs_b = find(cmd_b, cmd_e, ',');
	auto curs_e = find(cmd_b, cmd_e, ',') + 1;

	if(std::string(cmd_b, curs_b).compare("PACKET") != 0)
		return false;
	curs_b = curs_e;
	
	// length
	curs_e = find(curs_b, cmd_e, ',');

	if (curs_e >= cmd_e)
		return false;
	int len = std::stoi(std::string(curs_b, curs_e));
	curs_b = curs_e + 1;
	if (curs_b >= cmd_e)
		return false;

	// payload
	auto payload_beg = curs_b;
	if (payload_beg >= cmd_e)
		return false;
	auto payload_end = payload_beg + len;
	rx_payload = std::string(payload_beg, payload_end);

	return true;

}

void
UwModemCSA::startRealRx(const std::string &cmd)
{
	std::unique_lock<std::mutex> state_lock(status_m);

	status = ModemState::AVAILABLE;
	status_cv.notify_all();
	state_lock.unlock();

	Packet *p = Packet::alloc();
	createRxPacket(p);
	std::function<void(UwModem &, Packet * p)> callback =
			&UwModem::recv;
	ModemEvent e = {callback, p};
	event_q.push(e);
	// recv(p);

}

void
UwModemCSA::createRxPacket(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	uwalh->binPktLength() = rx_payload.size();
	std::memset(uwalh->binPkt(), 0, uwalh->binPktLength());
	std::copy(rx_payload.begin(), rx_payload.end(), uwalh->binPkt());
	HDR_CMN(p)->direction() = hdr_cmn::UP;
}
