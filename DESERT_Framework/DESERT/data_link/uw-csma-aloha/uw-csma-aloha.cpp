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
 * @file   uw-csma-aloha.cpp
 * @author Federico Guerra, Saiful Azad and Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the implementation of CsmaAloha Class
 *
 */

#include "uw-csma-aloha.h"
#include <climits>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <mac.h>
#include <rng.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

enum { NOT_SET = -1, SESSION_DISTANCE_NOT_SET = 0 };
/**
 * Class that represents the binding with the tcl configuration script
 */
static class CSMAModuleClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	CSMAModuleClass()
		: TclClass("Module/UW/CSMA_ALOHA")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new CsmaAloha());
	}
} class_module_csma;

void
CsmaAloha::AckTimer::expire(Event *e)
{
	timer_status = CSMA_EXPIRED;
	if (module->curr_state == CSMA_STATE_WAIT_ACK) {
		module->printOnLog(Logger::LogLevel::DEBUG,
				"CSMA_ALOHA",
				"AckTimer::expire(Event *)::current state = " +
						module->status_info[module->curr_state] +
						"; ACK not received, next state = " +
						module->status_info[CSMA_STATE_BACKOFF]);

		module->refreshReason(CSMA_REASON_ACK_TIMEOUT);
		module->stateBackoff();
	} else {
		module->printOnLog(Logger::LogLevel::DEBUG,
				"CSMA_ALOHA",
				"AckTimer::expire(Event *)");
	}
}

void
CsmaAloha::BackOffTimer::expire(Event *e)
{
	timer_status = CSMA_EXPIRED;
	if (module->curr_state == CSMA_STATE_BACKOFF) {
		module->printOnLog(Logger::LogLevel::DEBUG,
				"CSMA_ALOHA",
				"BackOffTimer::expire(Event *)::current state = " +
						module->status_info[module->curr_state] +
						"; ACK not received, next state = " +
						module->status_info[CSMA_STATE_BACKOFF]);

		module->refreshReason(CSMA_REASON_BACKOFF_TIMEOUT);
		module->exitBackoff();
		module->stateIdle();
	} else {
		module->printOnLog(Logger::LogLevel::DEBUG,
				"CSMA_ALOHA",
				"BackOffTimer::expire(Event *)");
	}
}

void
CsmaAloha::ListenTimer::expire(Event *e)
{
	timer_status = CSMA_EXPIRED;

	if (module->curr_state == CSMA_STATE_LISTEN) {

		module->printOnLog(Logger::LogLevel::DEBUG,
				"CSMA_ALOHA",
				"ListenTimer::expire(Event *)::current state = " +
						module->status_info[module->curr_state] +
						"; ACK not received, next state = " +
						module->status_info[CSMA_STATE_BACKOFF]);

		module->refreshReason(CSMA_REASON_LISTEN_TIMEOUT);
		module->stateTxData();
	} else {
		module->printOnLog(Logger::LogLevel::DEBUG,
				"CSMA_ALOHA",
				"ListenTimer::expire(Event *)");
	}
}

const double CsmaAloha::prop_speed = 1500.0;
int CsmaAloha::u_pkt_id;
bool CsmaAloha::initialized = false;

map<CsmaAloha::CSMA_STATUS, string> CsmaAloha::status_info;
map<CsmaAloha::CSMA_REASON_STATUS, string> CsmaAloha::reason_info;
map<CsmaAloha::CSMA_PKT_TYPE, string> CsmaAloha::pkt_type_info;

