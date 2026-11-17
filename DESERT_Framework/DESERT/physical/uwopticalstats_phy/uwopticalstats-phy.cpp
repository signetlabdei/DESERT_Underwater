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
 * @file uwopticalstats-phy.cpp
 * @author Filippo Donega'
 * @version 1.0.0
 *
 * \brief Implementation of the UwOpticalStatsPhy class.
 */

#include "uwopticalstats-phy.h"
#include "uwoptical-mpropagation.h"
#include "uwphy-clmsg.h"
#include "uwstats-utilities.h"
#include <uwinterference.h>
#include <float.h>
#include "uwopticalstats_phy-clmsg.h"

UwOpticalStats::UwOpticalStats()
	: Stats()
	, last_rx_power(0)
	, last_noise_power(0)
	, instant_noise_power(0)
	, last_interf_power(0)
	, has_error(false)
	, temperature(0)
{
	type_id = (int)StatsEnum::STATS_PHY_LAYER;
}

UwOpticalStats::UwOpticalStats(int mod_id, int stck_id)
	: Stats(mod_id, stck_id)
	, last_rx_power(0)
	, last_noise_power(0)
	, instant_noise_power(0)
	, last_interf_power(0)
	, has_error(false)
	, temperature(0)
{
	type_id = (int)StatsEnum::STATS_PHY_LAYER;
}

void 
UwOpticalStats::updateStats(int mod_id, int stck_id, double rx_pwr, double noise_pwr, double interf_pwr, 
		bool error, double T, ChannelStates channel_state)
{
	module_id = mod_id;
	stack_id = stck_id;
	last_rx_power = rx_pwr;
	last_noise_power = noise_pwr;
	instant_noise_power = noise_pwr;
	last_interf_power = interf_pwr;
	has_error = error;
	temperature = T;
	channel_state = channel_state;
}

Stats*
UwOpticalStats::clone() const
{
	return new UwOpticalStats( *this );
}

static class UwOpticalStatsPhyClass : public TclClass
{
	public:
		UwOpticalStatsPhyClass()
			: TclClass("Module/UW/OPTICAL/STATSPHY")
		{
		}
		TclObject *
		create(int, const char *const *)
		{
			return (new UwOpticalStatsPhy());
		}
} class_module_opticalstats;

UwOpticalStatsPhy::UwOpticalStatsPhy()
	: UwOpticalPhy()
	, threshold(0)
{
	bind("Threshold_", &threshold);
	bind("T_", &accessTemperature());
	delete stats_ptr;
	stats_ptr = new UwOpticalStats();
}

UwOpticalStatsPhy::~UwOpticalStatsPhy() {
	delete stats_ptr;
	stats_ptr = nullptr;
}

int
UwOpticalStatsPhy::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "setInterference") == 0) {
			interference_ = dynamic_cast<uwinterference *>(TclObject::lookup(argv[2]));
			if (!interference_) {
				return TCL_ERROR;
			}
			return TCL_OK;
		}
	}
	return UwOpticalPhy::command(argc, argv);
}

void 
UwOpticalStatsPhy::updateInstantaneousStats()
{
	Packet *temp = Packet::alloc();
	hdr_MPhy *ph = HDR_MPHY(temp);
	ph->dstSpectralMask = getRxSpectralMask(temp);
	ph->dstPosition = getPosition();
	ph->dstAntenna = getRxAntenna(temp);
	assert(ph->dstSpectralMask);
	assert(ph->dstPosition);

	ph->srcSpectralMask = getTxSpectralMask(temp);
	ph->srcAntenna = getTxAntenna(temp);
	ph->srcPosition = getPosition();
	assert(ph->srcSpectralMask);

	(dynamic_cast<UwOpticalStats*>(stats_ptr))->instant_noise_power = UwOpticalPhy::getNoisePower(temp);
	Packet::free(temp);
}

int
UwOpticalStatsPhy::recvSyncClMsg(ClMessage* m)
{
	if (m->type() == CLMSG_STATS)
	{
		updateInstantaneousStats();
		(dynamic_cast<ClMsgStats*>(m))->setStats(stats_ptr);
		return 0;
	}
	return MPhy_Bpsk::recvSyncClMsg(m);
}

void
UwOpticalStatsPhy::startRx(Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	if ((PktRx == 0) && (txPending == false)) {
		double snr_dB = getSNRdB(p);
		if (snr_dB > MPhy_Bpsk::getAcquisitionThreshold()) {
			if (ph->modulationType == MPhy_Bpsk::modid) {
				PktRx = p;
				Phy2MacStartRx(p);
				return;
			} else {
				if (debug_)
					std::cout << "UwOpticalStatsPhy::Drop packet::Wrong modulation" << std::endl;
			}
		} else {
			if (debug_)
				std::cout << "UwOpticalStatsPhy::Drop packet::Below threshold: SNRdB = " << snr_dB
						<< ", threshold = " << MPhy_Bpsk::getAcquisitionThreshold() << std::endl;
		}
	} else {
		if (debug_)
			std::cout << "UwOpticalStatsPhy::Drop packet::Synced onto another packet: PktRx = "
					<< PktRx << ", pending = " << txPending << std::endl;
	}
}

void
UwOpticalStatsPhy::endRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);

	if (MPhy_Bpsk::PktRx != 0) {
		if (MPhy_Bpsk::PktRx == p) {
			if (interference_) {
				double interference_power = interference_->getInterferencePower(p);
				if (interference_power == 0) {
					if (UwOpticalStatsPhy::getSNRdB(p) > UwOpticalStatsPhy::getAcquisitionThresholdDb()) {
						// No interference and SNRdB above threshold
						ch->error() = 0;
						if (debug_) {
							std::cout << "UwOpticalStatsPhy::correct reception" << std::endl;
						}
					} else {
						// No interference but SNRdB below threshold
						ch->error() = 1;
						if (debug_) {
							std::cout << "UwOpticalStatsPhy::SNRdB under threshold: "
							<< UwOpticalStatsPhy::getSNRdB(p) << " dB" << std::endl;
						}
					}
				} else {
					// At least one interferent packet
					ch->error() = 1;
					if (debug_) {
						if (UwOpticalStatsPhy::getSNRdB(p) > UwOpticalStatsPhy::getAcquisitionThresholdDb()) {
							// SNRdB above threshold but at least one interferent packet
							std::cout << "UwOpticalStatsPhy::endRx interference power = "
									<< interference_power << std::endl;
						}
						else {
							// SNRdB below threshold and at least one interferent packet
							std::cout << "UwOpticalStatsPhy::endRx interference power = "
									<< interference_power << " and SNRdB under threshold: " 
									<< UwOpticalStatsPhy::getSNRdB(p) << " dB" << std::endl;
						}
					}	 
				}
			} else {
				// No interference model set
				ch->error() = 1;
			}

			auto uwopticalstats = dynamic_cast<UwOpticalStats*>(stats_ptr);
			if (uwopticalstats) {
				uwopticalstats->updateStats(getId(), getStackId(),
						ph->Pr, ph->Pn, interference_->getInterferencePower(p), ch->error(), getTemperature());
			}

			ClMsgTriggerStats m = ClMsgTriggerStats();
			sendSyncClMsg(&m);

			sendUp(p);
			PktRx = 0;
		} else {
			dropPacket(p);
		}
	} else {
		dropPacket(p);
	}
}

void
UwOpticalStatsPhy::endTx(Packet *p) {
	ClMsgOpDeque d = ClMsgOpDeque(getId());
	sendSyncClMsg(&d);
	MPhy_Bpsk::endTx(p);
}
