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
 * @file   mclinkextended.cpp
 * @author Nicola Toffolo
 * @version 1.0.0
 *
 * \brief Implementation of MCLinkExtended class
 *
 */

#include <string.h>
#include <rng.h>
#include <tclcl.h>
#include <iostream>
#include <array>
#include "mclinkextended.h"

static class MCLinkExtendedClass : public TclClass
{
public:
	MCLinkExtendedClass()
		: TclClass("Module/UW/HMMPHYSICAL/MCLINK/EXTENDED")
	{
	}
	TclObject *
	create(int argc, const char *const * argv)
	{
		if (argc > 12) {
			if (argc == 13) {
				return (new MCLinkExtended(std::stod(argv[4]), std::stod(argv[5]), 
						std::stod(argv[6]),	std::stod(argv[7]), std::stod(argv[8]),
						std::stod(argv[9]), std::stod(argv[10]), std::stod(argv[11]),
						std::stod(argv[12])));
			}  
			if (argc == 14 || argc == 15) {
				MCLinkExtended::ChState ch_state;
				if (strcasecmp(argv[13], "GOOD") == 0) {
					ch_state = MCLinkExtended::GOOD;
				} else if (strcasecmp(argv[13], "MEDIUM") == 0) {
					ch_state = MCLinkExtended::MEDIUM;
				} else if (strcasecmp(argv[13], "BAD") == 0) {
					ch_state = MCLinkExtended::BAD;
				} else {
					std::cerr << "TCL: MCLinkExtended state must be GOOD, MEDIUM or BAD"
					<< std::endl;
					exit(1);
				}
				if (argc == 14) {
					return (new MCLinkExtended(std::stod(argv[4]), std::stod(argv[5]), 
							std::stod(argv[6]),	std::stod(argv[7]), std::stod(argv[8]),
							std::stod(argv[9]), std::stod(argv[10]), std::stod(argv[11]),
							std::stod(argv[12]), ch_state));
				}
				if (argc == 15) {
					return (new MCLinkExtended(std::stod(argv[4]), std::stod(argv[5]), 
							std::stod(argv[6]),	std::stod(argv[7]), std::stod(argv[8]),
							std::stod(argv[9]), std::stod(argv[10]), std::stod(argv[11]),
							std::stod(argv[12]), ch_state, std::stoi(argv[14])));
				}
			}
			std::cerr << "TCL: check MCLinkExtended constructor args" << std::endl;
			exit(1);
		}

		return (new MCLinkExtended);
	}
} class_module_mclinkextended;

MCLinkExtended::MCLinkExtended() 
	: MCLink()
	, p_succ_medium(0.0)
	, p_gm(0.0)
	, p_mg(0.0)
	, p_mb(0.0)
	, p_bm(0.0)
	, P{{1.0 - p_gm - p_gb, p_gm, p_gb}, {p_mg, 1.0 - p_mg - p_mb, p_mb},
			{p_bg, p_bm, 1.0 - p_bg - p_bm}}
{
}

MCLinkExtended::MCLinkExtended(double p_succ_good, double p_succ_medium,
		double p_succ_bad, double p_gb, double p_gm, double p_mg, double p_mb, double p_bg,
		double p_bm, ChState ch_state, int curr_step)
	: MCLink(p_succ_good, p_succ_bad, p_gb, p_bg, ch_state, curr_step)
	, p_succ_medium(p_succ_medium)
	, p_gm(p_gm)
	, p_mg(p_mg)
	, p_mb(p_mb)
	, p_bm(p_bm)
	, P{{1.0 - p_gm - p_gb, p_gm, p_gb}, {p_mg, 1.0 - p_mg - p_mb, p_mb},
			{p_bg, p_bm, 1.0 - p_bg - p_bm}}
{
	assert(p_succ_good >=0.0 && p_succ_good <= 1.0 && 
			p_succ_medium >= 0.0 && p_succ_medium <= 1.0 &&
			p_succ_bad >= 0.0 && p_succ_bad <= 1.0 && 
			p_gb >= 0.0 && p_gb <= 1.0 && p_gm >= 0.0 && p_gm <= 1.0 &&
			p_mg >= 0.0 && p_mg <= 1.0 && p_mb >= 0.0 && p_mb <= 1.0 &&
			p_bg >= 0.0 && p_bg <= 1.0 && p_bm >= 0.0 && p_bm <= 1.0);
}

void
MCLinkExtended::mul_matrix (double (&A)[3][3], const double (&B)[3][3])
{
	double C[3][3] = {0};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				C[i][j] = (C[i][j] + A[i][k] * B[k][j]);
			}
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			A[i][j] = C[i][j];
		}
	}
}

void
MCLinkExtended::pow_matrix(const double (&A)[3][3], int n, double (&R)[3][3])
{
	double B[3][3];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			B[i][j] = A[i][j];
			R[i][j] = A[i][j];
		}
	}
	
	n--;
	while (n > 0)
	{
		if (n % 2) {
			mul_matrix(R, const_cast<double (&)[3][3]>(B));
		}
		mul_matrix(B, const_cast<double (&)[3][3]>(B));
		n = n >> 1;  
	}
}

MCLink::ChState
MCLinkExtended::updateChState(int curr_step)
{
	int n_step = curr_step - last_step;
	
	double P_n[3][3] = {0.0};
	pow_matrix(const_cast<double (&)[3][3]>(P), n_step, P_n);
	
	if (ch_state == MCLinkExtended::GOOD) {
		double new_p_gg = P_n[0][0];
		double new_p_gm = P_n[0][1];
		double rng = RNG::defaultrng()->uniform_double();
		if (rng > new_p_gg + new_p_gm) {
			ch_state = MCLinkExtended::BAD;
		} else if (rng > new_p_gg) {
			ch_state = MCLinkExtended::MEDIUM;
		}
	} else if (ch_state == MCLinkExtended::MEDIUM) {
		double new_p_mg = P_n[1][0];
		double new_p_mm = P_n[1][1];
		double rng = RNG::defaultrng()->uniform_double();
		if (rng > new_p_mm + new_p_mg) {
			ch_state = MCLinkExtended::BAD;
		} else if (rng > new_p_mm) {
			ch_state = MCLinkExtended::GOOD;
		}
	} else if (ch_state == MCLinkExtended::BAD) {
		double new_p_bg = P_n[2][0];
		double new_p_bb = P_n[2][2];
		double rng = RNG::defaultrng()->uniform_double();
		if (rng > new_p_bb + new_p_bg) {
			ch_state = MCLinkExtended::MEDIUM;
		} else if (rng > new_p_bb) {
			ch_state = MCLinkExtended::GOOD;
		}
	}
	last_step = curr_step;
	return ch_state;
}

int
MCLinkExtended::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "getLastStep") == 0) {
			tcl.resultf("%d", getLastStep());
			return TCL_OK;
		} else
		if (strcasecmp(argv[1], "getChState") == 0) {
			tcl.resultf("%d", getChState());
			return TCL_OK;
		} else
		if (strcasecmp(argv[1], "getPSucc") == 0) {
			tcl.resultf("%f", getPSucc());
			return TCL_OK;
		}
	}
	std::cerr << "TCL: unknown MCLinkExtended command" << std::endl;
	return TCL_ERROR;
}
