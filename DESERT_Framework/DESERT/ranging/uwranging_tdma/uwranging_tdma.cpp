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
 * @file   uwranging_tdma.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UwRangingTDMA</i>.
 *
 */

#include <mmac.h>
#include <mac.h>
#include "uwranging_tdma.h"
#include "uwranging_tdma_hdr.h"
#include <iostream>
#include <tclcl.h>
#include <cmath>
#include <iomanip>
 
//define a macro to print debug messages
#define DEBUG(level,text) {if (1) {std::cout << NOW << " UwRangingTDMA(" << node_id << "): " << text << std::endl;}}
/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwRangingTDMAModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	UwRangingTDMAModuleClass()
		: TclClass("Module/UW/RANGING_TDMA")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwRangingTDMA());
	}

} class_module_uwrangingtdma;

//initializing static variables
int UwRangingTDMA::count_nodes = 0;

UwRangingTDMA::UwRangingTDMA()
	: UwTDMA()
	,node_id(count_nodes++)
	,slot_id(node_id)
	,n_nodes(tot_slots)
	,slotidmax(SLOTIDMAX_HDR - std::fmod(SLOTIDMAX_HDR,n_nodes) -1)
	,owtt_vec()
	,owtt_map()
{
	slot_number = node_id;
	owtt_vec.resize(((n_nodes * (n_nodes - 1)) / 2) + 1, -1.0);
	owtt_vec[0] = 0.; //so that owtt_vec[owtt_map[a][a]] will return 0

	if (count_nodes > n_nodes) {
		std::cerr << NOW << " UwRangingTDMA() instances are " << count_nodes 
		<< " but parameter tot_slots is set to " << tot_slots << std::endl;
	}	

	// initialize the owtt_map
	owtt_map.resize(n_nodes, std::vector<int>(n_nodes, 0));
	{
		int temp = 1;
		for (int ni = 0; ni < n_nodes; ni++)
		{
			for (int nj = ni + 1; nj < n_nodes; nj++)
			{
				owtt_map[ni][nj] = temp;
				owtt_map[nj][ni] = temp++;
			}
		}
	}
}

UwRangingTDMA::~UwRangingTDMA()
{
}

void
UwRangingTDMA::sendRange()
{
	// build the ranging packet
	Packet *p = Packet::alloc();
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_ranging_tdma *rangh = HDR_RANGING_TDMA(p);
	ch->ptype() = PT_UWRANGING_TDMA;
	mach->set(MF_CONTROL, addr, BCAST_ADDR);
	mach->macDA() = BCAST_ADDR; // hdr_mac->set() doesn't set correctly broadcast address as -1
	rangh->slotId() = slot_id;
	//DEBUG(5,"sending ping with slot_id: " << slot_id)
	slot_id += n_nodes;
	if(slot_id > slotidmax) {
		slot_id = node_id;
	}
	for(int i= 0; i< n_nodes;i++){
		if ( i != node_id) {
			//rangh->times().push_back(half_float::half_cast<half_float::half>(owtt_vec[owtt_map[node_id][i]]));
			rangh->times().push_back(owtt_vec[owtt_map[node_id][i]]);
		}	
	}
	// DEBUG(5,"sending ping with values:")
	// for (auto &&i : rangh->times())
	// {
	// 	std::cout << i*1500.0 << " ";
	// }
	// std::cout << std::endl;

	(ch->size()) += rangh->getSize();
	incrCtrlPktsTx();
	Mac2PhyStartTx(p);
}

void
UwRangingTDMA::stateTxData()
{
	sendRange();
	//txData() should be called by UwTDMA::Phy2MacEndTx()
}

void
UwRangingTDMA::Phy2MacStartRx(const Packet *p)
{
	if (transceiver_status == IDLE)
		transceiver_status = RECEIVING;
}

void
UwRangingTDMA::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	incrCtrlPktsRx(); //use to count all control packets (good+errored ones)
	if(ch->ptype() == PT_UWRANGING_TDMA) {
		
		if (transceiver_status != TRANSMITTING) {
			transceiver_status = IDLE;			
			if (!(ch->error())) {
				hdr_ranging_tdma *rangh = HDR_RANGING_TDMA(p);
				int origin_node = ((rangh->slotId()) % n_nodes);
				double range_rx_time = std::fmod(NOW-Mac2PhyTxDuration(p),slot_duration*(slotidmax+1));
				double tt = range_rx_time - ((rangh->slotId())*slot_duration);
				//DEBUG(0,"reveiced ping from "<<origin_node << " with values")
				{
					int i=0;
					for (auto el : rangh->times())
					{
						if (i == origin_node) {++i;}
						if (el >= 0.) {
							owtt_vec[owtt_map[origin_node][i]] = el; //copy owttimes data from packet						
						}
						++i;
					}
				}
				owtt_vec[owtt_map[node_id][origin_node]] = tt; //save measured owtt
				//DEBUG(0,"update [" << node_id << "][" << origin_node << "] : " << tt*1500.0)
			} else {incrXCtrlPktsRx();} //use incrXCtrlPktsRx() to count control packets with errors
		} else {incrXCtrlPktsRx();}
		Packet::free(p);
	}
	else {UwTDMA::Phy2MacEndRx(p);}
}

void
UwRangingTDMA::Mac2PhyStartTx(Packet *p)
{
	transceiver_status = TRANSMITTING;
	if(sea_trial_) {
		sendDown(p, 0.01);
	} else {
		MMac::Mac2PhyStartTx(p);
	}
}

int
UwRangingTDMA::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 4)
	{
		if (strcasecmp(argv[1], "get_distance") == 0)
		{
			int n1 = atoi(argv[2]);
			int n2 = atoi(argv[3]);
			if (n1 >= 0 && n1 < n_nodes && n2 >= 0 && n2 < n_nodes)
			{
				tcl.resultf("%.17f", owtt_vec[owtt_map[n1][n2]]);
				return TCL_OK;
			}
			return TCL_ERROR;
		}
	}
	return UwTDMA::command(argc, argv);
}
