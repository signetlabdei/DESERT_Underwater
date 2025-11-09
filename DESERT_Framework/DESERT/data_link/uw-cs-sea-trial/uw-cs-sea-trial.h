//
// Copyright (c) 2025 Regents of the SIGNET lab, University of Padova.
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
 * @file   uw-cs-sea-trial.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UwCsSeaTrial</i>.
 * This is simple CSMA with random deferring to be used when your modem
 * does not have real carrier-sensing abilities.
 * Once a packet is received from the apper layers, it starts a random timer.
 * If no packets are received from the PHY in such time, it starts transmitting,
 * otherwise it restarts the timer again.
 * It performs burst transmisionsonce the channel is accessed.
 *
 */

#ifndef UWCSTRIAL_H
#define UWCSTRIAL_H

#include <deque>
#include <mmac.h>

class UwCsSeaTrial;

/**
 * UwSensingTimer class is used to handle the scheduling period of
 * <i>UWCSTRIAL</i> slots.
 */

class UwSensingTimer : public TimerHandler
{

public:
	/**
	 * Costructor of the class UwSensingTimer
	 * @param Pointer of a UwCSTRIAL object
	 */
	UwSensingTimer(UwCsSeaTrial *m)
		: TimerHandler()
	{
		assert(m != NULL);
		module_ = m;
	}

	~UwSensingTimer() = default;

protected:
	/**
	 * Method call when the timer expire
	 * @param Event*  pointer to an object of type Event
	 */
	virtual void expire(Event *e);

	UwCsSeaTrial *module_;
};

/**
 * Class that represents a CSTRIAL Node
 */
class UwCsSeaTrial : public MMac
{

	friend class UwSensingTimer;

public:
	/**
	 * Constructor of the CSTRIAL class
	 */
	UwCsSeaTrial();

	/**
	 * Destructor of the CSTRIAL class
	 */
	virtual ~UwCsSeaTrial() = default;

protected:
	/**
	 * Transmit a data packet if in my slot
	 */
	virtual void txData();

	/**
	 * Sensing timer expired
	 */
	virtual void sensingExpired();

	/**
	 * Start sensing the channel
	 */
	virtual void sensing();

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
	 * Enumeration class of CSTRIAL status.
	 */
	enum class UWCS_STATUS { IDLE, SENSING, TRANSMITTING };

	UWCS_STATUS tx_status_; /**< Variable holding the status enum type. */
	double fix_sens_time_; /**< Frame duration. */
	double rv_sens_time_; /**< Random guard time between slots. */
	UwSensingTimer sensing_timer_; /**< Carrier sensing timer handler. */
	std::deque<Packet *> buffer_; /**< Buffer of the MAC node. */
	uint max_queue_size_; /**< Maximum dimension of queue. */
	uint max_packet_per_burst_; /**< Max numer of packet it can transmit in a tx
								   burst. */
	uint packet_sent_curr_burst_; /**< Counter of packet has been sent in the
									 current burst. */
	uint n_rx_while_sensing_; /*< Number of rx packets while channel sensing. */
};

#endif
