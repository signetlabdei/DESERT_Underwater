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

#include <ahoitypes.h>
#include <clmessage.h>
#include <hdr-uwal.h>
#include <mphy_timers.h>
#include <uwahoimodem.h>
#include <uwal.h>
#include <uwphy-clmsg.h>
#include <uwserial.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>

const std::chrono::milliseconds UwAhoiModem::MODEM_TIMEOUT =
		std::chrono::milliseconds(3600);

uint UwAhoiModem::WAIT_DELIVERY_INT = 3000;

uint UwAhoiModem::MAX_RETX = 0;

uint8_t UwAhoiModem::sn = 0;

/**
 * Class to create the Otcl shadow object for an object of the class
 * UwAhoiModem.
 */
static class UwAhoiModem_TclClass : public TclClass
{

public:
	UwAhoiModem_TclClass()
		: TclClass("Module/UW/UwModem/AHOI")
	{
	}

	TclObject *
	create(int args, const char *const *argv)
	{
		return (new UwAhoiModem());
	}

} class_ahoimodem;

UwAhoiModem::UwAhoiModem()
	: UwModem()
	, p_connector(new UwSerial())
	, p_interpreter(new UwInterpreterAhoi(id))
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
	, rx_thread()
	, tx_thread()
	, rx_payload("")
	, virtual_time_ref(0.0)
	, WAIT_DELIVERY(std::chrono::milliseconds(3000))
	, tmpPacket{0}
	, id(0)
	, parity_bit(0)
	, stop_bit(0)
	, flow_control(0)
	, baud_rate(115200)
	, power(0)
	, rssi(0)
	, bit_errors(0)
	, agc_mean(0)
	, agc_min(0)
	, agc_max(0)
{
	bind("buffer_size", (int *) &DATA_BUFFER_LEN);
	bind("max_read_size", (int *) &MAX_READ_BYTES);
	bind("parity_bit", (int *) &parity_bit);
	bind("stop_bit", (int *) &stop_bit);
	bind("flow_control", (int *) &flow_control);
	bind("baud_rate", (int *) &baud_rate);
	bind("modem_id", (int *) &id);
	bind("max_n_retx", (int *) &MAX_RETX);
	bind("wait_delivery", (uint *) &UwAhoiModem::WAIT_DELIVERY_INT);
}

UwAhoiModem::~UwAhoiModem()
{
	stop();
}

void
UwAhoiModem::recv(Packet *p)
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

		std::unique_lock<std::mutex> tx_lock(tx_queue_m);
		tx_queue.push(p);
		tx_lock.unlock();
		printOnLog(LogLevel::DEBUG, "AHOIMODEM", "recv::PUSHING_IN_TX_QUEUE");
		tx_queue_cv.notify_one();
	}
}

int
UwAhoiModem::command(int argc, const char *const *argv)
{
	return UwModem::command(argc, argv);
}

int
UwAhoiModem::recvSyncClMsg(ClMessage *m)
{
	return 0;
}

void
UwAhoiModem::startTx(Packet *p)
{
	ahoi::packet_t ahoiPkt = {0};
	ahoiPkt = fillAhoiPkt(p);

	std::string cmd_s = p_interpreter->buildSend(ahoiPkt);

	// write the obtained command to the device, through the connector
	std::unique_lock<std::mutex> tx_state_lock(tx_status_m);

	if (!tx_status_cv.wait_for(tx_state_lock, WAIT_DELIVERY, [&]
	    { return tx_status == TransmissionState::TX_IDLE;	})) {

		tx_status = TransmissionState::TX_IDLE;
		printOnLog(LogLevel::DEBUG, "AHOIMODEM",
		    "startTx::FORCING_TX_STATUS_IDLE");
	}

	tx_status = TransmissionState::TX_WAITING;

	printOnLog(LogLevel::DEBUG, "AHOIMODEM", "startTx::SENDING_PACKET");
	if ((p_connector->writeToDevice(cmd_s)) < 0) {
		printOnLog(LogLevel::ERROR,
				"AHOIMODEM",
				"startTx::FAIL_TO_WRITE_TO_DEVICE::[" + cmd_s + "]");
		return;
	}

	// retry if the tx_status does not appear to slip to TX_IDLE
	std::chrono::milliseconds WAIT_DELIVERY(WAIT_DELIVERY_INT);
	uint n_retx = 1;
	while (n_retx <= MAX_RETX &&
			!(tx_status_cv.wait_for(tx_state_lock, WAIT_DELIVERY, [&] {
				return tx_status == TransmissionState::TX_IDLE;
			}))) {

		printOnLog(LogLevel::DEBUG,
		    "AHOIMODEM",
			"startTx::SENDING_PACKET[" + std::to_string(n_retx) + "]");

		tx_status = TransmissionState::TX_WAITING;

		if ((p_connector->writeToDevice(cmd_s)) < 0) {
			printOnLog(LogLevel::ERROR,
					"AHOIMODEM",
					"startTx::FAIL_TO_WRITE_TO_DEVICE::[" + cmd_s + "]");
			return;
		}
		n_retx++;
	}
}

