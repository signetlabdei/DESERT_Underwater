//
// Copyright (c) 2018 Regents of the SIGNET lab, University of Padova.
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
 * @file    uwconnector.h
 * @author  Roberto Francescon, Emanuele Coccolo
 * @version 1.0.0
 * @brief   Generic class that provides a method to interface with the devices.
 *          Will be specialized for, e.g., socket connections, serial ecc...
 */

#ifndef UWCONNECTOR_H
#define UWCONNECTOR_H

#include <array>
#include <string>
#include <vector>

/**
 * Class UwConnector allows to specify an interface between the UwDriver object
 * and the device. The connector is typically a TCP or UDP connection, but
 * may also be a serial connection. It allows to retrieve the data packets
 * received by the transducer and to send commands to it.
 * UwConnector assumes the connector works on a byte-based flow.
 */
class UwConnector
{

public:
	/**
	 * UwConnector constructor
	 * @param address string representing an address, whatever that is
	 */
	inline UwConnector(){};

	/**
	 * UwConnector destructor
	 */
	inline virtual ~UwConnector(){};

	/**
	 * Method required in client-server connection (e.g. TCP)
	 */
	virtual void setServer(){};

	/**
	 * Method that should set, for a socket connector, the TCP transport
	 * protocol. To be left untouched by other types of connectors.
	 */
	virtual void setTCP(){};

	/**
	 * Method that should set, for a socket connector, the UDP transport
	 * protocol. To be left untouched by other types of connectors.
	 */
	virtual void setUDP(){};

	/**
	 * Method that opens up a connection to a device. This method can open
	 * a conection with the modem's data/config interface
	 * or directly to some remote host at the end of the connection.
	 * @path string representing the path to connect to: its meaning depends
	 *       on the type of connection established
	 * @return boolean true if connection is correctly opened, false otherwise
	 */
	virtual bool openConnection(const std::string &path) = 0;

	/**
	 * Method that closes an active connection to a device interface
	 * @return boolean true if connection is correctly closed, false otherwise
	 */
	virtual bool closeConnection() = 0;

	/**
	 * Function that writes some command to the connected interface
	 * @param cmd string command to write to the device
	 * @return boolean true if command is correctly written to the device
	 */
	virtual int writeToDevice(const std::string& msg) = 0;

	/**
	 * Function that dumps data from the device's memory to data char array.
	 * The downloaded data is saved to a temporary buffer, to be later parsed.
	 * @param pos char pointer to the beginning of the writable area;
	 *        it is duty of the caller to take care of the writable area
	 * @param maxlen maximum integer number of char to write to wpos
	 * @return number of bytes read in a single dump
	 */
	virtual int readFromDevice(void *wpos, int maxlen) = 0;

	/**
	 * Function that retrieves the last saved errno code: to be implemented
	 * @return error code from <cerrno> system library
	 */
	virtual const int getErrno()
	{
		return local_errno;
	};

	/**
	 * Returns true if connection is up
	 * @return if connection is up
	 */
	virtual const bool isConnected() = 0;

protected:

	int local_errno; /** Local variable to stoe the errno of connectors */
};

#endif
