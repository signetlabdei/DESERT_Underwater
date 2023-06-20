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
* @file uwsc-tracker-module.h
* @author Filippo Campagnaro, Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWROV</i>.
*
*/

#ifndef UWSCTRACK_MODULE_H
#define UWSCTRACK_MODULE_H
#include <uwtracker-module.h>


/**
* UwSCTrackerModule class is used to track mobile nodes via sonar and share tracking information via packets.
*/
class UwSCTrackerModule : public UwTrackerModule {
public:

	/**
	 * Default Constructor of UwSCTrackerModule class.
	 */
	UwSCTrackerModule();

	/**
	 * Constructor with position setting of UwSCTrackerModule class.
	 *
	 * @param UWSMPosition* p Pointer to the track position
	 */
	UwSCTrackerModule(UWSMPosition* p);

	/**
	 * Destructor of UwSCTrackerModule class.
	 */
	virtual ~UwSCTrackerModule();

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
	virtual void initPkt(Packet* p);

	/**
	 * recv syncronous cross layer messages to require an operation from another module
	 *
	 * @param m Pointer cross layer message
	 *
	 */
	int recvSyncClMsg(ClMessage* m);
};

#endif // UWSCTRACK_MODULE_H
