//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
 * @file minterpreterTEL.h
 * @author Roberto francescon
 * \version 0.0.1
 * \brief Class that is in charge of building and parsing the
 * required messages to make the UWMdriver able to communicate with the modem
 * according to TELEGRAMS accepted by the low level firmware.
 */

#ifndef MINTERPRETERTEL_H
#define MINTERPRETERTEL_H

#include <uwminterpreter.h>

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>
#include <climits>



/** Class used to build and parse TELEGRAMS (this class derives UWMinterpreter);
 * currently, this class implements methods to build/parse:
 *  - simple configuration messages (e.g., setting of DSP and FPGA of modems)
 *  - messages involved in the transmission of CONTROL and DATA messages.
 * Contact EvoLogics GmbH, Ackerstr. 76 d-13355 Berlin for more
 * info (email: support@evologics.de).
 */
class MinterpreterTEL : public UWMinterpreter
{
	// Additional info (further than payload, SRC, DST -> see UWMdriver) that
	// may be stored for each last received packet
	double rx_integrity; /**< Integrity of the last received packet. */

public:
	/**
	 * Class constructor.
	 *
	 * @param pmDriver_  pointer to the UWMdriver object to link with this
	 * UWMinterpreter object.
	 */
	MinterpreterTEL(UWMdriver *);

	/**
	 * Class destructor.
	 */
	~MinterpreterTEL();

	// METHODS TO USE THE GPIO PINS
	/**
	 * Method for building the string that allow turning on the DSP of the
	 * modem,
	 * which basically turns on the physical layer: it is made up of four steps,
	 * numbered from 1 to 4,and there is a short interval of 1 second in the
	 * middle of the procedure.
	 * @param _step At which step of the turn on process are you (1-4)?
	 * @return turnon_string
	 */
	std::string build_poweron_DSP(int _step);
	/**
	 * Method for building the string for turning off the DSP of the modem
	 * which basically turns off the physical layer. Steps go from 1 to 3.
	 * @param _step At which step of the turn off process are you (1-3)?
	 * @return turnoff_string
	 */
	std::string build_poweroff_DSP(int _step);
	/**
	 * Method which builds the string to ask the DSP if it is busy doing
	 * modulation/demodulation. The pin under consideration is a read-only pin.
	 * @return Formatted string
	 */
	std::string build_busy_FPGA();

	// METHODS WHICH BUILDS TELEGRAMS TO BE SENT TO PHY
	/**
	 * Method which builds the TELEGRAM that makes the physical layer to exit
	 * the
	 * listen mode. The modem stops listening for incoming signals asnd returns
	 * a
	 * timestamp.
	 * @return Formatted string
	 */
	std::string build_stop_listen();
	/**
	 * Method for building the TELEGRAM that will allow configuring the DSP.
	 * It sets basic settings like gain and source level.
	 * @param _gain
	 * @param _chipset
	 * @param _sl
	 * @param _th
	 * @param mps_th
	 * @return config_string
	 */
	std::string build_config_DSP(
			int _gain, int _chipset, int _sl, int _th, int _mps_th);
	/**
	 * Method for building the TELEGRAM that will allow receiving BITS,
	 * wheter CONTROL or DATA messages. It basically switches the physical layer
	 * to detect signals and demodulate them. Any new TELEGRAM automatically
	 * makes the physical layer to exit this mode.
	 * @param _stop_f Flag to tell the firmware to stop listening after _stop
	 * @param _delay  Time to wait before shutting stopping listening
	 * @return recv_string
	 */
	std::string build_recv_data(int msg_bytes, int _stop_f, double _delay);
	/**
	 * Method for building the TELEGRAM that will transmit CONTROL messages,
	 * which are short messanges of maximum length with fixed bitrate.
	 * The bitrate is ? and maximum allowed length is ?.
	 * @param _crtl
	 * @param _delay_f
	 * @param _delay
	 * @return ctrl_telegram
	 */
	std::string build_send_ctrl(std::string _ctrl, int _delay_f, double _delay);
	/**
	 * Method for building the TELEGRAM that will send DATA messages.
	 * The maximum allowed length of DATA messages is 7881
	 * @param _data
	 * @param _delay_f Flag to tell the firmware to real _delay
	 * @param _delay   Can make the operation to wait some maount of time
	 * @return data_telegram
	 */
	std::string build_send_data(std::string _data, int _delay_f, double _delay);
	/**
	 * Method for building the TELEGRAM that will set the bitrate used for
	 * transmission.
	 * @param _bitrate
	 * @return bitrate_telegram
	 */
	std::string build_bitrate(int _bitrate);
	/**
	 * Method for building the COMMAND that will clear the tx_on pin
	 * @return clear_command
	 */
	std::string build_clear_tx();
	/**
	 * Method for parsing what is received upon receiving a TELEGRAM
	 * @param _received
	 *
	 */
	void parse_TELEGRAM(std::string telegram);
};
#endif /*MINTERPRETERTEL_H*/
