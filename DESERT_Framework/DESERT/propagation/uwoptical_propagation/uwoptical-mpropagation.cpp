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
 * @file   uwoptical-propagation.cpp
 * @author Filippo Campagnaro, Federico Favaro, Federico Guerra
 * @version 1.0.0
 *
 * \brief Implementation of UwOpticalMPropagation class.
 *
 */

#include <node-core.h>
#include "uwoptical-mpropagation.h"
#include <math.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

/**
 * Class that represents the binding with the tcl configuration script
 */
static class UwOpticalMPropagationClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwOpticalMPropagationClass()
		: TclClass("Module/UW/OPTICAL/Propagation")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwOpticalMPropagation);
	}
} class_UwOpticalMPropagation;

UwOpticalMPropagation::UwOpticalMPropagation()
	: Ar_(1)
	, At_(1)
	, c_(0)
	, theta_(0)
	, omnidirectional_(false)
	, variable_c_(false)
	, use_woss_(false)
	, lut_file_name_("")
	, lut_token_separator_(',')

{
	/*bind_error("token_separator_", &token_separator_);*/
	bind("Ar_", &Ar_);
	bind("At_", &At_);
	bind("c_", &c_);
	bind("theta_", &theta_);
	bind("debug_", &debug_);
}

int
UwOpticalMPropagation::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "setOmnidirectional") == 0) {
			omnidirectional_ = true;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setDirectional") == 0) {
			omnidirectional_ = false;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setVariableC") == 0) {
			variable_c_ = true;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setFixedC") == 0) {
			variable_c_ = false;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setLUT") == 0) {
			initializeLUT();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setAr") == 0) {
			Ar_ = strtod(argv[2], NULL);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setAt") == 0) {
			At_ = strtod(argv[2], NULL);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setC") == 0) {
			c_ = strtod(argv[2], NULL);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setTheta") == 0) {
			theta_ = strtod(argv[2], NULL);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setLUTFileName") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty string for the file name");
				return TCL_ERROR;
			}
			lut_file_name_ = tmp_;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setLUTSeparator") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty char for the file name");
				return TCL_ERROR;
			}
			lut_token_separator_ = tmp_.at(0);
			return TCL_OK;
		}
	}

	return MPropagation::command(argc, argv);
}

void
UwOpticalMPropagation::setWoss(bool flag)
{
	use_woss_ = flag;
}

void
UwOpticalMPropagation::updateC(double depth)
{
	std::cout << NOW << " UwOpticalMPropagation::updateC depth = " << depth
			  << std::endl;

	LUT_c_iter it = lut_c_.lower_bound(depth);
	assert(it != lut_c_.end());
	if (it->first == depth) {
		c_ = it->second.first;
		return;
	}
	assert(it != lut_c_.begin());
	it--;
	double d_low = it->first;
	double c_low = it->second.first;
	double d_up = (++it)->first;
	double c_up = it->second.first;
	c_ = linearInterpolator(depth, d_low, d_up, c_low, c_up);
}

double
UwOpticalMPropagation::getWossOrientation(Position *src, Position *dest)
{
	if (src->getDist(dest) == 0)
		return 0;
	double src_depth = -src->getAltitude();
	double dest_depth = -dest->getAltitude();
	if (debug_)
		std::cout << NOW << " UwOpticalMPropagation::getWossOrientation() "
				  << "src_depth = " << src_depth
				  << " dest_depth = " << dest_depth
				  << " dist = " << dest->getDist(src) << std::endl;
	double cosBeta = (dest_depth - src_depth) / src->getDist(dest);
	cosBeta = cosBeta > 1 ? 1 : cosBeta < -1 ? -1 : cosBeta;
	return asin(cosBeta);
}

double 
UwOpticalMPropagation::getBeta(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	Position *source = ph->srcPosition;
	Position *destination = ph->dstPosition;
	assert(source);
	assert(destination);
	double beta_ = use_woss_
			? (destination->getAltitude() == source->getAltitude()
							  ? 0
							  : getWossOrientation(source, destination))
			: (source->getZ() == destination->getZ()
							  ? 0
							  : M_PI / 2 -
									  (source->getRelZenith(destination) > 0
													  ? source->getRelZenith(
																destination)
													  : -source->getRelZenith(
																destination)));
	return beta_;
}

