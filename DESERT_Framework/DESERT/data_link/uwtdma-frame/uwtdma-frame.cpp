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
 * @file   uwtdma-frame.cpp
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UWTDMA_FRAME</i>.
 *
 */

#include "uwtdma-frame.h"
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mac.h>
#include <string>

extern packet_t PT_UWHEALTHCBR;
extern packet_t PT_UWCONTROLCBR;
/**
 * Class that represent the binding of the protocol with tcl
 */
static class TDMA_FRAMEModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDMAGenericModule class
	 */
	TDMA_FRAMEModuleClass()
		: TclClass("Module/UW/TDMA_FRAME")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwTDMA_frame());
	}

} class_uwtdma;

/*void UwTDMATimer::expire(Event *e)
{
  ((UwTDMA *)module)->changeStatus();
}*/

UwTDMA_frame::UwTDMA_frame()
	: UwTDMA()
	, tot_nodes(0)
	, my_slots_counter(0)
	, topology_S_file_name_("")
	, topology_S_token_separator_(',')
	, topology_index(0)
{
	fair_mode = 1;
	{
		bind("guard_time", (double *) &guard_time);
		tot_slots = 0;
	}
}

UwTDMA_frame::~UwTDMA_frame()
{
}

void
UwTDMA_frame::changeStatus()
{
	packet_sent_curr_slot_ = 0;
	if (slot_status == UW_TDMA_STATUS_NOT_MY_SLOT) {
		my_slots_counter++;
		if (debug_)
			std::cout << NOW << " ID:" << addr
					  << ", my_slots_counter:" << my_slots_counter << std::endl;
	}
	if (slot_status == UW_TDMA_STATUS_MY_SLOT) {
		slot_status = UW_TDMA_STATUS_NOT_MY_SLOT;
		int num_jumping_slots =
				getNextMySlot()->first - getCurrentSlot()->first;
		num_jumping_slots = num_jumping_slots > 0
				? num_jumping_slots
				: num_jumping_slots + tot_slots;
		double nextSlotTime =
				(num_jumping_slots - 1) * slot_duration + guard_time;
		tdma_timer.resched(nextSlotTime);

		if (debug_ < -5)
			std::cout << NOW << " Off ID " << addr << " " << nextSlotTime << ""
					  << std::endl;
		if (sea_trial_)
			out_file_stats << left << "[" << getEpoch() << "]::" << NOW
						   << "::TDMA_node(" << addr << ")::Off timeslot "
						   << getCurrentSlot()->first << std::endl;
	} else
		UwTDMA::changeStatus();
}

int
UwTDMA_frame::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			if (fair_mode == 0) {
				std::cout << "Error: the near far tdma works only in fair mode"
						  << std::endl;
				return TCL_ERROR;
			}
			if (tot_slots == 0) {
				std::cout << "Error: number of slots set to 0, please, "
							 "initialize the topology"
						  << std::endl;
				return TCL_ERROR;
			} else {
				slot_duration = frame_duration / tot_slots;
				if (slot_duration - guard_time < 0) {
					std::cout << "Error: guard time or frame set incorrectly"
							  << std::endl;
					return TCL_ERROR;
				} else {
					Slot::iterator iter = my_slot_numbers_.begin();
          start_time = iter->first * slot_duration;
					start(iter->first * slot_duration);
					return TCL_OK;
				}
			}
		} else if (strcasecmp(argv[1], "stop") == 0) {
			stop();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setSlotNumber") == 0) {
			std::cout << "Use near far topology!!" << std::endl;
			return TCL_ERROR;
		} else if (strcasecmp(argv[1], "setTopologyIndex") == 0) {
			topology_index = atoi(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setSTopologyFileName") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty string for the file name");
				return TCL_ERROR;
			}
			topology_S_file_name_ = tmp_;
			initializeTopologyS();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setTopologySeparator") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty char for the file name");
				return TCL_ERROR;
			}
			topology_S_token_separator_ = tmp_.at(0);
			return TCL_OK;
		}
	}
	return UwTDMA::command(argc, argv);
}

Slot::iterator
UwTDMA_frame::getCurrentSlot()
{
	Slot::iterator iter = my_slot_numbers_.begin();
	int pos = ((my_slots_counter - 1) % my_slot_numbers_.size());
	// std::cout<< NOW << " ID:" << addr << " Slots SIZE:" <<
	// my_slot_numbers_.size() << std::endl;
	if (debug_ < -5)
		std::cout << NOW << " ID:" << addr << " Slot Pos:" << pos << std::endl;
	for (int i = 0; i < pos; i++) {
		iter++;
	}
	return iter;
}

Slot::iterator
UwTDMA_frame::getNextMySlot(int skip)
{
	Slot::iterator iter = my_slot_numbers_.begin();
	int pos = ((my_slots_counter + skip) % my_slot_numbers_.size());
	if (debug_ < -5)
		std::cout << NOW << " ID:" << addr << " Pos:" << pos << std::endl;
	for (int i = 0; i < pos; i++) {
		iter++;
	}
	return iter;
}

void
UwTDMA_frame::initializeTopologyS()
{
	topology_index = topology_index ? topology_index : addr;
	ifstream input_file_;
	string line_;
	char *tmp_ = new char[topology_S_file_name_.length() + 1];
	strcpy(tmp_, topology_S_file_name_.c_str());
	input_file_.open(tmp_);
	int slot_number = 0;
	if(tot_slots || tot_nodes) { //initialize the frame again due to frame change
		tot_slots = 0;
		tot_nodes = 0;
		s_.clear();
		my_slot_numbers_.clear();
	}	
	if (input_file_.is_open()) {
		while (std::getline(input_file_, line_)) {
			::std::stringstream line_stream(line_);
			string result_;
			slot_number = 0;
			while (std::getline(
					line_stream, result_, topology_S_token_separator_)) {
				slot_number++;
				int tx_status = atoi(result_.c_str());
				s_[tot_nodes + 1][slot_number] = tx_status;
				if (tot_nodes + 1 == topology_index && tx_status > 0) {
					// cout << "Node " <<addr << "S[" << slot_number << "] = "
					// << tx_status <<endl;
					my_slot_numbers_[slot_number] = tx_status;
				}
				if (!tot_nodes) {
					tot_slots++;
				}
			}
			tot_nodes++;
		}
		// std::cout << NOW << " ID " << addr << " num slots: "
		// <<my_slot_numbers_.size() << std::endl;
	} else {
		cerr << "Impossible to open file " << topology_S_file_name_.c_str() <<
				endl;
	}
	if (debug_) {
		std::cout << NOW << " ID " << addr
				  << ": Topology S initialized, tot_nodes = " << tot_nodes
				  << ", Slots in a frame = " << tot_slots << std::endl;
	}
}
