//
// Copyright (c) 2016 Regents of the SIGNET lab, University of Padova.
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
 * @file minterpreterTEL.cpp
 * @author Roberto Francescon
 * \version 0.0.1
 * \brief Implementation of the MinterpreterTEL class.
 */

#include "minterpreterTEL.h"
#include <uwmdriver.h>

#include <cctype>
#include <iostream>
#include <string>

static void
hexdump(std::string name, std::string str)
{
	int len = str.size();
	const char *data = str.c_str();

	std::cout << name << "[" << len << "]: " << std::hex;
	for (int i = 0; i < len; i++) {
		std::cout.fill('0');
		std::cout.width(2);
		std::cout << std::right << (int) data[i];

		if (std::isalnum(data[i]) || std::ispunct(data[i]))
			std::cout << "(" << data[i] << ")";
		std::cout << " ";
	}

	std::cout.width(0);
	std::cout << std::dec << std::endl;
}

static std::string
hexdumplog(std::string str)
{
	int len = str.size();
	const char *data = str.c_str();

	std::string str_out = "";

	for (int i = 0; i < len; i++) {
		if (std::isalnum(data[i]) || std::ispunct(data[i]))
			str_out += data[i];
		else {
			std::string str;
			std::stringstream sstr("");
			sstr << "[" << std::hex << (unsigned int) (unsigned char) data[i]
				 << std::dec << "]";
			sstr >> str;
			str_out += str;
		}
	}

	return str_out;
}

static std::string
hexdumpdata(std::string str)
{
	size_t len = str.size();
	const char *data = str.c_str();

	std::string str_out = "";

	for (size_t i = 0; i < len; i++) {
		std::stringstream sstr("");
		sstr.fill('0');
		sstr.width(2);
		sstr << std::hex << (unsigned int) (unsigned char) data[i] << std::dec;
		str_out += sstr.str();
	}
	return str_out;
}

static std::string
dechexdumpdata(std::string str)
{
	std::istringstream istr(str);
	std::string str_out = "";
	std::string str_sym("");

	while (std::getline(istr, str_sym, ' ')) {
		int symbol;
		char ch;
		std::stringstream sstr("");
		sstr << "0x" << str_sym;
		sstr >> std::hex >> symbol;
		ch = symbol;
		str_out += ch;
	}

	return str_out;
}

MinterpreterTEL::MinterpreterTEL(UWMdriver *pmDriver_)
	: UWMinterpreter(pmDriver_)
{
	// Member initialization
	rx_integrity = 0;
}

MinterpreterTEL::~MinterpreterTEL()
{
}

std::string
MinterpreterTEL::build_poweron_DSP(int _step)
{
	std::string turnon_string;
	std::stringstream astr("");
	switch (_step) {
		case 1:
			astr << "gpio,clear,dsp_power";
			break;
		case 2:
			astr << "gpio,set,ena_tx";
			break;
		case 3:
			astr << "gpio,clear,fpga_reset";
			break;
		case 4:
			astr << "gpio,clear,dsp_reset";
			break;
	}
	astr >> turnon_string;
	return turnon_string;
}

std::string
MinterpreterTEL::build_poweroff_DSP(int _step)
{
	std::string turnoff_string;
	std::stringstream astr;
	switch (_step) {
		case 1:
			astr << "gpio,set,dsp_reset";
			break;
		case 2:
			astr << "gpio,set,fpga_reset";
			break;
		case 3:
			astr << "gpio,set,dsp_power";
			break;
	}
	astr >> turnoff_string;
	return turnoff_string;
}

std::string
MinterpreterTEL::build_busy_FPGA()
{
	return "gpio,get,fpga_busy";
}

std::string
MinterpreterTEL::build_stop_listen()
{
	return "stop_listen";
}

std::string
MinterpreterTEL::build_config_DSP(
		int _gain, int _chipset, int _sl, int _th, int _mps_th)
{
	std::string config_string;
	std::stringstream astr("");
	astr << "config_dsp"
		 << ","
		 << "1"
		 << "," << _gain << "," << _chipset << "," << _sl << "," << _th << ","
		 << "0"
		 << "," << _mps_th;
	astr >> config_string;

	return config_string;
}