bool 
UwOpticalMPropagation::isOmnidirectional()
{
	return omnidirectional_;
}

double 
UwOpticalMPropagation::getC(Packet *p)
{
	if (variable_c_ && p != NULL) {
		hdr_MPhy *ph = HDR_MPHY(p);
		Position *source = ph->srcPosition;
		Position *destination = ph->dstPosition;
		double destination_depth =
				use_woss_ ? -destination->getAltitude() : -destination->getZ();
		double source_depth =
				use_woss_ ? -source->getAltitude() : -source->getZ();
		double min_depth_ = min(destination_depth,source_depth);
		double max_depth_ = max(destination_depth,source_depth);
		if ((lut_c_.empty() == true) ||
			(min_depth_ < ((lut_c_.begin())->first) ||
					(max_depth_ > ((lut_c_.rbegin())->first)))) {

			goto done;
			//return c_;
		}
		
		LUT_c_iter lower = lut_c_.lower_bound(min_depth_);
		assert(lower != lut_c_.end());
		if (lower->first > min_depth_) {
			assert(lower != lut_c_.begin());
			lower--;
		}
		double first_depth = lower->first;
		double temp_depth = lower->first;
		double c_temp = lower->second.first;
		++lower;
		if (lower->first >= max_depth_) {
			return c_temp;
		}

		double average_c = 0;
		for (; lower->first < max_depth_; ++lower) {
			average_c = average_c + (c_temp + lower->second.first) * (lower->first-temp_depth)/2;
			temp_depth = lower->first;
			c_temp = lower->second.first;
		}
		
		return average_c / (temp_depth - first_depth);
	}

done:
	return c_;
}

double
UwOpticalMPropagation::getGain(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	Position *source = ph->srcPosition;
	Position *destination = ph->dstPosition;
	//assert(source);
	//assert(destination);
	
	double beta_ = getBeta(p);
	double PCgain;
	
	if (source != 0 || destination != 0) {
		if (!variable_c_ || beta_ == 0) {
			if (variable_c_)
				updateC(use_woss_ ? -destination->getAltitude()
							  : -destination->getZ());
			double dist = source->getDist(destination);
			PCgain = getLambertBeerGain(dist, beta_);
			if (debug_)
				std::cout << NOW << " UwOpticalMPropagation::getGain()"
					  << " dist=" << dist << " gain=" << PCgain << std::endl;
		} else {
			double destination_depth =
				use_woss_ ? -destination->getAltitude() : -destination->getZ();
			double source_depth =
				use_woss_ ? -source->getAltitude() : -source->getZ();
			if (debug_)
				std::cout << NOW
					  << " UwOpticalMPropagation::getGain() destination_depth "
					  << destination_depth << " source_depth " << source_depth
					  << std::endl;
			PCgain = getLambertBeerGain_variableC(beta_,
				min(destination_depth, source_depth),
				max(destination_depth, source_depth));
			if (debug_)
				std::cout << NOW << " UwOpticalMPropagation::getGain()"
					  << "c_variable"
					  << " gain=" << PCgain << std::endl;
		} 
	} else {
		std::cerr << "UwOpticalMPropagation::getGain(), source or destination not found";
	}
	return PCgain;
}

double
UwOpticalMPropagation::getLambertBeerGain(double d, double beta_)
{
	double cosBeta = omnidirectional_ ? 1 : cos(beta_);
	double L = d / cosBeta;
	double PCgain = 2 * Ar_ * cosBeta /
			(M_PI * pow(L, 2.0) * (1 - cos(theta_)) + 2 * At_) * exp(-c_ * d);
	return (PCgain == PCgain) ? PCgain : 0;
}

