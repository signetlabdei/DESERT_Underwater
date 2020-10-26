//
// Copyright (c) 2018 Regents of the SIGNET lab, University of Padova.
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

/**
 * @file   uwmulti-destination.cc
 * @author William Rizzo
 * @version 1.0.0
 *
 * \brief Implementation of UwMultiDestination class.
 *
 */

#include "uwmulti-destination.h"
/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwMultiDestinationClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwMultiDestinationClass()
		: TclClass("Module/UW/MULTI_DESTINATION")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwMultiDestination);
	}
} class_multi_destination;

const int UwMultiDestination::layer_not_exist = -1;

UwMultiDestination::UwMultiDestination()
	: Module()
	, debug_(0)
	, min_delay_(0)
	, switch_mode_(UW_MANUAL_SWITCH)
	, lower_id_active_(0)
	, layer_list()
	, default_lower_id(0)
{
	bind("debug_", &debug_);
	bind("min_delay_", &min_delay_);
	bind("switch_mode_", (int *) &switch_mode_);
	bind("set_lower_id_active_", &lower_id_active_);
}

int
UwMultiDestination::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "setAutomaticSwitch") == 0) {
			switch_mode_ = UW_AUTOMATIC_SWITCH;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setManualSwitch") == 0) {
			switch_mode_ = UW_MANUAL_SWITCH;
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setManualLowerlId") == 0) {
			lower_id_active_ = atoi(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setDefaultLowerId") == 0) {
			default_lower_id = atoi(argv[2]);
			return TCL_OK;
		}
	} else if (argc == 4) {
		
		//parameters: IP_val, layer_id
		if (strcasecmp(argv[1], "addLayer") == 0) {
			if (atoi(argv[2]) < 0) {
				std::cerr << "UwMultiDestination::command. " 
					<<"Error, negative IP address" << std::endl;
				return TCL_ERROR;
			} else {
				IP_range range(atoi(argv[2]), atoi(argv[2]));
				if (!addLayer(range, atoi(argv[3]))) {
					std::cerr << "UwMultiDestination::command. " 
						<<"Error, overlapping IP ranges" << std::endl;
					return TCL_ERROR;
				} 
				
				return TCL_OK;
			}
			
		}
		
	} else if (argc == 5) {
		//parameters: IP_val_min, IP_val_max, layer_id
		if (strcasecmp(argv[1], "addLayer") == 0) {
			if (atoi(argv[2]) < 0 || atoi(argv[3]) < atoi(argv[2])) {
				std::cerr << "UwMultiDestination::command. " 
					<<"Wrong IP address range" << std::endl;
				return TCL_ERROR;
			} else {
				IP_range range(atoi(argv[2]),atoi(argv[3]));
				if (!addLayer(range, atoi(argv[4]))) {
					std::cerr << "UwMultiDestination::command. " 
						<<"Error, overlapping IP ranges" << std::endl;
					return TCL_ERROR;
				}
				return TCL_OK;
			}
			
		}
	}

	return Module::command(argc, argv);
} /* UwMultiDestination::command */

bool
UwMultiDestination::addLayer(IP_range range, int id)
{

	if (layer_list.empty()) {
		layer_IPrange item = std::make_pair(id,range);
		layer_list.push_back(item);
		return true;
	} else {
		if (checkNotOverlap(range)) {
			layer_IPrange item = std::make_pair(id,range);
			layer_list.push_back(item);
			return true;
		}
	}
	return false;
}

void
UwMultiDestination::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	if (ch->direction() == hdr_cmn::UP) {
		sendUp(p, min_delay_);
	} else {
		// direction DOWN: packet is coming from upper layers
		recvFromUpperLayers(p);
	}
}

void
UwMultiDestination::recvFromUpperLayers(Packet *p)
{

	if (switch_mode_ == UW_AUTOMATIC_SWITCH) {
		sendDown(getDestinationLayer(p), p, min_delay_);
	} else {
		sendDown(lower_id_active_, p, min_delay_);
	}
}

int
UwMultiDestination::getDestinationLayer(Packet *p)
{
	hdr_uwip *ih = HDR_UWIP(p);


	int dest_addr = ih->daddr();
	auto it = layer_list.begin();
	for (; it != layer_list.end(); it++) {
		if (it->second.isInRange(dest_addr)) {
			return it->first;
		}
	}
	return default_lower_id;
}

bool
UwMultiDestination::checkNotOverlap(IP_range range)
{
	auto it = layer_list.begin();
	for (; it != layer_list.end(); it++) {
		if (it->second.overlappingRange(range)) {
			return false;
		}
	}
	return true;
}