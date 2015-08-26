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
 * @file   woss-groupmobility-3d.h
 * @author Saiful Azad
 * @version 1.0.0
 *
 * @brief This is a implementation of a leader-follower mobility model for WOSS (World Ocean Simulation System). Details of this technique is given in paper, 
 * named "A Group Mobility Model Based on Nodes' Attraction for Next Generation Wireless Networks", by Leonardo Badia and Nicola Bui, In proc. of Mobility '06,
 * ACM New York, NY, USA, DOI: 10.1145/1292331.1292402
 */


#ifndef _WOSS_GROUPMOBMODEL_3D_
#define _WOSS_GROUPMOBMODEL_3D_

#include <iostream>
#include <cmath>
#include "uw-woss-position.h"
#include <plugin.h>

#include "uwrandomlib.h"

#define sgn(x) ( ((x)==0.0) ? 0.0 : ((x)/fabs(x)) )
#define pi (4*atan(1.0))
#define earth_radius 6371000.0

/**
* Base class of Group Mobility Model. This is inherited from WossPosition Class.
*/

class WossGroupMob3D : public WossPosition
{
  
	public:
		/**
		* Constructor of WossGroupMob3D class
		*/
		
		WossGroupMob3D();
		/**
		* Destructor of WossGroupMob3D class
		*/
		virtual ~WossGroupMob3D();
		
		/**
		* \enum BoundType
		* \brief Defines the behaviour of the node when it reaches an edge of the simulation field
		*/
		enum BoundType { SPHERIC = 1, THOROIDAL, HARDWALL, REBOUNCE };

		/**
		* Method that return the starting projection of the node on the cartesian x-axis.
		* If it's necessary (updating time when it is expired), update the position values 
		* before returns it.
		*/
		virtual double getStartX();
		
		/**
		* Method that return the starting projection of the node on the cartesian y-axis.
		* If it's necessary (updating time when it is expired), update the position values 
		* before returns it.
		*/	
		virtual double getStartY();
		
		/**
		* Method that sets the latitude of the node after update.
		* @param changes in x-axis
		* @param changes in y-axis
		*/
		virtual void setLat(double x_coord, double y_coord);
		
		/**
		* Method that sets the longitude of the node after update.
		* @param changes in x-axis
		* @param changes in y-axis
		*/
		virtual void setLong(double x_coord, double y_coord);
		
		/**
		* Attraction charge to the leader. 
		* @return double charge value.
		*/
		virtual double getCharge() { return charge_; }

		double newx, newy, newz; /**< new position of a node respectively in x-axis, y-axis and z-axis. */
		double vx, vy, vz; /**< new velocity of a node respectively in x-axis, y-axis and z-axis. */
		double x_coord, y_coord, z_coord; /**< position of the node. */
		
		/**
		* TCL command intepreter. It inherits all the OTcl method of Position 
		* @param argc number of arguments in <i>argv</i>
		* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
		* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
		*/
		virtual int command(int argc, const char*const* argv);
		
	protected:
	  
	  	/**
		* Base class of UpdateTimerPosition class which is inherited from TimerHandler class.
		*/
		class UpdatePositionTimer : public TimerHandler
		{
		  public:
		    
		    /**
		    * Constructor of UpdateTimerPosition class
		    */
		    UpdatePositionTimer(WossGroupMob3D* mo) : TimerHandler() { mod = mo; }
		    
		    /**
		    * Destructor of UpdateTimerPosition class
		    */
		    virtual ~UpdatePositionTimer() {}
		    
		    /**
		    * Update Time is scheduled using this method.
		    */
		    virtual void schedule( double val) { resched(val); }
		    
		  protected:
		    
		    virtual void expire(Event* e); /**< This method tell the node what to do when update timer expire. */
		    
		    WossGroupMob3D* mod; /**< An Object pointer of WossGroupMob3D class. */
		
		};
		/**
		* Method that updates both the position coordinates as function of the number 
		* of states to be evaluated.
		*/
		virtual void update();
		
		/** 
		* Method that returns a value from a normal random Gaussian variable (zero mean, unitary viariance)
		*
		*/	
		double Gaussian();
		
		/**
		* Calculate the distance between two nodes. 
		* @param pointer of node position 1
		* @param pointer of node position 2
		* @return double distance value
		*/
		double distance(WossPosition* pos1, WossPosition* pos2);
		
		/**
		* Approximate position at x-axis
		* @param x-axis value of follower
		* @param x-axis value of leader
		* @return double type value
		*/
		double mirror_posx(double x_coord_node, double x_coord_leader);
		
		/**
		* Approximate position at y-axis
		* @param y-axis value of follower
		* @param y-axis value of leader
		* @return double type value
		*/
		double mirror_posy(double y_coord_node, double y_coord_leader);
		
		/**
		* Approximate position at z-axis
		* @param z-axis value of follower
		* @param z-axis value of leader
		* @return double type value
		*/
		double mirror_posz(double z_coord_node, double z_coord_leader);
	
		
		double xFieldWidth_; /**< Range of the x-axis of the field to be simulated */
		
		double yFieldWidth_; /**< Range of the y-axis of the field to be simulated */
		
		double zFieldWidth_; /**< Range of the z-axis of the field to be simulated */
		
		double alpha_;	     /**< Parameter to be used to vary the randomness. 0: totally random values (Brownian motion). 1: linear motion */
  
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
		
		double speed_;	/**< Current value of the speed */
		
		double direction_; /**< Current value of the direction */
		
		int wossgroup_debug_; /**< Debug flag */

		int maddr; /**< Mac address of the node whose movement we would like to trace */
		
		double start_latitude; /**< Starting latitude of the simualted area */
		
		double start_longitude; /**< Starting longitude of the simualted area */
		
		double start_x; /**< Internal variable */
		
		double start_y; /**< Internal variable */
		
		int mtrace_; /**< Flag to enable trace */
		
		int mtrace_of_node_; /**< The node whose movement pattern we want to trace */
		
		double pitch_; /**< Current value of the pitch */
		
		double zmin_; /**< Minimum z-axis value */

		double speedM_; /**< Mean of the speed which is used to compute a Gaussian random variable.*/
		
		double speedS_; /**< Standard deviation of speed which is also used to compute a Gaussian random variable */
		
		double eta_; /**< A tunable variable which is the coefficient of the filter in that range between 0 and 1 */
		
		double beta_; /**< A variable which is employed to calculate attraction force towards the leader */
		
		double charge_; /**< Attraction charge of the follower */ 
		
		double leaderCharge_; /**< Attraction charge of the leader */
		
		double galpha_; /**< It tells the intensity of the attraction filed */
		
		int count; /**< A counting variable */

		string gm3dGroupTraceFile; /**< Trace file information */
		
		WossPosition* leader_; /**< Position pointer of the leader */
		
		UpdatePositionTimer update_position_timer; /**< An object of UpdateTimerPosition class */

		Uwrandomlib randlib;
};

#endif		// _WOSS_GROUPMBMODEL_3D_


