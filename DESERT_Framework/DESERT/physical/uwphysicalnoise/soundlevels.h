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
 * @file   soundlevels.h
 * @author Emanuele Coccolo
 * @version 1.0.0
 *
 * \brief Functions to compute the noise power spectral density for
 * different type of ships and auv.
 *
 */

#ifndef SOUNDLEVELS_H
#define SOUNDLEVELS_H

#include <string>
#include <unordered_map>

namespace ship_noise
{
/**
 * Enum type representing the ship categories.
 * CARGO: Cargo ship
 * CARGO_L: Long cargo ship
 * CRUISE: Cruise ship
 * FERRY: Ferry boat
 * FISHING: Fishing vessel
 * AUV: Autonomous Underwater Vehicle
 */
enum class ShipCategory { CARGO, CARGO_L, CRUISE, FERRY, FISHING, AUV };

/** Dictionary of ship categories. */
static const std::unordered_map<std::string, ShipCategory> cat_dict = {
		{"cargo", ShipCategory::CARGO},
		{"cargo_l", ShipCategory::CARGO_L},
		{"cruise", ShipCategory::CRUISE},
		{"ferry", ShipCategory::FERRY},
		{"fishing_vessel", ShipCategory::FISHING},
		{"auv", ShipCategory::AUV}};

/**
 * Compute the sound level of a ship given the type
 *
 * @param ShipCategory Type of ship
 * @param double Noise frequency of the ship
 * @param double Speed of the ship
 * @param double Length of the ship
 * @return double Sound level of the ship
 */
double getNoisefromCategory(
		const ShipCategory &cat, double freq, double speed, double length);

/**
 * Compute the sound level of a cargo ship
 *
 * @param double Noise frequency of a cargo ship
 * @param double Speed of a cargo ship
 * @param double Length of a cargo ship
 * @return double Sound level of a cargo ship
 */
double cargo_ship(double freq, double speed, double length);

/**
 * Compute the sound level of a long cargo ship
 *
 * @param double Noise frequency of a long cargo ship
 * @param double Speed of a long cargo ship
 * @param double Length of a long cargo ship
 * @return double Sound level of a long cargo ship
 */
double cargoL_ship(double freq, double speed, double length);

/**
 * Compute the sound level of a cruise ship
 *
 * @param double Noise frequency of a cruise ship
 * @param double Speed of a cruise ship
 * @param double Length of a cruise ship
 * @return double Sound level of a cruise ship
 */
double cruise_ship(double freq, double speed, double length);

/**
 * Compute the sound level of a ferry
 *
 * @param double Noise frequency of a ferry
 * @param double Speed of a ferry
 * @param double Length of a ferry
 * @return double Sound level of a ferry
 */
double ferry(double freq, double speed, double length);

/**
 * Compute the sound level of a fishing vessel
 *
 * @param double Noise frequency of a fishing vessel
 * @param double Speed of a fishing vessel
 * @param double Length of a fishing vessel
 * @return double Sound level of a fishing vessel
 */
double fishing_vessel(double freq, double speed, double length);

/**
 * Compute the sound level of an Autonomous Underwater Vehicle (AUV)
 *
 * @param double Noise frequency of a fishing vessel
 * @return double Sound level of a fishing vessel
 */
double auv(double freq);
} // namespace ship_noise

#endif
