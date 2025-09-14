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
 * @file   uw-aloha-q-sync-sink.h
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UWALOHAQ-SYNC-SINK</i>.
 *
 */

#ifndef UW_ALOHA_Q_SYNC_SINK_H
#define UW_ALOHA_Q_SYNC_SINK_H

#include <mmac.h>
#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include <vector>

#include <mphy.h>

#include <uw-aloha-q-sync-hdrs.h>

class UwAloha_Q_Sync_SINK;

/**
 * UwAlohaQSyncTimerSink class is used to handle the scheduling period of <i>UwAloha_Q_Sync_SINK</i>
 * slots.
 */

class UwAlohaQSyncTimerSink : public TimerHandler
{

public:
	/**
	 * Costructor of the class UwAlohaQSyncTimerSink
	 * @param Pointer of a UwAloha_Q_Sync_SINK
	 */
	UwAlohaQSyncTimerSink(UwAloha_Q_Sync_SINK *m)
		: TimerHandler()
	{
		assert(m != NULL);
		module = m;
	}

protected:
	/**
	 * Method call when the timer expire
	 * @param Event*  pointer to an object of type Event
	 */
	virtual void expire(Event *e);
	UwAloha_Q_Sync_SINK *module;
};

/**
 * Class that represents an UWALOHAQ Sink
 */
class UwAloha_Q_Sync_SINK : public MMac
{

	friend class UwAlohaQSyncTimerSink;

public:
	/**
	 * Constructor of the UwAloha_Q_Sync_SINK class
	 */
	UwAloha_Q_Sync_SINK();

	/**
	 * Destructor of the UwAloha_Q_Sync_SINK class
	 */
	virtual ~UwAloha_Q_Sync_SINK() = default;
	
	virtual int recvSyncClMsg(ClMessage* m);

protected:
	
	/**
	* Triggered upon timer expiration event
	**/
	virtual void handleTimerExpiration();
	
	/**
	 * Receive the packet from the upper layer (e.g. IP)
	 * @param Packet* pointer to the packet received
	 *
	 */
	 /**
	 * Schedule the beginning of each SINK frame cycle
	 * @param delay to await before starting the protocol
	 */
	virtual void start(double delay);
	/**
	 * Cancel the timer
	 */
	virtual void stop();
	 
	virtual void recvFromUpperLayers(Packet *p);
	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p);
	
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(Packet *p);
	/**
	 * Method called when the Mac Layer start to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */	
	//virtual void Mac2PhyStartTx(Packet *p);
	/**
	 * Method called when the Mac Layer finish to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Phy2MacEndTx(const Packet *p);
	
	/**
	 * Method called to transmit ACK packet
	 */
	virtual void txAck();
	
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
	 * @param argv Array of strings which are the command parameters
	 (Note that <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 successfully or
	 not.
	 */
	virtual int command(int argc, const char *const *argv);

	/**
	 * Enumeration class of UwAlohaQSink status
	 */
	enum UWALOHAQ_SINK_STATUS { IDLE, TRANSMITTING, RECEIVING };
	/**
	 * Enumeration class of UwAlohaQSink slot status
	 */
	enum UWALOHAQ_SINK_SLOT_STATUS { RECV_PERIOD, TRANSMIT_PERIOD};

	UWALOHAQ_SINK_STATUS sink_status; /**<Variable that holds 
								UWALOHAQ_SINK_STATUS*/
	
	UWALOHAQ_SINK_SLOT_STATUS sink_slot_status; /**<Variable that holds 
								UWALOHAQ_SINK_SLOT_STATUS*/
	
	double start_time; /**<Time to wait before starting the protocol*/
	
	UwAlohaQSyncTimerSink alohaq_sync_sink_timer; /**<UwAlohaQ Sink Timer*/
	
	int debug_; /**<Debug variable: 0 for no info,
				> 1 for detailed info*/
				
	int sea_trial_; /**<Written log variable*/
	int ACK_size; /**<Size of the ACK HDR if any*/
	int HDR_size; /**<Size of the HDR if any*/
	
	bool enable;
	double t_prop_max; /**<Maximal propagation delay*/
	double t_dp; /**<Transmission delay*/
	double t_guard; /**<Guard interval*/
	int nn; /**<Number of nodes*/
	std::vector<int> succ_macs; /**<Store MAC adresses of successful transmittors*/
	double slot_duration_factor; /**<slot_duration = t_dp * slot_duration_factor 
			(applicable for nodes)*/
	
};

#endif
