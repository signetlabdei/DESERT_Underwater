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
 * @file   uwinterference.cpp
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Implementation of uwinterference class.
 *
 */

#include "uwinterference.h"

#include <mphy.h>
#include <mac.h>
#include <iostream>

#define POWER_PRECISION_THRESHOLD (1e-14)
#define EPSILON_TIME 0.000000000001

static class Interf_Overlap_Class : public TclClass
{
public:
	Interf_Overlap_Class()
		: TclClass("Module/UW/INTERFERENCE")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new uwinterference);
	}
} class_interf_overlap;

void
EndInterfTimer::handle(Event *e)
{

	EndInterfEvent *ee = (EndInterfEvent *) e;
	interference->removeFromInterference(ee->power, ee->type);
	delete ee;
}

uwinterference::uwinterference()
	: power_list()
	, end_timer(this)
	, use_maxinterval_(1)
	, initial_interference_time(0)
	, start_rx_time(0)
	, end_rx_time(0)
{
	bind("use_maxinterval_", &use_maxinterval_);
}

uwinterference::~uwinterference()
{
}

void
uwinterference::addToInterference(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_mac *mach = HDR_MAC(p);
	if (mach->ftype() == MF_CONTROL) {
		addToInterference(ph->Pr, CTRL);
		EndInterfEvent *ee = new EndInterfEvent(ph->Pr, CTRL);
		// EPSILON_TIME needed to avoid the scheduling of simultaneous events
		Scheduler::instance().schedule(
				&end_timer, ee, ph->duration - EPSILON_TIME);
	} else {
		addToInterference(ph->Pr, DATA);
		EndInterfEvent *ee = new EndInterfEvent(ph->Pr, DATA);
		// EPSILON_TIME needed to avoid the scheduling of simultaneous events
		Scheduler::instance().schedule(
				&end_timer, ee, ph->duration - EPSILON_TIME);
	}
}

void
uwinterference::addToInterference(double pw, PKT_TYPE tp)
{
	if (use_maxinterval_) {
		std::list<ListNode>::iterator it;
		for (it = power_list.begin(); it != power_list.end();) {
			if (it->time < NOW - maxinterval_) {
				it = power_list.erase(it); // Side effect: it++
			} else
				break;
		}
	}

	if (power_list.empty()) {
		if (tp == CTRL) {
			ListNode pn(NOW, pw, 1, 0);
			power_list.push_back(pn);
		} else {
			ListNode pn(NOW, pw, 0, 1);
			power_list.push_back(pn);
		}
	} else {
		double power_temp = power_list.back().sum_power;
		int ctrl_temp = power_list.back().ctrl_cnt;
		int data_temp = power_list.back().data_cnt;
		if (tp == CTRL) {
			ListNode pn(NOW, pw + power_temp, ctrl_temp + 1, data_temp);
			power_list.push_back(pn);
		} else {
			ListNode pn(NOW, pw + power_temp, ctrl_temp, data_temp + 1);
			power_list.push_back(pn);
		}
	}

	if (debug_) {
		std::cout << NOW << " uwinterference::addToInterference, power: " << pw
				  << " ,total power: " << power_list.back().sum_power
				  << " ,ctrl_packet: " << power_list.back().ctrl_cnt
				  << " ,data packets: " << power_list.back().data_cnt
				  << std::endl;
	}
}

void
uwinterference::removeFromInterference(double pw, PKT_TYPE tp)
{
	if (use_maxinterval_) {
		std::list<ListNode>::iterator it;

		for (it = power_list.begin(); it != power_list.end();) {
			if (it->time < NOW - maxinterval_) {
				it = power_list.erase(it); // Side effect: it++
			} else
				break;
		}
	}

	if (power_list.empty()) {
		std::cerr << "uwinterference::removeFromInterference, "
				  << "some interference removed wrongly" << std::endl;
		return;
	} else {
		double power_temp = power_list.back().sum_power;
		int ctrl_temp = power_list.back().ctrl_cnt;
		int data_temp = power_list.back().data_cnt;

		if (tp == CTRL) {
			// NOW+EPSILON_TIME to compensate the early scheduling in
			// addToInterference(Packet* p)
			ListNode pn(NOW + EPSILON_TIME,
					power_temp - pw,
					ctrl_temp - 1,
					data_temp);
			power_list.push_back(pn);
		} else {
			// NOW+EPSILON_TIME to compensate the early scheduling in
			// addToInterference(Packet* p)
			ListNode pn(NOW + EPSILON_TIME,
					power_temp - pw,
					ctrl_temp,
					data_temp - 1);
			power_list.push_back(pn);
		}
	}
	if (debug_) {
		std::cout << NOW << " uwinterference::removeFromInterference, "
				  << "power: " << pw
				  << " ,total power: " << power_list.back().sum_power
				  << " ,ctrl_packet: " << power_list.back().ctrl_cnt
				  << " ,data packets: " << power_list.back().data_cnt
				  << std::endl;
	}

	double power_temp = power_list.back().sum_power;
	if (power_temp < 0) {
		if (debug_) {
			std::cout << NOW
					  << " Precision error, negative power: " << power_temp
					  << std::endl;
		}
		power_list.back().sum_power = 0;
	}
}

