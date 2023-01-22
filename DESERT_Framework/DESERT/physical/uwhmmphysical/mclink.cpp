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
 * @file   uwhmmphysical.cpp
 * @author Antonio Montanari
 * @version 1.1.0
 *
 * \brief Implementation of UnderwaterHMMPhysical class
 *
 */

#include <string.h>
#include <rng.h>
#include <tclcl.h>
#include <iostream>
#include "mclink.h"

MCLink::MCLink() 
	: p_succ_good(0.0)
	, p_succ_bad(0.0)
	, p_gb(0.0)
	, p_bg(0.0)
	, ch_state(GOOD)
	, last_step(0)
{	
	bind("p_succ_good", &p_succ_good);
	bind("p_succ_bad", &p_succ_bad);
	bind("p_gb", &p_gb);
	bind("p_bg", &p_bg);
	bind("ch_state", (int*)&ch_state);
	bind("last_step", &last_step);
}


MCLink::MCLink(double p_succ_good, double p_succ_bad,double p_gb, double p_bg, 
		ChState ch_state,int curr_step)
	: p_succ_good(p_succ_good)
	, p_succ_bad(p_succ_bad)
	, p_gb(p_gb)
	, p_bg(p_bg)
	, ch_state(ch_state)
	, last_step(curr_step)
{
	assert(p_succ_good >=0.0 && p_succ_good <= 1.0 && 
			p_succ_bad >= 0.0 && p_succ_bad <= 1.0 &&
			p_gb >= 0.0 && p_gb <= 1.0 && p_bg >= 0.0 && p_bg <= 1.0);
}

MCLink::ChState
MCLink::updateChState(int curr_step)
{
	int n_step = curr_step - last_step;
	double c1 = 1.0 / (p_gb + p_bg);
	double c2 = pow(1.0 - p_gb - p_bg, n_step) / (p_gb + p_bg);

	if (ch_state == MCLink::GOOD) {
		double new_p_gg = c1 * p_bg + c2 * p_gb;
		if (RNG::defaultrng()->uniform_double() > new_p_gg) {
			ch_state = MCLink::BAD;
		}
	} else if (ch_state == MCLink::BAD) {
		double new_p_bb = c1 * p_gb + c2 * p_bg;
		if (RNG::defaultrng()->uniform_double() > new_p_bb) {
			ch_state = MCLink::GOOD;	
		}
	}
	last_step = curr_step;
	return ch_state;
}

int
MCLink::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "getLastStep") == 0) {
			tcl.resultf("%d", getLastStep());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getChState") == 0) {
			tcl.resultf("%d", getChState());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getPSucc") == 0) {
			tcl.resultf("%f", getPSucc());
			return TCL_OK;
		}
	}

	std::cerr << "TCL: unknown MCLink command" << std::endl;
	return TCL_ERROR;
}

static class MCLinkClass : public TclClass
{
public:
	MCLinkClass()
	: TclClass("Module/UW/HMMPHYSICAL/MCLINK")
	{
	}
	TclObject *
	create(int argc, const char *const * argv)
	{
		if (argc > 7) {
			if (argc == 8) {
				return (new MCLink(std::stod(argv[4]), std::stod(argv[5]), 
						std::stod(argv[6]), std::stod(argv[7])));
			}
			if (argc == 9 || argc == 10) {
				MCLink::ChState ch_state;
				if (strcasecmp(argv[8], "GOOD") == 0) {
					ch_state = MCLink::GOOD;
				} else if (strcasecmp(argv[8], "BAD") == 0) {
					ch_state = MCLink::BAD;
				} else {
					std::cerr << "TCL: MCLink state must be GOOD or BAD"
					<< std::endl;
					exit(1);
				}
				if (argc == 9) {
					return (new MCLink(std::stod(argv[4]), std::stod(argv[5]), 
							std::stod(argv[6]), std::stod(argv[7]), ch_state));
				}
				if (argc == 10) {
					return (new MCLink(std::stod(argv[4]), std::stod(argv[5]), 
							std::stod(argv[6]), std::stod(argv[7]),
							ch_state, std::stoi(argv[9])));
				}	
			}
			std::cerr << "TCL: check MCLink constructor args" << std::endl;
			exit(1);
		}

		return (new MCLink);
	}
} class_module_mclink;