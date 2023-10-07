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
* @file uwsc-tracker-follower-module.h
* @author Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWSCFTRACKER</i>.
*
* Provides the definition of the class UwSCFTracker.
*/

#ifndef UWTRACKF_MODULE_H
#define UWTRACKF_MODULE_H
#include <uwtracker-module.h>
#include <uwsc-tracker-follower-packet.h>
#include <node-core.h>
#include <vector>
#include <algorithm>

#define HDR_UWSCFTRACK(p) (hdr_uwSCFTracker::access(p))

class UwSCFTrackerModule; // forward declaration

/**
 * UwUpdateMineStatus class is used to handle the scheduling period of <i>UWSCFTRACKER</i> packets.
 */
class UwUpdateMineStatus : public TimerHandler
{
public:
	UwUpdateMineStatus(UwSCFTrackerModule *m)
		: TimerHandler()
		, module(m)
	{
	}

protected:
	virtual void expire(Event *e);
	UwSCFTrackerModule *module;
};

/**
 * Followerstate list all the possible state of the follower.
 */
enum class FollowerState
{
	TRACK,	/**< Tracking mines. */
	DETECT,	/**< Detecting a mine. */
	DEMINE,	/**< Removed a mine. */
	IDLE	/**< Not tracking. */
};

/**
 * UwSCFTrackerModule class is used to track mines via sonar and share tracking information via packets.
 * After a given time a mine is detected, this module provides to remove it.
 */
class UwSCFTrackerModule : public UwTrackerModule {
	friend class UwUpdateMineStatus;
public:

	/**
	 * Default Constructor of UwSCFTrackerModule class.
	 */
	UwSCFTrackerModule();


	/**
	 * Destructor of UwSCFTrackerModule class.
	 */
	virtual ~UwSCFTrackerModule();


	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * 
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
	 * 
	 */
	virtual int command(int argc, const char*const* argv);

	/**
	 * Initializes a monitoring data packet passed as argument with the default values.
	 * 
	 * @param Packet* Pointer to a packet already allocated to fill with the right values.
	 *
	 */
	virtual void initPkt(Packet* p) ;

protected:
	std::vector<UWSMPosition*> mine_positions; /**< Positions of the mines in the area. */
	Position auv_position; /**< Current position of the follower. */
	FollowerState auv_state; /**< Current state of the follower. */
	double demine_period; /**< Timer to schedule packets transmission.*/
	hdr_uwSCFTracker mine_measure; /**< Detected mine packets. */
	UwUpdateMineStatus mine_timer; /**< Timer to schedule detecting measurements*/

	/**
	 * Allocates, initialize and sends a packet with the default priority flag
	 * set from tcl.
	 *
	 */
	void sendPkt();

	/**
	 * Update the mine measure
	 */
	void updateMineRemove();

	/**
	  * Update the current track position with the nearest mine position
	  *
	  * @return UWSMPosition Pointer to the nearest mine position
	  */
	UWSMPosition* updateTrackPosition();

	/**
	 * Start to send packets.
	 */
	virtual void start();

	/**
	 * Stop to send packets.
	 */
	virtual void stop();
};

#endif // UWTRACKF_MODULE_H
