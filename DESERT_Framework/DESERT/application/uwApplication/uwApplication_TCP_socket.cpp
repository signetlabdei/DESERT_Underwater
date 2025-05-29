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
//

/**
 * @file   uwApplication_TCP_socket.cc
 * @author Federico Favaro
 * @version 1.1.0
 *
 * \brief Provides the implementation of the TCP socket between the protocol and
 *any other application media.
 *
 */

#include "uwApplication_module.h"

bool
uwApplicationModule::listenTCP()
{
	int sockoptval = 1;

	if ((servSockDescr = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"listenTCP()::Socket creation failed");

		return false;
	}

	if (setsockopt(servSockDescr,
				SOL_SOCKET,
				SO_REUSEADDR,
				&sockoptval,
				sizeof(int)) == -1) {

		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"listenTCP()::Set socket failed");

		return false;
	}

	printOnLog(Logger::LogLevel::INFO,
			"UWAPPLICATION",
			"listenTCP()::Socket created");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(servPort);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (::bind(servSockDescr, (struct sockaddr *) &servAddr, sizeof(servAddr)) <
			0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"listenTCP()::Socket binding failed: " +
						std::string(strerror(errno)));

		return false;
	}

	if (listen(servSockDescr, 1)) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"listenTCP()::Socket listen failed");

		return false;
	}

	printOnLog(Logger::LogLevel::INFO,
			"UWAPPLICATION",
			"listenTCP()::Socket listening");

	return true;
}

void
uwApplicationModule::acceptTCP()
{
	socklen_t clnLen = sizeof(sockaddr_in);

	clnLen = sizeof(clnAddr);

	while (receiving.load()) {
		if ((clnSockDescr = accept(servSockDescr,
					 (struct sockaddr *) &(clnAddr),
					 (socklen_t *) &clnLen)) < 0) {
			printOnLog(Logger::LogLevel::ERROR,
					"UWAPPLICATION",
					"acceptTCP()::Socket connection not accepted");

			continue;
		}

		printOnLog(Logger::LogLevel::INFO,
				"UWAPPLICATION",
				"acceptTCP()::Socket accept connection from " +
						std::string(inet_ntoa(clnAddr.sin_addr)));

		readFromTCP(clnSockDescr);
	}
}

void
uwApplicationModule::readFromTCP(int clnSock)
{
	while (true) {
		int recvMsgSize = 0;
		char buffer_msg[MAX_LENGTH_PAYLOAD];

		for (int i = 0; i < MAX_LENGTH_PAYLOAD; i++) {
			buffer_msg[i] = 0;
		}

		if ((recvMsgSize = read(clnSock, buffer_msg, MAX_READ_LEN)) < 0) {
			printOnLog(Logger::LogLevel::ERROR,
					"UWAPPLICATION",
					"readFromTCP(int)::Read from socket failed");

			break;
		}

		if (recvMsgSize == 0) {
			printOnLog(Logger::LogLevel::INFO,
					"UWAPPLICATION",
					"readFromTCP(int)::Socket disconnected");

			shutdown(clnSock, SHUT_RDWR);
			break;
		}

		std::unique_lock<std::mutex> lk(socket_mutex);

		Packet *p = Packet::alloc();
		hdr_cmn *ch = HDR_CMN(p);
		hdr_DATA_APPLICATION *hdr_Appl = HDR_DATA_APPLICATION(p);

		printOnLog(Logger::LogLevel::DEBUG,
				"UWAPPLICATION",
				"readFromTCP(int)::Socket payload received : " +
						std::string(buffer_msg, MAX_LENGTH_PAYLOAD));

		for (int i = 0; i < recvMsgSize; i++)
			hdr_Appl->payload_msg[i] = buffer_msg[i];
		ch->size() = recvMsgSize;
		hdr_Appl->payload_size() = recvMsgSize;

		queuePckReadTCP.push(p);
		incrPktsPushQueue();

		lk.unlock();
	}
}
