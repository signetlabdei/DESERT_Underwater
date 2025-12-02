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

#include <uwapplicon-interp.h>

#include <algorithm>
#include <iostream>
#include <sstream>

std::vector<std::pair<std::string, UwAppliconInterpr::Response> >
		UwAppliconInterpr::syntax_pool{
				std::make_pair("RECV,", Response::RECV),
				std::make_pair("\"cmd\": \"msgs\"", Response::MSGS),
				std::make_pair("\"cmd\": \"oook\"", Response::AOK)
			};

UwAppliconInterpr::UwAppliconInterpr()
	: sep(",")
	, r_term("\r\n")
	, w_term("\n")
{
}

UwAppliconInterpr::~UwAppliconInterpr()
{
}

std::string
UwAppliconInterpr::buildSend(std::string msg, int dest)
{
    
	std::cout << "Raw payload received by buildSend: " << msg << std::endl;
	std::cout << "Raw payload length: " << msg.length() << " bytes" << std::endl;

	// Create a stringstream to build the payload byte array
    std::stringstream payload_stream;
    payload_stream << "[";

    // Iterate over each character of the input message (payload)
    for (size_t i = 0; i < msg.length(); ++i) {
        // Convert the character to its integer value (byte)
        payload_stream << static_cast<int>(static_cast<unsigned char>(msg[i]));

        // Add a comma if it is not the last byte
        if (i < msg.length() - 1) {
            payload_stream << ",";
        }
    }
    payload_stream << "]";

    // Build the final JSON string
    std::string cmd = "{\"cmd\": \"send\", \"payload\": " + payload_stream.str() + "}";

    // Note: The new command string is created exactly as required.
    // The original line terminator (w_term) was removed because
    // the b'...' format suggests a literal string.

    return cmd;
}
UwAppliconInterpr::Response
UwAppliconInterpr::findResponse(std::vector<char>::iterator beg,
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
UwAppliconInterpr::parseResponse(Response rsp, std::vector<char>::iterator end,
		std::vector<char>::iterator rsp_beg,
		std::vector<char>::iterator &rsp_end, std::string &rx_payload, bool &integrity)
{
	integrity = true; // Default to true
	switch (rsp) {
		case Response::AOK: {
			// AOK is a JSON object. We look for the closing curly brace '}'.
			auto it = std::find(rsp_beg, end, '}');
			if (it != end) {
				rsp_end = it + 1; // The end of the response is after '}'
				
				// The ACK has been correctly identified.
				// We pass the entire JSON as payload for subsequent processing.
				rx_payload = std::string(rsp_beg, rsp_end);
				return true;
			} else {
				// The JSON is incomplete, more data is needed
				return false;
			}
		}
		case Response::MSGS: {
			// Search for the end of the JSON
			auto json_end_it = std::find(rsp_beg, end, '}');
			if (json_end_it == end) {
				return false; // Incomplete JSON
			}
			
			rsp_end = json_end_it + 1;
			std::string json_str(rsp_beg, rsp_end);
			//std::cout << "DEBUG: Full JSON received: " << json_str << std::endl;

			// Check for crccheck field
			size_t crc_pos = json_str.find("\"crc_check\":");
			if (crc_pos != std::string::npos) {
				// Simple check for "false" after "crccheck"
				size_t false_pos = json_str.find("false", crc_pos);
				size_t true_pos = json_str.find("true", crc_pos);
				
				if (false_pos != std::string::npos && (true_pos == std::string::npos || false_pos < true_pos)) {
					integrity = false;
					std::cout << "DEBUG: crc_check found FALSE" << std::endl;
				} else {
					integrity = true;
					//std::cout << "DEBUG: crc_check found TRUE (or assumed true)" << std::endl;
				}
			} else {
				std::cout << "DEBUG: crc_check field NOT FOUND" << std::endl;
			}

			// Find the start of the payload array: "["
			size_t payload_start_pos = json_str.find("\"payload\": [");
			if (payload_start_pos == std::string::npos) return false;
			payload_start_pos += 12; // Skip '"payload": ['

			// Find the end of the payload array: "]"
			size_t payload_end_pos = json_str.find("]", payload_start_pos);
			if (payload_end_pos == std::string::npos) return false;

			// Extract the substring containing the numbers
			std::string numbers_str = json_str.substr(payload_start_pos, payload_end_pos - payload_start_pos);
			std::cout << "DEBUG: raw payload received by MSGS: " << numbers_str << std::endl;
			
			// Use a stringstream to read the comma-separated numbers
			std::stringstream ss(numbers_str);
			std::string single_number;
			rx_payload.clear(); // Clear the output payload

			while(std::getline(ss, single_number, ',')) {
				try {
					// Convert the number (string) to integer and then to char (byte)
					int byte_val = std::stoi(single_number);
					rx_payload += static_cast<char>(byte_val);
				} catch (const std::invalid_argument& ia) {
					// Handle the error if conversion fails
					return false;
				}
			}
			rx_payload.pop_back(); // remove crc
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
			int integrity_val = std::stoi(std::string(curs_b, curs_e));
			integrity = (integrity_val != 0); // Assuming non-zero is valid

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

		default:
			return false;

	} // end of switch on commands
}
