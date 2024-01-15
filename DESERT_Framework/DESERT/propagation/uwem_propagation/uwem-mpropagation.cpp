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
 * @file   uwem-mpropagation.cc
 * @author Riccardo Tumiati
 * @version 1.0.0
 *
 * \brief Implementation of UwElectroMagneticMPropagation class.
 *
 */

#include <node-core.h>
#include "uwem-mpropagation.h"
#include <math.h>


/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwElectroMagneticMPropagationClass : public TclClass {
public:

  /**
   * Constructor of the class
   */
  UwElectroMagneticMPropagationClass() : TclClass("Module/UW/ElectroMagnetic/Propagation") {}
  /**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
  TclObject* create(int, const char*const*) {
    return (new UwElectroMagneticMPropagation);
  }
}class_UwElectroMagneticMPropagation;


UwElectroMagneticMPropagation::UwElectroMagneticMPropagation()
:
T_(20),
S_(0)
{
  /*bind_error("token_separator_", &token_separator_);*/
  bind("T_", &T_);
  bind("S_", &S_);
  bind("debug_", &debug_);
}

int UwElectroMagneticMPropagation::command(int argc, const char*const* argv){
  if (argc == 3) {
    if (strcasecmp(argv[1], "setT") == 0) {
      T_ = strtod(argv[2], NULL);
      return TCL_OK;
    } 
    else if (strcasecmp(argv[1], "setS") == 0) {
      S_ = strtod(argv[2], NULL);
      return TCL_OK;
    }
  }
  return MPropagation::command(argc, argv);
}

double UwElectroMagneticMPropagation::getGain(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);
  Position* sp = ph->srcPosition;
  Position* rp = ph->dstPosition;
  double f_ = ph -> srcSpectralMask-> getFreq();

  assert(sp);
  assert(rp);

  if(debug_){
		std::cout << NOW << " UwElectroMagneticMPropagation: Tx position= " << sp -> getX() << " " << sp -> getY() << " " << sp -> getZ() << std::endl;
		std::cout << NOW << " UwElectroMagneticMPropagation: Rx position= " << rp -> getX() << " " << rp -> getY() << " " << rp -> getZ() << std::endl;
	}

  double duw;
  double daw;
  if(sp -> getZ() < 0 && rp -> getZ() >= 0){
    duw = fabs(sp -> getZ());
    daw = sqrt(pow(rp -> getZ(),2) + pow((sp -> getY() - rp -> getY()),2) + pow((sp -> getX() - rp -> getX()),2));
  }
  else {
    if(sp -> getZ() >= 0 && rp -> getZ() < 0){
      duw = fabs(rp -> getZ());
      daw = sqrt(pow(sp -> getZ(),2) + pow((sp -> getY() - rp -> getY()),2) + pow((sp -> getX() - rp -> getX()),2));
    }
    else {
      if(sp -> getZ() < 0 && rp -> getZ() < 0){
        duw = sqrt(pow(sp -> getZ() - rp -> getZ(),2) + pow((sp -> getY() - rp -> getY()),2) + pow((sp -> getX() - rp -> getX()),2));
        daw = 0;
      }
      else
        cerr << "Devices aren't placed in the right position" << endl;
    }
  }
  

  double w = 2 * M_PI * (f_);

  double rel_e[2];
  getRelativePermittivity(f_,rel_e);

  double sigmaW = rel_e[1] * E_0 * w;
  double alpha = getAlpha(rel_e,w, sigmaW);
  double beta = getBeta(rel_e,w,sigmaW);

  double etaW[2];
  getEtaW(etaW,rel_e,w,sigmaW);

  double modTauSq = getModTauSquared(etaW);

  double PLaw = 0;
  double PLuw2aw = 0;
  double PLuw = 0;

  if(daw > 0)
    PLaw = 20 * log10(daw) + 20 * log10(f_) - 147.5; 
  
  if(daw > 0 && duw > 0)
    PLuw2aw = 10 * log10(1 / (modTauSq * (etaW[0]/ETA_A)));
  
  if(duw > 0)
    PLuw = 8.69 * alpha * duw + 20 * log10(duw) + 20 * log10(beta) + 6;

  double PLtot = PLaw + PLuw2aw + PLuw;

  if (debug_){
    std::cout << NOW << " UwElectroMagneticMPropagation: distance aw= " << daw << std::endl; 
    std::cout << NOW << " UwElectroMagneticMPropagation: distance uw= " << duw << std::endl; 
    std::cout << NOW << " UwElectroMagneticMPropagation: total attenuation= " << PLtot << std::endl;
  }

  return PLtot;
}

