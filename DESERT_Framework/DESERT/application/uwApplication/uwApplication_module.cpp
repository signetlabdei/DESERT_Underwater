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
 * @file   uwApplication_module.cc
 * @author Federico Favaro
 * @version 1.1.0
 *
 * \brief Provides the implementation of uwApplicationModule class.
 *
 */

#include "uwApplication_module.h"
#include "uwudp-module.h"
#include <rng.h>

uint uwApplicationModule::MAX_READ_LEN = 64;

static class uwApplicationModuleClass : public TclClass
{
public:
	/**
	 * Constructor of uwApplicationModuleClass class
	 */
	uwApplicationModuleClass()
		: TclClass("Module/UW/APPLICATION")
	{
	}

	/**
	 *  Creates the TCL object needed for the TCL language interpretation
	 *
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new uwApplicationModule());
	}
} class_module_uwapplicationmodule;

uwApplicationModule::uwApplicationModule()
	: servSockDescr(0)
	, clnSockDescr(0)
	, servAddr()
	, clnAddr()
	, servPort(0)
	, queuePckReadTCP()
	, queuePckReadUDP()
	, out_log()
	, logging(false)
	, node_id(0)
	, exp_id(0)
	, debug_(0)
	, period(5)
	, poisson_traffic(0)
	, payloadsize(16)
	, port_num(55550)
	, drop_out_of_order(0)
	, dst_addr(0)
	, chkTimerPeriod(this)
	, socket_active(false)
	, socket_tcp(true)
	, sn_check()
	, uidcnt(0)
	, txsn(1)
	, rftt(0)
	, pkts_lost(0)
	, pkts_recv(0)
	, pkts_ooseq(0)
	, pkts_invalid(0)
	, pkts_last_reset(0)
	, lrtime(0)
	, sumrtt(0)
	, sumrtt2(0)
	, rttsamples(0)
	, sumftt(0)
	, sumftt2(0)
	, fttsamples(0)
	, esn(0)
	, sumbytes(0)
	, sumdt(0)
	, hrsn(0)
{
	bind("debug_", (int *) &debug_);
	bind("period_", (double *) &period);
	bind("node_ID_", (int *) &node_id);
	bind("EXP_ID_", (int *) &exp_id);
	bind("PoissonTraffic_", (int *) &poisson_traffic);
	bind("Payload_size_", (int *) &payloadsize);
	bind("destAddr_", (int *) &dst_addr);
	bind("destPort_", (int *) &port_num);
	bind("Socket_Port_", (int *) &servPort);
	bind("drop_out_of_order_", (int *) &drop_out_of_order);
	bind("max_read_length", (uint *) &uwApplicationModule::MAX_READ_LEN);

	if (period < 0) {
		if (debug_ >= 0)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "::UWAPPLICATION::ERROR_TIMER SET TO NEGATIVE VALUE"
					  << endl;
		period = 10;
	}

	sn_check = new bool[USHRT_MAX];
	for (int i = 0; i < USHRT_MAX; i++) {
		sn_check[i] = false;
	}
} // end uwApplicationModule() Method

int
uwApplicationModule::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			if (withoutSocket()) {
				if (debug_ >= 1)
					std::cout << "[" << getEpoch() << "]::" << NOW
							  << "::UWAPPLICATION::START_GENERATION_DATA"
							  << endl;
				if (logging)
					out_log << "[" << getEpoch() << "]::" << NOW
							<< "::UWAPPLICATION::START_GENERATION_DATA" << endl;

				chkTimerPeriod.resched(getTimeBeforeNextPkt());
			} else {
				if (useTCP()) {
					if (!listenTCP()) {
						std::cout << "error listen" << std::endl;

						return TCL_ERROR;
					}

					rx_thread =
							std::thread(&uwApplicationModule::acceptTCP, this);

				} else {
					if (!openConnectionUDP()) {
						std::cout << "error listen" << std::endl;

						return TCL_ERROR;
					}

					rx_thread = std::thread(
							&uwApplicationModule::readFromUDP, this);
				}

				chkTimerPeriod.resched(getPeriod());
			}
			return TCL_OK;
		} else if (strcasecmp(argv[1], "stop") == 0) {
			stop();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getsentpkts") == 0) {
			tcl.resultf("%d", getPktSent());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "lostpkts") == 0) {
			tcl.resultf("%f", getPktLost());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getrecvpkts") == 0) {
			tcl.resultf("%d", getPktRecv());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "outofsequencepkts") == 0) {
			tcl.resultf("%f", getPktsOOSequence());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "notknownpktrx") == 0) {
			tcl.resultf("%f", getPktsInvalidRx());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getrecvpktsqueue") == 0) {
			tcl.resultf("%d", getPktsPushQueue());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getrtt") == 0) {
			tcl.resultf("%f", GetRTT());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getrttstd") == 0) {
			tcl.resultf("%f", GetRTTstd());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getftt") == 0) {
			tcl.resultf("%f", GetFTT());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getfttstd") == 0) {
			tcl.resultf("%f", GetFTTstd());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getper") == 0) {
			tcl.resultf("%f", GetPER());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getthr") == 0) {
			tcl.resultf("%f", GetTHR());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "print_log") == 0) {
			std::stringstream stat_file;
			stat_file << "UWAPPLICATION_LOG_NODE_ID_" << node_id << "_EXP_ID_"
					  << exp_id << ".out";
			out_log.open(stat_file.str().c_str(), std::ios_base::app);
			out_log << left << "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::FILE_CREATED" << endl;
			logging = true;
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "SetSocketProtocol") == 0) {
			std::string protocol = argv[2];

			if (protocol == "TCP") {
				socket_tcp = true;
			} else if (protocol == "UDP") {
				socket_tcp = false;
			} else {
				tcl.result("Invalid socket protocol.");
				return TCL_ERROR;
			}
			socket_active = true;

			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
} // end command() Method

void
uwApplicationModule::recv(Packet *p)
{
	if (withoutSocket()) {
		if (debug_ >= 1)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "::UWAPPLICATION::RECV_PACKET_WITHOUT_SOCKET_MODE"
					  << endl;
		if (logging)
			out_log << left << "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::RECV_PACKET_WITHOUT_SOCKET_MODE"
					<< endl;
	} else {
		if (useTCP()) {
			if (debug_ >= 1)
				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::RECV_PACKET_USING_TCP" << endl;
			if (logging)
				out_log << left << "[" << getEpoch() << "]::" << NOW
						<< "::UWAPPLICATION::RECV_PACKET_USING_TCP" << endl;
		} else {
			if (debug_ >= 1)
				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::RECV_PACKET_USING_UDP" << endl;
			if (logging)
				out_log << left << "[" << getEpoch() << "]::" << NOW
						<< "::UWAPPLICATION::RECV_PACKET_USING_UDP" << endl;
		}
	}

	// TODO: split in separate methods
	statistics(p);
}; // end recv() Method

void
uwApplicationModule::statistics(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_DATA_APPLICATION *uwApph = HDR_DATA_APPLICATION(p);

	if (ch->ptype_ != PT_DATA_APPLICATION) {
		if (debug_ >= 0)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "::UWAPPLICATION::DROP_PACKET_NOT_APPLICATION_TYPE"
					  << endl;
		if (logging)
			out_log << left << "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::DROP_PACKET_NOT_APPLICATION_TYPE"
					<< endl;
		drop(p, 1, UWAPPLICATION_DROP_REASON_UNKNOWN_TYPE);
		incrPktInvalid(); // Increment the number of packet received invalid
		return;
	}
	esn = hrsn + 1; // Increase the expected sequence number

	// Verify if the data packet is already processed.
	if (useDropOutOfOrder()) {
		if (sn_check[uwApph->sn_ &
					0x00ffffff]) { // Packet already processed: drop it
			if (debug_ >= 0)
				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::DROP_PACKET_PACKET_ALREADY_"
							 "PROCESSED_ID_"
						  << (int) uwApph->sn_ << endl;
			if (logging)
				out_log << left << "[" << getEpoch() << "]::" << NOW
						<< "::UWAPPLICATION::DROP_PACKET_PACKET_ALREADY_"
						   "PROCESSED_ID_"
						<< (int) uwApph->sn_ << endl;
			incrPktInvalid();
			drop(p, 1, UWAPPLICATION_DROP_REASON_DUPLICATED_PACKET);
			return;
		}
	}
	// The data packet with this particular SN is not already processed
	// Set true sn_check. In this way we assume that these sn are already
	// processed by the node
	sn_check[uwApph->sn_ & 0x00ffffff] = true;

	// The data packet received is out of order
	if (useDropOutOfOrder()) {
		if (uwApph->sn_ <
				esn) { // packet is out of sequence and is to be discarded
			incrPktOoseq(); // Increase the number of data packets receive out
							// of
			// sequence.
			if (debug_ >= 0)
				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::DROP_PACKET_PACKET_OOS_ID_"
						  << (int) uwApph->sn_ << "_LAST_SN_" << hrsn << endl;
			if (logging)
				out_log << left << "[" << getEpoch() << "]::" << NOW
						<< "::UWAPPLICATION::DROP_PACKET_PACKET_OOS_ID_"
						<< (int) uwApph->sn_ << "_LAST_SN_" << hrsn << endl;
			drop(p, 1, UWAPPLICATION_DROP_REASON_OUT_OF_SEQUENCE);
			return;
		}
	}

	// Compute the Forward Trip time
	rftt = NOW - ch->timestamp();
	if ((uwApph->rftt_valid_) / 10000) {
		double rtt = rftt + uwApph->rftt();
		updateRTT(rtt);
	}

	updateFTT(rftt); // Update the forward trip time

	hrsn = uwApph->sn_; // Update the highest sequence number received

	// Verify if a packet is lost
	if (useDropOutOfOrder()) {
		if (uwApph->sn_ > esn) {
			if (debug_ >= 0)
				std::cout << "[" << getEpoch() << "]::" << NOW
						  << "::UWAPPLICATION::PACKET_LOST_ID_RECEIVED"
						  << (int) uwApph->sn_ << "_ID_EXPECTED_" << esn
						  << endl;
			if (logging)
				out_log << left << "[" << getEpoch() << "]::" << NOW
						<< "::UWAPPLICATION::PACKET_LOST_ID_RECEIVED"
						<< (int) uwApph->sn_ << "_ID_EXPECTED_" << esn << endl;
			incrPktLost(uwApph->sn_ - (esn));
		}
	}

	double dt = Scheduler::instance().clock() - lrtime;
	// updateThroughput(ch->size(), dt); //Update Throughput
	updateThroughput(uwApph->payload_size(), dt);
	incrPktRecv(); // Increase the number of data packets received

	lrtime = Scheduler::instance().clock(); // Update the time in which the last
											// packet is received.
	if (debug_ >= 0 && socket_active) {
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::PAYLOAD_RECEIVED--> ";
		for (int i = 0; i < uwApph->payload_size(); i++) {
			cout << uwApph->payload_msg[i];
		}
	}
	if (debug_ >= 0)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::SN_RECEIVED_" << (int) uwApph->sn_
				  << endl;
	if (debug_ >= 0)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::PAYLOAD_SIZE_RECEIVED_"
				  << (int) uwApph->payload_size() << endl;
	if (debug_ >= 1 && !withoutSocket())
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::PAYLOAD_RECEIVED_" << uwApph->payload_msg
				  << endl;

	if (logging)
		out_log << left << "[" << getEpoch() << "]::" << NOW
				<< "::UWAPPLICATION::PAYLOAD_SIZE_RECEIVED_"
				<< (int) uwApph->payload_size() << endl;
	if (logging)
		out_log << left << "[" << getEpoch() << "]::" << NOW
				<< "::UWAPPLICATION::SN_RECEIVED_" << (int) uwApph->sn_ << endl;

	// TODO: questo fa la stessa cosa di 28 righe sopra ma invece che cout
	// stampa in file
	if (logging && !withoutSocket()) {
		out_log << left << "::" << NOW
				<< "::UWAPPLICATION::PAYLOAD_RECEIVED--> ";
		for (int i = 0; i < uwApph->payload_size(); i++) {
			out_log << uwApph->payload_msg[i];
		}
		out_log << std::endl;
	}
	// TODO: check se socket_active?
	if (clnSockDescr) {
		write(clnSockDescr,
				uwApph->payload_msg,
				(size_t) uwApph->payload_size());
	}
	Packet::free(p);
} // end statistics method

void
uwApplicationModule::transmit()
{
	Packet *p;

	if (withoutSocket()) {
		p = Packet::alloc();
	} else {
		if (useTCP()) {
			if (queuePckReadTCP.empty()) {
				chkTimerPeriod.resched(getPeriod());
				return;
			}

			p = queuePckReadTCP.front();
			queuePckReadTCP.pop();
		} else {
			if (queuePckReadUDP.empty()) {
				chkTimerPeriod.resched(getPeriod());
				return;
			}

			p = queuePckReadUDP.front();
			queuePckReadUDP.pop();
		}
	}

	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);
	hdr_DATA_APPLICATION *uwApph = HDR_DATA_APPLICATION(p);

	ch->uid() = uidcnt++;
	ch->ptype() = PT_DATA_APPLICATION;
	ch->direction() = hdr_cmn::DOWN;
	ch->timestamp() = NOW;

	uwudp->dport() = port_num;
	uwiph->daddr() = dst_addr;

	if (withoutSocket()) {
		ch->size() = payloadsize;
		uwApph->payload_size() = payloadsize;

		if (payloadsize < MAX_LENGTH_PAYLOAD) {
			for (int i = 0; i < payloadsize; i++) {
				(*uwApph).payload_msg[i] = RNG::defaultrng()->uniform(26) + 'a';
			}
			for (int i = payloadsize; i < MAX_LENGTH_PAYLOAD; i++) {
				(*uwApph).payload_msg[i] = '0';
			}
		} else {
			for (int i = 0; i < MAX_LENGTH_PAYLOAD; i++) {
				(*uwApph).payload_msg[i] = RNG::defaultrng()->uniform(26) + 'a';
			}
		}
	}
	incrPktSent();
	uwApph->sn() = txsn;

	if (rftt >= 0) {
		uwApph->rftt() = (int) (rftt * 10000);
		uwApph->rftt_valid() = true;
	} else {
		uwApph->rftt_valid() = false;
	}

	uwApph->priority() = 0;

	if (debug_ >= 2)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::INIT_PACKET::UID_" << ch->uid() << endl;
	if (debug_ >= 0)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::INIT_PACKET::DEST_"
				  << (int) uwiph->daddr() << endl;
	if (debug_ >= 0)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::INIT_PACKET::SIZE_"
				  << (int) uwApph->payload_size() << endl;
	if (debug_ >= 0)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::INIT_PACKET::SN_" << (int) uwApph->sn()
				  << endl;
	if (debug_ >= 0)
		std::cout << "[" << getEpoch() << "]::" << NOW
				  << "::UWAPPLICATION::INIT_PACKET::SEND_DOWN_PACKET" << endl;

	if (logging)
		out_log << left << "[" << getEpoch() << "]::" << NOW
				<< "::UWAPPLICATION::INIT_PACKET::UID_" << ch->uid_ << endl;
	if (logging)
		out_log << left << "[" << getEpoch() << "]::" << NOW
				<< "::UWAPPLICATION::INIT_PACKET::DEST_" << (int) uwiph->daddr()
				<< endl;
	if (logging)
		out_log << left << "[" << getEpoch() << "]::" << NOW
				<< "::UWAPPLICATION::INIT_PACKET::SIZE_"
				<< (int) uwApph->payload_size() << endl;
	if (logging)
		out_log << left << "[" << getEpoch() << "]::" << NOW
				<< "::UWAPPLICATION::INIT_PACKET::SN_" << (int) uwApph->sn_
				<< endl;
	if (logging)
		out_log << left << "[" << getEpoch() << "]::" << NOW
				<< "::UWAPPLICATION::INIT_PACKET::SEND_DOWN_PACKET" << endl;

	sendDown(p);

	if (withoutSocket())
		chkTimerPeriod.resched(getTimeBeforeNextPkt());
	else
		chkTimerPeriod.resched(getPeriod());
}

void
uwApplicationModule::stop()
{
	if (withoutSocket()) {
		chkTimerPeriod.force_cancel();
	} else {
		if (useTCP()) {
			if (clnSockDescr >= 0) {
				shutdown(clnSockDescr, SHUT_RDWR);
				close(servSockDescr);
				clnSockDescr = -1;
			}

			if (servSockDescr >= 0) {
				shutdown(servSockDescr, SHUT_RDWR);
				close(servSockDescr);
				servSockDescr = -1;
			}
		}

		if (rx_thread.joinable()) {
			rx_thread.join();
		}

		chkTimerPeriod.force_cancel();
	}
} // end stop() method

double
uwApplicationModule::getTimeBeforeNextPkt()
{
	if (usePoissonTraffic()) {
		double u = RNG::defaultrng()->uniform_double();
		double lambda = 1.0 / period;

		if (debug_ >= 2)
			std::cout
					<< "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::PACKET_GENERATED_WITH_POISSON_period_"
					<< period << endl;
		if (logging)
			out_log << left << "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::PACKET_GENERATED_WITH_POISSON_period_"
					<< period << endl;

		return (-log(u) / lambda);
	} else {
		if (debug_ >= 2)
			std::cout << "[" << getEpoch() << "]::" << NOW
					  << "::UWAPPLICATION::PACKET_GENERATED_WITH_FIXED_period_"
					  << period << endl;
		if (logging)
			out_log << left << "[" << getEpoch() << "]::" << NOW
					<< "::UWAPPLICATION::PACKET_GENERATED_WITH_FIXED_period_"
					<< period << endl;

		return period;
	}
}

double
uwApplicationModule::GetRTT() const
{
	return (rttsamples > 0) ? sumrtt / rttsamples : 0;
}

double
uwApplicationModule::GetRTTstd() const
{
	if (rttsamples > 1) {
		double var =
				(sumrtt2 - (sumrtt * sumrtt / rttsamples)) / (rttsamples - 1);
		return (sqrt(var));
	} else
		return 0;
} // end GetRTTstd() method

void
uwApplicationModule::updateRTT(const double &rtt)
{
	sumrtt += rtt;
	sumrtt2 += rtt * rtt;
	rttsamples++;
} // end updateRTT() method

double
uwApplicationModule::GetFTT() const
{
	return (fttsamples > 0) ? sumftt / fttsamples : 0;
} // end getFTT() method

double
uwApplicationModule::GetFTTstd() const
{
	if (fttsamples > 1) {
		double var = 0;
		var = (sumftt2 - (sumftt * sumftt / fttsamples)) / (fttsamples - 1);
		if (var > 0)
			return (sqrt(var));
		else
			return 0;
	} else {
		return 0;
	}
} // end getFTT() method

double
uwApplicationModule::GetPER() const
{
	if (drop_out_of_order) {
		if ((pkts_recv + pkts_lost) > 0) {
			return ((double) pkts_lost / (double) (pkts_recv + pkts_lost));
		} else {
			return 0;
		}
	} else {
		if (esn > 1)
			return (1 - (double) pkts_recv / (double) (esn - 1));
		else
			return 0;
	}
} // end getPER() method

double
uwApplicationModule::GetTHR() const
{
	return ((sumdt != 0) ? sumbytes * 8 / sumdt : 0);
} // end GetTHR() method

void
uwApplicationModule::updateFTT(const double &ftt)
{
	sumftt += ftt;
	sumftt2 += ftt * ftt;
	fttsamples++;
} // end updateFTT() method

void
uwApplicationModule::updateThroughput(const int &bytes, const double &dt)
{
	sumbytes += bytes;
	sumdt += dt;
}

void
uwSendTimerAppl::expire(Event *e)
{
	module->transmit();
}
