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
 * @file   uwPosEstimation.cpp
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Estimate position given initial point, end point and time
 */

#include "uwPosEstimation.h"

#include <cmath>

UwPosEstimation::UwPosEstimation()
	: initialPos()
	, destPos()
	, timestamp(0)
	, speed(1)
{
}

UwPosEstimation::~UwPosEstimation()
{
}

Position UwPosEstimation::getInitPos()
{
	return initialPos;
}

Position UwPosEstimation::getDest()
{
	return destPos;
}

double UwPosEstimation::getTimestamp()
{
	return timestamp;
}

double UwPosEstimation::getSpeed()
{
	return speed;
}

void UwPosEstimation::update(Position newInitPos, Position newDest, double newTime, double newSpeed)
{
	if (newTime <= timestamp) {
		std::cout << "UwPosEstimation::update, new informations are obslete" 
			<< std::endl;
		return;
	}
	initialPos.setX(newInitPos.getX());
	initialPos.setY(newInitPos.getY());
	initialPos.setZ(newInitPos.getZ());
	destPos.setX(newDest.getX());
	destPos.setY(newDest.getY());
	destPos.setZ(newDest.getZ());
	timestamp = newTime;
	speed = newSpeed;
}

Position UwPosEstimation::getEstimatePos(double time)
{
	double x_ROV = initialPos.getX();
	double y_ROV = initialPos.getY();
	double z_ROV = initialPos.getZ();
	double x_wp = destPos.getX();
	double y_wp = destPos.getY();
	double z_wp = destPos.getZ();
	double x = std::abs(x_ROV - x_wp);
	double y = std::abs(y_ROV - y_wp);
	double z = std::abs(z_ROV - z_wp);
	double rho = sqrt(pow(x,2) + pow(y,2) + pow(z,2));
	double theta = 0; //theta [-pi, pi]
	double psi = 0;	//psi [0, pi]
	double deltaT = time - timestamp;
	double dist = nodesDistance(initialPos,destPos);
	Position estimated_ROV_pos;
	if (speed == 0 || dist/speed < deltaT) {  //waypoint reached
		estimated_ROV_pos.setX(x_wp);
		estimated_ROV_pos.setY(y_wp);
		estimated_ROV_pos.setZ(z_wp);
		return estimated_ROV_pos;
	}

	if (x == 0 && y == 0) { //true both for z=0 and z!=0
		estimated_ROV_pos.setX(x_ROV);
		estimated_ROV_pos.setY(y_ROV);
		estimated_ROV_pos.setZ(z_ROV + deltaT * speed);
	} else {
		psi = acos(z/rho);
		if (x == 0) {
			if (y > 0) {
				theta = pi/2;
			} else {
				theta = -pi/2;
			}
			psi = acos(z/rho);
		} else if (x > 0) {
			theta = atan(y/x);
		} else { // x < 0
			if (y >= 0) {
				theta = atan(y/x) + pi;
			} else {
				theta = atan(y/x) - pi;
			}
		}
	}
	estimated_ROV_pos.setX(x_ROV + deltaT*speed*sin(psi)*cos(theta));
	estimated_ROV_pos.setY(y_ROV + deltaT*speed*sin(psi)*sin(theta));
	estimated_ROV_pos.setZ(z_ROV + deltaT*speed*cos(psi));	

	return estimated_ROV_pos;
}

double UwPosEstimation::nodesDistance(Position& p1, Position& p2)
{
	double x1 = p1.getX();
	double y1 = p1.getY();
	double z1 = p1.getZ();
	double x2 = p2.getX();
	double y2 = p2.getY();
	double z2 = p2.getZ();

	return sqrt(pow(x2-x1,2) + pow(y2-y1,2) + pow(z2-z1,2));
}
