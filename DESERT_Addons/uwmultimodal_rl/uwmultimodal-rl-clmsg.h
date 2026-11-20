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
 * @file uwmultimodal-rl-clmsg.h
 * @author Filippo Donegà and Filippo Bragato
 * @version 1.0.0
 */

#ifndef UWMULTIMODAL_RL_CLMSG_H
#define UWMULTIMODAL_RL_CLMSG_H

#include <timer-handler.h>
#include <clmessage.h>
#include <packet.h>
#include <uwmultimodal-rl.h>

extern ClMessage_t CLMSG_INFO_VEC_POINTER;
extern ClMessage_t CLMSG_QUEUE_POINTER;

/**
 * Class representing the cross-layer message for sending the
 * pointer to info_vec
 */
class ClMsgInfoVecPointer : public ClMessage
{

public:
	/**
	 * Constructor for a BROADCAST ClMsg
	 */
	ClMsgInfoVecPointer();
	/**
	 * Constructor for a UNICAST ClMsg
	 * @param dest_id Id of the destination module
	 */
	ClMsgInfoVecPointer(int dest_id);
	/**
	 * Destructor of the ClMsgInfoVecPointer class
	 */
	virtual ~ClMsgInfoVecPointer();
	/**
	 * Return the pointer to info_vec
	 * @return Pointer to info_vec
	 */
	std::vector<Info> *getInfoVecPointer() const;
	/**
	 * Set the info_vec pointer to be sent
	 * @param v Pointer to info_vec
	 */
	void setInfoVecPointer(std::vector<Info> *v);
	/**
	 * Copy method of the ClMsgInfoVecPointer class, the specialization of the
	 * return value is intentional and it is allowed by the C++ standard
	 *
	 * @return Pointer to a copy of the current info_vec object
	 */
	virtual ClMsgInfoVecPointer *copy();
	static const uint CLMSG_INFO_VEC_POINTER_VERBOSITY =
			3; /**< Verbosity level */

protected:
	std::vector<Info> *info_vec_ptr; /**< Pointer to info_vec */
	bool valid; /**< False by default, true after setInfoVecPointer */
};

/**
 * Class representing the cross-layer message for sending
 * the pointer to the MAC queue (used by the MAC layer)
 */
class ClMsgQueuePointer : public ClMessage
{

public:
	/**
	 * Constructor for a BROADCAST ClMsg
	 */
	ClMsgQueuePointer();
	/**
	 * Constructor for a UNICAST ClMsg
	 * @param dest_id Id of the destination module
	 */
	ClMsgQueuePointer(int dest_id);
	/**
	 * Destructor of the ClMsgQueuePointer class
	 */
	virtual ~ClMsgQueuePointer();
	/**
	 * Return a pointer to the MAC queue
	 * @return Pointer to the MAC queue
	 */
	std::queue<Packet *> *getQueuePointer() const;
	/**
	 * Get the type of the queue
	 * @return Type of the queue [0 : acoustic | 1 : optical]
	 */
	int getType();
	/**
	 * Set the queue pointer to be sent
	 * @param q Pointer to the queue
	 * @param type Type of the queue [0 : acoustic | 1: optical]
	 */
	void setQueuePointer(std::queue<Packet *> *q, int type);
	/**
	 * Copy method of the ClMsgQueuePointer class, the specialization of the
	 * return value is intentional and it is allowed by the C++ standard
	 *
	 * @return Pointer to a copy of the current queue object
	 */
	virtual ClMsgQueuePointer *copy();
	static const uint CLMSG_QUEUE_POINTER_VERBOSITY = 3; /**< Verbosity level */

protected:
	std::queue<Packet *> *queue_ptr; /**< Pointer to the queue */
	int type; /**< Type of the queue [0 : acoustic | 1 : optical] */
	bool valid; /**< False by default, true after setQueuePointer */
};

#endif // UWMULTI_TRAFFIC_RL_CLMSG_H