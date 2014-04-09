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
 * @file   uwgmposition.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief 3D Gauss Markov mobility model.
 *
 * 3D Gauss Markov mobility model.
 */

#ifndef _UWGAUSSMARKOVMOBMODEL_
#define _UWGAUSSMARKOVMOBMODEL_

#include "node-core.h"

#define sgn(x) ( ((x)==0.0) ? 0.0 : ((x)/fabs(x)) )
#define pi (4*atan(1.0))

/**
 * \enum BoundType
 * \brief Defines the behaviour of the node when it reaches an edge of the simulation field
 *
 **/
enum BoundType {
    SPHERIC,
    THOROIDAL,
    HARDWALL,
    REBOUNCE
};

/**
 * UwGMPosition class implements the Gauss Markov 3D mobility model.
 **/
class UwGMPosition : public Position {
    
public:
    
    /**
     * Constructor of UwGMPosition class.
     */
    UwGMPosition();
    
    /**
     * Destructor of UwGMPosition class.
     */
    virtual ~UwGMPosition();
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int argc, const char*const* argv);
    
protected:
    
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
    
    double xFieldWidth_;        /**< Range of the x-axis of the field to be simulated, in meters. */
    double yFieldWidth_;        /**< Range of the y-axis of the field to be simulated, in meters. */
    double zFieldWidth_;        /**< Range of the z-axis of the field to be simulated, in meters. */
    double alpha_;              /**< Parameter to be used to vary the randomness: <i>0</i>: totally random values (Brownian motion), <i>1</i>: linear motion. */
    double alphaPitch_;         /**< Pitch of alpha variable. */
    double speedMean_;          /**< Defines the mean value of the speed, when it is setted to zero the node moves anyway. */
    double directionMean_;      /**< Defines the mean value of the direction. */
    double pitchMean_;          /**< Mean value for the pitch. */
    BoundType bound_;           /** Defines the behaviour of the node when it reaches the edge:
                                 * SPHERIC: return in the simulation field on the opposite side
                                 * THOROIDAL: return in the centre of simulation field
                                 * HARDWALL: the movement is stopped in the edge
                                 * REBOUNCE: the node rebounce (i.e., the movement that should be outside the simulation field is mirrored inside)
                                 */
    double updateTime_;         /**< Time between two update computation. */
    double nextUpdateTime_;     /**< Internal variable used to evaluate the steps to be computed. */
    double speed_;              /**< Current value of the speed. */
    double direction_;          /**< Current value of the direction. */
    double pitch_;              /**< Current value of the pitch. */
    int debug_;                 /**< Flag to enable or disable dirrefent levels of debug. */
    
private:
    
    /**
     * Method that updates both the position coordinates as function of the number 
     * of states to be evaluated.
     */
    virtual void update(double now);
    /** 
     * Method that returns a value from a normal random Gaussian variable (zero mean, unitary viariance)
     *
     */
    double Gaussian();
    
    double vx;        /**< Temporary variable. */
    double vy;        /**< Temporary variable. */
    double vz;        /**< Temporary variable. */
};

#endif // _UWGAUSSMARKOVMOBMODEL_