ahoi::packet_t
UwAhoiModem::fillAhoiPkt(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	hdr_mac* mach = HDR_MAC(p);

	ahoi::packet_t packet = {0};

	std::string payload(uwalh->binPkt(), uwalh->binPktLength());

	ahoi::header_t header;
	header.src = (unsigned int)modemID;
	header.dst = mach->macDA();
	header.type = ahoi::commands_id[ahoi::Command::send];
	header.status = ahoi::ACK_NONE;
	header.dsn = sn;
	header.len = static_cast<uint8_t>(payload.length());

	packet.header = header;
	for (uint i = 0; i < payload.size(); i++) {
		packet.payload[i] = (uint8_t) payload[i];
	}

	return packet;
}

void
UwAhoiModem::startRx(Packet *p)
{
	printOnLog(LogLevel::INFO, "UWAHOIMODEM", "startRx::PHY2MACSTARTRX");
	Phy2MacStartRx(p);
}

void
UwAhoiModem::endRx(Packet *p)
{
	printOnLog(LogLevel::INFO, "UWAHOIMODEM", "endRx::SENDUP");
	sendUp(p, 0.01);
}

void
UwAhoiModem::start()
{
	if (modem_address == "") {
		printOnLog(LogLevel::ERROR, "AHOIMODEM", "start::PORT_NOT_SET");
		return;
	}

	//generate complete path with parameters for serial connection
	if (parity_bit)
		modem_address = modem_address + ":p=1";
	else
		modem_address = modem_address + ":p=0";

	if (stop_bit)
		modem_address = modem_address + ":s=1";
	else
		modem_address = modem_address + ":s=0";

	if (flow_control)
		modem_address = modem_address + ":f=1";
	else
		modem_address = modem_address + ":f=0";

	if (baud_rate > 0)
		modem_address = modem_address + ":b=" + std::to_string(baud_rate);
	else {
		printOnLog(
		    LogLevel::ERROR, "AHOIMODEM", "start::BAUD_RATE_NOT_VALID");
		exit(1);
	}


	if (!p_connector->openConnection(modem_address)) {
		printOnLog(
		    LogLevel::ERROR, "AHOIMODEM", "start::CONNECTION_OPEN_FAILED");
		exit(1);
	}

	// set flags to true so loops can start
	receiving.store(true);
	transmitting.store(true);

	// spawn off threads
	rx_thread = std::thread(&UwAhoiModem::receivingData, this);
	tx_thread = std::thread(&UwAhoiModem::transmittingData, this);

	checkTimer = new CheckTimer(this);
	checkTimer->resched(period);

	printOnLog(LogLevel::INFO, "AHOIMODEM", "start::STARTING_OPERATIONS");
}

void
UwAhoiModem::stop()
{
	printOnLog(LogLevel::INFO, "AHOIMODEM", "stop::CLOSING_CONNECTION");

	receiving.store(false);
	transmitting.store(false);
	tx_queue_cv.notify_one();
	if (tx_thread.joinable())
		tx_thread.join();
	if (p_connector->isConnected() && !p_connector->closeConnection()) {
		printOnLog(LogLevel::ERROR, "AHOIMODEM", "stop::CONNECTION_CLOSE_FAIL");
	}
	if (rx_thread.joinable()) {
		rx_thread.join();
	}

	checkTimer->force_cancel();
}

