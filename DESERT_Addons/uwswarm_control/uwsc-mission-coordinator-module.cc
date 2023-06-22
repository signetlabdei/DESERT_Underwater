//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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
* @file uwmc-module.cc
* @author Filippo Campagnaro, Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the <i>UWROVCtr</i> class implementation.
*
* Provides the <i>UWROVCtr</i> class implementation.
*/

#include "uwsc-mission-coordinator-module.h"
#include <iostream>
#include <rng.h>
#include <stdint.h>

/**
* Class that represents the binding with the tcl configuration script 
*/
static class UwMissionCoordinatorModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwMissionCoordinatorModuleClass() : TclClass("Module/UW/MC") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwMissionCoordinatorModule());
	}
} class_module_uwMC;


UwMissionCoordinatorModule::UwMissionCoordinatorModule() 
	: UwCbrModule()
	, follower_position()
{
	UWSMPosition lp = UWSMPosition();
	leader_position=&lp;

	UWSMPosition tp = UWSMPosition();
	track_position=&tp;
}

UwMissionCoordinatorModule::UwMissionCoordinatorModule(UWSMPosition* p) 
	: UwCbrModule()
	, leader_position(p)
	, follower_position()
{
	UWSMPosition tp = UWSMPosition();
	track_position=&tp;
}

UwMissionCoordinatorModule::~UwMissionCoordinatorModule() {}

int
UwMissionCoordinatorModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "getX") == 0) {
			tcl.resultf("%f", leader_position->getX());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getY") == 0) {
			tcl.resultf("%f", leader_position->getY());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getZ") == 0) {
			tcl.resultf("%f", leader_position->getZ());
			return TCL_OK;
		}
	}
	else if(argc == 3){
		if (strcasecmp(argv[1], "setPosition") == 0) {
			UWSMPosition* p = dynamic_cast<UWSMPosition*> (tcl.lookup(argv[2]));
			leader_position = p;
			tcl.resultf("%s", "position Setted\n");
			return TCL_OK;
		}
	}
	else if(argc == 5){
		if (strcasecmp(argv[1], "setdest") == 0) {
			leader_position->setdest(atof(argv[2]),atof(argv[3]),atof(argv[4]));
			return TCL_OK;
		}
	}
	else if(argc == 6){
		if (strcasecmp(argv[1], "setdest") == 0) {
			leader_position->setdest(atof(argv[2]),atof(argv[3]),atof(argv[4]),
				atof(argv[5]));
			return TCL_OK;
		}
	}
	return UwCbrModule::command(argc,argv);
}

void
UwMissionCoordinatorModule::setPosition(UWSMPosition* p){
	leader_position = p;
}

int
UwMissionCoordinatorModule::recvSyncClMsg(ClMessage* m)
{
	if (m->type() == CLMSG_CTR2MC_GETPOS)
	{
		int id = ((ClMsgCtr2McPosition*)m)->getSource();
		UWSMPosition* p = ((ClMsgCtr2McPosition*)m)->getRovPosition();

		follower_position[id] = p;

		return 0;
	}
	if (m->type() == CLMSG_TRACK2MC_TRACKPOS)
	{
		track_position = ((ClMsgTrack2McPosition*)m)->getTrackPosition();

		return 0;
	}
	return UwCbrModule::recvSyncClMsg(m);
}
