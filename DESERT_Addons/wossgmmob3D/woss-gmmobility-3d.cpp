//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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

/**
 * @file   woss-gmmobility-3d.cpp
 * @author Saiful Azad
 * @version 1.0.0
 * 
 * @brief Implements the Gauss-Markov mobility model
 */

#include<rng.h>
#include<cmath>
#include<fstream>
#include<iomanip>
#include "woss-gmmobility-3d.h"


/**
 * TCL Hooks for the simulator
 */
static class WossGMMob3DClass : public TclClass {
public:
	/**
	 * Constructor of the class
	 */
	WossGMMob3DClass() : TclClass("WOSS/GMMob3D") {}
	TclObject* create(int, const char*const*) {
		return (new WossGMMob3D());
	}
} class_wossgmmob3d;

void WossGMMob3D::UpdateTimerPosition::expire(Event* e) {
  
  if (module->wossgm_debug_) cout << NOW << " WossGMMob3D::UpdateTimerPosition(" << module->maddr << "), timer expire, curr_state = UpdatePositionTime(), next_state = update()" << endl;
    
  module -> update();
  
}

WossGMMob3D::WossGMMob3D() : 
	xFieldWidth_(0),
	yFieldWidth_(0),
	zFieldWidth_(0.0),
	alpha_(0),
	speedMean_(0),
	directionMean_(0),
	bound_(REBOUNCE),
	updateTime_(0),
	speed_(0),
	direction_(0),
	maddr(-1),
	start_latitude(0.0),
	start_longitude(0.0),
	start_x(0.0),
	start_y(0.0),
	nextUpdateTime_(0.0),
	update_timer_position(this),
	mtrace_(0),
	mtrace_of_node_(0),
	pitch_(0.0),
	pitchMean_(0.0),
	vx(0.0),
	vy(0.0),
	vz(0.0),
	alphaPitch_(0.0),
	sigmaPitch_(0.0),
	zmin_(0.0)
{
	bind("xFieldWidth_", &xFieldWidth_);
	bind("yFieldWidth_", &yFieldWidth_);
	bind("zFieldWidth_", &zFieldWidth_);
	bind("alpha_", &alpha_);
	bind("alphaPitch_", &alphaPitch_);
	bind("updateTime_", &updateTime_);
	bind("directionMean_", &directionMean_);
	bind("pitchMean_", &pitchMean_);
	bind("zmin_", &zmin_);
	bind("sigmaPitch_",&sigmaPitch_);
	bind("wossgm_debug_",&wossgm_debug_);
	
	update_timer_position.schedule (1.0);
}

WossGMMob3D::~WossGMMob3D()
{
}

int WossGMMob3D::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 3)
	{
		if(strcasecmp(argv[1], "bound") == 0)
		{
			if (strcasecmp(argv[2],"SPHERIC")==0)
				bound_ = SPHERIC;
			else
			{
				if (strcasecmp(argv[2],"THOROIDAL")==0)
					bound_ = THOROIDAL;
				else
				{
					if (strcasecmp(argv[2],"HARDWALL")==0)
						bound_ = HARDWALL;
					else
					{
						if (strcasecmp(argv[2],"REBOUNCE")==0)
							bound_ = REBOUNCE;
						else
						{
							fprintf(stderr,"WossGMMob3D::command(%s), unrecognized bound_ type (%s)\n",argv[1],argv[2]);
							exit(1);
						}
					}
				}
			}
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "speedMean") == 0)
		{
			speedMean_ = speed_ = atof(argv[2]);
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "maddr") == 0)
		{
			maddr = atoi(argv[2]);
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "startLatitude") == 0)
		{
			start_latitude = atof(argv[2]);
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "startLongitude") == 0)
		{
			start_longitude = atof(argv[2]);
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "mtrace") == 0)
		{
			mtrace_ = atoi(argv[2]);
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "mTraceOfNode") == 0)
		{
			mtrace_of_node_ = atoi(argv[2]);
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "gm3dTraceFile") == 0)
		{
			gm3dTraceFile= argv[2];
			return TCL_OK;
		}
	}
	else if(argc == 2) {
	  if(strcasecmp(argv[1], "start") == 0)
		{
			update();
			return TCL_OK;
		}
	
	}
	
	return Position::command(argc, argv);
}

double WossGMMob3D::getStartX() 
{
	start_x = (earth_radius - WossPosition::getDepth()) * sin((90.0 - start_latitude) * M_PI / 180.0) * cos (start_longitude * M_PI / 180.0);
	if (wossgm_debug_) cout << "values of x0: " << setprecision(12) << start_x << endl;
	return start_x;
  
}

