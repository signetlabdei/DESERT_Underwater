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
 * @file uwmodemcsa.h
 * @author Emanuele Coccolo
 * \version 0.1.0
 * \brief Class that implements a generic modem driver for end-to-end communications.
 */

#ifndef UWMODEMCSA_H
#define UWMODEMCSA_H

#include <uwconnector.h>
#include <uwmodem.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

enum class ModemState { AVAILABLE, BUSY };

class UwModemCSA : public UwModem {
public:

	/**
	 * Constructor of the UwModemCSA class
	 * @param address string containing the address to connect to
	 * @param buflen lenght in char of the data buffer
	 * @param read len length in char of a signle read from the connector
	 */
	UwModemCSA();

	/**
	 * Destructor of the UwModemCSA class
	 */
	virtual ~UwModemCSA();

	/**
	 * Method that handles the reception of packets arriving from upper layers
	 * of the network simulator.
	 * @param p pointer to the packet that has been received from the simulator
	 *        upper layers
	 */
	virtual void recv(Packet *p);

	/**
	 * Cross-Layer messages synchronous interpreter.
	 * @param ClMessage* an instance of ClMessage that represent the
	 * message received
	 * @return <i>0</i> if successful.
	 */
	virtual int recvSyncClMsg(ClMessage *m);

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

private:
	/**
	 * Method that triggers the transmission of a packet through a specified
	 * modem.
	 * @param p Packet pointer to the packet to be sent
	 */
	virtual void startTx(Packet *p);

	/**
	 * Method that starts a packet reception. This method is also in charge of
	 * sending a ClMsg, Phy2MacStartRx(p), to notify the upper layers of
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

	/**
	 * Method that starts the driver operations. It performs all the needed
	 * operations to correctly fire up the device's driver.
	 */
	virtual void start();

	/**
	 * Method that stops the driver operations. It performs all the needed
	 * operations to correctly stop the device's driver before closing the
	 * simulation.
	 */
	virtual void stop();

	/**
	 * Method that detach a thread devoted to sending packets found in tx_queue.
	 */
	virtual void transmittingData();

	/**
	 * Method that detach a thread devoted to receiving data from the connector.
	 */
	virtual void receivingData();

	/**
	 * Method that finds the position of a command in a buffer.
	 */
	virtual std::string findCommand(std::vector<char>::iterator beg_it,
									std::vector<char>::iterator end_it,
									std::vector<char>::iterator &cmd_b,
									std::vector<char>::iterator &cmd_e);

	/**
	 * Method that parses the command to obtain the recquired informations.
	 */
	virtual bool parseCommand(std::vector<char>::iterator cmd_b,
							std::vector<char>::iterator cmd_e,
							std::string &rx_payload);

	/**
	 *
	 */
	virtual std::string buildSend(const std::string &payload, int dest);

	/**
	 * Method that updates the status of the modem State Machine: state change
	 * is triggered by reception of commands on the connector interface, or by
	 * commands to be sent to the device, e.g., SEND or SENDIM.
	 * @param cmd command received on the connector interface
	 */
	void startRealRx(const std::string &cmd);

	void createRxPacket(Packet *p);

	/** Pointer to Connector object that interfaces with the device */
	std::unique_ptr<UwConnector> p_connector;

	ModemState status; /**< Variable holding the current status of the modem */

	/** Mutex associated with the state machine of the modem */
	std::mutex status_m;
	/** Mutex associated with the transmission queue */
	std::mutex tx_queue_m;
	/** Condition variable to wait for ModemState::AVAILABLE */
	std::condition_variable status_cv;
	/** Condition variable that is linked with the transmitting queue */
	std::condition_variable tx_queue_cv;
	/** Atomic boolean variable that controls the receiving looping thread */
	std::atomic<bool> receiving;
	/** Atomic boolean variable that controls the transmitting looping thread */
	std::atomic<bool> transmitting;
	/**Object with the rx thread */
	std::thread rx_thread;
	/**Object with the tx thread */
	std::thread tx_thread;
	/** String that is updated witn each new received messsage */
	std::string rx_payload;
	/**  */
	std::string del_b;
	/**  */
	std::string del_e;
	/** Maximum time to wait for modem to become ModemState::AVAILABLE */
	const static std::chrono::milliseconds MODEM_TIMEOUT;

	/** minimum time to wait before to schedule a new event in seconds*/
	static const double EPSILON_S;
	/** maximum number of time to poll the modem transmission status before to
	 * discard the transmitted packet*/
	static const size_t MAX_TX_STATUS_POLL;
};

#endif