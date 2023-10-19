//
// Copyright (c) 2023 Regents of the SIGNET lab, University of Padova.
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
			cout << "Alarm_mode"<< alarm_mode <<", dest("<< x_dest <<","<< 
			y_dest <<","<< z_dest <<") not accepted"<< std::endl; 

	}else{

		if (!waypoints.empty()){

			if (waypoints[0][0] == x_dest && waypoints[0][1] == y_dest && 
				waypoints[0][2] == z_dest){

					if (debug_)
						cout << "New dest next waypoint in the queue (" << 
						waypoints[0][0] << "," << waypoints[0][1] << "," 
						<< waypoints[0][2] << ")"<< std::endl;


			}else{

				waypoints.insert(waypoints.begin(),{x_dest,y_dest,z_dest,speed_setted});
				if (debug_)
					cout << "New destination (" << x_dest << "," << y_dest <<
						 "," << z_dest << "), skip the queue"<< std::endl; 


			}

		}else{

			waypoints.push_back({x_dest,y_dest,z_dest,speed_setted});
		}

		if (debug_)
			cout << "Pos (" << x_ << "," << y_ << "," << z_ << "), last dest("
				<< Xdest_ << "," << Ydest_ << "," << Zdest_ << "), new dest = ("
				<< x_dest << "," << y_dest << "," << z_dest << ")"<< std::endl; 


		
		UWSMPosition::setdest(x_dest,y_dest,z_dest,speed_setted);

		if (debug_)
			cout << "Pos (" << x_ << "," << y_ << "," << z_ << "), new dest(" 
				<< Xdest_ << "," << Ydest_ << "," << Zdest_ << "), speed = " 
				<< speed_setted << std::endl; 

	}
	
}


void
UWSMWPPosition::setDest(double x_dest, double y_dest, double z_dest)
{
	if (alarm_mode){
		if (debug_)
			cout << "Alarm_mode " << alarm_mode << ", dest(" << x_dest <<
				"," << y_dest << "," << z_dest << ") not accepted" << std::endl; 

	}else{

		if (!waypoints.empty()){
			if (waypoints[0][0] == x_dest && waypoints[0][1] == y_dest && 
				waypoints[0][2] == z_dest){

				if (debug_)
        			cout << "New dest next waypoint in the queue (" << waypoints[0][0]
						<< "," << waypoints[0][1] << "," << waypoints[0][2] <<
						")"<< std::endl; 

			}else{

				waypoints.insert(waypoints.begin(),{x_dest,y_dest,z_dest});
				if (debug_)
					cout << "New destination (" << x_dest << "," << y_dest << "," 
						<< z_dest << "), skip the queue" << std::endl; 


			}

		}else{		

			waypoints.push_back({x_dest,y_dest,z_dest});
		}
		
		
		UWSMPosition::setdest(x_dest,y_dest,z_dest);


		if (debug_)
			cout << "Pos (" << x_ << "," << y_ << "," << z_ << "), new dest(" << 
				Xdest_ << "," << Ydest_ << "," << Zdest_ << ")"<< std::endl; 

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
				cout << "New waypoint (" << x_dest << "," << y_dest << "," 
					<< z_dest << ")\n";

		}
		
				
	}else{
		UWSMWPPosition::setDest(x_dest,y_dest,z_dest, speed_setted);
		if (debug_)
			cout << "Pos (" << x_ << "," << y_ << "," << z_ << "), new dest(" 
				<< Xdest_ << "," << Ydest_ << "," << Zdest_ << ")\n";

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
				cout << "New waypoint (" << x_dest << "," << y_dest << "," 
					<< z_dest << ")\n";

		}
		
				
	}else{

		UWSMWPPosition::setDest(x_dest,y_dest,z_dest);

		if (debug_)
			cout << "Pos (" << x_ << "," << y_ << "," << z_ << "), new dest("
				<< Xdest_ << "," << Ydest_ << "," << Zdest_ << ")\n";

	}
		
}

void
UWSMWPPosition::update(double now)
{
	if ((trgTime_ < 0.) || (now < lastUpdateTime_ + 1e-6))
		return;

	double gamma;
	double theta;
	double theta_den = sqrt(pow(Ydest_ - Ysorg_, 2.0) +				
			pow(Xdest_ - Xsorg_, 2.0) + pow(Zdest_ - Zsorg_, 2.0));

	if (theta_den == 0) {

		x_ = Xsorg_;
		y_ = Ysorg_;
		z_ = Zsorg_;


		if (!waypoints.empty()){

			if(waypoints[0][0] == Xdest_ && waypoints[0][1] == Ydest_ 
				&& waypoints[0][2] == Zdest_){
				
				if (debug_)
					cout << "Last waypoints erased (" << waypoints[0][0] << ","
						<< waypoints[0][1] << "," << waypoints[0][2] << ")\n";


				waypoints.erase(waypoints.begin());

				if (!waypoints.empty()){

					if(waypoints[0].size()>3){
						UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],
							waypoints[0][2],waypoints[0][3]);
					}else{
						UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],
							waypoints[0][2]);
					}

					if (debug_)
						cout << "New dest (" << Xdest_ << "," << Ydest_ << "," 
							<< Zdest_ << ") from waypoints list\n";

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

		double dist_dest_sorg = pow(Ydest_ - Ysorg_, 2.0) + pow(Xdest_ - Xsorg_, 2.0)
			+ pow(Zdest_ - Zsorg_, 2.0);

		double dist_pos_sorg = pow(x_ - Xsorg_, 2.0) + pow(y_ - Ysorg_, 2.0) +
			pow(z_ - Zsorg_, 2.0);

		if ( dist_dest_sorg < dist_pos_sorg ){

			x_ = Xdest_;
			y_ = Ydest_;
			z_ = Zdest_;


			if (debug_)
				cout << "Destination (" << Xdest_ << "," << Ydest_ << "," << 
					Zdest_ << ") reached\n";


			if (!waypoints.empty()){

				if(waypoints[0][0] == Xdest_ && waypoints[0][1] == Ydest_ && 
					waypoints[0][2] == Zdest_){
					
					if (debug_)
						cout << "Last waypoints erased (" << waypoints[0][0] <<
							"," << waypoints[0][1] << "," << waypoints[0][2] << 
							")\n";

					waypoints.erase(waypoints.begin());
				

					if (!waypoints.empty()){

						if (debug_)
								cout << "Next dest (" << waypoints[0][0] << "," 
									<< waypoints[0][1] << "," << waypoints[0][2] 
									<< ")\n";


						if(waypoints[0].size()>3){
							UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],
								waypoints[0][2],waypoints[0][3]);
						}else{
							UWSMWPPosition::setDest(waypoints[0][0],waypoints[0][1],
								waypoints[0][2]);
						}

						if (debug_)
							cout << "New dest (" << Xdest_ << "," << Ydest_ << "," <<
								Zdest_ << ") from waypoints list\n";

					}
				}
			
			}

		}
		if (debug_)
			cout << "New pos (" << x_ << "," << y_ << "," << z_ << "), dest(" << Xdest_ <<
				"," << Ydest_ << "," << Zdest_ << "), source(" << Xsorg_ << "," << Ysorg_ <<
				"," << Zsorg_ << "), speed " << speed_ << " sen(" << gamma << ")=" << 
				sin(gamma) << "\n";

	}

	lastUpdateTime_ = now;
}

void 
UWSMWPPosition::setAlarm(bool alarm)
{
	alarm_mode = alarm;
}
