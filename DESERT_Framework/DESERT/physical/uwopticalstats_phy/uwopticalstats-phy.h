//
// Copyright (c) 2024 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
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
 * @file   uwopticalstats-phy.h
 * @author Filippo Donega'
 * @version 1.0.0
 *
 * \brief Definition of the UwOpticalStatsPhy class.
 */

#ifndef UWOPTICALSTATS_PHY_H
#define UWOPTICALSTATS_PHY_H

#include <uwoptical-phy.h>

#include "clmsg-stats.h"
#include <phymac-clmsg.h>

#include <rng.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>

#include <iostream>
#include <string.h>
#include <cmath>
#include <limits>
#include <climits>
#include <math.h>
#include <node-core.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#define OPTICAL_MODULATION_TYPE "OPTIC_BPSK"
#define NOT_FOUND_VALUE 0
#define IL_ILLEGAL -1

typedef ::std::map<double, double> DepthMap;
typedef DepthMap::iterator DMIt;


class UwOpticalStats : public Stats
{
public:
	/**
	 * Constructor of UwOpticalStats class
	 */
	UwOpticalStats();

	/**
	 *	Constructor of UwOpticalStats class
	 */
	UwOpticalStats(int mod_id, int stck_id);

	/**
	 * Destructor of UwOpticalStats class
	 */
	virtual ~UwOpticalStats()
	{
	}

	/**
	 * Virtual method used by the Module class in order to copy its stats an a generic fashion,
	 * without the need to know the derived stats implementation.
	 * @return the copy of a module the stats.
  	**/
	virtual Stats* clone() const;

	/**
	 * Method to update stats with the param of last received packet
	 * @param rx_pwr, received power of the packet
	 * @param noise_pwr, noise power
	 * @param interf_pwr, noise power
	 * @param error, true if packet has error
	 */
	enum ChannelStates {NOT_DEFINED = -1, GOOD = 1, BAD = 2};
	virtual void updateStats(int mod_id, int stck_id, double rx_pwr, 
		double noise_pwr, double interf_pwr, bool error, double T, 
		ChannelStates channel_state = NOT_DEFINED);

	double last_rx_power;       /** Power of the last received packet.*/
	double last_noise_power;    /** Noise power of the last received packet. */
	double instant_noise_power; /** Noise power measured on query. */
	double last_interf_power;   /** Interference power of the last rx packet.*/
	bool has_error;             /** True if last packet wasn't correctly received. */
	double temperature;			/** Temperature from UwOptical. */
	ChannelStates channel_state {NOT_DEFINED}; /** used by UwHMMOptical */
};

class UwOpticalStatsPhy : public UwOpticalPhy
{

public:
	/**
	 * Constructor of UwMultiPhy class.
	 */
	UwOpticalStatsPhy();

	/**
	 * Destructor of UwMultiPhy class.
	 */
	virtual ~UwOpticalStatsPhy();

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not.
	 *
	 */
	virtual int command(int, const char *const *);
	/**
	* Update the stats before sending them through crosslayer message
	*/
	virtual void updateInstantaneousStats();
	/*
	* Function used to bind the Threshold value inputted from TCL
	*/
	inline virtual void setThreshold(int threshold) {this->threshold = threshold;};

	virtual void startRx(Packet *p);

	virtual void endRx(Packet *p);
	/**
	* recv syncronous cross layer messages to require an operation from another module
	*
	* @param m Pointer cross layer message
	*
	*/

	virtual void endTx(Packet *p);

	virtual int recvSyncClMsg(ClMessage* m);

private:
	int threshold; //Threshold for some purpose
};

#endif /* UWOPTICALSTATS_PHY_H  */