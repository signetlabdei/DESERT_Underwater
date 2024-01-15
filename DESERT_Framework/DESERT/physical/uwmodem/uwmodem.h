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
 * @file    uwmodem.h
 * @author  Roberto Francescon
 * @version 0.1.0
 * @brief   Header of the main class that implements the general interface
 * 			between DESERT and real acoustic modems.
 */

#ifndef UWMODEM_H
#define UWMODEM_H

#include <iostream>
#include <memory>
#include <queue>
#include <string>

#include <functional>
#include <hdr-uwal.h>
#include <mac.h>
#include <mphy.h>
#include <tclcl.h>
#include <uwal.h>
#include <uwip-module.h>

class CheckTimer;
struct ModemEvent;
/**
 * Class that implements the interface to DESERT, as used through Tcl scripts.
 * This class provides common functions to operate as a physical layer;
 * its derivate classes will implement its virtual methods as specific to each
 * device.
 */
class UwModem : public MPhy
{
	friend class CheckTimer;

public:
	/**
	 * Enum representing the amount of logs being generated
	 * ERROR: only errors will be generated
	 * INFO : general info about the algorithms running
	 * DEBUG: details that allow to understand the execution flow
	 */
	enum class LogLevel { ERROR = 0, INFO = 1, DEBUG = 2 };

	/**
	 * Method that converts a string representing the loglevel
	 * into the enum type of loglevel
	 * @param ll_string string to be converted
	 * @param ll return parameter containing the converted loglevel
	 * @return boolean true if conversion was correctly performed
	 */
	static bool string2log(const std::string &ll_string, LogLevel &ll);

	/**
	 * Method that converts an enum type of the loglevel
	 * into the string representing it
     * @param ll return loglevel to be converted
	 * @param ll_string return param string representing the loglevel
	 * @return boolean true if conversion was correctly performed
	 */
	static bool log2string(LogLevel ll, std::string &ll_string);

	/**
	 * UwModem constructor
	 * @param address string representing the address to connect to
	 */
	UwModem();

	/**
	 * UwModem destructor
	 */
	virtual ~UwModem();

	/**
	 * Method that handles the reception of packets arriving from the
	 * upper layers of the network simulator.
	 * @param p pointer to the packet that has been received from the simulator
	 *        upper layers
	 */
	virtual void recv(Packet *p) = 0;

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
	 *
	 * @param ClMessage* an instance of ClMessage that represent the
	 * message received
	 * @return <i>0</i> if successful.
	 */
	virtual int recvSyncClMsg(ClMessage *m) = 0;

	/**
	 * Method that returnd the duration of a given transmitted packet.
	 * Inherited from MPhy, in NS-MIRACLE, could be empty if there is no way
	 * to retrieve this information.
	 * @param p Packet pointer to the given packet being transmitted
	 * @return duration in seconds
	 */
	virtual double
	getTxDuration(Packet *p)
	{
		return -1.0;
	}

	/**
	 * Method that should return the modulation type used for the packet being
	 * transmitted. Inherited from MPhy, in NS-MIRACLE, could be left empty if
	 * no way exists to retrieve this information
	 * @param p Packet pointer to the given packet being transmitted
	 * @param modulation type represented by an integer
	 */
	virtual int
	getModulationType(Packet *P)
	{
		return 0;
	}

	/**
	 * Set the modem's ID number. Used for identification purposes
	 * @param ID integer number used as modem ID
	 */
	void
	setModemID(int ID)
	{
		modemID = ID;
	}

	/**
	 * Method that returns the current ID of the modem.
	 * @return integer current ID of the modem
	 */
	int
	getModemID()
	{
		return modemID;
	}

	/**
	 * Function that, given the appropriate level of log, prints to the set
	 * log file the provided log message.
	 */
	void printOnLog(LogLevel log_level, string module, string message);

	/**
	 * Method to return the flag used to enable the printing of log messages in
	 * UwEvoLogicsS2CModem::logFile.
	 * @return UwEvologicsS2CModem::log_
	 */
	LogLevel
	getLogLevel()
	{
		return loglevel_;
	}

	/**
	 * Method to return the flag used to enable debug messages.
	 * @return UwEvoLogicsS2CModem::debug_
	 */
	int
	getDebug()
	{
		return debug_;
	}

	/**
	 * Method to return the name of the file where to log messages.
	 * @return UwEvoLogicsS2CModem::logFile
	 */
	std::string
	getLogFile()
	{
		return logFile + std::to_string(modemID) + log_suffix;
	}

