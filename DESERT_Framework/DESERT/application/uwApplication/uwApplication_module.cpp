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
#include <rng.h>
#include <uwip-module.h>
#include <uwudp-module.h>

#include <climits>
#include <iostream>
#include <string>

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
	: socket_active(false)
	, socket_tcp(true)
	, sn_check()
	, dst_addr(0)
	, poisson_traffic(0)
	, payloadsize(16)
	, port_num(55550)
	, drop_out_of_order(0)
	, uidcnt(0)
	, hrsn(0)
	, txsn(1)
	, rftt(0)
	, pkts_lost(0)
	, pkts_recv(0)
	, pkts_ooseq(0)
	, pkts_invalid(0)
	, pkts_last_reset(0)
	, sea_trial(0)
	, node_id(0)
	, servSockDescr(0)
	, clnSockDescr(0)
	, servPort(0)
	, esn(0)
	, rttsamples(0)
	, fttsamples(0)
	, period(10)
	, lrtime(0)
	, sumrtt(0)
	, sumrtt2(0)
	, sumftt(0)
	, sumftt2(0)
	, sumbytes(0)
	, sumdt(0)
	, servAddr()
	, clnAddr()
	, socket_thread()
	, receiving(false)
	, queuePckReadTCP()
	, queuePckReadUDP()
{
	bind("period_", (double *) &period);
	bind("Socket_Port_", (int *) &servPort);
	bind("Payload_size_", (int *) &payloadsize);
	bind("PoissonTraffic_", (int *) &poisson_traffic);
	bind("drop_out_of_order_", (int *) &drop_out_of_order);
	bind("node_ID_", (int *) &node_id);
	bind("max_read_length", (uint *) &uwApplicationModule::MAX_READ_LEN);
	bind("sea_trial_", (int *) &sea_trial);
	bind("destAddr_", (int *) &dst_addr);
	bind("destPort_", (int *) &port_num);

	if (period < 0) {
		std::cout << "UWAPPLICATION::uwApplicationModule()::Period < 0, "
					 "default to 10."
				  << endl;

		period = 10;
	}

	sn_check = new bool[USHRT_MAX];
	for (int i = 0; i < USHRT_MAX; i++) {
		sn_check[i] = false;
	}
}

uwApplicationModule::~uwApplicationModule()
{
	stop();
}

int
uwApplicationModule::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			if (!withoutSocket()) {
				receiving.store(true);

				if (useTCP()) {
					if (!listenTCP())
						return TCL_ERROR;

					socket_thread = std::thread(&uwApplicationModule::acceptTCP, this);

				} else {
					if (!openConnectionUDP())
						return TCL_ERROR;

					socket_thread = std::thread(&uwApplicationModule::readFromUDP, this);
				}
			}

			chkTimerPeriod = new uwSendTimerAppl(this);
			chkTimerPeriod->resched(getTimeBeforeNextPkt());

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
}

void
uwApplicationModule::recv(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_DATA_APPLICATION *uwApph = HDR_DATA_APPLICATION(p);

	if (ch->ptype_ != PT_DATA_APPLICATION) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWAPPLICATION",
				"recv(Packet *)::Drop packet, wrong type");

		drop(p, 1, UWAPPLICATION_DROP_REASON_UNKNOWN_TYPE);
		incrPktInvalid();

		return;
	}

	esn = hrsn + 1;

	if (useDropOutOfOrder()) {
		// Packet already processed: drop it
		if (sn_check[uwApph->sn_ & 0x00ffffff]) {
			printOnLog(Logger::LogLevel::ERROR,
					"UWAPPLICATION",
					"recv(Packet *)::Drop packet with sn " +
							to_string((int) uwApph->sn_) +
							", already processed");

			incrPktInvalid();
			drop(p, 1, UWAPPLICATION_DROP_REASON_DUPLICATED_PACKET);

			return;
		}
	}

	// The data packet with this particular SN is not already processed
	// Set true sn_check. In this way we assume that these sn are already
	// processed by the node
	sn_check[uwApph->sn_ & 0x00ffffff] = true;

	if (useDropOutOfOrder()) {
		// packet is out of sequence and is to be discarded
		if (uwApph->sn_ < esn) {
			incrPktOoseq();

			printOnLog(Logger::LogLevel::ERROR,
					"UWAPPLICATION",
					"recv(Packet *)::Drop packet with sn " +
							to_string((int) uwApph->sn_) + ", out of sequence");

			drop(p, 1, UWAPPLICATION_DROP_REASON_OUT_OF_SEQUENCE);

			return;
		}
	}

	rftt = NOW - ch->timestamp();
	if ((uwApph->rftt_valid_) / 10000) {
		double rtt = rftt + uwApph->rftt();
		updateRTT(rtt);
	}
	updateFTT(rftt);

	hrsn = uwApph->sn_;

	if (useDropOutOfOrder()) {
		// Verify if a packet is lost
		if (uwApph->sn_ > esn) {
			printOnLog(Logger::LogLevel::DEBUG,
					"UWAPPLICATION",
					"recv(Packet *)::Packet lost. Received has sn " +
							to_string((int) uwApph->sn_) +
							", expected has sn " + to_string(esn));

			incrPktLost(uwApph->sn_ - (esn));
		}
	}

	double dt = NOW - lrtime;
	updateThroughput(uwApph->payload_size(), dt);
	incrPktRecv();
	lrtime = NOW;

	if (!withoutSocket())
		printOnLog(Logger::LogLevel::DEBUG,
				"UWAPPLICATION",
				"recv(Packet *)::Payload received : " +
						std::string(uwApph->payload_msg));

	printOnLog(Logger::LogLevel::DEBUG,
			"UWAPPLICATION",
			"recv(Packet *)::Payload size : " +
					to_string((int) uwApph->payload_size()));
	printOnLog(Logger::LogLevel::DEBUG,
			"UWAPPLICATION",
			"recv(Packet *)::Sequence number : " +
					to_string((int) uwApph->sn()));

	if (!withoutSocket() && clnSockDescr)
		write(clnSockDescr,
				uwApph->payload_msg,
				(size_t) uwApph->payload_size());

	Packet::free(p);
};

