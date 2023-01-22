//
// Copyright (c) 2021 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwhmmphysical.h
 * @author Antonio Montanari
 * @version 1.1.0
 *
 * \brief Definition of UwHMMPhysical class.
 *
 */

#ifndef UWHMMPHYSICAL_H
#define UWHMMPHYSICAL_H

#include "uwphysical.h"
#include "mclink.h"

/**
 * \brief UnderwaterHMMPhysical models an hidden Markov Model phy channel
 */
class UnderwaterHMMPhysical : public UnderwaterPhysical
{

public:
	/**
	 * Constructor of UnderwaterHMMPhysical class.
	 */
	UnderwaterHMMPhysical();

	/**
	 * Destructor of UnderwaterHMMPhysical class.
	 */
	virtual ~UnderwaterHMMPhysical()
	{
	}

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not.
	 *
	 */
	virtual int command(int, const char *const *);

	/**
	 * Adds the Markov Chain transition matrix for each source MAC
	 * 
	 * @param mac MAC of the transmitting node
	 * @param link Pointer to associated MCLink
	 */	
	void setMCLink(int mac, MCLink *link);

	/**
	 *
	 * @return the number of packets sent with bad channel
	 */
	int	getPktsTotBad() const
	{
		return pkts_tot_bad;
	}

	/**
	 *
	 * @return the number of packets sent with good channel
	 */
	int getPktsTotGood() const
	{
		return pkts_tot_good;
	}

	/**
	 *
	 * increase the counter of packets sent taking into account
	 * the channel state
	 */
	virtual void incrTotPkts(MCLink::ChState ch_state)
	{
		if (ch_state == MCLink::GOOD) {
			pkts_tot_good++;
		} else if (ch_state == MCLink::BAD) {
			pkts_tot_bad++;
		}
	}

protected:
	
	/**
	 * Handles the end of a packet reception
	 *
	 * @param Packet* p Pointer to the packet received
	 *
	 */
	virtual void endRx(Packet *p) override;
	

	// Variables
	std::map<int, MCLink*> link_map; /**< maps source mac to associated MCLink*/
	double step_duration; /**< sampling period for channel transitions */
	int pkts_tot_good; /**< Total number of packets arrived with good channel*/
	int pkts_tot_bad; /**< Total number of packets arrived with bad channel*/

private:
	// Variables	
};

#endif /* UWHMMPHYSICAL_H  */
