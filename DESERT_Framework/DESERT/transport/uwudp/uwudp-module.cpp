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
 * @file   uwudp-module.cpp
 * @author Giovanni Toso
 * @version 1.0.0
 *
 * \brief Provides the <i>UWUDP</i> class implementation.
 *
 * Provides the <i>UWUDP</i> class implementation.
 */

#include "uwudp-module.h"

#include <iostream>
#include <set>
#include <sstream>
#include <string>

extern packet_t PT_UWUDP;

int hdr_uwudp::offset_ = 0;

static class UwUdpPktClass : public PacketHeaderClass
{
public:
	UwUdpPktClass()
		: PacketHeaderClass("PacketHeader/UWUDP", sizeof(hdr_uwudp))
	{
		this->bind();
		bind_offset(&hdr_uwudp::offset_);
	}
} class_uwudp_pkt;

static class UwUdpClass : public TclClass
{
public:
	UwUdpClass()
		: TclClass("Module/UW/UDP")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UwUdp);
	}
} class_module_uwudp;

UwUdp::UwUdp()
	: portcounter(0)
	, drop_duplicated_packets_(0)
	, debug_(0)
{
	bind("drop_duplicated_packets_", &drop_duplicated_packets_);
	bind("debug_", &debug_);
}

int
UwUdp::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "getudpheadersize") == 0) {
			tcl.resultf("%d", this->getUdpHeaderSize());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "printidspkts") == 0) {
			this->printIdsPkts();
			return TCL_OK;
		}
	}
	if (argc == 3) {
		if (strcasecmp(argv[1], "assignPort") == 0) {
			Module *m = dynamic_cast<Module *>(tcl.lookup(argv[2]));
			if (!m)
				return TCL_ERROR;
			int port = assignPort(m);
			tcl.resultf("%d", port);
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

void
UwUdp::recv(Packet *p)
{
	printOnLog(Logger::LogLevel::ERROR,
			"UWUDP",
			"recv(Packet *)::packet sent without source module");
	Packet::free(p);
}

void
UwUdp::recv(Packet *p, int idSrc)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *iph = HDR_UWIP(p);

	if (!ch->error()) {
		if (ch->direction() == hdr_cmn::UP) {

			map<int, int>::const_iterator iter = id_map.find(uwudp->dport());

			if (iter == id_map.end()) {
				printOnLog(Logger::LogLevel::ERROR,
						"UWUDP",
						"recv(Packet *, int)::unknown port number, dport = " +
								to_string(uwudp->dport()));

				drop(p, 1, DROP_UNKNOWN_PORT_NUMBER);
				return;
			}

			int module_id = iter->second;

			printOnLog(Logger::LogLevel::DEBUG,
					"UWUDP",
					"recv(Packet *, int)::new packet with id " +
							to_string(ch->uid()) + " from ip " +
							to_string(static_cast<uint16_t>(iph->saddr())) +
							" : " + to_string(iter->first));

			if (drop_duplicated_packets_ == 1) {
				map<uint16_t, map_packets_el>::iterator it =
						map_packets.find(iter->first);

				if (it == map_packets.end()) {
					printOnLog(Logger::LogLevel::DEBUG,
							"UWUDP",
							"recv(Packet *, int)::packet to a new port");

					std::set<int> tmp_set_;
					tmp_set_.insert(ch->uid());
					map_packets_el tmp_map_el_;
					tmp_map_el_.insert(pair<uint8_t, std::set<int>>(
							iph->saddr(), tmp_set_));
					map_packets.insert(pair<uint16_t, map_packets_el>(
							iter->first, tmp_map_el_));
				} else {
					printOnLog(Logger::LogLevel::DEBUG,
							"UWUDP",
							"recv(Packet *, int)::packet to a known port");

					std::map<uint8_t, std::set<int>>::iterator it2 =
							it->second.find(iph->saddr());
					if (it2 == it->second.end()) {
						printOnLog(Logger::LogLevel::DEBUG,
								"UWUDP",
								"recv(Packet *, int)::packet from a new "
								"source");

						std::set<int> tmp_set_;
						tmp_set_.insert(ch->uid());
						it->second.insert(pair<uint8_t, std::set<int>>(
								iph->saddr(), tmp_set_));
					} else {
						printOnLog(Logger::LogLevel::DEBUG,
								"UWUDP",
								"recv(Packet *, int)::packet from a known "
								"source");

						if (it2->second.count(ch->uid()) < 1) {
							printOnLog(Logger::LogLevel::DEBUG,
									"UWUDP",
									"recv(Packet *, int)::new packet received");

							it2->second.insert(ch->uid());
						} else { // Packet already received.
							printOnLog(Logger::LogLevel::DEBUG,
									"UWUDP",
									"recv(Packet *, int)::duplicate packet "
									"dropped");

							drop(p, 1, DROP_RECEIVED_DUPLICATED_PACKET);
							return;
						}
					}
				}
			}

			ch->size() -= sizeof(hdr_uwudp);
			sendUp(module_id, p);
		} else {
			map<int, int>::const_iterator iter = port_map.find(idSrc);

			if (iter == port_map.end()) {
				printOnLog(Logger::LogLevel::ERROR,
						"UWUDP",
						"recv(Packet *, int)::no port assigned to id " +
								to_string(idSrc) + ", dropping packet");

				Packet::free(p);
				return;
			}

			int sport = iter->second;
			assert(sport > 0 && sport <= portcounter);
			uwudp->sport() = sport;

			ch->size() += sizeof(hdr_uwudp);
			sendDown(p);
		}
	}
}

int
UwUdp::assignPort(Module *m)
{
	int id = m->getId();

	// Check that the provided module has not been given a port before
	if (port_map.find(id) != port_map.end())
		return TCL_ERROR;

	int newport = ++portcounter;

	port_map[id] = newport;
	assert(id_map.find(newport) == id_map.end());
	id_map[newport] = id;
	assert(id_map.find(newport) != id_map.end());

	std::stringstream msg;
	msg << "assignPort(Module *)::"
		<< "id = " << id << " port = " << newport
		<< " portcounter = " << portcounter;
	printOnLog(Logger::LogLevel::INFO, "UWUDP", msg.str());

	return newport;
}