CsmaAloha::CsmaAloha()
	: u_data_id(0)
	, last_sent_data_id(-1)
	, TxActive(false)
	, RxActive(false)
	, session_active(false)
	, print_transitions(false)
	, has_buffer_queue(false)
	, start_tx_time(0)
	, srtt(0)
	, sumrtt(0)
	, sumrtt2(0)
	, rttsamples(0)
	, curr_tx_rounds(0)
	, last_data_id_rx(NOT_SET)
	, curr_data_pkt(0)
	, session_distance(SESSION_DISTANCE_NOT_SET)
	, ack_timer(this)
	, backoff_timer(this)
	, listen_timer(this)
	, last_reason(CSMA_REASON_NOT_SET)
	, curr_state(CSMA_STATE_IDLE)
	, prev_state(CSMA_STATE_IDLE)
	, prev_prev_state(CSMA_STATE_IDLE)
	, ack_mode(CSMA_ACK_MODE)
{
	u_pkt_id = 0;
	mac2phy_delay_ = 1e-19;

	bind("HDR_size_", (int *) &HDR_size);
	bind("ACK_size_", (int *) &ACK_size);
	bind("max_tx_tries_", (int *) &max_tx_tries);
	bind("wait_costant_", (double *) &wait_costant);
	bind("debug_", (double *) &debug_);
	bind("max_payload_", (int *) &max_payload);
	bind("ACK_timeout_", (double *) &ACK_timeout);
	bind("alpha_", (double *) &alpha_);
	bind("backoff_tuner_", (double *) &backoff_tuner);
	bind("buffer_pkts_", (int *) &buffer_pkts);
	bind("max_backoff_counter_", (int *) &max_backoff_counter);
	bind("listen_time_", &listen_time);

	if (max_tx_tries <= 0)
		max_tx_tries = INT_MAX;
	if (buffer_pkts > 0)
		has_buffer_queue = true;
	if (listen_time <= 0.0)
		listen_time = 1e-19;
}

int
CsmaAloha::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "setAckMode") == 0) {
			ack_mode = CSMA_ACK_MODE;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setNoAckMode") == 0) {
			ack_mode = CSMA_NO_ACK_MODE;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "initialize") == 0) {
			if (initialized == false)
				initInfo();
			if (print_transitions)
				fout.open("/tmp/CSMAstateTransitions.txt", ios_base::app);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "printTransitions") == 0) {
			print_transitions = true;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getQueueSize") == 0) {
			tcl.resultf("%d", Q.size());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getUpLayersDataRx") == 0) {
			tcl.resultf("%d", getUpLayersDataPktsRx());
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setMacAddr") == 0) {
			std::stringstream addr_ss(argv[2]);
			if (addr_ss >> addr) {
				printOnLog(Logger::LogLevel::INFO,
						"CSMA_ALOHA",
						"command(int, const char *const)::current node MAC "
						"address = " +
								to_string(addr));
				return TCL_OK;
			}

			return TCL_ERROR;
		}
	}
	return MMac::command(argc, argv);
}

