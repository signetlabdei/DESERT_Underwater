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
 * @file   uwPosEstimation.h
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Estimate position given initial point, end point and time
 *
 */

#ifndef UW_POS_EST_H
#define UW_POS_EST_H

#include "node-core.h"
#include <iostream>
#include <cmath>

#define pi (4 * atan(1.0))

class UwPosEstimation : public Position
{
public:
	/**
	 * Constructor of UwPosEstimation class
	 */
	UwPosEstimation();

	/**
	 * Destructor of UwPosEstimation class
	 */
	virtual ~UwPosEstimation();

	/**
	 *	Update initial position,  destination, time of last update
	 *
	 *  @param Position new initial position
	 *  @param Position new destination
	 *  @param double time of update
	 *  @param double speed of the node
	 */
	virtual void update (Position newInitPos, Position newDest, double newTime, double newSpeed);

	/**
	 *	Get position estimation at a given time
	 *
	 *  @param double time in which estimate position
	 */
	virtual Position getEstimatePos(double time);

	/**
	 *	Get itnitial position of last update
	 */
	virtual Position getInitPos();

	/**
	 *	Get destination of last update
	 */
	virtual Position getDest();

	/**
	 *	Get time of last update
	 */
	virtual double getTimestamp();

	/**
	 *	Get speed of the node
	 */
	virtual double getSpeed();

private:
	/**
	* Compute absoulute distance between 2 nodes
	*
	* @param Position position first node.
	* @param Position position second node.
	*/
	virtual double nodesDistance(Position& p1, Position& p2);

	Position initialPos; /**<Initial position related to last update. */
	Position destPos; /**<Destination of last update. */
	double timestamp; /**<time of last update. */
	double speed; /**<speed of the node. */
};

#endif //UW_POS_EST_H
