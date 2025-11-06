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

#include "mclink.h"
#include <iostream>
#include <rng.h>
#include <string.h>
#include <tclcl.h>

MCLink::MCLink()
	: ber_good(0.0)
	, ber_bad(0.0)
	, p_gb(0.0)
	, p_bg(0.0)
	, last_update(NOW)
	, step_period(5.0)
	, ch_state(GOOD)
{
}

MCLink::MCLink(double ber_good, double ber_bad, double p_gb, double p_bg,
		double step_period, ChState ch_state)
	: ber_good(ber_good)
	, ber_bad(ber_bad)
	, p_gb(p_gb)
	, p_bg(p_bg)
	, last_update(NOW)
	, step_period(step_period)
	, ch_state(ch_state)
{
	assert(ber_good >= 0.0 && ber_good <= 1.0 && ber_bad >= 0.0 &&
			ber_bad <= 1.0 && p_gb >= 0.0 && p_gb <= 1.0 && p_bg >= 0.0 &&
			p_bg <= 1.0);
}

MCLink::ChState
MCLink::updateChState()
{
	int n_step = floor((NOW - last_update) / step_period);
	if (n_step == 0) {
		return ch_state;
	}

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
	last_update += n_step * step_period;
	return ch_state;
}

double
MCLink::getBER()
{
	updateChState();
	if (ch_state == GOOD) {
		return ber_good;
	} else {
		return ber_bad;
	}
}

int
MCLink::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "getChState") == 0) {
			tcl.resultf("%d", getChState());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getBER") == 0) {
			tcl.resultf("%f", getBER());
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
	create(int argc, const char *const *argv)
	{
		if (argc > 8) {
			if (argc == 9) {
				return (new MCLink(std::stod(argv[4]),
						std::stod(argv[5]),
						std::stod(argv[6]),
						std::stod(argv[7]),
						std::stod(argv[8])));
			}
			if (argc == 10) {
				MCLink::ChState ch_state;
				if (strcasecmp(argv[9], "GOOD") == 0) {
					ch_state = MCLink::GOOD;
				} else if (strcasecmp(argv[9], "BAD") == 0) {
					ch_state = MCLink::BAD;
				} else {
					std::cerr << "TCL: MCLink state must be GOOD or BAD"
							  << std::endl;
					exit(1);
				}
				return (new MCLink(std::stod(argv[4]),
						std::stod(argv[5]),
						std::stod(argv[6]),
						std::stod(argv[7]),
						std::stod(argv[8]),
						ch_state));
			}
			std::cerr << "TCL: check MCLink constructor args" << std::endl;
			exit(1);
		}

		return (new MCLink);
	}
} class_module_mclink;