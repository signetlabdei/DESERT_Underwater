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
 * @file   uwahoi-phy.cpp
 * @author Filippo Campagnaro
 * @version 1.1.0
 *
 * \brief Implementation of UwAhoiPhy class
 *
 */

#include "uwahoi-phy.h"
#include <fstream>
#include <sstream>

/**
 * Adds the module for UwCbrModuleClass in ns2.
 */
static class UwAhoiPhyClass : public TclClass
{
public:
	UwAhoiPhyClass()
		: TclClass("Module/UW/AHOI/PHY")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UwAhoiPhy);
	}
} class_module_UwAhoiPhy;
/*
L{ 25, 50, 95, 120, 140, 160, 180, 190 },
P_SUCC{ 0.923077*39/40, 0.913793*58/60, 0.924528*53/60, 0.876712*73/80,
  0.61643*73/80, 0.75*28/40, 0.275862*29/40, 0 }*/
UwAhoiPhy::UwAhoiPhy()
	: UnderwaterPhysical()
	, pdr_file_name_("dbs/ahoi/default_pdr.csv")
	, sir_file_name_("dbs/ahoi/default_sir.csv")
	, pdr_token_separator_(',')
	, range2pdr_()
	, sir2pdr_()
	, initLUT_(false)
{ // binding to TCL variables
	Interference_Model = "MEANPOWER";
}

UwAhoiPhy::~UwAhoiPhy()
{
}

int
UwAhoiPhy::command(int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcasecmp(argv[1], "initLUT") == 0) {
			initializeLUT();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setRangePDRFileName") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty string for the file name");
				return TCL_ERROR;
			}
			pdr_file_name_ = tmp_;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setSIRFileName") == 0) {
			string tmp_ = ((char *) argv[2]);
			if (tmp_.size() == 0) {
				fprintf(stderr, "Empty string for the file name");
				return TCL_ERROR;
			}
			sir_file_name_ = tmp_;
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
UwAhoiPhy::initializeLUT()
{
	ifstream input_file_;
	string line_;
	char *tmp_ = new char[pdr_file_name_.length() + 1];
	strcpy(tmp_, pdr_file_name_.c_str());
	input_file_.open(tmp_);
	if (input_file_.is_open()) {
		// skip first 2 lines
		if (debug_)
			std::cout << "UwAhoiPhy::initializeRangeLUT()" << endl;
		while (std::getline(input_file_, line_)) {
			// TODO: retrive the right row and break the loop
			std::istringstream line_stream(line_);
			std::string token;
			std::getline(line_stream, token, pdr_token_separator_);
			double d = stod(token);
			std::getline(line_stream, token, pdr_token_separator_);
			double p = stod(token);
			range2pdr_[d] = p;
			if (debug_)
				std::cout << d << " " << p << endl;
		}
	} else {
		cerr << "Impossible to open file " << pdr_file_name_ << endl;
	}

	ifstream input_file_2;
	tmp_ = new char[sir_file_name_.length() + 1];
	strcpy(tmp_, sir_file_name_.c_str());
	input_file_2.open(tmp_);
	if (input_file_2.is_open()) {
		// skip first 2 lines
		if (debug_)
			std::cout << "UwAhoiPhy::initializeSIRLUT()" << endl;
		while (std::getline(input_file_2, line_)) {
			// TODO: retrive the right row and break the loop
			std::istringstream line_stream(line_);
			std::string token;
			std::getline(line_stream, token, pdr_token_separator_);
			double sir = stod(token);
			std::getline(line_stream, token, pdr_token_separator_);
			double p = stod(token);
			sir2pdr_[sir] = p;
			if (debug_)
				std::cout << sir << " " << p << endl;
		}
	} else {
		cerr << "Impossible to open file " << sir_file_name_ << endl;
	}

	initLUT_ = true;
}

void
UwAhoiPhy::endRx(Packet *p)
{
	if (!initLUT_)
		cerr << "UwAhoiPhy ERROR: FIRST INITIALIZE LUT!" << endl;
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
			double x = RNG::defaultrng()->uniform_double();
			per_n = getPER(ph->Pr / ph->Pn, p);
			bool error_n = x <= per_n;
			error_n = 0;
			bool error_ni = 0;
			double interference = interference_ ? 
				interference_->getInterferencePower(p):0;
			if (!error_n) {
				per_ni = interference?getPER(ph->Pr/interference,p):getPER(0,p); 
				error_ni = x <= per_ni;
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
							<< NOW << "  UwAhoiPhy(" << mac_addr
							<< ")::endRx() packet " << ch->uid()
							<< " contains errors due to noise and interference."
							<< std::endl;
				} else if (error_n == 1) {
					std::cout << NOW << "  UwAhoiPhy(" << mac_addr
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
UwAhoiPhy::getPER(double _sir, Packet *_p)
{
	double pdr = matchDistancePDR(getDistance(_p));

	if(_sir > 0) {
		pdr = pdr * matchSIR_PDR(_sir);
	}
	return 1 - pdr;
}

double
UwAhoiPhy::getDistance(Packet *_p)
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
UwAhoiPhy::matchDistancePDR(double distance)
{ // success probability of AHOI
	if (debug_)
		std::cout << NOW
				  << "  UwAhoiPhy()::matchDistancePDR(double distance)"
				  << "distance = " << distance << std::endl;
	if (distance <= range2pdr_.begin()->first)
		return range2pdr_.begin()->second;

	PdrLut::iterator it = range2pdr_.lower_bound(distance);
	if (it == range2pdr_.end())
		return (--it)->second;
	double l_sup = it->first;
	double p_sup = it->second;
	it--;
	double l_inf = it->first;
	double p_inf = it->second;
	if (debug_) {
		std::cout << " Distance between " << l_inf << " and " << l_sup;
		std::cout << " Succ Prob between " << p_inf << " and " << p_sup;
	}
	return linearInterpolator(distance, l_inf, p_inf, l_sup, p_sup);
}

double
UwAhoiPhy::matchSIR_PDR(double sir)
{ // success probability of AHOI
	double sir_db = 10*log10(sir/10);
	if (debug_)
		std::cout << NOW
				  << "  UwAhoiPhy()::matchSIR_PDR(double sir)"
				  << "sir = " << sir << std::endl;
	if (sir_db <= sir2pdr_.begin()->first)
		return sir2pdr_.begin()->second;

	PdrLut::iterator it = sir2pdr_.lower_bound(sir_db);
	if (it == sir2pdr_.end())
		return (--it)->second;
	double l_sup = it->first;
	double p_sup = it->second;
	it--;
	double l_inf = it->first;
	double p_inf = it->second;
	if (debug_) {
		std::cout << " SIR between " << l_inf << " and " << l_sup;
		std::cout << " Succ Prob between " << p_inf << " and " << p_sup;
	}
	return linearInterpolator(sir_db, l_inf, p_inf, l_sup, p_sup);
}

double
UwAhoiPhy::linearInterpolator(
		double x, double x1, double y1, double x2, double y2)
{
	double m = (y1 - y2) / (x1 - x2);
	double q = y1 - m * x1;
	if (debug_)
		std::cout << NOW << "  UwAhoiPhy::linearInterpolator( double x, "
							"double x1, double y1, double x2, double y2 )"
				  << "m = " << m << " q= " << q << std::endl;
	return m * x + q;
}

