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
 * @file   uwoptical-mpropagation.h
 * @author Filippo Campagnaro, Federico Favaro, Federico Guerra
 * @version 1.0.0
 *
 * \brief Definition of UwOpticalMPropagation class.
 *
 */

#ifndef UWOPTICAL_MPROPAGATION_H
#define UWOPTICAL_MPROPAGATION_H

#include <mpropagation.h>
#include <mphy.h>
#include <iostream>
#include <map>

#define NOT_FOUND_C_VALUE -1
#define NOT_VARIABLE_TEMPERATURE -20

/*typedef ::std::map< double, double > LUT_c;*/
typedef ::std::pair<double, double> c_temperat;
typedef ::std::map<double, c_temperat> LUT_c;
typedef LUT_c::iterator LUT_c_iter;

/**
 * Class used to represents the UWOPTICAL_MPROPAGATION.
 */
class UwOpticalMPropagation : public MPropagation
{
public:
	/**
	 * Constructor of the UwOpticalMPropagation class
	 */
	UwOpticalMPropagation();

	/**
	 * Destructor of the UwOpticalMPropagation class
	 */
	virtual ~UwOpticalMPropagation()
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
	 * Calculate the gain following the Lambert and Beer's law
	 *
	 * @param Packet* Pointer to the packet that has to be received.
	 * @return the gain due to the optical propagation.
	 *
	 */

	virtual double getGain(Packet *p);

	virtual void setWoss(bool flag);

	/**
	 * Provide the temperature from LUT
	 *
	 * @param depth depth of the receiver.
	 * @return the temperature at certain depth.
	 *
	 */
	double getTemperature(double depth);
	
	/**
	 * Provide angle between transmitter and receiver.
	 *
	 * @param Packet* Pointer to the packet that has to be received.
	 * @return the angle between transmitter and receiver.
	 *
	 */
	double getBeta(Packet *p);

	/**
	 * Provide the attenuation coefficient.
	 *
	 * @return c.
	 *
	 */
	double getC(Packet *p = NULL);

	/**
	 * Provide if we are assuming omnidirectional tx and rx or not.
	 *
	 * @return if it is omnidirectional or not.
	 *
	 */
	bool isOmnidirectional();

	int debug_;

protected:
	/**
	 * Calculate the gain following the Lambert and Beer's law
	 *
	 * @param d Distance between transmitter and receiver.
	 * @param beta inclination angle between the transmitter and the receiver.
	 * @return the gain due to the optical propagation.
	 *
	 */
	virtual double getLambertBeerGain(double d, double beta_);

	/**
	 * Inizialize LUT of c_variable values
	 */
	virtual void initializeLUT();

	/**
	 * Calculate the inclination angle between the transmitter and the receiver
	 *
	 * @param src pointer to the transmitter position.
	 * @param dest pointer to the receiver position.
	 * @return the inclination angle between the transmitter and the receiver
	 *
	 */
	double getWossOrientation(Position *src, Position *dest);

	/**
	 * Calculate the linear interpolation between two 2-D points
	 *
	 * @param x x-coordinate of which we need to finde the value.
	 * @param x1 x-coordinate of the first point
 	 * @param x2 x-coordinate of the second point
	 * @param y1 y-coordinate of the first point
	 * @param y2 y-coordinate of the second point
	 * @return the value assumed by y obtained by linear interpolation
	 */
	double linearInterpolator(
			double x, double x1, double x2, double y1, double y2);

	/**
	 * Set the attenuation coefficient from lookup table when both the nodes are
	 * alligned and at the same depth
	 *
	 * @param d depth of the nodes.
	 *
	 */
	void updateC(double d);

	/**
	 * Calculate the gain following the Lambert and Beer's law in the case of
	 * variable c model
	 *
	 * @param beta_
	 * @param min_depth_ the max between depth of rx and depth of tx.
	 * @param max_depth_ the min between depth of rx and depth of tx.
	 * @return the gain due to the optical propagation in the case of variable c
	 * model.
	 *
	 */
	double getLambertBeerGain_variableC(
			double beta_, double min_depth_, double max_depth_);

	double Ar_; /**< Receiver area [m^2] */
	double At_; /**< Transmitter size [m^2] */
	double c_; /**< Beam light attenuation coefficient c = a + b [m^-1] */
	double theta_; /**< Transmitting beam diverge angle [rad] */
	bool omnidirectional_; /**< Flag to set whether the system is
							  omnidirectional or not. By default it is false */
	bool variable_c_; /**< Flag to set whether the attenuation is constant or
						 not. By default it is false */
	bool use_woss_; /**< Flag to set the woss. By default it is false */
	LUT_c lut_c_; /**< Lookup table map of the attenuation coefficient and the
					 temperature versus the depth*/
	string lut_file_name_; /**< LUT file name */
	char lut_token_separator_; /**< LUT token separator */
};

#endif /* UWOPTICAL_MPROPAGATION_H */
