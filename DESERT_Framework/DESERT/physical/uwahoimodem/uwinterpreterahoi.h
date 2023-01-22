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

/**
 * @file uwinterpreterahoi.h
 * @author Emanuele Coccolo
 * @author Roberto Francescon
 * @version 0.1.0
 * @brief Interpreter for commands and responses for ahoi! modems, TUHH, Hamburg
 */

#ifndef UWINTERPRETERAHOI_H
#define UWINTERPRETERAHOI_H

#include <memory>
#include <string>
#include <vector>
#include <array>

#include "ahoitypes.h"

/**
 * Class used for building syntactically compliant commands to send to
 * the ahoi! modems and interpreting the responses from ahoi! modems.
 * For doubts and requests please refer to:
 * https://www.tuhh.de/smartport/research/acoustic-modem.html
 */
class UwInterpreterAhoi
{

public:
	/**
	 * Constructor of the interpreter class.
	 * @param id integer identifier of the modem
	 */
	UwInterpreterAhoi(int id);

	/**
	 * Destructor of the interpreter class
	 */
	~UwInterpreterAhoi();

	/**
	 * Serialize the provided packet.
	 * @param packet the ahoi::packet_t pck to be serialized
	 * @return a vector of uint8_t of serialized data
	 */
	std::string serializePacket(ahoi::packet_t *packet);

	/**
	 * Method that builds the command to send a given message
	 * @param msg string message to be sent through the modem
	 * @return formatted string for sending msg
	 */
	std::string buildSend(ahoi::packet_t pck);

	/**
	 * Method that builds the command to ask the modem its own ID number
	 * @param id integer identifier to set: if 0, ask for id
	 * @return string containing the symbols of the command
	 */
	std::string buildID(int id);

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildBatVol();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildReset();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildRange();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildAgc();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildRangeDelay();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildDistance();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildRxGain();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildTxGain();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildPacketStat();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildPacketStatReset();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildSyncStat();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildSyncStatReset();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildSfdStat();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildSfdStatReset();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildAllStat();

	/**
	 * Method that builds the command
	 * @return string containing the symbols of the command
	 */
	std::string buildAllStatReset();

	/**
	 * Method that looks for ahoi! modem responses
	 * @param beg beginning of the region to search
	 * @param end end of the region to search
	 * @param rsp beginning of the modem response found by the method
	 * @return boolean true if the method found a command
	 */
	std::string findResponse(std::vector<char>::iterator beg,
			std::vector<char>::iterator end,
			std::vector<char>::iterator &rsp_beg,
			std::vector<char>::iterator &rsp_end);

	/**
	 * Method that erases an escape (DLE) char if part of 2-cahrs escape
	 * sequence.
	 * @param iterator pointing to the beginning of the response region
	 * @param iterator pointing to the end of the response region
	 */
	void fixEscapes(std::vector<char> &buffer,
			std::vector<char>::iterator &c_beg,
			std::vector<char>::iterator &c_end);

	/**
	 * Method that parses a region of memory where a response
	 * was previously found. This methods extracts the fields of the
	 * found response.
	 * @param iterator pointing to the beginning of the response region
	 * @param iterator pointing to the end of the response region
	 * @return packet filled with the correct fields
	 */
	std::shared_ptr<ahoi::packet_t> parseResponse(
			std::vector<char>::iterator c_beg,
			std::vector<char>::iterator c_end);

private:
	uint8_t id; /**< Identifier of the modem: to fill the src addres field */
	uint8_t sn; /**< Sequence number for commands tranmission: at end restart */

	static uint header_size; /**< Standard ahoi! packet has 6 bytes of header */
	static uint footer_size; /**< Standard ahoi! packet has 6 bytes of header */

	static const uint8_t dle; /*< Escaping sequence */
	static const uint8_t stx; /**< Starting sequence */
	static const uint8_t etx; /**< Ending sequence */

	std::array<uint8_t, 2> beg_del; /**< Beginning delimiter */
	std::array<uint8_t, 2> end_del; /**< Ending delimiter */
};

#endif
