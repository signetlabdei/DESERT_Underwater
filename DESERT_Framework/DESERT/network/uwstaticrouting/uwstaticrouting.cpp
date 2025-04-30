//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwstaticrouting.cpp
 * @author Giovanni Toso
 * @version 1.0.0
 *
 * \brief Provides the class implementation of UWSTATICROUTING.
 *
 * Provides the class implementation of UWSTATICROUTING.
 */

#include "uwstaticrouting.h"
#include <string>

/**
 * Adds the module for UwStaticRoutingModuleClass in ns2.
 */
static class UwStaticRoutingModuleClass : public TclClass
{
public:
	UwStaticRoutingModuleClass()
		: TclClass("Module/UW/StaticRouting")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UwStaticRoutingModule);
	}
} class_uwstaticrouting_module;

UwStaticRoutingModule::UwStaticRoutingModule()
	: default_gateway(0)
{
	clearRoutes();
}

void
UwStaticRoutingModule::addRoute(const uint8_t &dst, const uint8_t &next)
{
	if (dst == 0 || next == 0) {
		std::cerr << "You are trying to insert an invalid entry in the routing "
					 "table with destination: "
				  << static_cast<uint32_t>(dst)
				  << " and next hop: " << static_cast<uint32_t>(next)
				  << std::endl;
		exit(1);
	}

	std::map<uint8_t, uint8_t>::iterator it = routing_table.find(dst);

	if (it != routing_table.end()) {
		it->second = next;
	} else {
		if (routing_table.size() < IP_ROUTING_MAX_ROUTES) {
			routing_table.insert(std::pair<uint8_t, uint8_t>(dst, next));
		} else {
			std::cerr << "The routing table is full!" << std::endl;
		}
	}
}

int
UwStaticRoutingModule::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "numroutes") == 0) {
			tcl.resultf("%d", routing_table.size());
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "clearroutes") == 0) {
			clearRoutes();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "defaultGateway") == 0) {
			if (static_cast<uint8_t>(atoi(argv[2])) != 0) {
				default_gateway = static_cast<uint8_t>(atoi(argv[2]));
				return TCL_OK;
			}

			tcl.result("invalid address as default gateway. Exiting ...");
			return TCL_ERROR;
		}

	} else if (argc == 4) {
		if (strcasecmp(argv[1], "addroute") == 0) {
			int dst = std::atoi(argv[2]);
			int next = std::atoi(argv[3]);

			if (dst <= 0 || dst > 255 || next <= 0 || next > 255) {
				tcl.result("invalid destination or next hop address");
				return TCL_ERROR;
			}

			addRoute(static_cast<uint8_t>(dst), static_cast<uint8_t>(next));
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

void
UwStaticRoutingModule::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwip *uwiph = HDR_UWIP(p);

	printOnLog(Logger::LogLevel::DEBUG,
			"UWSTATICROUTING",
			"recv(Packet *)::next hop " + to_string(ch->next_hop()) +
					" destination ip = " + to_string((uint) uwiph->daddr()));

	if (ch->direction() == hdr_cmn::UP) {
		if (uwiph->daddr() == ch->next_hop() ||
				uwiph->daddr() == UWIP_BROADCAST) {
			sendUp(p);
			return;
		}

		ch->direction() = hdr_cmn::DOWN;
		ch->next_hop() = getNextHop(p);

		if (ch->next_hop() == 0)
			drop(p, 1, DROP_DEST_NO_ROUTE);
		else
			sendDown(p);
	} else {
		ch->next_hop() = getNextHop(p);

		if (ch->next_hop() == 0)
			drop(p, 1, DROP_DEST_NO_ROUTE);
		else
			sendDown(p);
	}
}

uint8_t
UwStaticRoutingModule::getNextHop(const Packet *p) const
{
	return getNextHop(HDR_UWIP(p)->daddr());
}

uint8_t
UwStaticRoutingModule::getNextHop(const uint8_t &dst) const
{
	std::map<uint8_t, uint8_t>::const_iterator it = routing_table.find(dst);

	if (it != routing_table.end()) {
		return it->second;
	} else {
		if (default_gateway != 0)
			return default_gateway;

		return 0;
	}
}
