//
// Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwranging_tokenbus.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UwRangingTokenBus</i>.
 *
 */

#include "uwranging_tokenbus.h"
#include "uwranging_tokenbus_hdr.h"
#include <mac.h>
#include <iostream>
#include <tclcl.h>
#include "least_squares.h"

extern packet_t PT_UWTOKENBUS;
extern packet_t PT_UWRANGING_TOKENBUS;
/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwRangingTokenBusModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the UwRangingTokenBusModule class
	 */
	UwRangingTokenBusModuleClass()
		: TclClass("Module/UW/RANGING_TOKENBUS")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int argc, const char *const *argv)
	{
		return (new UwRangingTokenBus());
	}

} class_module_uwranging_tokenbus;

// define a macro to print debug messages
#define DEBUG(level, text)                                                                      \
	{                                                                                           \
		if (debug >= level)                                                                     \
		{                                                                                       \
			std::cout << NOW << " UwRangingTokenBus(" << node_id << "): " << text << std::endl; \
		}                                                                                       \
	}

// default constructor
UwRangingTokenBus::UwRangingTokenBus()
	: UwTokenBus(),
	  dist_num(n_nodes * (n_nodes - 1) / 2),
	  epsilon(1e-6),
	  max_tt(5),
	  dist_map(),
	  times_mat(),
	  times_age(),
	  x_mat(),
	  time_last_range(0),
	  id_last_range(-1)
{
	bind("epsilon", (double *)&epsilon);
	bind("max_tt", (double *)&max_tt);

	times_mat.resize(n_nodes, std::vector<double>(n_nodes - 1, -1.0)); // initialize the matrix
	times_age.resize(n_nodes, std::vector<int>(n_nodes - 1, 0));
	distances.resize(dist_num, -1.0);

	// initialize the distance map
	dist_map.resize(n_nodes, std::vector<int>(n_nodes, -1));
	int temp = 0;
	for (size_t ni = 0; ni < n_nodes; ni++)
	{
		for (size_t nj = ni + 1; nj < n_nodes; nj++)
		{
			dist_map[NMOD(ni)][NMOD(nj)] = temp;
			dist_map[NMOD(nj)][NMOD(ni)] = temp++;
		}
	}

	x_mat.resize(2 * dist_num, std::vector<double>(dist_num, 0.0)); // initialize null matrix 2d x dist_num
	size_t eq = 0;													// counter from 0 to 2d as index
	for (size_t ni = 0; ni < n_nodes; ni++)
	{
		x_mat[eq++][dist_map[NMOD(ni)][NMOD(ni + 1)]] = 2.0; // TWTT range measure with next node t(ni+1,ni)
		for (size_t n = ni + 2; n < ni + n_nodes; n++)		 //[t(n,n-1) + t(n,ni) - t(n-1,ni),...] for n = [ni+2…ni+n_nodes-1]
		{
			x_mat[eq][dist_map[NMOD(n)][NMOD(n - 1)]] = 1.0;
			x_mat[eq][dist_map[NMOD(n)][NMOD(ni)]] = 1.0;
			x_mat[eq++][dist_map[NMOD(n - 1)][NMOD(ni)]] = -1.0;
		}
	}

	/*uncomment to print the coefficient matrix for each node*/
	//  std::cout << std::endl;
	//  for (size_t i = 0; i < x_mat.size(); i++)
	//  {
	//  	for (size_t j = 0; j < x_mat[i].size(); j++)
	//  	{
	//  		cout << x_mat[i][j] << " ";
	//  	}
	//  	std::cout << std::endl;
	//  }
}


UwRangingTokenBus::~UwRangingTokenBus()
{
}

