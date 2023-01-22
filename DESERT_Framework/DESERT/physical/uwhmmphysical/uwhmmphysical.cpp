//
// Copyright (c) 2021 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwhmmphysical.cpp
 * @author Antonio Montanari
 * @version 1.1.0
 *
 * \brief Implementation of UnderwaterHMMPhysical class
 *
 */

#include "uwhmmphysical.h"
#include "mclink.h"
#include "uwphy-clmsg.h"
#include "uwstats-utilities.h"
#include "mac.h"
#include "clmsg-stats.h"
#include <phymac-clmsg.h>
#include <string.h>
#include <rng.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>
#include <iostream>

#define DEBUG(level,text) {if (debug_ >= level) {std::cout << NOW << " UwHMMPhysical error: "<< text << std::endl;}}


static class UwHMMPhysicalClass : public TclClass
{
public:
	UwHMMPhysicalClass()
		: TclClass("Module/UW/HMMPHYSICAL")
	{
	}
	TclObject *
	create(int argc, const char *const * argv)
	{
		return (new UnderwaterHMMPhysical);
	}
} class_module_uwhmmphysical;

UnderwaterHMMPhysical::UnderwaterHMMPhysical()
	: link_map()
	, step_duration(0.0)
	, pkts_tot_good(0.0)
	, pkts_tot_bad(0.0)
{
	bind("step_duration", &step_duration);
}

int
UnderwaterHMMPhysical::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	
	if (argc == 2) {
		if (strcasecmp(argv[1], "getPktsTotBad") == 0) {
			tcl.resultf("%d", getPktsTotBad());
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getPktsTotGood") == 0) {
			tcl.resultf("%d", getPktsTotGood());
			return TCL_OK;
		}
	}
	if (strcasecmp(argv[1], "setMCLink") == 0) {
		if (argc == 4) {

			auto link = dynamic_cast<MCLink *>(TclObject::lookup(argv[3]));
			if (link) {
				setMCLink(std::stoi(argv[2]), link);
				return TCL_OK;
			}
			std::cerr << "TCL: failed cast on MCLink" << std::endl;
			return TCL_ERROR;
		}  
		std::cerr << "TCL error: setMCLink mac MCLink" << std::endl;
		return TCL_ERROR;
	}
	return UnderwaterPhysical::command(argc, argv);
} /* UnderwaterHMMPhysical::command */

void
UnderwaterHMMPhysical::setMCLink(int mac, MCLink *link) 
{
	link_map[mac] = link;
} /* UnderwaterHMMPhysical::setMCLink */


void
UnderwaterHMMPhysical::endRx(Packet *p){
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_mac *mach = HDR_MAC(p);
	const int mac_addr = mach->macSA();
	
	if (PktRx != 0) {
		if (PktRx == p) {

			/* HMM model */
			auto link = link_map.find(mac_addr);
			if (link != link_map.end()) {
				int curr_step = floor(NOW / step_duration);	
				auto ch_state = (*link->second).updateChState(curr_step);
				bool error_hmm = false;
				double psucc_hmm = (*link->second).getPSucc();
				double chance_hmm = RNG::defaultrng()->uniform_double();
				if (chance_hmm > psucc_hmm) {
					error_hmm = true;
				}
				incrTotPkts(ch_state);
				/* end HMM model */

				/* interference model as in UwPhysical
				with zeroed Pn since channel noise is already modelled by HMM */
				counter interferent_pkts;
				int nbits = ch->size() * 8;
				double interf_power = 0.0;
				double perr_interf = 0.0;
				bool error_interf = false;
				double chance_interf = RNG::defaultrng()->uniform_double();
				
				if (interference_) {
					if (Interference_Model == "CHUNK") {
						const PowerChunkList &power_chunk_list =
								interference_->getInterferencePowerChunkList(p);
						for (PowerChunkList::const_iterator itInterf =
										power_chunk_list.begin();
								itInterf != power_chunk_list.end();
								itInterf++) {
							int nbits2 = itInterf->second * BitRate_;
							interf_power = itInterf->first;
							if (interf_power > 0.0) {
								perr_interf = getPER(
										ph->Pr / interf_power, nbits2, p);
								chance_interf = RNG::defaultrng()->uniform_double();
								error_interf = chance_interf < perr_interf;
								if (error_interf) {
									break;
								}
							}
						}
					} else if (Interference_Model == "MEANPOWER") {
						interf_power = interference_->getInterferencePower(p);
						if (interf_power > 0.0) {
							perr_interf = getPER(
								ph->Pr / interf_power, nbits, p);
							error_interf = chance_interf < perr_interf;
						}
					} else {
						std::cerr << "Please choose the interference model "
									"CHUNK or MEANPOWER"
								<< std::endl;
						exit(1);
					}
					interferent_pkts = interference_->getCounters(p);

				} else {
					interf_power = ph->Pi;
					if (interf_power > 0.0) {
						perr_interf = getPER(ph->Pr / ph->Pi, nbits, p);
						error_interf = chance_interf < perr_interf;
					}
				} /* end of interference model */

				/* update and trigger the modules that collect the stats */
				auto uwphystats = dynamic_cast<UwPhysicalStats *>(stats_ptr);
				if (uwphystats) {
					uwphystats->updateStats(getId(),getStackId(), ph->Pr,
						ph->Pn, interf_power, (error_hmm || error_interf),
						(UwPhysicalStats::ChannelStates)ch_state);
				}
				
				ClMsgTriggerStats m = ClMsgTriggerStats();
				sendSyncClMsg(&m);

				if (time_ready_to_end_rx_ > Scheduler::instance().clock()) {
					Rx_Time_ = Rx_Time_ + ph->duration - time_ready_to_end_rx_ +
							Scheduler::instance().clock();
				} else {
					Rx_Time_ += ph->duration;
				}
				time_ready_to_end_rx_ =
						Scheduler::instance().clock() + ph->duration;
				Energy_Rx_ += consumedEnergyRx(ph->duration);
			
				ch->error() = (error_hmm || error_interf);

				/* update counters of UwPhysical */
				if (error_interf) {
					if (mach->ftype() != MF_CONTROL) {
						incrErrorPktsInterf();
						incrTot_pkts_lost();
						if (interferent_pkts.second >= 1) {
							incrCollisionDATA();
						} else {
							if (interferent_pkts.first > 0) {
								incrCollisionDATAvsCTRL();
							}
						}
					} else if (mach->ftype() == MF_CONTROL) {
						incrTotCrtl_pkts_lost();
						incrErrorCtrlPktsInterf();
						if (interferent_pkts.first > 0) {
							incrCollisionCTRL();
						}
					}
					if (error_hmm) {
						incrErrorPktsNoise();
					}
				} else if (error_hmm) { //only HMM error, no interference
					incrErrorPktsNoise();
					if (mach->ftype() != MF_CONTROL) {
						incrTot_pkts_lost();
					} else if (mach->ftype() == MF_CONTROL) {
						incrTotCrtl_pkts_lost();
					}
				} /* done update UwPhysical counters */

				sendUp(p);
				PktRx = 0;
			} else {
				ch->error() = 1;
				sendUp(p);
				PktRx = 0;
				DEBUG(0,"McLink not set for origin MAC: "<< mac_addr)
			}
		} else {
			dropPacket(p);
		}
	} else {
		dropPacket(p);
	}
} /* UnderwaterHMMPhysical::endRx */


