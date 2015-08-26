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
//    derived from this software without specific prior written permission
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
 * @file   uwphysicalrogersmodel.cc
 * @author Giovanni Toso
 * @version 1.0.0
 *
 * \brief Implementation of UnderwaterPhysicalRogersModelRogersModel class
 *
 */

#include "uwphysicalrogersmodel.h"

static class UwPhysicalRogersModelClass : public TclClass {
public:
    UwPhysicalRogersModelClass() : TclClass("Module/UW/PROPAGATIONROGERS") {}
    TclObject* create(int, const char*const*) {
        return (new UnderwaterPhysicalRogersModel);
    }
} class_module_UnderwaterPhysicalRogersModel;

UnderwaterPhysicalRogersModel::UnderwaterPhysicalRogersModel() :
bottom_depth(100),
sound_speed_water_bottom(1500),
sound_speed_water_surface(1520),
sound_speed_sediment(1585),
density_sediment(1.740),
density_water(1),
attenuation_coeff_sediment(0.51),
debug_(0)
{
    bind("bottom_depth_", &bottom_depth);
    bind("sound_speed_water_bottom_", &sound_speed_water_bottom);
    bind("sound_speed_water_surface_", &sound_speed_water_surface);
    bind("sound_speed_sediment_", &sound_speed_sediment);
    bind("density_sediment_", &density_sediment);
    bind("density_water_", &density_water);
    bind("attenuation_coeff_sediment_", &attenuation_coeff_sediment);
    bind("debug_", &debug_);
}

int UnderwaterPhysicalRogersModel::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();

    if (argc == 2) {
        if (strcasecmp (argv[1], "getBottomDepth") == 0) {
            tcl.resultf("%f", bottom_depth);
            return TCL_OK;
        } else if (strcasecmp (argv[1], "getSoundSpeedWaterBottom") == 0) {
            tcl.resultf("%f", sound_speed_water_bottom);
            return TCL_OK;
        } else if (strcasecmp (argv[1], "getSoundSpeedWaterSurface") == 0) {
            tcl.resultf("%f", sound_speed_water_surface);
            return TCL_OK;
        } else if (strcasecmp (argv[1], "getSoundSpeedSediment") == 0) {
            tcl.resultf("%f", sound_speed_sediment);
            return TCL_OK;
        } else if (strcasecmp (argv[1], "getDensitySediment") == 0) {
            tcl.resultf("%f", density_sediment);
            return TCL_OK;
        } else if (strcasecmp (argv[1], "getDensityWater") == 0) {
            tcl.resultf("%f", density_water);
            return TCL_OK;
        } else if (strcasecmp (argv[1], "getAttenuationCoeffSediment") == 0) {
            tcl.resultf("%f", attenuation_coeff_sediment);
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "setBottomDepth") == 0) {
            bottom_depth = static_cast<double>(atof(argv[2]));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setSoundSpeedWaterBottom") == 0) {
            sound_speed_water_bottom = static_cast<double>(atof(argv[2]));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setSoundSpeedWaterSurface") == 0) {
            sound_speed_water_surface = static_cast<double>(atof(argv[2]));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setSoundSpeedSediment") == 0) {
            sound_speed_sediment = static_cast<double>(atof(argv[2]));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setDensitySediment") == 0) {
            density_sediment = static_cast<double>(atof(argv[2]));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setDensityWater") == 0) {
            density_water = static_cast<double>(atof(argv[2]));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setAttenuationCoeffSediment") == 0) {
            attenuation_coeff_sediment = static_cast<double>(atof(argv[2]));
            return TCL_OK;
        }
    }
    return UnderwaterMPropagation::command(argc, argv);
} /* UnderwaterPhysicalRogersModel::command */

double UnderwaterPhysicalRogersModel::getGain(Packet* p) {
    hdr_MPhy *ph = HDR_MPHY(p);

    Position* sp = ph->srcPosition;
    Position* rp = ph->dstPosition;

    MSpectralMask* sm = ph->srcSpectralMask;

    assert(sp);
    assert(rp);
    assert(sm);

    const double frequency_ = ph->srcSpectralMask->getFreq(); // Frequency of the carrier in Hz
    const double distance_ = sp->getDist(rp); // Distance in meters

    // getAttenuation is in dB. The attanuation is converted in linear and it is converted in gain.
    const double gain = pow (10, -0.1 * getAttenuation(sound_speed_water_bottom, distance_, frequency_, bottom_depth));
    //const double gainUrick = uwlib_AInv(distance_/1000.0, uw.practical_spreading, frequency_/1000.0);

    //std::cout << (10*log10(gain) - 10*log10(gainUrick)) << std::endl;
    if (debug_)
        std::cout << NOW
        << " UnderwaterPhysicalRogersMode::getGain()"
        << " distance=" << distance_
        << " frequency=" << frequency_
        << " gain=" << gain
        << std::endl;
    return (gain);
} /* UnderwaterPhysicalRogersModel::getGain */

double UnderwaterPhysicalRogersModel::getAttenuation(const double& _sound_speed_water_bottom, const double& _distance, const double& _frequency, const double& _bottom_depth) {
    const double theta_g_ = getTheta_g(_bottom_depth, _distance);
    const double theta_l_ = std::max (getTheta_g_max(_sound_speed_water_bottom), getTheta_c(_sound_speed_water_bottom, _frequency, _bottom_depth));

    if (_distance > 0) { // If the distane is known
        if (theta_g_ >= theta_l_) {
            return (15 * log10 (_distance) +
                5 * log10 (_bottom_depth * getBeta()) +
                (getBeta() * _distance * pow(theta_l_, 2)) / (4 * _bottom_depth) -
                7.18 +
                getThorp(_frequency/1000.0) * _distance);
        } else {
            return (10 * log10 (_distance) +
                10 * log10 (_bottom_depth / (2 * theta_l_)) +
                (getBeta() * _distance * pow(theta_l_, 2)) / (4 * _bottom_depth) +
                getThorp(_frequency/1000.0) * _distance);
        }
    } else {
        return 1;
    }
} /* UnderwaterPhysicalRogersModel::getAttenutation */

