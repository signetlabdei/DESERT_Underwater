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
* @author  Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UwMissionCooridnator</i>.
*
* This module is used to coordinates a swarm of AUV with a node leader and node
* followers which perform an underwater mine detection.
* The leader receives tracking information from the followers  and decides
* where they have to go.
*/

#ifndef UWMC_MODULE_H
#define UWMC_MODULE_H
#include "uwsc-clmsg.h"
#include <uwsmposition.h>
#include <plugin.h>
#include <tclcl.h>
#include <vector>

class UwMissionCoordinatorModule;

/**
 * Mine describes a mine by its position and state.
 */
typedef struct Mine
{
	/**
	 * MineState list all the possible state of a mine.
	 */
	enum MineState
	{
		MINE_TRACKED,	/**< Mine tracked.*/
		MINE_DETECTED,	/**< Mine found and started the removing process */
		MINE_REMOVED	/**< Mine removed */
	};

	Position* track_position;	/**< Mine tracked position*/
	MineState state;			/**< Mine state */

	Mine(Position* p, MineState s)
		: track_position(p)
		, state(s)
	{
	}
} Mine;

/**
 * AUV_stats describes statistics about the AUV follower.
 * It also contains ids of respectively the ROV controller
 * and Tracker receiver module installed in the AUV leader.
 */
typedef struct AUV_stats
{
	int ctr_id;	/**< Id of the CTR module. */
	int trk_id;	/**< Id of the Tracker module. */
	Position* rov_position;			/**< Position of ROV follower. */
	std::vector<Mine> rov_mine;	/** Mines found by ROV follower. */
	int n_mines;		/** Number of mines found by ROV follower. */
	bool rov_status;	/** Status of the ROV, if true is detecting a mine. */

	/**
	* Constructor of AUV_stats struct.
	*/
	AUV_stats(int id_ctr, int id_trk)
		: ctr_id(id_ctr)
		, trk_id(id_trk)
		, rov_position(nullptr)
		, rov_mine()
		, n_mines(0)
		, rov_status(false)
	{
	}

} AUV_stats;

/**
* UwMissionCoordinatorModule class is used to manage AUV followers and to
* collect statistics about them.
*/
class UwMissionCoordinatorModule : public PlugIn {
public:

	/**
	 * Default Constructor of UwMissionCoordinatorModule class.
	 */
	UwMissionCoordinatorModule();

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
	 * Recv syncronous cross layer messages to require an operation from another module.
	 *
	 * @param m Pointer cross layer message
	 * @return zero if successful
	 *
	 */
	int recvSyncClMsg(ClMessage* m);


protected:
	std::vector<AUV_stats> auv_follower;	/**< ROV followers info. */

	/**
	 * Send a signal to the AUV follower to inform it, that the mine
	 * it is detecting is removed.
	 *
	 * @param int id of the ROV sent to detect the mine
	 *
	 */
	void removeMine(int id);

	/**
	 * Check if the mine at received position is already tracked.
	 *
	 * @param p Pointer to mine position
	 * @return bool true if the mine is already tracked false otherwise
	 *
	 */
	bool isTracked(Position* p);
};

#endif // UWMC_MODULE_H