double WossGMMob3D::getStartY() 
{
	start_y = (earth_radius - WossPosition::getDepth()) * sin((90.0 - start_latitude) * M_PI / 180.0) * sin (start_longitude * M_PI / 180.0);
	if (wossgm_debug_) cout << "values of y0: " << setprecision(12) << start_y << endl;
	return start_y;
  
}

void WossGMMob3D::setLat(double x_coord, double y_coord) 
{
	double lati, longi;
	longi = atan2(y_coord,x_coord) * 180.0 / M_PI;
	lati = 90.0 - (asin ( x_coord / ((earth_radius - WossPosition::getDepth()) * cos ( longi * M_PI / 180.0 ))) * 180.0 / M_PI);
	if (wossgm_debug_) cout << "New latitude lati(" << maddr << "): " << lati << endl;
	WossPosition::setLatitude(lati);
}

void WossGMMob3D::setLong(double x_coord, double y_coord) 
{
	double longi;
	longi = atan2(y_coord,x_coord) * 180.0 / M_PI;
	if (wossgm_debug_) cout << "New longitude (" << maddr << "): " << longi << endl;
	WossPosition::setLongitude(longi);
}

double WossGMMob3D::Gaussian()
{
	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;
	if (use_last)             
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do
		{
			x1 = 2.0 * RNG::defaultrng()->uniform_double() - 1.0;
			x2 = 2.0 * RNG::defaultrng()->uniform_double() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}
	return(y1);
}

