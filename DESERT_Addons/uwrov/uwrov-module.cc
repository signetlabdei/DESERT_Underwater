//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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

/**
 * @file uwrov-module.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Provides the <i>UWROV</i> class implementation.
 *
 */

#include "uwrov-module.h"
#include <iostream>

int hdr_uwROV_monitoring::offset_; /**< Offset used to access in <i>hdr_uwROV</i> packets header. */
int hdr_uwROV_ctr::offset_; /**< Offset used to access in <i>hdr_uwROV</i> packets header. */

/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwROVModuleClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwROVModuleClass()
		: TclClass("Module/UW/ROV")
	{
	}

	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwROVModule());
	}
} class_module_uwROV;

void
UwROVSendAckTimer::expire(Event *e)
{
	module->sendAck();
}

UwROVModule::UwROVModule()
	: UwCbrModule()
	, last_sn_confirmed(0)
	, ack(0)
	, ackPriority(0)
	, ackNotPgbk(0)
	, drop_old_waypoints(0)
	, ackTimeout(10)
	, posit()
	, ackTimer_(this)
	, ackPolicy(ACK_PIGGYBACK)
	, log_flag(0)
	, out_file_stats(0)
{
	bind("ackTimeout_", (int *) &ackTimeout);
	bind("ackPriority_", (int *) &ackPriority);
	bind("drop_old_waypoints_", (int *) &drop_old_waypoints);
	bind("log_flag_", (int *) &log_flag);

	if (ackTimeout < 0) {
		cerr << NOW << " Invalide ACK timout < 0, timeout set to 10 by defaults"
			 << std::endl;
		ackTimeout = 10;
	}
}

int
UwROVModule::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "getROVMonheadersize") == 0) {
			tcl.resultf("%d", getROVMonHeaderSize());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getROVctrheadersize") == 0) {
			tcl.resultf("%d", getROVCTRHeaderSize());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getX") == 0) {
			tcl.resultf("%f", posit->getX());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getY") == 0) {
			tcl.resultf("%f", posit->getY());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getZ") == 0) {
			tcl.resultf("%f", posit->getZ());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getAckNotPgbk") == 0) {
			tcl.resultf("%d", ackNotPgbk);

			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setPosition") == 0) {
			UWSMPosition *p = dynamic_cast<UWSMPosition *>(tcl.lookup(argv[2]));
			if (p) {
				posit = p;

				tcl.resultf("%s", "Position set\n");

				return TCL_OK;
			}

			tcl.result("Position error\n");

			return TCL_ERROR;
		}
		if (strcasecmp(argv[1], "setAckPolicy") == 0) {
			if (atof(argv[2]) == 1) {
				ackPolicy = ACK_PIGGYBACK;
				return TCL_OK;
			}
			if (atof(argv[2]) == 2) {
				ackPolicy = ACK_IMMEDIATELY;
				return TCL_OK;
			}
			if (atof(argv[2]) == 3) {
				ackPolicy = ACK_PGBK_OR_TO;
				return TCL_OK;
			}
		}
		if (strcasecmp(argv[1], "setAckTimeout") == 0) {
			ackTimeout = atof(argv[2]);

			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setAckPriority") == 0) {
			ackPriority = atof(argv[2]);

			return TCL_OK;
		}
	} else if (argc == 5) {
		if (strcasecmp(argv[1], "setdest") == 0) {
			posit->setdest(atof(argv[2]), atof(argv[3]), atof(argv[4]));

			return TCL_OK;
		}
	} else if (argc == 6) {
		if (strcasecmp(argv[1], "setdest") == 0) {
			posit->setdest(atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]));

			return TCL_OK;
		}
	}

	return UwCbrModule::command(argc, argv);
}

void
UwROVModule::initPkt(Packet *p)
{
	hdr_uwROV_monitoring *uwROVh = HDR_UWROV_MONITORING(p);
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);

	uwROVh->x() = posit->getX();
	uwROVh->y() = posit->getY();
	uwROVh->z() = posit->getZ();
	uwROVh->ack() = ack;

	ack = 0;

	if (debug_)
		std::cout << NOW << " UwROVModule::initPkt(Packet *p) ROV current "
				  << "position: X = " << uwROVh->x() << ", Y = " << uwROVh->y()
				  << ", Z = " << uwROVh->z() << std::endl;

	ackTimer_.force_cancel();

	UwCbrModule::initPkt(p);

	if (debug_) {
		std::cout << NOW << " UwROVModule::sending packet with priority "
				  << (int) uwcbrh->priority() << std::endl;
	}

	priority_ = 0;
}

void
UwROVModule::recv(Packet *p, Handler *h)
{
	recv(p);
}

void
UwROVModule::recv(Packet *p)
{
	hdr_uwROV_ctr *uwROVh = HDR_UWROV_CTR(p);

	if (drop_old_waypoints == 1 && uwROVh->sn() <= last_sn_confirmed) {
		// obsolete packets
		if (debug_)
			std::cout << NOW << " UwROVModule::old waypoint with sn "
					  << uwROVh->sn() << " dropped " << std::endl;

	} else {
		// packet in order
		posit->setdest(uwROVh->x(), uwROVh->y(), uwROVh->z(), uwROVh->speed());
		last_sn_confirmed = uwROVh->sn();
	}

	ack = last_sn_confirmed + 1;
	priority_ = (char) ackPriority;

	if (log_flag == 1) {
		out_file_stats.open("my_log_file.csv", std::ios_base::app);
		out_file_stats << left << "time: " << NOW
					   << ", positions ROV: x = " << posit->getX()
					   << ", y = " << posit->getY() << ", z = " << posit->getZ()
					   << std::endl;

		out_file_stats.close();
	}

	if (debug_) {
		std::cout << NOW
				  << " UwROVModule::recv(Packet *p) ROV received new "
					 "way point: X = "
				  << uwROVh->x() << ", Y = " << uwROVh->y()
				  << ", Z = " << uwROVh->z() << std::endl;
	}

	UwCbrModule::recv(p);

	if (ackPolicy == ACK_IMMEDIATELY) {

		if (ackPriority == 0) {
			UwCbrModule::sendPkt();

			if (debug_)
				cout << NOW << " ACK sent immediately with standard priority "
					 << std::endl;
		} else {
			UwCbrModule::sendPktHighPriority();

			if (debug_)
				cout << NOW << " ACK sent immediately with high priority "
					 << std::endl;
		}
	}

	if (ackPolicy == ACK_PGBK_OR_TO) {
		ackTimer_.resched(ackTimeout);
	}
}

void
UwROVModule::sendAck()
{
	ackNotPgbk++;

	if (ackPriority == 0) {
		UwCbrModule::sendPkt();

		if (debug_)
			cout << NOW << " ACK timeout expired, ACK sent with standard "
				 << "priority " << std::endl;
	} else {
		UwCbrModule::sendPktHighPriority();

		if (debug_)
			cout << NOW << " ACK timeout expired, ACK sent with high priority "
				 << std::endl;
	}
}
