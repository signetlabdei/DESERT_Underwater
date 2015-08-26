//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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

/**
 * @file   uwphysicalrogersmodel.h
 * @author Giovanni Toso
 * @version 1.0.0
 *
 * \brief Definition of UwPhysicalRogersModel class.
 *
 */

#ifndef UWPHYSICALROGERSMODEL_H
#define UWPHYSICALROGERSMODEL_H

#include <underwater-mpropagation.h>
#include <node-core.h>
#include <uwlib.h>

#include <cmath>
#include <iostream>

class UnderwaterPhysicalRogersModel : public UnderwaterMPropagation {

public:
    /**
     * Constructor of UnderwaterMPhyBpskDb class.
     */
    UnderwaterPhysicalRogersModel();

    /**
     * Destructor of UnderwaterMPhyBpskDb class.
     */
    virtual ~UnderwaterPhysicalRogersModel() { }

    /**
     * TCL command interpreter. It implements the following OTcl methods:
     *
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     *
     */
    virtual int command(int, const char*const*);

protected:
    virtual double getGain(Packet* p);

    /**
     * Attenuation of acoustic signal in underwater channel.
     * The value returned is base on Rogers model for shallow water.
     *
     * @param _sound_speed_water_bottom sound speed of the sound at the bottom (in m/s)
     * @param _distance distance between source and destination (in meters)
     * @param _frequency frequency (in kHz)
     * @param _bottom_depth height of the column of water (in meters)
     *
     * @return Attenuation in dB
     *
     */
    virtual double getAttenuation (const double& _sound_speed_water_bottom, const double& _distance, const double& _frequency, const double& _bottom_depth);

    /**
     * Bottom loss (dB/rad) derived from the expression for the Rayleigh reflection coefficient for a two-fluid lossy interface
     *
     * return Bottom loss (dB/rad)
     */
    inline const double getBeta () const {
        return (0.477 * get_M0() * get_N0() * get_Ks()) / sqrt (pow ((1 - pow (get_N0(), 2)), 3));
    }

    /**
     * Sediment attenuation coefficient (dB/(m*kHz))
     *
     * @return Sediment attenuation coefficient
     */
    inline const double get_Ks () const {
        //TODO: fix this and find the correct value
        return attenuation_coeff_sediment;
    }

    /**
     * Ratio between the sound speed on sound at the surface and sound speed of sound in the sediment
     */
    inline const double get_N0 () const {
        return (sound_speed_water_bottom / sound_speed_sediment);
    }

    /**
     * Ratio between the density of the sediment and the density of the water
     */
    inline const double get_M0 () const {
        return (density_sediment / density_water);
    }

    /**
     * Magnitude of the negative sound speed profile
     *
     * @return Magnitude of the negative sound speed profile
     */
    inline const double get_g () const {
        return std::abs (sound_speed_water_surface - sound_speed_water_bottom);
    }

    /**
     * Effective angle of the last mode striped
     *
     * @param _bottom_depth height of the column of water in meters
     * @param _distance distance between source and destination (in meters)
     *
     * @return Effective angle of the last mode striped
     */
    inline const double getTheta_g (const double& _bottom_depth, const double& _distance) const {
        return std::sqrt ((1.7 * _bottom_depth) / (getBeta() * _distance));
    }

    /**
     * Maximum grazion angle for an RBR mode
     *
     * @param _sound_speed_bottom sound speed in m/s of the sound at the bottom
     *
     * @return Maximum grazion angle for an RBR mode
     */
    inline const double getTheta_g_max (const double& _sound_speed_water_bottom) const {
        return std::sqrt ((2 * get_g()) / (_sound_speed_water_bottom));
    }

    /**
     * Cutoff angle of the lowest mode
     *
     * @param _sound_speed_water_bottom sound speed in m/s of the sound at the bottom
     * @param _frequency in kHz
     * @param _bottom_depth height of the column of water in meters
     *
     * @return Cutoff angle of the lowest mode
     */
    inline const double getTheta_c (const double& _sound_speed_water_bottom, const double& _frequency, const double& _bottom_depth) const {
        return (_sound_speed_water_bottom / (2 * _frequency * _bottom_depth));
    }

    /**
     * Absorption coefficient calculated by using Thorp's equation.
     *
     * @param _frequency in kHz
     *
     * @return absorprion coefficient in dB/m
     */
    double getThorp(double _frequency) {
        double f2_ = pow (_frequency, 2);
        // Thorp's eqution for frequencies above or below a few kHz is different.
        if (_frequency >= 0.4)
            return (0.11 * f2_ / (1.0 + f2_) + 44.0 * f2_ / (4100.0 + f2_) +  2.75e-4 * f2_ + 0.003) * FROMDBPERKYARDTODMPERM;
        else
            return (0.002  + 0.11 * f2_ / (1 + f2_) + 0.011 * f2_) * FROMDBPERKYARDTODMPERM;
    }

    static const double FROMDBPERKYARDTODMPERM = 0.001093613298338; /**< Conversion factor from dB/kyard to dB/m. */
private:
    //Variables
    double bottom_depth;               /**< Water depth (m) */
    double sound_speed_water_bottom;   /**< Speed of sound in water at the sea bottom level (m/s). */
    double sound_speed_water_surface;  /**< Speed of sound in water at the sea surface level (m/s). */
    double sound_speed_sediment;       /**< Speed of sound in the sediment (m/s). */
    double density_sediment;           /**< Sediment density (g/cm^3). */
    double density_water;              /**< Water density (g/cm^3). */
    double attenuation_coeff_sediment; /**< Attenuation coefficient of the sediment (dB/(m*kHz)). */
    int debug_;                        /**< Debug level. */
};

#endif /* UWPHYSICALROGERSMODEL_H  */
