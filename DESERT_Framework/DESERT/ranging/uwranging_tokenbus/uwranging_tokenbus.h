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
 * @file   uwranging_tokenbus.h
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UwRangingTokenBus</i>.
 *
 */

#ifndef UWRANGINGTOKENBUS_H
#define UWRANGINGTOKENBUS_H

#include "uwtokenbus.h"

extern packet_t PT_UWRANGING_TOKENBUS;

/**
 * Class that represents a TokenBus Node
 */
class UwRangingTokenBus : public UwTokenBus
{

public:
	/**
	 * Default constructor of the TokenBus class
	 */
	UwRangingTokenBus();

	/**
	 * Destructor of the TokenBus class
	 */
	virtual ~UwRangingTokenBus();


protected:
	
	/**
	 * Assert if the received token id is valid, i.e it follows the monotonic progression
	 * taking in account overflow.
	 * 
	 * @param p Packet with token
	 */
	virtual bool validToken(Packet *p) const override;

	/**
	 * Passes the token to the next node
	 * @param next_id node receiving the token
	 */
	virtual void sendToken(int next_id) override;

	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param p pointer to a Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p) override;


	/**
	 * @brief compute the linear regression and updates the distances vector
	 * 
	 */
	virtual void computeDist();

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
							 (Note that <i>argv[0]</i> is the name of the
	 object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
															  successfully or
	 not.
	 */
	virtual int command(int argc, const char *const *argv);

	const int dist_num; /**< num of distances: will be initialized to n_nodes*(n_nodes-1)/2 */
	/** difference between virtually equal distances can result in small negative numbers due to floating point precision so values within epsilon will not be discarded by NNLS algo */
	double epsilon; 			
	double max_tt; /**< max travel time between nodes in seconds, used to discard bad nnleast_squares() results */
	std::vector<std::vector<int>> dist_map; /**< of size [n_nodes][n_nodes] maps(nodeX,nodeY) -> distance */
	/** vector of shape [n_nodes][n_nodes-1] holds the travel times: the first index is the node_id which has calculated them */
	std::vector<std::vector<double>> times_mat;
	/** vector of shape [n_nodes][n_nodes-1] holds the age of a time (slot number in which the time was calculate or received)*/
	std::vector<std::vector<int>> times_age;  										
	std::vector<std::vector<double>>  x_mat; /**< of size [2D][D] it's the sparse matrix with the equations coefficients (-1,0,1) */
	/** vector of shape [D], contains the one way travel times between nodes to be transformed to distances by the user according to the chosen speed of sound model */
	std::vector<double> distances;  
	double time_last_range; /**< time of last ping reception (or transmission) */
	int id_last_range; /**< node id from which I received the last ranging pkt */
};

#endif
