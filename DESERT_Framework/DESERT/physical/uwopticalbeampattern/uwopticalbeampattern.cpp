//
// Copyright (c) 2018 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwopticalbeampattern.cpp
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of UwOpticalBeamPattern class.
 *
 */

#include "uwopticalbeampattern.h"
#include "uwoptical-mpropagation.h"
#include <float.h>

const double K = 1.38 * 1.E-23; // Boltzmann constant
const double q = 1.6 * 1.E-19; // electronic charge

static class UwOpticalBeamPatternClass : public TclClass
{
public:
	UwOpticalBeamPatternClass()
		: TclClass("Module/UW/UWOPTICALBEAMPATTERN")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UwOpticalBeamPattern);
	}
} class_module_optical;

UwOpticalBeamPattern::UwOpticalBeamPattern()
	: 
	UwOpticalPhy(),
	beam_pattern_path_tx_(""),
	beam_pattern_path_rx_(""),
	max_dist_path_(""),
	beam_pattern_separator_(','),
	max_dist_separator_(','),
	dist_lut_(),
	beam_lut_tx_(),
	beam_lut_rx_(),
	back_noise_threshold_(0),
	inclination_angle_(0),
	sameBeam(true)
{
	bind("noise_threshold", &back_noise_threshold_);
	bind("inclination_angle_", &inclination_angle_);
	checkInclinationAngle();
}

int
UwOpticalBeamPattern::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "useSameBeamPattern") == 0) {
			sameBeam = true;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "useDifferentBeamPattern") == 0) {
			sameBeam = false;
			return TCL_OK;
		}

	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setBeamPatternPath") == 0) {
			if (!sameBeam) {
				fprintf(stderr, "sameBeam set as false, 2 beam pattern paths needed");
				return TCL_ERROR;
			}
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty string for the file name");
				return TCL_ERROR;
			}
			std::cout << "Use same beam pattern for tx and rx" << std::endl;
			beam_pattern_path_tx_ = tmp_;
			beam_pattern_path_rx_ = tmp_;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setMaxRangePath") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty string for the file name");
				return TCL_ERROR;
			}
			max_dist_path_ = tmp_;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setBeamSeparator") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty char for the file name");
				return TCL_ERROR;
			}
			beam_pattern_separator_ = tmp_.at(0);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setMaxRangeSeparator") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty char for the file name");
				return TCL_ERROR;
			}
			max_dist_separator_ = tmp_.at(0);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setInclinationAngle") == 0) {
			inclination_angle_ = atof(argv[2]);
			checkInclinationAngle();
			return TCL_OK;
		}
	} else if (argc == 4) {
		if (strcasecmp(argv[1], "setBeamPatternPath") == 0) {
			if (sameBeam) {
				fprintf(stderr, "sameBeam set as true, 1 beam pattern path needed");
				return TCL_ERROR;
			}
			string tmp_tx = ((char *) argv[2]);
			string tmp_rx = ((char *) argv[3]);
			if (tmp_tx.size() == 0 || tmp_rx.size() == 0) {
				fprintf(stderr, "Empty string for the file name");
				return TCL_ERROR;
			}
			std::cout << "Use different LUTs for tx and rx" << std::endl; 
			beam_pattern_path_tx_ = tmp_tx;
			beam_pattern_path_rx_ = tmp_rx;
			return TCL_OK;
		}
	}
	return UwOpticalPhy::command(argc, argv);
}

void
UwOpticalBeamPattern::checkInclinationAngle()
{
	inclination_angle_ = inclination_angle_ > M_PI ? 
		inclination_angle_ - 2 * M_PI : inclination_angle_;
	inclination_angle_ = inclination_angle_ < - M_PI ? 
		inclination_angle_ + 2 * M_PI : inclination_angle_;
}

void
UwOpticalBeamPattern::startRx(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	Position *source = ph->srcPosition;
	Position *destination = ph->dstPosition;
	double dist = source->getDist(destination);


	if ((PktRx == 0) && (txPending == false)) {
		double max_tx_range = getMaxTxRange(p);
		if (debug_)
			cout << NOW << " UwOpticalBeamPattern::startRx max_tx_range LUT = " << max_tx_range 
					 << " distance = " << dist << endl;
		if (max_tx_range > dist) {
			if (ph->modulationType == MPhy_Bpsk::modid) {
				PktRx = p;
				Phy2MacStartRx(p);
				return;
			} else {
				if (debug_)
					cout << NOW << " UwOpticalBeamPattern::Drop Packet::Wrong modulation"
						 << endl;
			}
		} else {
			if (debug_)
				cout << NOW << " UwOpticalBeamPattern::Drop Packet::Distance = "
					 << dist
					 << ", Above Max Range = " << max_tx_range
					 << endl;
		}
	} else {
		if (debug_)
			cout << NOW << " UwOpticalBeamPattern::Drop Packet::Synced onto another packet "
					"PktRx = "
				 << PktRx << ", pending = " << txPending << endl;
	}
}

