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
 * @file   uwslotted-aloha.h
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UwSlottedAloha</i>.
 * This module implements a simple Slotted Aloha:
 * The channel is divided into equal time slots, each equal to one frame
transmission time.
 * All nodes are synchronized to these slots. A node that wants to transmit must
wait for the beginning of the next time slot.
 * Acknowledgment: If the frame is received correctly, the receiver sends an
acknowledgment (ACK). If multiple nodes transmit in the same slot, a collision
may occur and the frames are destroyed.
 * After a collision, nodes wait for a random back-off period before
retransmitting to avoid repeated collisions.
 *
 */

#ifndef UWSLOTTEDALOHA_H
#define UWSLOTTEDALOHA_H

#include <deque>
#include <mmac.h>
#include <timer-handler.h>

class UwSlottedAloha;

/**
 * UwSlotTimer class is used to handle the scheduling period of
 * <i>UwSlottedAloha</i> slots.
 */

class UwSlotTimer : public TimerHandler
{

public:
	/**
	 * Costructor of the class UwSlotTimer
	 * @param Pointer to a UwSlottedAloha object
	 */
	UwSlotTimer(UwSlottedAloha *m)
		: TimerHandler()
	{
		assert(m != NULL);
		module_ = m;
	}

	~UwSlotTimer() = default;

protected:
	/**
	 * Method call when the timer expire
	 * @param Event*  pointer to an object of type Event
	 */
	virtual void expire(Event *e);

	UwSlottedAloha *module_;
};

/**
 * Class that represents a UwSlottedAloha module
 */
class UwSlottedAloha : public MMac
{

	friend class UwSlotTimer;

public:
	/**
	 * Constructor of the UwSlottedAloha class
	 */
	UwSlottedAloha();

	/**
	 * Destructor of the UwSlottedAloha class
	 */
	virtual ~UwSlottedAloha() = default;

protected:
	/**
	 * Transmit a data packet at the beginning of a slot
	 */
	virtual void txData();

	/**
	 * Receive the packet from the upper layer (e.g. IP)
	 * @param Packet* pointer to the packet received
	 *
	 */
	virtual void recvFromUpperLayers(Packet *p) override;

	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p) override;

	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p) override;

	/**
	 * Method called when the Mac Layer start to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Mac2PhyStartTx(Packet *p);

	/**
	 * Method called when the Mac Layer finish to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Phy2MacEndTx(const Packet *p) override;

	/**
	 * Method called when the Packet received is determined to be not for me
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void rxPacketNotForMe(Packet *p);

	/**
	 * Method called to add the MAC header size
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void initPkt(Packet *p);

	/**
	 * Schedule the beginning of each TDMA cycle, each one after \p delay
	 * @param delay to await before starting the TDMA
	 */
	virtual void start();
	/**
	 * Terminate a TDMA cycle, essentially cancel the TDMA timer
	 */
	virtual void stop();

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 successfully or not.
	 */
	virtual int command(int argc, const char *const *argv) override;

	/**
	 * Enumeration class of UwSlottedAloha status.
	 */
	enum class UWSA_STATUS { IDLE, RECEIVING, TRANSMITTING };

	double slot_time_;
	uint max_queue_size_; /**< Maximum dimension of queue. */
	UWSA_STATUS status_; /**< Variable holding the status enum type. */
	UwSlotTimer timer_; /**< Aloha slot timer handler. */
	std::deque<Packet *> buffer_; /**< Buffer of the MAC node. */
};

#endif
