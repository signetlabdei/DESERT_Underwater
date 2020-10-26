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
 * @file    uwinterpreters2c.h
 * @author  Roberto Francescon
 * @version 0.0.1
 * @brief   Header of the interepreter class: this class is used ot translate
 *          form DESERT software to/from EvoLogics S2C syntax
 */

#ifndef UWINTERPRETERS2C_H
#define UWINTERPRETERS2C_H

#include <cstdarg>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

class UwInterpreterS2C
{

public:
	/**
	 * Enum listing the types of commands that could be received or sent by
	 * a S2C device; See the EvoLogics S2C manuals or reach for www.evologics.de
	 */
	enum class Response {
		RECVIM = 0,
		RECV,
		OK,
		EMPTY,
		BUSY,
		DELIVERING,
		DELIVERED,
		DROPCNT,
		PHYOFF,
		NOT_ACCEPTED,
		WRONG_ADDR,
		CONN_CLOSED,
		BUFF_NOT_EMPTY,
		OUT_OF_RANGE,
		PROTOCOL_ID,
		INTERNAL,
		BUFFER_FULL,
		FAIL,
		CURR_SETTINGS,
		MODEM_STATUS,
		INIT_NOISE,
		INIT_DEAF,
		INIT_LISTEN,
		RECVSTART,
		RECVEND,
		RECVFAIL,
		SENDSTART,
		SENDEND,
		BITRATE,
		UNKNOWN,
		NO_COMMAND
	};
	/**
	 * Class constructor
	 */
	UwInterpreterS2C();

	/**
	 * Class destructor
	 */
	virtual ~UwInterpreterS2C();

	/**
	 * Method that builds the command to send data through SEND command
	 * SEND is used to send burst data: payload is saved in S2C internal memory
	 * until the device is ready to transmit, at the negotiated bitrate.
	 * @param msg string containing the payload to be sent
	 * @param dest integer number representing the destination local address
	 * @return string of the requested command
	 */
	std::string buildSend(std::string msg, int dest);

	/**
	 * Method that builds the command to send data through SENDIM command
	 * SENDIM is used to send Instant Messages: these messages are immediately
	 * transmitted at the minimum bitrate availbale  in the device.
	 * @param msg string containing the payload to be sent
	 * @param dest integer number representing the destination local address
	 * @param ack boolean set to true if ACK is needed, false otherwise
	 * @return string of the requested command
	 */
	std::string buildSendIM(std::string msg, int dest, bool ack);

	/**
	 * Method that builds a reboot sequence: depending on the provided index
	 * different levels of reset can be requested.
	 * @param level integer level of reset requested
	 * @return string of the requested command
	 */
	std::string buildATZ(int level);

	/**
	 * Method that builds the command to get the source level of the device
	 * @return string of the requested command
	 */
	std::string buildGetATL();

	/**
	 * Method that builds the command to set the source level of the device
	 * @param level integer level of the Source Level of the device
	 * @return string of the requested command
	 */
	std::string buildSetATL(int level);

	/**
	 * Method that builds the command to set the local address of the device
	 * @return string of the requested command
	 */
	std::string buildGetATAL();

	/**
	 * Method that builds the command to set the local address of the device
	 * @param level integer value of the Local Address of the device
	 * @return string of the requested command
	 */
	std::string buildSetATAL(int addr);

	/**
	 * Method that builds the command to check the IM delivery status
	 * @return string of the requested command
	 */
	std::string buildATDI();

	/**
	 * Method that builds the command to ask for modem status: it enlists
	 * local address, acoustic link status and pool status and whether
	 * promiscous mode is on.
	 * @return string of the requested command
	 */
	std::string buildATS();

	/**
	 * Method that builds the command to ask for current settings: it enlists
	 * source level, gain, retry timeout, carrier ID and other
	 * See the EvoLogics S2C manuals or reach for www.evologics.de
	 * @return string of the requested command
	 */
	std::string buildATV();

	/**
	 * Method to look for S2C response inside a provided chunk of unparsed data
	 * This method only finds the beginning of the returned response,not the end
	 * @param beg iterator to beginning of search section
	 * @param end iterator to end of search section
	 * @param rsp output iterator first response found
	 * @return type of the first valid response found,
	 *         of type UwInterpreterS2C::Response
	 */
	UwInterpreterS2C::Response findResponse(std::vector<char>::iterator beg,
			std::vector<char>::iterator end, std::vector<char>::iterator &rsp);

	/**
	 * Method that tries to parse a found response: if the response section
	 * of the buffer, which needs to be passed, is found to be incomplete,
	 * the method return false.
	 * If parsing is successful, proper action is taken.
	 * The method also locates the end of the response looking for `term`.
	 * @param[in]  rsp response found by call to UwInterpreterS2C::findResponse
	 * @param[in]  end beginning of the buffer section to parse
	 * @param[in]  rsp_beg of the responses as found by findResponse()
	 * @param[out] rsp_end of the response, if found by parsing
	 * @param[out] rx_payload containing the payload stored in the response, if
	 * any.
	 * @return false if attempt to parse command fails (e.g., for missing bytes)
	 */
	bool parseResponse(UwInterpreterS2C::Response rsp,
			std::vector<char>::iterator end,
			std::vector<char>::iterator rsp_beg,
			std::vector<char>::iterator &rsp_end, std::string &rx_payload);

private:
	std::string sep; /**< Separator for paramters fo the commands: a comma */
	std::string r_term; /**<Terminating sequence for commands read from device*/
	std::string w_term; /**<Terminating sequence for commands wrtten to device*/

	/**
	 * Vector holding all possible commands for the S2C syntax and
	 * corresponding identifying token.
	 * It is important that RECV and RECVIM remain at the beginning, as the
	 * other commands may be inside the binary payload of these two.
	 */
	static std::vector<std::pair<std::string, UwInterpreterS2C::Response> >
			syntax_pool;
};

#endif