void
UwAhoiModem::transmittingData()
{
	while (transmitting.load()) {

		std::unique_lock<std::mutex> tx_q_lock(tx_queue_m);
		tx_queue_cv.wait(
		    tx_q_lock, [&] { return !tx_queue.empty() || !transmitting; });
		if (!transmitting) {
			break;
		}

		Packet *pck = tx_queue.front();
		tx_queue.pop();
		tx_q_lock.unlock();

		if (pck) {

			std::unique_lock<std::mutex> state_lock(status_m);
			if (!status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
					return status == ModemState::AVAILABLE;
				})) {

				status = ModemState::AVAILABLE;
				printOnLog(LogLevel::ERROR,
						"AHOIMODEM",
						"transmittingData::FORCING_MODEM_AVAILABILITY");
			}

			status = ModemState::TRANSMITTING;
			state_lock.unlock();
			startTx(pck);

		} else {
			printOnLog(LogLevel::DEBUG, "AHOIMODEM",
			    "transmittingData::INVALID_PACKET_RETRIEVED");
		}

		updateSN();

		// schedule call to endTx in events queue
		std::function<void(UwModem &, Packet * p)> callback =
				&UwModem::realTxEnded;
		ModemEvent e = {callback, pck};
		event_q.push(e);

		printOnLog(LogLevel::DEBUG, "AHOIMODEM",
		    "transmittingData::BLOCKING_ON_NEXT_PACKET");
	}
}

void
UwAhoiModem::receivingData()
{
	data_buffer.resize(DATA_BUFFER_LEN);
	std::fill(data_buffer.begin(), data_buffer.end(), '\0');
	// iterators that keep track of read/write operations
	std::vector<char>::iterator beg_it = data_buffer.begin();
	std::vector<char>::iterator end_it = data_buffer.begin();
	// iterators that keep track of commands research
	std::vector<char>::iterator cmd_b = data_buffer.begin();
	std::vector<char>::iterator cmd_e = data_buffer.begin();
	int r_bytes = 0; // bytes read from the device interface
	int offset = 0; // offset to not overwrite unparsed data

	std::shared_ptr<ahoi::packet_t> pck = std::make_shared<ahoi::packet_t>();

	while (receiving.load()) {

		r_bytes = p_connector->readFromDevice(
				&(*end_it), MAX_READ_BYTES - offset);

		if (r_bytes < 0) {
			if (p_connector->getErrno() != 0) {
				printOnLog(LogLevel::ERROR,
						"AHOIMODEM",
						strerror(p_connector->getErrno()));
			}
			r_bytes = 0;
		}

		end_it += r_bytes;

		while (p_interpreter->findResponse(
		    beg_it, end_it, cmd_b, cmd_e) != "") {

			offset = std::distance(cmd_e, end_it);

			p_interpreter->fixEscapes(data_buffer, cmd_b, cmd_e);

			if ((pck = p_interpreter->parseResponse(cmd_b, cmd_e)) != nullptr) {

				updateStatus(pck);
				printOnLog(LogLevel::DEBUG,
						   "AHOIMODEM",
						   "receivingData::RX_MSG=" + std::string(cmd_b, cmd_e));
			}

			offset = std::distance(cmd_e, end_it);
			std::copy(cmd_e, end_it, beg_it);
			std::fill(beg_it + offset, end_it, '\0');
			end_it = beg_it + offset;

			pck.reset();
		}
	}
}

void
UwAhoiModem::createRxPacket(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	uwalh->binPktLength() = rx_payload.size();
	std::memset(uwalh->binPkt(), 0, uwalh->binPktLength());
	std::copy(rx_payload.begin(), rx_payload.end(), uwalh->binPkt());
	HDR_CMN(p)->direction() = hdr_cmn::UP;
	rx_payload = "";  // clean up the rx payload string
}

