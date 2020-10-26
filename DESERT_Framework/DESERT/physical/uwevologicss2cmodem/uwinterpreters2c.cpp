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

#include <uwinterpreters2c.h>

#include <algorithm>
#include <iostream>
#include <sstream>

std::vector<std::pair<std::string, UwInterpreterS2C::Response> >
		UwInterpreterS2C::syntax_pool{std::make_pair("RECVIM,", Response::RECVIM),
				std::make_pair("RECV,", Response::RECV),
				std::make_pair("OK", Response::OK),
				std::make_pair("EMPTY", Response::EMPTY),
				std::make_pair("BUSY", Response::BUSY),
				std::make_pair("DELIVERING", Response::DELIVERING),
				std::make_pair("DELIVERED", Response::DELIVERED),
				std::make_pair("DROPCNT", Response::DROPCNT),
				std::make_pair("ERROR PHY OFF", Response::PHYOFF),
				std::make_pair("ERROR NOT ACCEPTED", Response::NOT_ACCEPTED),
				std::make_pair(
						"ERROR WRONG DESTINATION ADDRESS", Response::WRONG_ADDR),
				std::make_pair("ERROR CONNECTION CLOSED", Response::CONN_CLOSED),
				std::make_pair("ERROR UNKNOWN COMMAND", Response::UNKNOWN),
				std::make_pair("ERROR WRONG FORMAT", Response::UNKNOWN),
				std::make_pair(
						"ERROR BUFFER IS NOT EMPTY", Response::BUFF_NOT_EMPTY),
				std::make_pair("ERROR BUFFER FULL", Response::BUFFER_FULL),
				std::make_pair("ERROR OUT OF RANGE", Response::OUT_OF_RANGE),
				std::make_pair("ERROR PROTOCOL ID", Response::PROTOCOL_ID),
				std::make_pair("ERROR INTERNAL", Response::INTERNAL),
				std::make_pair("FAILED", Response::FAIL),
	            std::make_pair("Source Level:", Response::CURR_SETTINGS),
		        std::make_pair("Remote Address:", Response::MODEM_STATUS),
				std::make_pair("INITIATION NOISE", Response::INIT_NOISE),
				std::make_pair("INITIATION DEAF", Response::INIT_DEAF),
				std::make_pair("INITIATION LISTEN", Response::INIT_LISTEN),
				std::make_pair("RECVSTART", Response::RECVSTART),
				std::make_pair("RECVEND", Response::RECVEND),
				std::make_pair("RECVFAILED", Response::RECVFAIL),
		        std::make_pair("SENDSTART", Response::SENDSTART),
				std::make_pair("SENDEND", Response::SENDEND),
				std::make_pair("BITRATE", Response::BITRATE)};

UwInterpreterS2C::UwInterpreterS2C()
	: sep(",")
	, r_term("\r\n")
	, w_term("\n")
{
}

UwInterpreterS2C::~UwInterpreterS2C()
{
}

std::string
UwInterpreterS2C::buildSend(std::string msg, int dest)
{
	std::string base_cmd = "AT*SEND";
	std::string length = std::to_string(msg.size());
	std::string destination = std::to_string(dest);

	std::string cmd =
			base_cmd + sep + length + sep + destination + sep + msg + w_term;

	return cmd;
}

std::string
UwInterpreterS2C::buildSendIM(std::string msg, int dest, bool ack)
{
	std::string base_cmd = "AT*SENDIM";
	std::string length = std::to_string(msg.size());
	std::string destination = std::to_string(dest);

	std::string cmd;
	if (ack) {
		cmd = base_cmd + sep + length + sep + destination + sep + "ack" + sep +
				msg + w_term;
	} else {
		cmd = base_cmd + sep + length + sep + destination + sep + "noack" +
				sep + msg + w_term;
	}

	return cmd;
}

std::string
UwInterpreterS2C::buildATZ(int level)
{
	if (level < 0 || level > 4) {
		std::string err_msg = "ERROR::RESET_LEVEL_OUT_OF_BOUNDS";
		return "";
	}
	std::string base_cmd = "ATZ";
	std::string lev = std::to_string(level);

	std::string cmd = base_cmd + lev + w_term;

	return cmd;
}

std::string
UwInterpreterS2C::buildATDI()
{
	std::string cmd = "AT?DI" + w_term;
	return cmd;
}

std::string
UwInterpreterS2C::buildATS()
{
	std::string cmd = "AT?S" + w_term;
	return cmd;
}

