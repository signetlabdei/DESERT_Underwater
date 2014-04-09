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
 * @file   woss-groupmobility-3d.cpp
 * @author Saiful Azad
 * @version 1.0.0
 *
 * @brief Implementation of GroupMobility-3D for WOSS framework
 */

#include <node-core.h>
#include<rng.h>
#include<cmath>
#include<fstream>
#include<iomanip>
#include<string>
#include "woss-groupmobility-3d.h"



/**
 * TCL Hooks for the simulator
 */
static class WossGroupMob3DClass : public TclClass {
public:
	/**
	 * Class constructor
	 */
	WossGroupMob3DClass() : TclClass("WOSS/GroupMob3D") {}
	TclObject* create(int, const char*const*) {
		return (new WossGroupMob3D);
	}
} class_wossgroupmob3D;

void WossGroupMob3D::UpdatePositionTimer::expire(Event* e) {
  
  if (mod->wossgroup_debug_) cout << NOW << " WossGroupMob3D::UpdateTimerPosition(" << mod->maddr << "), timer expire, curr_state = UpdatePositionTime(), next_state = update()" << endl;
    
  mod -> update();
  
}

WossGroupMob3D::WossGroupMob3D() : 
	update_position_timer(this),
	xFieldWidth_(0),
	yFieldWidth_(0),
	zFieldWidth_(0.0),
	alpha_(0),
	speedMean_(0),
	directionMean_(0),
	bound_(REBOUNCE),
	updateTime_(0),
	wossgroup_debug_(0),
	speed_(0),
	maddr(-1),
	start_latitude(0.0),
	start_longitude(0.0),
	start_x(0.0),
	start_y(0.0),
	mtrace_(0),
	mtrace_of_node_(0),
	pitch_(0.0),
	pitchMean_(0.0),
	sigmaPitch_(0.0),
	speedM_(0.0),
	speedS_(0.0),
	eta_(0.0),
	charge_(0.0),
	beta_(0.0),
	direction_(0.0),
	galpha_(0.0),
	count(0),
	zmin_(0.0),
	vx(0.0),
	vy(0.0),
	vz(0.0),
	x_coord(0.0),
	y_coord(0.0),
	z_coord(0.0),
	newx(0),
	newy(0),
	newz(0),
	leader_(0)
{
	bind("xFieldWidth_", &xFieldWidth_);
	bind("yFieldWidth_", &yFieldWidth_);
	bind("zFieldWidth_", &zFieldWidth_);
	bind("alpha_", &alpha_);
	bind("updateTime_", &updateTime_);
	bind("directionMean_", &directionMean_);
	bind("pitchMean_", &pitchMean_);
	bind("sigmaPitch_", &sigmaPitch_);
	bind("zmin_", &zmin_);
	bind("wossgroup_debug_", &wossgroup_debug_);
	bind("speedM_", &speedM_);
	bind("speedS_", &speedS_);
	bind("eta_", &eta_);
	bind("charge_", &charge_);
	bind("leaderCharge_", &leaderCharge_);
	bind("galpha_", &galpha_);
	
	count = 0;
	update_position_timer.schedule (1.0);
}

WossGroupMob3D::~WossGroupMob3D()
{
}

