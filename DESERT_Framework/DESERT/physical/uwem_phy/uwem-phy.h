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
 * @file   uwem-phy.h
 * @author Riccardo Tumiati
 * @version 1.0.0
 *
 * \brief Definition of UwEm class.
 *
 */

#ifndef UWEM_PHY_H
#define UWEM_PHY_H

#include <bpsk.h>
#include <rng.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>

#include <iostream>
#include <string.h>
#include <cmath>
#include <limits>
#include <climits>
#include <math.h>
#include <node-core.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#define ELECTROMAGNETIC_MODULATION_TYPE "ELECTROMAGNETIC_BPSK"

typedef ::std::map<double, double> RSSIMap;
typedef RSSIMap::iterator DMIt;

class UwElectroMagneticPhy : public MPhy_Bpsk
{

public:
	/**
	 * Constructor of UwElectroMagneticPhy class.
	 */
	UwElectroMagneticPhy();

	/**
	 * Destructor of UwElectroMagneticPhy class.
	 */
	virtual ~UwElectroMagneticPhy()
	{
	}

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
	virtual int command(int, const char *const *);

protected:

	void startRx(Packet *p);

	void endRx(Packet *p);

	double getRxPower(Packet *p);

	double getPER(double rx_power);

	virtual double linearInterpolator(
			double x, double x1, double y1, double x2, double y2);

	/**
	 * Inizialize LUT of c_variable values
	*/
	virtual void initializeLUT();

	// Variables
private:
	// Variables
	string lut_file_name_; // LUT file name
	char lut_token_separator_; //
	RSSIMap lut_map; /**< Lookup table map of the RSSI versus PER*/
	double rxPowerThreshold_;
};

#endif /* UWEM_PHY_H  */
