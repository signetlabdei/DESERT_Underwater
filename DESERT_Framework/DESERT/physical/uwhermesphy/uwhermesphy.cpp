//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
//  names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
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
 * @file   uwhermesphy.cpp
 * @author Filippo Campagnaro
 * @version 1.1.0
 *
 * \brief Implementation of UwHermesPhy class
 *
 */

#include "uwhermesphy.h"
#include <fstream>
#include <sstream>

/**
 * Adds the module for UwCbrModuleClass in ns2.
 */
static class UwHermesPhyClass : public TclClass
{
public:
	UwHermesPhyClass()
		: TclClass("Module/UW/HERMES/PHY")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UwHermesPhy);
	}
} class_module_uwhermesphy;
/*
L{ 25, 50, 95, 120, 140, 160, 180, 190 },
P_SUCC{ 0.923077*39/40, 0.913793*58/60, 0.924528*53/60, 0.876712*73/80,
  0.61643*73/80, 0.75*28/40, 0.275862*29/40, 0 }*/
UwHermesPhy::UwHermesPhy()
	: UnderwaterPhysical()
	, pdr_file_name_("dbs/hermes/default.csv")
	, pdr_token_separator_('\t')
	, initLUT_(false)
{ // binding to TCL variables
	bind("BCH_N", &BCH_N);
	bind("BCH_N", &BCH_K);
	bind("BCH_N", &BCH_T);
	bind("FRAME_BIT", &FRAME_BIT);
	Interference_Model = "MEANPOWER";
}

UwHermesPhy::~UwHermesPhy()
{
}

int
UwHermesPhy::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "initLUT") == 0) {
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
			pdr_file_name_ = tmp_;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setLUTSeparator") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty char for the file name");
				return TCL_ERROR;
			}
			pdr_token_separator_ = tmp_.at(0);
			return TCL_OK;
		}
	}
	return UnderwaterPhysical::command(argc, argv);
}

void
UwHermesPhy::initializeLUT()
{
	ifstream input_file_;
	string line_;
	char *tmp_ = new char[pdr_file_name_.length() + 1];
	strcpy(tmp_, pdr_file_name_.c_str());
	input_file_.open(tmp_);
	if (input_file_.is_open()) {
		// skip first 2 lines
		if (debug_)
			std::cout << "UwHermesPhy::initializeLUT()" << endl;
		while (std::getline(input_file_, line_)) {
			// TODO: retrive the right row and break the loop
			::std::stringstream line_stream(line_);
			double d;
			double p;
			line_stream >> d;
			line_stream >> p;
			range2pdr_[d] = p;
			if (debug_)
				std::cout << d << " " << p << endl;
		}
	} else {
		cerr << "Impossible to open file " << pdr_file_name_ << endl;
	}
	initLUT_ = true;
}

void
UwHermesPhy::endRx(Packet *p)
{
	if (!initLUT_)
		cerr << "UwHermesPhy ERROR: FIRST INITIALIZE LUT!" << endl;
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_mac *mach = HDR_MAC(p);
	counter interferent_pkts;
	static int mac_addr = -1;
	ClMsgPhy2MacAddr msg;
	sendSyncClMsg(&msg);
	mac_addr = msg.getAddr();
	if (PktRx != 0) {
		if (PktRx == p) {
			double per_ni; // packet error rate due to noise and/or interference
			double per_n; // packet error rate due to noise only
			int nbits = ch->size() * 8;
			double x = RNG::defaultrng()->uniform_double();
			per_n = getPER(ph->Pr / ph->Pn, nbits, p);
			bool error_n = x <= per_n;
			bool error_ni = 0;
			if (!error_n) {
				if (interference_) {
					if (Interference_Model == "MEANPOWER") { // only meanpower
															 // is allow in
															 // Hermesphy
						double interference =
								interference_->getInterferencePower(p);
						per_ni = interference > 0; // the Hermes interference
												   // model is unknown, thus it
												   // is taken as always
												   // destructive
						if (per_ni and debug_)
							std::cout << "INTERF" << interference << std::endl;
					} else {
						std::cerr << "Please choose only MEANPOWER as "
									 "Interference_Model"
								  << std::endl;
						exit(1);
					}
					interferent_pkts = interference_->getCounters(p);

				} else {
					per_ni = getPER(ph->Pr / (ph->Pn + ph->Pi),
							nbits,
							p); // PER of Hermes acoustic modem
					error_ni = x <= per_ni;
				}
			}
			if (time_ready_to_end_rx_ > Scheduler::instance().clock()) {
				Rx_Time_ = Rx_Time_ + ph->duration - time_ready_to_end_rx_ +
						Scheduler::instance().clock();
			} else {
				Rx_Time_ += ph->duration;
			}
			time_ready_to_end_rx_ =
					Scheduler::instance().clock() + ph->duration;
			Energy_Rx_ += consumedEnergyRx(ph->duration);

			ch->error() = error_ni || error_n;
			if (debug_) {
				if (error_ni == 1) {
					std::cout
							<< NOW << "  UwHermesPhy(" << mac_addr
							<< ")::endRx() packet " << ch->uid()
							<< " contains errors due to noise and interference."
							<< std::endl;
				} else if (error_n == 1) {
					std::cout << NOW << "  UwHermesPhy(" << mac_addr
							  << ")::endRx() packet " << ch->uid()
							  << " contains errors due to noise." << std::endl;
				}
			}
			if (error_n) {
				incrErrorPktsNoise();
				if (mach->ftype() != MF_CONTROL) {
					incrTot_pkts_lost();
				} else if (mach->ftype() == MF_CONTROL) {
					incrTotCrtl_pkts_lost();
				}
			} else if (error_ni) {
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
			}
			sendUp(p);
			PktRx = 0;
		} else {
			dropPacket(p);
		}
	} else {
		dropPacket(p);
	}
}

