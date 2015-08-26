/*
 * Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Padova (SIGNET lab) nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /**
 * @file   uwsmposition.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 * 
 * \brief Provides the <i>UWSMPosition</i> class implementation.
 * 
 * Provides the <i>UWSMPosition</i> class implementation.
 */



#include <iostream>
#include "uwsmposition.h"


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class UWSMPositionClass : public TclClass {
public:
	UWSMPositionClass() : TclClass("Position/UWSM") {}
	TclObject* create(int, const char*const*) {
		return (new UWSMPosition());
	}
} class_uwsmposition;

UWSMPosition::UWSMPosition() : Position(), trgTime_(-1), Xsorg_(0), Ysorg_(0), Zsorg_(0) , Xdest_(0), Ydest_(0), Zdest_(0), speed_(0), lastUpdateTime_(0)
{
	bind("debug_", &debug_);
}

UWSMPosition::~UWSMPosition()
{
}

int UWSMPosition::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 6)
	{
		if(strcasecmp(argv[1], "setdest") == 0)
		{
		  if (debug_ > 10)
		    cerr << NOW << "UWSMPosition::command(setdest, "
			 << argv[2] << ", "
			 << argv[3] << ", "
			 << argv[4] << ", "
			 << argv[5] << ")"
			 << endl;
		      
			trgTime_ = Scheduler::instance().clock();
			if(trgTime_ <= 0.0)
			  cerr << "Warning: calling set dest at time <= 0 will not work" << endl;
			setdest(atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]));
			return TCL_OK;
		}
	} else if(argc == 5)
	{
		if(strcasecmp(argv[1], "setdest") == 0)
		{
		  if (debug_ > 10)
		    cerr << NOW << "UWSMPosition::command(setdest, "
			 << argv[2] << ", "
			 << argv[3] << ", "
			 << argv[4] << ")"
			 << endl;
		      
			trgTime_ = Scheduler::instance().clock();
			if(trgTime_ <= 0.0)
			  cerr << "Warning: calling set dest at time <= 0 will not work" << endl;
			setdest(atof(argv[2]),atof(argv[3]),atof(argv[4]));
			return TCL_OK;
		}
	} else if(argc == 2)
	{
		if(strcasecmp(argv[1], "update") == 0)
		{
			double now = Scheduler::instance().clock();
			update(now);
			return TCL_OK;
		}
	}
	return Position::command(argc, argv);
}

void UWSMPosition::setdest(double x_dest,double y_dest,double z_dest,double spead_setted){
	double now = Scheduler::instance().clock();
	update(now);
	trgTime_ = now;
	Xdest_ = x_dest;
	Ydest_ = y_dest;
	Zdest_ = z_dest;
	speed_ = spead_setted;
	Xsorg_ = x_;
	Ysorg_ = y_;
	Zsorg_ = z_;
}

void UWSMPosition::setdest(double x_dest,double y_dest,double z_dest){
	double now = Scheduler::instance().clock();
	update(now);
	trgTime_ = now;
	Xdest_ = x_dest;
	Ydest_ = y_dest;
	Zdest_ = z_dest;
	Xsorg_ = x_;
	Ysorg_ = y_;
	Zsorg_ = z_;
}

void UWSMPosition::update(double now)
{
	if ((trgTime_<0.)||(now<lastUpdateTime_+1e-6))
		return;
	double gamma;
	double theta;
	double theta_den=sqrt(pow(Ydest_-Ysorg_,2.0)+pow(Xdest_-Xsorg_,2.0)+pow(Zdest_-Zsorg_,2.0));
	if (theta_den==0) {
		x_ = Xsorg_;
		y_ = Ysorg_;
		z_ = Zsorg_;
	} else {
		theta=acos((Zdest_-Zsorg_)/theta_den);
		if (Xdest_-Xsorg_==0)
			gamma = pi/2*sgn(Ydest_-Ysorg_);
		else 
			gamma = atan((Ydest_-Ysorg_)/(Xdest_-Xsorg_));
		if ((Xdest_-Xsorg_)<0.0) gamma += (Ysorg_-Ydest_)>=0.0?pi:-pi;
		x_ = Xsorg_ + (speed_*(now - trgTime_) )*sin(theta)*cos(gamma);
		y_ = Ysorg_ + (speed_*(now - trgTime_) )*sin(theta)*sin(gamma);
		z_ = Zsorg_ + (speed_*(now - trgTime_) )*cos(theta);
		if (pow(Ydest_-Ysorg_,2.0)+pow(Xdest_-Xsorg_,2.0)+pow(Zdest_-Zsorg_,2.0)<pow(x_-Xsorg_,2.0)+pow(y_-Ysorg_,2.0)+pow(z_-Zsorg_,2.0)){
			x_=Xdest_;y_=Ydest_;z_=Zdest_;
		}
		if (debug_>50)
			printf("New pos (%f,%f,%f), dest(%f,%f,%f), speed %f sen(%f)=%f\n",x_,y_,z_,Xdest_,Ydest_,Zdest_,speed_,gamma, sin(gamma));
	}
		
	lastUpdateTime_ = now;
}

double UWSMPosition::getX()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_>0.)&&(now>lastUpdateTime_+1e-6))
		update(now);
	return (x_);
}

void UWSMPosition::setX(double x){
	x_=x;
	Xdest_=x;
	Xsorg_=x;
}
void UWSMPosition::setY(double y){
	y_=y;
	Ydest_=y;
	Ysorg_=y;
}
void UWSMPosition::setZ(double z){
	z_=z;
	Zdest_=z;
	Zsorg_=z;
}
double UWSMPosition::getY()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_>0.)&&(now>lastUpdateTime_+1e-6))
		update(now);
	return (y_);
}

double UWSMPosition::getZ()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_>0.)&&(now>lastUpdateTime_+1e-6))
		update(now);
	return (z_);
}
