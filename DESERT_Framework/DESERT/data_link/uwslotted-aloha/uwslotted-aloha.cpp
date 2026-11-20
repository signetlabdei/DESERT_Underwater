//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwslotted-aloha.cpp
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UwSlottedAloha</i>.
 *
 */

#include "uwslotted-aloha.h"
#include "uwslotted-aloha-cmn-hdr.h"
#include <iostream>
#include <mac.h>
#include <rng.h>

void
UwSASlotTimer::expire(Event *e)
{
	((UwSlottedAloha *) module_)->txData();
}

void
UwSAAckTimer::expire(Event *e)
{
	((UwSlottedAloha *) module_)->retxData();
}

/**
 * Class that represent the binding of the protocol with tcl
 */
static class SlottedAlohaModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the SlottedAlohaModule class
	 */
	SlottedAlohaModuleClass()
		: TclClass("Module/UW/SLOTTEDALOHA")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwSlottedAloha());
	}

} class_UwSlottedAloha;

UwSlottedAloha::UwSlottedAloha()
	: MMac()
	, enable_ack(true)
	, retx_(false)
	, max_queue_size_()
	, slot_time_()
	, retry_prob_()
	, status_(UWSA_STATUS::IDLE)
	, slot_timer_(this)
	, ack_timer_(this)
	, backoff_timer_(this)
	, buffer_()
{
	bind("slot_time", (double *) &slot_time_);
	bind("max_queue_size", (uint *) &max_queue_size_);

	if (slot_time_ < 0) {
		std::cout << "UWSA : not valid slot_time < 0!! set to 10 by default"
				  << std::endl;
		slot_time_ = 10;
	}
}

void
UwSlottedAloha::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	if (buffer_.size() < max_queue_size_) {
		initPkt(p);
		buffer_.push_back(p);
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"UWSA",
				"recvFromUpperLayers()::dropping pkt due to buffer full");

		Packet::free(p);
	}
}

void
UwSlottedAloha::txData()
{
	if (buffer_.empty()) {
		if (status_ != UWSA_STATUS::IDLE) {
			status_ = UWSA_STATUS::IDLE;

			printOnLog(Logger::LogLevel::DEBUG,
					"UWSA",
					"txData()::no data to transmit. Change status to IDLE");
		}
	} else if (status_ == UWSA_STATUS::IDLE) {

		status_ = UWSA_STATUS::TRANSMITTING;

		if (!retx_) {
			last_data_pkt = buffer_.front();
			buffer_.pop_front();
		}

		Mac2PhyStartTx(last_data_pkt);

		incrDataPktsTx();
	}

	slot_timer_.resched(slot_time_);
}

void
UwSlottedAloha::retxData()
{
	if (status_ != UWSA_STATUS::WAITING) {
		printOnLog(Logger::LogLevel::DEBUG,
				"UWSA",
				"retxData()::error");

		return;
	}

	// after 2*t_slot try to retx in the next slot with retry_prob.
	// If not wait another slot and try to transmit in the slot after
	if (retx_count_ < max_retx_) {
		if (RNG::defaultrng()->uniform_double() < retry_prob_) {
			ack_timer_.force_cancel();
			retx_ = true;
			retx_count_++;
		} else {
			ack_timer_.resched(slot_time_);
			return;
		}
	} else {
		ack_timer_.force_cancel();
		retx_count_ = 0;
		retx_ = false;
		Packet::free(last_data_pkt);
	}

	status_ = UWSA_STATUS::IDLE;
}

void
UwSlottedAloha::Mac2PhyStartTx(Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"UWSA",
			"Mac2PhyStartTx()::Start transmitting");

	MMac::Mac2PhyStartTx(p);
}

void
UwSlottedAloha::Phy2MacEndTx(const Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"UWSA",
			"Mac2PhyEndTx()::End transmitting");

	if (enable_ack && (hdr_cmn::access(p))->ptype() != PT_SA_ACK) {
		status_ = UWSA_STATUS::WAITING;
		retx_ = false;
		ack_timer_.resched(2 * slot_time_);
	} else
		status_ = UWSA_STATUS::IDLE;
}