void
CsmaAloha::initInfo()
{

	initialized = true;

	if ((print_transitions) && (system(NULL))) {
		system("rm -f /tmp/CSMAstateTransitions.txt");
		system("touch /tmp/CSMAstateTransitions.txt");
	}

	status_info[CSMA_STATE_IDLE] = "Idle state";
	status_info[CSMA_STATE_BACKOFF] = "Backoff state";
	status_info[CSMA_STATE_TX_DATA] = "Transmit DATA state";
	status_info[CSMA_STATE_TX_ACK] = "Transmit ACK state";
	status_info[CSMA_STATE_WAIT_ACK] = "Wait for ACK state";
	status_info[CSMA_STATE_DATA_RX] = "DATA received state";
	status_info[CSMA_STATE_ACK_RX] = "ACK received state";
	status_info[CSMA_STATE_LISTEN] = "Listening channel state";
	status_info[CSMA_STATE_RX_IDLE] = "Start rx Idle state";
	status_info[CSMA_STATE_RX_BACKOFF] = "Start rx Backoff state";
	status_info[CSMA_STATE_RX_LISTEN] = "Start rx Listen state";
	status_info[CSMA_STATE_RX_WAIT_ACK] = "Start rx Wait ACK state";
	status_info[CSMA_STATE_CHK_LISTEN_TIMEOUT] = "Check Listen timeout state";
	status_info[CSMA_STATE_CHK_BACKOFF_TIMEOUT] = "Check Backoff timeout state";
	status_info[CSMA_STATE_CHK_ACK_TIMEOUT] = "Check Wait ACK timeout state";
	status_info[CSMA_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";

	reason_info[CSMA_REASON_DATA_PENDING] = "DATA pending from upper layers";
	reason_info[CSMA_REASON_DATA_RX] = "DATA received";
	reason_info[CSMA_REASON_DATA_TX] = "DATA transmitted";
	reason_info[CSMA_REASON_ACK_TX] = "ACK tranmsitted";
	reason_info[CSMA_REASON_ACK_RX] = "ACK received";
	reason_info[CSMA_REASON_BACKOFF_TIMEOUT] = "Backoff expired";
	reason_info[CSMA_REASON_ACK_TIMEOUT] = "ACK timeout";
	reason_info[CSMA_REASON_DATA_EMPTY] = "DATA queue empty";
	reason_info[CSMA_REASON_MAX_TX_TRIES] = "DATA dropped due to max tx rounds";
	reason_info[CSMA_REASON_LISTEN] = "DATA pending, listening to channel";
	reason_info[CSMA_REASON_LISTEN_TIMEOUT] =
			"DATA pending, end of listening period";
	reason_info[CSMA_REASON_START_RX] = "Start rx pkt";
	reason_info[CSMA_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
	reason_info[CSMA_REASON_BACKOFF_PENDING] = "Backoff timer pending";
	reason_info[CSMA_REASON_WAIT_ACK_PENDING] = "Wait for ACK timer pending";
	reason_info[CSMA_REASON_LISTEN_PENDING] = "Listen to channel pending";
	reason_info[CSMA_REASON_PKT_ERROR] = "Erroneous pkt";

	pkt_type_info[CSMA_ACK_PKT] = "ACK pkt";
	pkt_type_info[CSMA_DATA_PKT] = "DATA pkt";
	pkt_type_info[CSMA_DATAMAX_PKT] = "MAX payload DATA pkt";
}

void
CsmaAloha::resetSession()
{
	session_distance = SESSION_DISTANCE_NOT_SET;
}

void
CsmaAloha::updateRTT(double curr_rtt)
{
	srtt = alpha_ * srtt + (1 - alpha_) * curr_rtt;
	sumrtt += curr_rtt;
	sumrtt2 += curr_rtt * curr_rtt;
	rttsamples++;
	ACK_timeout = srtt;
}

void
CsmaAloha::updateAckTimeout(double rtt)
{
	updateRTT(rtt);

	printOnLog(Logger::LogLevel::INFO,
			"CSMA_ALOHA",
			"updateAckTimeout(double)::current ACK_timeout =  " +
					to_string(ACK_timeout));
}

bool
CsmaAloha::keepDataPkt(int serial_number)
{
	if (serial_number > last_data_id_rx) {
		last_data_id_rx = serial_number;
		return true;
	}

	return false;
}

double
CsmaAloha::computeTxTime(CSMA_PKT_TYPE type)
{
	double duration;
	Packet *temp_data_pkt;

	if (type == CSMA_DATA_PKT) {
		if (!Q.empty()) {
			temp_data_pkt = (Q.front())->copy();
			hdr_cmn *ch = HDR_CMN(temp_data_pkt);
			ch->size() = HDR_size + ch->size();
		} else {
			temp_data_pkt = Packet::alloc();
			hdr_cmn *ch = HDR_CMN(temp_data_pkt);
			ch->size() = HDR_size + max_payload;
		}
	} else if (type == CSMA_ACK_PKT) {
		temp_data_pkt = Packet::alloc();
		hdr_cmn *ch = HDR_CMN(temp_data_pkt);
		ch->size() = ACK_size;
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"CSMA_ALOHA",
				"computeTxTime(CSMA_PKT_TYPE)::invalid packet type =  " +
						to_string(type));
		return -1;
	}

	duration = Mac2PhyTxDuration(temp_data_pkt);
	Packet::free(temp_data_pkt);
	return (duration);
}

void
CsmaAloha::exitBackoff()
{
	backoff_timer.stop();
}

double
CsmaAloha::getBackoffTime()
{
	incrTotalBackoffTimes();
	double random = RNG::defaultrng()->uniform_double();

	backoff_timer.incrCounter();
	int counter = backoff_timer.getCounter();
	if (counter > max_backoff_counter)
		counter = max_backoff_counter;

	double backoff_duration =
			backoff_tuner * random * 2.0 * ACK_timeout * pow(2.0, counter);
	backoffSumDuration(backoff_duration);

	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"getBackoffTime()::backoff_duration =  " +
					to_string(backoff_duration) + " s");

	return backoff_duration;
}

