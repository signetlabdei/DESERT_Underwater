//
// Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwtap.h
 * @author Antonio Montanari
 * @version 1.0.0
 *
 *
 */

#ifndef UWTAP_H
#define UWTAP_H

#include <module.h>

class UwTap : public Module
{
public:
	/**
	 * Constructor of the overhearing class
	 */
	UwTap() = default;

	/**
	 * Virtual destructor of the overhearing class
	 */
	virtual ~UwTap() = default;
	/**
	 * Retrieve packets from other modules
	 * @param m ClMessage
	 */
	virtual int recvSyncClMsg(ClMessage *m) override;

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param p* Pointer to the packet will be received.
	 */
	virtual void recv(Packet *p) override;

	/**
	 * Asks the PHY the time needed to transmit a packet
	 *
	 * @param p* Pointer to the packet to be transmitted.
	 */
	virtual double Tap2PhyTxDuration(Packet *p);

protected:
};

#endif