void UwRangingTokenBus::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_tokenbus *tbh = HDR_TOKENBUS(p);
	int dest_mac = mach->macDA();
	int pkt_token_id = tbh->tokenId();
	int pkt_node_id = NMOD(pkt_token_id); // find which node the token is meant to
	DEBUG(10, " Phy2MacEndRx(); pkt token_id : " << pkt_token_id << " macSA: " << mach->macSA() << " macDA: " << mach->macDA())
	if (rtx_status != TRANSMITTING)
	{
		rtx_status = IDLE;
		if (ch->error())
		{
			/* in case of errors I assume the token_id in the header might be corrupted
			 * so I won't consider it for rescheduling the timers
			 */
			DEBUG(3, "Phy2MacEndRx() dropping pkt with errors")
			if (ch->ptype() == PT_UWRANGING_TOKENBUS) 
				{incrXCtrlPktsRx();}
			else {incrErrorPktsRx();}
			Packet::free(p);
		}
		else
		{
			if (validToken(p))
			{
				token_pass_timer.force_cancel();
				bus_idle_timer.force_cancel();
				last_token_id_heard = pkt_token_id;

				if (ch->ptype() == PT_UWRANGING_TOKENBUS)
				{
					DEBUG(4, " received a ping with pkt_token_id: " << pkt_token_id << " from ID " << NMOD(pkt_token_id - 1))
					hdr_uwranging_tokenbus *tbrh = HDR_UWRANGING_TOKENBUS(p);
					incrCtrlPktsRx();
					if (pkt_token_id >= id_last_range)	
					{
						if ((normId(id_last_range + 1) == pkt_token_id) && (tbrh->token_hold() >= 0)) //if false packet RX time is not meaningful
						{
							times_mat[NMOD(node_id)][NMOD(pkt_node_id - node_id - 2)] = (NOW - time_last_range - tbrh->token_hold()); // update times_mat according to rx time
							times_age[NMOD(node_id)][NMOD(pkt_node_id - node_id - 2)] = pkt_token_id;
						}																										  // true if the token timestamp is valid
						
						for (size_t i = 0; i < (tbrh->times()).size(); i++)
						{
							if(tbrh->times()[i] >= 0) {
								times_mat[NMOD(pkt_node_id - 1)][i] = tbrh->times()[i]; //copy times data from packet
								times_age[NMOD(pkt_node_id - 1)][i] = pkt_token_id;
							}
						}
												
						if (debug > 10)
						{
							DEBUG(0, " updated times_mat[" << node_id << "][" << NMOD(pkt_node_id - node_id - 2)
														   << "]" << " = " << NOW - time_last_range - tbrh->token_hold() 
														   << " and times_mat[" << NMOD(pkt_node_id - 1) << "] = ")
							for (int i=0;i<times_mat[NMOD(pkt_token_id-1)].size();i++)
							{
								std::cout << times_mat.at(NMOD(pkt_token_id-1)).at(i) << " ";
							}
							std::cout << std::endl;
						}
					}

					id_last_range = pkt_token_id;
					time_last_range = NOW;
				}

				if (pkt_node_id == node_id)
				{
					got_token = true;
					last_token_id_owned = pkt_token_id;
					token_rx_time = NOW;
					token_pass_timer.resched(min_token_hold_time);
					DEBUG(4, "Received the token")
					txData();
				}
				else if (pkt_node_id < node_id)
				{ // pkt from previous node in the ring
					bus_idle_timer.resched((3 * (node_id - pkt_node_id + 1) * bus_idle_timeout));
				}
				else // consider ring turnaround
					bus_idle_timer.resched((3 * (n_nodes - pkt_node_id + node_id + 1) * bus_idle_timeout));
			}
			else
			{
				DEBUG(0, " received a pkt with invalid token id")
			}

			if (ch->ptype() == PT_UWTOKENBUS || ch->ptype() == PT_UWRANGING_TOKENBUS)
			{
				Packet::free(p);
			}
			else if ((dest_mac != addr) && (dest_mac != BCAST_ADDR))
			{
				Packet::free(p);
				incrXDataPktsRx();
			}
			else
			{
				sendUp(p);
				incrDataPktsRx();
			}
		}
	}
	else
	{
		if (ch->ptype() == PT_UWRANGING_TOKENBUS) 
			{incrXCtrlPktsRx();}
		else {incrErrorPktsRx();}
		DEBUG(0, " discard packet rx while tx; pkt token_id : " << pkt_token_id << " macSA: " << mach->macSA() << " macDA: " << mach->macDA())
		Packet::free(p); // assume the packet got corrupted in collision with tx packet
	}
}

void UwRangingTokenBus::sendToken(int token_id)
{
	if (!got_token)
	{
		DEBUG(0, " attempt to send token without owning it ")
		return;
	}
	// build the token packet
	Packet *p = Packet::alloc();
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_tokenbus *tbh = HDR_TOKENBUS(p);
	hdr_uwranging_tokenbus *tbrh = HDR_UWRANGING_TOKENBUS(p);
	ch->ptype() = PT_UWRANGING_TOKENBUS;
	mach->set(MF_CONTROL, addr, BCAST_ADDR);
	mach->macDA() = BCAST_ADDR; // hdr_mac->set() doesn't set correctly broadcast address as -1
	tbh->tokenId() = token_id;
	id_last_range = token_id;
	tbrh->token_resend() = false;
	for (size_t i = 0; i < times_mat[NMOD(node_id)].size(); i++)
	{
		if (normId(times_age[NMOD(node_id)][i] + 1*n_nodes) >= token_id) { //send only time measures not older than 1*n_nodes
			//(tbrh->times()).push_back(half_float::half_cast<half>(times_mat[NMOD(node_id)][i]));
			(tbrh->times()).push_back((times_mat[NMOD(node_id)][i]));
		}
		else {
			//(tbrh->times()).push_back(half_float::half_cast<half>(-1.)); //mark as invalid if older than 1 round
			(tbrh->times()).push_back((-1.)); //mark as invalid if older than 1 round
		}
	}
	
	(ch->size()) += tbrh->getSize() + tbh->getSize();
	tbrh->token_hold() = NOW + Mac2PhyTxDuration(p) - token_rx_time;

	if (last_token_id_heard == token_id) // true when I'm sending the token for the second time
	{
		tbh->tokenId() = normId(token_id + n_nodes); //jump n_nodes in tokenId to make distinguishable from the first token sent
		id_last_range = normId(token_id + n_nodes);
		tbrh->token_resend() = true;
	}
	else if (normId(last_token_id_heard + 1) != token_id) // true when bus_idle_timer has expired and i regen the token
	{
		tbrh->token_resend() = false;
		tbrh->token_hold() = -1.0; // flag an invalid hold time
	}

	time_last_range = NOW + Mac2PhyTxDuration(p);
	got_token = false;
	DEBUG(4, " sending TOKEN to node: " << NMOD(token_id) << " tokenid: " << token_id << " token hold: " << tbrh->token_hold() << " from mac " << mach->macSA() << " to mach " << mach->macDA())
	DEBUG(10,"pkt size: "<<ch->size() << " tx duration: "<< Mac2PhyTxDuration(p))
	incrCtrlPktsTx();
	Mac2PhyStartTx(p);
}

