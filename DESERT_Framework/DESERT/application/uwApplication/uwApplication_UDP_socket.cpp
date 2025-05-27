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
 * @file   uwApplication_UDP_socket.cc
 * @author Federico Favaro
 * @version 1.1.0
 *
 * \brief Provides the implementation of the UDP socket between the protocol and
 *any other application media.
 *
 */

#include "uwApplication_module.h"

bool
uwApplicationModule::openConnectionUDP()
{
	int sockoptval = 1;

	// Create socket for incoming connections
	if ((servSockDescr = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"openConnectionUDP()::Socket creation failed");

		return false;
	}

	if (setsockopt(servSockDescr,
				SOL_SOCKET,
				SO_REUSEADDR,
				&sockoptval,
				sizeof(int)) == -1) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"openConnectionUDP()::Set socket failed");
	}

	printOnLog(Logger::LogLevel::INFO,
			"UWAPPLICATION",
			"openConnectionUDP()::Socket created");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(servPort);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (::bind(servSockDescr, (struct sockaddr *) &servAddr, sizeof(servAddr)) <
			0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"openConnectionUDP()::Socket binding failed: " +
						std::string(strerror(errno)));

		return false;
	}

	return true;
}

void
uwApplicationModule::readFromUDP()
{
	char buffer_msg[MAX_LENGTH_PAYLOAD];
	int recvMsgSize;
	socklen_t clnLen = sizeof(sockaddr_in);

	while (receiving.load()) {
		clnLen = sizeof(clnAddr);
		for (int i = 0; i < MAX_LENGTH_PAYLOAD; i++) {
			buffer_msg[i] = 0;
		}

		if ((recvMsgSize = recvfrom(servSockDescr,
					 buffer_msg,
					 MAX_LENGTH_PAYLOAD,
					 0,
					 (struct sockaddr *) &(clnAddr),
					 &clnLen)) < 0) {

			printOnLog(Logger::LogLevel::ERROR,
					"UWAPPLICATION",
					"readFromUDP()::Receive from socket failed");

			continue;
		}

		if (recvMsgSize > 0) {
			std::unique_lock<std::mutex> lk(socket_mutex);
			Packet *p = Packet::alloc();
			hdr_cmn *ch = HDR_CMN(p);
			hdr_DATA_APPLICATION *hdr_Appl = HDR_DATA_APPLICATION(p);

			printOnLog(Logger::LogLevel::DEBUG,
					"UWAPPLICATION",
					"readFromUDP()::Socket payload received : " +
							std::string(buffer_msg));

			for (int i = 0; i < recvMsgSize; i++)
				hdr_Appl->payload_msg[i] = buffer_msg[i];
			ch->size() = recvMsgSize;
			hdr_Appl->payload_size() = recvMsgSize;

			queuePckReadUDP.push(p);
			incrPktsPushQueue();
		}
	}
}
