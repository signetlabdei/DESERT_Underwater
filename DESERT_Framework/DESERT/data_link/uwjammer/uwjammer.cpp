//
// Copyright (c) 2024 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwjammer.cpp
 * @author Riccardo Casagrande
 * @version 1.0.0
 *
 * \brief Implementation of Uwjammer class.
 *
 */

#include "uwjammer.h"
#include "mac.h"
#include "uwjammer_cmn_hdr.h"
#include <iostream>

const std::map<Uwjammer::JammerStatus, std::string> Uwjammer::status_info{
		{JammerStatus::IDLE, "Idle State"},
		{JammerStatus::BUSY, "PHY is transmitting"}};

/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwjammerModule_Class : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwjammerModule_Class()
		: TclClass("Module/UW/JAMMER")
	{
	}

	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new Uwjammer());
	}
} class_module_uwjammer;

Uwjammer::Uwjammer()
	: buffer_data_pkts(0)
	, n_jam_sent(0)
	, n_jam_discarded(0)
	, n_data_discarded(0)
	, curr_data_pkt(0)
	, Q_data()
{
	bind("buffer_data_pkts_", (int *) &buffer_data_pkts);

	if (buffer_data_pkts > MAX_BUFFER_SIZE)
		buffer_data_pkts = MAX_BUFFER_SIZE;
	else if (buffer_data_pkts <= 0)
		buffer_data_pkts = 1;

	curr_state = JammerStatus::IDLE;
}

int
Uwjammer::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "getDataQueueSize") == 0) {
			tcl.resultf("%d", Q_data.size());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getJamSent") == 0) {
			tcl.resultf("%d", getJamSent());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getJamDiscarded") == 0) {
			tcl.resultf("%d", getJamDiscarded());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getDataDiscarded") == 0) {
			tcl.resultf("%d", getDataDiscarded());

			return TCL_OK;
		}

	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setMacAddr") == 0) {
			const char *str = argv[2];
			char *end_ptr{};
			int mac_addr = std::strtol(str, &end_ptr, 10);

			if (str != end_ptr) {
				addr = mac_addr;

				return TCL_OK;
			}

			return TCL_ERROR;
		}
	}

	return MMac::command(argc, argv);
}

int
Uwjammer::crLayCommand(ClMessage *m)
{
	switch (m->type()) {
		default:
			return MMac::crLayCommand(m);
	}
}

void
Uwjammer::recvFromUpperLayers(Packet *p)
{
	if ((int) Q_data.size() < buffer_data_pkts) {
		if (debug_)
			std::cout << NOW << "Uwjammer(" << addr << ")::RECV_FROM_U_LAYERS_"
					  << std::endl;

		Q_data.push(p);

		if (curr_state == JammerStatus::IDLE)
			txJam();

	} else {
		if (debug_)
			std::cout << NOW << "Uwjammer(" << addr << ")::DROP_FULL_QUEUE"
					  << std::endl;

		n_jam_discarded++;

		drop(p, 1, UWJAMMER_DROP_REASON_BUFFER_FULL);
	}
}

void
Uwjammer::txJam()
{
	curr_data_pkt = (Q_data.front())->copy();
	Q_data.pop();

	hdr_cmn *ch = HDR_CMN(curr_data_pkt);
	hdr_mac *mach = HDR_MAC(curr_data_pkt);
	hdr_JAMMER *jammerhdr = HDR_JAMMER(curr_data_pkt);

	if (Q_data.size() > 0) {
		ch->ptype() = PT_JAMMER;
		ch->size() += sizeof(hdr_JAMMER);
	}

	jammerhdr->id_node() = node_id;
	mach->macSA() = addr;

	n_jam_sent++;

	Mac2PhyStartTx(curr_data_pkt);
}

void
Uwjammer::Mac2PhyStartTx(Packet *p)
{
	if (debug_)
		std::cout << NOW << "Uwjammer(" << addr << ")::BUSY_STATE" << std::endl;

	refreshState(JammerStatus::BUSY);

	MMac::Mac2PhyStartTx(p);
}

void
Uwjammer::Phy2MacEndTx(const Packet *p)
{
	MMac::Phy2MacEndTx(p);

	if (debug_)
		std::cout << NOW << "Uwjammer(" << addr
				  << ")::JAM_TRANSMITTED_AND_IDLE_STATE" << std::endl;

	refreshState(JammerStatus::IDLE);
	stateIdle();
}

void
Uwjammer::stateIdle()
{
	if ((int) Q_data.size() > 0)
		txJam();
}

void
Uwjammer::Phy2MacEndRx(Packet *p)
{
	if (debug_)
		std::cout << NOW << "Uwjammer(" << addr
				  << ")::DROP_DATA_PACKET_RECEIVED" << std::endl;

	n_data_discarded++;

	drop(p, 1, UWJAMMER_DROP_REASON_JAMMER_PROTOCOL);
}