void
CsmaAloha::recvFromUpperLayers(Packet *p)
{
	if ( (has_buffer_queue && (Q.size() < buffer_pkts)) || !has_buffer_queue) {
		initPkt(p, CSMA_DATA_PKT);
		Q.push(p);
		incrUpperDataRx();
		waitStartTime();

		if (curr_state == CSMA_STATE_IDLE) {
			refreshReason(CSMA_REASON_DATA_PENDING);
			stateListen();
		}
	} else {
		incrDiscardedPktsTx();
		drop(p, 1, CSMA_DROP_REASON_BUFFER_FULL);
	}
}

void
CsmaAloha::initPkt(Packet *p, CSMA_PKT_TYPE type, int dest_addr)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);

	int curr_size = ch->size();

	switch (type) {

		case (CSMA_DATA_PKT): {
			ch->size() = curr_size + HDR_size;
			data_sn_queue.push(u_data_id);
			u_data_id++;
		} break;

		case (CSMA_ACK_PKT): {
			ch->ptype() = PT_MMAC_ACK;
			ch->size() = ACK_size;
			ch->uid() = u_pkt_id++;
			mach->set(MF_CONTROL, addr, dest_addr);
			mach->macSA() = addr;
			mach->macDA() = dest_addr;
		} break;
		default:
			printOnLog(Logger::LogLevel::ERROR,
					"CSMA_ALOHA",
					"initPkt(Packet *, CSMA_PKT_TYPE, int)::invalid packet "
					"type =  " + to_string(type));
	}
}

void
CsmaAloha::Mac2PhyStartTx(Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"Mac2PhyStartTx(Packet *)::start tx packet ");

	MMac::Mac2PhyStartTx(p);
}

void
CsmaAloha::Phy2MacEndTx(const Packet *p)
{

	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"Phy2MacEndTx(Packet *)::end tx packet ");

	switch (curr_state) {

		case (CSMA_STATE_TX_DATA): {
			refreshReason(CSMA_REASON_DATA_TX);
			if (ack_mode == CSMA_ACK_MODE) {
				printOnLog(Logger::LogLevel::DEBUG,
						"CSMA_ALOHA",
						"Phy2MacEndTx(Packet *)::DATA sent, from" +
								status_info[curr_state] + " to " +
								status_info[CSMA_STATE_WAIT_ACK]);

				stateWaitAck();
			} else {
				printOnLog(Logger::LogLevel::DEBUG,
						"CSMA_ALOHA",
						"Phy2MacEndTx(Packet *)::DATA sent, from" +
								status_info[curr_state] + " to " +
								status_info[CSMA_STATE_IDLE]);

				stateIdle();
			}
		} break;

		case (CSMA_STATE_TX_ACK): {
			refreshReason(CSMA_REASON_ACK_TX);

			if (prev_prev_state == CSMA_STATE_RX_BACKOFF) {
				printOnLog(Logger::LogLevel::DEBUG,
						"CSMA_ALOHA",
						"Phy2MacEndTx(Packet *)::ACK sent, from" +
								status_info[curr_state] + " to " +
								status_info[CSMA_STATE_CHK_BACKOFF_TIMEOUT]);

				stateCheckBackoffExpired();
			} else if (prev_prev_state == CSMA_STATE_RX_LISTEN) {
				printOnLog(Logger::LogLevel::DEBUG,
						"CSMA_ALOHA",
						"Phy2MacEndTx(Packet *)::ACK sent, from" +
								status_info[curr_state] + " to " +
								status_info[CSMA_STATE_CHK_LISTEN_TIMEOUT]);

				stateCheckListenExpired();
			} else if (prev_prev_state == CSMA_STATE_RX_IDLE) {

				printOnLog(Logger::LogLevel::DEBUG,
						"CSMA_ALOHA",
						"Phy2MacEndTx(Packet *)::ACK sent, from" +
								status_info[curr_state] + " to " +
								status_info[CSMA_STATE_IDLE]);

				stateIdle();
			} else if (prev_prev_state == CSMA_STATE_RX_WAIT_ACK) {
				printOnLog(Logger::LogLevel::DEBUG,
						"CSMA_ALOHA",
						"Phy2MacEndTx(Packet *)::ACK sent, from" +
								status_info[curr_state] + " to " +
								status_info[CSMA_STATE_IDLE]);

				stateCheckAckExpired();
			} else {

				printOnLog(Logger::LogLevel::DEBUG,
						"CSMA_ALOHA",
						"Phy2MacEndTx(Packet *)::logical error in timers, "
						"current state = " + status_info[curr_state]);
				stateIdle();
			}
		} break;

		default: {
			printOnLog(Logger::LogLevel::DEBUG,
					"CSMA_ALOHA",
					"Phy2MacEndTx(Packet *)::logical error in timers, "
					"current state = " + status_info[curr_state]);

			stateIdle();
		} break;
	}
}