double 
UwOpticalBeamPattern::getMaxTxRange(Packet *p)
{
	double beta = getBetaRx(p);	
	//double beta_xy = getBetaXY(p, inclination_angle_);
	double beta_xy = getBetaXYRx(p);

	double c = ((UwOpticalMPropagation *) propagation_)->getC(p);

	if (debug_)
		cout << NOW << " UwOpticalBeamPattern::getMaxTxRange c = " 
			<< c << std::endl;

	hdr_uwopticalbeampattern *bl = HDR_UWOPTICALBEAMPATTERN(p);
	double beta_tx = getBetaTx(p);
	//double beta_xy_tx = getBetaXY(p, bl->get_inclination_angle());	
	double beta_xy_tx = getBetaXYTx(p);	

	hdr_MPhy *ph = HDR_MPHY(p);
	Position *dest = ph->dstPosition;
	double dest_depth = use_woss_ ? -dest->getAltitude() : -dest->getZ();
	double na = lookUpLightNoiseE(dest_depth); // background noise
	
	if ((dist_lut_.empty() == true) || 
			beam_lut_tx_.empty() == true || beam_lut_rx_.empty() == true) {
		cerr << "UwOpticalBeamPattern::getMaxTxRange error: LUTs not init." << endl;
		return 0;
	}
	double max_distance = getLutMaxDist(c,na);	
	if (debug_)
		cout << NOW << " UwOpticalBeamPattern::getMaxTxRange max_distance LUT = " 
			<< max_distance << endl;
	/*if(beta == 0) {
		return max_distance;
	}*/
	double norm_beam_factor_rx = getLutBeamFactor(beam_lut_rx_, beta);
	double norm_beam_factor_tx = getLutBeamFactor(beam_lut_tx_, beta_tx);	
	if (debug_)
		cout << NOW << " UwOpticalBeamPattern::getMaxTxRange norm_beam_factor = " 
			<< norm_beam_factor_rx << ", beta = " << beta << endl;
	double norm_beam_factor_xy_rx = getLutBeamFactor(beam_lut_rx_, beta_xy);
	double norm_beam_factor_xy_tx = getLutBeamFactor(beam_lut_tx_, beta_xy_tx);		
	if (debug_)
		cout << NOW << " UwOpticalBeamPattern::getMaxTxRange norm_beam_factor_xy = " 
			<< norm_beam_factor_xy_rx << ", beta_xy = " << beta_xy << endl;
	return max_distance * norm_beam_factor_rx * norm_beam_factor_xy_rx 
			* norm_beam_factor_tx * norm_beam_factor_xy_tx;
}

double 
UwOpticalBeamPattern::getBetaRx(Packet *p)
{
	bool omnidirectional = 
		((UwOpticalMPropagation *) propagation_)->isOmnidirectional();
	if(omnidirectional) {
		return 0;
	}
	hdr_MPhy *ph = HDR_MPHY(p);
	Position *rx = ph->dstPosition;
	Position *tx = ph->srcPosition;
	double rx_x = use_woss_ ? rx->getLongitude() : rx->getX();
	double tx_x = use_woss_ ? tx->getLongitude() : tx->getX();
	double rx_y = use_woss_ ? rx->getLatitude() : rx->getY();
	double tx_y = use_woss_ ? tx->getLatitude() : tx->getY();
	double rx_depth = use_woss_ ? rx->getAltitude() : rx->getZ();
	double tx_depth = use_woss_ ? tx->getAltitude() : tx->getZ();
	double dx = tx_x - rx_x;
  	double dy = tx_y - rx_y;
  	double dz = tx_depth - rx_depth;
  	double dx_prime = dx*cos(-inclination_angle_) - dz*sin(-inclination_angle_);
  	double dz_prime = dx*sin(-inclination_angle_) + dz*cos(-inclination_angle_);
  	double dy_prime = dy;

  	double dist_xy = sqrt(dx_prime*dx_prime + dy_prime*dy_prime);
  	double beta = 0;
  	if (dist_xy == 0) {
  		beta = dz_prime > 0 ? M_PI/2 : - M_PI/2;
  	}
  	else {
  		beta = use_woss_ ? 
  			((UwOpticalMPropagation *) propagation_)->getBeta(p) 
  			: atan(dz_prime/dist_xy);
  	}
  	beta = dz_prime == 0 ? 0 : beta;
  	
  	if (dx_prime < 0) {
  		beta = beta > 0 ? beta - M_PI : beta + M_PI;
  	}
	beta = beta > M_PI ? beta - 2 * M_PI 
		: beta < -M_PI ? beta + 2 * M_PI : beta;
	return beta;
}


