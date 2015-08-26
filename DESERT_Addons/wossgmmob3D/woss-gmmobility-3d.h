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
 * @file   woss-gmmobility-3d.h
 * @author Saiful Azad
 * @version 1.0.0
 * 
 * @brief This is a Gauss-Markov random mobility model designed to use with WOSS (World Ocean Simulation System). 
 * Details can be found in the following reference
 * Tracy camp, Jeff Boleng, and Venessa Davies, "A survey of mobility models for ad hoc network research," Wireless Communications and Mobile Computing,
 * 2002, 2:483-502, DOI: 10.1002/wcm.72.
 */


#ifndef _WOSS_GMMOBMODEL_3D_
#define _WOSS_GMMOBMODEL_3D_

#include <iostream>
#include <cmath>
#include <string>
#include <plugin.h>
#include <uw-woss-position.h>
#include "uwrandomlib.h"


#define sgn(x) ( ((x)==0.0) ? 0.0 : ((x)/fabs(x)) )
#define pi (4*atan(1.0))
#define earth_radius 6371000.0

//extern double Gauss(double,double,int);         // gaussian random variable with average standard deviation 

/**
 * This class implements the Gauss Markov mobility model. Movement occurs by 
 * updating the speed and the direction only when an information regarding 
 * node position is required, in this case, if is necessary, all the uncalculated
 * previous states are computed according to a finite state Markov process.
 * The model is designed to adapt to different level of randomness via one tuning
 * parameter (alpha). Initially each node is assigned a current speed and direction,
 * which will be updated during the simulation.
 */
class WossGMMob3D : public WossPosition
{
	public:
		/**
		* Constructor of WossGMMob3D class 
		*/
		WossGMMob3D();
		/**
		* Destructor of WossGMMob3D class
		*/
		virtual ~WossGMMob3D();
		/**
		* Method that return the current projection of the node on the x-axis.
		* If it's necessary (updating time ia expired), update the position values 
		* before returns it.
		*/
		virtual double getStartX();
		
		/**
		* Method that return the current projection of the node on the y-axis.
		* If it's necessary (updating time ia expired), update the position values 
		* before returns it.
		*/
		virtual double getStartY();
		
		/**
		* Enumeration BoundType. Defines the behaviour of the node when it reaches an edge of the simulated field.
		*/
		enum BoundType { SPHERIC = 1, THOROIDAL, HARDWALL, REBOUNCE };
			
		/**
		* Method that return the starting curtesian coordinates of the node on the x-axis and y-axis.
		* It converts the starting latitude and longitude given in the tcl script into cartesian coordinates.
		*/
		virtual void setLat(double x_coord, double y_coord);
		virtual void setLong(double x_coord, double y_coord);
		
		double newx, newy, newz; /**< new position of a node respectively in x-axis, y-axis and z-axis. */
		double vx, vy, vz; /**< new velocity of a node respectively in x-axis, y-axis and z-axis. */
		double x_coord, y_coord, z_coord; /**< Previous position of the node. */
			
		
		/**
		* TCL command intepreter
		* Moreover it inherits all the OTcl method of Position
		* 
		* 
		* @param argc number of arguments in <i>argv</i>
		* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
		* 
		* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
		* 
		**/
		virtual int command(int argc, const char*const* argv);
		
	protected:
		
		/**
		* Base class of UpdateTimerPosition class which is inherited from TimerHandler class.
		*/
		class UpdateTimerPosition : public TimerHandler
		{
		  public:
		    
		    /**
		    * Constructor of UpdateTimerPosition class
		    */
		    UpdateTimerPosition(WossGMMob3D* m) : TimerHandler() { module = m; }
		    
		    /**
		    * Destructor of UpdateTimerPosition class
		    */
		    virtual ~UpdateTimerPosition() {}
		    
		    /**
		    * Update Time is scheduled using this method.
		    */
		    virtual void schedule( double val) { resched(val); }
		    
		  protected:
		
		    virtual void expire(Event* e); /**< This method tell the node what to do when update timer expire. */
		    
		    WossGMMob3D* module; /**< An Object pointer of WossGMMob3D class. */
		
		};
		
		/**
		* Method that updates both the position coordinates as function of the number 
		* of states to be evaluated.
		*/
		virtual void update();
		
		/** 
		* Method that returns a value from a normal random Gaussian variable.
		*
		*/
		double Gaussian();
		
		
		double xFieldWidth_; /**< Range of the x-axis of the field to be simulated */
		
		double yFieldWidth_; /**< Range of the y-axis of the field to be simulated */
		
		double zFieldWidth_; /**< Range of the z-axis of the field to be simulated */
		
		double alpha_;	     /**< Parameter to be used to vary the randomness. 0: totally random values (Brownian motion). 1: linear motion */
		
		double alphaPitch_;  /**< Parameter to be used to vary the randomness in z-axis. */
  
		double speedMean_;   /**< Defines the mean value of the speed. When it is setted to zero the node moves anyway */
													
		double directionMean_; /**< Defines the mean value of the direction */
		
		double pitchMean_; /**< Defines the mean value of the shifting in z-axis */
		
		double sigmaPitch_; /**< Standard deviation in the z-axis */
		
		BoundType bound_;  /**< Defines the behaviour of the node when it reaches the edge.
				   * SPHERIC: return in the simulation field on the opposite side
				   * THOROIDAL: return in the centre of simulation field
				   * HARDWALL: the movement is stopped in the edge
				   * REBOUNCE: the node rebounce (i.e., the movement that should be outside the simulation field is mirrored inside)
				   */
				   
		double updateTime_; /**< Time between two update computation */
		
		double nextUpdateTime_;	/**< Intenal variable used to evaluate the steps to be computed */
		
		double speed_;	/**< Current value of the speed */
		
		double direction_; /**< Current value of the direction */
		
		int wossgm_debug_; /**< Debug flag */
		
		int maddr; /**< Mac address of the node whose movement we would like to trace */
		
		double start_latitude; /**< Starting latitude of the simualted area */
		
		double start_longitude; /**< Starting longitude of the simualted area */
		
		double start_x; /**< Internal variable */
		
		double start_y; /**< Internal variable */
		
		int mtrace_; /**< Flag to enable trace */
		
		int mtrace_of_node_; /**< The node whose movement pattern we want to trace */
		
		double pitch_; /**< Current value of the pitch */
		
		double zmin_; /**< Minimum z-axis value */
		
		string gm3dTraceFile; /**< Trace file information */
		
		UpdateTimerPosition update_timer_position; /**< An object of UpdateTimerPosition class */

		Uwrandomlib randlib;
};

#endif /*_WOSS_GMMOBMODEL_3D_*/
