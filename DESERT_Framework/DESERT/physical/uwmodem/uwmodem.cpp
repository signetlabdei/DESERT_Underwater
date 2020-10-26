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

#include <chrono>
#include <iomanip>
#include <uwmodem.h>

bool
UwModem::string2log(const std::string &ll_string, LogLevel &ll)
{
	if (ll_string == "ERR") {
		ll = LogLevel::ERROR;
		return true;
	}
	if (ll_string == "INFO") {
		ll = LogLevel::INFO;
		return true;
	}
	if (ll_string == "DBG") {
		ll = LogLevel::DEBUG;
		return true;
	}
	return false;
}

bool
UwModem::log2string(LogLevel ll, std::string &ll_string)
{
	if (ll == LogLevel::ERROR) {
		ll_string = "ERR";
		return true;
	}
	if (ll == LogLevel::INFO) {
		ll_string = "INFO";
		return true;
	}
	if (ll == LogLevel::DEBUG) {
		ll_string = "DBG";
		return true;
	}
	return false;
}

UwModem::UwModem()
	: MPhy()
	, modemID(0)
	, data_buffer()
	, tx_queue()
	, rx_queue()
	, DATA_BUFFER_LEN(0)
	, MAX_READ_BYTES(0)
	, modem_address("")
	, debug_(0)
	, outLog()
	, logFile("modem_")
	, log_suffix("_log")
	, loglevel_(LogLevel::ERROR)
	, log_is_open(false)
	, checkTimer(NULL)
	, period(0.01)
	, event_q()
{
	bind("debug_", (int *) &debug_);
	bind("period_", (double *) &period);
	bind("buffer_size", (unsigned int *) &DATA_BUFFER_LEN);
	bind("max_read_size", (int *) &MAX_READ_BYTES);
	bind("ID_", (int *) &modemID);
}

UwModem::~UwModem()
{
	outLog.flush();
	outLog.close();
}

void
UwModem::printOnLog(LogLevel log_level, std::string module, std::string message)
{

	LogLevel actual_log_level = getLogLevel();
	if (actual_log_level >= log_level) {
		double timestamp =
				(double) (std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch())
								  .count()) /
				1000.0;
		if (!log_is_open) {
			outLog.open((getLogFile()).c_str(), ios::app);
			log_is_open = true;
		}
		std::string ll_descriptor = "";
		log2string(log_level, ll_descriptor);

		outLog << std::setprecision(15) << left << ll_descriptor << "::["
			   << timestamp << "]::[" << NOW << "]::" << module << "("
			   << modemID << ")::" << message << endl;
		outLog.flush();
	}
}

int
UwModem::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (!strcmp(argv[1], "start")) {
			start();
			return TCL_OK;
		}
		if (!strcmp(argv[1], "stop")) {
			stop();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (!strcmp(argv[1], "setModemAddress")) {
			modem_address = argv[2];
			return TCL_OK;
		}
		if (!strcmp(argv[1], "setLogSuffix")) {
			log_suffix = argv[2];
			return TCL_OK;
		}
		if (!strcmp(argv[1], "setLogLevel")) {
			std::string log_lv = argv[2];
			if (string2log(log_lv, loglevel_)) {
				return TCL_OK;
			}
			std::cout << "setLogLevel::INVALID_LOGLEVEL" << std::endl;
			return TCL_ERROR;
		}
	}

	return MPhy::command(argc, argv);
}

void
UwModem::endTx(Packet *p)
{
	Phy2MacEndTx(p);
	Packet::free(p);
}

void
UwModem::checkEvent()
{
	while (event_q.size() > 0) {
		ModemEvent e = event_q.front();
		e.f(*this, e.p);
		event_q.pop();
	}
	checkTimer->resched(period);
}

void
CheckTimer::expire(Event *e)
{
	pmModem->checkEvent();
}
