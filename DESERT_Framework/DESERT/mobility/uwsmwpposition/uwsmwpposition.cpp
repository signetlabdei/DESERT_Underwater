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
* @file   uwsmwpposition.cpp
* @author Alessia Ortile
* @version 1.0.0
*
* \brief Provides the <i>UWSMWPPosition</i> class implementation.
*
* Provides the <i>UWSMWPPosition</i> class implementation.
*/

#include <iostream>
#include "uwsmwpposition.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class UWSMWPPositionClass : public TclClass
{
public:
	UWSMWPPositionClass()
		: TclClass("Position/UWSMWP")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UWSMWPPosition());
	}
} class_uwsmwpposition;


UWSMWPPosition::UWSMWPPosition()
	: UWSMPosition()
	, alarm_mode(false) 
{
	bind("debug_", &debug_);
}

UWSMWPPosition::~UWSMWPPosition()
{
}

int
UWSMWPPosition::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 6) {
		if (strcasecmp(argv[1], "setDest") == 0) {
			if (debug_)
				cerr << NOW << "UWSMWPPosition::command(setDest, " << argv[2]
					 << ", " << argv[3] << ", " << argv[4] << ", " << argv[5]
					 << ")" << endl;
			setDest(atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]));
			return TCL_OK;
		} else if (strcasecmp(argv[1], "addDest") == 0) {
			if (debug_)
				cerr << NOW << "UWSMWPPosition::command(addDest, " << argv[2]
					 << ", " << argv[3] << ", " << argv[4] << ", " << argv[5]
					 << ")" << endl;
			addDest(atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]));
			return TCL_OK;
		}
	} else if (argc == 5) {
		if (strcasecmp(argv[1], "setDest") == 0) {
			if (debug_)
				cerr << NOW << "UWSMWPPosition::command(setDest, " << argv[2]
					 << ", " << argv[3] << ", " << argv[4] << ")" << endl;
			setDest(atof(argv[2]), atof(argv[3]), atof(argv[4]));
			return TCL_OK;
		}else if (strcasecmp(argv[1], "addDest") == 0) {
			if (debug_)
				cerr << NOW << "UWSMWPPosition::command(addDest, " << argv[2]
					 << ", " << argv[3] << ", " << argv[4] << ")" << endl;
			addDest(atof(argv[2]), atof(argv[3]), atof(argv[4]));
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
UWSMWPPosition::setDest(
		double x_dest, double y_dest, double z_dest, double speed_setted)
{
	if (alarm_mode){

		if (debug_)
			printf("Alarm_mode %f, dest(%f,%f,%f) not accepted\n",
					alarm_mode,
					x_dest,
					y_dest,
					z_dest);
	}else{

		if (!waypoints.empty()){

			if (waypoints[0][0] == x_dest && waypoints[0][1] == y_dest && waypoints[0][2] == z_dest){

					if (debug_)
						printf("New dest next waypoint in the queue (%f,%f,%f)\n",
						waypoints[0][0],
						waypoints[0][1],
						waypoints[0][2]);

			}else{

				waypoints.insert(waypoints.begin(),{x_dest,y_dest,z_dest,speed_setted});
				if (debug_)
					printf("New destination (%f,%f,%f), skip the queue\n",
					x_dest,
					y_dest,
					z_dest);

			}

		}else{

			waypoints.push_back({x_dest,y_dest,z_dest,speed_setted});
		}

		if (debug_)
			printf("Pos (%f,%f,%f), last dest(%f,%f,%f), new dest = (%f,%f,%f)\n",
					x_,
					y_,
					z_,
					Xdest_,
					Ydest_,
					Zdest_,
					x_dest,
					y_dest,
					z_dest);

		
		UWSMPosition::setdest(x_dest,y_dest,z_dest,speed_setted);

		if (debug_)
			printf("Pos (%f,%f,%f), new dest(%f,%f,%f), speed = %f\n",
					x_,
					y_,
					z_,
					Xdest_,
					Ydest_,
					Zdest_,
					speed_setted);
	}
	
}


void
UWSMWPPosition::setDest(double x_dest, double y_dest, double z_dest)
{
	if (alarm_mode){
		if (debug_)
			printf("Alarm_mode %f, dest(%f,%f,%f) not accepted\n",
					alarm_mode,
					x_dest,
					y_dest,
					z_dest);
	}else{

		if (!waypoints.empty()){
			if (waypoints[0][0] == x_dest && waypoints[0][1] == y_dest && waypoints[0][2] == z_dest){

				if (debug_)
					printf("New dest next waypoint in the queue (%f,%f,%f)\n",
					waypoints[0][0],
					waypoints[0][1],
					waypoints[0][2]);

			}else{

				waypoints.insert(waypoints.begin(),{x_dest,y_dest,z_dest});
				if (debug_)
					printf("New destionation (%f,%f,%f), skip the queue\n",
					x_dest,
					y_dest,
					z_dest);

			}

		}else{		

			waypoints.push_back({x_dest,y_dest,z_dest});
		}
		
		
		UWSMPosition::setdest(x_dest,y_dest,z_dest);


		if (debug_)
			printf("Pos (%f,%f,%f), new dest(%f,%f,%f)\n",
					x_,
					y_,
					z_,
					Xdest_,
					Ydest_,
					Zdest_);
	}
}

void
UWSMWPPosition::addDest(
		double x_dest, double y_dest, double z_dest, double speed_setted)
{
	if (!waypoints.empty()){

		bool exist = false;

		for (const auto& vec : waypoints) {
			// Controlla se le coordinate corrispondono
			if (vec[0] == x_dest && vec[1] == y_dest && vec[2] == z_dest) {
				exist = true;
				break;
			}
		}

		if (!exist){
			waypoints.push_back({x_dest,y_dest,z_dest, speed_setted});
			if (debug_)
				printf("New waypoint (%f,%f,%f)\n",
					x_dest,
					y_dest,
					z_dest);
		}
		
				
	}else{
		UWSMWPPosition::setDest(x_dest,y_dest,z_dest, speed_setted);
		if (debug_)
			printf("Pos (%f,%f,%f), new dest(%f,%f,%f)\n",
					x_,
					y_,
					z_,
					Xdest_,
					Ydest_,
					Zdest_);
	}	
}
	
void
UWSMWPPosition::addDest(
		double x_dest, double y_dest, double z_dest)
{

	if (!waypoints.empty()){

		bool exist = false;

		for (const auto& vec : waypoints) {
			if (vec[0] == x_dest && vec[1] == y_dest && vec[2] == z_dest) {
				exist = true;
				break;
			}
		}

		if (!exist){
			waypoints.push_back({x_dest,y_dest,z_dest});
			if (debug_)
				printf("New waypoint (%f,%f,%f)\n",
					x_dest,
					y_dest,
					z_dest);
		}
		
				
	}else{

		UWSMWPPosition::setDest(x_dest,y_dest,z_dest);

		if (debug_)
			printf("Pos (%f,%f,%f), new dest(%f,%f,%f)\n",
					x_,
					y_,
					z_,
					Xdest_,
					Ydest_,
					Zdest_);
	}
		
}

void
UWSMWPPosition::update(double now)
{
	if ((trgTime_ < 0.) || (now < lastUpdateTime_ + 1e-6))
		return;

	double gamma;
	double theta;
	double theta_den = sqrt(pow(Ydest_ - Ysorg_, 2.0) +				//distance to destination
			pow(Xdest_ - Xsorg_, 2.0) + pow(Zdest_ - Zsorg_, 2.0));

	if (theta_den == 0) {

		x_ = Xsorg_;
		y_ = Ysorg_;
		z_ = Zsorg_;


		if (!waypoints.empty()){

			if(waypoints[0][0] == Xdest_ && waypoints[0][1] == Ydest_ && waypoints[0][2] == Zdest_){
				
				if (debug_)
						printf("Last waypoints erased (%f,%f,%f)\n",
						waypoints[0][0],
						waypoints[0][1],
						waypoints[0][2]);

				waypoints.erase(waypoints.begin());

				if (!waypoints.empty()){

					if(waypoints[0].size()>3){
						UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],waypoints[0][2],waypoints[0][3]);
					}else{
						UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],waypoints[0][2]);
					}

					if (debug_)
						printf("New dest (%f,%f,%f) from waypoints list\n",
						Xdest_,
						Ydest_,
						Zdest_);
				}
			}
				
		}

		

	} else {

		theta = acos((Zdest_ - Zsorg_) / theta_den);

		if (Xdest_ - Xsorg_ == 0.0)
			gamma = pi / 2 * sgn(Ydest_ - Ysorg_);

		else
			gamma = atan((Ydest_ - Ysorg_) / (Xdest_ - Xsorg_));

		if ((Xdest_ - Xsorg_) < 0.0)
			gamma += (Ysorg_ - Ydest_) >= 0.0 ? pi : -pi;


		x_ = Xsorg_ + (speed_ * (now - trgTime_)) * sin(theta) * cos(gamma);
		y_ = Ysorg_ + (speed_ * (now - trgTime_)) * sin(theta) * sin(gamma);
		z_ = Zsorg_ + (speed_ * (now - trgTime_)) * cos(theta);

		if (pow(Ydest_ - Ysorg_, 2.0) + pow(Xdest_ - Xsorg_, 2.0) +
						pow(Zdest_ - Zsorg_, 2.0) <
				pow(x_ - Xsorg_, 2.0) + pow(y_ - Ysorg_, 2.0) +
						pow(z_ - Zsorg_, 2.0)) {

			x_ = Xdest_;
			y_ = Ydest_;
			z_ = Zdest_;


			if (debug_)
					printf("Destination (%f,%f,%f) reached\n",
					Xdest_,
					Ydest_,
					Zdest_);

			if (!waypoints.empty()){

				if(waypoints[0][0] == Xdest_ && waypoints[0][1] == Ydest_ && waypoints[0][2] == Zdest_){
					
					if (debug_)
						printf("Last waypoints erased (%f,%f,%f)\n",
						waypoints[0][0],
						waypoints[0][1],
						waypoints[0][2]);

					waypoints.erase(waypoints.begin());
				

					if (!waypoints.empty()){

						if (debug_)
								printf("Next dest (%f,%f,%f)\n",
								waypoints[0][0],
								waypoints[0][1],
								waypoints[0][2]);

						if(waypoints[0].size()>3){
							UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],waypoints[0][2],waypoints[0][3]);
						}else{
							UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],waypoints[0][2]);
						}

						if (debug_)
							printf("New dest (%f,%f,%f) from waypoints list\n",
							Xdest_,
							Ydest_,
							Zdest_);
					}
				}
			
			}

		}
		if (debug_)
			printf("New pos (%f,%f,%f), dest(%f,%f,%f), source(%f,%f,%f), speed %f sen(%f)=%f\n",
					x_,
					y_,
					z_,
					Xdest_,
					Ydest_,
					Zdest_,
					Xsorg_,
					Ysorg_,
					Zsorg_,
					speed_,
					gamma,
					sin(gamma));
	}

	lastUpdateTime_ = now;
}

void 
UWSMWPPosition::setAlarm(bool alarm)
{
	alarm_mode = alarm;
}
