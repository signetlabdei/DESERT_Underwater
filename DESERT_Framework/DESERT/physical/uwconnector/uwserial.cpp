//
// Copyright (c) 2019 Regents of the SIGNET lab, University of Padova.
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

#include <uwserial.h>

#include <cerrno>
#include <algorithm>
#include <iostream>

UwSerial::UwSerial()
	: UwConnector()
	, serialfd(-1)
{
	local_errno = 0;
}

UwSerial::~UwSerial()
{
}

const bool
UwSerial::isConnected()
{
	return (serialfd >= 0);
}

bool
UwSerial::openConnection(const std::string &path)
{
	std::string device_port;
	std::string sub_path;
	device_port = "/dev/";
	sub_path = path.substr(0, path.find(":"));
	device_port += sub_path;
	serialfd = open(device_port.c_str(), O_RDWR);

	if (serialfd < 0) {
		local_errno = errno;
		std::cout << "Port not open. Unable to open port " << device_port
				  << std::endl;
		std::cout << "Error " << local_errno
				  << " from open: " << strerror(local_errno)
				  << std::endl;
		return (false);

	} else {
		fcntl(serialfd, F_SETFL, 0);
		std::cout << "Port " << device_port << " is open.\n";
	}

	if (configurePort(path) != 0) {
		std::cout << "Error configuring port, check specific output."
				  << std::endl;
		return (false);
	}

	return (true);
}

bool
UwSerial::closeConnection()
{
	if (serialfd >= 0) {
		close(serialfd);
		serialfd = -1;
		return (true);
	}
	return (false);
}

int
UwSerial::writeToDevice(const std::string& msg)
{
	if (serialfd > 0) {
		int s_bytes = write(serialfd, msg.c_str(), msg.size());
		if (s_bytes >= static_cast<int>(msg.size())) {
			return (s_bytes);
		}
	}
	return 0;
}

int
UwSerial::readFromDevice(void *wpos, int maxlen)
{
	if (serialfd > 0) {
		int n_bytes = read(serialfd, wpos, maxlen);
		if (n_bytes >= 1) {
			return n_bytes;
		}

		local_errno = errno;
	}
	return -1;
}

int
UwSerial::configurePort(const std::string &path)
{
	std::string sub;
	int baud;
	int num_param = 0;
	std::size_t curs = 0, curs_d = 0;

	memset(&tty, 0, sizeof(tty));

	if (tcgetattr(serialfd, &tty) != 0) {
		local_errno = errno;
		std::cout << "Error " << local_errno
				  << "from tcgetattr: " << strerror(local_errno)
				  << std::endl;
	}

	while ((curs = path.find(":", curs)) != std::string::npos) {
		++num_param;
		curs++;
	}

	if (num_param < 4) {
		std::cout << "ERROR: too few number of parameters for serial port."
				  << std::endl;
		std::cout << "num_param: " << num_param << std::endl;
		return -1;
	} else if (num_param > 4) {
		std::cout << "ERROR: too many params for serial port. Check path or "
					 "add options in code."
				  << std::endl;
		std::cout << "num_param: " << num_param << std::endl;
		return -1;
	}

	curs_d = path.find(":");
	curs = path.find(":") + 1;

	while ((curs_d = path.find(":", curs_d)) != std::string::npos) {
		if (path.at(curs) == 'p') {
			if (path.at(curs + 2) == '0') {
				tty.c_cflag &= ~PARENB;
			} else if (path.at(curs + 2) == '1') {
				tty.c_cflag |= PARENB;
			}
		} else if (path.at(curs) == 's') {
			if (path.at(curs + 2) == '0') {
				tty.c_cflag &= ~CSTOPB;
			} else if (path.at(curs + 2) == '1') {
				tty.c_cflag |= CSTOPB;
			}
		} else if (path.at(curs) == 'f') {
			if (path.at(curs + 2) == '0') {
				tty.c_cflag &= ~CRTSCTS;
			} else if (path.at(curs + 2) == '1') {
				tty.c_cflag |= CRTSCTS;
			}
		} else if (path.at(curs) == 'b') {
			sub = path.substr(curs + 2, (path.find(":", curs) - (curs + 2)));
			baud = std::stoi(sub);

			if (baud == 115200) {
				if ((cfsetispeed(&tty, B115200)) != 0 &&
						(cfsetospeed(&tty, B115200) != 0)) {
					std::cout << "Baud(" << baud << ") is not a valid speed."
							  << std::endl;
				}
			} else if (baud == 1200) {
				if ((cfsetispeed(&tty, B1200)) != 0 &&
						(cfsetospeed(&tty, B1200) != 0)) {
					std::cout << "Baud(" << baud << ") is not a valid speed."
							  << std::endl;
				}
			} else if (baud == 2400) {
				if ((cfsetispeed(&tty, B2400)) != 0 &&
						(cfsetospeed(&tty, B2400) != 0)) {
					std::cout << "Baud(" << baud << ") is not a valid speed."
							  << std::endl;
				}
			} else if (baud == 4800) {
				if ((cfsetispeed(&tty, B4800)) != 0 &&
						(cfsetospeed(&tty, B4800) != 0)) {
					std::cout << "Baud(" << baud << ") is not a valid speed."
							  << std::endl;
				}
			} else if (baud == 19200) {
				if ((cfsetispeed(&tty, B19200)) != 0 &&
						(cfsetospeed(&tty, B19200) != 0)) {
					std::cout << "Baud(" << baud << ") is not a valid speed."
							  << std::endl;
				}
			} else if (baud == 38400) {
				if ((cfsetispeed(&tty, B38400)) != 0 &&
						(cfsetospeed(&tty, B38400) != 0)) {
					std::cout << "Baud(" << baud << ") is not a valid speed."
							  << std::endl;
				}
			} else if (baud == 57600) {
				if ((cfsetispeed(&tty, B57600)) != 0 &&
						(cfsetospeed(&tty, B57600) != 0)) {
					std::cout << "Baud(" << baud << ") is not a valid speed."
							  << std::endl;
				}
			} else {
				std::cout << "Baud rate [" << baud << "] not valid" << std::endl;
				return -1;
			}
		} else {
			std::cout << "ERROR: flag not defined." << std::endl;
		}
		curs = path.find(":", curs + 1) + 1;
		curs_d++;
	}

	tty.c_cflag |= CS8; // add option if needed in future implementation
						// e.g. CS5, CS6, CS7
	tty.c_cflag |= CREAD | CLOCAL;
	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;
	tty.c_lflag &= ~ECHOE;
	tty.c_lflag &= ~ECHONL;
	tty.c_lflag &= ~ISIG;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
	tty.c_oflag &= ~OPOST;
	tty.c_oflag &= ~ONLCR;

	tty.c_cc[VTIME] = 10;
	tty.c_cc[VMIN] = 0;

	if (tcsetattr(serialfd, TCSANOW, &tty) != 0) {
		std::cout << "ERROR: Port configuration error." << std::endl;
		return -1;
	}

	return 0;
}

bool
UwSerial::refreshConnection(const std::string &path)
{
	if (serialfd > 0) {
		closeConnection();
		openConnection(path);
		return (true);
	} else {
		std::cout << "Connection not established, cannot refresh." << std::endl;
	}
	return (false);
}
