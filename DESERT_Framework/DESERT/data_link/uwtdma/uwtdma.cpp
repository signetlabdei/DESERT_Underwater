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
 * @file   uwtdma.cpp
 * @author Filippo Campagnaro
 * @author Roberto Francescon
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UWTDMA</i>.
 *
 */

#include "uwtdma.h"
#include <iostream>
#include <mac.h>
#include <stdint.h>
#include <string>
#include <uwcbr-module.h>
#include <uwmmac-clmsg.h>

/**
 * Class that represent the binding of the protocol with tcl
 */
static class TDMAModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	TDMAModuleClass()
		: TclClass("Module/UW/TDMA")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwTDMA());
	}

} class_uwtdma;

void
UwTDMATimer::expire(Event *e)
{
	((UwTDMA *) module)->changeStatus();
}

UwTDMA::UwTDMA()
	: MMac()
	, transceiver_status(IDLE)
	, slot_status(SlotStatus::NOT_MY_SLOT)
	, sea_trial_(0)
	, fair_mode(0)
	, tot_slots(0)
	, slot_number(0)
	, HDR_size(0)
	, frame_duration(0)
	, guard_time(0)
	, slot_duration(0)
	, start_time(0)
	, tdma_timer(this)
	, out_file_stats()
	, enable(true)
	, max_queue_size(10)
	, max_packet_per_slot(1)
	, packet_sent_curr_slot_(0)
	, drop_old_(0)
	, name_label_("")
	, checkPriority(0)
{
	bind("queue_size_", (int *) &max_queue_size);
	bind("frame_duration", (double *) &frame_duration);
	bind("sea_trial_", (int *) &sea_trial_);
	bind("fair_mode", (int *) &fair_mode);
	bind("HDR_size_", (int *) &HDR_size);
	bind("max_packet_per_slot", (int *) &max_packet_per_slot);
	bind("drop_old_", (int *) &drop_old_);
	bind("checkPriority_", (int *) &checkPriority);
	bind("mac2phy_delay_", (double *) &mac2phy_delay_);
	if (fair_mode == 1) {
		bind("guard_time", (double *) &guard_time);
		bind("tot_slots", (int *) &tot_slots);
	}

	if (max_queue_size < 0) {
		std::cout << "UWTDMA::"
				  << "UwTDMA()::invalid max_queue_size < 0. Set to 1 by "
					 "default.\n";
		max_queue_size = 1;
	}

	if (frame_duration < 0) {
		std::cout << "UWTDMA::"
				  << "UwTDMA()::invalid frame_duration < 0. Set to 1 by "
					 "default.\n";
		frame_duration = 1;
	}
	if (max_packet_per_slot < 0) {
		std::cout << "UWTDMA::"
				  << "UwTDMA()::invalid max_packet_per_slot < 0. Set to 1 by "
					 "default.\n";
		max_packet_per_slot = 1;
	}
	if (drop_old_ == 1 && checkPriority == 1) {
		std::cout << "UWTDMA::"
				  << "UwTDMA()::drop_old_ and checkPriority cannot be set both "
					 "to 1"
					 "checkPriority set to 0 by default.\n";
		checkPriority = 0;
	}
	if (mac2phy_delay_ <= 0) {
		std::cout << "UWTDMA::"
				  << "UwTDMA()::invalid mac2phy_delay_ <= 0. Set to 1e-9 by "
					 "default.\n";
		mac2phy_delay_ = 1e-9;
	}
}

