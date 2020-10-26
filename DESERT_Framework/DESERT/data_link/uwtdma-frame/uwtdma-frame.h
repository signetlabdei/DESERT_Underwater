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
 * @file   uwtdma-frame.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UWTDMA_FRAME</i>.
 *
 */

#ifndef UWTDMA_FRAME_H
#define UWTDMA_FRAME_H

#include <uwtdma.h>
#include <queue>
#include <iostream>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <vector>
#include <map>



typedef std::map<int, int> Slot;
typedef std::map<int, Slot> SlotTopology;

class UwTDMA_frame;

/**
 * Class that represents a TDMA_frame MAC layer of a Node
 */
class UwTDMA_frame : public UwTDMA
{

	friend class UwTDMATimer;

public:
	/**
	 * Constructor of the TDMA_frame class
	 */
	UwTDMA_frame();

	/**
	 * Destructor of the TDMA_frame class
	 */
	virtual ~UwTDMA_frame();

protected:
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
			  (Note that <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
													  successfully or not.
	 */
	virtual int command(int argc, const char *const *argv);

	/**
	 * Alternate TDMA status between MY_STATUS and NOT_MY_STATUS
	 */
	virtual void changeStatus();

	/**
	 * Initialize the topology S 2D matrix from file.
	 * This matrix cointains the schedule slot of each node.
	 */
	virtual void initializeTopologyS();

	Slot::iterator getCurrentSlot();
	Slot::iterator getNextMySlot(int skip = 0);

	int my_slots_counter; /**<count the passed number of slots in which it was
							 active*/
	int tot_nodes; /**<total number of nodes in the network */
	int topology_index; /**<index in the topology matrix */
	int max_packet_per_slot; /**<max numer of packet it can transmit per slot */
	int packet_sent_curr_slot_; /**<counter of packet has been sent in the
								   current slot */
	SlotTopology
			s_; /**<matrix cointaining the transmission schedule of all the
				   network */
	Slot my_slot_numbers_; /**<set the position of the node in the frame
							  (fair_mode)
										  (starting from 0 to tot_slots-1)*/

private:
	string topology_S_file_name_; /**<Topology S file name */
	char topology_S_token_separator_; /**<character used as token separator when
										 importing
										  the S topology from file*/
};

#endif
