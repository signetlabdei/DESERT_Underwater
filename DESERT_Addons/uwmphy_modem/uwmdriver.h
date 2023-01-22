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
 * @file uwmdriver.h
 * @author Riccardo Masiero
 * @author Federico Favaro
 * \version 2.0.0
 * \brief Header of the class needed by UWMPhy_modem to handle the different
 * transmissions cases and corresponding protocol messages to be generated
 * according to the tcl-user choices and modem firmware, respectively.
 */

#ifndef UWMDRIVER_H
#define UWMDRIVER_H

#include "uwmconnector.h"
#include "uwminterpreter.h"

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#define MAX_LOG_LEVEL 2

enum MODEM_STATES {
	MODEM_IDLE = 0,
	MODEM_TX,
	MODEM_RX,
	MODEM_IDLE_RX,
	MODEM_CFG,
	MODEM_TX_PAUSED,
	MODEM_TX_RX,
	MODEM_RESET,
	MODEM_NOISE,
	MODEM_MULTIPATH,
	MODEM_CHANGE_POWER_LEVEL,
	MODEM_QUIT
};

enum LOG_LEVEL { LOG_LEVEL_ERROR = 0, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG };

typedef enum LOG_LEVEL log_level_t;

typedef enum MODEM_STATES modem_state_t;



// Forward declaration(s)
class UWMPhy_modem;
class UWMcodec;

/**
 * The class needed by UWMPhy_modem to handle the different transmissions cases
 * and corresponding protocol messages to be generated according to the tcl-user
 * choices and modem firmware, respectively. This class contains all the basic
 * functions needed to drive modem in transmitting/receiving messages. NOTE:
 * this class must be inherited and extended to implement any specific modem
 * driver.
 */
class UWMdriver
{
public:
	/**
* Class constructor.
*
* @param pmModem_ pointer to the UWMPhy_modem object to link with this UWMdriver
*object.
*/
	UWMdriver(UWMPhy_modem *);

	/**
	 * Class destructor.
	 */
	~UWMdriver();

	virtual void modemSetID() = 0;

	/**
	 *  Driver starter. This method starts the driver performing all the needed
	 * operations
	 *  to open an host-modem connection.
	 */
	virtual void start() = 0;

	/**
	 *  Driver stopper. This method should be used before stopping the
	 * simulation. It closes and, if needed,
	 *  resets all the opened files and ports.
	 */
	virtual void stop() = 0;

	/**
	 *  Method to notify to the driver that there is a packet to be sent via
	 * modem.
	 *  NOTE: when this function is called (by an UWMPhy_modem object), the
	 * driver's status must be set to TX_ and the packet must be sent
	 * immediately to the modem.
	 */
	virtual void modemTx() = 0;
	/**
	 *  Method to update modem status. This method has to update the modem
	 *status according to the  messages
	 *  received from the modem/channel (e.g., after a check of the modem
	 *buffer's output). NOTE: This method may return after an arbitrary period
	 *if nothing has happened, but it must return immediately after a change of
	 *UWMdriver::status.
	 *
	 *  @return UWMdriver::status, the updated modem's status.
	 */
	virtual modem_state_t updateStatus() = 0;

	/**
	 * Method to change the modem ID. This method is called by the UWMPhy_modem
	 *object linked to this UWMdriver (the one pointed by pmModem).
	 *
	 *  @param[in] ID the ID that must be assigned to the modem.
	 *  @param[out] ID (i.e., the member UWMdriver::ID), changed to \e ID_.
	 */
	inline void
	setID(int ID_)
	{
		ID = ID_;
	}

	/**
	 *  Method to reset the modem status. NOTE: this function should be used by
	 *the UWMPhy_modem object linked to this
	 *  UWMdriver (the one pointed by pmModem) to reset the status to _IDLE
	 *after the processing of a received packet or
	 *  to abort a given ongoing reception process.
	 *
	 *  @param[out] status  (i.e., the member UWMdriver::status), changed to \e
	 *IDLE_.
	 */
	void resetModemStatus();