double 
UwOpticalBeamPattern::getBetaXYRx(Packet *p)
{
	bool omnidirectional = 
		((UwOpticalMPropagation *) propagation_)->isOmnidirectional();
	if(omnidirectional) {
		return 0;
	}
	hdr_MPhy *ph = HDR_MPHY(p);
	Position *rx = ph->dstPosition;
	Position *tx = ph->srcPosition;
	double rx_x = use_woss_ ? rx->getLongitude() : rx->getX();
	double tx_x = use_woss_ ? tx->getLongitude() : tx->getX();
	double rx_y = use_woss_ ? rx->getLatitude() : rx->getY();
	double tx_y = use_woss_ ? tx->getLatitude() : tx->getY();
	double rx_depth = use_woss_ ? rx->getAltitude() : rx->getZ();
	double tx_depth = use_woss_ ? tx->getAltitude() : tx->getZ();
	double dx = tx_x - rx_x;
  	double dy = tx_y - rx_y;
  	double dz = tx_depth - rx_depth;
  	double dx_prime = dx*cos(-inclination_angle_) - dz*sin(-inclination_angle_);
  	double dz_prime = dx*sin(-inclination_angle_) + dz*cos(-inclination_angle_);
  	double dy_prime = dy;



	double beta_xy = 0;
	if(dx_prime == 0) { 
		beta_xy = dy_prime > 0 ? M_PI/2 : - M_PI/2;
	}
	else {
		beta_xy= atan((dy_prime)/(dx_prime));
	}
	beta_xy = dy_prime == 0 ? 0 : beta_xy;

	return beta_xy;


}

double 
UwOpticalBeamPattern::getBetaXYTx(Packet *p)
{
	bool omnidirectional = 
		((UwOpticalMPropagation *) propagation_)->isOmnidirectional();
	if(omnidirectional) {
		return 0;
	}


	hdr_uwopticalbeampattern *bl = HDR_UWOPTICALBEAMPATTERN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	Position *tx = ph->srcPosition;
	Position *rx = ph->dstPosition;
	double tx_x = use_woss_ ? tx->getLongitude() : tx->getX();
	double rx_x = use_woss_ ? rx->getLongitude() : rx->getX();
	double tx_y = use_woss_ ? tx->getLatitude() : tx->getY();
	double rx_y = use_woss_ ? rx->getLatitude() : rx->getY();
	double tx_depth = use_woss_ ? tx->getAltitude() : tx->getZ();
	double rx_depth = use_woss_ ? rx->getAltitude() : rx->getZ();
	double dx = rx_x - tx_x;
  	double dy = rx_y - tx_y;
  	double dz = rx_depth - tx_depth;
  	double dx_prime = dx*cos(-bl->get_inclination_angle()) - dz*sin(-bl->get_inclination_angle());
  	double dz_prime = dx*sin(-bl->get_inclination_angle()) + dz*cos(-bl->get_inclination_angle());
  	double dy_prime = dy;

	double beta_xy = 0;
	if(dx_prime == 0) { 
		beta_xy = dy_prime > 0 ? M_PI/2 : - M_PI/2;
	}
	else {
		beta_xy= atan((dy_prime)/(dx_prime));
	}
	beta_xy = dy_prime == 0 ? 0 : beta_xy;

	return beta_xy;

}

double
UwOpticalBeamPattern::getLutMaxDist(double c, double na)
{
	double max_distance = 0;
	CMaxDistIter c_dist_it = dist_lut_.lower_bound(c);
	assert(c_dist_it != dist_lut_.end());
	if (c_dist_it->first > c && c_dist_it != dist_lut_.begin()) {
		double dist_next = na > back_noise_threshold_ ? 
			c_dist_it->second.max_range_with_noise : 
			c_dist_it->second.max_range;
		double c_next = c_dist_it->first;
		c_dist_it--;
		double dist_prev = na > back_noise_threshold_ ? 
			c_dist_it->second.max_range_with_noise : 
			c_dist_it->second.max_range;
		double c_prev = c_dist_it->first;
		max_distance = linearInterpolator(c, c_prev, dist_prev, c_next, dist_next);
	}
	else {
		max_distance = na > back_noise_threshold_ ? 
			c_dist_it->second.max_range_with_noise : 
			c_dist_it->second.max_range;
	}
	return max_distance;
}

