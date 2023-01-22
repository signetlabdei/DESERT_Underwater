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
 * \brief MCLink class stores and updates the probabilities 
 * and the channel state for UnderwaterHMMPhysical
 */
class MCLink : public TclObject
{
public:
	/** 
	 * enum class for channel state; in class MCLink, being a channel modeled by a
	 * 2-states Markov chain, only the states GOOD and BAD are used, while
	 * MCLinkExtended child class exploits also the intermediate MEDIUM state.
	 * The states are associated with the probabilities of successful packet
	 * reception of the nodes between which the link is in place, with the
	 * GOOD state having the highest probability and the BAD state having the
	 * worst
	 */
	enum ChState {GOOD = 1, MEDIUM = 2, BAD = 3};

	/**
	 * Default constructor of MCLink class.
	 */
	MCLink();

	/**
	 * Constructor of MCLink class.
	 * 
	 * @param p_succ_good Probability of successful reception with channel in
	 *  GOOD state
	 * @param p_succ_bad Probability of successful reception with channel in
	 *  BAD state
	 * @param p_gb Probability of transition from GOOD to BAD in one step
	 * @param p_bg Probability of transition from BAD to GOOD in one step
	 * @param ch_state Optional (default = GOOD) channel state 
	 * of type MCLink::{GOOD | BAD}
	 * @param curr_step Optional (default = 0) time step associated to the state
	 */	
	
	MCLink(double p_succ_good, double p_succ_bad,double p_gb, double p_bg,
			ChState ch_state = MCLink::GOOD, int curr_step = 0);
	
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
	virtual ChState updateChState(int curr_step);

	/**
	 *
	 * @return current channel state
	 */
	ChState getChState() const
	{
		return ch_state;
	}

	/**
	 *
	 * @return prob of successful reception with current channel state
	 */
	virtual double getPSucc() const
	{
		if (ch_state == GOOD) {
			return p_succ_good;
		} else { 
			return p_succ_bad;
		}
	}
	/**
	 *
	 * @return last step with a known channel state
	 */
	virtual int getLastStep() const
	{
		return last_step;
	}	

protected:

	// Variables
	double p_succ_good; /**< Prob of successful reception with good channel*/
	double p_succ_bad; /**< Prob of successful reception with bad channel*/
	double p_gb; /**< Prob of transition from good to bad channel */
	double p_bg; /**< Prob of transition from bad to good channel */
	ChState ch_state; /**< last channel state */
	int last_step; /**< last time step associate to channel state */

};

#endif /* MCLINK_H  */