void UwElectroMagneticMPropagation::getRelativePermittivity(double f_, double* e)
{
  double theta = (300/(T_ + 273.15)) - 1;
  double es = 77.66 + 103.3 * theta;
  double e1 = 0.0671 * es;
  double einf = 3.52 - 7.52 * theta;
  double f1 = 20.20 - 146.4 * theta + 316 * pow(theta,2);
  double f2 = 39.8 * f1;
  double fGhz = f_ / 1e9;

  es = es * exp(-3.56417 * 1e-3 * S_ + 4.74868 * 1e-6 * pow(S_,2) + 1.15574 * 1e-5 * T_ * S_);
  e1 = e1 * exp(-6.28908 * 1e-3 * S_ + 1.76032 * 1e-4 * pow(S_,2) - 9.22144 * 1e-5 * T_ * S_);
  einf = einf * (1 + S_* (-2.04265 * 1e-3 + 1.57883 * 1e-4 * T_)); 
  f1 = f1 * (1 + S_ * (2.39357 * 1e-3 - 3.13530 * 1e-5 * T_ + 2.52477 * 1e-7 * pow(T_,2)));
  f2 = f2 * (1 + S_ * (-1.99723 * 1e-2 + 1.81176 * 1e-4 * T_));
  double sigmaSW = getSigmaSW();

  double a = pow((fGhz/f1),2);
  double b = pow((fGhz/f2),2);

  e[0] = ((es - e1)/(1 + a)) + ((e1 - einf)/(1 + b)) + einf;
  e[1] = (((fGhz/f1)*(es - e1))/(1 + a)) + (((fGhz/f2)*(e1 - einf))/(1 + b)) + (18 * sigmaSW)/fGhz;

  if(debug_)
    std::cout << NOW << " UwElectroMagneticMPropagation: relativePermittivity= " << e[0] << " + " 
    << e[1] << "i" << std::endl;
}

double UwElectroMagneticMPropagation::getSigmaSW()
{
  double sigma35 = 2.903602 + 8.607 * 1e-2 * T_ + 4.738817 * 1e-4 * pow(T_,2) -2.991 * 1e-6 * pow(T_,3) + 4.3047 * 1e-9 * pow(T_,4);
  double R35 = S_ * (37.5109+5.45216 * S_ + 1.4409 * 1e-2 * pow (S_,2)) / (1004.75 + 182.283 * S_ + pow(S_,2));
  double alpha0 = (6.9431 + 3.2841 * S_ -9.9486 * 1e-2 * pow(S_,2)) / (84.850 + 69.024 * S_+ pow(S_,2)) ;
  double alpha1 = 49.843 - 0.2276*S_ + 0.198 * 1e-2 * pow(S_,2);

  double RT15 = 1 + (alpha0 * (T_ - 15)) / (alpha1 + T_);
  return sigma35 * R35 * RT15;
}

double UwElectroMagneticMPropagation::getAlpha(
  double* rel_e,
  double w,
  double sigma
)
{
  double a = MU_0 * rel_e[0] * E_0 / 2;
  double b = sqrt(1 + pow((sigma /(w * rel_e[0] * E_0)),2)) - 1;
  double alpha = w * sqrt(a * b);
  if(debug_)
    std::cout << NOW << " UwElectroMagneticMPropagation: alpha= " << alpha << std::endl;
  return alpha;
}

double UwElectroMagneticMPropagation::getBeta(
  double* rel_e,
  double w,
  double sigma
)
{
  double a = MU_0 * rel_e[0] * E_0 / 2;
  double b = sqrt(1 + pow((sigma /(w * rel_e[0] * E_0)),2)) + 1;
  double beta = w * sqrt(a * b);
  if(debug_)
    std::cout << NOW << " UwElectroMagneticMPropagation: beta= " << beta << std::endl;
  return beta;
}

void UwElectroMagneticMPropagation::getEtaW(
  double* etaW,
  double* rel_e,
  double w,
  double sigma
)
{
  double a = MU_0 / (rel_e[0] * E_0);
  double b = sqrt(1 + pow((sigma/(w * rel_e[0] * E_0)),2));
  double etaw_m = sqrt(a / b);
  double etaw_p = 0.5 * atan(sigma / (w * rel_e[0] * E_0));
  etaW[0] = etaw_m * cos(etaw_p);
  etaW[1] = etaw_m * sin(etaw_p);

  if(debug_)
    std::cout << NOW << " UwElectroMagneticMPropagation: etaW= " << etaW[0] << " + " 
    << etaW[1] << "i" << std::endl;
}

double UwElectroMagneticMPropagation::getModTauSquared(
  double* etaW
)
{
  double num_m = pow((2*ETA_A),2);
  double den_m = pow((etaW[0] + ETA_A),2) + pow(etaW[1],2);
  if(debug_)
    std::cout << NOW << " UwElectroMagneticMPropagation: modTauSquared= " << (num_m/den_m) << std::endl;
  return (num_m/den_m);
}




