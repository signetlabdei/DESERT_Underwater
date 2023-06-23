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
* @file uwmc-module.h
* @author Filippo Campagnaro, Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWMC</i>.
*
*/

#ifndef UWMC_MODULE_H
#define UWMC_MODULE_H
#include "uwsc-clmsg.h"
#include <uwsmposition.h>
#include <module.h>
#include <tclcl.h>
#include <vector>

/**
 * MineState list all the possible state of a mine
 */
enum MineState
{
	MINE_TRACKED, /**< Mine tracked.*/
	MINE_DETECTED,/**< Mine found and started the removing process */
	MINE_REMOVED /**< Mine removed */
};

/**
 * Mines describes a mine entry
 */
typedef struct Mines
{
	Position* track_position;	/**< Mine tracked position*/
	MineState state;	/**< Mine state */

	Mines(Position* p, MineState s)
		: track_position(p)
		, state(s)
	{
	}
} Mines;

/**
 * AUV_stats describe an AUV controlled by the leader
 */
typedef struct AUV_stats
{
	int ctr_id;	/**< Id of the ROV Ctr */
	Position* rov_position;		/**< Position of the ROV follower */
	std::vector<Mines> rov_mine;	/** Mines found by this ROV */
	int n_mines;	/** Number of mines found by this ROV */
	bool busy;	/** Status of the ROV **/

	AUV_stats(int id)
		: ctr_id(id)
		, n_mines(0)
		, busy(false)
	{
		Position p = Position();
		rov_position = &p;
	}

} AUV_stats;

/**
* UwMissionCoordinatorModule class is used to manage AUV followers and to collect statistics about them.
*/
class UwMissionCoordinatorModule : public PlugIn {
public:

	/**
	* Default Constructor of UwMissionCoordinatorModule class.
	*/
	UwMissionCoordinatorModule();

	/**
	* Constructor with position setting of UwMissionCoordinatorVModule class.
	*
	* @param UWSMPosition* p Pointer to the ROV leader position
	*/
	UwMissionCoordinatorModule(UWSMPosition* p);

	/**
	* Destructor of UwMissionCoordinatorModule class.
	*/
	virtual ~UwMissionCoordinatorModule();

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
	* Set the position of the ROV leader
	*
	* @param Position * p Pointer to the ROV leader position
	*/
	virtual void setPosition(UWSMPosition* p);

	/**
	* Returns the position of the ROV leader
	*
	* @return the current ROV leader position
	*/
	inline UWSMPosition* getPosition() { return leader_position;}

	/**
	 * recv syncronous cross layer messages to require an operation from another module
	 *
	 * @param m Pointer cross layer message
	 *
	 */
	int recvSyncClMsg(ClMessage* m);


protected:
	UWSMPosition* leader_position;	/**< ROV leader position */
	std::vector<AUV_stats> auv_follower;	/**< ROV followers info */

	void removeMine();
};

#endif // UWMC_MODULE_H
