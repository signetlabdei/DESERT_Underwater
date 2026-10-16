//
// Copyright (c) 2025 Regents of the SIGNET lab, University of Padova.
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
 * @file    uwapplicon-interp.h
 * @author  Gianni Cario
 * @version 0.0.1
 * @brief   Header of the interepreter class: this class is used ot translate
 *          form DESERT software to/from Applicon SeaModem syntax
 */

#ifndef UWINTERPRETERAPPL_H
#define UWINTERPRETERAPPL_H

#include <string>
#include <vector>

class UwAppliconInterpr
{

public:
	/**
	 * Enum listing the types of commands that could be received or sent by
	 * a SeaModem device.
	 */
	enum class Response {
		RECV,
		MSGS,
		AOK, // <-- NUOVA RIGA
		NO_COMMAND
	};
	/**
	 * Class constructor
	 */
	UwAppliconInterpr();

	/**
	 * Class destructor
	 */
	~UwAppliconInterpr()=default;

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
	 * Method to look for S2C response inside a provided chunk of unparsed data
	 * This method only finds the beginning of the returned response,not the end
	 * @param beg iterator to beginning of search section
	 * @param end iterator to end of search section
	 * @param rsp output iterator first response found
	 * @return type of the first valid response found,
	 *         of type UwAppliconInterpr::Response
	 */
	UwAppliconInterpr::Response findResponse(std::vector<char>::iterator beg,
			std::vector<char>::iterator end, std::vector<char>::iterator &rsp);

	/**
	 * Method that tries to parse a found response: if the response section
	 * of the buffer, which needs to be passed, is found to be incomplete,
	 * the method return false.
	 * If parsing is successful, proper action is taken.
	 * The method also locates the end of the response looking for `term`.
	 * @param[in]  rsp response found by call to UwAppliconInterpr::findResponse
	 * @param[in]  end beginning of the buffer section to parse
	 * @param[in]  rsp_beg of the responses as found by findResponse()
	 * @param[out] rsp_end of the response, if found by parsing
	 * @param[out] rx_payload containing the payload stored in the response, if
	 * any.
	 * @return false if attempt to parse command fails (e.g., for missing bytes)
	 */
	bool parseResponse(UwAppliconInterpr::Response rsp,
			std::vector<char>::iterator end,
			std::vector<char>::iterator rsp_beg,
			std::vector<char>::iterator &rsp_end, std::string &rx_payload, bool &integrity);

private:
	std::string sep; /**< Separator for paramters fo the commands: a comma */
	std::string r_term; /**<Terminating sequence for commands read from device*/


	/**
	 * Vector holding all possible commands for the S2C syntax and
	 * corresponding identifying token.
	 * It is important that RECV and RECVIM remain at the beginning, as the
	 * other commands may be inside the binary payload of these two.
	 */
	static std::vector<std::pair<std::string, UwAppliconInterpr::Response> >
			syntax_pool;
};

#endif
