//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file mS2C_EvoLogics.cc
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief Implementation of the MS2C_EvoLogics class.
 */


#include "mS2C_EvoLogics.h"

/**
 * Class to create the Otcl shadow object for an object of the class MS2C_EvoLogics.
 */
static class MS2C_EvoLogics_TclClass : public TclClass
{       
        // Variable to read the path to the device
        std::string pToDevice_;      
  
	public:
		MS2C_EvoLogics_TclClass() : TclClass("Module/UW/MPhy_modem/S2C") {}
		TclObject* create(int args, const char* const* argv) 
		{ 
		        pToDevice_ = (std::string) argv[4];
			return (new MS2C_EvoLogics(pToDevice_)); 
		}
} class_module_s2cevologics;

	
MS2C_EvoLogics::MS2C_EvoLogics(std::string pToDevice_):UWMPhy_modem(pToDevice_), checkTmr(this), mDriver(this){
 
  setConnections(&checkTmr, &mDriver);
  
}
		
MS2C_EvoLogics::~MS2C_EvoLogics(){
}