void
uwApplicationModule::transmit()
{
	Packet *p;

	if (withoutSocket()) {
		p = Packet::alloc();
	} else {
		if (useTCP()) {
			if (queuePckReadTCP.empty()) {
				chkTimerPeriod->resched(getTimeBeforeNextPkt());
				return;
			}

			p = queuePckReadTCP.front();
			queuePckReadTCP.pop();
		} else {
			if (queuePckReadUDP.empty()) {
				chkTimerPeriod->resched(getTimeBeforeNextPkt());
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

	printOnLog(Logger::LogLevel::DEBUG,
			"UWAPPLICATION",
			"transmit()::Unique id : " + to_string(ch->uid()));
	printOnLog(Logger::LogLevel::DEBUG,
			"UWAPPLICATION",
			"transmit()::Dest addr : " + to_string((int) uwiph->daddr()));
	printOnLog(Logger::LogLevel::DEBUG,
			"UWAPPLICATION",
			"transmit()::Payload size : " +
					to_string((int) uwApph->payload_size()));
	printOnLog(Logger::LogLevel::DEBUG,
			"UWAPPLICATION",
			"transmit()::Sequence number : " + to_string((int) uwApph->sn()));
	printOnLog(Logger::LogLevel::DEBUG,
			"UWAPPLICATION",
			"transmit()::Send down packet");

	sendDown(p);
	chkTimerPeriod->resched(getTimeBeforeNextPkt());
}

void
uwApplicationModule::stop()
{
	if (!withoutSocket()) {
		receiving.store(false);

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

		if (socket_thread.joinable()) {
			socket_thread.join();
		}
	}

	chkTimerPeriod->force_cancel();
}

double
uwApplicationModule::getTimeBeforeNextPkt()
{
	if (withoutSocket() && usePoissonTraffic()) {
		double u = RNG::defaultrng()->uniform_double();
		double lambda = 1.0 / period;

		return (-log(u) / lambda);
	}

	return period;
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
	}

	return 0;
}

void
uwApplicationModule::updateRTT(double rtt)
{
	sumrtt += rtt;
	sumrtt2 += rtt * rtt;
	rttsamples++;
}

double
uwApplicationModule::GetFTT() const
{
	return (fttsamples > 0) ? sumftt / fttsamples : 0;
}

double
uwApplicationModule::GetFTTstd() const
{
	if (fttsamples > 1) {
		double var =
				(sumftt2 - (sumftt * sumftt / fttsamples)) / (fttsamples - 1);

		return (var > 0) ? sqrt(var) : 0;
	}

	return 0;
}

double
uwApplicationModule::GetPER() const
{
	if (drop_out_of_order) {
		if ((pkts_recv + pkts_lost) > 0)
			return ((double) pkts_lost / (double) (pkts_recv + pkts_lost));
	} else {
		if (esn > 0)
			return (1 - (double) pkts_recv / (double) esn);
	}
	return 0;
}

double
uwApplicationModule::GetTHR() const
{
	return ((sumdt != 0) ? sumbytes * 8 / sumdt : 0);
}

void
uwApplicationModule::updateFTT(double ftt)
{
	sumftt += ftt;
	sumftt2 += ftt * ftt;
	fttsamples++;
}

void
uwApplicationModule::updateThroughput(int bytes, double dt)
{
	sumbytes += bytes;
	sumdt += dt;
}

void
uwSendTimerAppl::expire(Event *e)
{
	module->transmit();
}
