//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
 * @file uwlumaxuvmodem.h
 * @author Pietro Trabuio
 * @version 1.0.0
 * @brief Driver for the LumaX-UV, UW modem
 */

#ifndef UWLUMAXUVMODEMEM_H
#define UWLUMAXUVMODEMEM_H

#include "uwconnector.h"
#include "uwmodem.h"
#include "uwsocket.h"

#include <atomic>
#include <condition_variable>
#include <thread>

/**
class UwLumaXUVModem
 * This class implements the interface to the LumaXUV modem.
 * It is derived from UwModem and implements the methods to send and receive
 * data from the modem with UDP sockets. It also implements a method to
 * configure the modem parameters via HTTP requests with curl.
*/
class UwLumaXUVModem : public UwModem
{

public:
	enum class ModemState { AVAILABLE = 0, TRANSMITTING, RECEIVING };

	/**
	 * Constructor of the UwLumaXUVModem class
	 */
	UwLumaXUVModem();

	/**
	 * Destructor of the UwLumaXUVModem class
	 */
	virtual ~UwLumaXUVModem();

	/**
	 * Method that handles the reception of packets arriving from upper layers
	 * of the network simulator.
	 * @param p pointer to the packet that has been received from the simulator
	 *        upper layers
	 */
	virtual void recv(Packet *p);

	/**
	 * Tcl command interpreter: Method that maps Tcl commands into C++ methods.
	 *
	 * @param argc number of arguments in <i> argv </i>
	 * @param argv array of strings which are the command parameters
	 * 		  (Note that <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 *		   successfully or not
	 */
	virtual int command(int argc, const char *const *argv);

	/**
	 * Method that returns the modulation type used for the packet being
	 * transmitted. Inherited from MPhy, in NS-MIRACLE, could be left empty if
	 * no way exists to retrieve this information
	 * @param p Packet pointer to the given packet being transmitted
	 * @return modulation type represented by an integer
	 */
	virtual int getModulationType(Packet *p);

	/**
	 * Method that returns the duration of a given transmitted packet.
	 * It uses a linear interpolation given the packet size.
	 * Inherited from MPhy, in NS-MIRACLE, could be empty if there is no way
	 * to retrieve this information.
	 * @param p Packet pointer to the given packet being transmitted
	 * @return duration in seconds
	 */
	virtual double getTxDuration(Packet *p);

	/**
	 * Cross-Layer messages synchronous interpreter.
	 *
	 * @param m Instance of ClMessage that represents the
	 * message received
	 * @return <i>0</i> if successful.
	 */
	virtual int recvSyncClMsg(ClMessage *m);

protected:
	/**
	 * Method that triggers the transmission of a packet through a specified
	 * modem.
	 * @param p Packet pointer to the packet to be sent
	 */
	virtual void startTx(Packet *p);

	/**
	 * Method that starts a packet reception. This method is also in charge of
	 * sending a CrLayerMsg, Phy2MacStartRx(p), to notify the upper layers of
	 * the simulator about the start of the reception
	 * @param p Packet pointer to the packet to be received
	 */
	virtual void startRx(Packet *p);

	/**
	 * Method that ends a packet reception. This method is also in charge of
	 * sending the received NS-MIRACLE packet to the upper layers
	 * @param p Packet pointer to the packet being sent
	 */
	virtual void endRx(Packet *p);

	ModemState status;

private:
	/**
	 * Method that starts the driver operations. It performs all the needed
	 * operations to correctly fire up the device's driver.
	 */
	void start();

	/**
	 * Method that stops the driver operations. It performs all the needed
	 * operations to correctly stop the device's driver before closing
	 * operations.
	 */
	void stop();

	/**
	 * Method that dispatches a thread dedicated to receiving data from the data
	 * connector
	 */
	void receivingData();

	/**
	 * Method that creates a packet from the received stream of bytes
	 * @param p allocated empty packet to fill in with the received bytes
	 */
	void createRxPacket(Packet *p);

	/**
	 * Method that dispatches a thread dedicated to transmitting data through
	 * the data connector
	 */
	void transmittingData();

	/**
	 * Configure modem parameters
	 * @param endpoint endpoint of the param
	 * @param param_name parameter to configure
	 * @param param_value value to assign
	 * @return true, parameter configured correctly, false otherwise
	 */
	bool configure(std::string endpoint, std::string param_name,
			std::string param_value);

	/**
	 * Get param_name value from the APIs
	 * @param endpoint endpoint of the param
	 * @param param_name parameter to get value of
	 * @return value of param_name if found, empty string on error
	 */
	std::string getParameter(std::string endpoint, std::string param_name);

	/** Mutex associated with the state machine of the modem */
	std::mutex status_m;
	/** Condition variable that is linked with the status variable */
	std::condition_variable status_cv;
	/** Mutex associated with the transmission queue */
	std::mutex tx_queue_m;
	/** Condition variable that is linked with the transmitting queue */
	std::condition_variable tx_queue_cv;
	/** Atomic boolean variable that controls the receiving looping thread */
	std::atomic<bool> receiving;
	/** Atomic boolean variable that controls the transmitting looping thread */
	std::atomic<bool> transmitting;

	/** String that is updated with each new received message */
	std::string rx_payload;
	/** Size of each new received message, coming from signaling */
	int rx_size;

	std::thread
			sig_thread; /**< Thread managing the signaling reception process */
	std::thread rx_thread; /**< Thread managing the data reception process */
	std::thread tx_thread; /**< Thread managing the data transmission process */

	static const int SIGNALING_ADDRESS; /**< Port of the signaling channel */

	/**
	 * socket used to send data
	 */
	std::unique_ptr<UwSocket> send_conn;

	/**
	 * socket used to receive data
	 */
	std::unique_ptr<UwSocket> recv_conn;

	/** Bytes buffer for the signaling channel (unparsed data) */
	std::vector<char> signal_buffer;

	/** Maximum time to wait for modem to become ModemState::AVAILABLE */
	const static std::chrono::milliseconds MODEM_TIMEOUT;

	/** String separator used in reception signaling */
	const std::string sep = {"::"};
	/** String end delimiter used in reception signaling */
	const std::string end_delim = {";"};

	/**
	 * Signaling tag to recognize signaling from the modem. It is not necessary
	 * but added in case of future additional features that want to use the
	 * signaling channel
	 */
	std::string signal_tag;

	/**
	 * Modem address used only for configuration
	 */
	std::string modem_address;

	/**
	 * Local interface address, used to transmit or receive data
	 */
	std::string data_address;

	int premodulation; /**< True if premodulation is on, false otherwise */
};

/**
 * Class to create the Otcl shadow object for an object of the class
 * UwLumaXUVModem.
 */
static class UwLumaXUVModem_TclClass : public TclClass
{

public:
	UwLumaXUVModem_TclClass()
		: TclClass("Module/UW/UwModem/LumaXUV")
	{
	}

	TclObject *
	create(int args, const char *const *argv)
	{
		return (new UwLumaXUVModem());
	}

} class_lumaxuvmodem;

#endif
