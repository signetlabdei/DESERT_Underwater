//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
 * @file minterpreterAT.cc
 * @author Riccardo Masiero, Oleksandr Novychenko (EvoLogics GmbH)
 * \version 1.0.0
 * \brief Implementation of the MinterpreterAT class.
 */

#include "minterpreterAT.h"
#include <uwmdriver.h>

#include <cctype>


static void hexdump(std::string name, std::string str)
{
    int len = str.size();
    const char *data = str.c_str();

    std::cout << name << "[" << len << "]: " << std::hex;
    for (int i = 0; i < len; i++)
    {
        std::cout.fill('0');
        std::cout.width(2);
        std::cout << std::right << (int)data[i];

        if (std::isalnum(data[i]) || std::ispunct(data[i]))
            std::cout << "(" << data[i] << ")";
        std::cout << " ";
    }

    std::cout.width(0);
    std::cout << std::dec << std::endl;
}

static std::string hexdumplog(std::string str)
{
    int len = str.size();
    const char *data = str.c_str();
    
    std::string str_out = "";
   
    for (int i = 0; i < len; i++)
    {
       if (std::isalnum(data[i]) || std::ispunct(data[i]))
            str_out += data[i];
	else {
	    std::string str;
	    std::stringstream sstr("");
	    sstr << "[" << std::hex << (unsigned int)(unsigned char) data[i] << std::dec << "]";
	    sstr >> str;	
	    str_out += str;
	}
    }

    return str_out;
}


MinterpreterAT::MinterpreterAT(UWMdriver* pmDriver_):UWMinterpreter(pmDriver_){
   // Member initialization
   rx_integrity = 0;
}
		
MinterpreterAT::~MinterpreterAT(){
}
	


std::string MinterpreterAT::build_setAL(int _al){
    string at_string;
	stringstream astr("");
	astr << "AT!AL" << _al;
	astr >> at_string;	
	return at_string; 
}
	
std::string MinterpreterAT::build_sendim(int _length, int _dest, std::string _flag, std::string _payload)
{
    string at_string;
	stringstream astr("");
    astr << "AT*SENDIM," << _length << "," << _dest << "," << _flag << ",";
	astr >> at_string;	
	at_string =  at_string + _payload;
	
	if (debug_ >= 1)
    {
        cout << "MS2C_EVOLOGICS_AT_MESSAGE_" << hexdumplog(at_string) << endl;
    }
	// Return the created at_string
	return at_string;
}

std::string MinterpreterAT::build_atko(int value)
{
    string at_string;
    stringstream astr("");
    astr << "AT!KO" << value;
    astr >> at_string;
    if (debug_ >= 1)
    {
        cout << "MS2C_EVOLOGICS_AT_MESSAGE_" << hexdumplog(at_string) << endl;
    }    
    return at_string;
}


std::string MinterpreterAT::build_atzn(int _drop_type) 
{
    string at_string;
    stringstream astr("");
    
    astr << "ATZ" << _drop_type;
    astr >> at_string;
    
    return at_string;
}

std::string MinterpreterAT::build_atsend(int _length, int _dest, std::string _payload)
{
    string at_string;
    stringstream astr("");
    astr << "AT*SEND," << _length << "," << _dest << ",";
    astr >> at_string;
    at_string = at_string + _payload;
    if (debug_ >= 1)
    {
        cout << "MS2C_EVOLOGICS_AT_MESSAGE_" << hexdumplog(at_string) << endl;
    }
    return at_string;
}

std::string MinterpreterAT::build_ath(int _type_connection_close)
{
    string at_string;
    stringstream astr("");
    astr << "ATH" << _type_connection_close;
    astr >> at_string;
    if (debug_ >= 1)
    {
        cout << "MS2C_EVOLOGICS_AT_MESSAGE_" << at_string << endl;
    }
    return at_string;
}

