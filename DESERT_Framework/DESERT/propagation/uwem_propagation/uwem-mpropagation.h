//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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

/**−3.56417 × 10−3𝑆 + 4.74868 × 10−6𝑆2 + 1.15574 × 10−5𝑇𝑆pagation class.
 *
 */

#ifndef UWEM_MPROPAGATION_H
#define UWEM_MPROPAGATION_H

#include <mpropagation.h>
#include <mphy.h>
#include <iostream>
#include <math.h>
#include <map>

#define NOT_FOUND_C_VALUE -1
#define MU_0 (1.2566 * pow(10,-6))		// [H/m]	
#define E_0 (8.854187817 * pow(10,-12))	// [F/m]
#define ETA_A 377						// [Ohm]

/**
 * Class used to represents the UWOPTICAL_MPROPAGATION.
 */
class UwElectroMagneticMPropagation : public MPropagation
{
public:
	/**
	 * Constructor of the UwElectroMagneticMPropagation class
	 */
	UwElectroMagneticMPropagation();

	/**
	 * Destructor of the UwElectroMagneticMPropagation class
	 */
	virtual ~UwElectroMagneticMPropagation()
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

	/**
	 * Calculate the gain of the transmission (attenuation) 
	 *
	 * @param Packet* Pointer to the packet that has to be received.
	 * @return the gain due to the electromagnetic propagation.
	 *
	 */

	virtual double getGain(Packet *p);

	int debug_;

protected:
	double T_; /**< Temperature [°C] */
	double S_; /**< Salinity [(g/kg)^2] */
	
	void getRelativePermittivity(double f_,double* rel_e);
	double getSigmaSW();
	double getAlpha(double* rel_e,double w,double sigma);
	double getBeta(double* rel_e,double w,double sigma);
	void getEtaW(double* etaW,double* rel_e,double w,double sigma);
	double getModTauSquared(double* etaW);
};

#endif /* UWEM_MPROPAGATION_H */
