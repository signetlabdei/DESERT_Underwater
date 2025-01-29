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
 * @file   uwphysicalnoise.cpp
 * @author Emanuele Coccolo
 * @version 1.0.0
 *
 * \brief Implementation of UnderwaterPhysicalNoise class
 *
 */

#include "uwphysicalnoise.h"
#include "underwater-mpropagation.h"
#include "uwlib.h"
#include <algorithm>
#include <phymac-clmsg.h>

static class UwPhysicalNoiseClass : public TclClass
{
public:
	UwPhysicalNoiseClass()
		: TclClass("Module/UW/PHYSICALNOISE")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UnderwaterPhysicalNoise);
	}
} class_module_uwphysicalnoise;

UnderwaterPhysicalNoise::UnderwaterPhysicalNoise()
	: debug_noise(0)
	, ship_stop(0)
	, granularity(100)
	, noise_src()
{
	bind("ship_stop", &ship_stop);
	bind("debug_noise_", &debug_noise);
	bind("granularity", &granularity);
}

int
UnderwaterPhysicalNoise::command(int argc, const char *const *argv)
{
	if (argc == 6) {
		if (strcasecmp(argv[1], "addSource") == 0) {

			int id = std::atoi(argv[2]);
			double len = std::atof(argv[3]);
			std::string cat = argv[4];
			Position *pos =
					dynamic_cast<Position *>(TclObject::lookup(argv[5]));

			auto cat_it = ship_noise::cat_dict.find(cat);

			if (id > 0 && len > 0 && cat_it != ship_noise::cat_dict.end() &&
					pos) {

				addNoiseSource(id, len, cat_it->second, pos);

				return TCL_OK;
			}

			return TCL_ERROR;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "removeSource") == 0) {

			int id = std::atoi(argv[2]);

			if (id > 0) {

				removeNoiseSourcefromID(id);

				return TCL_OK;
			}

			return TCL_ERROR;
		}
	}

	return UnderwaterPhysical::command(argc, argv);
}

void
UnderwaterPhysicalNoise::addNoiseSource(
		size_t id, double len, ship_noise::ShipCategory cat, Position *pos)
{

	static int mac_addr = -1;

	ClMsgPhy2MacAddr msg;
	sendSyncClMsg(&msg);
	mac_addr = msg.getAddr();

	Noisesource src{id, len, cat, pos};
	noise_src.emplace_back(src);

	if (debug_noise)
		cout << "MAC: " << mac_addr
			 << ": NOISE ADDED, vector size: " << noise_src.size() << endl;
}

void
UnderwaterPhysicalNoise::removeNoiseSourcefromID(size_t id)
{
	noise_src.erase(std::remove_if(noise_src.begin(),
							noise_src.end(),
							[&](Noisesource const &ns) { return ns.id == id; }),
			noise_src.end());
}

double
UnderwaterPhysicalNoise::vesselNoisePower(Packet *p)
{
	static int mac_addr = -1;

	ClMsgPhy2MacAddr msg;
	sendSyncClMsg(&msg);
	mac_addr = msg.getAddr();

	MSpectralMask *sm = getRxSpectralMask(p);
	assert(sm);

	double f_c = sm->getFreq();
	double bw = sm->getBandwidth();
	double sub_bw = bw / granularity;
	double total = 0;

	if (debug_noise)
		cout << NOW << " MAC Addr: " << mac_addr << ", f_c: " << f_c
			 << ", bandwidth: " << bw << ", sub_bandwidth: " << sub_bw
			 << ", granularity: " << granularity << endl;

	for (const auto &elem : noise_src) {

		Position *dstPos = getPosition();
		double speed = getSpeedKnots(elem.pos);

		assert(dstPos);
		assert(propagation_);

		UnderwaterMPropagation *uwmp =
				dynamic_cast<UnderwaterMPropagation *>(propagation_);

		assert(uwmp);

		double dist = elem.pos->getDist(dstPos);

		for (double freq = f_c - (bw / 2); freq < f_c + (bw / 2);
				freq += sub_bw) {

			double noiseTx = ship_noise::getNoisefromCategory(
					elem.category, (freq + sub_bw / 2), speed, elem.length);

			if (noiseTx < 0) {
				if (debug_noise)
					std::cout << "ERR: Undefined or wrong ship type!"
							  << std::endl;

				continue;
			}

			double gain = uwlib_AInv(dist / 1000.0,
					uwmp->uw.practical_spreading,
					(freq + sub_bw / 2) / 1000.0);
			double noiseRx = noiseTx * gain * sub_bw;

			total += noiseRx;
		}
	}

	if (debug_noise)
		std::cout << "Total: " << total << ", db: " << 10 * log10(total)
				  << std::endl;

	return total;
}

double
UnderwaterPhysicalNoise::getSpeedKnots(Position *p) const
{
	UWSMPosition *smp = dynamic_cast<UWSMPosition *>(p);

	if (smp) {
		if (ship_stop && smp->isDestReached())
			return 0;

		return (smp->getSpeed() * MS_TO_KNOTS);
	}

	return -1;
}

double
UnderwaterPhysicalNoise::getNoisePower(Packet *p)
{
	double noise = UnderwaterMPhyBpsk::getNoisePower(p);

	if (!noise_src.empty())
		noise += vesselNoisePower(p);

	return noise;
}
