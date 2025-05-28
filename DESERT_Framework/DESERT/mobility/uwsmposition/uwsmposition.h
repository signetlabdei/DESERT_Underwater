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
 * @file   uwsmposition.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Provides the definition of the class <i>UWSMPosition</i>.
 *
 * Provides the definition of the class <i>UWSMPosition</i>.
 * This class implements a simple movement behaviour: it is possible to
 * define the direction and the speed of a linear movement with a TCL
 * command in which the user has to define the destination point and the speed
 * of the movement required.
 * NOTE: the destination point is used to define the direction of the node and
 * when it is reached the node will proceed on the same direction.
 *
 * @see NodeCore, Position
 **/

#ifndef _UWSMPOSITION_
#define _UWSMPOSITION_

#include <node-core.h>

#define sgn(x) (((x) == 0.0) ? 0.0 : ((x) / fabs(x)))
#define pi (4 * atan(1.0))

class UWSMPosition : public Position
{
public:
	/**
	 * UWSMPosition constructor
	 */
	UWSMPosition();

	/**
	 * UWSMPosition destructor
	 */
	virtual ~UWSMPosition() = default;

	/**
	 * Get the current projection on x-axis of the node postion
	 * If necessary (updating time expired), update the position values.
	 *
	 * @return double Current projection on x-axis of the node postion
	 */
	virtual double getX() override;

	/**
	 * Get the current projection on y-axis of the node postion
	 * If necessary (updating time expired), update the position values.
	 *
	 * @return double Current projection on y-axis of the node postion
	 */
	virtual double getY() override;

	/**
	 * Get the current projection on z-axis of the node postion
	 * If necessary (updating time expired), update the position values.
	 *
	 * @return double Current projection on z-axis of the node postion
	 */
	virtual double getZ() override;

	/**
	 * Get the x coordinate of the destination point
	 *
	 * @return double Coordinate along x-axis of destination point
	 */
	virtual double getXdest() const;

	/**
	 * Get the y coordinate of the destination point
	 *
	 * @return double Coordinate along y-axis of destination point
	 */
	virtual double getYdest() const;

	/**
	 * Get the z coordinate of the destination point
	 *
	 * @return double Coordinate along z-axis of destination point
	 */
	virtual double getZdest() const;

	/**
	 * Get the current speed of the node.
	 *
	 * @return double Current speed of the node
	 */
	double getSpeed() const;

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * <ul>
	 * <li><b>setdest &lt;<i>x</i>&gt; &lt;<i>y</i>&gt; &lt;<i>z</i>&gt; &lt;<i>speed</i>&gt;</b>
	 * <li><b>setdest &lt;<i>x</i>&gt;&lt; <i>y</i>&gt;&lt; <i>z</i>&gt;</b>
	 * <li><b>update</b>
	 * </ul>
	 * Moreover it inherits all the OTcl method of Position
	 *
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the comand parameters (Note that
	 * argv[0] is the name of the object)
	 *
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * succesfully or not.
	 *
	 */
	virtual int command(int argc, const char *const *argv) override;

	/*
	 * Set the destination of the node with a constant speed.
	 *
	 * @param double Coordinate along x-axis of destination point
	 * @param double Coordinate along y-axis of destination point
	 * @param double Coordinate along z-axis of destination point
	 * @param double Speed of the node
	 */
	virtual void setdest(double x_dest, double y_dest, double z_dest, double speed);

	/*
	 * Set the destination of the node with default speed.
	 *
	 * @param double Coordinate along x-axis of destination point
	 * @param double Coordinate along y-axis of destination point
	 * @param double Coordinate along z-axis of destination point
	 * @param double Speed of the node
	 */
	virtual void setdest(double x_dest, double y_dest, double z_dest);

	/*
	 * Check if the set destination is reached or not.
	 *
	 * @return bool True if reached, false otherwise
	 */
	virtual bool isDestReached() const;

	/**
	 * Set the projection on x-axis of the node postion
	 *
	 * @param double Projection on the x-axis of the node position
	 * 
	 **/
	virtual void setX(double x) override;

	/**
	 * Set the projection on y-axis of the node postion
	 *
	 * @param double Projection on the y-axis of the node position
	 * 
	 **/
	virtual void setY(double y) override;

	/**
	 * Set the projection on z-axis of the node postion
	 *
	 * @param double Projection on the z-axis of the node position
	 * 
	 **/
	virtual void setZ(double z) override;

private:
	/**
	 * Update both the current position coordinates.
	 *
	 * @param double Time when to update position
	 */
	virtual void update(double now);

	int debug_;
	double trgTime_; /**< Time when the TCL command <i>setdest</i> is invoked. */
	double lastUpdateTime_; /**< Time when last update of the coordinates was computed. */
	double Xdest_; /**< Position along x-axis of the destination point. */
	double Ydest_; /**< Position along y-axis of the destination point. */
	double Zdest_; /**< Position along z-axis of the destination point. */
	double Xsorg_; /**< Position along x-axis of the starting point. */
	double Ysorg_; /**< Position along y-axis of the starting point. */
	double Zsorg_; /**< Position along z-axis of the starting point. */
	double speed_; /**< Speed of the node. */
};

#endif
