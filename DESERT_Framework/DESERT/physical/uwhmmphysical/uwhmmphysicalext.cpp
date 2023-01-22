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
 * @file   uwhmmphysicalext.cpp
 * @author Nicola Toffolo
 * @version 1.0.0
 *
 * \brief Implementation of UnderwaterHMMPhysicalExt class
 *
 */

#include "uwhmmphysicalext.h"
#include "mclinkextended.h"
#include "uwphy-clmsg.h"
#include "uwstats-utilities.h"
#include "mac.h"
#include "clmsg-stats.h"
#include <phymac-clmsg.h>
#include <string.h>
#include <rng.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>
#include <iostream>

static class UwHMMPhysicalExtendedClass : public TclClass
{
public:
	UwHMMPhysicalExtendedClass()
		: TclClass("Module/UW/HMMPHYSICAL/EXTENDED")
	{
	}
	TclObject *
	create(int argc, const char *const * argv)
	{
		return (new UnderwaterHMMPhysicalExtended);
	}
} class_module_uwhmmphysicalextended;

UnderwaterHMMPhysicalExtended::UnderwaterHMMPhysicalExtended()
	: UnderwaterHMMPhysical()
	, pkts_tot_medium(0)
{
}

int
UnderwaterHMMPhysicalExtended::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "getPktsTotBad") == 0) {
			tcl.resultf("%d", getPktsTotBad());
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getPktsTotMedium") == 0) {
			tcl.resultf("%d", getPktsTotMedium());
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getPktsTotGood") == 0) {
			tcl.resultf("%d", getPktsTotGood());
			return TCL_OK;
		}
	}
	if (strcasecmp(argv[1], "setMCLink") == 0) {
		if (argc == 4) {

			auto link = dynamic_cast<MCLink *>(TclObject::lookup(argv[3]));
			if (link) {
				setMCLink(std::stoi(argv[2]), link);
				return TCL_OK;
			}
			std::cerr << "TCL: failed cast on MCLink" << std::endl;
			return TCL_ERROR;
		}  
		std::cerr << "TCL error: setMCLink mac MCLink" << std::endl;
		return TCL_ERROR;
	}
	return UnderwaterPhysical::command(argc, argv);
} /* UnderwaterHMMPhysicalExtended::command */


