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
 * @file msocket.h
 * @author Riccardo Masiero
 * \version 2.0.0
 * \brief  Header of the class derived from UWMconnector to handle the TCP/IP socket connection of a client between NS-Miracle and a modem. 
*/

#ifndef MSOCKET_H
#define MSOCKET_H

#include <uwmconnector.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

using namespace std;

/**
 * Class used to manage host/modem string exchange via TCP/IP connection (this class is a derived class of UWMconnector).
 */
class Msocket: public UWMconnector 
{   
   int sockfd; /**< Pointer to the socket. */
   int portno; /**< Port number. */
   int rc; /**< Indicator of the thread creation. */
   pthread_t thread_id; /**< Id of a parallel thread. */
   struct sockaddr_in serv_addr; /**< Structure to contain the Internet address to be used. */
   std::string server_host; /**< Name of the server host on the Internet to which we want to be connected as client. */ 
   struct hostent *server; /**< Structure to define the server host. */
   char msg_tx[_MAX_MSG_LENGTH]; /**< Message to be transmitted host to modem. */
   
   /**
    *  Method report errors.
    *  @param[in] msg pointer to an error message to report.
    */
   void error(const char*);
  
   public: 
   
   /** 
    * Class constructor.
    * 
    * @param pmDriver_  pointer to the UWMdriver object to link with this UWMconnector object.
    * @param pathToDevice_ the path to the device that must be connected with NS-Miracle (e.g., the port "9200" for a TCP/IP connection)
    */
   Msocket(UWMdriver*,std::string);

   /**
    * Class destructor.
    */
   ~Msocket();
    
   /**
    * Method to open the connection with the modem. 
    * 
    *  @return _MODEM_OK (see uwmconnector.h) if everything went fine, any other int value otherwise.
    */
   virtual int openConnection();
   
   /**
    * Method to close the connection with the modem. 
    */ 
   virtual void closeConnection();
   
   /**
    * Method for writing to the modem. 
    * 
    * @param[in] str the string to pass to the modem
    * @return w_bytes, the number of transmitted bytes 
    */
   virtual int writeToModem(std::string);
   
   /**
    * Method to read the pointer to the socket where to write (it is needed by the reading function invoked by the parallel thread)
    * 
    * @return Msocket::sockfd 
    */
   int getSocket(){return sockfd;}
	
   /**
    * Method to return the readingBuff string (it is needed by the reading function invoked by the parallel thread)
    * 
    * @return UWMconnector::readingBuff
    */ 
   //std::string getReadingBuff(){return readingBuff;}
        
   /**
    * Method to return the debug_ flag (it is needed by the reading function invoked by the parallel thread)
    * 
    * @return UWMconnector::debug_
    */
   int getDebug(){return debug_;}
};


/** 
 * Function to read from the modem via a TCP/IP connection (it must be called as a pure C function).
 */
extern "C"
{
	void *read_process_msocket(void *);
}

#endif /* MSOCKET_H */