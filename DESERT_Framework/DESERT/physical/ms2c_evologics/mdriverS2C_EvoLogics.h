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
 * @file mdriverS2C_EvoLogics.h
 * @author Riccardo Masiero and Federico Favaro
 * \version 1.0.0
 * \brief Header of the class derived from UWMdriver to interface ns2/NS-Miracle with the S2C EvoLogics acoustic modems.
 */

#ifndef UWMDRIVERS2CEVOLOGICS_H
#define UWMDRIVERS2CEVOLOGICS_H

#include "msocket.h"
#include "minterpreterAT.h"
#include <uwmdriver.h>

#include <queue>

// MACROS for the transmission management of AT messages
// MESSAGES FLAGS
//TX CONFIGURATION MESSAGES
#define _SETID 1 /**< Status 1 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_tx): set modem ID */
#define _SETIDS 2 /**< Status 2 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_tx): modem ID sent */
#define _DROPBUFFER 4 /**< Drop burst data and IM at the begin of connection with the modem */
#define _DROPBUFFERS 5 /**< Drop burst data and IM at the begin of connection with the modem */
#define _IM 11 /**< Status 11 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_tx): send instant message */
#define _IMS 12 /**< Status 12 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_tx): instant message sent to modem */
#define _RXIM -1 /**< Status -1 of the driver's AT-complaint RX state machine (see MdriverS2C_EvoLogics::m_status_rx): reception of an instant message */
#define _TXKO 7 /**< Status 7 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_rx): TX of an AT!KO command */
#define _TXKOD 8 /**< Status 7 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_rx): TX of an AT!KO command done  */
#define _BURST 13 /**< Status 7 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_rx): send a BURST message  */
#define _BURSTS 14 /**< Status 7 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_rx): BURST message sent  */
#define _PBM 15 /**< Status 7 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_rx): send a PiggyBack message  */
#define _PBMS 16 /**< Status 7 of the driver's AT-complaint TX state machine (see MdriverS2C_EvoLogics::m_status_rx): PiggyBack message sent  */
#define _RXBURST 20 /**< Status -1 of the driver's AT-complaint RX state machine (see MdriverS2C_EvoLogics::m_status_rx): reception of a BURST message */
#define _RXPBM 21 /**< Status -1 of the driver's AT-complaint RX state machine (see MdriverS2C_EvoLogics::m_status_rx): reception of a PiggyBack message */
#define _CLOSE 23 /**< Status -1 of the driver's AT-complaint RX state machine (see MdriverS2C_EvoLogics::m_status_rx): reception of a PiggyBack message */
#define _CLOSED 24 /**< Status -1 of the driver's AT-complaint RX state machine (see MdriverS2C_EvoLogics::m_status_rx): reception of a PiggyBack message */


/**
 * Class containing the basic functions to drive the S2C EvoLogics acoustic modem transmissions/receptions (this class is a derived class of UWMdriver).           
 */
class MdriverS2C_EvoLogics : public UWMdriver
{       
       MinterpreterAT mInterpreter; /** < Object that builds/parses AT messages. */
       Msocket mConnector; /** < Object that handles the physical host to modem communications via TCP/IP sockets. */
       
       int m_status_tx; /**< TX status for the transmission manager methods, see methods MdriverS2C_EvoLogics::modemTxManager and MdriverS2C_EvoLogics::updateStatus. */
            
       int m_status_rx; /**< RX status for the MdriverS2C_EvoLogics::updateStatus() method. */
       
       queue<std::string> queue_tx; /**< Queue used to buffer incoming strings for tx messages.*/
       
       queue<std::string> queue_rx; /**< Queue used to buffer incoming strings for rx messages.*/
       
       public:
                /** 
	         * Class constructor.
	         * 
	         * @param pmModem_ pointer to the UWMPhy_modem object to link with this UWMdriver object.
	         */
		MdriverS2C_EvoLogics(UWMPhy_modem*);

		virtual void modemSetID();
		
		/**
	         * Class destructor.
	         */
		~MdriverS2C_EvoLogics();
		
		/**
	         *  Driver starter. This method starts the driver performing all the needed operations 
	         *  to open an host-modem connection. 
	         */
		virtual void start();
		
		/**
	         *  Driver stopper. This method should be used before stopping the simulation. It closes and, if needed, 
	         *  resets all the opened files and ports.
	         */
		virtual void stop();
		
		/**
	         *  Method to notify to the driver that there is a packet to be sent via modem.
	         *  NOTE: when this function is called (by an UWMPhy_modem object), the driver's status must be set to TX_ and the packet must be sent immediately to the modem.
	         */
		virtual void modemTx();
		
		/** 
	         *  Method to update modem status. This method has to update the modem status according to the  messages 
	         *  received from the modem/channel (e.g., after a check of the modem buffer's output). NOTE: This method may return after an arbitrary period if nothing has happened, but it must return immediately after a change of UWMdriver::status.
	         * 
	         *  @return UWMdriver::status, the updated modem's status.
	         */
		virtual int updateStatus();


		virtual void modemTxBurst();

		virtual void modemTxPBM();
		
		/** 
	         *  Method to get the Integrity value of the last received packet. NOTE: This method is used by McodecS2C_EvoLogics.
	         * 
	         *  @return MinterpreterAT::rx_integrity, the integrity value of the last received packet.
	         */
		double getIntegrity();
                /**
                 * Method to empty modem queue
                 */
                virtual void emptyModemQueue();
                        
                        		
	protected:

		/** 
	         * Method to manage modem to host and host to modem communications. This method has to handle the different transmissions cases and corresponding protocol messages to be generated according to the tcl-user choices and modem firmware, respectively.
	         */
		virtual void modemTxManager();
                               
};
#endif	/* UWMDRIVERS2CEVOLOGICS_H */