void
UwTDMA::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	if (buffer.size() < max_queue_size) {
		initPkt(p);
		if (checkPriority == 0) {
			buffer.push_back(p);
		} else {
			hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
			if (uwcbrh->priority() == 0) {
				buffer.push_back(p);

				printOnLog(Logger::LogLevel::INFO,
						"UWTDMA",
						"recvFromUpperLayers(Packet*)::inserted packet with "
						"standard priority in the queue.");
			} else {
				buffer.push_front(p);

				printOnLog(Logger::LogLevel::INFO,
						"UWTDMA",
						"recvFromUpperLayers(Packet*)::inserted packet with "
						"high priority in the queue.");
			}
		}
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTDMA",
				"recvFromUpperLayers(Packet*)::dropping packet due to buffer "
				"full");

		if (drop_old_) {
			Packet *p_old = buffer.front();
			buffer.pop_front();
			Packet::free(p_old);
			initPkt(p);
			buffer.push_back(p);
		} else if (checkPriority == 1) {
			hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
			if (uwcbrh->priority() == 1) {
				Packet *p_old = buffer.back();
				buffer.pop_back();
				Packet::free(p_old);
				initPkt(p);
				buffer.push_front(p);
			}
		} else
			Packet::free(p);
	}
	txData();
}

void
UwTDMA::stateTxData()
{
	txData();
}