void
CsmaAloha::Phy2MacStartRx(const Packet *p)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"Phy2MacStartRx(Packet *)::rx packet");

	refreshReason(CSMA_REASON_START_RX);

	switch (curr_state) {

		case (CSMA_STATE_IDLE):
			stateRxIdle();
			break;

		case (CSMA_STATE_LISTEN):
			stateRxListen();
			break;

		case (CSMA_STATE_BACKOFF):
			stateRxBackoff();
			break;

		case (CSMA_STATE_WAIT_ACK):
			stateRxWaitAck();
			break;

		default: {
			printOnLog(Logger::LogLevel::ERROR,
					"CSMA_ALOHA",
					"Phy2MacStartRx(Packet *)::cannot RX in current state = " +
							status_info[curr_state]);
			stateIdle();
		}
	}
}

void
CsmaAloha::Phy2MacEndRx(Packet *p)
{

	hdr_cmn *ch = HDR_CMN(p);
	packet_t rx_pkt_type = ch->ptype();
	hdr_mac *mach = HDR_MAC(p);
	hdr_MPhy *ph = HDR_MPHY(p);

	int dest_mac = mach->macDA();

	double gen_time = ph->txtime;
	double received_time = ph->rxtime;
	double diff_time = received_time - gen_time;

	double distance = diff_time * prop_speed;

	std::stringstream log_stream;
	log_stream << "Phy2MacEndRx(Packet *)::current state = "
			   << status_info[curr_state]
			   << ", received a pkt type = " << ch->ptype()
			   << ", src addr = " << mach->macSA()
			   << " dest addr = " << mach->macDA()
			   << ", estimated distance between nodes = " << distance << " m ";
	printOnLog(Logger::LogLevel::DEBUG, "CSMA_ALOHA", log_stream.str());

	if (ch->error()) {

		printOnLog(Logger::LogLevel::ERROR,
				"CSMA_ALOHA",
				"Phy2MacEndRx(Packet *)::dropping corrupted packet");
		incrErrorPktsRx();

		refreshReason(CSMA_REASON_PKT_ERROR);
		drop(p, 1, CSMA_DROP_REASON_ERROR);
		stateRxPacketNotForMe(NULL);
	} else {
		if (dest_mac == addr || dest_mac == MAC_BROADCAST) {
			if (rx_pkt_type == PT_MMAC_ACK) {
				refreshReason(CSMA_REASON_ACK_RX);
				stateRxAck(p);
			} else if (curr_state != CSMA_STATE_RX_WAIT_ACK) {
				refreshReason(CSMA_REASON_DATA_RX);
				stateRxData(p);
			} else {
				refreshReason(CSMA_REASON_PKT_NOT_FOR_ME);
				stateRxPacketNotForMe(p);
			}
		} else {
			refreshReason(CSMA_REASON_PKT_NOT_FOR_ME);
			stateRxPacketNotForMe(p);
		}
	}
}

