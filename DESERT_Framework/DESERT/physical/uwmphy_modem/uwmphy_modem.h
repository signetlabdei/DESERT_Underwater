//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file uwmphy_modem.h
 * @author Riccardo Masiero, Matteo Petrani
 * \version 2.0.0
 * \brief Header of the main class that implements the general interface between ns2/NS-Miracle and real acoustic modems. 
 */

#ifndef UWMPHY_MODEM_H
#define UWMPHY_MODEM_H


#include "uwmdriver.h"
#include <uwal.h>
#include <hdr-uwal.h>
#include <uwip-module.h>
#include <mac.h>
#include <mphy.h> 

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <time.h>


#define _MTBL 5 /**< Defintion of the maximum length of the transmission buffer to store packets at the interface level. */

using namespace std;

// Forward declaration
class CheckTimer; 

/**
 * The main class implementing the module used to implement the interface between ns2/NS-Miracle and real acoustic modems. UWMPhy_modem (as well as its possible derived classes) handles all the messages needed by NS-Miracle (e.g., cross-layer messages between MAC and PHY)  and contains all the variables set by the tcl-user; furthermore, this module coordinates the interactions between UWMcodec and UWMdriver. This class replaces the physical layer of NS-Miracle and inherits from MPhy (see NS-Miracle documentation at  http://telecom.dei.unipd.it/ns/miracle/doxygen/classMPhy.html).
 */
class UWMPhy_modem : public MPhy
{         
        /**
         * Friend class used to implement the timer handler.
         *   
         * @see CheckTimer
         */
	 friend class CheckTimer;

public:
	 /** 
	  * Class constructor.
	  * 
	  * @param pToDevice_ the path to the device that must be connected with NS-Miracle (e.g., /dev/ttyUSB0 for a serial connection)
	  */ 
	  UWMPhy_modem(std::string);

	 /**
	  * Class destructor.
	  */
	 ~UWMPhy_modem();

	 /** 
	  * Method to handle the reception of packets arriving from the upper layers of the network simulator.
	  * 
	  * @param p pointer to the packet that has been received from the simulator's upper layers.
	  */
	  virtual void recv(Packet*);

	 /** 
	  * Method to map tcl commands into c++ methods.
	  * 
	  * @param argc number of arguments in <em> argv </em> 
	  * @param argv array of arguments where <em> argv[3] </em> is the tcl command name and <em> argv[4, 5, ...] </em> are the parameters for the corresponding c++ method.
	  */
	  virtual int command(int, const char*const*);

	 /**
	  * Method to return the node ID.
	  * 
	  * @return UWMPhy_modem::ID
	  */
	  int getID()
	  {
		  return ID;
	  }
	 
	 
	 /**
	  * Method to return the checking period of the modem (i.e., the time interval between two succesive checks on the modem buffer).
	  * 
	  * @return UWMPhy_modem::period
	  */
	  double getPeriod()
	  {
		  return period;
	  } 
	 
	 /**
	  * Method to return the path to the device to be connect with the network simulator.
	  * 
	  * @return UWMPhy_modem::pToDevice
	  */
	  std::string getPathToDevice()
	  {
		  return pToDevice;
	  }
	 
	 /**
	  * Method to return the flag used to enable debug messages.
	  * 
	  * @return UWMPhy_modem::debug_
	  */
	  int getDebug()
	  {
		  return debug_;
	  }	    
	  /**
	  * Method to return the name of the file where to log messages.
	  * 
	  * @return UWMPhy_modem::logFile
	  */
	  std::string getLogFile()
	  {
		  return logFile;
	  }
	  
	  /**
	  * Method to return the flag used to enable the printing of log messages in UWMPhy_modem::logFile.
	  * 
	  * @return UWMPhy_modem::log_
	  */
	  int getLog()
	  {
		  return log_;
	  }
          
          int getBin()
          {
              return bin_;
          }
          
                   
        /**
         * Calculate the epoch of the event. Used in sea-trial mode
         * @return the epoch of the system
         */
         inline unsigned long int getEpoch() {return time(NULL);}

	 /**
	  * Method to update the value of the pointer to the last received packet. This method should be used by an object of the class UWMcodec.
	  * @see UWMcodec
	  * 
	  * @param[in] p pointer to the last received packet (recovered from the last received acoustic message)
	  * @param[out] PktRx (i.e., the member UWMPhy_modem::PktRx)
	  */
	  void updatePktRx(Packet*);

protected:

	 CheckTimer* pcheckTmr; /**< Pointer to an object to schedule the "check-modem" events. */
	 UWMdriver* pmDriver; /**< Pointer to an object to drive the modem operations. */

	 int ID; /**< ID of the node. NOTE: when the node transmits, this value must coincide with the source ID. */
	 double period; /**< Checking period of the modem's buffer. */
	 Packet *modemTxBuff[_MTBL]; /**< Transmission buffer to store packets that cannot be sent immediately because the real acoustic modem is busy (receiving or transmitting). */
	 int t; /** Transmission buffer's index; it must be in {-1, 0, 1, ..., _MTBL-1}. */
	 Packet* PktRx; /**< Address of the last received packet. */
	 std::string pToDevice; /**< A string containing the path to the device to be connected with the network simulator.*/
	 int debug_; /**< Flag to enable debug mode (i.e., printing of debug messages) if set to 1. */
         std::ofstream outLog; /**< output strem to print into a disk-file log messages. See UWMPhy_modem::logFile.*/
	 int bin_;	 
	 std::string logFile; /**< Name of the disk-file where to write the interface's log messages.*/	
	 int log_; /**< Flag to enable, if set different than 0, the printing of log messages in UWMPhy_modem::logFile. */
         int SetModemID; /**< Flag to indicate if the interface has to force the modem to have the ID indicated in the tcl script */
	 