void WossGMMob3D::update()
{
		if (wossgm_debug_)
			printf("WossGMMob3D::Update(%d), Update at (%.3f) old speed %.2f old direction %.2f \n", maddr, NOW, speed_, direction_);
		
		x_coord = WossPosition::getX() - getStartX();
		if (wossgm_debug_) printf ("x-coord(%d) value: %.6f \n", maddr, x_coord);
		
		y_coord = WossPosition::getY() - getStartY();
		if (wossgm_debug_) printf ("y-coord(%d) value: %.6f \n", maddr, y_coord);
		
		z_coord = WossPosition::getDepth();
		if (wossgm_debug_) printf ("z-coord(%d) value: %.6f \n", maddr, z_coord);
		
		speed_ = (alpha_*speed_) + (((1.0-alpha_))*speedMean_) + (sqrt(1.0-pow(alpha_,2.0))*Gaussian());
		direction_ = (alpha_*direction_) + (((1.0-alpha_))*directionMean_) + (sqrt(1.0-pow(alpha_,2.0))*Gaussian());
		
		pitch_ = randlib.Gauss(pitchMean_, sigmaPitch_, 0);
		
		if (wossgm_debug_)
			printf("WossGMMob3D::Update(%d), new speed %.2f new direction %.2f new pitch %.2f \n", maddr, speed_, direction_, pitch_);
		
		//calculate velocity
		vx = speed_ * cos(direction_) * cos(pitch_);
		if (wossgm_debug_) cout << "velocity in x(" << maddr << "): " << vx << endl;
		
		vy = speed_ * sin(direction_) * cos(pitch_);
		if (wossgm_debug_) cout << "velocity in y(" << maddr << "): " << vy << endl;
		
		vz = speed_*sin(pitch_);
		if (wossgm_debug_) cout << "velocity in z(" << maddr << "): " << vz << endl;
	
		// calculate new position
		newx = x_coord + (vx * updateTime_);
		if (wossgm_debug_) cout << "newx(" << maddr << ") value: " << newx << endl;
		
		newy = y_coord + (vy * updateTime_);
		if (wossgm_debug_) cout << "newy(" << maddr << ") value: " << newy << endl;
		
		newz = z_coord + (vz * updateTime_);
		if (wossgm_debug_) cout << "newz(" << maddr << ") value: " << newz << endl;
		
		if (wossgm_debug_)
			printf("WossGMMob3D::Update(%d), X:%.3f->%.3f Y:%.3f->%.3f Z:%.3f->%.3f \n", maddr, x_coord, newx, y_coord, newy, z_coord, newz);
		// verify whether the new position has to be re-computed in order
		// to maintain node position within the simulation 
		
		if ((newy > yFieldWidth_) || ( newy < 0))
		{
			switch (bound_)
			{
			case SPHERIC:
				y_coord -= yFieldWidth_*(sgn(newy));
				newy -= yFieldWidth_*(sgn(newy));
				break;
			case THOROIDAL:
				x_coord = (xFieldWidth_/2) + x_coord - newx;
				y_coord = (yFieldWidth_/2) + y_coord - newy;
				z_coord = (zFieldWidth_/2) + z_coord - newz;
				newx = xFieldWidth_/2;		
				newy = yFieldWidth_/2;
				newz = zFieldWidth_/2;
				break;
			case HARDWALL:
				newy = newy<0?0:yFieldWidth_; 
				break;
			case REBOUNCE:
				if (newy>yFieldWidth_)
				{
					newy = 2*yFieldWidth_ - newy;
					y_coord = 2*yFieldWidth_ - y_coord;
					//direction_ += pi/4;
				}else{
					newy = 0 - newy;
					y_coord = 0 - y_coord;
					//direction_ -= pi/4;
				}
				direction_ *= -1.0;
				break;
			}
		}
		
		if ((newx > xFieldWidth_) || ( newx < 0))
		{
			switch (bound_)
			{
			case SPHERIC:
				x_coord -= xFieldWidth_*(sgn(newx));
				newx -= xFieldWidth_*(sgn(newx));
				break;
			case THOROIDAL:
				x_coord = (xFieldWidth_/2) + x_coord - newx;
				y_coord = (yFieldWidth_/2) + y_coord - newy;
				z_coord = (zFieldWidth_/2) + z_coord - newz;
				newx = xFieldWidth_/2;
				newy = yFieldWidth_/2;
				newz = zFieldWidth_/2;
				break;
			case HARDWALL:
				newx = newx<0?0:xFieldWidth_;
				break;
			case REBOUNCE:
				if (newx > xFieldWidth_)
				{
					newx = 2*xFieldWidth_ - newx;
					x_coord = 2*xFieldWidth_ - x_coord;
				}else{		  
					newx = 0 - newx;
					x_coord = 0 - x_coord;
				}
				if (newy==y_coord)
				{
					if (newx>x_coord)
						direction_ = 0;
					else
						direction_ = pi;
				}else{
					if (newy>y_coord)
						direction_ = pi - direction_;
					else 
						direction_ = -pi - direction_;
				}
				break;
			}
		}
		
		if ((newz > zFieldWidth_) || ( newz < zmin_))
		{
			switch (bound_)
			{
			case SPHERIC:
				z_coord -= zFieldWidth_*(sgn(newz));
				newz -= zFieldWidth_*(sgn(newz));
				break;
			case THOROIDAL:
				x_coord = (xFieldWidth_/2) + x_coord - newx;
				y_coord = (yFieldWidth_/2) + y_coord - newy;
				z_coord = (zFieldWidth_/2) + z_coord - newz;
				newx = xFieldWidth_/2;
				newy = yFieldWidth_/2;
				newz = zFieldWidth_/2;
				break;
			case HARDWALL:
				newz = newz<0?0:zFieldWidth_;
				break;
			case REBOUNCE:
				if (newz > zFieldWidth_)
				{
					newz = 2*zFieldWidth_ - newz;
					z_coord = 2*zFieldWidth_ - z_coord;
				}else{		  
					newz = 2*zmin_ - newz;
					z_coord = 2*zmin_ - z_coord;
				}
				break;
			}
		}
		
	if ((newx < 0.0)||(newx > xFieldWidth_)||(newy < 0.0)||(newy > yFieldWidth_)||(newz < zmin_)||(newz>zFieldWidth_)) {
	  cout << NOW << " WossGMMob3D::update(" << maddr << "), error in postion! Abort!! " << endl;
	  update();
	}
		
	if (wossgm_debug_) cout << NOW << " WossGMMob3D::Update("<< maddr << "), newx value is: " << newx << endl;
	if (wossgm_debug_) cout << NOW << " WossGMMob3D::Update("<< maddr << "), newy value is: " << newy << endl;
	if (wossgm_debug_) cout << NOW << " WossGMMob3D::Update("<< maddr << "), newy value is: " << newz << endl;
		
	x_coord = newx + getStartX();
	y_coord = newy + getStartY();
	z_coord = newz;
	if (wossgm_debug_) cout << NOW << " WossGMMob3D::Update("<< maddr << "), final x_coord value is: " << x_coord << endl;
	if (wossgm_debug_) cout << NOW << " WossGMMob3D::Update("<< maddr << "), final y_coord value is: " << y_coord << endl;
	if (wossgm_debug_) cout << NOW << " WossGMMob3D::Update("<< maddr << "), final z_coord value is: " << z_coord << endl;
		
	setLat(x_coord,y_coord);
	setLong(x_coord,y_coord);			
	WossPosition::setDepth(z_coord);
		
	directionMean_ = direction_;
		
	if (wossgm_debug_)
		printf("WossGMMob3D::Update(%d), now %f, updateTime %f\n", maddr,NOW, updateTime_);
	
	if (mtrace_) {
	  if (maddr == mtrace_of_node_) {
	    ofstream saveInFile(gm3dTraceFile.c_str(), ios::app);
	    saveInFile << NOW << "\t" << setprecision(12) << newx << "\t" << setprecision(12) << newy << "\t" << setprecision(12) << newz << endl;
	    saveInFile.close();
	  }
	}
	
	update_timer_position.schedule (updateTime_);
}