double
UwHermesPhy::getPER(double _snr, int _nbits, Packet *_p)
{
	double distance = getDistance(_p);
	return 1 - matchPS(distance, _nbits);
}

double
UwHermesPhy::getDistance(Packet *_p)
{
	hdr_MPhy *ph = HDR_MPHY(_p);
	double x_src = (ph->srcPosition)->getX();
	double y_src = (ph->srcPosition)->getY();
	double z_src = (ph->srcPosition)->getZ();
	double x_dst = (ph->dstPosition)->getX();
	double y_dst = (ph->dstPosition)->getY();
	double z_dst = (ph->dstPosition)->getZ();
	return sqrt(pow(x_src - x_dst, 2.0) + pow(y_src - y_dst, 2.0) +
			pow(z_src - z_dst, 2.0));
}

double
UwHermesPhy::matchPS(double distance, int size)
{ // success probability of Hermes
	if (debug_)
		/*    std::cout << NOW << "  UnderwaterPhysical(" << mac_addr <<
		   ")::matchPS(double distance, int size)"
			  << "distance = " << distance <<  " packet size = " << size <<;*/
		std::cout << NOW
				  << "  UwHermesPhy()::matchPS(double distance, int size)"
				  << "distance = " << distance << " packet size = " << size
				  << std::endl;
	if (distance <= range2pdr_.begin()->first)
		return chunckInterpolator(range2pdr_.begin()->second, size);

	PdrLut::iterator it = range2pdr_.lower_bound(distance);
	if (it == range2pdr_.end())
		return chunckInterpolator((--it)->second, size);
	double l_sup = it->first;
	double p_sup = it->second;
	it--;
	double l_inf = it->first;
	double p_inf = it->second;
	if (debug_) {
		std::cout << " Distance between " << l_inf << " and " << l_sup;
		std::cout << " Succ Prob between " << p_inf << " and " << p_sup;
	}
	double p_succ_frame =
			linearInterpolator(distance, l_inf, p_inf, l_sup, p_sup);
	if (debug_)
		std::cout << " Psucc_frame = " << p_succ_frame;
	double ps = chunckInterpolator(p_succ_frame, size);
	if (debug_)
		std::cout << " Ps = " << ps << std::endl;
	return ps;
}

double
UwHermesPhy::linearInterpolator(
		double x, double x1, double y1, double x2, double y2)
{
	double m = (y1 - y2) / (x1 - x2);
	double q = y1 - m * x1;
	if (debug_)
		/*    std::cout << NOW << "  UnderwaterPhysical(" << mac_addr <<
		   ")::linearInterpolator( double x, double x1, double y1, double x2,
		   double y2 )"
			  << "m = " << m << " q= " << q << std::endl;*/
		std::cout << NOW << "  UwHermesPhy::linearInterpolator( double x, "
							"double x1, double y1, double x2, double y2 )"
				  << "m = " << m << " q= " << q << std::endl;
	return m * x + q;
}

double
UwHermesPhy::chunckInterpolator(double p, int size)
{
	int n_chunck_coded_frame = ceil(float(FRAME_BIT) / 11); // BCH(15,11,1)
	int n_chunck_coded_packet = ceil(float(size) / 11);
	if (debug_)
		/*    std::cout <<  NOW << "  UnderwaterPhysical(" << mac_addr <<
		   ")::chunckInterpolator( double p, int size ) n_chunck_coded_frame = "
			  << n_chunck_coded_frame <<  " n_chunck_coded_packet = " <<
		   n_chunck_coded_packet << std::endl;*/
		std::cout << NOW << "  UwHermesPhy::chunckInterpolator( " << p << " "
				  << size << ") n_chunck_coded_frame = " << n_chunck_coded_frame
				  << " n_chunck_coded_packet = " << n_chunck_coded_packet
				  << std::endl;
	return pow(p, (float(n_chunck_coded_packet) / n_chunck_coded_frame));
}
