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
//

/**
 * @file   uw-aloha-q-sync/initlib.cpp
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * @brief Provides the initializazion of the uw-aloha-q-sync libraries
 */

#include <tclcl.h>
#include <uw-aloha-q-sync-hdrs.h>

extern EmbeddedTcl UwAlohaQSyncTclCode;

int aloha_q_sync_ACK::offset_ = 0;

packet_t PT_ALOHAQ_SYNC_ACK;

static class Alohaqsync_ACK_HeaderClass : public PacketHeaderClass
{
public:
	Alohaqsync_ACK_HeaderClass()
		: PacketHeaderClass("PacketHeader/ALOHAQ_SYNC_ACK", sizeof(aloha_q_sync_ACK))
	{
		this -> bind();
		bind_offset(&aloha_q_sync_ACK::offset_);
	}
}	class_Alohaqsync_ACK_HeaderClass;

extern "C" int
Uwalohaqsync_Init()
{

	PT_ALOHAQ_SYNC_ACK = p_info::addPacket("UWALOHAQSYNC/ACK");	
	
	UwAlohaQSyncTclCode.load();
	return 0;
}

extern "C" int 
Cyguwalohaqsync_Init() 
{
     return Uwalohaqsync_Init();
}