int WossGroupMob3D::command(int argc, const char*const* argv)
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
							fprintf(stderr,"GMPosition::command(%s), unrecognized bound_ type (%s)\n",argv[1],argv[2]);
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
		if(strcasecmp(argv[1], "leader") == 0)
		{
			WossPosition* lead = (WossPosition*)TclObject::lookup(argv[2]);
			leader_ = lead;
			if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::command(" << maddr << "), pointer of the leader is: " << leader_ << endl;
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "gm3dGroupTraceFile") == 0)
		{
			gm3dGroupTraceFile = argv[2];
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

double WossGroupMob3D::distance(WossPosition* pos1, WossPosition* pos2) 
{
  double xdiff, ydiff, zdiff;
  double dist;
  
  xdiff = pos1->getX() - pos2->getX();
  ydiff = pos1->getY() - pos2->getY();
  zdiff = pos1->getZ() - pos2->getZ();
  
  if (bound_==SPHERIC) {
    
    xdiff = (fabs(xdiff)<((xFieldWidth_)/2))? xdiff : (xFieldWidth_ - fabs(xdiff));
    ydiff = (fabs(ydiff)<((yFieldWidth_)/2))? ydiff : (yFieldWidth_ - fabs(ydiff));
    zdiff = (fabs(zdiff)<((zFieldWidth_)/2))? zdiff : (zFieldWidth_ - fabs(zdiff));
  
  }
  
  dist = (sqrt(xdiff*xdiff + ydiff*ydiff + zdiff*zdiff));
  
  if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::distance(" << maddr << "), distance between leader and follower is: " << dist << endl;
  
  return dist;

}

double WossGroupMob3D::mirror_posx(double x_coord_node, double x_coord_leader) 
{
  if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::mirror_posx(" << maddr << "), calculating mirror position x " << endl;
  
  double d, dref;
  
  d = fabs(x_coord_node - x_coord_leader);
  dref = (xFieldWidth_)/2.0;
  
  if ((d>dref) && (bound_==SPHERIC))
    return ( x_coord_leader - (xFieldWidth_)*(sgn(x_coord_leader - x_coord_node)) );
  return x_coord_leader;
  
}

double WossGroupMob3D::mirror_posy(double y_coord_node, double y_coord_leader) 
{
  if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::mirror_posy(" << maddr << "), calculating mirror position y " << endl;
  
  double d, dref;
  
  d = fabs(y_coord_node - y_coord_leader);
  dref = (yFieldWidth_)/2.0;
  
  if ((d>dref) && (bound_==SPHERIC))
    return ( y_coord_leader - (yFieldWidth_)*(sgn(y_coord_leader - y_coord_node)) );
  return y_coord_leader;
  
}

double WossGroupMob3D::mirror_posz(double z_coord_node, double z_coord_leader) 
{
  if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::mirror_posz(" << maddr << "), calculating mirror position z " << endl;
  
  double d, dref;
  
  d = fabs(z_coord_node - z_coord_leader);
  dref = (zFieldWidth_)/2.0;
  
  if ((d>dref) && (bound_==SPHERIC))
    return ( z_coord_leader - (zFieldWidth_)*(sgn(z_coord_leader - z_coord_node)) );
  return z_coord_leader;
  
}

double WossGroupMob3D::Gaussian()
{
	double x1, x2, w, y1;
	static double y2;
	static int cache = 0;
	
	if (wossgroup_debug_) cout << "value of cache(" << maddr << "): " << cache << endl;
	
	if (cache)             
	{
		y1 = y2;
		cache = 0;
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
		cache = 1;
	}
	if (wossgroup_debug_) cout << "value of y1(" << maddr << "): " << y1 << endl;
	return(y1);
}


double WossGroupMob3D::getStartX() 
{
	start_x = (earth_radius - WossPosition::getDepth()) * sin((90.0 - start_latitude) * M_PI / 180.0) * cos (start_longitude * M_PI / 180.0);
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::getStartX(" << maddr << "), values of x0: " << setprecision(12) << start_x << endl;
	return start_x;
  
}

double WossGroupMob3D::getStartY() 
{
	start_y = (earth_radius - WossPosition::getDepth()) * sin((90.0 - start_latitude) * M_PI / 180.0) * sin (start_longitude * M_PI / 180.0);
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::getStartY(" << maddr << "), values of y0: " << setprecision(12) << start_y << endl;
	return start_y;
  
}

void WossGroupMob3D::setLat(double x_coord, double y_coord) 
{
	double lati, longi;
	longi = atan2(y_coord,x_coord) * 180.0 / M_PI;
	lati = 90.0 - (asin ( x_coord / ((earth_radius - WossPosition::getDepth()) * cos ( longi * M_PI / 180.0 ))) * 180.0 / M_PI);
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::setLatitude(" << maddr << "), new latitude value: " << setprecision(12) << lati << endl;
	WossPosition::setLatitude(lati);
}

void WossGroupMob3D::setLong(double x_coord, double y_coord) 
{
	double longi;
	longi = atan2(y_coord,x_coord) * 180.0 / M_PI;
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::setLongitude(" << maddr << "), new longitude value: " << setprecision(12) << longi << endl;
	WossPosition::setLongitude(longi);
}

void WossGroupMob3D::update()
{
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), updating value. " << endl;
	
	double pr_pitch_;

		// calculate new sample of speed and direction
		if (wossgroup_debug_)
			printf("GMPosition::Update(%d), Update at (%.3f) old speed %.2f old direction %.2f old pitch %.2f\n", maddr, NOW, speed_, direction_, pitch_);
		
		if (wossgroup_debug_) printf ("previous x-coord(%d) value: %.6f \n", maddr, WossPosition::getX());
		x_coord = WossPosition::getX() - getStartX();
		if (wossgroup_debug_) printf ("x-coord(%d) value: %.6f \n", maddr, x_coord);
		
		if (wossgroup_debug_) printf ("previous y-coord(%d) value: %.6f \n", maddr, WossPosition::getY());
		y_coord = WossPosition::getY() - getStartY();
		if (wossgroup_debug_) printf ("y-coord(%d) value: %.6f \n", maddr, y_coord);
		
		if (wossgroup_debug_) printf ("previous z-coord(%d) value: %.6f \n", maddr, WossPosition::getDepth());
		z_coord = WossPosition::getDepth();
		if (wossgroup_debug_) printf ("z-coord(%d) value: %.6f \n", maddr, z_coord);
	
	if (speedMean_ == 0) {
		newx = x_coord;
		newy = y_coord;
		newz = z_coord;
	}		
	else {
		speed_ = (alpha_*speed_) + (((1.0-alpha_))*speedMean_) + (sqrt(1.0-pow(alpha_,2.0))*Gaussian());
		direction_ = (alpha_*direction_) + (((1.0-alpha_))*directionMean_) + (sqrt(1.0-pow(alpha_,2.0))*Gaussian());
		pitch_ = Gauss(pitchMean_, sigmaPitch_, 0);
		
		pr_pitch_ = pitch_;
		
		if (wossgroup_debug_)
			printf("GMPosition::Update(%d), new speed %.2f new direction %.2f new pitch %.2f\n", maddr, speed_, direction_, pitch_);
		
		//calculate velocity
		
		vx = speed_ * cos(direction_) * cos(pitch_);
		if (wossgroup_debug_) cout << "velocity in x(" << maddr << "): " << vx << endl;
		vy = speed_ * sin(direction_) * cos(pitch_);
		if (wossgroup_debug_) cout << "velocity in y(" << maddr << "): " << vy << endl;
		vz = speed_ * sin(pitch_);
		if (wossgroup_debug_) cout << "velocity in z(" << maddr << "): " << vz << endl;
		
		// calculate new position
		newx = x_coord + (vx * updateTime_);
		if (wossgroup_debug_) cout << "newx(" << maddr << ") value: " << newx << endl;
		newy = y_coord + (vy * updateTime_);
		if (wossgroup_debug_) cout << "newy(" << maddr << ") value: " << newy << endl;
		newz = z_coord + (vz * updateTime_);
		if (wossgroup_debug_) cout << "newz(" << maddr << ") value: " << newz << endl;
		
	//Add leader Attraction
	
	if (leader_ != 0) {
	  
	  double cx, cy, cz, dist, gamma;
	  
	  //gamma = direction_;
	  
	  cx = mirror_posx(newx, (leader_->getX() - getStartX()));
	  cy = mirror_posy(newy, (leader_->getY() - getStartY()));
	  cz = mirror_posz(newz, (leader_->getDepth()));
	  
	  dist = distance(leader_,this);
	  if((galpha_ < 0) && (dist < 1)) dist = 1.0;
	  
	  beta_ = ((1.0 - eta_) * beta_) + (eta_ * Gauss(speedM_,speedS_,0));
	  	  
	  double rho = updateTime_ * beta_ * charge_ * leaderCharge_ * pow(dist, -galpha_);
	  
	  if ((cx - newx) == 0) direction_ = pi / 2 * sgn (cy - newy);
	  else direction_ = atan ((cy - newy) / (cx - newx));
	  if ((cx - newx) < 0.0) direction_ += (newy - cy) >= 0.0 ? pi : - pi;
	  if (((cx - newx) == 0) && ((cy - newy) == 0) && ((cz - newz) == 0)) pitch_ = pitch_ / pi;
	  
	  if (wossgroup_debug_) cout << "Value of rho: " << rho << endl;
	  if (wossgroup_debug_) cout << "Value of direction: " << gamma << endl;
	  
	  if (pr_pitch_ == pitch_) {
	    newx += rho * cos(direction_);
	    if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new x-coord value (after leader attraction): " << setprecision(12) << newx << endl;
	    newy += rho * sin(direction_);
	    if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new y-coord value (after leader attraction): " << setprecision(12) << newy << endl;
	    } 
	  else {
	    newx += rho * cos(direction_) * cos(pitch_);
	    if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new x-coord value (after leader attraction): " << setprecision(12) << newx << endl;
	    
	    newy += rho * sin(direction_) * cos(pitch_);
	    if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new x-coord value (after leader attraction): " << setprecision(12) << newy << endl;
	    
	    newz += rho * sin(pitch_);
	    if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new z-coord value (after leader attraction): " << setprecision(12) << newz << endl;
	    }
	  
	  }
	
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
			}else{
				newy = 0 - newy;
				y_coord = 0 - y_coord;
			}
			direction_ *= -1;
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
	
	if ((newx < 0.0)||(newx > xFieldWidth_)||(newy < 0.0)||(newy > yFieldWidth_)||(newz < zmin_)||(newz > zFieldWidth_)) {
	  cout << NOW << " WossGroupMob3D::update(" << maddr << "), error in postion! Abort!! " << endl;
	  update();
	}
	
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new x-coord value (after wrapping): " << setprecision(12) << newx << endl;
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new y-coord value (after wrapping): " << setprecision(12) << newy << endl;
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new z-coord value (after wrapping): " << setprecision(12) << newz << endl;
			
	x_coord = newx + getStartX();
	Position::setX(x_coord);
	y_coord = newy + getStartY();
	Position::setY(y_coord);
	z_coord = newz;
	Position::setZ(z_coord);
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new cartesian x-coord value: " << setprecision(12) << x_coord << endl;
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new cartesian y-coord value: " << setprecision(12) << y_coord << endl;
	if (wossgroup_debug_) cout << NOW << " WossGroupMob3D::update(" << maddr << "), new depth value: " << setprecision(12) << z_coord << endl;
	
	directionMean_ = direction_;
      }
	
	setLat(x_coord,y_coord);
	setLong(x_coord,y_coord);
	WossPosition::setDepth(z_coord);

	if (wossgroup_debug_)
		printf(" WossGroupMob3D::Update(%d), now %f, updateTime %f\n", maddr,NOW, updateTime_);
	
	if (mtrace_) {
	  if (maddr == mtrace_of_node_) {
	    ofstream saveInFile(gm3dGroupTraceFile.c_str(), ios::app);
	    saveInFile << NOW << "\t" << setprecision(12) << newx << "\t" << setprecision(12) << newy << "\t" << WossPosition::getDepth() << endl;
	    saveInFile.close();
	  }
	}
	
	update_position_timer.schedule (updateTime_);
}