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
 * @file mS2C_EvoLogics.h
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief Header of the main class that implements the general interface between ns2/NS-Miracle and the S2C EvoLogics modem. 
 */

#ifndef MS2CEVOLOGICS_H
#define MS2CEVOLOGICS_H

#include <uwmphy_modem.h>
#include "mdriverS2C_EvoLogics.h"

using namespace std;

/**
 *  Class that implements the interface between ns2/NS-Miracle and the S2C EvoLogics modem (it derives UWMPhy_modem).
 *  NOTE: For tcl-user, set node($id) [new "UW/MPhy_modem/S2C" "path to the device"]
 *  Example: in case of a modem to be connected via TCP/IP socket "path to the device"
 *           would be the application port opened via ssh, e.g., 9200
 */
class MS2C_EvoLogics : public UWMPhy_modem
{
     CheckTimer checkTmr; /**< Object to schedule the "check-modem" events. */
     MdriverS2C_EvoLogics mDriver; /**< Object to drive the modem operations. */ 
     
     public:
	
        /** 
	  * Class constructor.
	  * 
	  * @param pToDevice_ the path to the device that must be connected with NS-Miracle (e.g., /dev/ttyUSB0 for a serial connection)
	  */
	MS2C_EvoLogics(std::string);
		
	/**
	  * Class destructor.
	  */
	~MS2C_EvoLogics();
        
};
#endif	/* MS2CEVOLOGICS_H */