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

/**
 * @file uwmultimodal-rl-clmsg.cpp
 * @author Filippo Donegà and Filippo Bragato
 * @version 1.0.0
 */

#include "uwmultimodal-rl-clmsg.h"
#include <iostream>

ClMsgInfoVecPointer::ClMsgInfoVecPointer()
	: ClMessage(ClMsgInfoVecPointer::CLMSG_INFO_VEC_POINTER_VERBOSITY,
			  CLMSG_INFO_VEC_POINTER)
	, info_vec_ptr(nullptr)
	, valid(false)
{
}

ClMsgInfoVecPointer::ClMsgInfoVecPointer(int dest_id)
	: ClMessage(ClMsgInfoVecPointer::CLMSG_INFO_VEC_POINTER_VERBOSITY,
			  CLMSG_INFO_VEC_POINTER, UNICAST, dest_id)
	, info_vec_ptr(nullptr)
	, valid(false)
{
}

ClMsgInfoVecPointer::~ClMsgInfoVecPointer()
{
}

std::vector<Info> *
ClMsgInfoVecPointer::getInfoVecPointer() const
{
	if (valid) {
		return info_vec_ptr;
	} else
		return nullptr;
}

void
ClMsgInfoVecPointer::setInfoVecPointer(std::vector<Info> *v)
{
	info_vec_ptr = v;
	if (info_vec_ptr != nullptr)
		valid = true;
}

ClMsgInfoVecPointer *
ClMsgInfoVecPointer::copy()
{
	return new ClMsgInfoVecPointer(*this);
}

ClMsgQueuePointer::ClMsgQueuePointer()
	: ClMessage(ClMsgQueuePointer::CLMSG_QUEUE_POINTER_VERBOSITY,
			  CLMSG_QUEUE_POINTER)
	, queue_ptr(nullptr)
	, valid(false)
{
}

ClMsgQueuePointer::ClMsgQueuePointer(int dest_id)
	: ClMessage(ClMsgQueuePointer::CLMSG_QUEUE_POINTER_VERBOSITY,
			  CLMSG_QUEUE_POINTER, UNICAST, dest_id)
	, queue_ptr(nullptr)
	, valid(false)
{
}

ClMsgQueuePointer::~ClMsgQueuePointer()
{
}

std::queue<Packet *> *
ClMsgQueuePointer::getQueuePointer() const
{
	if (valid) {
		return queue_ptr;
	} else
		return nullptr;
}

void
ClMsgQueuePointer::setQueuePointer(std::queue<Packet *> *q, int t)
{
	queue_ptr = q;
	type = t;
	if (queue_ptr != nullptr)
		valid = true;
}

ClMsgQueuePointer *
ClMsgQueuePointer::copy()
{
	return new ClMsgQueuePointer(*this);
}

int
ClMsgQueuePointer::getType()
{
	return type;
}