std::string
UwInterpreterS2C::buildATV()
{
	std::string cmd = "AT&V" + w_term;
	return cmd;
}

std::string
UwInterpreterS2C::buildGetATL()
{
	std::string base_cmd = "AT?L";
	std::string cmd = base_cmd + w_term;

	return cmd;
}

std::string
UwInterpreterS2C::buildSetATL(int level)
{
	if (level < 0 || level > 4) {
		std::string err_msg = "ERROR::SOURCE_LEVEL_OUT_OF_BOUNDS";
		return "";
	}
	std::string base_cmd = "AT!L";
	std::string sl = std::to_string(level);

	std::string cmd = base_cmd + sl + w_term;

	return cmd;
}

std::string
UwInterpreterS2C::buildGetATAL()
{
	std::string base_cmd = "AT?AL";

	std::string cmd = base_cmd + w_term;

	return cmd;
}

std::string
UwInterpreterS2C::buildSetATAL(int addr)
{
	if (addr <= 0 || addr >= 255) {
		std::string err_msg = "ERROR::LOCAL_ADDRESS_OUT_OF_BOUNDS";
		return "";
	}
	std::string base_cmd = "AT!AL";
	std::string address = std::to_string(addr);

	std::string cmd = base_cmd + address + w_term;

	return cmd;
}

UwInterpreterS2C::Response
UwInterpreterS2C::findResponse(std::vector<char>::iterator beg,
		std::vector<char>::iterator end, std::vector<char>::iterator &rsp)
{

	Response cmd = Response::NO_COMMAND;
	std::vector<char>::iterator first = end;

	for (uint i = 0; i < syntax_pool.size(); i++) {

		auto it = std::search(beg,
				end,
				syntax_pool[i].first.begin(),
				syntax_pool[i].first.end());

		if (it < first) {
			first = it;
			cmd = syntax_pool[i].second;
		}

		rsp = first;
	}

	return cmd;
}

