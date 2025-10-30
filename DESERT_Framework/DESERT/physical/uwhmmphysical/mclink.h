//
// Copyright (c) 2021 Regents of the SIGNET lab, University of Padova.
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
 * @file   mclink.h
 * @author Antonio Montanari
 * @version 1.1.0
 *
 * \brief Definition of MCLink class.
 *
 */

#ifndef MCLINK_H
#define MCLINK_H

#include "uwphysical.h"

/**
 * \brief MCLink class models the BER of a directed link between two nodes.
 * The base class allows for GOOD/BAD channel states only, while the extended
 * McLinkExtended class allows for MEDIUM state as well.
 * Every state is associated with a different channel BER and
 * every step_period the state is updated according to the transition
 * probabilities.
 */
class MCLink : public TclObject
{
public:
	/**
	 * Channel state
	 */
	enum ChState { NOT_DEFINED = 0, GOOD = 1, MEDIUM = 2, BAD = 3 };

	/**
	 * Default constructor of MCLink class.
	 */
	MCLink();

	/**
	 * Constructor of MCLink class.
	 *
	 * @param ber_good BER with channel in GOOD state
	 * @param ber_bad BER with channel in BAD state
	 * @param p_gb Probability of transition from GOOD to BAD in one step
	 * @param p_bg Probability of transition from BAD to GOOD in one step
	 * @param step_period period (s) for channel transition between states
	 * @param ch_state Optional (default = GOOD) initial channel state
	 * @return a new MCLink object
	 */

	MCLink(double ber_good, double ber_bad, double p_gb, double p_bg,
			double step_period, ChState ch_state = MCLink::GOOD);

	/**
	 * Default destructor of MCLink class.
	 */
	virtual ~MCLink()
	{
	}

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
	 * Called upon packet reception, decides and returns the new channel state
	 * updates the members state and last_step.
	 * @param curr_step current step number at packet arrival
	 * @return the channel state at curr_step as MCLink::{GOOD | BAD}
	 */
	virtual ChState updateChState();

	/**
	 *
	 * @return current (updated) channel state
	 */
	ChState
	getChState()
	{
		return updateChState();
	}

	/**
	 * @return BER with current (updated) channel state
	 */
	virtual double
	getBER(); // remember to call updateChState() before returning a BER value

protected:
	// Variables
	double ber_good; /**< BER with good channel*/
	double ber_bad; /**< BER with bad channel*/
	double p_gb; /**< Prob of transition from good to bad channel */
	double p_bg; /**< Prob of transition from bad to good channel */
	double last_update; /**< last time channel state has been updated */
	double step_period; /**< period (s) for channel transition between states */
	ChState ch_state; /**< last channel state */
};

#endif /* MCLINK_H  */