bool
UwAhoiModem::storePacketInfo(std::shared_ptr<ahoi::packet_t> packet, Packet *p)
{
	ahoi::header_t header = packet->header;
	ahoi::footer_t footer = packet->footer;

	hdr_uwal *uwalh = HDR_UWAL(p);
	uwalh->srcID() = header.src;

	if (header.type == 0x00) {

		rx_payload = "";
		for (int i = 0; i < header.len; i++) {
			rx_payload += packet->payload[i];
		}
		power = footer.power;
		rssi = footer.rssi;
		bit_errors = footer.biterrors;
		agc_mean = footer.agcMean;
		agc_min = footer.agcMin;
		agc_max = footer.agcMax;

		printOnLog(LogLevel::INFO,
			"AHOIMODEM", "storePacketInfo::PAYLOAD[" + rx_payload + "]");
	}

	return true;
}

bool
UwAhoiModem::parseFooter(std::shared_ptr<ahoi::footer_t> footer)
{
	power = footer->power;
	rssi = footer->rssi;
	bit_errors = footer->biterrors;
	agc_mean = footer->agcMean;
	agc_min = footer->agcMin;
	agc_max = footer->agcMax;

	return true;
}

void
UwAhoiModem::updateStatus(std::shared_ptr<ahoi::packet_t> packet)
{

	uint8_t packet_type = (packet->header).type;
	auto cmd = std::find_if(ahoi::commands_id.begin(),
			ahoi::commands_id.end(),
			[&packet_type](const std::pair<ahoi::Command, uint8_t> &p) {
				return p.second == packet_type;
			});

	if (cmd == ahoi::commands_id.end()) {
		printOnLog(LogLevel::ERROR,
				"AHOIMODEM",
				"updateStatus::UNKNOWN_COMMAND_RECEIVED");
	}

	std::string dbg_str = std::string("updateStatus::HEADER::") +
			"[SRC::" + std::to_string((packet->header).src) +
			"][DST::" + std::to_string((packet->header).dst) +
			"][TYP::" + std::to_string((packet->header).type) +
			"][DSN::" + std::to_string((packet->header).dsn) +
		    "][LEN::" + std::to_string((packet->header).len) + "]";
	printOnLog(LogLevel::DEBUG, "AHOIMODEM", dbg_str);

	std::lock(status_m, tx_status_m);
	std::unique_lock<std::mutex> state_lock(status_m, std::adopt_lock);
	std::unique_lock<std::mutex> tx_state_lock(tx_status_m, std::adopt_lock);

	// Set the modem status to available, then proceed to check the cmd type
	status = ModemState::AVAILABLE;
	status_cv.notify_all();
	state_lock.unlock();

	switch (cmd->first) {
		case ahoi::Command::send: {

			Packet *p = Packet::alloc();

			if (packet->header.dst == modemID || packet->header.dst == 0xFF) {

			  storePacketInfo(packet, p);
			  createRxPacket(p);

			  std::function<void(UwModem &, Packet * p)> callback =
				&UwModem::recv;
			  ModemEvent e = {callback, p};
			  event_q.push(e);

			}

			break;

		}
		case ahoi::Command::confirm: {
			if ((packet->header).src == modemID && (packet->header).dsn == sn) {

				tx_status = TransmissionState::TX_IDLE;
				tx_status_cv.notify_all();
				tx_state_lock.unlock();
			} else {
				std::string err_str = "WRONG_RESPONSE::[" +
						p_interpreter->serializePacket(packet.get()) + "]";
				printOnLog(LogLevel::ERROR, "AHOIMODEM", err_str);
			}
			break;
		}
		default: {
			printOnLog(LogLevel::ERROR, "AHOIMODEM",
			    "updateStatus::UNMANAGED_CMD_TYPE");
		}
	}
}

void
UwAhoiModem::updateSN()
{
	sn++;
	printOnLog(LogLevel::ERROR, "AHOIMODEM",
	    "updateSN::CURRENT_SEQ_NUM::[" + std::to_string(sn) + "]");
}