bool
UwInterpreterS2C::parseResponse(Response rsp, std::vector<char>::iterator end,
		std::vector<char>::iterator rsp_beg,
		std::vector<char>::iterator &rsp_end, std::string &rx_payload)
{
	switch (rsp) {

		case Response::RECVIM: {
			auto it = std::search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
			} else {
				return false;
			}
			// length
			auto curs_b = std::find(rsp_beg, rsp_end, ',') + 1;
			if (curs_b >= end) {
				return false;
			}
			auto curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int len = std::stoi(std::string(curs_b, curs_e));
			// source address
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int sour_a = std::stoi(std::string(curs_b, curs_e));
			// destination address
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int dest_a = std::stoi(std::string(curs_b, curs_e));
			// flag (ACK ro NOT-ACK)
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			std::string ack = std::string(curs_b, curs_e);
			// duration
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int dur = std::stoi(std::string(curs_b, curs_e));
			// rssi
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int rssi = std::stoi(std::string(curs_b, curs_e));
			// integrity
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int integrity = std::stoi(std::string(curs_b, curs_e));
			// velocity
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			std::istringstream velocity_ss(std::string(curs_b, curs_e));
			double velocity;
			velocity_ss >> velocity;
			// payload
			auto payload_beg = std::find(curs_e, rsp_end, ',') + 1;
			if (payload_beg >= end) {
				return false;
			}
			rsp_end = payload_beg + len;

			auto term_beg =
					std::search(rsp_end, end, r_term.begin(), r_term.end());
			if (rsp_end != term_beg) {
				return false;
			}
			rsp_end += r_term.size();
			rx_payload = std::string(payload_beg, rsp_end - r_term.size());
			return true;
		}

		case Response::RECV: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
			} else {
				return false;
			}
			// length
			auto curs_b = std::find(rsp_beg, rsp_end, ',') + 1;
			if (curs_b >= end) {
				return false;
			}
			auto curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int len = std::stoi(std::string(curs_b, curs_e));
			// source address
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int sour_a = std::stoi(std::string(curs_b, curs_e));
			// destination address
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int dest_a = std::stoi(std::string(curs_b, curs_e));
			// bitrate
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int bitrate = std::stoi(std::string(curs_b, curs_e));
			// rssi
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int rssi = std::stoi(std::string(curs_b, curs_e));
			// integrity
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int integrity = std::stoi(std::string(curs_b, curs_e));
			// delay
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			int delay = std::stoi(std::string(curs_b, curs_e));
			// velocity
			curs_b = curs_e + 1;
			if (curs_b >= end) {
				return false;
			}
			curs_e = std::find(curs_b, rsp_end, ',');
			if (curs_e >= end) {
				return false;
			}
			std::istringstream velocity_ss(std::string(curs_b, curs_e));
			double velocity;
			velocity_ss >> velocity;
			// payload
			auto payload_beg = std::find(curs_e, rsp_end, ',') + 1;
			if (payload_beg >= end) {
				return false;
			}
			rsp_end = payload_beg + len;
			auto term_beg =
					std::search(rsp_end, end, r_term.begin(), r_term.end());
			if (rsp_end != term_beg) {
				return false;
			}
			rsp_end += r_term.size();
			rx_payload = std::string(payload_beg, rsp_end - r_term.size());
			return true;
		}

		case Response::OK: {
			auto it = std::search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::EMPTY: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::BUSY: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::CURR_SETTINGS: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
			} else {
				return false;
			}
			// Source Level
			auto curs_b = std::find(rsp_beg, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			auto curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int sl_ = std::stoi(std::string(curs_b, curs_e));
			// Source Level Control
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int sl_control = std::stoi(std::string(curs_b, curs_e));
			// Gain
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int gain = std::stoi(std::string(curs_b, curs_e));
			// Carrier Waveform ID
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int cwID = std::stoi(std::string(curs_b, curs_e));
			// Local Address
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int loc_addr = std::stoi(std::string(curs_b, curs_e));
			// Highest Address
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int loc_addr_MAX = std::stoi(std::string(curs_b, curs_e));
			// Cluster size
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int cluster_size = std::stoi(std::string(curs_b, curs_e));
			// Packet time
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int pck_time = std::stoi(std::string(curs_b, curs_e));
			// Retry count
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int retry_count = std::stoi(std::string(curs_b, curs_e));
			// Retry timeout
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int retry_to = std::stoi(std::string(curs_b, curs_e));
			// Wake up active time
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int wa_time = std::stoi(std::string(curs_b, curs_e));
			// Wake up period
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int wa_period = std::stoi(std::string(curs_b, curs_e));
			// Promiscsous mode
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int promiscous = std::stoi(std::string(curs_b, curs_e));
			// Sound speed
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int sound_speed = std::stoi(std::string(curs_b, curs_e));
			// IM retry count
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int im_retry_count = std::stoi(std::string(curs_b, curs_e));
			// Pool sizes
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			std::string pool_size = std::string(curs_b, curs_e);
			// Hold timeout
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int hold_to = std::stoi(std::string(curs_b, curs_e));
			// Idle timeout
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int idle_to = std::stoi(std::string(curs_b, curs_e));

			return true;
		}

		case Response::DELIVERING: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::DELIVERED: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::FAIL: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::DROPCNT: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::UNKNOWN: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::BUFFER_FULL: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::RECVSTART: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::RECVEND: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::RECVFAIL: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::SENDSTART: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::SENDEND: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::BITRATE: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
				return true;
			} else {
				return false;
			}
		}

		case Response::MODEM_STATUS: {
			auto it = search(rsp_beg, end, r_term.begin(), r_term.end());
			if (it != end) {
				rsp_end = it + r_term.size();
			} else {
				return false;
			}
			// Local Address
			auto curs_b = std::find(rsp_beg, rsp_end, ':') + 2;
			if (curs_b >= end)
				return false;
			auto curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e >= end)
				return false;
			int loc_addr = std::stoi(std::string(curs_b, curs_e));
			// Remote address
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b > end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e > end)
				return false;
			int rem_addr = std::stoi(std::string(curs_b, curs_e));
			// ACoustic Link Status
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b > end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			if (curs_e > end)
				return false;
			std::string ac_status = std::string(curs_b, curs_e);
			// Pool Status: packages and bytes free
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b > end)
				return false;
			curs_e = std::find(curs_b, rsp_end, ' ');
			int n_pck = std::stoi(std::string(curs_b, curs_e));

			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b > end)
				return false;
			curs_e = std::find(curs_b, rsp_end, ' ');
			int bytes_free = std::stoi(std::string(curs_b, curs_e));
			// Whether promiscous mode is ON or OFF
			curs_b = curs_e + 1;
			curs_b = std::find(curs_b, rsp_end, ':') + 2;
			if (curs_b > end)
				return false;
			curs_e = std::find(curs_b, rsp_end, '\n');
			bool is_promiscous = (std::string(curs_b, curs_e) == "ON");

			return true;
		}

		default:
			return false;

	} // end of switch on commands
}
