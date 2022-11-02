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
 * @file   uwinterferenceofdm.cpp
 * @author Sara Falleni
 * @version 1.0.0
 *
 * \brief Implementation of uwinterferenceofdm class.
 *
 */

#include "uwinterferenceofdm.h"

#include <mphy.h>
#include <mac.h>
#include <iostream>

#define POWER_PRECISION_THRESHOLD (1e-14)
#define EPSILON_TIME 0.000000000001

static class Interf_FOverlap_Class : public TclClass
{
public:
	Interf_FOverlap_Class()
		: TclClass("Module/UW/INTERFERENCEOFDM")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new uwinterferenceofdm);
	}
} class_interf_foverlap;

void EndInterfTimerOFDM::handle(Event *e)
{

	EndInterfEvent *ee = (EndInterfEvent *)e;
	interference->removeFromInterference(ee->power, ee->type, ee->carrier_power);
	delete ee;
}

uwinterferenceofdm::uwinterferenceofdm()
	: uwinterference(), end_timerOFDM(this)
{
	bind("use_maxinterval_", &use_maxinterval_);
	bind("inodeID_", (int *)&inodeID);
}

uwinterferenceofdm::~uwinterferenceofdm()
{
}

int uwinterferenceofdm::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2)
	{
		if (strcasecmp(argv[1], "getInterfCarriers") == 0)
		{
			tcl.resultf("%d", getInterfCarriers());
			return TCL_OK;
		}
	}
	if (argc == 3)
	{
		if (strcasecmp(argv[1], "setInterfCarriers") == 0)
		{
			setInterfCarriers(atoi(argv[2]));
			return TCL_OK;
		}
	}
	return uwinterferenceofdm::command(argc, argv);
}

void uwinterferenceofdm::addToInterference(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);
	std::vector<double> car_power;
	int used_carriers = 0;

	// Check how many carriers are effectively used
	for (int i = 0; i < ofdmph->carrierNum; i++)
	{
		used_carriers += ofdmph->carriers[i];
	}

	// For each used carrier fill with associated power
	for (int i = 0; i < ofdmph->carrierNum; i++)
	{
		car_power.push_back(ph->Pr / used_carriers * ofdmph->carriers[i]);
	}
	bool ctrl_pkt = (mach->ftype() == MF_CTS || mach->ftype() == MF_RTS || mach->ftype() == MF_ACK );  

	if (ctrl_pkt) {

		if (debug_)
			std::cout << NOW << " uwinterference::addToInterference() CTRL packet" << std::endl;
		addToInterference(ph->Pr, CTRL, ofdmph->carriers, ofdmph->carrierNum);
		EndInterfEvent *ee = new EndInterfEvent(ph->Pr, CTRL, car_power);
		// EPSILON_TIME needed to avoid the scheduling of simultaneous events
		Scheduler::instance().schedule(
			&end_timerOFDM, ee, ph->duration - EPSILON_TIME);

	} else {

		if (debug_)
			std::cout << NOW << " uwinterference::addToInterference() DATA packet" << std::endl;
		addToInterference(ph->Pr, DATA, ofdmph->carriers, ofdmph->carrierNum);
		EndInterfEvent *ee = new EndInterfEvent(ph->Pr, DATA, car_power);
		// EPSILON_TIME needed to avoid the scheduling of simultaneous events
		Scheduler::instance().schedule(
			&end_timerOFDM, ee, ph->duration - EPSILON_TIME);
	}
}

