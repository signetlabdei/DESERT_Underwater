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
 * @file   uwsmwpposition.h
 * @author Alessia Ortile
 * @version 1.0.0
 *
 * \brief Provides the definition of the class <i>UWSMWPPosition</i>.
 *
 * Provides the definition of the class <i>UWSMWPPosition</i>.
 * This class implements the a simple movement behaviour: it is possible to
 * define the direction and the speed of the linear movement thanks to a TCL 
 * command in which the user has to define the destination point an the 
 * speed of the movement required. Additionally, this class supports the 
 * addition of a list of waypoints to reach sequentially.
 * NOTE: the destination point is used to define the direction od the node and
 * when it is reached the node will proceed for the same direction
 *
 * @see NodeCore, Position
 **/

#ifndef _UWSMEPOSITION_
#define _UWSMEPOSITION_

#include <node-core.h>
#include <uwsmposition.h>
#include <vector>

#define sgn(x) (((x) == 0.0) ? 0.0 : ((x) / fabs(x)))
#define pi (4 * atan(1.0))

class UWSMWPPosition;

class UWSMWPPosition : public UWSMPosition
{
public:
	/**
	* Constructor
	*/
	UWSMWPPosition();
	/**
	* Destructor
	*/
	virtual ~UWSMWPPosition();
	/**
	

	/**
	* TCL command interpreter
	*  <li><b>setDest &lt;<i>integer value</i>&gt;<i>integer
	*value</i>&gt;<i>integer value</i>&gt;</b>:
	*  	set the movement pattern: the firts two values define the point to be
	*reached (i.e., the
	*  	direction of the movement) and the third value defines the speed to be
	*used
	* </ul>
	*
	* Moreover it inherits all the OTcl method of Position
	*
	*
	* @param argc number of arguments in <i>argv</i>
	* @param argv array of strings which are the comand parameters (Note that
	*argv[0] is the name of the object)
	*
	* @return TCL_OK or TCL_ERROR whether the command has been dispatched
	*succesfully or no
	*
	**/
	virtual int command(int argc, const char *const *argv);


	/**
 	* Updates the next destination to reach.
	*
 	* @param x_dest The x-coordinate of the destination.
 	* @param y_dest The y-coordinate of the destination.
 	* @param z_dest The z-coordinate of the destination.
 	* @param speed The speed of the vehicle.
 	*/
	virtual void setDest(
			double x_dest, double y_dest, double z_dest, double speed);

	/**
 	* Adds a new waypoint to the queue of destinations.
	*
 	* @param x_dest The x-coordinate of the waypoint.
 	* @param y_dest The y-coordinate of the waypoint.
 	* @param z_dest The z-coordinate of the waypoint.
 	* @param speed The speed of the vehicle.
 	*/		

	virtual void addDest(
			double x_dest, double y_dest, double z_dest, double speed);

	/**
 	* Updates the next destination to reach.
	*
 	* @param x_dest The x-coordinate of the destination.
 	* @param y_dest The y-coordinate of the destination.
 	* @param z_dest The z-coordinate of the destination.
 	*/

	virtual void setDest(double x_dest, double y_dest, double z_dest);

	/**
 	* Adds a new waypoint to the queue of destinations.
	*
 	* @param x_dest The x-coordinate of the waypoint.
 	* @param y_dest The y-coordinate of the waypoint.
 	* @param z_dest The z-coordinate of the waypoint.
 	* @param speed The speed of the vehicle.
 	*/

	virtual void addDest(double x_dest, double y_dest, double z_dest);

	/**
 	* Updates the state of the vehicle. If the alarm mode is set to true, 
 	* new destinations cannot be set until it is turned off.
	*
 	* @param  alarmStatus status of the alarm mode.
 	*/
	
	virtual void setAlarm(bool alarmStatus);


protected:
	/**
	* Method that updates both the position coordinates
	* @param now time
	*/
	virtual void update(double now);

private:
	
	std::vector<std::vector<double>> waypoints; /**< queue of successive destination to reach*/
	bool alarm_mode; /**< statuse of the alarm mode, if true block all the application from updating the destination*/	
	int debug_;	
};


#endif
