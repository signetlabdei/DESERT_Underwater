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
 * @file msocket.cpp
 * @author Riccardo Masiero
 * @author Federico Favaro
 * \version 2.0.0
 * \brief  Implementation of the Msocket class.
*/

#include "msocket.h"
#include <uwmdriver.h>

#include <string>
#include <cctype>

void
Msocket::error(const char *msg)
{
	perror(msg);
	exit(-1);
}

Msocket::Msocket(UWMdriver *pmDriver_, std::string portno_)
	: UWMconnector(pmDriver_, portno_)
{
	sockfd = 0;
	std::string tokenizer(":");
	size_t p_tokenizer = pathToDevice.find(tokenizer);

	if (p_tokenizer == std::string::npos) {

		server_host = "localhost";
		portno = atoi(pathToDevice.c_str());

	} else {

		server_host = pathToDevice.substr(0, p_tokenizer);
		portno = atoi((pathToDevice.substr(p_tokenizer + 1)).c_str());
	}
}

Msocket::~Msocket()
{
}

int
Msocket::openConnection()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("MSOCKET::ERROR::OPEN_SOCKET");
	}
	server = gethostbyname(server_host.c_str());
	if (server == NULL) {
		// fprintf(stderr,"ERROR, no such host\n");
		perror("MSOCKET::ERROR::NO_SUCH_HOST");
		exit(1);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
			(char *) &serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <
			0) {
		perror("MSOCKET::OPEN_CONNECTION::CONNECT_ERROR");
		exit(1);
	}
	rc = pthread_create(&thread_id, NULL, read_process_msocket, (void *) this);
	if (rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	return _MODEM_OK;
}

void
Msocket::closeConnection()
{
	// Close the connection with the server
	close(sockfd);
}

int
Msocket::writeToModem(std::string str)
{
	bzero(msg_tx, _MAX_MSG_LENGTH);
	memcpy(msg_tx, str.c_str(), str.size());
	int msg_ssz = str.size();

	{
		std::string s;
		s.assign(msg_tx, msg_ssz);
	}

	// Append to the message that we want to send the string terminator '\n'
	// (S2C
	// modem requirements)
	msg_tx[msg_ssz++] = '\n';

	// Send message host to modem via TCP-IP socket
	int w_bytes = write(sockfd, msg_tx, msg_ssz);

	// Return the number of written bytes
	return w_bytes;
}

void *
read_process_msocket(void *pMsocket_me_)
{
	// Array to store the received message
	char msg_rx[_MAX_MSG_LENGTH + 1];
	// Structure to queue the received message
	msgModem tmp_;
	std::ofstream out;

	Msocket *pMsocket_me = (Msocket *) pMsocket_me_;

	while (1) {
		// Read from the socket
		tmp_.msg_length =
				read(pMsocket_me->getSocket(), msg_rx, _MAX_MSG_LENGTH);

		if (tmp_.msg_length < 0) {
			perror("SOCKET::READ::ERROR_READ_FROM_SOCKET");
		}

		// Set end of string
		msg_rx[tmp_.msg_length] = '\0';

		// Check the queue length
		if (pMsocket_me->queueMsg.size() >
				pMsocket_me->getDriverQueueLength()) {
			std::cout << "MSOCKET::READ::ERROR::BUFFER_FULL ---> drop the oldest "
					"packet"
				 << std::endl;
			pMsocket_me->queueMsg.pop();
		}

		tmp_.msg_rx.assign(msg_rx, tmp_.msg_length);
		pMsocket_me->queueMsg.push(tmp_);

		usleep(1000);
	}
	pthread_exit(NULL);
}
