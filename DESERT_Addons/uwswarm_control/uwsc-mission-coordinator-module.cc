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
* @author Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the <i>UwMissionCoordinator</i> class implementation.
*
* Provides the <i>UwMissionCoordinator</i> class implementation.
*/

#include "uwsc-mission-coordinator-module.h"
#include <iostream>
#include <algorithm>

/**
* Class that represents the binding with the tcl configuration script.
*/
static class UwMissionCoordinatorModuleClass : public TclClass {
public:

	/**
	* Constructor of the class.
	*/
	UwMissionCoordinatorModuleClass() : TclClass("Plugin/UW/SC/MC") {
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
}

UwMissionCoordinatorModule::~UwMissionCoordinatorModule() {}

int
UwMissionCoordinatorModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if (argc == 2)
	{
		if (strcasecmp(argv[1], "getremovedmines") == 0)
		{
			int count = 0;

			for (auto auv : auv_follower)
				count += auv.n_mines;

			tcl.resultf("%d", count);
			return TCL_OK;
		}
	}
	else if (argc == 4)
	{
		if (strcasecmp(argv[1], "addAUV") == 0)
		{
			AUV_stats auv(atoi(argv[2]),atoi(argv[3]));

			auv_follower.emplace_back(auv);

			tcl.resultf("%d", "auv follower added\n");
			return TCL_OK;
		}
	}

	return PlugIn::command(argc,argv);
}

int
UwMissionCoordinatorModule::recvSyncClMsg(ClMessage* m)
{
	if (m->type() == CLMSG_CTR2MC_GETPOS)
	{
		int id = ((ClMsgCtr2McPosition*)m)->getSource();
		Position* p = ((ClMsgCtr2McPosition*)m)->getRovPosition();

		auto auv = std::find_if(auv_follower.begin(), auv_follower.end(),
				[id](const AUV_stats& element) {
						return element.ctr_id == id;
				});

		if (auv != auv_follower.end())
		{
			auv->rov_position = p;

			if (auv->rov_status)
			{
				auto mine = auv->rov_mine.end()-1;

				if (mine->state == Mine::MINE_TRACKED &&
					mine->track_position->getDist(p) == 0)
					mine->state = Mine::MINE_DETECTED;
			}

			if (debug_)
			{
				std::cout << NOW
						<< "  UwMissionCoordinatorModule::recvSyncClMsg()"
						<< " Received ROV (" << auv->ctr_id
						<< ") updated position X: " << auv->rov_position->getX()
						<< " Y: " << auv->rov_position->getY()
						<< " Z: " << auv->rov_position->getZ()
						<< " #mine tracked = " << auv->n_mines
						<< " rov_status = " << auv->rov_status;

				if (auv->rov_status)
					std::cout << " mine status = "
							<< auv->rov_mine[auv->n_mines-1].state;

				std::cout << std::endl;
			}

			return 0;
		}

		if (debug_)
			std::cout << NOW << "  UwMissionCoordinatorModule::recvSyncClMsg()"
					<< " no auv found with id (" << id << ")"
					<< std::endl;

	}
	else if (m->type() == CLMSG_TRACK2MC_TRACKPOS)
	{
		int id = ((ClMsgTrack2McPosition*)m)->getSource();
		Position* p = ((ClMsgTrack2McPosition*)m)->getTrackPosition();

		if (isTracked(p))
			return 0;

		auto auv = std::find_if(auv_follower.begin(), auv_follower.end(),
				[id](const AUV_stats& element) {
						return element.trk_id == id;
				});

		if (auv->rov_status)
		{
			auto mine = auv->rov_mine.end()-1;
			if (mine->track_position->getDist(p) > 0 &&
					mine->state == Mine::MINE_DETECTED)
				removeMine(auv->ctr_id);
		}

		if (auv != auv_follower.end() && !auv->rov_status)
		{
			auv->rov_mine.emplace_back(p, Mine::MINE_TRACKED);
			auv->n_mines++;
			auv->rov_status = true;

			ClMsgMc2CtrPosition msg(auv->ctr_id);
			msg.setRovDestination(p);
			sendSyncClMsg(&msg);

			if (debug_)
				std::cout << NOW
						<< "  UwMissionCoordinatorModule::recvSyncClMsg()"
						<< " ROV (" << auv->trk_id
						<< ") tracked mine at position X: " << p->getX()
						<< " Y: " << p->getY() << " Z: " << p->getZ()
						<< " #mine tracked = " << auv->n_mines
						<< " rov_status = " << auv->rov_status << std::endl;

			return 0;
		}

		if (debug_)
			std::cout << NOW << "  UwMissionCoordinatorModule::recvSyncClMsg()"
					<< " can't track mine at position X: " << p->getX()
					<< " Y: " << p->getY() << " Z: " << p->getZ()
					<< std::endl;
	}
	else if(m->type() == CLMSG_TRACK2MC_GETSTATUS)
	{
		int id = ((ClMsgTrack2McStatus*)m)->getSource();
		bool remove = ((ClMsgTrack2McStatus*)m)->getMineStatus();

		auto auv = std::find_if(auv_follower.begin(), auv_follower.end(),
				[id](const AUV_stats& element) {
						return element.trk_id == id;
				});

		if (auv != auv_follower.end() && remove)
		{
			removeMine(auv->ctr_id);

			return 0;
		}
	}

	return PlugIn::recvSyncClMsg(m);
}

void
UwMissionCoordinatorModule::removeMine(int id)
{
	auto auv = std::find_if(auv_follower.begin(), auv_follower.end(),
			[id](const AUV_stats& element) {
					return element.ctr_id == id;
			});

	if (auv != auv_follower.end() && auv->n_mines > 0)
	{
		auto mine = auv->rov_mine.end()-1;

		if(mine->state != Mine::MINE_REMOVED)
		{
			mine->state = Mine::MINE_REMOVED;
			auv->rov_status = false;

			ClMsgMc2CtrStatus msg(auv->ctr_id);
			msg.setRovStatus(auv->rov_status);
			sendSyncClMsg(&msg);

			if (debug_)
				std::cout << NOW
						<< "  UwMissionCoordinatorModule::removeMine()"
						<< " Removed mine at position"
						<< " X: " << mine->track_position->getX()
						<< " Y: " << mine->track_position->getY()
						<< " Z: " << mine->track_position->getZ() << std::endl;
		}
	}
	else
		if (debug_)
			std::cout << NOW << "  UwMissionCoordinatorModule::removeMine()"
					<< " Cannot remove mine detected by ROV (" << id << ")"
					<< std::endl;
}

bool
UwMissionCoordinatorModule::isTracked(Position* p)
{
	for (auto& auv : auv_follower)
	{
		for(auto mine : auv.rov_mine)
		{
			if (mine.track_position->getDist(p) == 0)
			{
				if (debug_)
					std::cout << NOW
							<< "  UwMissionCoordinatorModule::isTracked()"
							<< " Mine at position X: "
							<< p->getX() << " Y: " << p->getY()
							<< " Z: " << p->getZ()
							<< " is already tracked by ROV ("
							<< auv.trk_id << ")"
							<< std::endl;

				return true;
			}
		}
	}

	return false;
}
