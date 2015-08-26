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
 * @file   uwsmposition.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 * 
 * \brief Provides the definition of the class <i>UWSMPosition</i>.
 *
 * Provides the definition of the class <i>UWSMPosition</i>.
 * This class implements the a simple movement behaviour: it is possible to define
 * the direction and the speed of the linear movement thanks to a TCL command
 * in which the user has to define the destination point an the speed of the
 * movement required.
 * NOTE: the destination point is used to define the direction od the node and
 * when it is reached the node will proceed for the same direction
 *
 * @see NodeCore, Position
 **/

#ifndef _UWSMPOSITION_
#define _UWSMPOSITION_

#include <node-core.h>


#define sgn(x) ( ((x)==0.0) ? 0.0 : ((x)/fabs(x)) )
#define pi (4*atan(1.0))


class UWSMPosition : public Position
{
	public:
		/**
		* Constructor
		*/
		UWSMPosition();
		/**
		* Destructor
		*/
		virtual ~UWSMPosition();
		/**
		* Method that return the current projection of the node on the x-axis.
		* If it's necessary (updating time ia expired), update the position values 
		* before returns it.
		*/
		virtual double getX();
		/**
		* Method that return the current projection of the node on the y-axis.
		* If it's necessary (updating time ia expired), update the position values 
		* before returns it.
		*/
		virtual double getY();
		
		/**
		* Method that return the current projection of the node on the z-axis.
		* If it's necessary (updating time ia expired), update the position values 
		* before returns it.
		*/
		virtual double getZ();
		
		/**
		* TCL command interpreter
		*  <li><b>setdest &lt;<i>integer value</i>&gt;<i>integer value</i>&gt;<i>integer value</i>&gt;</b>: 
		*  	set the movement pattern: the firts two values define the point to be reached (i.e., the 
		*  	direction of the movement) and the third value defines the speed to be used
		* </ul>
		* 
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
		virtual void setdest(double x_dest,double y_dest,double z_dest,double spead);
		virtual void setdest(double x_dest,double y_dest,double z_dest);
		virtual void setX(double x);
		virtual void setY(double y);
		virtual void setZ(double z);
	private:
		/**
		* Method that updates both the position coordinates
		*/
		virtual void update(double now);
		
		double trgTime_;						/// time in which the TCL command <i>setdest</i> is invoked
		double lastUpdateTime_;				/// time last updated of the coordinates was computed
		double Xdest_;							/// position on the x-axis of the destination point
		double Ydest_;							/// position on the y-axis of the destination point
		double Zdest_;							/// position on the z-axis of the destination point
		double Xsorg_;							/// position on the x-axis of the starting point (when the TCL command <i>setdest</i> is invoked)
		double Ysorg_;							/// position on the y-axis of the starting point (when the TCL command <i>setdest</i> is invoked)
		double Zsorg_;							/// position on the z-axis of the starting point (when the TCL command <i>setdest</i> is invoked)
		double speed_;							/// speed of the node
		
		int debug_;
};

#endif
