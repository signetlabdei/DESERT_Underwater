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
//    notice, this list of conditions and the following disclaimer.
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
 * @file uw-csma-aloha-rl.cpp
 * \brief Implementation of CsmaAlohaRl derived from CsmaAloha
 */

#include "uw-csma-aloha-rl.h"
#include <iostream>
#include <strings.h>
#include <uwmultimodal-rl-clmsg.h>
#include <vector>

static class CSMARlModuleClass : public TclClass
{
public:
	CSMARlModuleClass()
		: TclClass("Module/UW/CSMA_ALOHA_RL")
	{
	}

	TclObject *
	create(int, const char *const *) override
	{
		return (new CsmaAlohaRl());
	}
} class_module_csma_aloha_rl;

CsmaAlohaRl::CsmaAlohaRl()
	: CsmaAloha()
	, info_vec_ptr(nullptr)
{
}

void
CsmaAlohaRl::sendQueuePointerClmsg(int type)
{
	ClMsgQueuePointer m = ClMsgQueuePointer();
	m.setQueuePointer(&Q, type);
	sendSyncClMsg(&m);
}

CsmaAlohaRl::~CsmaAlohaRl()
{
}


int
CsmaAlohaRl::command(int argc, const char *const *argv)
{
	if (argc == 3) {
		if (strcasecmp(argv[1], "sendQueuePointerClmsg") == 0) {
			sendQueuePointerClmsg(atoi(argv[2]));
			if (debug_)
				std::cout << "Sent queue pointer" << std::endl;
			return TCL_OK;
		}
	}
	return CsmaAloha::command(argc, argv);
}

int
CsmaAlohaRl::crLayCommand(ClMessage *m)
{
	switch (m->type()) {

		default:
			return Module::crLayCommand(m);
	}
}


void
CsmaAlohaRl::recvFromUpperLayers(Packet *p)
{
	if ((has_buffer_queue && (Q.size() < buffer_pkts)) || !has_buffer_queue) {
		CsmaAloha::recvFromUpperLayers(p);
	} else {
		if (info_vec_ptr != nullptr) {
			hdr_cmn *ch = HDR_CMN(p);
			for (auto it = info_vec_ptr->begin(); it != info_vec_ptr->end();
					++it) {
				if (it->packet_id_ == ch->uid() && it->action_ != 2) {
					it->delay_ = -1;
				}
			}
		}
		CsmaAloha::recvFromUpperLayers(p);
	}
}

int
CsmaAlohaRl::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_INFO_VEC_POINTER) {
		std::vector<Info> *v = dynamic_cast<ClMsgInfoVecPointer *>(m)->getInfoVecPointer();
		info_vec_ptr = v;
		return 0;
	}
	return Module::recvSyncClMsg(m);
}