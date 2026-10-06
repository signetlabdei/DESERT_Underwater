//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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

#include "uwlumaxuvmodem.h"
#include "phymac-clmsg.h"
#include "uwmodem.h"
#include "uwsocket.h"

#include <curl/curl.h>
#include <regex>

const std::chrono::milliseconds UwLumaXUVModem::MODEM_TIMEOUT =
		std::chrono::milliseconds(3000);

UwLumaXUVModem::UwLumaXUVModem()
	: UwModem()
	, status_m()
	, status(ModemState::AVAILABLE)
	, tx_queue_m()
	, tx_queue_cv()
	, receiving(false)
	, transmitting(false)
	, rx_payload("")
	, sig_thread()
	, rx_thread()
	, tx_thread()
	, modem_address("")
	, send_conn(new UwSocket())
	, recv_conn(new UwSocket())
	, data_address("")
	, signal_buffer()
	, signal_tag("DRIVER")
	, premodulation(0)
{
	send_conn->setUDP();
	send_conn->setMulticast();

	recv_conn->setUDP();
	recv_conn->setMulticast();
	// sets recv_conn in reception mode (server receives data)
	recv_conn->setServer();

	data_buffer.clear();
}

UwLumaXUVModem::~UwLumaXUVModem()
{
	stop();
}

void
UwLumaXUVModem::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);

	if (ch->direction() == hdr_cmn::UP) {
		if (isOn) {
			startRx(p);
			endRx(p);
		} else {
			Packet::free(p);
		}
	} else { // hdr_cmn::DOWN
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
		printOnLog(
				LogLevel::DEBUG, "LUMAXUVMODEM", "recv::PUSHING_IN_TX_QUEUE");
		tx_queue_cv.notify_one();
	}
	return;
}