void
CsmaAloha::txData()
{
	Packet *data_pkt = curr_data_pkt->copy();

	if (ack_mode == CSMA_NO_ACK_MODE)
		queuePop();

	incrDataPktsTx();
	incrCurrTxRounds();
	Mac2PhyStartTx(data_pkt);
}

void
CsmaAloha::txAck(int dest_addr)
{
	Packet *ack_pkt = Packet::alloc();
	initPkt(ack_pkt, CSMA_ACK_PKT, dest_addr);

	incrAckPktsTx();
	Mac2PhyStartTx(ack_pkt);
}

void
CsmaAloha::stateRxPacketNotForMe(Packet *p)
{
	printOnLog(Logger::LogLevel::ERROR,
			"CSMA_ALOHA",
			"stateRxPacketNotForMe(Packet *)::dropping packet for another "
			"address");

	if (p != NULL)
		Packet::free(p);

	refreshState(CSMA_STATE_WRONG_PKT_RX);

	switch (prev_state) {

		case CSMA_STATE_RX_IDLE:
			stateIdle();
			break;

		case CSMA_STATE_RX_LISTEN:
			stateCheckListenExpired();
			break;

		case CSMA_STATE_RX_BACKOFF:
			stateCheckBackoffExpired();
			break;

		case CSMA_STATE_RX_WAIT_ACK:
			stateCheckAckExpired();
			break;

		default:
			printOnLog(Logger::LogLevel::ERROR,
					"CSMA_ALOHA",
					"stateRxPacketNotForMe(Packet *)::cannot RX in previous "
					"state = " + status_info[prev_state]);

			stateIdle();
	}
}

void
CsmaAloha::stateCheckListenExpired()
{
	refreshState(CSMA_STATE_CHK_LISTEN_TIMEOUT);

	printOnLog(
			Logger::LogLevel::DEBUG, "CSMA_ALOHA", "stateCheckListenExpired()");

	if (print_transitions)
		printStateInfo();
	if (listen_timer.isActive()) {
		refreshReason(CSMA_REASON_LISTEN_PENDING);
		refreshState(CSMA_STATE_LISTEN);
	} else if (listen_timer.isExpired()) {
		refreshReason(CSMA_REASON_LISTEN_TIMEOUT);
		if (!(prev_state == CSMA_STATE_TX_ACK ||
					prev_state == CSMA_STATE_WRONG_PKT_RX ||
					prev_state == CSMA_STATE_ACK_RX ||
					prev_state == CSMA_STATE_DATA_RX))
			stateTxData();
		else
			stateListen();
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"CSMA_ALOHA",
				"stateCheckListenExpired()::cannot RX in current listen timer "
				"state = " + status_info[curr_state]);

		stateIdle();
	}
}

void
CsmaAloha::stateCheckAckExpired()
{
	refreshState(CSMA_STATE_CHK_ACK_TIMEOUT);

	printOnLog(Logger::LogLevel::DEBUG, "CSMA_ALOHA", "stateCheckAckExpired()");

	if (print_transitions)
		printStateInfo();
	if (ack_timer.isActive()) {
		refreshReason(CSMA_REASON_WAIT_ACK_PENDING);
		refreshState(CSMA_STATE_WAIT_ACK);
	} else if (ack_timer.isExpired()) {
		refreshReason(CSMA_REASON_ACK_TIMEOUT);
		stateBackoff();
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"CSMA_ALOHA",
				"stateCheckAckExpired()::cannot RX in current ack timer "
				"state = " + status_info[curr_state]);

		stateIdle();
	}
}

void
CsmaAloha::stateCheckBackoffExpired()
{
	refreshState(CSMA_STATE_CHK_BACKOFF_TIMEOUT);

	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"stateCheckBackoffExpired()");

	if (print_transitions)
		printStateInfo();
	if (backoff_timer.isActive()) {
		refreshReason(CSMA_REASON_BACKOFF_PENDING);
		stateBackoff();
	} else if (backoff_timer.isExpired()) {
		refreshReason(CSMA_REASON_BACKOFF_TIMEOUT);
		exitBackoff();
		stateIdle();
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"CSMA_ALOHA",
				"stateCheckAckExpired()::cannot RX in current backoff timer "
				"state = " + status_info[curr_state]);

		stateIdle();
	}
}

