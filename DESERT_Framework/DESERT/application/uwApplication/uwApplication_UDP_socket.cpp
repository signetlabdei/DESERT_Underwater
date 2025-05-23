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
#include <iostream>

bool
uwApplicationModule::openConnectionUDP()
{
	int sockoptval = 1;

	// Create socket for incoming connections
	if ((servSockDescr = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		if (debug_ >= 0)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "::UWAPPLICATION::OPEN_CONNECTION_UDP::SOCKET_"
						 "CREATION_FAILED"
					  << endl;
		if (logging)
			out_log << left << "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::OPEN_CONNECTION_UDP::SOCKET_CREATION_"
					   "FAILED"
					<< endl;
		return false;
	}

	if (setsockopt(servSockDescr,
				SOL_SOCKET,
				SO_REUSEADDR,
				&sockoptval,
				sizeof(int)) == -1) {
		if (debug_ >= 0)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "UWAPPLICATION::ERROR::REUSABLE_FAIL" << std::endl;
	}

	if (debug_ >= 2)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::OPEN_CONNECTION_UDP::SOCKET_CREATED"
				  << endl;

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(servPort);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (::bind(servSockDescr, (struct sockaddr *) &servAddr, sizeof(servAddr)) <
			0) {
		if (debug_ >= 0)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "::UWAPPLICATION::OPEN_CONNECTION_UDP::BINDING_FAILED_"
					  << strerror(errno) << endl;
		if (logging)
			out_log << left << "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::OPEN_CONNECTION_UDP::BINDING_FAILED_"
					<< strerror(errno) << endl;

		return false;
	}

	return true;
}

void
uwApplicationModule::readFromUDP()
{
	// TODO: don't hardcode debug
	int debug_ = 1;
	char buffer_msg[MAX_LENGTH_PAYLOAD];
	int recvMsgSize;

	socklen_t clnLen = sizeof(sockaddr_in);

	while (true) {
		clnLen = sizeof(clnAddr);
		for (int i = 0; i < MAX_LENGTH_PAYLOAD; i++) {
			buffer_msg[i] = 0;
		}

		// Block until receive message from a client
		if ((recvMsgSize = recvfrom(servSockDescr,
					 buffer_msg,
					 MAX_LENGTH_PAYLOAD,
					 0,
					 (struct sockaddr *) &(clnAddr),
					 &clnLen)) < 0) {

			if (debug_ >= 0)
				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::READ_PROCESS_UDP::CONNECTION_"
							 "NOT_ACCEPTED"
						  << endl;
			if (logging)
				out_log << left << "[" << getEpoch() << "]::" << NOW
							 << "::UWAPPLICATION::READ_PROCESS_UDP::CONNECTION_"
								"NOT_ACCEPTED"
							 << endl;
		}

		if (debug_ >= 1)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "::UWAPPLICATION::READ_PROCESS_TCP::NEW_CLIENT_IP_"
					  << inet_ntoa(clnAddr.sin_addr) << std::endl;
		if (logging)
			out_log << left << "[" << getEpoch() << "]::" << NOW
						 << "::UWAPPLICATION::READ_PROCESS_UDP::NEW_CLIENT_IP_"
						 << inet_ntoa(clnAddr.sin_addr) << std::endl;

		std::unique_lock<std::mutex> lk(rx_mutex);

		if (recvMsgSize > 0) {
			Packet *p = Packet::alloc();
			hdr_cmn *ch = HDR_CMN(p);
			hdr_DATA_APPLICATION *hdr_Appl = HDR_DATA_APPLICATION(p);
			ch->size() = recvMsgSize;
			hdr_Appl->payload_size() = recvMsgSize;

			if (debug_ >= 0) {
				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::READ_PROCESS_UDP::NEW_PACKET_"
							 "CREATED--> "
						  << endl;

				if (logging)
					out_log << left << "[" << getEpoch()
								 << "]::" << NOW
								 << "::UWAPPLICATION::READ_PROCESS_UDP::NEW_"
									"PACKET_CREATED"
								 << endl;

				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::READ_PROCESS_UDP::PAYLOAD_"
							 "MESSAGE--> ";
				for (int i = 0; i < recvMsgSize; i++) {
					hdr_Appl->payload_msg[i] = buffer_msg[i];
					cout << buffer_msg[i];
				}
			}

			queuePckReadUDP.push(p);
			incrPktsPushQueue();
		}
		lk.unlock();
	}

}
