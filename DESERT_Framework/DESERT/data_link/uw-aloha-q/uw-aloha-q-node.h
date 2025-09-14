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
 * @file   uw-aloha-q-node.h
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * \brief Provides the declaration of UW_ALOHAQ_NODE class
 *
 */

#ifndef UW_ALOHA_Q_NODE_H
#define UW_ALOHA_Q_NODE_H

#include <mmac.h>
#include <deque>
#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include <string>

#include <mphy.h>

#include <vector>

extern packet_t PT_MMAC_ACK;

#define UW_ALOHAQ_STATUS_MY_SLOT 1 /**< Status slot active>*/
#define UW_ALOHAQ_STATUS_NOT_MY_SLOT 2 /**< Status slot not active >*/

class UwAloha_Q_NODE;

/**
 * UwAlohaQimer class is used to handle the scheduling period of <i>UWALOHAQ NODE</i>
 * slots.
 */

class UwAlohaQTimer : public TimerHandler
{

public:
	/**
	 * Costructor of the class UwAlohaQTimer
	 * @param Pointer of a UwAloha_Q_NODE object
	 */
	UwAlohaQTimer(UwAloha_Q_NODE *m)
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
	UwAloha_Q_NODE *module;
};

/**
 * Class that represents a UwAloha_Q Node
 */
class UwAloha_Q_NODE : public MMac
{

	friend class UwAlohaQTimer;

public:
	/**
	 * Constructor of the UW_Aloha_Q_NODE class
	 */
	UwAloha_Q_NODE();

	/**
	 * Destructor of the UW_Aloha_Q_NODE class
	 */
	virtual ~UwAloha_Q_NODE() = default;


	/**
	* Cross-Layer messages synchronous interpreter.
	* 
	* @param ClMessage* an instance of ClMessage that represent the message received
	* @return <i>0</i> if successful.
	*/
	virtual int recvSyncClMsg(ClMessage* m);

protected:
	enum UWALOHAQ_PKT_TYPE{
		UWALOHAQ_ACK_PKT,
		UWALOHAQ_DATA_PKT
	};
	
	/**Decide if backoff should be applied in the following slot*/
	virtual int decide_if_backoff(int slot); 
	
	/**Acknowledge ACK is received in the right slot*/
	virtual void stateRxAck(Packet *p); 
	
	/**Check channel packet is sent through*/
	virtual int getLayerIdFromTag(const std::string& tag);
	
	/**Search Q-table and find optimal slot for transmission*/
	virtual int  findMySlot();
	
	/**Update Q-table*/
	virtual void updateQ_table(int ack_received); 
	
	virtual void handleTimerExpiration();
	
	/**
	 * Transmit a data packet if in my slot
	 */
	virtual void txData();
	/**
	 * Schedule the beginning of each cycle, each one after \p delay
	 * @param delay to await before starting the protocol
	 */
	virtual void start(double delay);
	/**
	 * Cancel the timer
	 */
	virtual void stop();
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
	virtual void Phy2MacEndRx(Packet *p);
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p);
	/**
	 * Method called when the Mac Layer finish to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Phy2MacEndTx(const Packet *p);
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
	 * @param argv Array of strings which are the command parameters
							 (Note that <i>argv[0]</i> is the name of the
	 object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
	 */
	virtual int command(int argc, const char *const *argv);
	/**
	 * Enumeration class of UWALOHAQ status.
	 */
	enum UWALOHAQ_STATUS { IDLE, TRANSMITTING, RECEIVING, WAIT_ACK };
	/**
	 * Enumeration class of UWALOHAQ ACK status.
	 */
	enum UWALOHAQ_ACK_STATUS { ACK_RECEIVED, ACK_NOT_RECEIVED };
	/**
	 * Enumeration class of UWALOHAQ backoff activation flag
	 */
	enum UWALOHAQ_BACKOFF_STATUS {ADD_BACKOFF, HALT} ;
	/**B1 - backoff after every unsuccessful transmission ; B2 - backoff after best slot change*/
	enum UWALOHAQ_BACKOFF_MODE {B1, B2};
	
	UwAlohaQTimer alohaq_timer; /**<TDMA timer handler*/
	int slot_status; /**<Is it my turn to transmit data?*/
	double slot_duration; /**Slot duration*/
	std::vector <double> Q_table {};
	double start_time; /**<Time to wait before starting the protocol*/
	UWALOHAQ_STATUS transceiver_status; /**<Variable holding transceiver status*/
	UWALOHAQ_ACK_STATUS ack_status; /**<Variable holding ack status*/
	UWALOHAQ_BACKOFF_STATUS backoff_status; /**<Variable holding backoff status*/
	UWALOHAQ_BACKOFF_MODE backoff_mode; /**<Variable holding backoff mode*/
	double guard_time; /**<Guard time between slots*/
	int tot_slots; /**<Number of slots in the frame */
	
	int HDR_size; /**<Size of the HDR if any*/
	int max_packet_per_slot; /**<max numer of packet it can transmit per slot */
	int packet_sent_curr_slot_; /**<counter of packet has been sent in the
								current slot */
	int packet_sent_curr_frame; /**<counter of packet has been sent in the
								current frame */
	int max_queue_size; /**< Maximum dimension of Queue */

	bool enable;
	
	int debug_; /**<Debug variable: 0 for no info,
				1 for displaying debug info*/
	int sea_trial_; /**<Written log variable*/
	
	std::deque<Packet *> buffer; /**<Buffer of the MAC node*/
	int curr_slot; /**<Current slot*/
	int my_curr_slot; /**<Node's current slot in the ongoing frame*/
	int data_phy_id; /**<Data channel identifier*/
	int decide_backoff;  /**<Dercide about triggering backoff*/
	
	std::string phy_data_tag;
};

#endif
