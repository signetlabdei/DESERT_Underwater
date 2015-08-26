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
 * @file uwmdriver.cc
 * @author Riccardo Masiero, Matteo Petrani
 * \version 2.0.0
 * \brief Implementation of the UWMdriver class.
 */

#include "uwmdriver.h"
#include "uwmphy_modem.h"

UWMdriver::UWMdriver(UWMPhy_modem* pmModem_){
  
  pmModem = pmModem_;
  SetModemID = false;
  pmInterpreter = NULL;
  pmConnector = NULL;
  ID = pmModem -> getID();
  status = _IDLE;
  payload_tx = "";
  dest = -1;
  payload_rx = "";
  src = -1;
  debug_ = pmModem -> getDebug();
  
}
		
UWMdriver::~UWMdriver(){
}

void UWMdriver::setConnections(UWMinterpreter* pmInterpreter_, UWMconnector* pmConnector_){
  
  pmInterpreter = pmInterpreter_;
  pmConnector = pmConnector_;

}


std::string UWMdriver::getLogFile(){
     return pmModem -> getLogFile();
}

int UWMdriver::getLog(){
     return pmModem -> getLog();
}

void UWMdriver::setID(int ID_){
   ID = ID_;  
}

void UWMdriver::resetModemStatus(){
  status = _IDLE;
  
  if (debug_ >= 2)
  {
      cout << "UWMDRIVER::STATUS_IDLE" << endl;
  }
}
		
void UWMdriver::updateTx(int d, std::string ptx){
   dest = d;
   payload_tx = ptx;
}
		
void UWMdriver::updateRx(int s, int d, std::string prx){
   src = s;
   dstPktRx = d;
   payload_rx = prx;
}

