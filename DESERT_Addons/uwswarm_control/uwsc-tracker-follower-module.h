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
* @author Filippo Campagnaro, Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWSCTRACKER</i>.
*
* Provides the definition of the class UwSCTracker.
*/

#ifndef UWTRACKF_MODULE_H
#define UWTRACKF_MODULE_H
#include <uwtracker-module.h>
#include <uwsc-tracker-follower-packet.h>
#include <node-core.h>

#define HDR_UWSCFTRACK(p) (hdr_uwSCFTracker::access(p))

class UwSCFTrackerModule; // forward declaration

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
* UwTrackerModule class is used to track mobile nodes via sonar and share tracking information via packets.
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
	* Initializes a monitoring data packet passed as argument with the default values.
	* 
	* @param Packet* Pointer to a packet already allocated to fill with the right values.
	*/
	virtual void initPkt(Packet* p) ;

protected:
	Position auv_position;
	double demine_period;
	hdr_uwSCFTracker mine_measure;
	UwUpdateMineStatus mine_timer; /**< timer to schedule tracking measurements*/

	/**
	* Allocates, initialize and sends a packet with the default priority flag
	* set from tcl.
	*
	*/
	void sendPkt();

	void updateMineRemove();

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
