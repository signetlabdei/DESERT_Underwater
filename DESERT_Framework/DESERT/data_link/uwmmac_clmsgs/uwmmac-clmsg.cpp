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
 * @file   uwphy-clmsg.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of ClMsgUwMmac class.
 *
 */

#include "uwmmac-clmsg.h"

ClMsgUwMmac::ClMsgUwMmac(ClMessage_t type)
: ClMessage(CLMSG_UWMMAC_VERBOSITY, type),
  stack_id(CLMSG_UWMMAC_STACK_ID_NOT_VALID),
  req_type(NOT_VALID)
{
}


ClMsgUwMmac::ClMsgUwMmac(int sid, int dest_module_id, ClMessage_t type)
: ClMessage(CLMSG_UWMMAC_VERBOSITY, type, UNICAST, dest_module_id),
  stack_id(sid),
  req_type(NOT_VALID)
{
}

ClMsgUwMmac::ClMsgUwMmac(const ClMsgUwMmac& msg)
: ClMessage(msg),
  stack_id(CLMSG_UWMMAC_STACK_ID_NOT_VALID)
{
  req_type = msg.req_type;
}

ClMsgUwMmac::~ClMsgUwMmac()
{
}

ClMsgUwMmac* ClMsgUwMmac::copy()
{
  return new ClMsgUwMmac(*this);
}

void ClMsgUwMmac::setReqType(ReqType type)
{
  req_type = type;
}

ClMsgUwMmac::ReqType ClMsgUwMmac::getReqType()
{
  return req_type;
}


ClMsgUwMmacEnable::ClMsgUwMmacEnable()
: ClMsgUwMmac(CLMSG_UWMMAC_ENABLE),
  enable_(true)
{
}

ClMsgUwMmacEnable::ClMsgUwMmacEnable(int sid, int dest_module_id)
: ClMsgUwMmac(sid, dest_module_id,CLMSG_UWMMAC_ENABLE),
  enable_(true)
{
}

ClMsgUwMmacEnable::ClMsgUwMmacEnable(const ClMsgUwMmacEnable& msg)
: ClMsgUwMmac(msg)
{
  enable_ = msg.enable_;
}

ClMsgUwMmacEnable::~ClMsgUwMmacEnable()
{
}

double ClMsgUwMmacEnable::getEnable()
{
	return enable_;
}

void ClMsgUwMmacEnable::enable()
{
	enable_ = true;
}


void ClMsgUwMmacEnable::disable()
{
  enable_ = false;
}

