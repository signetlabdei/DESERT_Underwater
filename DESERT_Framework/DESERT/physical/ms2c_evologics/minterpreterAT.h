//
// Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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
 * @file minterpreterAT.h
 * @author Riccardo Masiero
 * \version 2.0.0
 * \brief Header of the class that is in charge of building/parsing the necessary messages to make the UWMdriver able to communicate with the modem according to the AT standard.
 */


#ifndef MINTERPRETERAT_H
#define MINTERPRETERAT_H

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

using namespace std;

/* @author: Riccardo Masiero
 * @date:   06/02/2012 
 * @brief:  The header of the MinterpreterAT class, to build/parse the necessary messages to
 *          make the UWMdriver able to communicate with the modem according to AT commands. 
 
 */

/** Class used to build or parse AT messages (this class derives UWMinterpreter); currently, this class implements methods 
 * to build/parse:
 *  - simple configuration messages (e.g., setting of the modem ID)
 *  - messages involved in the transmission of an instant message (see AT*SENSIM command)
 * NOTE: for a detailed description of the messages build and parse in this class, please refer to the S2C Underwater Acoustic
 * Modem Guide. Contact EvoLogics GmbH, Ackerstr. 76 d-13355 Berlin for more infos (email: support@evologics.de).
 */
class MinterpreterAT : public UWMinterpreter {
    // Additional info (further than payload, SRC, DST -> see UWMdriver) that may be stored for each last received packet
    double rx_integrity; /**< Integrity of the last received packet. */

public:

    /** 
     * Class constructor.
     * 
     * @param pmDriver_  pointer to the UWMdriver object to link with this UWMinterpreter object.
     */
    MinterpreterAT(UWMdriver*);

    /**
     * Class destructor.
     */
    ~MinterpreterAT();

    // METHODS to BUILD MESSAGES

    /** 
     * Method to build an AT message to set the Local Address of a node. 
     * 
     * @param _al the local address that we want to assign to the modem.
     * @return at_string the requested AT message. 
     */
    std::string build_setAL(int _al);

    /** 
     * Method to build an AT message to check the modem delivery status
     * (i.e., the Instant message delivery status AT message).
     */
    std::string build_di() {
        return "AT?DI";
    }

    /** 
     * Method to build an AT message to send instant messages. 
     * 
     * @param _length the length in bytes of the message to send (max is 64 bytes).
     * @param _dest the ID of the modem that must receive the packet.
     * @param _flag to enable the acknowledge feature (allowed values: 'ack', 'noack') 
     * @param _data message to send in form of a binary data array (string of _length chars)
     * @return at_string the requested AT message.
     */
    std::string build_sendim(int _length, int _dest, std::string _flag, std::string _payload);
    /** 
     * Method to build an AT message to drop all messages in queue (IM + burst data) 
     * 
     * @param _drop_type type of drop requested
     * @return at_string the requested AT message.
     */    
    std::string build_atzn(int _drop_type);

    // METHODS to PARSE MESSAGES
    // NOTE: These methods must know and use the reception variable of the linked UWdriver object and the corresponding methods to update them

    /**
     * Method to parse an AT message (reception of an instant message). 
     * NOTE: this method calls UWMdriver::updateRx(int,int,std::string).
     * @param at_string the received string.
     */
    void parse_recvim(std::string at_string);
    /** 
     *  Method to get the Integrity value of the last received packet. NOTE: This method is used by McodecS2C_EvoLogics.
     * 
     *  @return MinterpreterAT::rx_integrity, the Integrity value of the last received packet.
     */
    double getIntegrity() {
        return rx_integrity;
    }
};
#endif	/* MINTERPRETERAT_H */