	/**
	 * Method to update the values of both UWMdriver::payload_tx and
	 *UWMdriver::dest. NOTE: This method should be used by an
	 * UWMcodec object linked to this UWMdriver (and contained in the
	 *UWMPhy_modem object pointed by pmModem)
	 *
	 *  @param[in] d the ID of the modem to which transmit the next packet.
	 *  @param[in] ptx the payload of the modem packet to transmit acoustically.
	 *  @param[out] dest  (i.e., the member UWMdriver::dest), changed to \e d.
	 *  @param[out] payload_tx  (i.e., the member UWMdriver::payload_tx),
	 *changed to \e ptx.
	 */
	void updateTx(int, std::string);

	/**
	 * Method to write in UWMdriver::payload_rx, UWMdriver::src and
	 *UWMdriver::dstPktRx. NOTE: This method should be used
	 * by an UWMinterpreter object linked to this UWMdriver (and contained in
	 *the UWMPhy_modem object pointed by pmModem)
	 *
	 *  @param[in] s the ID of the modem that sent the last received packet.
	 *  @param[in] d the ID of the modem that is the destination of the last
	 *received packet.
	 *  @param[in] prx the payload of the last packet acoustically received.
	 *  @param[out] src  (i.e., the member UWMdriver::src), changed to \e s.
	 *  @param[out] dstPktRx  (i.e., the member UWMdriver::dstPktRx), changed to
	 *\e d.
	 *  @param[out] payload_rx  (i.e., the member UWMdriver::payload_rx),
	 *changed to \e prx.
	 */
	void updateRx(int, int, std::string);

	/**
	 * Method to return modem ID.
	 *
	 * @return UWMdriver::ID.
	 *
	 */
	int
	getID()
	{
		return ID;
	}

	/**
	 * Method to return the modem's status.
	 *
	 * @return UWMdriver::status.
	 */
	modem_state_t
	getStatus()
	{
		return status;
	}

	/**
	 * Method to access to the payload of the last packet acoustically received.
	 *NOTE: This function should be used by the
	 * UWMcodec object linked to this UWMdriver (and contained in the
	 *UWMPhy_modem object pointed by pmModem) to recover the
	 * orginal NS-Miracle to be sent to the above protocol layers of the
	 *simulator.
	 *
	 * @return UWMdriver::payload_rx.
	 */
	std::string
	getRxPayload()
	{
		return payload_rx;
	}

	/**
	 * Method to access to the ID of the source of the last packet acoustically
	 *received. NOTE: This function should be used
	 * by the UWMcodec object linked to this UWMdriver (and contained in the
	 *UWMPhy_modem object pointed by pmModem) to
	 * recover the orginal NS-Miracle to be sent to the above protocol layers of
	 *the simulator.
	 *
	 * @return UWMdriver::src.
	 */
	int
	getSrc()
	{
		return src;
	}

	/**
	 * Method to access to the ID of the destination of the last packet
	 *acoustically received. NOTE: This function should be
	 * used by the UWMcodec object linked to this UWMdriver (and contained in
	 *the UWMPhy_modem object pointed by pmModem) to
	 * recover the orginal NS-Miracle to be sent to the above protocol layers of
	 *the simulator.
	 *
	 * @return UWMdriver::dstPktRx.
	 */
	int
	getDstPktRx()
	{
		return dstPktRx;
	}

	/**
	 * Method to return the flag used to enable debug messages.
	 *
	 * @return UWMPhy_modem::debug_
	 */
	inline log_level_t
	getDebug()
	{
		return (log_level_t) debug_;
	}

	/**
	 *
	 * @param set true if the interface has to set the modem ID
	 */
	inline void
	setModemID(bool set)
	{
		SetModemID = set;
	}
	/**
	 *
	 * @return true if the interface sets the modem ID
	 */
	inline bool
	getModemID()
	{
		return SetModemID;
	}

	inline void
	setResetModemQueue(bool reset_m_queue)
	{
		ResetModemQueue = reset_m_queue;
	}

	inline bool
	getResetModemQueue()
	{
		return ResetModemQueue;
	}
	/**
	 * Method to return the flag used to enable the printing of log messages in
	 *UWMPhy_modem::logFile.
	 *
	 * @return UWMPhy_modem::log_
	 */
	log_level_t getLogLevel();

