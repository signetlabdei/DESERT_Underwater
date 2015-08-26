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
 * @file uwmphypatch.h
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief Header of a dumb module intended to patch the absence of a PHY layer's module when we want to use a module of the MAC layer. 
 */

#ifndef UWMPHYPATCH_H
#define UWMPHYPATCH_H

#include <mphy.h>

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

/** 
 * Class to patch the absence of a PHY layer's module when we want to use a module of the MAC layer. This module just 
 * receive a packet and forward it to the layer below (or above) sending the required cross-layer messages to the MAC layer. 
 */
class UWMPhypatch : public MPhy
{       
        int debug_; /**< Flag to enable debug mode (i.e., printing of debug messages) if set to 1. */
        
	public: 
                /** 
	         * Class constructor.
	         */
		UWMPhypatch();
                
                /**
	         * Class destructor.
	         */
		~UWMPhypatch();
                
                /** 
	         * Method to handle the reception of packets arriving from the upper layers of the network simulator.
	         * 
	         * @param p pointer to the packet that has been received from the simulator's upper layers.
	         */
		virtual void recv(Packet*);
		
		
	protected:
		
               /**
	        *  Method to send the packet to be transmitted over the communication channel. 
	        *  
	        *  @param p pointer to the packet to be transmitted.
	        */
		virtual void startTx(Packet* p);
		
		/**
	         *  Method to send a cross layer message Phy2MacEndTx(p) to notify the above layers of the simulator about the end
		 *  of a given transmission, see http://telecom.dei.unipd.it/ns/miracle/doxygen/classMPhy.html#c8c2f52d3833fed8ac736aca0ee52c98. 
	         *  
	         *  @param p pointer to the last transmitted packet.
	         */
		virtual void endTx(Packet* p);
		
		/**
	         *  Method in charge to send a cross layer message Phy2MacStartRx(p) to notify the above layers of the simulator
		 *  about the start of a reception, see http://telecom.dei.unipd.it/ns/miracle/doxygen/classMPhy.html#a15cc91b98013e1c631ad85072867ab6. 
	         * 
	         *  @param p pointer to the last received packet.
	         */
		virtual void startRx(Packet* p);
		
		/**
	         *  Method to send a packet to the above layers after the finishing of a reception. 
	         * 
	         *  @param p pointer to the last received packet.
	         */
		virtual void endRx(Packet* p);
		
		/** 
		 * Unused method.
		 */
		virtual double getTxDuration(Packet*);
		
		/** 
		 * Unused method.
		 */
		virtual int getModulationType(Packet*);
                              
};

#endif /* UWMPHYPATCH_H */
