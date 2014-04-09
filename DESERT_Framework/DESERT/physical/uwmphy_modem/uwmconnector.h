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
 * @file uwmconnector.h
 * @author Riccardo Masiero, Matteo Petrani, Ivano Calabrese
 * \version 2.0.0
 * \brief  Header of the class needed by UWMPhy_modem to handle the physical connection between NS-Miracle and a real acoustic modem device. 
 */

#ifndef UWMCONNECTOR_H
#define UWMCONNECTOR_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <queue>

#define _MODEM_OK 1 /**< Variable to test the right opening of the modem's connection. */
#define _MAX_MSG_LENGTH (0x1000) /**< Variable defining the maximum length of the messages exchanged between host and modem */
#define _MAX_QUEUE_LENGTH 20 /**< Maximum length of queue containing the messages from modem */

using namespace std;

// Forward declaration to avoid dependence from UWMdriver.h
class UWMdriver; 

// This struct is used in the queue data structure to save the messages received from the modem.
/* struct msgModem {
  char msg_rx[_MAX_MSG_LENGTH]; /**< Message from the modem.*/
//  int msg_length; /**< Length of the message (bytes).*/
//};

struct msgModem {
  string msg_rx; /**< Message from the modem.*/
  int msg_length; /**< Length of the message (bytes).*/
};

/**
 * The class needed by UWMPhy_modem to manage string exchange with the modem. This class just provides the definition of the basic needed functionalities and it must be extended to handle e.g., serial connection (see Mserial) or TCP/IP connection (see Msocket). 
 */
class UWMconnector 
{       
	public:

		queue<msgModem> queueMsg; /**< Queue used to buffer incoming strings from the modem.*/
		
		/** 
		 * Class constructor.
		 * 
		 * @param pmDriver_  pointer to the UWMdriver object to link with this UWMconnector object.
		 * @param pathToDevice_ the path to the device that must be connected with NS-Miracle (e.g., /dev/ttyUSB0 for a serial connection)
		 */
		UWMconnector(UWMdriver*,std::string);

		/**
		 * Class destructor.
		 */
		~UWMconnector();
		
		/**
		 * Method to open the connection with the modem. 
		 * 
		 *  @return _MODEM_OK if everything went fine, any other int value otherwise.
		 */
		virtual int openConnection() = 0;
		
		/**
		 * Method to close the connection with the modem. 
		 */
		virtual void closeConnection();
		
		/**
		 * Method for writing to the modem. 
		 * 
		 * @param[in] str the string to pass to the modem
		 * @return the number of transmitted bytes 
		 */
		virtual int writeToModem(std::string) = 0;
		
		/**
		 * Method to check the receiving modem buffer.
		 * 
		 * @return return_str the string corresponding to the last received message from the modem
		 */
		std::string readFromModem();
		
		/**
		 * Method to create receiving modem buffer.
		 * 
		 * @param[out] disk-file that implements the receiving modem buffer and that is called according to the value set for UWMconnector::readingBuff.
		 */
		//void create_readingBuff();
		
        protected:
	        
	        UWMdriver* pmDriver; /**< Pointer to UWMdriver object that contains this UWMconnector.*/
	        std::string pathToDevice; /**< The path to be connected with the modem device */
		std::string readingBuff; /**< Name of the disk-file where to write the incoming messages from the modem (i.e., the receiving modem buffer).*/
		std::ifstream in; /**< Variable to read from the disk-file used as receiving modem buffer */
		std::ofstream out; /**< Variable to write to the disk-file used as receiving modem buffer */
		int debug_; /**< Flag to enable debug mode (i.e., printing of debug messages) if set to 1 */

};
#endif /* UWMCONNECTOR_H */