void UwRangingTokenBus::computeDist()
{
	std::vector<double> y;									  // vector with known terms for linear regression
	auto x_full = std::vector<std::vector<double>>(dist_num); // outer vec contains the coeff for a single unknown distance
	auto dist_mask = std::vector<bool>(dist_num, false);	  // indicates if a distance should be included in the LR (there is at least one equation involving it)
	size_t i_eq = 0;										  // index of eq in X_mat matrix
	size_t i_valid_eq = 0;									  // index of eq in the final x matrix
	for (size_t n = 0; n < n_nodes; n++)
	{
		for (size_t t = 0; t < n_nodes - 1; t++)
		{
			if ((normId(times_age[NMOD(n)][NMOD(t)] + 2*n_nodes) >= id_last_range) && (times_mat[NMOD(n)][NMOD(t)] >= -epsilon))
			{
				y.push_back(max(times_mat[NMOD(n)][NMOD(t)], 0.0)); // populate the y vector
				for (size_t coeff = 0; coeff < dist_num; coeff++)	// coeff is the index of the distance in X_mat
				{
					(x_full[coeff]).push_back(x_mat[i_eq][coeff]);
					if (x_mat[i_eq][coeff])
					{ // if coeff is not zero, the relative unknown will be included in LR
						dist_mask[coeff] = true;
					}
				}
				++i_valid_eq;
			}
			++i_eq;
		}
	}

	// count the number of distances involved in the equations
	size_t count_valid_dist = 0;
	for (size_t i = 0; i < dist_num; i++)
	{
		if (dist_mask[i])
		{
			++count_valid_dist;
		}
	}

	std::vector<std::vector<double>> a;	  // a is the flattened version of the "A" coefficient matrix (x_full) for the least squares solver
	for (size_t i = 0; i < dist_num; i++) // copy all the meaningful columns (which have at least one non zero coeff)
	{
		if (dist_mask[i])
		{
			a.push_back(x_full[i]);
		}
	}

	if (count_valid_dist <= y.size())
	{
		auto solution = std::vector<double>(count_valid_dist, -1.0); // allocate solution vector
		LSSQ::LeastSqResult nnls_status = LSSQ::nnLeastSquares(a, y, solution);
		if (nnls_status == LSSQ::LeastSqResult::OK) // if I have a valid solution
		{
			// distances = std::vector<double> (dist_num,-1.0); //uncomment to forget previous distance values

			for (size_t i = 0, j = 0; i < dist_num; i++)
			{
				if (dist_mask[i] && solution[j] >= 0.0 && solution[j] <= max_tt)
				{
					distances[i] = solution[j++];
				}
			}
		}
		else
		{
			if (nnls_status == LSSQ::LeastSqResult::TIMEOUT)
			{
				DEBUG(0, " nnleast_squares() TIMEOUT! keeping old distances vector")
			}
			else
			{
				DEBUG(0, " nnleast_squares() ERROR! keeping old distances vector")
			}
		}
	}
}

int UwRangingTokenBus::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) 
	{
		if (strcasecmp(argv[1], "calc_distances") == 0)
		{
			computeDist(); 
			return TCL_OK;
		}
	}
	if (argc == 4)
	{
		if (strcasecmp(argv[1], "get_distance") == 0)
		{
			int n1 = atoi(argv[2]);
			int n2 = atoi(argv[3]);
			if (n1 >= 0 && n1 < n_nodes && n2 >= 0 && n2 < n_nodes)
			{
				if (n1==n2) {
					tcl.resultf("%.17f",0.);
				} 
				else {
					tcl.resultf("%.17f", distances[dist_map[NMOD(n1)][NMOD(n2)]]);
				}
				return TCL_OK;
			}
			return TCL_ERROR;
		}
	}
	return UwTokenBus::command(argc, argv);
}

bool UwRangingTokenBus::validToken(Packet *p) const
{
	if (HDR_UWRANGING_TOKENBUS(p)->token_resend()) // if the token have been resent, make sure I haven't already received the original
	{
		int token_id = HDR_TOKENBUS(p)->tokenId();
		return (token_id > normId(last_token_id_owned + n_nodes));
	}
	return UwTokenBus::validToken(p);
}