void uwinterferenceofdm::addToInterference(double pw, PKT_TYPE tp, int *carriers, int carNum)
{
	std::vector<double> car_power;
	int used_carriers = 0;

	// Check how many carriers are effectively used
	for (int i = 0; i < carNum; i++)
	{
		used_carriers += carriers[i];
	}

	// For each used carrier fill with associated power
	for (int i = 0; i < carNum; i++)
	{
		car_power.push_back(pw / used_carriers * carriers[i]);
	}

	if (use_maxinterval_) {

		std::list<ListNodeOFDM>::iterator it;
		for (it = power_list.begin(); it != power_list.end();)
		{
			if (it->time < NOW - maxinterval_) {
				it = power_list.erase(it); // Side effect: it++
			} else
				break;
		}
	}

	if (power_list.empty()) {

		if (tp == CTRL) {
			ListNodeOFDM pn(NOW, pw, 1, 0, car_power);
			power_list.push_back(pn);
		} else {
			ListNodeOFDM pn(NOW, pw, 0, 1, car_power);
			power_list.push_back(pn);
		}
	} else {

		double power_temp = power_list.back().sum_power;
		int ctrl_temp = power_list.back().ctrl_cnt;
		int data_temp = power_list.back().data_cnt;
		std::vector<double> car_pwr_old = power_list.back().carrier_power;

		string carPwr = "CarrierPower_Vector = [";

		for (int i = 0; i < car_pwr_old.size(); i++)
		{
			car_power.at(i) += car_pwr_old.at(i);
			carPwr += (std::to_string(car_power.at(i)) + ", ");
		}
		carPwr += "]";

		if (debug_)
			std::cout << NOW << " uwinterference::addToInterference() " << carPwr << std::endl;

		if (tp == CTRL)
		{
			ListNodeOFDM pn(NOW, pw + power_temp, ctrl_temp + 1, data_temp, car_power);
			power_list.push_back(pn);
		} else {
			ListNodeOFDM pn(NOW, pw + power_temp, ctrl_temp, data_temp + 1, car_power);
			power_list.push_back(pn);
		}
	}

	if (debug_)
	{
		std::cout << NOW << " uwinterference::addToInterference, power: " << pw
				  << " ,total power: " << power_list.back().sum_power
				  << " ,ctrl_packet: " << power_list.back().ctrl_cnt
				  << " ,data packets: " << power_list.back().data_cnt
				  << std::endl;
	}
}