double
uwinterference::getInterferencePower(Packet *p)
{

	hdr_MPhy *ph = HDR_MPHY(p);
	double a = getTimeOverlap(p);
	if (debug_) {
		std::cout << NOW << " uwinterference::getInterferencePower, "
				  << "percentage of overlap: " << a << std::endl;
	}
	return (getInterferencePower(ph->Pr, ph->rxtime, ph->duration));
}

double
uwinterference::getInterferencePower(
		double power, double starttime, double duration)
{
	std::list<ListNode>::reverse_iterator rit;

	double integral = 0;
	double lasttime = NOW;
	assert(starttime <= NOW);
	assert(duration > 0);

	for (rit = power_list.rbegin(); rit != power_list.rend(); ++rit) {
		if (starttime < rit->time) {
			integral += rit->sum_power * (lasttime - rit->time);
			lasttime = rit->time;

		} else {
			integral += rit->sum_power * (lasttime - starttime);

			break;
		}
	}
	double interference = (integral / duration) - power;

	if (abs(interference) < POWER_PRECISION_THRESHOLD) {
		if (debug_)
			std::cout << "getInterferencePower() WARNING:"
					  << " interference=" << interference
					  << " POWER_PRECISION_THRESHOLD="
					  << POWER_PRECISION_THRESHOLD 
					  << ". Precision error, interference set to 0"
					  << endl;
	}
	// Check for cancellation errors
	// which can arise when interference is subtracted
	if (interference < 0) {
		interference = 0;
		if (debug_)
				std::cout << "getInterferencePower() WARNING:"
						  << " cancellation errors, interference set to 0"
						  << endl;
	}

	if (debug_) {
		std::cout << "transmission from " << starttime << " to "
				  << starttime + duration << " power " << power
				  << " gets interference " << interference << std::endl;
	}
	return interference;
}

double
uwinterference::getCurrentTotalPower()
{
	if (power_list.empty())
		return 0.0;
	else
		return (power_list.back().sum_power);
}

double
uwinterference::getTimeOverlap(Packet *p)
{

	hdr_MPhy *ph = HDR_MPHY(p);
	return (getTimeOverlap(ph->rxtime, ph->duration));
}

double
uwinterference::getTimeOverlap(double starttime, double duration)
{
	std::list<ListNode>::reverse_iterator rit;

	double overlap = 0;
	double lasttime = NOW;
	assert(starttime <= NOW);
	assert(duration > 0);

	for (rit = power_list.rbegin(); rit != power_list.rend(); ++rit) {
		if (starttime < rit->time) {

			if (rit->ctrl_cnt > 1 || rit->data_cnt > 1) {
				overlap += (lasttime - rit->time);
			}
			lasttime = rit->time;

		} else {
			if (rit->ctrl_cnt > 1 || rit->data_cnt > 1) {
				overlap += (lasttime - starttime);
			}
			break;
		}
	}

	return overlap / duration;
}

counter
uwinterference::getCounters(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_mac *mach = HDR_MAC(p);
	counter cnt;
	if (mach->ftype() == MF_CONTROL) {
		cnt = getCounters(ph->rxtime, ph->duration, CTRL);
	} else {
		cnt = getCounters(ph->rxtime, ph->duration, DATA);
	}
	return cnt;
}

counter
uwinterference::getCounters(double starttime, double duration, PKT_TYPE tp)
{
	std::list<ListNode>::reverse_iterator rit;

	int ctrl_pkts = 0;
	int data_pkts = 0;

	assert(starttime <= NOW);
	assert(duration > 0);

	rit = power_list.rbegin();
	int last_ctrl_cnt = rit->ctrl_cnt;
	int last_data_cnt = rit->data_cnt;
	rit++;
	for (; rit != power_list.rend(); ++rit) {
		if (starttime < rit->time) {
			if (last_ctrl_cnt - rit->ctrl_cnt >= 0) {
				ctrl_pkts += last_ctrl_cnt - rit->ctrl_cnt;
			}
			if (last_data_cnt - rit->data_cnt >= 0) {
				data_pkts += last_data_cnt - rit->data_cnt;
			}
			last_ctrl_cnt = rit->ctrl_cnt;
			last_data_cnt = rit->data_cnt;
		} else {
			ctrl_pkts += rit->ctrl_cnt;
			data_pkts += rit->data_cnt;
			break;
		}
	}

	if (tp == CTRL) {
		ctrl_pkts--;
	} else {
		data_pkts--;
	}

	if (debug_) {
		std::cout << NOW << " uwinterference::getCounters(), collisions"
				  << " with ctrl pkts: " << ctrl_pkts
				  << ", collisions with data pkts: " << data_pkts << std::endl;
	}

	return counter(ctrl_pkts, data_pkts);
}
