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
 * @file   uw-csma-ca.cpp
 * @author Federico Favaro
 * @version 1.0.0
 *
 * \brief Provides the implementation of CsmaCa Class
 *
 */

#include "uw-csma-ca.h"
#include "uw-csma-ca-hdrs.h"
#include <queue>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "mac.h"
#include "mmac.h"
#include "rng.h"

extern packet_t PT_CA_CTS;
extern packet_t PT_CA_RTS;

static class CSMACAModuleClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	CSMACAModuleClass()
		: TclClass("Module/UW/CSMA_CA")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new CsmaCa());
	}
} class_module_csmaca;

int
CsmaCa::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	switch (argc) {
		case 2:
			if (!strcasecmp(argv[1], "initialize")) {
				initializeLog();
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "setAckMode")) {
				ack_mode = CSMA_CA_ACK_MODE;
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "setNoAckMode")) {
				ack_mode = CSMA_CA_NO_ACK_MODE;
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "getCTSDropped")) {
				tcl.resultf("%d", cts_pkt_dropped);
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "getRTSDropped")) {
				tcl.resultf("%d", rts_pkt_dropped);
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "getDataDropped")) {
				tcl.resultf("%d", data_pkt_dropped);
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "getQueueSize")) {
				tcl.resultf("%d", getQueueSize());
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "getUpDataRx")) {
				tcl.resultf("%d", up_data_pkts_rx);
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "getRTSRx")) {
				tcl.resultf("%d", n_rts_rx);
				return TCL_OK;
			}
			if (!strcasecmp(argv[1], "getCTSRx")) {
				tcl.resultf("%d", n_cts_rx);
				return TCL_OK;
			}
			break;
		case 3:
			if (!strcasecmp(argv[1], "setMacAddr")) {
				addr = atoi(argv[2]);
				return TCL_OK;
			}
			break;
	}
	return MMac::command(argc, argv);
}

CsmaCa::CsmaCa()
	: max_queue_size(10)
	, backoff_timer(this, CSMA_CA_BACKOFF_TIMER)
	, cts_timer(this, CSMA_CA_CTS_TIMER)
	, data_timer(this, CSMA_CA_DATA_TIMER)
	, ack_timer(this, CSMA_CA_ACK_TIMER)
	, backoff_delta(0)
	, ack_mode(CSMA_CA_NO_ACK_MODE)
	, state(CSMA_CA_IDLE)
	, previous_state(CSMA_CA_IDLE)
	, n_rts_rx(0)
	, n_cts_rx(0)
	, data_pkt_dropped(0)
	, cts_pkt_dropped(0)
	, rts_pkt_dropped(0)
	, ack_pkt_dropped(0)
	, log_level(CSMA_CA_ERROR)
	, actual_data_packet(0)
	, logfile("/dev/null")
{
	bind("queue_size_", (int *) &max_queue_size);
	bind("backoff_delta_", (int *) &backoff_delta);
	bind("backoff_max", (int *) &backoff_max);
	bind("data_size_", (int *) &data_size);
	bind("bitrate_", (int *) &bitrate);
	bind("cts_wait_val_", (int *) &cts_wait_val);
	bind("data_wait_val_", (int *) &data_wait_val);
	bind("ack_wait_val_", (int *) &ack_wait_val);
	bind("log_level_", (int *) &log_level);
}

CsmaCa::~CsmaCa()
{
}

void
CsmaCa::CsmaCaTimer::expire(Event *e)
{
	switch (timer_type) {
		case CSMA_CA_DATA_TIMER:
			module->data_timer_fired();
			break;
		case CSMA_CA_BACKOFF_TIMER:
			module->backoff_timer_fired();
			break;
		case CSMA_CA_CTS_TIMER:
			module->cts_timer_fired();
			break;
		case CSMA_CA_ACK_TIMER:
			module->ack_timer_fired();
			break;
	}
}

void
CsmaCa::ack_timer_fired()
{
	if (state != CSMA_CA_WAIT_ACK) {
		LOGERR("Ack timer fired but not waiting for an ACK\n");
	}
	state_Idle();
}

void
CsmaCa::backoff_timer_fired()
{
	if (getState() == CSMA_CA_BACKOFF) {
		LOGINFO("Backoff end");
		state_Idle();
	} else {
		LOGERR("Backoff timer expired but not in backoff");
	}
}

void
CsmaCa::cts_timer_fired()
{
	if (getState() == CSMA_CA_WAIT_CTS) {
		LOGINFO("CTS Timer expired");
		// state_Idle();
		state_Backoff(1);
	} else {
		LOGERR("CTS timer expired but not waiting for a CTS");
	}
}

