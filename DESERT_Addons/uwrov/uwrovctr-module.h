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
//
/**
 * @file uwrovctr-module.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Provides the definition of the class <i>UWROV</i>.
 *
 * Provides the definition of the class <i>UWROVCTR</i>, based on <i>UwCbr</i>.
 * <i>UWROVCTR</i> can manage no more than 2^16 packets. If a module generates
 * more than 2^16 packets, they will be dropped, according with <i>UwCbr</i>.
 * <i>UWROVCTR</i> sends control packets containing the next waypoint that has
 * to be reach by a ROV. In addition it receives monitoring packets containing
 * the current ROV position and acks of the sent packets. Whether the ack is not
 * received, the control packet is resent, according to the priority. In
 * particular, last waypoint transmitted has the highest priority, whereas the
 * others are forgotten.
 */

#ifndef UWROV_CTR_MODULE_H
#define UWROV_CTR_MODULE_H

#include <node-core.h>
#include <uwcbr-module.h>
#include "uwrov-packet.h"

#define UWROV_DROP_REASON_UNKNOWN_TYPE "UKT" /**< Reason for a drop in a <i>UWROV</i> module. */
#define UWROV_DROP_REASON_OUT_OF_SEQUENCE "OOS" /**< Reason for a drop in a <i>UWROV</i> module. */
#define UWROV_DROP_REASON_DUPLICATED_PACKET "DPK" /**< Reason for a drop in a <i>UWROV</i> module. */
#define HDR_UWROV_MONITORING(p) (hdr_uwROV_monitoring::access(p))
#define HDR_UWROV_CTR(p) (hdr_uwROV_ctr::access(p))

class UwROVCtrModule;

/**
 * UwROVCtrSendTimer class is used to handle the scheduling period of <i>UWROV</i>
 * packets.
 */
class UwROVCtrSendTimer : public UwSendTimer
{
public:
	/**
	 * Conscructor of UwSendTimer class
	 * @param UwROVCtrModule *m pointer to an object of type UwROVCtrModule
	 */
	UwROVCtrSendTimer(UwROVCtrModule *m)
		: UwSendTimer((UwCbrModule *) (m))
	{};
};

/**
 * UwROVCtrModule class is used to manage <i>UWROVCtr</i> packets and to collect
 * statistics about them.
 */
class UwROVCtrModule : public UwCbrModule
{
public:
	/**
	 * Constructor of UwROVCtrModule class.
	 */
	UwROVCtrModule();

	/**
	 * Destructor of UwROVCtrModule class.
	 */
	virtual ~UwROVCtrModule() = default;

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
	 * Initializes a control data packet passed as argument with the default
	 * values.
	 *
	 * @param Packet* Pointer to a packet already allocated to fill with the
	 * right values.
	 */
	virtual void initPkt(Packet *p) override;

	/**
	 * Reset retransmissions
	 */
	void
	reset_retx()
	{
		pkt = nullptr;
		sendTmr_.force_cancel();
	}

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
	 * Creates and transmits a packet.
	 *
	 * @see UwCbrModule::sendPkt()
	 */
	virtual void transmit() override;

	/**
	 * Start the controller.
	 */
	virtual void start() override;

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
	 * @return The size of a <i>hdr_uwROV_monitoring</i> packet header.
	 */
	static inline int
	getROVCTRHeaderSize()
	{
		return sizeof(hdr_uwROV_ctr);
	}

protected:
	int sn; /**Sequence number of the last control packet sent.*/
	int adaptiveRTO; /**< 1 if an adaptive RTO is used, 0 if a constant RTO is used.*/
	double adaptiveRTO_parameter; /**< Parameter for the adaptive RTO.*/
	float x_rov; /**< X of the last ROV position monitored.*/
	float y_rov; /**< Y of the last ROV position monitored.*/
	float z_rov; /**< Z of the last ROV position monitored.*/
	float newX; /**< X of the new position sent to the ROV.*/
	float newY; /**< Y of the new position sent to the ROV.*/
	float newZ; /**< Z of the new position sent to the ROV.*/
	float speed; /**< Moving speed sent to the ROV.*/
	Position* posit; /**< Controller position.*/
	Packet *pkt;
};

#endif // UWROVCtr_MODULE_H