void
CsmaAloha::stateIdle()
{
	ack_timer.stop();
	backoff_timer.stop();
	listen_timer.stop();
	resetSession();

	refreshState(CSMA_STATE_IDLE);

	if (print_transitions)
		printStateInfo();

	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"stateIdle()::queue size = " + to_string(Q.size()));

	if (!Q.empty()) {
		refreshReason(CSMA_REASON_LISTEN);
		stateListen();
	}
}

void
CsmaAloha::stateRxIdle()
{
	refreshState(CSMA_STATE_RX_IDLE);

	if (print_transitions)
		printStateInfo();
}

void
CsmaAloha::stateListen()
{
	listen_timer.stop();
	refreshState(CSMA_STATE_LISTEN);

	listen_timer.incrCounter();

	double time =
			listen_time * RNG::defaultrng()->uniform_double() + wait_costant;

	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"stateListen()::listen time = " + to_string(time));

	if (print_transitions)
		printStateInfo();

	listen_timer.schedule(time);
}

void
CsmaAloha::stateRxListen()
{
	refreshState(CSMA_STATE_RX_LISTEN);

	if (print_transitions)
		printStateInfo();
}

void
CsmaAloha::stateBackoff()
{
	refreshState(CSMA_STATE_BACKOFF);

	if (backoff_timer.isFrozen())
		backoff_timer.unFreeze();
	else
		backoff_timer.schedule(getBackoffTime());

	printOnLog(Logger::LogLevel::DEBUG, "CSMA_ALOHA", "stateBackoff()");

	if (print_transitions)
		printStateInfo(backoff_timer.getDuration());
}

void
CsmaAloha::stateRxBackoff()
{
	backoff_timer.freeze();
	refreshState(CSMA_STATE_RX_BACKOFF);

	if (print_transitions)
		printStateInfo();
}

void
CsmaAloha::stateTxData()
{
	refreshState(CSMA_STATE_TX_DATA);

	printOnLog(Logger::LogLevel::DEBUG, "CSMA_ALOHA", "stateTxData()");

	if (print_transitions)
		printStateInfo();

	curr_data_pkt = Q.front();

	if (data_sn_queue.front() != last_sent_data_id) {
		resetCurrTxRounds();
		ack_timer.resetCounter();
		listen_timer.resetCounter();
		backoff_timer.resetCounter();
	}
	if (curr_tx_rounds < max_tx_tries) {
		hdr_mac *mach = HDR_MAC(curr_data_pkt);

		mach->macSA() = addr;
		start_tx_time = NOW;
		last_sent_data_id = data_sn_queue.front();
		txData();
	} else {
		queuePop(false);
		incrDroppedPktsTx();

		refreshReason(CSMA_REASON_MAX_TX_TRIES);

		printOnLog(Logger::LogLevel::DEBUG,
				"CSMA_ALOHA",
				"stateTxData()::curr_tx_rounds " + to_string(curr_tx_rounds) +
						" > max_tx_tries = " + to_string(max_tx_tries));

		stateIdle();
	}
}

void
CsmaAloha::stateWaitAck()
{
	ack_timer.stop();
	refreshState(CSMA_STATE_WAIT_ACK);

	printOnLog(Logger::LogLevel::DEBUG, "CSMA_ALOHA", "stateWaitAck()");

	if (print_transitions)
		printStateInfo();

	ack_timer.incrCounter();
	ack_timer.schedule(ACK_timeout + 2 * wait_costant);
}

void
CsmaAloha::stateRxWaitAck()
{
	refreshState(CSMA_STATE_RX_WAIT_ACK);

	if (print_transitions)
		printStateInfo();
}

void
CsmaAloha::stateTxAck(int dest_addr)
{
	refreshState(CSMA_STATE_TX_ACK);

	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"stateTxAck(int)::dest_addr = " + to_string(dest_addr));

	if (print_transitions)
		printStateInfo();

	txAck(dest_addr);
}