	/**
	 * Method to call endTx from end of real packet transmission
	 * @param p Packet pointer to the packet being received
	 */
	void
	realTxEnded(Packet *p)
	{
		endTx(p);
	}

protected:
	int modemID; /**< Number used for identification purposes: not specified */

	/**
	 * Char buffer (vector) that holds data read from the modem (unparsed data)
	 * Main container for data received by the connector.
	 */
	std::vector<char> data_buffer;

	/** Modem's transmission queue: holds packets that are to be transmitted */
	std::queue<Packet *> tx_queue;

	/** Modem's reception queue: holds packets eceived from the channel awaiting
	 * to be pushed up the stack
	 */
	std::queue<Packet *> rx_queue;

	/** Size of the buffer that holds data */
	unsigned int DATA_BUFFER_LEN;

	/** Maximum number of bytes to be read by a single dump of data */
	int MAX_READ_BYTES;

	/**
	 * String containing the address needed to connect to the device
	 * In case of socket, it may be expressed as: 192.168.XXX.XXX:PORTNUM
	 * In case of serial, it will be expressed in some other way
	 */
	std::string modem_address;

	/** Usual debug value that chooses the debug level through Tcl interface */
	int debug_;

	std::ofstream outLog; /**< output strem to print into a disk-file log
							 messages. See UwEvoLogicsS2CModem::logFile.*/
	std::string logFile; /**< Name of the disk-file where to write the
							interface's log messages.*/
	std::string log_suffix; /**< Possibility to insert a log suffix */
	LogLevel loglevel_; /**< Log level on file, from ERROR (0) to DEBUG (2) in
					  UwEvoLogicsS2CModem::logFile. */
	bool log_is_open; /**< Flag to check if log file has already be opened*/
	CheckTimer *checkTimer; /**< Pointer to an object to schedule the
							  "check-modem" events. */
	double period; /**< Checking period of the modem's buffer. */
	/** Queue of events that are scheduled for NS2 to execute (callbacks) */
	std::queue<ModemEvent> event_q;

	/**
	 * Method that triggers the transmission of a packet through a specified
	 * modem.
	 * @param p Packet pointer to the packet to be sent
	 */
	virtual void startTx(Packet *p) = 0;

	/**
	 * Method that ends a packet transmission. This method is also in charge of
	 * sending a CrLayerMsg, Phy2MacEndTx(p), to notify the above layers of
	 * the simulator about the end of a transmission
	 * @param p Packet pointer to the packet being received
	 */
	virtual void endTx(Packet *p);

	/**
	 * Method that starts a packet reception. This method is also in charge of
	 * sending a CrLayerMsg, Phy2MacStartRx(p), to notify the upper layers of
	 * the simulator about the start of the reception
	 * @param p Packet pointer to the packet to be received
	 */
	virtual void startRx(Packet *p) = 0;

	/**
	 * Method that ends a packet reception. This method is also in charge of
	 * sending the received NS-MIRACLE packet to the upper layers
	 * @param p Packet pointer to the packet being sent
	 */
	virtual void endRx(Packet *p) = 0;

	/**
	 * Method that starts the driver operations. It performs all the needed
	 * operations to correctly fire up the device's driver.
	 */
	virtual void start() = 0;

	/**
	 * Method that stops the driver operations. It performs all the needed
	 * operations to correctly stop the device's driver before closing the
	 * simulation.
	 */
	virtual void stop() = 0;

	/**
	 * Method to check if any event from real world has to go to ns
	 */
	void checkEvent();
};

/**
 * The class used by UwModem to handle simulator's event expirations; it
 * is exploited to schedule the reading process from the modem;
 * see the ns2 manual for more details about TimerHandler.
 */
class CheckTimer : public TimerHandler
{
public:
  /**
   * Class constructor.
   *
   * @param pmModem_ pointer to the UwModem object to link with this
   *CheckTimer object.
   */
  CheckTimer(UwModem *pmModem_)
    : TimerHandler()
  {
    pmModem = pmModem_;
    if (pmModem->getDebug() >= 2) {
      std::cout << this
                << ": in constructor of CheckTimer which points to modem: "
                << pmModem << std::endl;
    }
  }

protected:
  /**
   * Method to handle the expiration of a given event.
   *
   * @param e event to be handled.
   */
  virtual void expire(Event *e);

  UwModem *pmModem; /**< Pointer to an UwModem object. It is used to
                         call UwModem::checkEvent() when the countdown
                         expires.*/
};

struct ModemEvent {
	std::function<void(UwModem &, Packet *p)> f;
	Packet *p;
};

#endif
