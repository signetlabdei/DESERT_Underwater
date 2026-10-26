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

#include <algorithm>
#include <arpa/inet.h>
#include <cstddef>
#include <exception>
#include <linux/limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ostream>
#include <stdexcept>
#include <string>
#include <uwsocket.h>

#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>
#include <iostream>
#include <unistd.h>

UwSocket::UwSocket()
	: UwConnector()
	, socketfd(-1)
	, proto(Transport::TCP)
	, isClient(true)
	, isMulticast(false)
	, multicast_address("224.1.1.2")
{
	local_errno = 0;
}

UwSocket::~UwSocket()
{
	close(socketfd);
}

const bool
UwSocket::isConnected()
{
	return (socketfd >= 0);
}

bool
UwSocket::openConnection(const std::string &path)
{
	std::string address;
	int port;
	int sockfd;
	socklen_t len_addr;
	struct sockaddr_in s_address;
	int sockoptval = 1;
	struct sockaddr_in cl_address;
	std::string host_section;
	std::string port_section;

	if (!resolveStringAddress(path, host_section, port_section)) {
		return false;
	}

	address = host_section;
	port = std::stoi(port_section);

	if (proto == Transport::TCP) {

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			local_errno = errno;
			std::cerr << "[ERROR]::UWSOCKET::openConnection()::TCP socket() "
						 "failed. Error code: " +
							std::to_string(local_errno)
					  << std::endl;
			return false;
		}

		if (setsockopt(sockfd,
					SOL_SOCKET,
					SO_REUSEADDR,
					&sockoptval,
					sizeof(int)) == -1) {
			local_errno = errno;
			std::cerr << "[ERROR]::UWSOCKET::openConnection()::TCP "
						 "setsockopt(...,SO_REUSEADDR,...) "
						 "failed. Error code: " +
							std::to_string(local_errno)
					  << std::endl;
			return false;
		}

		if (isClient) { // TCP client

			std::memset(&s_address, 0, sizeof(s_address));
			s_address.sin_family = AF_INET;
			s_address.sin_port = htons(port);

			if (inet_pton(AF_INET, address.c_str(), &s_address.sin_addr) <= 0) {
				local_errno = errno;
				std::cerr << "[ERROR]::UWSOCKET::openConnection()::TCP "
							 "inet_pton() failed. "
							 "Error code: " +
								std::to_string(local_errno)
						  << std::endl;
				return false;
			}

			if (connect(sockfd,
						(struct sockaddr *) &s_address,
						sizeof(s_address)) < 0) {
				local_errno = errno;
				std::cerr << "[ERROR]::UWSOCKET::openConnection()::TCP "
							 "connect() failed "
							 "Error code: " +
								std::to_string(local_errno)
						  << std::endl;
				return false;
			}

			socketfd = sockfd;

			return true;

		} else { // server TCP

			std::memset(&s_address, 0, sizeof(s_address));
			s_address.sin_family = AF_INET;
			s_address.sin_addr.s_addr = htonl(INADDR_ANY);
			s_address.sin_port = htons(port);
			// s_address.sin_port = htons((u_short) port);

			if (bind(sockfd,
						(struct sockaddr *) &s_address,
						sizeof(s_address)) == -1) {
				local_errno = errno;
				std::cerr << "[ERROR]::UWSOCKET::openConnection()::TCP bind() "
							 "failed. Error code: " +
								std::to_string(local_errno)
						  << std::endl;
				return false;
			}

			if (listen(sockfd, 1) < 0) {
				local_errno = errno;
				std::cerr << "[ERROR]::UWSOCKET::openConnection()::TCP "
							 "listen() failed. Error "
							 "code: " +
								std::to_string(local_errno)
						  << std::endl;
				return false;
			}

			len_addr = sizeof(cl_address);
			socketfd =
					accept(sockfd, (struct sockaddr *) &cl_address, &len_addr);
			if (socketfd < 0) {
				local_errno = errno;
				std::cerr << "[ERROR]::UWSOCKET::openConnection()::TCP "
							 "accept() failed. Error "
							 "code: " +
								std::to_string(local_errno)
						  << std::endl;
			}

			close(sockfd);
		}

	} else { // proto == Transport::UDP

		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			local_errno = errno;
			std::cerr << "[ERROR]::UWSOCKET::openConnection()::UDP socket() "
						 "failed. Error code: " +
							std::to_string(local_errno)
					  << std::endl;
			return false;
		}

		if (setsockopt(sockfd,
					SOL_SOCKET,
					SO_REUSEADDR,
					&sockoptval,
					sizeof(int)) == -1) {
			local_errno = errno;
			std::cerr << "[ERROR]::UWSOCKET::openConnection()::UDP "
						 "setsockopt(..., SO_REUSEADDR, "
						 "...) failed. Error code: " +
							std::to_string(local_errno)
					  << std::endl;
			return false;
		}

		if (isClient) { // sender UDP
			// local interface address
			struct sockaddr_in dest_addr;
			std::memset(&dest_addr, 0, sizeof(dest_addr));
			dest_addr.sin_family = AF_INET;
			dest_addr.sin_port = htons(port);
			dest_addr.sin_addr.s_addr = inet_addr(address.c_str());

			// adding multicast configuration to dest_addr
			if (isMulticast) {

				struct in_addr local_interface;
				local_interface.s_addr = inet_addr(address.c_str());

				// setting the interface the socker should send multicast
				// message to
				if (setsockopt(sockfd,
							IPPROTO_IP,
							IP_MULTICAST_IF,
							(char *) &local_interface,
							sizeof(local_interface)) < 0) {
					local_errno = errno;
					char ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &local_interface, ip, sizeof(ip));
					std::cerr << "[ERROR]::UWSOCKET::openConnection()::UDP "
								 "connection failed to "
								 "enable multicast. Error code: "
							  << std::strerror(local_errno)
							  << ", local_interface: " << ip << std::endl;
					return false;
				}

				unsigned char ttl = 1;
				if (setsockopt(sockfd,
							IPPROTO_IP,
							IP_MULTICAST_TTL,
							(char *) &ttl,
							sizeof(ttl)) < 0) {
					local_errno = errno;
					std::cerr << "[ERROR]::UWSOCKET::openConnection()::UDP "
								 "failed to set TTL. "
								 "Error code: "
							  << std::to_string(local_errno) << std::endl;
					return false;
				}

				// multicast group, same variable to have simple and multicast
				// udp on same block
				memset(&dest_addr, 0, sizeof(dest_addr));
				dest_addr.sin_family = AF_INET;
				dest_addr.sin_port = htons(port);
				inet_pton(AF_INET,
						multicast_address.c_str(),
						&dest_addr.sin_addr);
			}

			cl_addr = dest_addr;
			socketfd = sockfd;

			std::cout << "[DEBUG]::UWSOCKET::openConnection()::"
					  << "Sender connection opened." << std::endl;

			return true;

		} else { // receiver UDP

			struct sockaddr_in my_addr;

			std::memset(&my_addr, 0, sizeof(my_addr));
			my_addr.sin_family = AF_INET;
			my_addr.sin_port = htons(port);
			my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) <
					0) {
				local_errno = errno;
				std::cerr << "[ERROR]::UWSOCKET::openConnection()::UDP bind() "
							 "failed. Error code: " +
								std::to_string(local_errno)
						  << ", local_interface: " << address << ":" << port
						  << std::endl;
				return false;
			}

			struct ip_mreq group;
			if (isMulticast) {
				std::memset(&group, 0, sizeof(group));

				inet_pton(AF_INET,
						multicast_address.c_str(),
						&group.imr_multiaddr);
				inet_pton(AF_INET, address.c_str(), &group.imr_interface);

				if (setsockopt(sockfd,
							IPPROTO_IP,
							IP_ADD_MEMBERSHIP,
							(char *) &group,
							sizeof(group)) < 0) {
					local_errno = errno;
					std::cerr << "[ERROR]::UWSOCKET::openConnection()::UDP "
								 "setsockopt(..,IP_ADD_MEMBERSHIP,..) failed. "
								 "Error code: "
							  << std::to_string(errno) << std::endl;
				}
			}

			socklen_t addrlen = sizeof(cl_addr);
			char tmp_listen[] = {0};
			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &cl_addr, ip, sizeof(ip));

			socketfd = sockfd;

			std::cout << "[DEBUG]::UWSOCKET::openConnection()::"
					  << "Receiver connection opened." << std::endl;

			return true;
		}
	}

	return true;
}

