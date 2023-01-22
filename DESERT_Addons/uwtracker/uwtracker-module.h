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
* @file uwtracker-module.h
* @author Filippo Campagnaro
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWROV</i>.
*
* Provides the definition of the class <i>UWROV</i>, based on <i>UwCbr</i>.
* <i>UWROV</i> can manage no more than 2^16 packets. If a module generates more
* than 2^16 packets, they will be dropped, according with <i>UwCbr</i>.
* <i>UWROV</i> sends periodically monitoring packets containing information about
* the current position and acknowledges the last control packet received.
* Each control packet contains the next waypoint that has to be reach.
*/

#ifndef UWTRACK_MODULE_H
#define UWTRACK_MODULE_H
#include <uwcbr-module.h>
#include <uwtracker-packet.h>

class UwTrackerModule; // forward declaration
class UWSMPosition;  // forward declaration

class UwUpdateTrackMeasure : public TimerHandler
{
public:
	UwUpdateTrackMeasure(UwTrackerModule *m)
		: TimerHandler(), module(m)
	{

	}

protected:
	virtual void expire(Event *e);
	UwTrackerModule *module;
};

/**
* UwTrackerModule class is used to track mobile nodes via sonar and share tracking information via packets.
*/
class UwTrackerModule : public UwCbrModule {
	friend class UwUpdateTrackMeasure;
public:

	/**
	 * Default Constructor of UwTrackerModule class.
	 */
	UwTrackerModule();

	/**
	 * Constructor with position setting of UwTrackerModule class.
	 *
	 * @param UWSMPosition* p Pointer to the track position
	 */
	UwTrackerModule(UWSMPosition* p);

	/**
	 * Destructor of UwTrackerModule class.
	 */
	virtual ~UwTrackerModule();

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
	 */
	virtual void initPkt(Packet* p) ;

protected:

	UWSMPosition* track_position; /**< Track position.*/
	hdr_uwTracker track_measure; /**< Track position.*/

	double max_tracking_distance; /**< Maximum tracking distance, in [m]*/

	int send_only_active_trace; /**< send only active trace*/
	int track_my_position; /**< track also my position*/
	double tracking_period; /**< period between tracking measurements*/
	UwUpdateTrackMeasure measure_timer; /**< timer to schedule tracking measurements*/
	/**
	 * Print to tracefile details about a received packet
	 *
	 * @param Packet* Pointer to the received packet 
	 */
	virtual void printReceivedPacket(Packet *p);

	/**
	 * Allocates, initialize and sends a packet with the default priority flag
	 * set from tcl.
	 *
	 */
	void sendPkt();

	/**
	 * Update the track measure
	 *
	 */
	void updateTrackMeasure();

	/**
	 * Start to send packets.
	 */
	virtual void start();

	/**
	 * Stop to send packets.
	 */
	virtual void stop();
};

#endif // UWROV_MODULE_H
