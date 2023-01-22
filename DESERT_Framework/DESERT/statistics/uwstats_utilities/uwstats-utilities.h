//
// Copyright (c) 2019 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwstats-utilities.h
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Utilities to manage stats.
 *
 */

#ifndef UW_STATS_UTILTIES_H
#define UW_STATS_UTILTIES_H

#include "clmessage.h"

extern ClMessage_t CLMSG_TRIGGER_STATS;

namespace StatsEnum {
/** Enum to use in the d_type field of Stats class. */
	enum StatsEnum : int {
		STATS_PHY_LAYER = 0,/**Stats phy layer.*/
		STATS_MAC_LAYER,	/**Stats mac layer.*/
		STATS_NET_LAYER, 	/**Stats net layer.*/
		STATS_TRANSP_LAYER, /**Stats transport layer.*/
		STATS_APP_LAYER 	/**Stats app layer.*/
	};
}

class ClMsgTriggerStats : public ClMessage {
	
public:	
	/**
	 * Constructor for broadcast ClMsg
	 */
	ClMsgTriggerStats();

	/**
	 * Constructor for UNICAST ClMsg
	 */
	ClMsgTriggerStats(int dest_id);


	/**
	 * Destructor
	 */
	virtual ~ClMsgTriggerStats();
}; 

#endif /* UW_STATS_UTILTIES_H */
