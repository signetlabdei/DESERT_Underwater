//
// Copyright (c) 2024 Regents of the SIGNET lab, University of Padova.
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
 * @file   soundlevels.cpp
 * @author Emanuele Coccolo
 * @version 1.0.0
 *
 * \brief Implementation of noise functions.
 *
 */

#include "soundlevels.h"
#include <iostream>
#include <cmath>

namespace ship_noise
{

double
getNoisefromCategory(
		const ShipCategory &cat, double freq, double speed, double length)
{

	if (speed < 1)
		speed = 1;

	if (freq < 1)
		freq = 1;

	switch (cat) {
		case ShipCategory::CARGO:
			return cargo_ship(freq, speed, length);
			break;
		case ShipCategory::CARGO_L:
			return cargoL_ship(freq, speed, length);
			break;
		case ShipCategory::CRUISE:
			return cruise_ship(freq, speed, length);
			break;
		case ShipCategory::FERRY:
			break;
		case ShipCategory::FISHING:
			return fishing_vessel(freq, speed, length);
			break;
		case ShipCategory::AUV:
			return auv(freq);
			break;
	}

	return -1;
}

double
cargo_ship(double freq, double speed, double length)
{

	double SL_mach = 0.0;
	double SL_prop = 0.0;
	double SL_cav = 0.0;

	if (freq > 200)
		SL_mach = 186 - 22 * std::log10(freq) + 15 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for MACH." << std::endl;

	if (freq > 80)
		SL_prop = 156 - 30 * std::log10(freq) + 50 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for PROP." << std::endl;

	if ((freq > 200) && (speed > 10))
		SL_cav = 129 - 20 * std::log10(freq) + 60 * std::log10(speed);

	double SL_tot = 10 * std::log10(std::pow(10, (SL_mach / 10))
							+ std::pow(10, (SL_prop / 10))
							+ std::pow(10, (SL_cav / 10)))
							+ 25 * std::log10(length / 180);

	return (std::pow(10, (SL_tot / 10)));
}

double
cargoL_ship(double freq, double speed, double length)
{

	double SL_mach = 0;
	double SL_prop = 0;
	double SL_cav = 0;

	if (freq > 250)
		SL_mach = 182 - 22 * std::log10(freq) + 15 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for MACH." << std::endl;

	if (freq > 80)
		SL_prop = 150 - 30 * std::log10(freq) + 50 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for PROP." << std::endl;

	if ((freq > 50) && (speed > 13))
		SL_cav = 120 - 20 * std::log10(freq) + 60 * std::log10(speed);

	double SL_tot = 10 * std::log10(std::pow(10, (SL_mach / 10))
							+ std::pow(10, (SL_prop / 10))
							+ std::pow(10, (SL_cav / 10)))
							+ 25 * std::log10(length / 280);

	return (std::pow(10, (SL_tot / 10)));
}

double
cruise_ship(double freq, double speed, double length)
{

	double SL_mach = 0;
	double SL_prop = 0;
	double SL_cav = 0;

	if (freq > 100)
		SL_mach = 179 - 22 * std::log10(freq) + 15 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for MACH." << std::endl;

	if (freq > 80)
		SL_prop = 142 - 25 * std::log10(freq) + 50 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for PROP." << std::endl;

	if ((freq > 60) && (speed > 12))
		SL_cav = 125 - 20 * std::log10(freq) + 60 * std::log10(speed);

	double SL_tot = 10 * std::log10(std::pow(10, (SL_mach / 10))
							+ std::pow(10, (SL_prop / 10))
							+ std::pow(10, (SL_cav / 10)))
							+ 25 * std::log10(length / 250);

	return (std::pow(10, (SL_tot / 10)));
}

double
ferry(double freq, double speed, double length)
{

	double SL_mach = 0;
	double SL_prop = 0;
	double SL_cav = 0;

	if (freq > 200)
		SL_mach = 177 - 20 * std::log10(freq) + 15 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for MACH." << std::endl;

	if (freq > 80)
		SL_prop = 140 - 25 * std::log10(freq) + 50 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for PROP." << std::endl;

	if ((freq > 200) && (speed > 10))
		SL_cav = 127 - 20 * std::log10(freq) + 60 * std::log10(speed);

	double SL_tot = 10 * std::log10(std::pow(10, (SL_mach / 10))
							+ std::pow(10, (SL_prop / 10))
							+ std::pow(10, (SL_cav / 10)))
							+ 25 * std::log10(length / 180);

	return (std::pow(10, (SL_tot / 10)));
}

double
fishing_vessel(double freq, double speed, double length)
{

	double SL_mach = 0;
	double SL_prop = 0;
	double SL_cav = 0;

	if (freq > 150)
		SL_mach = 185 - 20 * std::log10(freq) + 25 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for MACH." << std::endl;

	if (freq > 80)
		SL_prop = 132 - 25 * std::log10(freq) + 50 * std::log10(speed);
	else
		std::cout << "WARNING: Too low frequency for PROP." << std::endl;

	if ((freq > 80) && (speed > 8) && (speed < 12))
		SL_cav = 180 - 20 * std::log10(freq);

	double SL_tot = 10 * std::log10(std::pow(10, (SL_mach / 10))
							+ std::pow(10, (SL_prop / 10))
							+ std::pow(10, (SL_cav / 10)))
							+ 25 * std::log10(length / 50);

	return (std::pow(10, (SL_tot / 10)));
}

double
auv(double freq)
{
	double SL_tot = 0;

	if (freq <= 7982.5)
		SL_tot = 10 *
				std::log10(113.392 - 0.0139886 * freq +
						1.68571e-6 * std::pow(freq, 2) -
						7.11945e-11 * std::pow(freq, 3));
	else
		SL_tot = 10 * std::log10(72.99);

	return std::pow(10, SL_tot / 10);
}

}
