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

#include <uwsocket.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <cerrno>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <unistd.h>

UwSocket::UwSocket()
	: UwConnector()
	, socketfd(-1)
	, proto(Transport::TCP)
	, isClient(true)
{
	local_errno = 0;
}

UwSocket::~UwSocket()
{
}

const bool
UwSocket::isConnected()
{
	return (socketfd >= 0);
}

bool
UwSocket::openConnection(const std::string &path)
{

	std::string sep(":");
	std::string address;
	int port;
	int sockfd;
	socklen_t len_addr;
	struct sockaddr_in s_address;
	int sockoptval = 1;
	struct sockaddr_in cl_address;
	
	// find the position of the colon
	size_t pos = path.find(sep);

	if (isClient) {
		if (pos == std::string::npos) {
			address = "127.0.0.1";
			port = std::stoi(path);
		} else {
			address = path.substr(0, pos);
			port = std::stoi(path.substr(pos + 1));
		}
	} else {
		port = std::stoi(path);
	}

	if (proto == Transport::TCP) {

		if(isClient) {
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, 
								sizeof(int)) == -1) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			std::memset(&s_address, 0, sizeof(s_address));
			s_address.sin_family = AF_INET;
			s_address.sin_port = htons(port);
	
			if (inet_pton(AF_INET, address.c_str(), &s_address.sin_addr) <= 0) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}
	
			if (connect(sockfd, (struct sockaddr *) &s_address, sizeof(s_address)) <
					0) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}
	
			socketfd = sockfd;
	
			return (true);
		
		} else {				//server TCP
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, 
								sizeof(int)) == -1) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			std::memset(&s_address, 0, sizeof(s_address));
			s_address.sin_family = AF_INET;
			s_address.sin_addr.s_addr = htonl(INADDR_ANY);
			s_address.sin_port = htons(port);
			// s_address.sin_port = htons((u_short) port);


			if (bind(sockfd, (struct sockaddr *) &s_address,
								sizeof(s_address)) == -1) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			if (listen(sockfd, 1) < 0) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			// while(1) {
				len_addr = sizeof(cl_address);
				socketfd = accept(sockfd, (struct sockaddr *) &cl_address,
								&len_addr);
				if (socketfd < 0) {
					local_errno = errno;
					std::cerr << "UWSOCKET::ERROR::" +
					    std::to_string(local_errno) << std::endl;
				}

				close(sockfd);
			// }
		}

	} else { // proto == Transport::UDP


		if (isClient) {

			if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, 
									sizeof(int)) == -1) {
					local_errno = errno;
					std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
							  << std::endl;
					return (false);
			}

			struct sockaddr_in dest_addr;

			// only port provided
			std::memset(&dest_addr, 0, sizeof(dest_addr));
			dest_addr.sin_family = AF_INET;
			dest_addr.sin_port = htons(port);
			dest_addr.sin_addr.s_addr = inet_addr(address.c_str());

			if (sockfd > 0) {
				int s_bytes = sendto(sockfd, &udp_init_string, 
							sizeof(udp_init_string), 0,
							(const struct sockaddr *) &dest_addr,
							sizeof(dest_addr));
			}
			
			cl_addr = dest_addr;
			socketfd = sockfd;

			return (true);

		} else {

			if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				local_errno = errno;
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}

			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, 
									sizeof(int)) == -1) {
					local_errno = errno;
					std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
							  << std::endl;
					return (false);
			}

			struct sockaddr_in my_addr;

			// only port provided
			std::memset(&my_addr, 0, sizeof(my_addr));
			my_addr.sin_family = AF_INET;
			my_addr.sin_port = htons(port);
			my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) <
					0) {
				std::cerr << "UWSOCKET::ERROR::" + std::to_string(local_errno)
						  << std::endl;
				return (false);
			}


			socklen_t addrlen = sizeof(cl_addr);
			char tmp_listen[] = {0};
			
			int n_bytes = recvfrom(sockfd, &tmp_listen, sizeof(tmp_listen), 0,
										(struct sockaddr *)&cl_addr, &addrlen);
			if(n_bytes > 0)
				std::cout << "Server connected to client." << std::endl;

			socketfd = sockfd;

			return (true);

		}

	} // Transport::UDP

	return (true);
}

bool
UwSocket::closeConnection()
{
	if (socketfd >= 0) {
		shutdown(socketfd, SHUT_RDWR);
		close(socketfd);
		socketfd = -1;
		return (true);
	} else {
		return (false);
	}
}

int
UwSocket::writeToDevice(const std::string& msg)
{
	if (proto == Transport::TCP) {

		if (socketfd > 0) {
			int s_bytes =
					send(socketfd, msg.c_str(), static_cast<int>(msg.length()), 0);
			if (s_bytes >= static_cast<int>(msg.length())) {
				return (s_bytes);
			}
		}
		return 0;

	} else {		//UDP protocol

		socklen_t claddr_len = sizeof(cl_addr);

		if (socketfd > 0) {
			int s_bytes = sendto(socketfd, msg.c_str(), 
								 static_cast<int>(msg.length()), 0,
								 (const struct sockaddr *) &cl_addr, claddr_len);

			if (s_bytes >= static_cast<int>(msg.length())) {
				return (s_bytes);
			}
		}
		return 0;
	}
}

int
UwSocket::readFromDevice(void *wpos, int maxlen)
{
	if (proto == Transport::TCP) {

		if (socketfd == -1) {
			return -1;
		}

		int n_bytes = read(socketfd, wpos, maxlen);
		return n_bytes;

	} else {		//UDP protocol

		if (socketfd == -1)
			return -1;

		socklen_t addrlen = sizeof(cl_addr);

		int n_bytes = recvfrom(socketfd, wpos, maxlen, 0,
		    (struct sockaddr *)&cl_addr, &addrlen);
		return n_bytes;
	}

	return -1;

}
