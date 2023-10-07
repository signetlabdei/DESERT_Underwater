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
* @file uwsc-tracker-follower-module.cc
* @author Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the <i>UWSCFTRACKER</i> class implementation.
*
* Provides the UwSFCTracker class implementation.
*/

#include "uwsc-tracker-follower-module.h"
#include <uwsmposition.h>
#include <iostream>

extern packet_t PT_UWSCFTRACK;
int hdr_uwSCFTracker::offset_; /**< Offset used to access in 
									<i>hdr_uwTracker</i> packets header. */

/**
* Class that represents the binding with the tcl configuration script.
*/
static class UwSCFTrackerModuleClass : public TclClass {
public:

	/**
	* Constructor of the class
	*/
	UwSCFTrackerModuleClass() : TclClass("Module/UW/SC/TRACKERF") {
	}

	/**
	* Creates the TCL object needed for the tcl language interpretation.
	* @return Pointer to an TclObject
	*/
	TclObject* create(int, const char*const*) {
		return (new UwSCFTrackerModule());
	}
} class_module_uwSCFTracker;

UwSCFTrackerModule::UwSCFTrackerModule()
	: UwTrackerModule()
	, mine_positions()
	, auv_position()
	, auv_state()
	, demine_period(0)
	, mine_measure{0}
	, mine_timer(this)
{
	bind("demine_period_", (double*) &demine_period);
}


UwSCFTrackerModule::~UwSCFTrackerModule() {}

int UwSCFTrackerModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();

	if(argc == 3){
		if (strcasecmp(argv[1], "setTrack") == 0) {
			UWSMPosition* p = dynamic_cast<UWSMPosition*> (tcl.lookup(argv[2]));

			if(p)
			{
				mine_positions.emplace_back(p);
				track_position = p;
				tcl.resultf("%s", "position Setted\n");
				return TCL_OK;
			}

			tcl.resultf("%s", "position error\n");
			return TCL_ERROR;
		}
	}

	return UwTrackerModule::command(argc,argv);
}

void
UwSCFTrackerModule::sendPkt()
{
	UwTrackerModule::sendPkt();

	if (auv_state == FollowerState::DETECT && mine_positions.size() > 1)
	{
		Position temp_position;
		temp_position.setX(track_measure.x());
		temp_position.setY(track_measure.y());
		temp_position.setZ(track_measure.z());

		for (auto it = mine_positions.begin(); it != mine_positions.end();++it)
	    {
			if((*it)->getDist(&temp_position) < 0.001)
			{
				if (it == mine_positions.end()-1)
				{
					track_position = *(--it);
					mine_positions.erase(++it);
				}
				else
				{
					track_position = *(++it);
					mine_positions.erase(--it);
				}
				break;
			}
	    }

		mine_measure.mine_remove() = false;

		auv_state = FollowerState::DEMINE;
	}

	if (auv_state == FollowerState::DETECT && mine_positions.size() <= 1)
	{
		mine_measure.mine_remove() = false;
		auv_state = FollowerState::IDLE;
	}

	mine_timer.resched(tracking_period);
}

void
UwSCFTrackerModule::initPkt(Packet* p)
{
	mine_measure.x() = track_measure.x();
	mine_measure.y() = track_measure.y();
	mine_measure.z() = track_measure.z();

	hdr_uwSCFTracker* uwscf_track_h = HDR_UWSCFTRACK(p);
	*uwscf_track_h = mine_measure;

	UwTrackerModule::initPkt(p);
}

void
UwSCFTrackerModule::updateMineRemove()
{
	if (auv_state == FollowerState::TRACK &&
			auv_position.getX() == track_measure.x() &&
			auv_position.getY() == track_measure.y() &&
			auv_position.getZ() == track_measure.z())
	{
		mine_measure.mine_remove() = true;

		auv_state = FollowerState::DETECT;

		mine_timer.resched(demine_period);
		sendTmr_.resched(demine_period);
		measure_timer.resched(demine_period);

		if (debug_)
			std::cout << NOW << "UwSCFTrackerModule::updateMineRemove()"
					<< " Mine at position: X = " << track_measure.x()
					<< ", Y = " << track_measure.y()
					<< ", Z = " << track_measure.z()
					<< " is detected" << std::endl;
	}

	auv_position.setX(getPosition()->getX());
	auv_position.setY(getPosition()->getY());
	auv_position.setZ(getPosition()->getZ());

	if (auv_state == FollowerState::DEMINE)
		auv_state = FollowerState::TRACK;

	if (auv_state != FollowerState::DETECT &&
			auv_state != FollowerState::IDLE)
	{
		track_position = updateTrackPosition();
		mine_timer.resched(tracking_period);
	}
}

UWSMPosition* 
UwSCFTrackerModule::updateTrackPosition()
{
	UWSMPosition* new_track_position (track_position);
	float min_distance = new_track_position->getDist(&auv_position);

	mine_measure.timestamp() = NOW;

	for (auto& pos : mine_positions)
	{
		float distance = pos->getDist(&auv_position);
		if(distance < min_distance)
		{
			min_distance = distance;
			new_track_position = pos;
		}
	}

	if (debug_)
		std::cout << NOW << " UwSCFTrackerModule::updateTrackPosition()"
				<< "New track position: X = " << new_track_position->getX()
				<< " Y = " << new_track_position->getY()
				<< " Z = " << new_track_position->getZ()
				<< std::endl;

	return new_track_position;
}

void
UwSCFTrackerModule::start()
{
	auv_position.setX(getPosition()->getX());
	auv_position.setY(getPosition()->getY());
	auv_position.setZ(getPosition()->getZ());

	mine_timer.resched(tracking_period);

	UwTrackerModule::start();
}

void
UwSCFTrackerModule::stop()
{
	mine_timer.force_cancel();
	UwTrackerModule::stop();
}

void
UwUpdateMineStatus::expire(Event *e) 
{
	module->updateMineRemove();
}