void uwinterferenceofdm::removeFromInterference(double pw, PKT_TYPE tp, const std::vector<double> &carPwr)
{
	if (use_maxinterval_) {
		std::list<ListNodeOFDM>::iterator it;

		for (it = power_list.begin(); it != power_list.end();)
		{
			if (it->time < NOW - maxinterval_) {
				it = power_list.erase(it); // Side effect: it++
			}
			else
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
		std::vector<double> car_pwr_old = power_list.back().carrier_power;
		std::vector<double> car_pwr_new;
		for (std::size_t i = 0; i < car_pwr_old.size(); ++i)
		{
			double tempPwr = car_pwr_old[i] - carPwr[i];
			if (tempPwr < 0)
			{
				if (tempPwr < -0.001)
					cerr << NOW << " NODE " << inodeID << " REMOVE FROM INTERFERENCE !!! result < 0 (" 
					<< tempPwr << ") car " << i << std::endl;
				tempPwr = 0;
			}
			car_pwr_new.push_back(tempPwr);
		}

		if (tp == CTRL) {
			// NOW+EPSILON_TIME to compensate the early scheduling in
			// addToInterference(Packet* p)
			ListNodeOFDM pn(NOW + EPSILON_TIME,
							power_temp - pw,
							ctrl_temp - 1,
							data_temp, car_pwr_new);
			power_list.push_back(pn);
		} else {

			// NOW+EPSILON_TIME to compensate the early scheduling in
			// addToInterference(Packet* p)
			ListNodeOFDM pn(NOW + EPSILON_TIME,
							power_temp - pw,
							ctrl_temp,
							data_temp - 1, car_pwr_new);
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

	if (power_temp < -0.001) {
		if (debug_)
			std::cout << NOW << " NODE " << inodeID << " Precision error, negative power: " << power_temp
					  << std::endl;
		if (power_temp < -1)
			cerr << NOW << " NODE " << inodeID << " Precision ERROR, negative power: " << power_temp
				 << std::endl;

		power_list.back().sum_power = 0;
	}
}

double
uwinterferenceofdm::getInterferencePower(Packet *p)
{

	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);

	return (getInterferencePower(ph->Pr, ph->rxtime, ph->duration, ofdmph->carriers, ofdmph->carrierNum));
}

double
uwinterferenceofdm::getInterferencePower(
	double power, double starttime, double duration, int *carriers, int ncarriers)
{
	std::list<ListNodeOFDM>::reverse_iterator rit;

	std::vector<double> car_pwr;

	double integral = 0;
	double car_integral = 0;
	double car_power = 0;
	double lasttime = NOW;
	assert(starttime <= NOW);
	assert(duration > 0);

	for (rit = power_list.rbegin(); rit != power_list.rend(); ++rit)
	{
		car_power = 0;
		if (starttime < rit->time)
		{
			integral += rit->sum_power * (lasttime - rit->time);

			// for each carrier add interf pwr if carrier is used
			for (int i = 0; i < rit->carrier_power.size(); i++)
			{
				car_power += carriers[i] * rit->carrier_power.at(i);
			}

			car_integral += car_power * (lasttime - rit->time);
			lasttime = rit->time;
		} else {

			integral += rit->sum_power * (lasttime - starttime);
			for (int i = 0; i < rit->carrier_power.size(); i++)
			{
				car_power += carriers[i] * rit->carrier_power.at(i);
			}
			car_integral += car_power * (lasttime - starttime);
			break;
		}
	}
	double interference = (integral / duration) - power;
	double ofdminterference = (car_integral / duration) - power;
	if(interference < (ofdminterference - 1))
		std::cerr << "PROBLEM interference VS OFDM interference " << interference << " - " << ofdminterference << std::endl;

	if (abs(interference) < POWER_PRECISION_THRESHOLD)
	{
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
		if (debug_)
			std::cout << "getInterferencePower() WARNING:"
					  << " cancellation errors, interference set to 0"
					  << " previous value " << interference << endl;
		interference = 0;
	}

	if (debug_) {
		std::cout << "transmission from " << starttime << " to "
				  << starttime + duration << " power " << power
				  << " gets interference " << interference << std::endl;
	}
	if (ofdminterference < 0) {
		if (debug_)
			std::cout << "getInterferencePower() WARNING:"
					  << " cancellation errors, ofdminterference set to 0 "
					  << "previous value " << ofdminterference << endl;

		ofdminterference = 0;
	}
	return ofdminterference;
}

double
uwinterferenceofdm::getCurrentTotalPower()
{
	if (power_list.empty())
		return 0.0;
	else
		return (power_list.back().sum_power);
}

double
uwinterferenceofdm::getCurrentTotalPowerOnCarrier(int carrier)
{
	if (power_list.empty())
		return 0.0;
	else
		return (power_list.back().carrier_power.at(carrier));
}

double
uwinterferenceofdm::getTimeOverlap(Packet *p)
{

	hdr_MPhy *ph = HDR_MPHY(p);
	return (getTimeOverlap(ph->rxtime, ph->duration));
}

double
uwinterferenceofdm::getTimeOverlap(double starttime, double duration)
{
	std::list<ListNodeOFDM>::reverse_iterator rit;

	double overlap = 0;
	double lasttime = NOW;
	assert(starttime <= NOW);
	assert(duration > 0);

	for (rit = power_list.rbegin(); rit != power_list.rend(); ++rit)
	{
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
uwinterferenceofdm::getCounters(Packet *p)
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
uwinterferenceofdm::getCounters(double starttime, double duration, PKT_TYPE tp)
{
	std::list<ListNodeOFDM>::reverse_iterator rit;

	int ctrl_pkts = 0;
	int data_pkts = 0;

	assert(starttime <= NOW);
	assert(duration > 0);

	rit = power_list.rbegin();
	int last_ctrl_cnt = rit->ctrl_cnt;
	int last_data_cnt = rit->data_cnt;
	rit++;
	for (; rit != power_list.rend(); ++rit)
	{
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
