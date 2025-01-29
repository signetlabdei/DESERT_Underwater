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
 * @file   uwphysicalnoise.h
 * @author Emanuele Coccolo
 * @version 1.0.0
 *
 * \brief Definition of UwPhysicalNoise class.
 *
 */

#ifndef UWPHYSICALNOISE_H
#define UWPHYSICALNOISE_H

#include "soundlevels.h"
#include "uwphysical.h"
#include "uwsmposition.h"

class UnderwaterPhysicalNoise : public UnderwaterPhysical
{

public:
	/**
	 * Constructor of UnderwaterPhysicalNoise class.
	 */
	UnderwaterPhysicalNoise();

	/**
	 * Destructor of UnderwaterPhysicalNoise class.
	 */
	virtual ~UnderwaterPhysicalNoise() = default;

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not.
	 *
	 */
	virtual int command(int argc, const char *const *argv) override;

	/**
	 * It adds the noise emitter corresponding to the given id.
	 *
	 * @param size_t ID of the emitter
	 * @param double Length of the vessel
	 * @param ShipCategory Category of the vessel
	 * @param Position* Pointer to the position of the emitter
	 *
	 */
	virtual void addNoiseSource(
			size_t id, double len, ship_noise::ShipCategory cat, Position *pos);

	/**
	 * It removes the noise emitter corresponding to the given id.
	 *
	 * @param size_t ID of the emitter
	 *
	 */
	virtual void removeNoiseSourcefromID(size_t id);

	/**
	 * It calculates the total noise power, iterating on the vessel map.
	 *
	 * @param Packet* Pointer to the packet that is going to be received
	 * @return double Total vessels noise power
	 *
	 */
	virtual double vesselNoisePower(Packet *p);

	constexpr static const double MS_TO_KNOTS =
			1.94384; /**< Conversion m/s to knots. */

protected:
	/**
	 * Struct that contains the parameters of a vessel.
	 */
	struct Noisesource {
		size_t id;
		double length;
		ship_noise::ShipCategory category;
		Position *pos;
	};

	/**
	 * Compute the noise power, considering also vessels noise if needed.
	 *
	 * @param Packet* pointer to the packet received
	 * @return double Total noise power
	 */
	virtual double getNoisePower(Packet *p) override;

	/**
	 * Compute the speed of the vessel in knots.
	 *
	 * @param Position* pointer to the position.
	 * @return double Speed of a vessel in m/s
	 */
	virtual double getSpeedKnots(Position *p) const;

	int debug_noise;
	int ship_stop;		/**< If enabled, the speed is set to zero when the ship reaches its destination. */
	double granularity; /**< Number of step for the integration. */
	std::vector<Noisesource>
			noise_src; /**< Vector that stores all the vessels. */
};

#endif
