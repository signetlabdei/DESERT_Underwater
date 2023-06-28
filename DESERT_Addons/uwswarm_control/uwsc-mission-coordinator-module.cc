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

/**
* Class that represents the binding with the tcl configuration script 
*/
static class UwMissionCoordinatorModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwMissionCoordinatorModuleClass() : TclClass("Module/UW/SC/MC") {
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
	: PlugIn()
	, auv_follower()
{
	UWSMPosition lp = UWSMPosition();
	leader_position=&lp;
}

UwMissionCoordinatorModule::UwMissionCoordinatorModule(UWSMPosition* p) 
	: PlugIn()
	, leader_position(p)
	, auv_follower()
{
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
		} else if (strcasecmp(argv[1], "setAUVId") == 0) {
			AUV_stats auv(atoi(argv[2]));
			auv_follower.emplace_back(auv);
			tcl.resultf("%d", "auv follower added\n");
			return TCL_OK;
		} else if (strcasecmp(argv[1], "removeMine") == 0) {
			removeMine(atoi(argv[2]));
			tcl.resultf("%d", "mine removed\n");
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
	return PlugIn::command(argc,argv);
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
		Position* p = ((ClMsgCtr2McPosition*)m)->getRovPosition();

		for (auto& auv : auv_follower)
		{
			if (auv.ctr_id == id)
			{
				auv.rov_position = p;

				int n = auv.n_mines;
				if (auv.busy && auv.rov_mine[n-1].state == MINE_TRACKED)
					auv.rov_mine[n-1].state = MINE_DETECTED;

				if (debug_)
					std::cout << NOW << "  UwMissionCoordinatorModule:recvSyncClMsg()"
						<< " received rov " << auv.ctr_id
						<< " new position: " << auv.rov_position 
						<< " number of mine tracked: " << auv.n_mines
						<< " detecting a mine: " << auv.busy << std::endl;
				return 0;
			}
		}

		if (debug_)
			std::cout << NOW << "  UwMissionCoordinatorModule:recvSyncClMsg()"
				<< " no rov found with id " << id << std::endl;
	}
	else if (m->type() == CLMSG_TRACK2MC_TRACKPOS)
	{
		int id = ((ClMsgCtr2McPosition*)m)->getSource();
		Position* p = ((ClMsgTrack2McPosition*)m)->getTrackPosition();
		
		for (auto& auv : auv_follower)
		{
			if (auv.rov_position->getX() == p->getX() &&
					auv.rov_position->getY() == p->getY() &&
					auv.rov_position->getZ() == p->getZ())
			{
				if (debug_)
					std::cout << NOW << "  UwMissionCoordinatorModule:recvSyncClMsg()"
						<< " Mine at position X: " 
						<< p->getX() << " Y: " << p->getY()
						<< " Z: " << p->getZ()
						<< " is already tracket by rov " << auv.ctr_id 
						<< std::endl;

				return 0;
			}
		}

		for (auto& auv : auv_follower)
		{
			if (auv.ctr_id == id)
			{
				auv.rov_mine.emplace_back(p, MINE_TRACKED);
				auv.n_mines++;
				auv.busy = true;

				if (debug_)
					std::cout << NOW << "  UwMissionCoordinatorModule:recvSyncClMsg()"
						<< " rov " << auv.ctr_id 
						<< " tracked a mine at position X: " 
						<< p->getX() << " Y: " << p->getY()
						<< " Z: " << p->getZ()
						<< " ,number of mine tracked: " << auv.n_mines
						<< " ,detecting a mine: " << auv.busy << std::endl;

				ClMsgMc2CtrPosition msg(auv.ctr_id);
				msg.setRovDestination(p);
				sendSyncClMsg(&msg);

				return 0;
			}
		}

		if (debug_)
			std::cout << NOW << "  UwMissionCoordinatorModule:recvSyncClMsg()"
				<< " no rov found with id " << id << std::endl;
	}

	return PlugIn::recvSyncClMsg(m);
}

void
UwMissionCoordinatorModule::removeMine(int id)
{
	for (auto& auv : auv_follower)
	{
		if (auv.ctr_id == id)
		{
			int n = auv.n_mines;
			auv.rov_mine[n-1].state = MINE_REMOVED;
			auv.busy = false;

			ClMsgMc2CtrStatus msg(auv.ctr_id);
			msg.setRovStatus(auv.busy);
			sendSyncClMsg(&msg);

			if (debug_)
				std::cout << NOW << "  UwMissionCoordinatorModule:removeMine()"
					<< " mine detected by rov " << auv.ctr_id
					<< " at " << auv.rov_position
					<< " has been removed " << std::endl;

			break;
		}
	}

	if (debug_)
		std::cout << NOW << "  UwMissionCoordinatorModule:removeMine()"
			<< " no rov found with id " << id << std::endl;
}
