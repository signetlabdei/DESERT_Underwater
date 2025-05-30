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
 * @file uwrovctr-module.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Provides the <i>UWROVCtr</i> class implementation.
 *
 * Provides the <i>UWROVCtr</i> class implementation.
 */

#include "uwrovctr-module.h"
#include <iostream>
#include <sstream>

/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwROVCtrModuleClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwROVCtrModuleClass()
		: TclClass("Module/UW/ROV/CTR")
	{
	}

	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwROVCtrModule());
	}
} class_module_uwROV_ctr;

UwROVCtrModule::UwROVCtrModule()
	: UwCbrModule()
	, sn(0)
	, adaptiveRTO(0)
	, adaptiveRTO_parameter(0.5)
	, speed(1)
	, posit(Position())
	, pkt()
{
	bind("adaptiveRTO_", (int *) &adaptiveRTO);
	if (adaptiveRTO == 1) {
		bind("adaptiveRTO_parameter_", (double *) &adaptiveRTO_parameter);
	}

	if (adaptiveRTO_parameter < 0) {
		cerr << NOW << "Invalide adaptive RTO parameter < 0, set to 0.5 "
			 << "by default " << std::endl;
		adaptiveRTO_parameter = 0.5;
	}
}

int
UwROVCtrModule::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "getROVMonheadersize") == 0) {
			tcl.resultf("%d", this->getROVMonHeaderSize());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getROVctrheadersize") == 0) {
			tcl.resultf("%d", this->getROVCTRHeaderSize());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getX") == 0) {
			tcl.resultf("%f", posit.getX());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getY") == 0) {
			tcl.resultf("%f", posit.getY());

			return TCL_OK;
		} else if (strcasecmp(argv[1], "getZ") == 0) {
			tcl.resultf("%f", posit.getZ());

			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setPosition") == 0) {
			Position *p = dynamic_cast<Position *>(tcl.lookup(argv[2]));

			if(p) {
				posit = *p;

				return TCL_OK;
			}

			return TCL_ERROR;
		} else if (strcasecmp(argv[1], "setSpeed") == 0) {
			std::stringstream speed_ss(argv[2]);
			if (speed_ss >> speed)
				return TCL_OK;

			return TCL_ERROR;
		} else if (strcasecmp(argv[1], "setAdaptiveRTOparameter") == 0) {
			std::stringstream artop_ss(argv[2]);
			if (artop_ss >> adaptiveRTO_parameter)
				return TCL_OK;

			return TCL_ERROR;
		}
	} else if (argc == 5) {
		if (strcasecmp(argv[1], "sendPosition") == 0) {
			newX = std::atof(argv[2]);
			newY = std::atof(argv[3]);
			newZ = std::atof(argv[4]);

			this->reset_retx();
			this->transmit();

			tcl.resultf("%s %f %f %f", "Position set to ", newX, newY, newZ);

			return TCL_OK;
		}
	} else if (argc == 6) {
		if (strcasecmp(argv[1], "sendPosition") == 0) {
			newX = std::atof(argv[2]);
			newY = std::atof(argv[3]);
			newZ = std::atof(argv[4]);
			speed = std::atof(argv[5]);

			this->reset_retx();
			this->transmit();

			tcl.resultf("%s %f %f %f", "Position set to ", newX, newY, newZ);
			tcl.resultf("%s %f", "Speed set to ", speed);

			return TCL_OK;
		}
	}

	return UwCbrModule::command(argc, argv);
}

void
UwROVCtrModule::transmit()
{
	sendPkt();

	if (adaptiveRTO == 1 && rttsamples > 0) {
		period_ = sumrtt / rttsamples + adaptiveRTO_parameter * sumrtt / rttsamples;

		if (debug_) {
			std::cout << NOW << " UwROVCtrModule::RTO set to " << period_
					  << std::endl;
		}
	}

	sendTmr_.resched(period_);
}

void
UwROVCtrModule::start()
{
}

void
UwROVCtrModule::initPkt(Packet *p)
{
	if (this->pkt == nullptr) {
		hdr_uwROV_ctr *uwROVh = HDR_UWROV_CTR(p);

		uwROVh->x() = newX;
		uwROVh->y() = newY;
		uwROVh->z() = newZ;
		uwROVh->speed() = speed;
		uwROVh->sn() = ++sn;

		this->pkt = p;
	} else {
		hdr_uwROV_ctr *uwROVh = HDR_UWROV_CTR(p);

		uwROVh->x() = newX;
		uwROVh->y() = newY;
		uwROVh->z() = newZ;
		uwROVh->speed() = speed;
		uwROVh->sn() = sn;

		if (debug_)
			std::cout << NOW << " UwROVCtrModule::initPkt(Packet *p) "
					  << "Retransmitting" << std::endl;
	}

	if (debug_) {
		if (rttsamples > 0) {
			std::cout << NOW << " RTT UwROVCtr: " << sumrtt / rttsamples << " s"
					  << std::endl;
		}
	}

	UwCbrModule::initPkt(p);

	if (debug_) {
		hdr_uwROV_ctr *uwROVh = HDR_UWROV_CTR(p);

		std::cout << NOW << " UwROVCtrModule::initPkt(Packet *p)  setting new"
				  << " ROV way-point: X = " << uwROVh->x()
				  << ", Y = " << uwROVh->y() << ", Z = " << uwROVh->z()
				  << std::endl;
	}
}

void
UwROVCtrModule::recv(Packet *p, Handler *h)
{
	recv(p);
}

void
UwROVCtrModule::recv(Packet *p)
{
	hdr_uwROV_monitoring *monitoring = HDR_UWROV_MONITORING(p);
	x_rov = monitoring->x();
	y_rov = monitoring->y();
	z_rov = monitoring->z();

	if (monitoring->ack() == sn + 1) {
		sendTmr_.force_cancel();
		this->pkt = nullptr;
	}

	if (monitoring->ack() > 0 && debug_)
		std::cout << NOW << " UwROVCtrModule::recv(Packet *p) control ACK "
				  << "received " << monitoring->ack() << std::endl;
	else if ((monitoring->ack()) < 0 && debug_)
		std::cout << NOW << " UwROVCtrModule::recv(Packet *p) control error "
				  << "received" << std::endl;
	if (debug_ > 10)
		std::cout << NOW << " UwROVCtrModule::recv(Packet *p) ROV monitoring "
				  << "position: X = " << x_rov << ", Y = " << y_rov
				  << ", Z = " << z_rov << std::endl;

	UwCbrModule::recv(p);
}