void
CsmaCa::data_timer_fired()
{
	if (getState() == CSMA_CA_WAIT_DATA) {
		LOGINFO("Data timer expired ");
		state_Idle();
	} else {
		LOGERR("Data timer expired but not waiting for a DATA");
	}
}

void
CsmaCa::initializeLog()
{
	stringstream strs("");
	strs << "CSMA_CA_" << addr;
	strs >> logfile;

	outLog.open(logfile.c_str());
	if (!outLog) {
		cout << "Error creating log for Csma-Ca" << endl;
	}
}

void
CsmaCa::Mac2PhyStartTx(Packet *p)
{
	MMac::Mac2PhyStartTx(p);
}

void
CsmaCa::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	packet_t rx_pkt_type = ch->ptype();
	hdr_mac *mach = HDR_MAC(p);
	hdr_ca_RTS *rts;
	hdr_ca_CTS *cts;
	int rx_data_ret = 0;

	LOGINFO("Phy2MacEndRx");
	if (ch->error()) {
		LOGWRN("Drop packet due to error");
		drop(p, 1, "REASON_ERROR");
	} else {
		if (rx_pkt_type == PT_CA_RTS) {
			int rts_ret = 0;
			rts = CA_RTS_HDR_ACCESS(p);
			n_rts_rx++;
			rts_ret = stateRxRTS(rts, mach->macSA(), mach->macDA());
			switch (rts_ret) {
				case -2:
					dropPacket(p, CSMA_CA_RTS, DROP_REASON_NOTFORME);
					break;
				case -1:
					dropPacket(p, CSMA_CA_RTS, DROP_REASON_NOTRIGHTSTATE);
					break;
				case 0:
					/* not a drop, just freeing the memory */
					Packet::free(p);
					break;
				default:
					dropPacket(p, CSMA_CA_RTS, DROP_REASON_GENERICERROR);
			}
		} else if (rx_pkt_type == PT_CA_CTS) {
			cts = CA_CTS_HDR_ACCESS(p);
			int cts_ret = 0;
			cts_ret = stateRxCTS(cts, mach->macSA(), mach->macDA());
			n_cts_rx++;
			switch (cts_ret) {
				case -1:
					dropPacket(p, CSMA_CA_CTS, DROP_REASON_NOTFORME);
					break;
				case 0:
					Packet::free(p);
					break;
				default:
					dropPacket(p, CSMA_CA_CTS, DROP_REASON_GENERICERROR);
			}
		} else if (rx_pkt_type == PT_CA_ACK) {
			int ack_ret = 0;
			ack_ret = stateRxACK(p);
			switch (ack_ret) {
				case -1:
					dropPacket(p, CSMA_CA_ACK, DROP_REASON_NOTFORME);
					break;
				case -2:
					dropPacket(p, CSMA_CA_ACK, DROP_REASON_NOTRIGHTSTATE);
					break;
				case 0:
				default:
					Packet::free(p);
			}
		} else {
			rx_data_ret = stateRxData(p);
			if (rx_data_ret == -1) {
				dropPacket(p, CSMA_CA_DATA, DROP_REASON_NOTRIGHTSTATE);
			} else if (rx_data_ret == -2) {
				dropPacket(p, CSMA_CA_DATA, DROP_REASON_NOTFORME);
			}
		}
	}
}

int
CsmaCa::stateRxACK(Packet *ack)
{
	hdr_mac *mac = HDR_MAC(ack);
	if (mac->macDA() == addr) {
		if (state == CSMA_CA_WAIT_ACK) {
			LOGINFO("Ack received\n");
			ack_timer.force_cancel();
			state_Idle();
			return (0);
		} else {
			LOGERR("Received an ACK but not waiting for it\n");
			return (-2);
		}
	} else if ((uint32_t) mac->macDA() == MAC_BROADCAST) {
		LOGERR("Received a broadcast ACK\n");
		return (-1);
	} else {
		LOGWRN("Received an ACK not for us\n");
		return (-1);
	}
}

void
CsmaCa::stateTxAck(int mac_dst)
{
	updateState(CSMA_CA_TX_ACK);
	Packet *ack = buildPacket(mac_dst, CSMA_CA_ACK, 0);
	Mac2PhyStartTx(ack);
}

int
CsmaCa::stateRxData(Packet *p)
{
	LOGINFO("State Rx Data\n");
	hdr_mac *mach = HDR_MAC(p);
	if (mach->macDA() == addr) {
		if (getState() == CSMA_CA_WAIT_DATA) {
			data_timer.force_cancel();
			incrDataPktsRx();
			sendUp(p);
			if (ack_mode == CSMA_CA_ACK_MODE) {
				LOGDBG("Sending ACK for Data\n");
				stateTxAck(mach->macSA());
			} else {
				LOGDBG("No ACK Mode Activated\n");
				state_Idle();
			}
			return (0);
		} else {
			LOGWRN("Received a Data Packet while not waiting for it");
			return (-1);
		}
	} else {
		LOGWRN("Received a Data Packet not for me");
		return (-2);
	}
}

