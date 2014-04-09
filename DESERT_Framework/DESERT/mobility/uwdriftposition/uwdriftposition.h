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
 * @file   uwdriftposition.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Drift mobility model.
 *
 */

#ifndef _UWDRIFTPOSITION_
#define _UWDRIFTPOSITION_

#include "node-core.h"

#include <iostream> 
#include <ctime> 
#include <cstdlib>
#include <cmath>

/**
 * UwDriftPosition class implements the drift mobility model.
 */
class UwDriftPosition : public Position {

public:
    
    /**
     * Constructor of UwDriftPosition class.
     */
    UwDriftPosition();
    
    /**
     * Destructor of UwDriftPosition class.
     */
    virtual ~UwDriftPosition();
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int, const char*const*);
    
protected:
    
    double xFieldWidth_;        /**< Range of the x-axis of the field to be simulated, in meters. */
    double yFieldWidth_;        /**< Range of the y-axis of the field to be simulated, in meters. */
    double zFieldWidth_;        /**< Range of the z-axis of the field to be simulated, in meters. */
    int boundx_;                /**< <i>1</i> if the x-axis is bounded, <i>0</i> otherwise. */
    int boundy_;                /**< <i>1</i> if the y-axis is bounded, <i>0</i> otherwise. */
    int boundz_;                /**< <i>1</i> if the z-axis is bounded, <i>0</i> otherwise. */
    double speed_horizontal_;   /**< Speed of the node in the x-axis, in m/s. */
    double speed_longitudinal_; /**< Speed of the node in the y-axis, in m/s. */
    double speed_vertical_;     /**< Speed of the node in the z-axis, in m/s. */
    double alpha_;              /**< Parameter to be used to vary the randomness: <i>0</i>: totally random values (Brownian motion), <i>1</i>: linear motion. */
    double deltax_;             /**< Max value of the Uniform Distribution: Random movement between [0, deltax_). */
    double deltay_;             /**< Max value of the Uniform Distribution: Random movement between [0, deltay_). */
    double deltaz_;             /**< Max value of the Uniform Distribution: Random movement between [0, deltaz_). */
    double starting_speed_x_;   /**< Initial speed of the node. x axis in m/s. */
    double starting_speed_y_;   /**< Initial speed of the node. y axis in m/s. */
    double starting_speed_z_;   /**< Initial speed of the node. z axis in m/s. */
    double updateTime_;         /**< Time between two update computation. */
    double nextUpdateTime_;     /**< Internal variable used to evaluate the steps to be computed. */
    int debug_;                 /**< Flag to enable or disable dirrefent levels of debug. */
    
    /**
     * Updates both the position coordinates as function of the number of states to be evaluated.
     * @param double Current time.
     */
    virtual void update(const double&);
    
    /**
     * Returns the current projection of the node on the x-axis.
     * If it's necessary (updating time ia expired), update the position values 
     * before returns it.
     * 
     * @return The new x-axis position value of the node.
     */
    virtual double getX();
    
    /**
     * Returns the current projection of the node on the y-axis.
     * If it's necessary (updating time ia expired), update the position values 
     * before returns it.
     * 
     * @return The new y-axis position value of the node.
     */
    virtual double getY();
    
    /**
     * Returns the current projection of the node on the z-axis.
     * If it's necessary (updating time ia expired), update the position values 
     * before returns it.
     * 
     * @return The new z-axis position value of the node.
     */
    virtual double getZ();
    
    /**
     * Returns a randomly <i>1</i> or <i>-</i>.
     * @return 
     */
    virtual short getSign() const;

private:
    double old_speed_x_;        /**< Temporary variable. */
    double old_speed_y_;        /**< Temporary variable. */
    double old_speed_z_;        /**< Temporary variable. */
};

#endif // _UWDRIFTPOSITION_
