//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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
 * @file uwrov-module.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Provides the definition of the class <i>UWROV</i>.
 *
 * Provides the definition of the class <i>UWROV</i>, based on <i>UwCbr</i>.
 * <i>UWROV</i> can manage no more than 2^16 packets. If a module generates more
 * than 2^16 packets, they will be dropped, according with <i>UwCbr</i>.
 * <i>UWROV</i> sends periodically monitoring packets containing information
 * about the current position and acknowledges the last control packet received.
 * Each control packet contains the next waypoint that has to be reach.
 */

#ifndef UWROV_MODULE_H
#define UWROV_MODULE_H

#include "uwsmposition.h"
#include "uwcbr-module.h"
#include "uwrov-packet.h"

#include <fstream>
#include <queue>

#define UWROV_DROP_REASON_UNKNOWN_TYPE "UKT" /**< Reason for a drop in a <i>UWROV</i> module. */
#define UWROV_DROP_REASON_OUT_OF_SEQUENCE "OOS" /**< Reason for a drop in a <i>UWROV</i> module. */
#define UWROV_DROP_REASON_DUPLICATED_PACKET "DPK" /**< Reason for a drop in a <i>UWROV</i> module. */
#define HDR_UWROV_MONITORING(p) (hdr_uwROV_monitoring::access(p))
#define HDR_UWROV_CTR(p) (hdr_uwROV_ctr::access(p))

class UwROVModule;

/**
 * UwROVSendACKTimer class is used to handle the scheduling period of
 * <i>UWROV</i> packets.
 */
class UwROVSendAckTimer : public TimerHandler
{
public:
	UwROVSendAckTimer(UwROVModule *m)
		: TimerHandler()
	{
		module = m;
	}

protected:
	virtual void expire(Event *e);
	UwROVModule *module;
};

/**
 * UwROVModule class is used to manage <i>UWROV</i> packets and to collect
 * statistics about them.
 */
class UwROVModule : public UwCbrModule
{
	friend class UwROVSendAckTimer;

public:
	/**
	 * Default Constructor of UwROVModule class.
	 */
	UwROVModule();

	/**
	 * Constructor with position setting of UwROVModule class.
	 *
	 * @param UWSMPosition* p Pointer to the ROV position
	 */
	UwROVModule(UWSMPosition *p);

	/**
	 * Destructor of UwROVModule class.
	 */
	virtual ~UwROVModule() = default;

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not.
	 *
	 **/
	virtual int command(int argc, const char *const *argv) override;

	/**
	 * Initializes a monitoring data packet passed as argument with the default
	 * values.
	 *
	 * @param Packet* Pointer to a packet already allocated to fill with the
	 * right values.
	 */
	virtual void initPkt(Packet *p) override;

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 */
	virtual void recv(Packet *) override;

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 * @param Handler* Handler.
	 */
	virtual void recv(Packet *p, Handler *h) override;

	/**
	 * Sets the position of the ROV.
	 *
	 * @param UWSMPosition * Pointer to the ROV position
	 */
	virtual void setPosition(UWSMPosition *p);

	/**
	 * Returns the position of the ROV.
	 *
	 * @return the current ROV position
	 */
	UWSMPosition *
	getPosition() const
	{
		return posit;
	}

	/**
	 * Returns the size in byte of a <i>hdr_uwROV_monitoring</i> packet header.
	 *
	 * @return The size of a <i>hdr_uwROV_monitoring</i> packet header.
	 */
	static inline int
	getROVMonHeaderSize()
	{
		return sizeof(hdr_uwROV_monitoring);
	}

	/**
	 * Returns the size in byte of a <i>hdr_uwROV_ctr</i> packet header.
	 *
	 * @return The size of a <i>hdr_uwROV_ctr</i> packet header.
	 */
	static inline int
	getROVCTRHeaderSize()
	{
		return sizeof(hdr_uwROV_ctr);
	}

	/**
	 * Sends ACK if ackTimeout expire;
	 */
	virtual void sendAck();

protected:
	enum UWROV_ACK_POLICY { ACK_PIGGYBACK, ACK_IMMEDIATELY, ACK_PGBK_OR_TO };

	int last_sn_confirmed; /**< Sequence number of the last command packet received.*/
	int ack; /**< If not zero, contains the ACK to the last command packet received.*/
	int ackPriority; /**< Flag to give higher priority to ACK or not.*/
	int ackNotPgbk; /** < Number of ACK not sent in piggyback when ackPolicy = 2. */
	int drop_old_waypoints; /** < Flag set to 1 to drop waypoints with sequence number lower or equal than last_sn_confirmed.*/
	int ackTimeout; /**< Timeout after which ACK is sent if ackPolicy = ACK_PGBK_OR_TO. */
	UWSMPosition *posit; /**< ROV position.*/
	UwROVSendAckTimer ackTimer_; /**< Timer to schedule ACK transmission.*/
	std::queue<Packet *> buffer; /**< Packets buffer.*/
	/** Flag to set the policy for ACK transimission,
	 * ACK_PIGGYBACK: ACK is always sent in piggyback,
	 * ACK_IMMEDIATELY: ACK is always sent immediately with a dedicated packet after the reception of CTR packet,
	 * ACK_PGBK_OR_TO:  ACK is sent in piggyback if a ROV packet is generated before an ackTimeout otherwise ACK is sent with a dedicated packet after the acKTimeout. */
	UWROV_ACK_POLICY ackPolicy;

	int log_flag; /**< Flag to enable log file writing.*/
	std::ofstream out_file_stats; /**< Output stream for the textual file of debug */
};

#endif // UWROV_MODULE_H
