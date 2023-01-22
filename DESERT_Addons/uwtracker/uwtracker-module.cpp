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
* @file uwtracker-module.cc
* @author Filippo Campagnaro
* @version 1.0.0
*
* \brief Provides the <i>UWTRACKER</i> class implementation.
*
*/

#include "uwtracker-module.h"
#include "mphy_pktheader.h"
#include <iostream>
#include <limits>
#include <math.h>
#include <uwsmposition.h>
#define HDR_UWTRACK(p) (hdr_uwTracker::access(p))
extern packet_t PT_UWTRACK;
int hdr_uwTracker::offset_; /**< Offset used to access in 
									<i>hdr_uwTracker</i> packets header. */

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwTrackerModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwTrackerModuleClass() : TclClass("Module/UW/TRACKER") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwTrackerModule());
	}
} class_module_uwTrack;

UwTrackerModule::UwTrackerModule() 
	: UwCbrModule()
	, track_position(nullptr)
	, track_measure{0}
	, max_tracking_distance(std::numeric_limits<int>::max())
	, send_only_active_trace(0)
	, tracking_period(0)
	, measure_timer(this)
{
	bind("max_tracking_distance_", (double*) &max_tracking_distance);
	bind("send_only_active_trace_", (int*) &send_only_active_trace);
	bind("tracking_period_", (double*) &tracking_period);
}

UwTrackerModule::UwTrackerModule(UWSMPosition* p) 
	: UwCbrModule()
	, track_position(p)
	, max_tracking_distance(std::numeric_limits<int>::max())
	, send_only_active_trace(0)
	, track_my_position(0)
	, tracking_period(0)
	, measure_timer(this)
{	
	bind("max_tracking_distance_", (double*) &max_tracking_distance);
	bind("send_only_active_trace_", (int*) &send_only_active_trace);
	bind("track_my_position_", (int*) &track_my_position);
	bind("tracking_period_", (double*) &tracking_period);
}

UwTrackerModule::~UwTrackerModule() {}

int UwTrackerModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 3){
		if (strcasecmp(argv[1], "setTrack") == 0) {
			UWSMPosition* p = dynamic_cast<UWSMPosition*> (tcl.lookup(argv[2]));
			track_position=p;
			tcl.resultf("%s", "position Setted\n");
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setMaxTrackDistance") == 0) {
			max_tracking_distance = atof(argv[2]);
			tcl.resultf("%s", "max_tracking_distance Setted\n");
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setTrackMyPosition") == 0) {
			track_my_position = atoi(argv[2]);
			tcl.resultf("%s", "max_tracking_distance Setted\n");
			return TCL_OK;
		}
	}
	return UwCbrModule::command(argc,argv);
}

void
UwTrackerModule::sendPkt()
{
	Position track_tmp_pos;
	track_tmp_pos.setX(track_measure.x());
	track_tmp_pos.setY(track_measure.y());
	track_tmp_pos.setZ(track_measure.z());
	if(!send_only_active_trace || 
		track_tmp_pos.getDist(getPosition()) < max_tracking_distance) {
		return UwCbrModule::sendPkt();
	}
	
}

void UwTrackerModule::initPkt(Packet* p) {
	hdr_uwTracker* uw_track_h = HDR_UWTRACK(p);
	*uw_track_h = track_measure;
	if (debug_)
		std::cout << NOW << " UwTrackerModule::initPkt(Packet *p) Track current "
			<< "position: X = " << uw_track_h->x() << 
			", Y = " << uw_track_h->y() 
			<< ", Z = " << uw_track_h->z() 
			<< ", speed = " << uw_track_h->speed() << std::endl;
	if(track_my_position) {
		printReceivedPacket(p);
	}
	UwCbrModule::initPkt(p);
}

void 
UwTrackerModule::printReceivedPacket(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_uwTracker* uw_track_h = HDR_UWTRACK(p);
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_uwip *uwiph = hdr_uwip::access(p);
	if (tracefile_enabler_) {
		tracefile << NOW << " " << ch->timestamp() << " " << uwcbrh->sn() 
			<< " " << (int) uwiph->saddr() << " " << (int) uwiph->daddr() 
			<< " " << ch->size();
		Position track_tmp_pos;
		track_tmp_pos.setX(uw_track_h->x());
		track_tmp_pos.setY(uw_track_h->y());
		track_tmp_pos.setZ(uw_track_h->z());
		if(track_my_position || track_tmp_pos.getDist(ph->srcPosition) < max_tracking_distance) {			
			tracefile << " " << uw_track_h->timestamp()
				<< " " << uw_track_h->x() << " " 
				<< uw_track_h->y() << " " << uw_track_h->z() << " " 
				<< uw_track_h->speed(); 
		} else {
			tracefile << " NO TRACKS IN RANGE, distance = " 
				<< track_tmp_pos.getDist(ph->srcPosition) << " max distance = "
				<< max_tracking_distance;
		}
		tracefile <<"\n";
		tracefile.flush();
	}
}

void UwTrackerModule::updateTrackMeasure()
{
	track_measure.timestamp() = NOW;
	if(track_position) {
		track_measure.x() = track_position->getX();
		track_measure.y() = track_position->getY();
		track_measure.z() = track_position->getZ();
		track_measure.speed() = track_position->getSpeed();
	} else {
		track_measure.x() = 0;
		track_measure.y() = 0;
		track_measure.z() = 0;
		track_measure.speed() = 0;
	}
	measure_timer.resched(tracking_period); // schedule next measure
}

void UwTrackerModule::start()
{
	measure_timer.resched(tracking_period);
	UwCbrModule::start();
}

void UwTrackerModule::stop()
{
	measure_timer.force_cancel();
	UwCbrModule::stop();
}

void UwUpdateTrackMeasure::expire(Event *e) 
{
	module->updateTrackMeasure();
}
