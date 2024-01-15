//
// Copyright (c) 2023 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwem-phy.cpp
 * @author Riccardo Tumiati
 * @version 1.0.0
 *
 * \brief Implementation of UwElectroMagnetic class.
 *
 */

#include "uwem-phy.h"
#include "uwem-mpropagation.h"
#include <float.h>

static class UwElectroMagneticPhyClass : public TclClass
{
public:
	UwElectroMagneticPhyClass()
		: TclClass("Module/UW/ElectroMagnetic/PHY")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UwElectroMagneticPhy);
	}
}class_UwElectroMagneticPhyClass;

UwElectroMagneticPhy::UwElectroMagneticPhy()
	: lut_file_name_("")
	, lut_token_separator_(',')
	, rxPowerThreshold_(-200)
{
	if (!MPhy_Bpsk::initialized) {
		MPhy_Bpsk::modid =
				MPhy::registerModulationType(ELECTROMAGNETIC_MODULATION_TYPE);
		MPhy_Bpsk::initialized = true;
	}
	bind("rxPowerThreshold_", &rxPowerThreshold_);
	MPhy_Bpsk();
}

int
UwElectroMagneticPhy::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "useLUT") == 0) {
			initializeLUT();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setLUTFileName") == 0) {
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
	return MPhy_Bpsk::command(argc, argv);
}

void
UwElectroMagneticPhy::startRx(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	if ((PktRx == 0) && (txPending == false)) {
		double rx_power = 10 * log10(getRxPower(p));
		if (rx_power > rxPowerThreshold_){
			if (ph->modulationType == MPhy_Bpsk::modid) {
				PktRx = p;
				Phy2MacStartRx(p);
				return;
			} else {
				if (debug_)
					std::cout << "UwElectroMagneticPhy::Drop Packet::Wrong modulation"
					<< std::endl;
			}
		} else {
			if (debug_){
				cout << NOW << " UwElectroMagneticPhy: Drop Packet::Below Threshold : received power = "
					 << rx_power
					 << ", threshold = " << rxPowerThreshold_
					 << endl;
			}
		}
	} else {
		if (debug_)
			std::cout << "UwElectroMagneticPhy::Drop Packet::Synced onto another packet "
					"PktRx = "
				 << PktRx << ", pending = " << txPending << std::endl;
	}
}


void
UwElectroMagneticPhy::endRx(Packet *p)
{
	if(PktRx != 0 ){
		hdr_cmn *ch = HDR_CMN(p);
		if (PktRx == p) {
			if (propagation_) {
				double rx_power = 10 * log10(getRxPower(p));
				if (debug_)
					std::cout << NOW << " UwElectroMagneticPhy: RSSI= " << rx_power << std::endl; 

				double PER = getPER(rx_power);
				if (debug_)
					std::cout << NOW << " UwElectroMagneticPhy: interpolated PER = " << PER << std::endl; 

				// random experiment
				double x = RNG::defaultrng()->uniform_double();

				if (x > PER) {
					// no errors
					ch->error() = 0;
				} else {
					// at least one interferent packet
					ch->error() = 1;
					
				}
			} else {
				// no propagation model set
				ch->error() = 1;
			}

			sendUp(p);
			PktRx = 0;
		} else {
			Packet::free(p);
		}
	}
	else{
		Packet::free(p);
	}
	
}

double 
UwElectroMagneticPhy::getRxPower(Packet *p){
	hdr_MPhy *ph = HDR_MPHY(p);

	if(debug_){
		std::cout  << " UwElectroMagneticPhy: TxAntenna= " << ph->srcAntenna->getGain(p) << std::endl;
		std::cout  << " UwElectroMagneticPhy: RxAntenna= " << ph->dstAntenna->getGain(p) << std::endl;
	}

	double tot_attenuation =propagation_->getGain(p);

	// rx_power in dB
	double rx_power = TxPower_ + ph->srcAntenna->getGain(p) + ph->dstAntenna->getGain(p) - tot_attenuation;
	// linear rx_power
	rx_power = pow(10,(rx_power/10));


	return rx_power;
}

double
UwElectroMagneticPhy::getPER(double rx_power)
{
	// rx_power founded in the LUT
	RSSIMap::iterator it = lut_map.lower_bound(rx_power);

	if (it != lut_map.end() && it->first == rx_power) {
		if (debug_)
			std::cout << rx_power << " " << it->first << " " << it->second
					  << std::endl;
		return it->second;
	}

	// rx_power exceeds the LUT
	if (it == lut_map.end() && it->first != rx_power) {
		return 0;
	}
	if (it == lut_map.begin() && it->first != rx_power) {
		return 1;
	}

	// rx_power intermediate => interpolation
	RSSIMap::iterator u_it = it;
	it--;
	return linearInterpolator(rx_power, it->first, it->second, u_it->first, u_it->second)/100;
}

double
UwElectroMagneticPhy::linearInterpolator(
		double x, double x1, double y1, double x2, double y2)
{
	assert(x1 != x2);
	double m = (y1 - y2) / (x1 - x2);
	double q = y1 - m * x1;
	return m * x + q;
}

void
UwElectroMagneticPhy::initializeLUT()
{
	ifstream input_file_;
	string line_;
	input_file_.open(lut_file_name_.c_str());
	if (input_file_.is_open()) {
		double d;
		double n;
		while (std::getline(input_file_, line_)) {
			::std::stringstream line_stream(line_);
			line_stream >> d;
			line_stream.ignore(256, lut_token_separator_);
			line_stream >> n;
			lut_map[d] = n;
		}
		input_file_.close();
	} else {
		cerr << "Impossible to open file " << lut_file_name_ << endl;
	}
}