int
CsmaCa::stateRxRTS(hdr_ca_RTS *rts, int mac_src, int mac_dst)
{
	const int WRONG_STATE = -1;
	const int NOT_FOR_US = -2;
	const int ERROR = -3;
	LOGINFO("stateRxRTS");
	if (mac_dst == addr) {
		actual_mac_data_src = mac_src;
		actual_expected_tx_time = rts->get_tx_time();
		if (actual_expected_tx_time <= 0) {
			actual_expected_tx_time = 0;
			actual_mac_data_src = 0;
			LOGERR("Actual actual_expected_tx_time is 0");
			return (ERROR);
		}
		if (getState() == CSMA_CA_IDLE || getState() == CSMA_CA_BACKOFF) {
			if (getState() == CSMA_CA_BACKOFF)
				backoff_timer.force_cancel();
			stateTxCTS();
			return (0);
		} else {
			actual_mac_data_src = 0;
			actual_expected_tx_time = 0;
			LOGWRN("Received an RTS and not in state IDLE");
			return (WRONG_STATE);
		}
	} else if ((uint32_t) mac_dst == MAC_BROADCAST) {
		LOGERR("Just received a broadcast RTS");
		return (NOT_FOR_US);
	} else {
		LOGDBG("Received an RTS not for us");
		return (NOT_FOR_US);
	}
}

void
CsmaCa::stateTxCTS()
{
	LOGINFO("Tx CTS");
	updateState(CSMA_CA_TX_CTS);
	if (!txCTS(actual_mac_data_src)) {
		state_Idle();
		LOGERR("Error sending CTS ");
	}
}

void
CsmaCa::state_Idle()
{
	LOGINFO("State IDLE");
	updateState(CSMA_CA_IDLE);
	if (data_q.size() > 0) {
		extractDataPacket();
	}
}

int
CsmaCa::stateRxCTS(hdr_ca_CTS *cts, int mac_src, int mac_dst)
{
	const int NOT_FOR_ME = -1;
	LOGINFO("State RX_CTS");
	if (mac_dst == addr && getState() == CSMA_CA_WAIT_CTS) {
		cts_timer.force_cancel();
		stateTxData();
		return (0);
	} else if (getState() == CSMA_CA_IDLE) {
		state_Backoff(cts->get_tx_time());
		return (0);
	} else {
		LOGERR("Overheared a CTS not for me and not in IDLE. Discarding \
				and not setting up backoff");
		return (NOT_FOR_ME);
	}
}

int
CsmaCa::stateTxData()
{
	updateState(CSMA_CA_TX_DATA);
	return txData();
}

void
CsmaCa::state_Backoff(int tx_time)
{
	LOGINFO("State Backoff");
	updateState(CSMA_CA_BACKOFF);
	int random = RNG::defaultrng()->uniform(backoff_max) + (int) (tx_time + backoff_delta);
	backoff_timer.resched(random);
}

void
CsmaCa::Phy2MacStartRx(const Packet *p)
{
	LOGDBG("Starting Rx a Packet");
	MMac::Phy2MacStartRx(p);
}

void
CsmaCa::state_Wait_ACK()
{
	LOGINFO("Waiting for ACK\n");
	updateState(CSMA_CA_WAIT_ACK);
	ack_timer.resched(ack_wait_val);
}

void
CsmaCa::Phy2MacEndTx(const Packet *p)
{
	LOGINFO("Phy2MacEndTx");
	hdr_cmn *ch = HDR_CMN(p);
	if (ch->ptype() == PT_CA_CTS) {
		state_Wait_Data();
	} else if (ch->ptype() == PT_CA_RTS) {
		state_Wait_CTS();
	} else if (ch->ptype() == PT_CA_ACK) {
		state_Idle();
	} else {
		if (ack_mode == CSMA_CA_ACK_MODE) {
			actual_data_packet = 0;
			state_Wait_ACK();
		} else {
			actual_data_packet = 0;
			state_Idle();
		}
	}
}

void
CsmaCa::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	if (getQueueSize() <= max_queue_size) {
		data_q.push(p);
		if (state == CSMA_CA_IDLE) {
			state_Idle();
		}
	} else {
		dropPacket(p, CSMA_CA_DATA, "Buffer full");
	}
}