void
UwOpticalMPropagation::initializeLUT()
{
	ifstream input_file_;
	string line_;
	input_file_.open(lut_file_name_.c_str());
	if (input_file_.is_open()) {
		lut_c_.clear();
		while (std::getline(input_file_, line_)) {
			::std::stringstream line_stream(line_);
			double d;
			double c;
			double t;
			line_stream >> d;
			line_stream.ignore(256, lut_token_separator_);
			line_stream >> c;
			line_stream.ignore(256, lut_token_separator_);
			line_stream >> t;
			lut_c_[d] = std::make_pair(c, t);
		}
		input_file_.close();
	} else {
		cerr << "Impossible to open file " << lut_file_name_ << endl;
	}
}

double
UwOpticalMPropagation::linearInterpolator(
		double x, double x1, double x2, double y1, double y2)
{
	double m = (y1 - y2) / (x1 - x2);
	double q = y1 - m * x1;
	return m * x + q;
}

double
UwOpticalMPropagation::getLambertBeerGain_variableC(
		double beta_, double min_depth_, double max_depth_)
{
	if (debug_)
		std::cout << NOW << " UwOpticalMPropagation::getLambertBeerGain_"
							"variableC min_depth_ = "
				  << min_depth_ << " max_depth_ = " << max_depth_
				  << " beta_ = " << beta_ << std::endl;
	double PCgain = 1;
	double c_med;
	double dist_top = 0;
	double dist_bottom = 0;

	// if(min_depth_ < ((lut_c_.begin()) -> first) || max_depth_ >
	// ((--lut_c_.end()) -> first) )
	if ((lut_c_.empty() == true) ||
			(min_depth_ < ((lut_c_.begin())->first) ||
					(max_depth_ > ((lut_c_.rbegin())->first))))
		return NOT_FOUND_C_VALUE;
	LUT_c_iter lower = lut_c_.lower_bound(min_depth_);
	assert(lower != lut_c_.end());
	if (lower->first > min_depth_) {
		assert(lower != lut_c_.begin());
		lower--;
	}
	LUT_c_iter upper = (--lut_c_.lower_bound(max_depth_));
	assert(upper != lut_c_.end());
	if (upper->first > max_depth_) {
		assert(upper != lut_c_.begin());
		upper--;
	}

	double temp_depth = lower->first;
	double c_temp = lower->second.first;
	++lower;
	double prev_c = linearInterpolator(
			min_depth_, temp_depth, lower->first, c_temp, lower->second.first);

	temp_depth = upper->first;
	c_temp = upper->second.first;
	++upper;
	double c_up = linearInterpolator(
			max_depth_, temp_depth, upper->first, c_temp, upper->second.first);

	double temp_c = 0;
	temp_depth = 0;

	for (; lower != upper; ++lower) {
		temp_c = lower->second.first;
		c_med = (prev_c + temp_c) / 2;
		prev_c = temp_c;
		temp_depth = lower->first;
		dist_bottom = (temp_depth - min_depth_) / sin(beta_);
		PCgain = exp(-c_med * (dist_bottom - dist_top)) * PCgain;
		dist_top = dist_bottom;
	}

	c_med = (prev_c + c_up) / 2;
	dist_bottom = (max_depth_ - min_depth_) / sin(beta_);
	double cosBeta = omnidirectional_ ? 1 : cos(beta_);
	// MATTEO: L_rx = omnidirectional_ ? dist_bottom : 2*dist_bottom*(cos(beta_)
	// - (sin(beta_)/tan(2*beta_)));
	double L_ = dist_bottom /
			cosBeta; // FILIPPO: sono equivalenti (provare per credere)
	PCgain = exp(-c_med * (dist_bottom - dist_top)) * PCgain;
	return (PCgain * 2 * Ar_ * cosBeta /
			(M_PI * pow(L_, 2) * (1 - cos(theta_)) + 2 * At_));
}

double
UwOpticalMPropagation::getTemperature(double depth)
{
	if (!variable_c_ || lut_c_.empty() == true)
		return NOT_VARIABLE_TEMPERATURE;
	LUT_c_iter it = lut_c_.lower_bound(depth);
	assert(it != lut_c_.end());
	if (it->first == depth) {
		return it->second.second;
	}
	it--;
	double d_low = it->first;
	double t_low = it->second.second;
	double d_up = (++it)->first;
	double t_up = it->second.second;
	return linearInterpolator(depth, d_low, d_up, t_low, t_up);
}
