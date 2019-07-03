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

/**
 * @file   uwopticalbeampattern.h
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Definition of UwOptical class.
 *
 */

#ifndef UWOPTICALBEAMPATTERN_PHY_H
#define UWOPTICALBEAMPATTERN_PHY_H

#include "uwopticalbeampattern-hdr.h"
#include <uwoptical-phy.h>


//structure for max distance from LUT without noise, with noise
struct MaxDist
{
	double max_range;
	double max_range_with_noise;
};
typedef ::std::map<double, MaxDist> CMaxDist; // max distance per c
typedef CMaxDist::iterator CMaxDistIter;

// LUT of angle in radiance vs normalized beam pattern
typedef ::std::map<double, double> BeamPattern; 
typedef BeamPattern::iterator BeamPatternIter;


class UwOpticalBeamPattern : public UwOpticalPhy
{

public:
	/**
	 * Constructor of UwMultiPhy class.
	 */
	UwOpticalBeamPattern();

	/**
	 * Destructor of UwMultiPhy class.
	 */
	virtual ~UwOpticalBeamPattern()
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


	virtual void startRx(Packet *p);

	virtual void startTx(Packet *p);

protected:

	/**
	 * Inizialize all the LUTs
	*/
	void initializeLUT();

	/**
	 * Inizialize beam pattern LUT
	*/
	void initializeBeamLUT(BeamPattern &beam_lut_, string beam_pattern_path_);

	/**
	 * Inizialize max range LUT
	*/
	void initializeMaxRangeLUT();

	/**
	 * Get the transmission range in the current conditions.
	*/
	double getMaxTxRange(Packet *p);

	/**
	 * Get the maximum transmission range for these water properties.
	*/
	double getLutMaxDist(double c, double na);
	double getLutBeamFactor(BeamPattern &beam_lut_, double beta);
	double getBetaRx(Packet *p);
	double getBetaXY(Packet *p, double rotation_angle);
	double getBetaXYRx(Packet *p);
	double getBetaXYTx(Packet *p);
	double getBetaTx(Packet *p);
	
private:
	
	//string beam_pattern_path_; // LUT file name
	string beam_pattern_path_rx_; // LUT file name
	string beam_pattern_path_tx_; // LUT file name
	string max_dist_path_; // LUT file name
	char beam_pattern_separator_; //
	char max_dist_separator_; //
	CMaxDist dist_lut_;
	//BeamPattern beam_lut_;
	BeamPattern beam_lut_rx_;
	BeamPattern beam_lut_tx_;
	double back_noise_threshold_;
	double inclination_angle_; /**< Angle of inclination from the 0 Zenith*/

	bool sameBeam; // Set to 1 if tx and rx use the same beam pattern

	void checkInclinationAngle();
};

#endif /* UWOPTICAL_H  */