void
UwTDMA::txData()
{
	if (packet_sent_curr_slot_ < max_packet_per_slot) {
		if (slot_status == SlotStatus::MY_SLOT && transceiver_status == IDLE) {
			if (buffer.size() > 0) {
				Packet *p = buffer.front();
				buffer.pop_front();
				Mac2PhyStartTx(p);
				incrDataPktsTx();
			}
		} else {
			if (slot_status != SlotStatus::MY_SLOT)
				printOnLog(Logger::LogLevel::INFO,
						"UWTDMA",
						"txData()::Addr " + std::to_string(addr) +
								"::Wait my slot to send");
			else
				printOnLog(Logger::LogLevel::INFO,
						"UWTDMA",
						"txData()::Addr " + std::to_string(addr) +
								"::Wait earlier packet expires to send the "
								"current one");
		}
	} else {
		if (sea_trial_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::TDMA_node(" << addr
						   << ")::already_tx max packet = "
						   << max_packet_per_slot << std::endl;

		printOnLog(Logger::LogLevel::INFO,
				"UWTDMA",
				"txData()::Addr " + std::to_string(addr) +
						"::already tx max packet = " +
						std::to_string(max_packet_per_slot));
	}
}

void
UwTDMA::Mac2PhyStartTx(Packet *p)
{
	if (!sea_trial_)
		assert(transceiver_status == IDLE);

	transceiver_status = TRANSMITTING;
	if (sea_trial_) {
		sendDown(p, 0.01);
	} else {
		MMac::Mac2PhyStartTx(p);
	}

	printOnLog(Logger::LogLevel::INFO,
			"UWTDMA",
			"Mac2PhyStartTx(Packet*)::Addr " + std::to_string(addr) +
					"::sending packet");

	if (sea_trial_)
		out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					   << "::TDMA_node(" << addr
					   << ")::PCK_SENT packet_sent_curr_slot_ = "
					   << packet_sent_curr_slot_
					   << " max_packet_per_slot = " << max_packet_per_slot
					   << std::endl;
}

void
UwTDMA::Phy2MacEndTx(const Packet *p)
{
	transceiver_status = IDLE;
	packet_sent_curr_slot_++;
	if (sea_trial_)
		out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					   << "::TDMA_node(" << addr << ")::Phy2MacEndTx(p)"
					   << std::endl;

	txData();
}

void
UwTDMA::Phy2MacStartRx(const Packet *p)
{
	if (sea_trial_ != 1)
		assert(transceiver_status != RECEIVING);

	if (transceiver_status == IDLE)
		transceiver_status = RECEIVING;
}

void
UwTDMA::Phy2MacEndRx(Packet *p)
{
	if (transceiver_status != TRANSMITTING) {
		hdr_cmn *ch = HDR_CMN(p);
		hdr_mac *mach = HDR_MAC(p);
		int dest_mac = mach->macDA();
		int src_mac = mach->macSA();

		if (ch->error()) {
			printOnLog(Logger::LogLevel::ERROR,
					"UWTDMA",
					"Phy2MacEndRx(Packet*)::Addr " + std::to_string(addr) +
							"::dropping corrupted packet from node with addr " +
							std::to_string(src_mac));

			incrErrorPktsRx();
			Packet::free(p);
		} else {
			if (dest_mac != addr && dest_mac != MAC_BROADCAST) {
				rxPacketNotForMe(p);

				printOnLog(Logger::LogLevel::ERROR,
						"UWTDMA",
						"Phy2MacEndRx(Packet*)::Addr " + std::to_string(addr) +
								"::dropping packet was for node with addr " +
								std::to_string(dest_mac));
			} else {
				sendUp(p);
				incrDataPktsRx();

				printOnLog(Logger::LogLevel::DEBUG,
						"UWTDMA",
						"Phy2MacEndRx(Packet*)::Addr " + std::to_string(addr) +
								"::received packet from " +
								std::to_string(src_mac));

				if (sea_trial_)
					out_file_stats << left << "[" << getEpoch() << "]::" << NOW
								   << "::TDMA_node(" << addr
								   << ")::PCK_FROM:" << src_mac << std::endl;
			}
		}

		transceiver_status = IDLE;

		if (slot_status == SlotStatus::MY_SLOT)
			txData();

	} else {
		printOnLog(Logger::LogLevel::DEBUG,
				"UWTDMA",
				"Phy2MacEndRx(Packet*)::Addr " + std::to_string(addr) +
						"::received packet while transmitting");

		if (sea_trial_) {
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::TDMA_node(" << addr << ")::RCVD_PCK_WHILE_TX"
						   << std::endl;
			sendUp(p);
			incrDataPktsRx();
		} else {
			Packet::free(p);
		}
	}
}

void
UwTDMA::initPkt(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	int curr_size = ch->size();

	ch->size() = curr_size + HDR_size;
}

void
UwTDMA::rxPacketNotForMe(Packet *p)
{
	if (p != NULL)
		Packet::free(p);
}

void
UwTDMA::changeStatus()
{
	packet_sent_curr_slot_ = 0;
	if (slot_status == SlotStatus::MY_SLOT) {
		slot_status = SlotStatus::NOT_MY_SLOT;
		double off_time = frame_duration - slot_duration + guard_time;
		tdma_timer.resched(off_time);

		printOnLog(Logger::LogLevel::DEBUG,
				"UWTDMA",
				"changeStatus(Packet*)::Addr " + std::to_string(addr) +
						"::Not my slot for the next " +
						std::to_string(off_time) + " seconds");

		if (sea_trial_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::TDMA_node(" << addr << ")::Off" << std::endl;
	} else {
		slot_status = SlotStatus::MY_SLOT;
		double on_time = slot_duration - guard_time;
		tdma_timer.resched(on_time);

		printOnLog(Logger::LogLevel::DEBUG,
				"UWTDMA",
				"changeStatus(Packet*)::Addr " + std::to_string(addr) +
						"::My slot for the next " + std::to_string(on_time) +
						" seconds");

		if (sea_trial_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::TDMA_node(" << addr << ")::On" << std::endl;

		stateTxData();
	}
}

void
UwTDMA::start(double delay)
{

	enable = true;
	if (sea_trial_) {
		std::stringstream stat_file;
		stat_file << "./TDMA_node_" << addr << "_" << name_label_ << ".out";
		std::cout << stat_file.str().c_str() << std::endl;
		out_file_stats.open(stat_file.str().c_str(), std::ios_base::app);

		out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					   << "::TDMA_node(" << addr << ")::Start_simulation"
					   << std::endl;
	}

	tdma_timer.sched(delay);

	printOnLog(Logger::LogLevel::DEBUG,
			"UWTDMA",
			"start(double)::Addr " + std::to_string(addr) +
					"::current status " +
					std::to_string(static_cast<int>(slot_status)));
}

void
UwTDMA::stop()
{
	enable = false;
	tdma_timer.cancel();

	if (sea_trial_)
		out_file_stats << left << "[" << getEpoch() << "]::" << NOW
					   << "::TDMA_node(" << addr << ")::TDMA_stopped_"
					   << std::endl;
}

int
UwTDMA::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			if (guard_time < 0) {
				tcl.resultf("guard_time must be non-negative");
				return TCL_ERROR;
			}

			if (fair_mode == 1) {
				if (tot_slots <= 0 || slot_number < 0 ||
						slot_number >= tot_slots) {
					tcl.resultf(
							"tot_slots must be positive and slot_number must "
							"be in [0, tot_slots)");
					return TCL_ERROR;
				}
			}

			const double duration =
					fair_mode == 1 ? frame_duration / tot_slots : slot_duration;
			const double delay =
					fair_mode == 1 ? slot_number * duration : start_time;
			if (duration <= guard_time || duration > frame_duration ||
					delay < 0) {
				tcl.resultf(
						"slot duration must exceed guard_time and not exceed "
						"frame_duration");
				return TCL_ERROR;
			}

			slot_duration = duration;
			start_time = delay;

			start(start_time);

			return TCL_OK;
		} else if (strcasecmp(argv[1], "stop") == 0) {
			stop();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "get_buffer_size") == 0) {
			tcl.resultf("%zu", buffer.size());
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
		if (strcasecmp(argv[1], "setStartTime") == 0) {
			double value;
			if (Tcl_GetDouble(tcl.interp(), argv[2], &value) != TCL_OK)
				return TCL_ERROR;

			if (value < 0) {
				tcl.resultf("%s requires a non-negative value", argv[1]);
				return TCL_ERROR;
			}

			start_time = value;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setSlotDuration") == 0) {
			if (fair_mode == 1) {
				tcl.resultf("Cannot set slot duration in fair mode");

				return TCL_ERROR;
			} else {
				double value;
				if (Tcl_GetDouble(tcl.interp(), argv[2], &value) != TCL_OK)
					return TCL_ERROR;

				if (value <= 0) {
					tcl.resultf("%s requires a positive value", argv[1]);
					return TCL_ERROR;
				}

				slot_duration = value;
				return TCL_OK;
			}
		} else if (strcasecmp(argv[1], "setGuardTime") == 0) {
			if (fair_mode == 1) {
				tcl.resultf("Cannot set guard time in fair mode");

				return TCL_ERROR;
			} else {
				double value;
				if (Tcl_GetDouble(tcl.interp(), argv[2], &value) != TCL_OK)
					return TCL_ERROR;

				if (value < 0) {
					tcl.resultf(
							"%s requires a finite nonnegative value", argv[1]);
					return TCL_ERROR;
				}

				guard_time = value;
				return TCL_OK;
			}
		} else if (strcasecmp(argv[1], "setSlotNumber") == 0) {
			int value;
			if (Tcl_GetInt(tcl.interp(), argv[2], &value) != TCL_OK)
				return TCL_ERROR;

			if (value < 0) {
				tcl.resultf("%s requires a nonnegative integer", argv[1]);
				return TCL_ERROR;
			}

			slot_number = value;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setMacAddr") == 0) {
			int value;
			if (Tcl_GetInt(tcl.interp(), argv[2], &value) != TCL_OK)
				return TCL_ERROR;

			if (value < 0) {
				tcl.resultf("%s requires a nonnegative integer", argv[1]);
				return TCL_ERROR;
			}

			addr = value;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setLogLabel") == 0) {
			name_label_ = argv[2];
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}

int
UwTDMA::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_UWMMAC_ENABLE) {
		ClMsgUwMmacEnable *command = dynamic_cast<ClMsgUwMmacEnable *>(m);
		if (command->getReqType() == ClMsgUwMmac::SET_REQ) {
			(command->getEnable()) ? start(NOW + start_time) : stop();
			command->setReqType(ClMsgUwMmac::SET_REPLY);
			return 0;
		}
	}
	return MMac::recvSyncClMsg(m);
}