void
CsmaCa::extractDataPacket()
{
	LOGINFO("Extracting Data Packet from queue and sending RTS");
	if (!actual_data_packet) {
		actual_data_packet = data_q.front();
		data_q.pop();
	}
	hdr_mac *mach = HDR_MAC(actual_data_packet);
	if (!txRTS(mach->macDA())) {
		LOGERR("Error sending RTS");
	}
}

void
CsmaCa::dropPacket(Packet *p, csma_ca_pkt_type_t type, char *reason)
{
	drop(p, 1, reason);
	switch (type) {
		case CSMA_CA_RTS:
			rts_pkt_dropped++;
			break;
		case CSMA_CA_CTS:
			cts_pkt_dropped++;
			break;
		case CSMA_CA_DATA:
			data_pkt_dropped++;
			break;
		case CSMA_CA_ACK:
			ack_pkt_dropped++;
			break;
		default:
			break;
	}
}

int
CsmaCa::txRTS(int mac_dest)
{
	Packet *rts;
	rts = buildPacket(mac_dest, CSMA_CA_RTS, computeTxTime());

	if (!rts) {
		return (0);
	}
	Mac2PhyStartTx(rts);
	return (1);
}

int
CsmaCa::txCTS(int mac_dest)
{
	Packet *cts;
	cts = buildPacket(mac_dest, CSMA_CA_CTS, actual_expected_tx_time);

	if (!cts) {
		return (0);
	}

	Mac2PhyStartTx(cts);
	return (1);
}

int
CsmaCa::txData()
{

	if (!actual_data_packet) {
		return (0);
	}

	Mac2PhyStartTx(actual_data_packet);
	incrDataPktsTx();
	return (1);
}

void
CsmaCa::state_Wait_CTS()
{
	LOGINFO("Waiting for CTS");
	updateState(CSMA_CA_WAIT_CTS);
	cts_timer.resched((double) cts_wait_val);
}

void
CsmaCa::state_Wait_Data()
{
	LOGINFO("Waiting for DATA Packet");
	updateState(CSMA_CA_WAIT_DATA);
	data_timer.resched((double) data_wait_val);
}

void
CsmaCa::buildRTShdr(hdr_ca_RTS **rts, uint8_t tx_time)
{
	(*rts)->set_tx_time(tx_time);
}

void
CsmaCa::buildCTShdr(hdr_ca_CTS **cts, uint8_t tx_time)
{
	(*cts)->set_tx_time(tx_time);
}

Packet *
CsmaCa::buildPacket(int mac_dest, csma_ca_pkt_type_t type, uint8_t tx_time)
{
	Packet *p;
	switch (type) {
		case CSMA_CA_RTS: {
			p = Packet::alloc();
			hdr_cmn *ch_r = hdr_cmn::access(p);
			hdr_mac *mac_r = HDR_MAC(p);
			ch_r->ptype() = PT_CA_RTS;
			ch_r->size() = sizeof(hdr_ca_RTS); // 8; /* 8 bit for tx_time */
			mac_r->set(MF_CONTROL, addr, mac_dest);
			hdr_ca_RTS *rts = CA_RTS_HDR_ACCESS(p);
			buildRTShdr(&rts, tx_time);
		} break;
		case CSMA_CA_CTS: {
			p = Packet::alloc();
			hdr_cmn *ch_c = hdr_cmn::access(p);
			hdr_mac *mac_c = HDR_MAC(p);
			ch_c->ptype() = PT_CA_CTS;
			ch_c->size() = sizeof(hdr_ca_CTS); // 8; /* 8 bit for tx_time */
			mac_c->set(MF_CONTROL, addr, mac_dest);
			hdr_ca_CTS *cts = CA_CTS_HDR_ACCESS(p);
			buildCTShdr(&cts, tx_time);
		} break;
		case CSMA_CA_ACK: {
			p = Packet::alloc();
			hdr_cmn *ch_c = hdr_cmn::access(p);
			hdr_mac *mac_c = HDR_MAC(p);
			ch_c->ptype() = PT_CA_ACK;
			ch_c->size() = 1;
			mac_c->set(MF_CONTROL, addr, mac_dest);
			break;
		}
		default:
			LOGERR("Called buidPacket for unknown packet type");
			p = NULL;
	}
	return p;
}

void
CsmaCa::printonLog(csma_ca_log_level_t level, string log)
{
	csma_ca_log_level_t actual_log_level = getLogLevel();
	if (actual_log_level >= log_level) {
		outLog.open((getLogFile()).c_str(), ios::app);
		outLog << left << "[" << getEpoch() << "]::" << NOW << "::"
			   << "(" << addr << ")::" << log_level_string[level] << "::" << log
			   << endl;
		outLog.flush();
		outLog.close();
	}
}
