//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
 * @file uw-csma-aloha-rl.h
 * @author Filippo Donegà
 * @version 1.0.0
 *
 * \brief Custom version of CsmaAloha adding cross-layer RL messages
 *
 */

#ifndef CSMA_ALOHA_RL_H
#define CSMA_ALOHA_RL_H

#include "uw-csma-aloha.h"
#include <uwmultimodal-rl.h>

/**
 * Class that describes a CsmaAlohaRl module
 */
class CsmaAlohaRl : public CsmaAloha
{
public:
	/**
	 * Constructor of the CsmaAlohaRl class
	 */
	CsmaAlohaRl();
	/**
	 * Destructor of the CsmaAlohaRl class
	 */
	virtual ~CsmaAlohaRl();
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * 		<i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * 		successfully or not.
	 *
	 **/
	virtual int command(int argc, const char *const *argv) override;
	/**
	 * Cross-Layer messages interpreter
	 *
	 * @param ClMessage* an instance of ClMessage that represent the message
	 * 		received
	 * @return <i>0</i> if successful.
	 */
	virtual int crLayCommand(ClMessage *m);

protected:
	virtual int recvSyncClMsg(ClMessage* m);
	virtual void recvFromUpperLayers(Packet *p) override;
	void sendQueuePointerClmsg(int type);
	
	std::vector<Info> *info_vec_ptr;
};

#endif /* CSMA_ALOHA_RL_H */