	/**
	 * Method to return the name of the disk-fiel used to print the log
	 *messages.
	 *
	 * @return UWMPhy_modem::logFile
	 */
	std::string getLogFile();

	void printOnLog(log_level_t log_level, std::string module, std::string message);

protected:
	UWMPhy_modem *pmModem; /**< link to the UWMPhy_modem object that contains
							  this driver */

	UWMinterpreter
			*pmInterpreter; /**< pointer to the object that builds/parses the
							   necessary
							   messages to make
									   UWMdriver able to communicate with a real
							   modem */
	UWMconnector *pmConnector; /**< pointer to the object that handles the
								  physical transmission and
										  reception of acoustic packets */

	int ID; /**< ID of the modem. NOTE: UWMdriver::ID (i.e., modem ID, hardware
			   side) is set equal to UWMPhy_modem::ID (i.e., node ID, simulator
			   side) (therefore when node ID transmits, it also coincides with
			   the
			   source ID). @see UWMPhy_modem::start(), UWMdriver::setID(int) */

	modem_state_t status; /**< Status of the driver's general state machine.
							 Seven possible
							 statuses = \e _IDLE, \e _TX, \e _RX , \e
							 _IDLE_RX,\e _CFG, \e
							 _TX_PAUSED and \e _TX_RX.*/

	// TX VARIABLES (variables for the next packet to be transmitted)
	std::string payload_tx; /**< String where to save the payload of the next
							   packet to send via modem. NOTE: an object of the
							   class UWMcodec must write here after the
							   host-to-modem mapping. */
	int dest; /**< Variable where to save the destination ID of the next packet
				 to
				 send via modem. NOTE: an object of the class UWMcodec must
				 write
				 here after the host-to-modem demapping. */

	// RX VARIABLES (variables reffering to the last received packet)
	std::string payload_rx; /**< String where to save the payload of the last
							   packet
							   received via modem. NOTE: an object of the class
							   UWMinterpreter must write here after a the
							   parsing of a
							   received data packet; instead, an object of the
							   class
							   UWMcodec reads here before the modem-to-host
							   mapping. */
	int src; /**< Variable storing the source ID of the last packet received
				via
				modem. NOTE: an object of the class UWMinterpreter must
				write here
				after a the parsing of a received data packet; instead, an
				object of
				the class UWMcodec reads here before the modem-to-host
				mapping. */
	int dstPktRx; /**< Variable where to save the destination ID of the last
					 packet received via modem. NOTE: an object of the class
					 UWMinterpreter must write here after a the parsing of a
					 received data packet; instead, an object of the class
					 UWMcodec reads here before the modem-to-host mapping. */

	bool SetModemID; /**< Variable to decide whether the interface has to set
						the acoustic ID of the modem or not */
	int debug_; /**< Flag to enable debug mode (i.e., printing of debug
				   messages) if set to 1 */
	std::ofstream outLog; /**< output strem to print into a disk-file log
							 messages. See UWMPhy_modem::logFile.*/
	bool ResetModemQueue;
	/**
	 * Link connector. This method must be used by any derived class D of
	 *UWDriver to link the members pmInterpreter and pmConnector of UWMdriver to
	 *the corresponding derived objects contained in D.
	 * @see: e.g. MdriverS2C_EvoLogics
	 *
	 * @param[in] pmInterpreter_ pointer to a UWMinterpreter object
	 * @param[in] pmConnector_ pointer to an UWMconnector object
	 * @param[out] pmInterpreter (i.e., the member UWMPhy_modem::pmInterpreter)
	 * @param[out] pmConnector (i.e., the member UWMPhy_modem::pmConnector)
	 */
	void setConnections(UWMinterpreter *, UWMconnector *);

	/**
	 * Method to manage modem to host and host to modem communications. This
	 * method has to handle the different transmissions cases and corresponding
	 * protocol messages to be generated according to the tcl-user choices and
	 * modem firmware, respectively.
	 */
	virtual void modemTxManager() = 0;
};
#endif /* UWMDRIVER_H */