void
UwSlottedAloha::Phy2MacStartRx(const Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"UWSA",
			"Mac2PhyStartRx()::Start receiving");

	if (status_ == UWSA_STATUS::IDLE)
		status_ = UWSA_STATUS::RECEIVING;
}

void
UwSlottedAloha::Phy2MacEndRx(Packet *p)
{
	if (status_ == UWSA_STATUS::TRANSMITTING) {
		printOnLog(Logger::LogLevel::DEBUG,
				"UWSA",
				"Phy2MacEndRx(Packet*)::Addr " + std::to_string(addr) +
						"::received packet while transmitting");

		Packet::free(p);
		return;
	}

	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();
	int src_mac = mach->macSA();

	if (ch->error()) {
		printOnLog(Logger::LogLevel::DEBUG,
				"UWSA",
				"Phy2MacEndRx()::dropping corrupted packet from node " +
						to_string(src_mac));

		incrErrorPktsRx();

		Packet::free(p);
	} else {
		if (dest_mac != addr && dest_mac != MAC_BROADCAST) {
			rxPacketNotForMe(p);

			printOnLog(Logger::LogLevel::DEBUG,
					"UWSA",
					"Phy2MacEndRx()::dropping packet, it was for node " +
							to_string(dest_mac));
		} else {
			if (ch->ptype() == PT_SA_ACK) {
				ack_timer_.force_cancel();

				Packet::free(p);

				printOnLog(Logger::LogLevel::DEBUG,
						"UWSA",
						"Phy2MacEndRx()::Received ACK from node " +
								to_string(src_mac));
			} else {
				sendUp(p);
				incrDataPktsRx();

				printOnLog(Logger::LogLevel::DEBUG,
						"UWSA",
						"Phy2MacEndRx()::Received packet from node " +
								to_string(src_mac));

				// Send back ack
				if (enable_ack) {
					status_ = UWSA_STATUS::TRANSMITTING;
					initAckPkt(src_mac);

					Mac2PhyStartTx(p);

					// TODO: incr ack packets sent

					return;
				}
			}
		}
	}

	status_ = UWSA_STATUS::IDLE;
}

void
UwSlottedAloha::initPkt(Packet *p)
{
	hdr_mac *mach = HDR_MAC(p);
	mach->macSA() = addr;
}

void
UwSlottedAloha::initAckPkt(int dst_addr)
{
	Packet *p_ack = Packet::alloc();

	hdr_cmn *ch = hdr_cmn::access(p_ack);
	hdr_mac *mach = HDR_MAC(p_ack);
	hdr_SAAck *saach = HDR_SA_ACK(p_ack);

	ch->ptype() = PT_SA_ACK;
	mach->macSA() = addr;
	saach->dst_addr() = dst_addr;
}

void
UwSlottedAloha::rxPacketNotForMe(Packet *p)
{
	if (p != NULL)
		Packet::free(p);
}

void
UwSlottedAloha::start()
{
	printOnLog(Logger::LogLevel::INFO,
			"UWSA",
			"start()::Addr " + std::to_string(addr));

	slot_timer_.sched(slot_time_);
}

void
UwSlottedAloha::stop()
{
	printOnLog(Logger::LogLevel::INFO,
			"UWSA",
			"stop()::Addr " + std::to_string(addr));

	slot_timer_.cancel();
}

int
UwSlottedAloha::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			start();

			return TCL_OK;
		} else if (strcasecmp(argv[1], "stop") == 0) {
			stop();

			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_buffer_size") == 0) {
			tcl.resultf("%zu", buffer_.size());

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
			int value;
			if (Tcl_GetInt(tcl.interp(), argv[2], &value) != TCL_OK)
				return TCL_ERROR;

			if (value < 0) {
				tcl.resultf("%s requires a nonnegative integer", argv[1]);
				return TCL_ERROR;
			}

			addr = value;

			return TCL_OK;
		}
	}

	return MMac::command(argc, argv);
}
