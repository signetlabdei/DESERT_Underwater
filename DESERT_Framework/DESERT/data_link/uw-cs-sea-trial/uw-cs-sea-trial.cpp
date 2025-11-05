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
 * @file   UwCsSeaTrial.cpp
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UwCsSeaTrial</i>.
 *
 */

#include <iostream>
#include <packet.cc>
#include <stdint.h>
#include <mac.h>
#include <uwmmac-clmsg.h>
#include <uwcbr-module.h>
#include "uw-cs-sea-trial.h"

void
UwSensingTimer::expire(Event *e)
{
	((UwCsSeaTrial *) module)->sensingExpired();
}


/**
 * Class that represent the binding of the protocol with tcl
 */
static class CSTrialModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	CSTrialModuleClass()
		: TclClass("Module/UW/CSTRIAL")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwCsSeaTrial());
	}

} class_UwCsSeaTrial;


UwCsSeaTrial::UwCsSeaTrial()
	: MMac()
	, tx_status(IDLE)
	, debug_(0)
	, HDR_size(0)
	, fix_sens_time(1)
	, rv_sens_time(1)
	, sensing_timer(this)
	, buffer()
	, max_queue_size()
	, max_packet_per_burst(10)
	, packet_sent_curr_burst(10)
	, n_rx_while_sensing(0)
{
	bind("queue_size", (int *) &max_queue_size);
	bind("debug_", (int *) &debug_);
	bind("HDR_size", (int *) &HDR_size);
	bind("max_packet_per_burst", (int *) &max_packet_per_burst);
	bind("fix_sens_time", (double *) &fix_sens_time);
	bind("rv_sens_time", (double *) &rv_sens_time);

	if (max_packet_per_burst < 0) {
		cerr << NOW << " UwCsSeaTrial() not valid max_packet_per_burst < 0!! set to 1 by default " 
			 << std::endl;
		max_packet_per_burst = 1;
	}
	if (fix_sens_time < 0) {
		cerr << NOW << " UwCsSeaTrial() not valid fix_sens_time < 0!! set to 1 by default " 
			 << std::endl;
		fix_sens_time = 1;
	}
	if (rv_sens_time < 0) {
		cerr << NOW << " UwCsSeaTrial() not valid rv_sens_time < 0!! set to 1 by default " 
			 << std::endl;
		rv_sens_time = 1;
	}
}


void
UwCsSeaTrial::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	if (buffer.size() < max_queue_size) {
		initPkt(p);
		buffer.push_back(p);
 		
 	} else {
	if (debug_)
			cout << NOW << " CSTRIAL(" << addr
				 << ")::recvFromUpperLayers() dropping pkt due to buffer full "
				 << std::endl;
		Packet::free(p);
	}
	if (debug_)
			cout << NOW << " CSTRIAL(" << addr
				 << ")::recvFromUpperLayers() and start sensing "
				 << std::endl;
	sensing();
}

void
UwCsSeaTrial::sensing()
{
	if(tx_status != IDLE) { // already sensing or transmitting
		if (debug_)
			cout << NOW << " CSTRIAL(" << addr
				 << ")sensing:: already sensing or transmitting" << std::endl;
		return;
	}	
	n_rx_while_sensing = 0;
	tx_status = SENSING;
	double sensing_time = fix_sens_time + 
		RNG::defaultrng()->uniform_double() * rv_sens_time;
	if (debug_)
			cout << NOW << " CSTRIAL(" << addr
				 << ")sensing::sensing_time = " << sensing_time
				 << std::endl;
	sensing_timer.sched(sensing_time);
}

void
UwCsSeaTrial::sensingExpired()
{
	if (debug_)
			cout << NOW << " CSTRIAL(" << addr
				 << ")::sensingExpired";
	packet_sent_curr_burst = 0;
	if(n_rx_while_sensing) {
		n_rx_while_sensing = 0;
		double sensing_time = fix_sens_time + 
			RNG::defaultrng()->uniform_double() * rv_sens_time;
		if (debug_)
			cout << " but packet rx, new sensing_time = " 
				 << sensing_time << std::endl;
		sensing_timer.resched(sensing_time);
		return;
	}
	if (debug_)
		cout << " I can now transmit" << std::endl;
	tx_status = TRANSMITTING;
	txData();
}

void
UwCsSeaTrial::txData()
{
	if (debug_)
		std::cout << NOW << " CSTRIAL(" << addr
				 << ")::txData" ;
	if (buffer.size() <= 0) {
		tx_status = IDLE;
		if (debug_)
			std::cout << " nothing to tx" << std::endl;
		return;
	}

	if (packet_sent_curr_burst < max_packet_per_burst) {
		Packet *p = buffer.front();
		buffer.pop_front();		
		if (debug_)
			std::cout << " transmitting" << std::endl;
		Mac2PhyStartTx(p);
		incrDataPktsTx();
	}
	else {
		if (debug_)
			cout << NOW << " CSTRIAL(" << addr
				 << ")::already_tx max packet = " << max_packet_per_burst
				 << std::endl;
		tx_status = IDLE;
		sensing();
	}
}

void
UwCsSeaTrial::Mac2PhyStartTx(Packet *p)
{
	MMac::Mac2PhyStartTx(p);

	if (debug_)
		std::cout << NOW << " CSTRIAL(" << addr
				 << ")::Mac2PhyStartTx" << std::endl;
}

void
UwCsSeaTrial::Phy2MacEndTx(const Packet *p)
{
	packet_sent_curr_burst++;
	if (debug_)
		std::cout << NOW << " CSTRIAL(" << addr
				 << ")::Phy2MacEndTx" << std::endl;
	txData();
}

void
UwCsSeaTrial::Phy2MacStartRx(const Packet *p)
{
	if (debug_)
		std::cout << NOW << " CSTRIAL(" << addr
				 << ")::Phy2MacStartRx" << std::endl;
	n_rx_while_sensing++;
}

void
UwCsSeaTrial::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();
	int src_mac = mach->macSA();

	if (ch->error()) {
		if (debug_)
			cout << NOW << " CSTRIAL(" << addr
				 << ")::Phy2MacEndRx() dropping corrupted pkt from node = "
				 << src_mac << std::endl;

		incrErrorPktsRx();
		Packet::free(p);
	} else {
		if (dest_mac != addr && dest_mac != MAC_BROADCAST) {
			rxPacketNotForMe(p);

			if (debug_)
				std::cout << NOW << " CSTRIAL(" << addr 
					<< ")::Phy2MacEndRx packet was for "
						  << dest_mac << std::endl;
		} else {
			sendUp(p);
			incrDataPktsRx();

			if (debug_)
				std::cout << NOW << " CSTRIAL(" << addr
						  << ")::Phy2MacEndRx Received packet from " << src_mac
						  << std::endl;
		}
	}

} 

void
UwCsSeaTrial::initPkt(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);
	mach->macSA() = addr;

	int curr_size = ch->size();

	ch->size() = curr_size + HDR_size;
}

void
UwCsSeaTrial::rxPacketNotForMe(Packet *p)
{
	if (p != NULL)
		Packet::free(p);
}

int
UwCsSeaTrial::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "get_buffer_size") == 0) {
			tcl.resultf("%d", buffer.size());
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
			if (debug_)
				cout << "CSTRIAL MAC address of current node is " << addr
					 << std::endl;
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}