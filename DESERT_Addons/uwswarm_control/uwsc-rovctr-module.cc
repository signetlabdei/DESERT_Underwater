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
* @file uwsc-rovctr-module.cc
* @author Filippo Campagnaro, Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the <i>UWROVCtr</i> class implementation.
*
* Provides the <i>UWROVCtr</i> class implementation.
*/

#include "uwsc-rovctr-module.h"
#include <iostream>

/**
* Class that represents the binding with the tcl configuration script 
*/
static class UwSCROVCtrModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwSCROVCtrModuleClass() : TclClass("Module/UW/SC/CTR") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwSCROVCtrModule());
	}
} class_module_uwROV_ctr;

UwSCROVCtrModule::UwSCROVCtrModule() 
	: UwROVCtrModule()
{
	bind("leader_id", (int*) &leader_id);
}

UwSCROVCtrModule::~UwSCROVCtrModule() {}

int UwSCROVCtrModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if (argc == 3){
		if (strcasecmp(argv[1], "setLeaderId") == 0) {
			leader_id = atoi(argv[2]);
			tcl.resultf("%s", "leader_id Setted\n");
			return TCL_OK;
		}
	}

	return UwROVCtrModule::command(argc,argv);
}

void
UwSCROVCtrModule::recv(Packet* p) {
	UwROVCtrModule::recv(p);

	Position rov_position = Position();
	rov_position.setX(x_rov);
	rov_position.setY(y_rov);
	rov_position.setZ(z_rov);

	ClMsgCtr2McPosition m(leader_id);
	m.setRovPosition(&rov_position);
	sendSyncClMsg(&m);
}

int
UwSCROVCtrModule::recvSyncClMsg(ClMessage* m)
{
	if (m->type() == CLMSG_MC2CTR_SETPOS)
	{
		Position* p = ((ClMsgMc2CtrPosition*)m)->getRovDestination();
		newX = p->getX();
		newY = p->getY();
		newZ = p->getZ();

		UwROVCtrModule::reset_retx();
		UwROVCtrModule::transmit();

		return 0;
	}
	return UwCbrModule::recvSyncClMsg(m);
}
