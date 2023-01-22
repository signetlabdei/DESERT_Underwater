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
 * @file   uwranging_tdma.h
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UwRangingTDMA</i>.
 *
 */

#ifndef UWRANGINGTDMA_H
#define UWRANGINGTDMA_H

#include "uwtdma.h"
extern packet_t PT_UWRANGING_TDMA;

/**
 * Class that represents a UwRangingTDMA Node
 */
class UwRangingTDMA : public UwTDMA
{

public:
	/**
	 * Constructor of the UwRangingTDMA class
	 */
	UwRangingTDMA();

	/**
	 * Destructor of the UwRangingTDMA class
	 */
	virtual ~UwRangingTDMA();

protected:

	/**
	 * sends the ranging packet at the beginning of my slot
	 */
	virtual void sendRange();
	/**
	 * Change transceiver status and and start to transmit if in my slot
	 * Used when there's spare time, useful for transmitting other packtes.
	 */
	virtual void stateTxData();
	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p);
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to a Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p);
	/**
	 * Method called when the Mac Layer start to transmit a Packet
	 * @param const Packet* Pointer to a Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Mac2PhyStartTx(Packet *p);

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
	 * @return TCL_OK or TCL_ERROR 
	 */
	virtual int command(int argc, const char *const *argv);

	static int count_nodes;	/**< counts the instantiated nodes, used for assigning node ids in default contructor*/
	int node_id;	/**<id of the node (0 to n_nodes-1)*/
	int slot_id; /**< = node_id + k*n_nodes; slot_id value is written in the outgoing ranging packet then k is incremented */
	int n_nodes; /**< number of nodes */
	int slotidmax; /**< maximum slot_id allowable in packet header*/
	/** vector of lenght D = n_nodes*(n_nodes-1)/2 + 1 where the one way travel times are stored*/
	std::vector<double> owtt_vec;
	std::vector<std::vector<int>> owtt_map; /**< of size [n_nodes][n_nodes] maps(nodeX,nodeY) -> owtt_vec index */
};

#endif
