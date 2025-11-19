//
// Copyright (c) 2025 Regents of the SIGNET lab, University of Padova.
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
 * @file   UwCsBurst.cpp
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UwCsBurst</i>.
 *
 */

#include "uw-cs-burst.h"
#include <mac.h>
#include <string>

void
UwSensingTimer::expire(Event *e)
{
	((UwCsBurst *) module_)->sensingExpired();
}

/**
 * Class that represent the binding of the protocol with tcl
 */
static class CSBurstModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the CSBurstModule class
	 */
	CSBurstModuleClass()
		: TclClass("Module/UW/CSBURST")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwCsBurst());
	}

} class_UwCsBurst;

UwCsBurst::UwCsBurst()
	: MMac()
	, tx_status_(UWCS_STATUS::IDLE)
	, fix_sens_time_(1)
	, rv_sens_time_(1)
	, sensing_timer_(this)
	, buffer_()
	, max_queue_size_()
	, max_packet_per_burst_(10)
	, packet_sent_curr_burst_(10)
	, n_rx_while_sensing_(0)
{
	bind("queue_size", (int *) &max_queue_size_);
	bind("max_packet_per_burst", (int *) &max_packet_per_burst_);
	bind("fix_sens_time", (double *) &fix_sens_time_);
	bind("rv_sens_time", (double *) &rv_sens_time_);

	if (max_packet_per_burst_ < 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWCSB",
				"not valid max_packet_per_burst < 0!! set to 1 by default");
		max_packet_per_burst_ = 1;
	}
	if (fix_sens_time_ < 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWCSB",
				"not valid fix_sens_time < 0!! set to 1 by default");
		fix_sens_time_ = 1;
	}
	if (rv_sens_time_ < 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWCSB",
				"not valid rv_sens_time < 0!! set to 1 by default");
		rv_sens_time_ = 1;
	}
}

void
UwCsBurst::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	if (buffer_.size() < max_queue_size_) {
		initPkt(p);
		buffer_.push_back(p);
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"UWCSB",
				"recvFromUpperLayers()::dropping pkt due to buffer full");

		Packet::free(p);
	}

	printOnLog(Logger::LogLevel::DEBUG,
			"UWCSB",
			"recvFromUpperLayers()::start sensing");

	sensing();
}

void
UwCsBurst::sensing()
{
	if (tx_status_ != UWCS_STATUS::IDLE) { // already sensing or transmitting
		printOnLog(Logger::LogLevel::DEBUG,
				"UWCSB",
				"sensing()::already sensing or transmitting");

		return;
	}

	n_rx_while_sensing_ = 0;
	tx_status_ = UWCS_STATUS::SENSING;

	double sensing_time = fix_sens_time_ +
			RNG::defaultrng()->uniform_double() * rv_sens_time_;
	sensing_timer_.sched(sensing_time);

	printOnLog(Logger::LogLevel::DEBUG,
			"UWCSB",
			"sensing()::sensing time = " + to_string(sensing_time) +
					". Change status to SENSING.");
}

void
UwCsBurst::sensingExpired()
{

	packet_sent_curr_burst_ = 0;
	if (n_rx_while_sensing_ > 0) {
		n_rx_while_sensing_ = 0;

		double sensing_time = fix_sens_time_ +
				RNG::defaultrng()->uniform_double() * rv_sens_time_;
		sensing_timer_.resched(sensing_time);

		printOnLog(Logger::LogLevel::DEBUG,
				"UWCSB",
				"sensingExpired()::received packet in the meanwhile, new "
				"sensing time = " + to_string(sensing_time));
		return;
	}

	printOnLog(Logger::LogLevel::DEBUG,
			"UWCSB",
			"sensingExpired()::change status to TRANSMITTING");

	tx_status_ = UWCS_STATUS::TRANSMITTING;
	txData();
}

void
UwCsBurst::txData()
{
	if (buffer_.size() <= 0) {
		tx_status_ = UWCS_STATUS::IDLE;

		printOnLog(Logger::LogLevel::DEBUG,
				"UWCSB",
				"txData()::no data to transmit. Change status to IDLE");

		return;
	}

	if (packet_sent_curr_burst_ < max_packet_per_burst_) {
		Packet *p = buffer_.front();
		buffer_.pop_front();

		Mac2PhyStartTx(p);

		incrDataPktsTx();
	} else {
		printOnLog(Logger::LogLevel::DEBUG,
				"UWCSB",
				"txData()::already sent max packet = " +
						to_string(max_packet_per_burst_) +
						". Change status to IDLE.");

		tx_status_ = UWCS_STATUS::IDLE;
		sensing();
	}
}

void
UwCsBurst::Mac2PhyStartTx(Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"UWCSB",
			"Mac2PhyStartTx()::Start transmitting");

	MMac::Mac2PhyStartTx(p);
}

void
UwCsBurst::Phy2MacEndTx(const Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"UWCSB",
			"Mac2PhyEndTx()::End transmitting");

	packet_sent_curr_burst_++;

	txData();
}

void
UwCsBurst::Phy2MacStartRx(const Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"UWCSB",
			"Mac2PhyStartRx()::Start receiving");

	n_rx_while_sensing_++;
}

void
UwCsBurst::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();
	int src_mac = mach->macSA();

	if (ch->error()) {
		printOnLog(Logger::LogLevel::DEBUG,
				"UWCSB",
				"Phy2MacEndRx()::dropping corrupted packet from node " +
						to_string(src_mac));

		incrErrorPktsRx();

		Packet::free(p);
	} else {
		if (dest_mac != addr && dest_mac != MAC_BROADCAST) {
			rxPacketNotForMe(p);

			printOnLog(Logger::LogLevel::DEBUG,
					"UWCSB",
					"Phy2MacEndRx()::dropping packet, it was for node " +
							to_string(dest_mac));
		} else {
			sendUp(p);
			incrDataPktsRx();

			printOnLog(Logger::LogLevel::DEBUG,
					"UWCSB",
					"Phy2MacEndRx()::Received packet from node " +
							to_string(src_mac));
		}
	}
}

void
UwCsBurst::initPkt(Packet *p)
{
	hdr_mac *mach = HDR_MAC(p);
	mach->macSA() = addr;
}

void
UwCsBurst::rxPacketNotForMe(Packet *p)
{
	if (p != NULL)
		Packet::free(p);
}

int
UwCsBurst::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "get_buffer_size") == 0) {
			tcl.resultf("%d", buffer_.size());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_upper_data_pkts_rx") == 0) {
			tcl.resultf("%d", up_data_pkts_rx);

			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_sent_pkts") == 0) {
			tcl.resultf("%d", data_pkts_tx);

			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_recv_pkts") == 0) {
			tcl.resultf("%d", data_pkts_rx);

			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setMacAddr") == 0) {
			addr = atoi(argv[2]);

			printOnLog(Logger::LogLevel::INFO,
					"UWCSB",
					"command()::MAC address of current node is " +
							to_string(addr));

			return TCL_OK;
		}
	}

	return MMac::command(argc, argv);
}
