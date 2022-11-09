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

/**
 * @file   msg-display.cpp
 * @author Sara Falleni
 * @version 1.0.0
 * @date October 2021
 *
 * @brief Class that provide the implementation of an object to display messages 
 */
    #include "msg-display.h"

    MsgDisplayer::MsgDisplayer(){}
    
    MsgDisplayer::MsgDisplayer(int n, std::string pn, int pactive){
        nodeNum = n; 
        protocolName = pn; 
        printActive = pactive;
    }

    // Destructor of the class
    MsgDisplayer::~MsgDisplayer(){}

    void MsgDisplayer::initDisplayer(int n, std::string pn, int pactive){
        nodeNum = n; 
        protocolName = pn; 
        printActive = pactive;
        return;
    }

    //prints the status of the node and the function that called it
    void MsgDisplayer::printStatus(std::string st, std::string fxName, double now, int addr){

        if(printActive)
            std::cout << now << " " << protocolName << " (" << addr << ")::" << fxName << "() " << st << std::endl;
        return;

    }