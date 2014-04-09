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
 * @file   sun-ipr-node-loadmetric.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Provides the implementation of all the methods regarding the Load metric.
 *
 * Provides the implementation of all the methods regarding the Load metric.
 */

#include "sun-ipr-node.h"

/* This function increases the counter of packets processed by the current node */
void SunIPRoutingNode::updatePacketsCount() {
    if (STACK_TRACE)
        cout << "> updatePacketsCount()" << endl;
    double current_instant_ = Scheduler::instance().clock();
    list_packets_max_time_ = current_instant_;
    list_packets[pointer_packets_] = current_instant_;
    if (pointer_packets_ + 1 >= LISTLENGTH)
        packets_array_full = true;
    pointer_packets_ = (pointer_packets_ + 1) % LISTLENGTH;
} /* SunIPRoutingNode::updatePacketsCount */

/* This function increases the counter of acks received by the current node */
void SunIPRoutingNode::updateAcksCount() {
    if (STACK_TRACE)
        cout << "> updateAcksCount()" << endl;
    double current_instant_ = Scheduler::instance().clock();
    list_acks_max_time_ = current_instant_;
    list_acks[pointer_acks_] = current_instant_;
    if (pointer_acks_ + 1 >= LISTLENGTH)
        acks_array_full = true;
    pointer_acks_ = (pointer_acks_ + 1) % LISTLENGTH;
} /* SunIPRoutingNode::updateAcksCount */

/* This function returns the number of packets processed by the current node
 * in the last minute.
 */
const int SunIPRoutingNode::getPacketsLastMinute() const {
    if (STACK_TRACE)
        cout << "> getPacketsLastMinute()" << endl;
    long t_max_ = list_packets_max_time_;
    long t_min_ = t_max_;
    int position_min_ = 0;
    // int position_max_ = pointer_packets_ - 1;
    for (int i = 0; i < LISTLENGTH; i++) {
        if ((list_packets[i] != 0) && (list_packets[i] < t_min_)) {
            t_min_ = list_packets[i];
            position_min_ = i;
        }
    }
    if (t_max_ == t_min_) {
        return 0;
    } else if ((t_max_ - t_min_) <= MINUTE) { // More than LISTLENGTH per minute.
        if (packets_array_full) {
            return int(MINUTE / (t_max_ - t_min_) * LISTLENGTH);
        }
        else
            return int(MINUTE / (t_max_ - t_min_));
    } else {
        for (int i = 0; i < LISTLENGTH; i++) {
            position_min_ = (position_min_ + 1) % LISTLENGTH;
            t_min_ = list_packets[position_min_];
            if ((t_max_ - t_min_) <= MINUTE) {
                break;
            }
        }
        if (t_max_ == t_min_) {
            return 0;
        } else {
            return int(MINUTE / (t_max_ - t_min_) * LISTLENGTH);
        }
    }
} /* SunIPRoutingNode::getPacketsLastMinute */

/* This function returns the number of acks received by the current node
 * in the last minute.
 */
const int SunIPRoutingNode::getAcksLastMinute() const {
    if (STACK_TRACE)
        cout << "> getAcksLastMinute()" << endl;
    long t_max_ = list_acks_max_time_;
    long t_min_ = t_max_;
    int position_min_ = 0;
    // int position_max_ = pointer_acks_ - 1;
    for (int i = 0; i < LISTLENGTH; i++) {
        if ((list_acks[i] != 0) && (list_acks[i] < t_min_)) {
            t_min_ = list_acks[i];
            position_min_ = i;
        }
    }
    if (t_max_ == t_min_) {
        return 0;
    } else if ((t_max_ - t_min_) <= MINUTE) { // More than LISTLENGTH per minute.
        if (acks_array_full)
            return int(MINUTE / (t_max_ - t_min_) * LISTLENGTH);
        else
            return int(MINUTE / (t_max_ - t_min_));
    } else {
        for (int i = 0; i < LISTLENGTH; i++) {
            position_min_ = (position_min_ + 1) % LISTLENGTH;
            t_min_ = list_acks[position_min_];
            if ((t_max_ - t_min_) <= MINUTE)
                break;
        }
        if (t_max_ == t_min_) {
            return 0;
        } else {
            return int(MINUTE / (t_max_ - t_min_) * LISTLENGTH);
        }
    }
} /* SunIPRoutingNode::getAcksLastMinute */

const double SunIPRoutingNode::getLoad() const {
    if (STACK_TRACE)
        cout << "> getLoad()" << endl;
    int packets_ = getPacketsLastMinute();
    int acks_ = getAcksLastMinute();
    return packets_*alpha_ + (packets_- acks_)*(1 - alpha_);
} /* SunIPRoutingNode::getLoad */