double 
UwOpticalBeamPattern::getLutBeamFactor(BeamPattern &beam_lut_, double beta)
{
	double beam_norm_d = 0;
	BeamPatternIter beam_it = beam_lut_.lower_bound(beta);
	assert(beam_it != beam_lut_.end());
	if (beam_it->first > beta && beam_it != beam_lut_.begin()) {
		double dist_next = beam_it->second;
		double beta_next = beam_it->first;
		beam_it--;
		double dist_prev = beam_it->second;
		double beta_prev = beam_it->first;
		beam_norm_d = linearInterpolator(beta, beta_prev, dist_prev, beta_next, dist_next);
	}
	else {
		beam_norm_d = beam_it->second;
	}
	return beam_norm_d;
}
void
UwOpticalBeamPattern::initializeLUT()
{
	initializeBeamLUT(beam_lut_tx_, beam_pattern_path_tx_);
	initializeBeamLUT(beam_lut_rx_, beam_pattern_path_rx_);
	initializeMaxRangeLUT();
	UwOpticalPhy::initializeLUT();
}

void 
UwOpticalBeamPattern::initializeBeamLUT(BeamPattern &beam_lut_, string beam_pattern_path_)
{
	ifstream input_file_;
	string line_;
	input_file_.open(beam_pattern_path_.c_str());
	if (input_file_.is_open()) {
		double beta;
		double r;
		while (std::getline(input_file_, line_)) {
			::std::stringstream line_stream(line_);
			line_stream >> beta;
			line_stream.ignore(256, beam_pattern_separator_);
			line_stream >> r;
			beam_lut_[beta] = r;
		}
		input_file_.close();
	} else {
		cerr << "Impossible to open file " << beam_pattern_path_ << endl;
	}
}

void 
UwOpticalBeamPattern::initializeMaxRangeLUT()
{
	ifstream input_file_;
	string line_;
	input_file_.open(max_dist_path_.c_str());
	if (input_file_.is_open()) {
		double c;
		double r;
		double r_e;
		while (std::getline(input_file_, line_)) {
			::std::stringstream line_stream(line_);
			line_stream >> c;
			line_stream.ignore(256, max_dist_separator_);
			line_stream >> r;
			line_stream.ignore(256, max_dist_separator_);
			line_stream >> r_e;
			MaxDist d;
			d.max_range = r;
			d.max_range_with_noise = r_e;
			dist_lut_[c] = d;
		}
		input_file_.close();
	} else {
		cerr << "Impossible to open file " << max_dist_path_ << endl;
	}
}

void
UwOpticalBeamPattern::startTx(Packet *p)
{
	hdr_uwopticalbeampattern *bl = HDR_UWOPTICALBEAMPATTERN(p);
	bl->get_inclination_angle() = inclination_angle_;	
	UwOpticalPhy::startTx(p);
}

double
UwOpticalBeamPattern::getBetaTx(Packet *p)
{
	/*
	bool omnidirectional = 
		((UwOpticalMPropagation *) propagation_)->isOmnidirectional();
	if(omnidirectional) {
		return 0;
	}
	*/
	hdr_uwopticalbeampattern *bl = HDR_UWOPTICALBEAMPATTERN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	Position *tx = ph->srcPosition;
	Position *rx = ph->dstPosition;
	double tx_x = use_woss_ ? tx->getLongitude() : tx->getX();
	double rx_x = use_woss_ ? rx->getLongitude() : rx->getX();
	double tx_y = use_woss_ ? tx->getLatitude() : tx->getY();
	double rx_y = use_woss_ ? rx->getLatitude() : rx->getY();
	double tx_depth = use_woss_ ? tx->getAltitude() : tx->getZ();
	double rx_depth = use_woss_ ? rx->getAltitude() : rx->getZ();
	double dx = rx_x - tx_x;
  	double dy = rx_y - tx_y;
  	double dz = rx_depth - tx_depth;
  	double dx_prime = dx*cos(-bl->get_inclination_angle()) - dz*sin(-bl->get_inclination_angle());
  	double dz_prime = dx*sin(-bl->get_inclination_angle()) + dz*cos(-bl->get_inclination_angle());
  	double dy_prime = dy;
  	
  	double dist_xy = sqrt(dx_prime*dx_prime + dy_prime*dy_prime);
  	double beta = 0;
  	if (dist_xy == 0) {
  		beta = dz_prime > 0 ? M_PI/2 : - M_PI/2;
  	}
  	else {
  		beta = use_woss_ ? 
  			((UwOpticalMPropagation *) propagation_)->getBeta(p) 
  			: atan(dz_prime/dist_xy);
  	}
  	beta = dz_prime == 0 ? 0 : beta;
  	if (dx_prime < 0) {
  		beta = beta > 0 ? beta - M_PI : beta + M_PI;
  	}
	beta = beta > M_PI ? beta - 2 * M_PI 
		: beta < -M_PI ? beta + 2 * M_PI : beta;

	return beta;
}