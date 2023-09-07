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
//
/**
* @file uwsc-rovctr-module.h
* @author Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWSCROVCTR</i>.
*
* Provides the definition of the class UwSCROVCtr.
* UwSCROVCTR receives cross layer messages containing the next waypoint that has to be sent to the associated node follower. In addition it receives monitoring packets containing the current node folllower position.
* SCROVCtr stands for Swarm control ROV Controller.
*/


#ifndef UWSCROV_CTR_MODULE_H
#define UWSCROV_CTR_MODULE_H
#include <uwrovctr-module.h>
#include "uwsc-clmsg.h"

/**
* UwSCROVCtrModule class adds to the UwROVCtrModule class the possibility
* to receive and send cross layer messages.
*
*/
class UwSCROVCtrModule : public UwROVCtrModule {
public:
	/**
	* Constructor of UwSCROVCtrModule class.
	*/
	UwSCROVCtrModule();

	/**
	* Destructor of UwSCROVCtrModule class.
	*/
	virtual ~UwSCROVCtrModule();

	/**
	* TCL command interpreter. It implements the following OTcl methods:
	*
	* @param argc Number of arguments in <i>argv</i>.
	* @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
	* @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
	*
	**/
	virtual int command(int argc, const char*const* argv);

	/**
	* Performs the reception of packets from upper and lower layers.
	*
	* @param Packet* Pointer to the packet will be received.
	*/
	virtual void recv(Packet*);


	/**
	 * Recv syncronous cross layer messages to require an operation from another module.
	 *
	 * @param m Pointer cross layer message
	 * @return zero if successful
	 *
	 */
	int recvSyncClMsg(ClMessage* m);

protected:
	int leader_id;		/**< Id of the Mission Coordinator module. */
	bool rov_status;	/**< Status of the ROV follower, true if detected a mine */
};

#endif // UWSCROVCtr_MODULE_H
