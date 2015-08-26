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
 * @file   uwoptical.cc
 * @author Federico Favaro, Federico Guerra, Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of UwOptical class.
 *
 */

#include "uwoptical-phy.h"
#include <float.h>

const double K = 1.38*1.E-23; //Boltzmann constant
const double q = 1.6*1.E-19; //electronic charge

static class UwOpticalPhyClass : public TclClass 
{
public:
    UwOpticalPhyClass() : TclClass("Module/UW/OPTICAL/PHY") {}
    TclObject* create(int, const char*const*) {
        return (new UwOpticalPhy);
    }
} class_module_optical;

UwOpticalPhy::UwOpticalPhy() :
    lut_file_name_(""),
    lut_token_separator_('\t'),
    use_woss_(false)
{
    if (!MPhy_Bpsk::initialized)
    {
        MPhy_Bpsk::modid = MPhy::registerModulationType(OPTICAL_MODULATION_TYPE);
        MPhy_Bpsk::initialized = true;
    }
    MPhy_Bpsk();
    bind("Id_",&Id);
    bind("Il_",&Il);
    bind("R_",&R);
    bind("S_",&S);
    bind("T_",&T);
    bind("Ar_",&Ar_);
}

int UwOpticalPhy::command(int argc, const char*const* argv)
{
  if (argc == 2)
  {
    if (strcasecmp(argv[1], "useLUT") == 0) 
    {
      initializeLUT();
      return TCL_OK;
    }
    else if (strcasecmp(argv[1], "useWOSS") == 0) 
    {
      use_woss_ = true;
      return TCL_OK;
    }
  }
  else if (argc == 3) 
  {
    if (strcasecmp(argv[1], "setLUTFileName") == 0) 
    {
      string tmp_ = ((char *) argv[2]);
      if (tmp_.size() == 0) 
      {
          fprintf(stderr, "Empty string for the file name");
          return TCL_ERROR;
      }
      lut_file_name_ = tmp_;
      return TCL_OK;
    } 
    else if (strcasecmp(argv[1], "setLUTSeparator") == 0) 
    {
      string tmp_ = ((char *) argv[2]);
      if (tmp_.size() == 0) {
          fprintf(stderr, "Empty char for the file name");
          return TCL_ERROR;
      }
      lut_token_separator_ = tmp_.at(0);
      return TCL_OK;
    }
  }
  return MPhy_Bpsk::command(argc, argv);
}

void UwOpticalPhy::startRx(Packet* p)
{
    hdr_MPhy* ph = HDR_MPHY(p);
    if ( (PktRx == 0) && (txPending == false) )
    {
        double snr_dB = getSNRdB(p);
    	if (snr_dB > MPhy_Bpsk::getAcquisitionThreshold())
    	{
    		if (ph->modulationType == MPhy_Bpsk::modid)
    		{
    			PktRx = p;
    			Phy2MacStartRx(p);
    			return;
    		}
    		else
    		{
    			if (debug_) cout << "UwOpticalPhy::Drop Packet::Wrong modulation" << endl;
    		}
    	} 
    	else 
    	{
    		if (debug_) cout << "UwOpticalPhy::Drop Packet::Below Threshold : snrdb = " << snr_dB << ", threshold = " << MPhy_Bpsk::getAcquisitionThreshold() << endl;
    	}
    }
    else
    {
    	if (debug_) cout << "UwOpticalPhy::Drop Packet::Synced onto another packet PktRx = " << PktRx << ", pending = " << txPending << endl;
    }
}

double UwOpticalPhy::getSNRdB(Packet* p)
{
    hdr_MPhy* ph = HDR_MPHY(p);
    double snr_linear = pow((S*ph->Pr),2)/((2*q*(Id+Il)*ph->srcSpectralMask->getBandwidth() + ((4*K*T*ph->srcSpectralMask->getBandwidth())/R)) + ph->Pn);
    return snr_linear ? 10*log10(snr_linear) : -DBL_MAX;
}

double UwOpticalPhy::getNoisePower(Packet* p)
{
    hdr_MPhy *ph = HDR_MPHY(p);
    Position* dest = ph->dstPosition;
    assert(dest);
    double depth = use_woss_ ? abs(dest->getAltitude()) : abs(dest->getZ());
    double lut_value = lut_map.empty() ? 0 : lookUpLightNoiseE(depth);
    return pow(lut_value * Ar_ * S , 2);//right now returns 0, due to not bias the snr calculation with unexpected values
}
    
void UwOpticalPhy::endRx(Packet* p)
{ 
    hdr_cmn* ch = HDR_CMN(p);
    if (MPhy_Bpsk::PktRx != 0)
    {
    	if (MPhy_Bpsk::PktRx == p)
	    {
            if(interference_)
            {
                double interference_power = interference_->getInterferencePower(p);
                if(interference_power == 0)
                {
                    //no interference
                    ch->error() = 0; 
                }
                else
                {
                    //at least one interferent packet
                    ch->error() = 1;
                    if (debug_) cout << "UwOpticalPhy::endRx interference power = " << interference_power << endl;
                }
            }
            else
            {
                //no interference model set
                ch->error() = 0; 
            }

            sendUp(p);
            PktRx = 0;
    	} 
    	else 
    	{
            dropPacket(p);
    	}
    }
    else 
    {
        dropPacket(p);
    }
}

double UwOpticalPhy::lookUpLightNoiseE(double depth)
{
  //TODO: search noise Energy in the lookup table
  double return_value_ = -1;
  DepthMap::iterator it = lut_map.lower_bound(depth);
  if (it != lut_map.end() && it->first == depth)
  {
    if (debug_) std::cout <<depth<< " "<<it->first << " " << it->second << std::endl; 
    return it->second;  
  }
  if (it == lut_map.end() || it == lut_map.begin())
  {
    if (debug_) std::cout <<depth<< " Nothing returned depth = " << depth << std::endl; 
    
    return NOT_FOUND_VALUE;
  }
  DepthMap::iterator u_it = it;
  it--;
  if (debug_)
    std::cout <<depth<< " "<<it->first << " " << it->second << " " 
              <<u_it->first << " " << u_it->second <<std::endl;
  return linearInterpolator(depth, it->first, it->second, u_it->first, u_it->second);
}

double UwOpticalPhy::linearInterpolator( double x, double x1, double y1, double x2, double y2 )
{ 
  assert (x1 != x2);  
  double m = (y1-y2)/(x1-x2);
  double q = y1 - m * x1;
  return m * x + q;
}

void UwOpticalPhy::initializeLUT()
{
  ifstream input_file_;
  string line_;
  char* tmp_ = new char[lut_file_name_.length() + 1];
  strcpy(tmp_, lut_file_name_.c_str());
  input_file_.open(tmp_);
  if (input_file_.is_open()) {
    // skip first 2 lines
    for (int i = 0; i < 2; ++i)
    {
      std::getline(input_file_, line_);
    }
    while (std::getline(input_file_, line_)) {
      //TODO: retrive the right row and break the loop
      ::std::stringstream line_stream(line_);
      double d;
      double n;
      line_stream >> d;
      line_stream >> n;
      lut_map[d] = n;
    }
  } else {
    cerr << "Impossible to open file " << lut_file_name_ << endl;
  }
}