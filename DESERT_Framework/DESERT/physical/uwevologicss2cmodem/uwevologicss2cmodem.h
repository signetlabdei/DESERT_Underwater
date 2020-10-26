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
 * @file    uwevologicss2cmodem.h
 * @author  Roberto Francescon
 * @version 0.1.0
 * @brief   Header of the main class that implements the drivers to manage the
 *          EvoLogics S2C line of devices. See www.evologics.de
 */

#ifndef UWEVOLOGICSS2CSMODEM_H
#define UWEVOLOGICSS2CSMODEM_H

#include <uwconnector.h>
#include <uwinterpreters2c.h>
#include <uwmodem.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/**
 * Enum containing the possible statuses of the driver. In particular, a command
 * can only sent if the driver is in AVAILABLE status.
 */
enum class ModemState { AVAILABLE, BUSY, RESET, QUIT, DEAF, NOISE, LISTEN };

class UwEvoLogicsS2CModem : public UwModem
{
	/**
	 * Enum listing the availbale configuration settings.
	 * See the EvoLogics S2C manuals or reach for www.evologics.de
	 */
	enum class Config {
		ATL_GET,
		ATL_SET,
		ATAL_GET,
		ATAL_SET,
		MODEM_STATUS,
		CURR_SETTINGS,
		UNKNOWN
	};

public:
	/** Transmission mode: either IM or BURST
	 * See the EvoLogics S2C manuals or reach for www.evologics.de
     */
	enum class TransmissionMode { BURST = 0, IM = 1 };

	/**
	 * Transmission state: controls the flow of execution for sending commands
	 * to the S2C device
	 */
	enum class TransmissionState { TX_IDLE = 0, TX_PENDING };

	/**
	 * Constructor of the UwEvoLogicsS2CModem class
	 * @param address string containing the address to connect to
	 * @param buflen lenght in char of the data buffer
	 * @param read len length in char of a signle read from the connector
	 */
	UwEvoLogicsS2CModem();

	/**
	 * Destructor of the UwEvoLogicsS2CModem class
	 */
	virtual ~UwEvoLogicsS2CModem();

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
	 * Cross-Layer messages synchronous interpreter.
	 * @param ClMessage* an instance of ClMessage that represent the
	 * message received
	 * @return <i>0</i> if successful.
	 */
	virtual int recvSyncClMsg(ClMessage *m);

	/**
	 * Method that sends a written configuration the the EvoLgoics modem.
	 * The configuration string could be a request of configurations (the AT
	 * commands conaining '?') or a setting command (the AT commands
	 * containing '!').
	 * @param command the command string complying with the device format
	 *        requirements.
	 * @return boolean true if the command was correctly sent to the device
	 */
	virtual bool configure(Config cmd);

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
	 * Mehod that detach a thread devoted to receiving data from the connector.
	 */
	virtual void receivingData();

	/**
	 * Method that updates the status of the modem State Machine: state change
	 * is triggered by reception of commands on the connector interface, or by
	 * commands to be sent to the device, e.g., SEND or SENDIM.
	 * @param cmd command received on the connector interface
	 */
	void updateStatus(UwInterpreterS2C::Response cmd);

	/**
	 * Method that allows to set the error flag on the packet to notify
	 * upper layers about un-tranmsitted packets due to, e.g., modem
	 * unavailability.
	 * @param p Packet pointer ot the packet being transmitted
	 */
	void setFailedTx(Packet *p);

	/**
	 * Method that fills up a packet with the needed header and payload
	 * and makes it ready to be sent to the layer above
	 * @param p instance of an empty packet to be filled
	 */
	void createRxPacket(Packet *p);

	/** Pointer to Connector object that interfaces with the device */
	std::unique_ptr<UwConnector> p_connector;
	/** Pointer to Interpreter object to parse device syntax */
	std::unique_ptr<UwInterpreterS2C> p_interpreter;

	ModemState status; /**< Variable holding the current status of the modem */

	TransmissionState tx_status; /**< Variable holding the current transmission
									status of the modem */

	/** Mutex associated with the state machine of the modem */
	std::mutex status_m;
	/** Mutex associated with the transmission state machine of the modem */
	std::mutex tx_status_m;
	/** Mutex associated with the transmission queue */
	std::mutex tx_queue_m;
	/** Condition variable to wait for ModemState::AVAILABLE */
	std::condition_variable status_cv;
	/** Condition variable to wait for TransmissionState::TX_IDLE */
	std::condition_variable tx_status_cv;
	/** Condition variable that is linked with the transmitting queue */
	std::condition_variable tx_queue_cv;
	/** Atomic boolean variable that controls the receiving looping thread */
	std::atomic<bool> receiving;
	/** Atomic boolean variable that controls the transmitting looping thread */
	std::atomic<bool> transmitting;
	/** Atomic boolean variable controlling if the modem had responded to ATDI*/
	std::atomic<bool> im_status_updated;
	/**Object with the rx thread */
	std::thread rx_thread;
	/**Object with the tx thread */
	std::thread tx_thread;
	/** String that is updated witn each new received messsage */
	std::string rx_payload;
	/** Maximum time to wait for modem to become ModemState::AVAILABLE */
	const static std::chrono::milliseconds MODEM_TIMEOUT;
	/** Time interval to wait for the modem notifying that there
	no more IM in its queue and a new IM can be sent */
	const static std::chrono::milliseconds WAIT_DELIVERY_IM;
	/** Time interval tu wait for a burst message tobe confirmed through a
	DELIVERED response. Given in SECONDS */
	const static std::chrono::seconds WAIT_DELIVERY_BURST;

	TransmissionMode tx_mode; /**< Either burst or im.*/
	bool ack_mode; /**< Set to true to enable IM ack*/
	double virtual_time_ref; /**< virtual time reference */
	int curr_source_level; /**< Current source level already set in device */
	int n_rx_failed; /**< Number of failed receptions up to now */
	int pend_source_level; /**< Pending source level, requested but not set */
	bool source_level_change; /**< Flag that tells a new SL value to be
								 applied*/

	/** Minimum time to wait before to schedule a new event in seconds*/
	static const double EPSILON_S;
	/** Maximum number of time to query the modem transmission status before to
	 * discard the transmitted packet */
	static uint MAX_N_STATUS_QUERIES;
};

#endif