std::string
MinterpreterTEL::build_recv_data(int msg_bytes, int _stop_f, double _delay)
{
	std::string recv_string;
	std::stringstream astr;
	if (msg_bytes != 0) {
		astr << "rcv_sync_data"
			 << "," << msg_bytes << "," << _stop_f << "," << _delay;
	} else {
		astr << "rcv_sync_data"
			 << ","
			 << "0"
			 << "," << _stop_f << "," << _delay;
	}
	astr >> recv_string;
	return recv_string;
}

std::string
MinterpreterTEL::build_send_ctrl(std::string _ctrl, int _delay_f, double _delay)
{
	std::string ctrl_telegram;
	std::stringstream astr;
	std::string hexdumped_ctrl;

	// hexdump the data
	hexdumped_ctrl = hexdumpdata(_ctrl);

	// ASCII coding: 8 bits per character
	int bitlen = _ctrl.size() * 8;

	astr << "send_sync_ctrl"
		 << "," << bitlen << "," << _delay_f << "," << _delay << ","
		 << hexdumped_ctrl;
	astr >> ctrl_telegram;
	return ctrl_telegram;
}

std::string
MinterpreterTEL::build_send_data(std::string _data, int _delay_f, double _delay)
{
	std::string data_telegram;
	std::stringstream astr;
	std::string hexdumped_data;

	// hexdump the data
	hexdumped_data = hexdumpdata(_data);

	// ASCII coding: 8 bits per character
	int bitlen = hexdumped_data.size() * 4;

	astr << "send_sync_data"
		 << "," << bitlen << "," << _delay_f << "," << _delay << ","
		 << hexdumped_data;
	astr >> data_telegram;
	return data_telegram;
}

std::string
MinterpreterTEL::build_bitrate(int _bitrate)
{
	std::string bitrate_telegram;
	std::stringstream astr;
	if ((_bitrate < 0) || (_bitrate > 63)) {
		std::cout << "ERROR: invalid bitrate code provided";
		astr << "bitrate"
			 << ","
			 << "1";
		astr >> bitrate_telegram;
	} else {
		astr << "bitrate"
			 << "," << _bitrate;
		astr >> bitrate_telegram;
	}
	return bitrate_telegram;
}

std::string
MinterpreterTEL::build_clear_tx()
{
	std::string clear_command("gpio,clear,tx_on");
	return clear_command;
}

void
MinterpreterTEL::parse_TELEGRAM(std::string telegram)
{
	// take everything up! for future needs...
	std::string _TEL;
	std::string _len, _data, _bitlen, _data_flag, _integrity, _rms, _error_code;
	std::string _rcv_time, _sync_detected_time, _nshift, _speed;
	std::string _sync_delta_phase, _symbol_phase_std, _symbol_mean_phase, _mps;

	std::istringstream iastr(telegram);
	getline(iastr, _TEL, ',');
	getline(iastr, _len, ',');
	getline(iastr, _data, ',');
	getline(iastr, _bitlen, ',');
	getline(iastr, _data_flag, ',');
	getline(iastr, _integrity, ',');
	getline(iastr, _rms, ',');
	getline(iastr, _error_code, ',');

	getline(iastr, _rcv_time, ',');
	getline(iastr, _sync_detected_time, ',');
	getline(iastr, _nshift, ',');
	getline(iastr, _speed, ',');

	getline(iastr, _sync_delta_phase, ',');
	getline(iastr, _symbol_phase_std, ',');
	getline(iastr, _symbol_mean_phase, ',');
	getline(iastr, _mps, '\n');

	// update the variable for packets: broadcast is set as dest, 0 as send
	std::string c_data;
	// erase double quotes
	std::string _data_p = _data.erase(0, 1);
	_data_p = _data_p.erase(_data_p.size() - 1, 1);

	c_data = dechexdumpdata(_data_p);

	pmDriver->updateRx(0, INT_MAX, c_data);
	rx_integrity = atof(_integrity.c_str());
}
