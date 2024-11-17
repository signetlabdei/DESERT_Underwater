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
 * @file   mclinkextended.h
 * @author Nicola Toffolo
 * @version 1.1.0
 *
 * \brief Definition of MCLinkExtended class, extending MCLink.
 *
 */
 
#ifndef MCLINKEXTENDED_H
#define MCLINKEXTENDED_H

#include "mclink.h"

/**
 * \brief MCLinkExtended class stores and updates the
 *	probabilities and the channel state for UnderwaterHMMPhysicalExt
 */
class MCLinkExtended : public MCLink
{
public:
	
	/**
	 * Default constructor of MCLinkExtended class.
	 */
	MCLinkExtended();

	/**
	 * Constructor of MCLinkExtended class.
	 * 
	 * @param ber_good BER with channel in GOOD state
	 * @param ber_medium BER with channel in MEDIUM state
	 * @param ber_bad BER with channel in BAD state
	 * @param p_gb Probability of transition from GOOD to BAD in one step
	 * @param p_gm Probability of transition from GOOD to MEDIUM in one step
	 * @param p_mg Probability of transition from MEDIUM to GOOD in one step
	 * @param p_mb Probability of transition from MEDIUM to BAD in one step
	 * @param p_bg Probability of transition from BAD to GOOD in one step
	 * @param p_bm Probability of transition from BAD to MEDIUM in one step
	 * @param step_period period (s) for state transitions
	 * @param ch_state Optional initial channel state (default = GOOD)
	 *  of type MCLink::{GOOD | MEDIUM | BAD}

	 */	
	MCLinkExtended(double ber_good, double ber_medium, double ber_bad, double p_gb,
			double p_gm, double p_mg, double p_mb, double p_bg, double p_bm,
			double step_period, ChState ch_state = MCLink::GOOD);
			
	/**
	 * Default destructor of MCLinkExtended class.
	 */
	virtual ~MCLinkExtended() 
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
	virtual int command(int, const char *const *) override;

	/**
	 * Called upon packet reception, decides and returns the new channel state
	 * updates the members state and last_step.
	 * @param curr_step current step number at packet arrival
	 * @return the channel state at curr_step as 
	 *	MCLink::{GOOD | MEDIUM | BAD}
	 */
	virtual ChState updateChState() override;

	/**
	 *
	 * @return BER with current channel state
	 */
	virtual double getBER() override; //remember to call updateChState() before returning a BER value
	
protected:

		/**
	 * Auxiliary function calculating the result of a 3x3 matrix multiplication;
	 * result is stored in first argument.
	 * @param A the first factor of matrix multiplication
	 * @param B the second factor of matrix multiplication
	 */
	void mul_matrix (double (&A)[3][3], const double (&B)[3][3]);

	/**
	 * Auxiliary function raising a 3x3 matrix to the power of n.
	 * @param A the matrix to be elevated to the n-th power
	 * @param n the exponent of the operation
	 * @param R the matrix that stores the result of the operation 
	 */
	void pow_matrix (const double (&A)[3][3], int n, double (&R)[3][3]);

	// Variables
	double ber_medium; /**< BER with medium channel*/
	double p_gm; /**< Prob of transition from good to medium channel */
	double p_mg; /**< Prob of transition from medium to good channel */
	double p_mb; /**< Prob of transition from medium to bad channel */
	double p_bm; /**< Prob of transition from bad to medium channel */
	double P[3][3]; /**< Transition matrix for the channel */

};

#endif /* MCLINKEXTENDED_H  */