int
UwLumaXUVModem::command(int argc, const char *const *argv)
{
	if (argc == 3) {
		if (!strcmp(argv[1], "setModemAddress")) {
			modem_address = argv[2];
			return TCL_OK;
		}
		// local network interface address that the modem will send multicast
		// messages to
		if (!strcmp(argv[1], "setLocalAddress")) {
			data_address = argv[2];
			return TCL_OK;
		}
		if (!strcmp(argv[1], "setMulticastAddress")) {
			if (send_conn->setMulticastAddress(argv[2]) &&
					recv_conn->setMulticastAddress(argv[2])) {
				return TCL_OK;
			}
		}
		// modem settings
		if (!strcmp(argv[1], "start_state")) {
			if (std::stoi(argv[2]) == 0 || std::stoi(argv[2]) == 1) {
				configure("parameters", "start_state", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "optical_speed")) {
			if (std::stoi(argv[2]) == 1 || std::stoi(argv[2]) == 4 ||
					std::stoi(argv[2]) == 6 || std::stoi(argv[2]) == 8 ||
					std::stoi(argv[2]) == 10) {
				configure("parameters", "optical_speed", argv[2]);
				return TCL_OK;
			} else {

				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "nb_led")) {
			if (std::stoi(argv[2]) >= 1 && std::stoi(argv[2]) <= 5) {
				configure("parameters", "nb_led", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "encoding")) {
			if (std::stoi(argv[2]) == 0 || std::stoi(argv[2]) == 1) {
				configure("parameters", "encoding", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "led_tx_pwr_normal")) {
			if (std::stoi(argv[2]) >= 0 && std::stoi(argv[2]) <= 100) {
				configure("parameters", "led_tx_pwr_normal", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "led_tx_pwr_cooldown")) {
			if (std::stoi(argv[2]) >= 0 && std::stoi(argv[2]) <= 100) {
				configure("parameters", "led_tx_pwr_cooldown", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "auto_gain_control")) {
			if (std::stoi(argv[2]) == 0 || std::stoi(argv[2]) == 1) {
				configure("parameters", "auto_gain_control", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "manual_gain")) {
			if (std::stoi(argv[2]) >= 0 && std::stoi(argv[2]) <= 4000) {
				configure("parameters", "manual_gain", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "nb_receivers")) {
			if (std::stoi(argv[2]) >= 1 && std::stoi(argv[2]) <= 4) {
				configure("parameters", "nb_receivers", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "agc_amplitude_adjusti")) {
			if (std::stoi(argv[2]) >= 0 && std::stoi(argv[2]) <= 4000) {
				configure("parameters", "agc_amplitude_adjusti", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "keep_alive_pkt")) {
			if (std::stoi(argv[2]) == 0 || std::stoi(argv[2]) == 1) {
				configure("parameters", "keep_alive_pkt", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "autogain_speed")) {
			if (std::stoi(argv[2]) >= 0 && std::stoi(argv[2]) <= 2) {
				configure("parameters", "autogain_speed", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "signal_filtering_type")) {
			if (std::stoi(argv[2]) >= 0 && std::stoi(argv[2]) <= 3) {
				configure("parameters", "signal_filtering_type", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "auto_power")) {
			if (std::stoi(argv[2]) == 0 || std::stoi(argv[2]) == 1) {
				configure("parameters", "auto_power", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
		if (!strcmp(argv[1], "auto_speed")) {
			if (std::stoi(argv[2]) == 0 || std::stoi(argv[2]) == 1) {
				configure("parameters", "auto_speed", argv[2]);
				return TCL_OK;
			} else {
				return TCL_ERROR;
			}
		}
	}
	return UwModem::command(argc, argv);
}

int
UwLumaXUVModem::getModulationType(Packet *P)
{
	return 0;
}

double
UwLumaXUVModem::getTxDuration(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	double tx_duration = -1;

	// With Flexframe modulation, the TX duration obtained
	// dividing the number of samples written by the sampling frequency
	// (192 kHz by default).
	// It can be written as a linear function of the packet size:
	// tx_duration = a + b * pkt_size
	// The two constants a = 0.560833333 [s] and b = 0.013333333 [s]
	// are obtained by interpolating the TX duration values obtained
	// with the smallest and biggest packet size possible.
	tx_duration = 0.560833333 + 0.013333333 * (uwalh->binPktLength());

	if (premodulation)
		tx_duration += 0.1;

	return tx_duration;
}

int
UwLumaXUVModem::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_MAC2PHY_GETTXDURATION) {
		Packet *p = ((ClMsgMac2PhyGetTxDuration *) m)->pkt;
		hdr_cmn *ch = HDR_CMN(p);

		if (ch->direction() == hdr_cmn::DOWN) {
			double duration = getTxDuration(p);

			if (duration > 0)
				printOnLog(LogLevel::INFO,
						"LUMAXUVMODEM",
						"recvSyncClMsg::GET_TXDURATION " +
								std::to_string(duration));

			((ClMsgMac2PhyGetTxDuration *) m)->setDuration(duration);

			return 0;
		}

		// Don't set TX duration on RX.
		((ClMsgMac2PhyGetTxDuration *) m)->setDuration(-1);

		return 1;
	}

	return MPhy::recvSyncClMsg(m);
}
// ----------------------------------------
// Helper callback required by libcurl to write the GET response into a
// std::string
static size_t
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string *) userp)->append((char *) contents, size * nmemb);
	return size * nmemb;
}

bool
UwLumaXUVModem::configure(
		std::string endpoint, std::string param_name, std::string param_value)
{
	int value = 0;

	try {
		value = stoi(param_value);
	} catch (std::invalid_argument &e) {
		printOnLog(UwModem::LogLevel::ERROR,
				"UWLUMAXUVMODEM",
				"invalid value for parameter '" + param_name +
						"': " + param_value);
		return false; // Changed from -1 to false to match the bool return type
	}

	curl_global_init(CURL_GLOBAL_ALL);
	CURL *curl = curl_easy_init();

	if (curl) {
		std::string url = "http://" + std::string(modem_address) + "/api/" +
				endpoint + ".json";

		printOnLog(UwModem::LogLevel::DEBUG,
				"UWLUMAXUVMODEM",
				"request url: " + url);

		// ---------------------------------------------------------
		// 1. PERFORM GET REQUEST
		// ---------------------------------------------------------
		std::string get_response;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); // Specify GET request
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &get_response);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			printOnLog(UwModem::LogLevel::ERROR,
					"UWLUMAXUVMODEM",
					"failed to connect to the modem for configuration. Check "
					"the address or the modem state. Error: " +
							std::string(curl_easy_strerror(res)));
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return false;
		}

		// CHECK 1: does the parameter exists in the json?
		std::regex key_pattern("\"" + param_name + "\"\\s*:");
		if (!std::regex_search(get_response, key_pattern)) {
			printOnLog(UwModem::LogLevel::ERROR,
					"UWLUMAXUVMODEM",
					"parameter '" + param_name +
							"' does not exist or wrong endpoint '" + endpoint +
							"' specified. Check the LumaXUV manual for more "
							"informations.");
			curl_easy_cleanup(curl);
			return false; // returns an error if parameter does not exist
		}

		// CHECK 2: Check if the parameter is already set to the desired value
		// Using regex to handle potential spacing differences in the JSON
		// response (e.g., "key": 1 vs "key":1)
		std::regex value_pattern(
				"\"" + param_name + "\"\\s*:\\s*" + param_value + "\\b");
		if (std::regex_search(get_response, value_pattern)) {
			printOnLog(UwModem::LogLevel::DEBUG,
					"UWLUMAXUVMODEM",
					"parameter '" + param_name + "' is already set to " +
							param_value + ". Skipping POST request.");
			curl_easy_cleanup(curl);
			return true;
		}
		// ---------------------------------------------------------
		// 2. PERFORM POST REQUEST (If payload did not match)
		// ---------------------------------------------------------
		std::string json_data = "{\"" + param_name + "\":" + param_value + "}";

		// Reset the write function so we don't accidentally append the POST
		// response to our GET string
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

		// Set the HTTP Header to tell the server we're sending JSON
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");

		// Set POST options
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl,
				CURLOPT_POSTFIELDS,
				json_data.c_str()); // Automatically switches libcurl to POST

		// Perform the POST request
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK) {
			printOnLog(UwModem::LogLevel::ERROR,
					"UWLUMAXUVMODEM",
					"POST request for " + param_name +
							" failed: " + curl_easy_strerror(res));
		} else {
			printOnLog(UwModem::LogLevel::DEBUG,
					"UWLUMAXUVMODEM",
					"POST request for " + param_name + "successfully sent.");
		}

		// Cleanup
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return true;
}

void
UwLumaXUVModem::start()
{
	printOnLog(LogLevel::DEBUG, "LUMAXUVMODEM", "STARTING_DRIVER");

	if (!send_conn->openConnection(data_address)) {
		std::string err_msg =
				"SENDING_SOCKET_FAILED_TO_OPEN_WITH_ADDRESS:" + data_address;
		printOnLog(LogLevel::ERROR, "LUMAXUVMODEM", err_msg);
		return;
	} else {
		printOnLog(LogLevel::INFO,
				"LUMAXUVMODEM",
				"SENDING_SOCKET_CONNECTION_ESTABLISHED:" + data_address);
	}

	if (!recv_conn->openConnection(data_address)) {
		std::string err_msg =
				"RECEIVING_SOCKET_FAILED_TO_OPEN_WITH_ADDRESS:" + data_address;
		printOnLog(LogLevel::ERROR, "LUMAXUVMODEM", err_msg);
		return;
	} else {
		printOnLog(LogLevel::INFO,
				"LUMAXUVMODEM",
				"RECEIVING_SOCKET_CONNECTION_ESTABLISHED:" + data_address);
	}
	// set flags to true so loops can start
	receiving.store(true);
	transmitting.store(true);

	// Dispatch threads
	rx_thread = std::thread(&UwLumaXUVModem::receivingData, this);
	tx_thread = std::thread(&UwLumaXUVModem::transmittingData, this);

	checkTimer = new CheckTimer(this);
	checkTimer->resched(period);
}

void
UwLumaXUVModem::stop()
{
	receiving.store(false);
	transmitting.store(false);

	status_cv.notify_all();
	tx_queue_cv.notify_all();

	if (tx_thread.joinable())
		tx_thread.join();

	if (send_conn->isConnected() && !send_conn->closeConnection())
		printOnLog(LogLevel::ERROR,
				"LUMAXUVMODEM",
				"SENDING_SOCKET_CONNECTION_UNABLE_TO_CLOSE");

	if (recv_conn->isConnected() && !recv_conn->closeConnection())
		printOnLog(LogLevel::ERROR,
				"LUMAXUVMODEM",
				"RECEIVING_SOCKET_CONNECTION_UNABLE_TO_CLOSE");

	if (sig_thread.joinable())
		sig_thread.join();
	if (rx_thread.joinable())
		rx_thread.join();

	if (checkTimer) {
		checkTimer->force_cancel();
	}
}

void
UwLumaXUVModem::receivingData()
{
	data_buffer.resize(DATA_BUFFER_LEN, '\0');
	std::fill(data_buffer.begin(), data_buffer.end(), '\0');

	while (receiving.load()) {
		auto beg_it = data_buffer.begin();
		int r_bytes = recv_conn->readFromDevice(&(*beg_it), MAX_READ_BYTES);

		if (r_bytes > 0) {
			std::unique_lock<std::mutex> state_lock(status_m);
			status = ModemState::RECEIVING;
			state_lock.unlock();

			printOnLog(UwModem::LogLevel::DEBUG,
					"LUMAXUV",
					"read " + std::to_string(r_bytes) + " bytes");

			rx_payload = std::string(beg_it, beg_it + r_bytes);
			printOnLog(LogLevel::DEBUG,
					"LUMAXUVMODEM",
					"LEN::" + std::to_string(rx_payload.size()) +
							"::DATA::" + rx_payload);

			Packet *p = Packet::alloc();
			createRxPacket(p);
			std::function<void(UwModem &, Packet * p)> callback =
					&UwModem::recv;
			ModemEvent e = {callback, p};
			event_q.push(e);

			state_lock.lock();
			status = ModemState::AVAILABLE;
			status_cv.notify_all();

		} else {
			printOnLog(UwModem::LogLevel::DEBUG,
					"LUMAXUVMODEM",
					"failed to read from device");
		}
	}
}

void
UwLumaXUVModem::createRxPacket(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	uwalh->binPktLength() = rx_payload.size();
	std::memset(uwalh->binPkt(), 0, uwalh->binPktLength());
	std::copy(rx_payload.begin(), rx_payload.end(), uwalh->binPkt());
	HDR_CMN(p)->direction() = hdr_cmn::UP;
}

void
UwLumaXUVModem::transmittingData()
{
	printOnLog(
			UwModem::LogLevel::DEBUG, "LUMAXUVMODEM", "STARTING_TRANSMISSION");

	while (transmitting.load()) {
		std::unique_lock<std::mutex> tx_lock(tx_queue_m);
		tx_queue_cv.wait(
				tx_lock, [&] { return !tx_queue.empty() || !transmitting; });

		if (!transmitting.load())
			break;

		Packet *pck = tx_queue.front();
		tx_queue.pop();
		tx_lock.unlock();
		if (pck) {
			startTx(pck);
		}

		printOnLog(LogLevel::DEBUG,
				"LUMAXUVMODEM",
				"transmittingData::BLOCKING_ON_NEXT_PACKET");
	}
}

void
UwLumaXUVModem::startTx(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	std::string payload;
	payload.assign(uwalh->binPkt(), uwalh->binPktLength());

	std::unique_lock<std::mutex> state_lock(status_m);
	if (status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
			return status == ModemState::AVAILABLE;
		})) {

		status = ModemState::TRANSMITTING;
		state_lock.unlock();

		int temp{0};
		if ((temp = send_conn->writeToDevice(payload)) < 0) {
			printOnLog(LogLevel::ERROR,
					"LUMAXUVMODEM",
					"startTx::FAIL_TO_WRITE_DATA_TO_DEVICE");

			state_lock.lock();
			status = ModemState::AVAILABLE;
			status_cv.notify_all();
			return;
		}

		std::function<void(UwModem &, Packet * p)> callback =
				&UwModem::realTxEnded;
		ModemEvent e = {callback, p};
		event_q.push(e);

		printOnLog(
				LogLevel::INFO, "LUMAXUVMODEM", "startTx::PACKET_TRANSMITTED");

		state_lock.lock();
		status = ModemState::AVAILABLE;
		status_cv.notify_all();
	}
}

void
UwLumaXUVModem::startRx(Packet *p)
{
	printOnLog(LogLevel::INFO, "LUMAXUVMODEM", "startRx::CALL_PHY2MACSTARTRX");
	Phy2MacStartRx(p);
}

void
UwLumaXUVModem::endRx(Packet *p)
{
	printOnLog(LogLevel::INFO, "LUMAXUVMODEM", "endRx::CALL_SENDUP");
	sendUp(p, 0.01);
}