bool
UwSocket::closeConnection()
{
	if (socketfd >= 0) {
		shutdown(socketfd, SHUT_RDWR);
		close(socketfd);
		socketfd = -1;
		return true;
	} else {
		return false;
	}
}

int
UwSocket::writeToDevice(const std::string &msg)
{
	if (proto == Transport::TCP) {

		if (socketfd > 0) {
			int s_bytes = send(
					socketfd, msg.c_str(), static_cast<int>(msg.length()), 0);
			if (s_bytes >= static_cast<int>(msg.length())) {
				return (s_bytes);
			}
		}
		return 0;

	} else { // UDP protocol
		socklen_t claddr_len = sizeof(cl_addr);
		if (socketfd > 0) {
			int s_bytes = sendto(socketfd,
					msg.c_str(),
					static_cast<int>(msg.length()),
					0,
					(const struct sockaddr *) &cl_addr,
					claddr_len);

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

	} else { // UDP protocol

		if (socketfd == -1)
			return -1;

		socklen_t addrlen = sizeof(cl_addr);

		int n_bytes = recvfrom(socketfd,
				wpos,
				maxlen,
				0,
				(struct sockaddr *) &cl_addr,
				&addrlen);
		return n_bytes;
	}

	return -1;
}

bool
UwSocket::resolveStringAddress(
		const std::string &address, std::string &host, std::string &port)
{
	const std::string LOCAL_ADDRESS = "127.0.0.1"; // localhost
	const std::string DEFAULT_PORT = "50420";
	std::string sep = ":";
	size_t sep_pos = address.find(sep);

	std::string host_section = (sep_pos != std::string::npos)
			? address.substr(0, sep_pos)
			: address;
	std::string port_section =
			(sep_pos != std::string::npos) ? address.substr(sep_pos + 1) : "";

	try {
		// host_section validation
		if (host_section.find(".") == std::string::npos &&
				host_section == "localhost") { // localhost
			host = LOCAL_ADDRESS;
		} else if (host_section.find(".") == std::string::npos ||
				std::count(host_section.begin(), host_section.end(), '.') !=
						3) {
			std::cerr
					<< "[ERROR]::UWSOCKET::resolveStringAddress()::Invalid address: "
					<< address << std::endl;
			return false;
		} else {
			host = host_section;
		}

		if (std::stoi(port) < 1 || std::stoi(port) > 65535)
			throw std::out_of_range("");

		// port section validation
		if (port_section.empty()) {
			port = DEFAULT_PORT;
		} else {
			port = port_section;
		}

		sockaddr_in sock_address;
		if (inet_pton(AF_INET, host.c_str(), &(sock_address.sin_addr)) != 1) {
			std::cerr
					<< "[ERROR]::UWSOCKET::resolveStringAddress()::Invalid address: "
					<< address << std::endl;
		}

	} catch (const std::invalid_argument &e) {
		std::cerr << "[ERROR]::UWSOCKET::resolveStringAddress()::Port must be a "
					 "number. "
					 "Invalid address: "
				  << address << std::endl;
		return false;
	} catch (const std::out_of_range &e) {
		std::cerr << "[ERROR]::UWSOCKET::resolveStringAddress()::Invalid port "
					 "number. Invalid address: "
				  << address << std::endl;
		return false;
	} catch (const std::exception &e) {
		std::cerr << "[ERROR]::USOCKET::resolveStringAddress()::Error parsing "
					 "address: '"
				  << address << "'. " << e.what() << std::endl;
	}

	return true;
}