void
CsmaAloha::stateRxData(Packet *data_pkt)
{
	refreshState(CSMA_STATE_DATA_RX);

	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"stateRxData(Packet *)::current state = " +
					status_info[curr_state]);

	refreshReason(CSMA_REASON_DATA_RX);

	hdr_mac *mach = HDR_MAC(data_pkt);
	int dst_addr = mach->macSA();

	switch (prev_state) {

		case CSMA_STATE_RX_IDLE: {
			hdr_cmn *ch = hdr_cmn::access(data_pkt);
			ch->size() = ch->size() - HDR_size;
			incrDataPktsRx();
			sendUp(data_pkt);

			if (ack_mode == CSMA_ACK_MODE)
				stateTxAck(dst_addr);
			else
				stateIdle();
		} break;

		case CSMA_STATE_RX_LISTEN: {
			hdr_cmn *ch = hdr_cmn::access(data_pkt);
			ch->size() = ch->size() - HDR_size;
			incrDataPktsRx();
			sendUp(data_pkt);

			if (ack_mode == CSMA_ACK_MODE)
				stateTxAck(dst_addr);
			else
				stateCheckListenExpired();
		} break;

		case CSMA_STATE_RX_BACKOFF: {
			hdr_cmn *ch = hdr_cmn::access(data_pkt);
			ch->size() = ch->size() - HDR_size;
			incrDataPktsRx();
			sendUp(data_pkt);
			if (ack_mode == CSMA_ACK_MODE)
				stateTxAck(dst_addr);
			else
				stateCheckBackoffExpired();
		} break;

		case CSMA_STATE_RX_WAIT_ACK: {
			hdr_cmn *ch = hdr_cmn::access(data_pkt);
			ch->size() = ch->size() - HDR_size;
			incrDataPktsRx();
			sendUp(data_pkt);
			if (ack_mode == CSMA_ACK_MODE)
				stateTxAck(dst_addr);
			else
				stateCheckAckExpired();
		} break;

		default:
			printOnLog(Logger::LogLevel::ERROR,
					"CSMA_ALOHA",
					"stateRxData(Packet *)::cannot RX in previous "
					"state = " + status_info[prev_state]);
	}
}

void
CsmaAloha::stateRxAck(Packet *p)
{
	ack_timer.stop();
	refreshState(CSMA_STATE_ACK_RX);

	printOnLog(Logger::LogLevel::DEBUG, "CSMA_ALOHA", "stateRxAck()");

	Packet::free(p);

	refreshReason(CSMA_REASON_ACK_RX);

	switch (prev_state) {

		case CSMA_STATE_RX_IDLE:
			stateIdle();
			break;

		case CSMA_STATE_RX_LISTEN:
			stateCheckListenExpired();
			break;

		case CSMA_STATE_RX_BACKOFF:
			stateCheckBackoffExpired();
			break;

		case CSMA_STATE_RX_WAIT_ACK:
			queuePop();
			updateAckTimeout(NOW - start_tx_time);
			incrAckPktsRx();
			stateIdle();
			break;

		default:
			printOnLog(Logger::LogLevel::ERROR,
					"CSMA_ALOHA",
					"stateRxAck(Packet *)::cannot RX in previous "
					"state = " + status_info[prev_state]);
	}
}

void
CsmaAloha::printStateInfo(double delay)
{
	printOnLog(Logger::LogLevel::DEBUG,
			"CSMA_ALOHA",
			"AckTimer::printStateInfo(double)::from " +
					status_info[prev_state] + " to " + status_info[curr_state] +
					". Reason: " + reason_info[last_reason]);

	if (curr_state == CSMA_STATE_BACKOFF) {
		fout << left << setw(10) << NOW << "  CsmaAloha(" << addr
			 << ")::printStateInfo() "
			 << "from " << status_info[prev_state] << " to "
			 << status_info[curr_state]
			 << ". Reason: " << reason_info[last_reason]
			 << ". Backoff duration = " << delay << endl;
	} else {
		fout << left << setw(10) << NOW << "  CsmaAloha(" << addr
			 << ")::printStateInfo() "
			 << "from " << status_info[prev_state] << " to "
			 << status_info[curr_state]
			 << ". Reason: " << reason_info[last_reason] << endl;
	}
}
