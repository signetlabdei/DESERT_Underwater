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
 * @file   uwdriftposition.cc
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Implementation of UWDRIFTPOSITION class.
 *
 * Implementation of UWDRIFTPOSITION class.
 */

#include "uwdriftposition.h"

#include <ostream>

/**
 * Adds the module for UwDriftPositionClass in ns2.
 */
static class UwDriftPositionClass : public TclClass {
public:

    UwDriftPositionClass() : TclClass("Position/UWDRIFT") {
    }

    TclObject* create(int, const char*const*) {
        return (new UwDriftPosition());
    }
} class_uwdriftposition;

UwDriftPosition::UwDriftPosition() :
Position(),
xFieldWidth_(0),
yFieldWidth_(0),
zFieldWidth_(0),
boundx_(0),
boundy_(1),
boundz_(1),
speed_horizontal_(0),
speed_longitudinal_(0),
speed_vertical_(0),
alpha_(0),
deltax_(0),
deltay_(0),
deltaz_(0),
starting_speed_x_(0),
starting_speed_y_(0),
starting_speed_z_(0),
updateTime_(0),
nextUpdateTime_(0.0) {
    bind("xFieldWidth_", &xFieldWidth_);
    bind("yFieldWidth_", &yFieldWidth_);
    bind("zFieldWidth_", &zFieldWidth_);
    bind("boundx_", &boundx_);
    bind("boundy_", &boundy_);
    bind("boundz_", &boundz_);
    bind("speed_horizontal_", &speed_horizontal_);
    bind("speed_longitudinal_", &speed_longitudinal_);
    bind("speed_vertical_", &speed_vertical_);
    bind("alpha_", &alpha_);
    bind("deltax_", &deltax_);
    bind("deltay_", &deltay_);
    bind("deltaz_", &deltaz_);
    bind("starting_speed_x_", &starting_speed_x_);
    bind("starting_speed_y_", &starting_speed_y_);
    bind("starting_speed_z_", &starting_speed_z_);
    bind("updateTime_", &updateTime_);
    bind("debug_", &debug_);
    old_speed_x_ = starting_speed_x_;
    old_speed_y_ = starting_speed_y_;
    old_speed_z_ = starting_speed_z_;
    srand((unsigned) time(0));
}

UwDriftPosition::~UwDriftPosition() {
}

int UwDriftPosition::command(int argc, const char*const* argv) {
    return Position::command(argc, argv);
}

void UwDriftPosition::update(const double& now) {
    double t;

    for (t = nextUpdateTime_; t < now; t += updateTime_) {
        // Calculate new speed
        double vx_ = (alpha_ * old_speed_x_) + (1 - alpha_) * (speed_horizontal_ + deltax_ * (rand() / (static_cast<double> (RAND_MAX) + 1.0)) * getSign());
        double vy_ = (alpha_ * old_speed_y_) + (1 - alpha_) * (speed_longitudinal_ + deltay_ * (rand() / (static_cast<double> (RAND_MAX) + 1.0)) * getSign());
        double vz_ = (alpha_ * old_speed_z_) + (1 - alpha_) * (speed_vertical_ + deltaz_ * (rand() / (static_cast<double> (RAND_MAX) + 1.0)) * getSign());

        // Save the new speed in a variable
        old_speed_x_ = vx_;
        old_speed_y_ = vy_;
        old_speed_z_ = vz_;

        // Calculate new position
        double newx_ = x_ + (vx_ * updateTime_);
        //cout << "-------->" << y_ << " + " << "(" << vy_ << " * " << updateTime_ << ")";
        double newy_ = y_ + (vy_ * updateTime_);
        //cout << " = " << newy_ << endl;
        double newz_ = z_ + (vz_ * updateTime_);

        // verify whether the new position has to be re-computed in order
        // to maintain node position within the simulation field	

        // Bounds check
        if (boundx_ == 1) {
            if (newx_ > xFieldWidth_) {
                //newx_ = x_ + (- vx_ * updateTime_);
                newx_ = (2 * xFieldWidth_) - newx_;
                old_speed_x_ = (-old_speed_x_); // The sign of the current speed is inverted = Rebounce
                speed_horizontal_ = (-speed_horizontal_); // The sign of the horizontal speed is inverted = Rebounce behaviour
            } else if (newx_ < 0) {
                //newx_ = x_ + (- vx_ * updateTime_);
                newx_ = -newx_;
                old_speed_x_ = (-old_speed_x_);
                speed_horizontal_ = (-speed_horizontal_);
            }
        }
        if (boundy_ == 1) {
            if (newy_ > yFieldWidth_) {
                //cout << "newy_ = " << newy_ << ". yFieldWidth_ = " << yFieldWidth_ << endl;
                //newy_ = y_ + (- vy_ * updateTime_);
                newy_ = (2 * yFieldWidth_) - newy_;
                old_speed_y_ = (-old_speed_y_);
                speed_longitudinal_ = (-speed_longitudinal_);
                //cout << "newy_ = " << newy_ << ". yFieldWidth_ = " << yFieldWidth_ << endl;
            } else if (newy_ < 0) {
                //newy_ = y_ + (- vy_ * updateTime_);
                newy_ = -newy_;
                old_speed_y_ = (-old_speed_y_);
                speed_longitudinal_ = (-speed_longitudinal_);
            }
        }
        if (boundz_ == 1) {
            if (newz_ < (-zFieldWidth_)) {
                //cout << "newz_ = " << newz_ << ". zFieldWidth_ = " << - zFieldWidth_ << endl;
                //newz_ = z_ + (- vz_ * updateTime_);
                newz_ = (-zFieldWidth_) - (newz_ + (zFieldWidth_));
                old_speed_z_ = -old_speed_z_;
                speed_vertical_ = (-speed_vertical_);
                //cout << "newz_ = " << newz_ << ". zFieldWidth_ = " << - zFieldWidth_ << endl;
            } else if (newz_ > 0) {
                //cout << "newz_ = " << newz_ << ". zFieldWidth_ = " << - zFieldWidth_ << endl;
                //newz_ = z_ + (- vz_ * updateTime_);
                newz_ = -newz_;
                old_speed_z_ = -old_speed_z_;
                speed_vertical_ = (-speed_vertical_);
                //cout << "newz_ = " << newz_ << ". zFieldWidth_ = " << - zFieldWidth_ << endl;
            }
        }
        if (debug_ > 10) {
            printf("X:%.3f->%.3f Y:%.3f->%.3f Z:%.3f->%.3f\n", x_, newx_, y_, newy_, z_, newz_);
        }
        x_ = newx_;
        y_ = newy_;
        z_ = newz_;
    }
    nextUpdateTime_ = t;
    if (debug_ > 10)
        printf("nextUpdateTime = %f, now %f, updateTime %f\n", nextUpdateTime_, now, updateTime_);
}

short UwDriftPosition::getSign() const {
    double rand_sign = (rand() / (static_cast<double> (RAND_MAX) + 1.0));
    if (rand_sign < 0.5) {
        return 1;
    } else if (rand_sign > 0.5) {
        return -1;
    } else { // rand_sign == 0.5
        return getSign();
    }
}

double UwDriftPosition::getX() {
    double now = Scheduler::instance().clock();
    if (now > nextUpdateTime_)
        update(now);
    return (x_);
}

double UwDriftPosition::getY() {
    double now = Scheduler::instance().clock();
    if (now > nextUpdateTime_)
        update(now);
    return (y_);
}

double UwDriftPosition::getZ() {
    double now = Scheduler::instance().clock();
    if (now > nextUpdateTime_)
        update(now);
    return (z_);
}
