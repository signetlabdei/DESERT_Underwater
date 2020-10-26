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
 * @file uwahoimodem.h
 * @author Emanuele Coccolo
 * @author Roberto Francescon
 * @version 1.0.0
 * @brief Driver for ahoi! acoustic UW modems: developed by TUUH, Hamburg.
 */

#ifndef UWAHOIMODEM_H
#define UWAHOIMODEM_H

#include <uwconnector.h>
#include <uwinterpreterahoi.h>
#include <uwmodem.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/**
 * The ahoi! modems are an underwater acoustic modems developed by TUUH
 * University, Hamburg by the SmartPORT Group. The reference website can be
 * found here: https://www.tuhh.de/smartport/research/acoustic-modem.html
 */
class UwAhoiModem : public UwModem
{
public:
	/**
	 * Enum type for the modem general state.
	 * AVAILABLE: immediately available to perform an operation
	 * TRANSMITTING: executing a tranmission command: must wait for it ot complete
	 * CONFIGURING: executing a configuration command: must wait for it to complete
	 */
	enum class ModemState { AVAILABLE = 0, TRANSMITTING, CONFIGURING };

	/**
	 * Enum type for the trasnmission state.
	 * TX_IDLE: the modem is available for transmission of a new packet
	 * TX_WAITING: a packet was tranmitted but the *command* was not acknowledged
	 */
	enum class TransmissionState { TX_IDLE = 0, TX_WAITING };

	/**
	 * Constructor of the UwAhoiModem class
	 * @param address string containing the address to connect to
	 * @param buflen lenght in char of the data buffer
	 * @param read len length in char of a signle read from the connector
	 */
	UwAhoiModem();

	/**
	 * Destructor of the UwAhoiModem class
	 */
	virtual ~UwAhoiModem();

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
	 * Mehod that detach a thread devoted to receiving data from the connector.
	 */
	virtual void receivingData();

	/**
	 * Method that updates the status of the modem State Machine: state change
	 * is triggered by recepting the response packet from the ahoi! modem on the
	 * connector interface.
	 * @param packet packet received on the connector interface
	 */
	void updateStatus(std::shared_ptr<ahoi::packet_t> packet);

	/**
	 * Method that fills the fields of an ahoi! packet with the needed values,
	 * so that later it can be serialized.
	 * param p Packet pointer of the DESERT packet */
	ahoi::packet_t fillAhoiPkt(Packet *p);

	/**
	 * Method that fills up a packet with the needed header and payload
	 * and makes it ready to be sent to the layer above
	 * @param p instance of an empty packet to be filled
	 */
	void createRxPacket(Packet *p);

	/**
	 * Method that extarcts fields from a provided ahoi! packet and save the
	 * info values to class field for later access. Info parameters include
	 * values such as received power, RSSI, number of corrupted packets, ecc...
	 * @param packet the packet from which to extract the values
	 */
	bool parseFooter(std::shared_ptr<ahoi::footer_t> footer);

	/**
	 * Method that extarcts fields from a provided ahoi! packet and save the
	 * info values to class field for later access. Info parameters include
	 * values such as received power, RSSI, number of corrupted packets, ecc...
	 * @param header ahoi header the packet from which to extract the values
	 * @param p ns2 packet in which to save the destination address, ecc...
	 */
	bool storePacketInfo(std::shared_ptr<ahoi::packet_t> header, Packet *p);

	/**
	 * Method for Sequence Number (SN) update
	 */
	void updateSN();

	/** Pointer to Connector object that interfaces with the device */
	std::unique_ptr<UwConnector> p_connector;

	/** Pointer to Interpreter object to parse device syntax */
	std::unique_ptr<UwInterpreterAhoi> p_interpreter;

	ModemState status; /**< Variable holding the current status of the modem */

	TransmissionState tx_status; /**< Variable holding the current transmission
									status of the modem */

	/** Mutex associated with the state machine of the modem */
	std::mutex status_m;
	/** Mutex associated with the state machine of the transmission process */
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
	/**Object with the rx thread */
	std::thread rx_thread;
	/** Object with the tx thread */
	std::thread tx_thread;
	/** String that is updated witn each new received messsage */
	std::string rx_payload;
	/** Maximum time to wait for modem to become ModemState::AVAILABLE */
	const static std::chrono::milliseconds MODEM_TIMEOUT;

	static uint MAX_RETX; /**< Maximum number of retransmissions for
							the same packet */

	static uint8_t sn; /**< Sequence number, which follows all the packets
						transmitted to the modem. */

	double virtual_time_ref; /**< virtual time reference */

	/** minimum time to wait before to schedule a new event in seconds*/
	static const double EPSILON_S;

	/** Time interval to wait for the modem notifying the response of a given
		packet*/
	std::chrono::milliseconds WAIT_DELIVERY;

	/** Time interval matching the WAIT_DELIVERY variable: version of type int
		to match the chrono one, needed because TclObject::bind does not
		support binding std::chrono variable [milliseconds]  */
	static uint WAIT_DELIVERY_INT;

	/** Packet to be checked to unlock state machine */
	ahoi::packet_t tmpPacket;

	/** Modem ID, to be set in simulation */
	uint id;

	/** flag for parity bit */
	int parity_bit;
	/** flag for stop bit */
	int stop_bit;
	/** flag for flow control */
	int flow_control;
	/** Integer for port baud rate */
	int baud_rate;

	// Parameters values retrieved from each packet footer
	/** Received power (RMSE) ratio and "ideal packet". Above 100 = clipping */
	uint power;
	/** RSSI vs an "ideal packet". Above 100 = clipping*/
	uint rssi;
	/** Number of repaired bit errors during reception in the last packet */
	uint bit_errors;
	/** Mean gain set by the AGC (if used) in reception of the last packet */
	uint agc_mean;
	/** Minimum gain set by the AGC (if used) in reception of the last packet */
	uint agc_min;
	/** Maximum gain set by the AGC (if used) in reception of the last packet */
	uint agc_max;
};

#endif