std::string MinterpreterAT::build_atsendpbm(int _length, int _dest, std::string _payload)
{
    string at_string;
    stringstream astr("");
    astr << "AT*SENDPBM," << _length << "," << _dest << ",";
    astr >> at_string;
    at_string = at_string + _payload;
    if (debug_ >= 1)
    {
        cout << "MS2C_EVOLOGICS_AT_MESSAGE_" << hexdumplog(at_string) << endl;
    }
    return at_string;   
}

void MinterpreterAT::parse_recvim(std::string at_string) {
    std::string _prefix, _length, _src, _dest, _flag, _bitrate, _rssi, _integrity, _ptime, _velox, _payload;
    
    if (debug_ >= 1) cout << "MS2C_EVOLOGICS_AT_MESSAGE_TO_PARSE=" << hexdumplog(at_string) << endl;

    size_t offset = 0;

    int commas_before_payload = 9;
    for (int i = 0; i < commas_before_payload; i++) {
        int pos = at_string.find(",", offset);
        offset = pos + 1;
    }
    
    _payload = at_string.substr(offset);
    istringstream iastr(at_string);
    getline(iastr, _prefix, ',');
    getline(iastr, _length, ',');
    getline(iastr, _src, ',');
    getline(iastr, _dest, ',');
    getline(iastr, _flag, ',');
    getline(iastr, _bitrate, ',');
    getline(iastr, _rssi, ',');
    getline(iastr, _integrity, ',');
    getline(iastr, _ptime, ',');
    getline(iastr, _velox, ',');
    pmDriver -> updateRx(atoi(_src.c_str()), atoi(_dest.c_str()), _payload);
    rx_integrity = atof(_integrity.c_str());
}

void MinterpreterAT::parse_recv(std::string at_string) {
    std::string _prefix, _length, _src, _dest, _bitrate, _rssi, _integrity, _ptime, _velox, _payload;
    if (debug_ >= 1) cout << "MS2C_EVOLOGICS_AT_MESSAGE_TO_PARSE=" << hexdumplog(at_string) << endl;

    size_t offset = 0;

    int commas_before_payload = 9;
    for (int i = 0; i < commas_before_payload; i++) {
        int pos = at_string.find(",", offset);
        offset = pos + 1;
    }
    
    _payload = at_string.substr(offset);
    istringstream iastr(at_string);
    getline(iastr, _prefix, ',');
    getline(iastr, _length, ',');
    getline(iastr, _src, ',');
    getline(iastr, _dest, ',');
    getline(iastr, _bitrate, ',');
    getline(iastr, _rssi, ',');
    getline(iastr, _integrity, ',');
    getline(iastr, _ptime, ',');
    getline(iastr, _velox, ',');
    pmDriver -> updateRx(atoi(_src.c_str()), atoi(_dest.c_str()), _payload);
    rx_integrity = atof(_integrity.c_str());
}

void MinterpreterAT::parse_recvpbm(std::string at_string) {
        std::string _prefix, _length, _src, _dest, _bitrate, _rssi, _integrity, _ptime, _velox, _payload;
    if (debug_ >= 1) cout << "MS2C_EVOLOGICS_AT_MESSAGE_TO_PARSE=" << hexdumplog(at_string) << endl;

    size_t offset = 0;

    int commas_before_payload = 9;
    for (int i = 0; i < commas_before_payload; i++) {
        int pos = at_string.find(",", offset);
        offset = pos + 1;
    }
    
    _payload = at_string.substr(offset);
    istringstream iastr(at_string);
    getline(iastr, _prefix, ',');
    getline(iastr, _length, ',');
    getline(iastr, _src, ',');
    getline(iastr, _dest, ',');
    getline(iastr, _bitrate, ',');
    getline(iastr, _rssi, ',');
    getline(iastr, _integrity, ',');
    getline(iastr, _ptime, ',');
    getline(iastr, _velox, ',');
    cout << "PAYLOAD " << hexdumplog(_payload) << endl;
    pmDriver -> updateRx(atoi(_src.c_str()), atoi(_dest.c_str()), _payload);
    rx_integrity = atof(_integrity.c_str());
}
