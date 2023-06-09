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
* @file   uwsmeposition.cpp
* @author Filippo Campagnaro
* @version 1.0.0
*
* \brief Provides the <i>UWSMEPosition</i> class implementation.
*
* Provides the <i>UWSMEPosition</i> class implementation.
*/

#include <iostream>
#include "uwsmeposition.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class UWSMEPositionClass : public TclClass
{
public:
	UWSMEPositionClass()
		: TclClass("Position/UWSME")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UWSMEPosition());
	}
} class_uwsmeposition;

UWSMEPosition::UWSMEPosition()
	: UWSMPosition()
	, alarm_mode(0)
{
	bind("debug_", &debug_);
}

UWSMEPosition::~UWSMEPosition()
{
}

int
UWSMEPosition::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 6) {
		if (strcasecmp(argv[1], "setdest") == 0) {
			if (debug_)
				cerr << NOW << "UWSMEPosition::command(setdest, " << argv[2]
					 << ", " << argv[3] << ", " << argv[4] << ", " << argv[5]
					 << ")" << endl;
			setdest(atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]));
			return TCL_OK;
		}
	} else if (argc == 5) {
		if (strcasecmp(argv[1], "setdest") == 0) {
			if (debug_)
				cerr << NOW << "UWSMEPosition::command(setdest, " << argv[2]
					 << ", " << argv[3] << ", " << argv[4] << ")" << endl;
			setdest(atof(argv[2]), atof(argv[3]), atof(argv[4]));
			return TCL_OK;
		}
	} else if (argc == 2) {
		if (strcasecmp(argv[1], "update") == 0) {
			double now = Scheduler::instance().clock();
			update(now);
			return TCL_OK;
		}
	}
	return Position::command(argc, argv);
}

void
UWSMEPosition::setdest(
		double x_dest, double y_dest, double z_dest, double spead_setted)
{
	if (alarm_mode){
		UWSMPosition::setdest(x_dest,y_dest,z_dest);
		if (debug_)
			printf("New pos (%f,%f,%f), dest(%f,%f,%f), speed blocked by ALARM\n",
					getX(),
					getY(),
					getZ(),
					getXdest(),
					getYdest(),
					getZdest());
	}else{
		UWSMPosition::setdest(x_dest,y_dest,z_dest,spead_setted);
	}
	
}

void
UWSMEPosition::setdest(double x_dest, double y_dest, double z_dest)
{
	UWSMPosition::setdest(x_dest,y_dest,z_dest);
}

void
UWSMEPosition::update(double now)
{
	UWSMPosition::update(now);
}
/*
double
UWSMEPosition::getX()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_ > 0.) && (now > lastUpdateTime_ + 1e-6))
		update(now);
	return (x_);
}

void
UWSMEPosition::setX(double x)
{
	x_ = x;
	Xdest_ = x;
	Xsorg_ = x;
}
void
UWSMEPosition::setY(double y)
{
	y_ = y;
	Ydest_ = y;
	Ysorg_ = y;
}
void
UWSMEPosition::setZ(double z)
{
	z_ = z;
	Zdest_ = z;
	Zsorg_ = z;
}
double
UWSMEPosition::getY()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_ > 0.) && (now > lastUpdateTime_ + 1e-6))
		update(now);
	return (y_);
}

double
UWSMEPosition::getZ()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_ > 0.) && (now > lastUpdateTime_ + 1e-6))
		update(now);
	return (z_);
}

double UWSMEPosition::getXdest() const
{
	return Xdest_;
}

double UWSMEPosition::getYdest() const
{
	return Ydest_;
}

double UWSMEPosition::getZdest() const
{
	return Zdest_;
}

double UWSMEPosition::getSpeed() const
{
	return speed_;
}*/

void 
UWSMEPosition::setAlarm(bool alarm)
{
	alarm_mode = alarm;
}