	 /** 
	  * Link connector. This method must be used by any derived class D of UWMPhy_modem  to link the members pcheckTmr, pmDriver and pmCodec to the corresponding derived objects contained in D;
	  * @see e.g., MFSK_WHOI_MM or MS2C_EvoLogics
	  * 
	  * @param[in] pcheckTmr_ pointer to a CheckTimer object
	  * @param[in] pmDriver_ pointer to an UWMdriver object
	  * @param[in] pmCodec_ pointer to an UWMcodec object
	  * @param[out] pcheckTmr (i.e., the member UWMPhy_modem::pcheckTmr)
	  * @param[out] pmDriver (i.e., the member UWMPhy_modem::pmDriver)
	  * @param[out] pmCodec (i.e., the member UWMPhy_modem::pmCodec)
	  */
         void setConnections(CheckTimer*, UWMdriver*);

	 /** 
	  *  Connection starter. This method starts the connection with the modem. It performs all the needed operations 
	  *  to open an host-modem connection (e.g., set up of the connection port's parameters, start of the "check-modem" process).
	  */
	 virtual void start();

	 /**
	  *  Connection stopper. This method should be used before stopping the simulation. It closes and, if needed, 
	  *  resets all the opened files and ports.
	  */
	 virtual void stop();

	 /** 
	  *  Modem checker. This method is at the core of the "check-modem" process. It is called periodically 
	  *  by the timer object linked to this UWMPhy_modem object (see UWMPhy_modem::checkTmr_); its due is to verify if
	  *  something has been received or is going to be received from the channel. 
	  * 
	  *  @return modemStatus, a flag on the status of the modem, see UWMdriver for a description of the driver state machine.
	  */
	 virtual int check_modem();

	 /**
	  *  Method to send to an UWMdriver object the packet to be transmitted, see UWMdriver::modemTx(). 
	  * 
	  *  @param p pointer to the packet to be transmitted.
	  */
	 virtual void startTx(Packet*);

	 /**
	  *  Method to end a packet transmission. This method is also in charge to send a cross layer message Phy2MacEndTx(p) to notify the above layers of the simulator about the end of a transmission, see http://telecom.dei.unipd.it/ns/miracle/doxygen/classMPhy.html#c8c2f52d3833fed8ac736aca0ee52c98. 
	  * 
	  *  @param p pointer to the last transmitted packet.
	  */
	 virtual void endTx(Packet*);

	 /**
	  *  Method to start a packet reception. This method is also in charge to send a cross layer message Phy2MacStartRx(p) to notify the above layers of the simulator about the start of a reception, see http://telecom.dei.unipd.it/ns/miracle/doxygen/classMPhy.html#a15cc91b98013e1c631ad85072867ab6. 
	  * 
	  *  @param p pointer to the last received packet.
	  */
	 virtual void startRx(Packet*);

	 /**
	  *  Method to end a packet reception. This method is also in charge to send the received NS-Miracle packet (as recovered from the received acoustic packet) to the above layers of the simulator. 
	  * 
	  *  @param p pointer to the last received packet.
	  */
	 virtual void endRx(Packet*);

	 /**
	  * Method to pop the oldest packet in the TX buffer or to delete after a tx or to drop it. 
	  * 
	  *  @param temp pointer to the popped packet. 
	  */
	 Packet* popTxBuff();
	 
	 /**
	  * Method to get the transmission duration for a given packet. NOTE: not implemented or used at the moment.
	  * 
	  *  @param p a pointer to the packet for which to return the duration.
	  *  @return the transmission duration.  
	  */
	 virtual double getTxDuration(Packet*) {}

	 /**
	  * Method to get the modulation type used to transmit/receive a given packet. NOTE: not implemented or used at the moment.
	  * 
	  *  @param p a pointer to the packet for which to return the modulation used.
	  *  @return the modulation type.  
	  */
	 virtual int getModulationType(Packet*) {}

	 
};

/** 
 *  The class used by UWMPhy_modem to handle simulator's event expirations; it is exploited to schedule the reading process from the modem; 
 *  see the ns2 manual for more details about TimerHandler.
 */
class CheckTimer : public TimerHandler
{
public:
	
        /** 
	 * Class constructor.
	 * 
	 * @param pmModem_ pointer to the UWMPhy_modem object to link with this CheckTimer object.
	 */ 	 
	 CheckTimer(UWMPhy_modem* pmModem_) : TimerHandler()
	 {
		  pmModem = pmModem_;
		  if (pmModem -> getDebug() >= 2)
		  {
				cout << this << ": in constructor of CheckTimer which points to modem: " << pmModem << "\n";
		  }
	 }

protected:
         
         /** 
	 * Method to handle the expiration of a given event.
	 * 
	 * @param e event to be handled.
	 */
	 virtual void expire(Event* e);

	 UWMPhy_modem* pmModem; /**< Pointer to an UWMPhy_modem object. It is used to call UWMPhy_modem::check_modem() when the countdown expires.*/
};
#endif	/* UWMPHY_MODEM_H */