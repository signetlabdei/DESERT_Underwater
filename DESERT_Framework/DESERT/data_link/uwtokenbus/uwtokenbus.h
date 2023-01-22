//
// Copyright (c) 2021 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwtokenbus.h
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UwTokenBus</i>.
 *
 */

#ifndef UWTOKENBUS_H
#define UWTOKENBUS_H

#include "uwtokenbus_hdr.h"
#include <mmac.h>
#include <deque>

extern packet_t PT_UWTOKENBUS;

/**
 * Class that represents a TokenBus Node
 */
class UwTokenBus : public MMac
{

public:
	/**
	 * Default constructor of the TokenBus class
	 */
	UwTokenBus();

	/**
	 * Destructor of the TokenBus class
	 */
	virtual ~UwTokenBus();

	/**
 * TimerBusIdle when expires calls UwTokenBus::TxData() to start the transmission
 */
	class TimerBusIdle : public TimerHandler
	{

	public:
		/**
	 * Costructor of the class TimerBusIdle
	 * @param Pointer of a UwTokenBus object
	 */
		TimerBusIdle(UwTokenBus *m)
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
		UwTokenBus *module;
	};

	/**
 * TimerTokenPass when expires it resends the token
 */

	class TimerTokenPass : public TimerHandler
	{

	public:
		/**
	 * Costructor of the class TimerTokenPass
	 * @param Pointer of a UwTokenBus object
	 */
		TimerTokenPass(UwTokenBus *m)
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
		UwTokenBus *module;
	};

	enum UWTokenBus_STATUS
	{
		IDLE,
		TRANSMITTING,
		RECEIVING
	}; /**<rtx state */

protected:

	/**
	 * called when bus_idle_timer expires
	 */
	virtual void expireBusIdle();

	/**
	 * called when token_pass_timer expires
	 */
	virtual void expireTokenPass();

	/**
	 * Initialize the network and generates the first token
	 */
	virtual void initRing();

	/**
	 * Assert if the received token id is valid, i.e it follows the monotonic progression
	 * taking in account uint16 overflow.
	 * 
	 * @param p Packet with token
	 */
	virtual bool validToken(Packet *p) const;

	/**
	 * Passes the token to the next node
	 * @param next_id node receiving the token
	 */
	virtual void sendToken(int next_id);

	/**
	 * Starts transmitting the packets from the queue
	 */
	virtual void txData();

	/**
	 * @return the token id modulo TOKENIDMAX
	 */
	virtual int normId(int id) const;

	/**
	 * @return token Id of next node
	 */
	virtual int nextId(int id) const;

	/**
	 * @return next token id owned by this node
	 */
	virtual int nextIdOwned(int id) const;

	/**
	 * Receive the packet from the upper layer (e.g. IP)
	 * @param p pointer to the packet received
	 *
	 */
	virtual void recvFromUpperLayers(Packet *p) override;
	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param p pointer to a Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p) override;
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param p pointer to a packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p) override;
	/**
	 * Method called when the Mac Layer start to transmit a Packet
	 * @param p pointer to a packet object that rapresent the
	 * packet in transmission
	 */
	virtual void Mac2PhyStartTx(Packet *p);
	/**
	 * Method called when the Mac Layer finish to transmit a Packet
	 * @param p pointer to a Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Phy2MacEndTx(const Packet *p) override;

	/**
	 * Method called to add the MAC header size
	 * @param p pointer to a Packet object that rapresent the
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
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
															  successfully or
	 not.
	 */
	virtual int command(int argc, const char *const *argv) override;

	int node_id;				  /**<id of the node (0 to n_nodes-1)*/
	int n_nodes;				  /**< number of nodes in the ring */
	int last_token_id_heard = 0; /**<last token id heard on the bus */
	int last_token_id_owned = 0; /**<last token id owned */

	double max_token_hold_time;	 /**<max token holding time */
	double min_token_hold_time;	 /**< if the node has en empty queue when it receive the token, it waits this time before passing the token */
	double token_rx_time;		 /**< time of token reception */
	int max_queue_size;			 /**<max packets in the queue */
	std::deque<Packet *> buffer; /**<outgoing packets dequeue */
	UWTokenBus_STATUS rtx_status;
	bool got_token;					 /**<set if node is currently holding the token */
	double slot_time;				 /**<max travel time between any pair of nodes, used as time unit for some of the timers timeouts */
	double token_pass_timeout;		 /**<timeout for the namesake timer for token retransmission attempt, should be 2*slot_time+min_token_hold_time */
	double bus_idle_timeout;		 /**<base timeout for the namesake timer should be (slot_time+max_token_hold_time) */
	TimerTokenPass token_pass_timer; /**token_pass_timer is scheduled when a node pass the token, 
										* it's cancelled when activity from the following node is heard
										* and when it expires it resends the token.*/
	TimerBusIdle bus_idle_timer;	 /** bus_idle_timer is rescheduled everytime a new token_id is heard on the bus:
 									the first time node n hears a token_id meant to node k, it sets the timeout to
									(3*(n-k+1)*bus_idle_timeout) in order to allow all the previous nodes to regenerate the token first
									When it expires, it regenerates the token and starts transmitting. */

	int count_token_resend;	 /**< node count of token retransmissions */
	int count_token_regen ;	 /**< node count of token regeneration */
	int count_token_invalid; /**< node count of invalid received token */

	int debug;		   /**<Debug variable: 0 for no info*/
	int drop_old_;	   /**<flag to set the drop packet policy in case of buffer overflow: 
					if 0 (default) drops the new packet, if 1 the oldest*/
	int checkPriority; /**<flag to set to 1 if UWCBR module uses packets with priority,
						set to 0 otherwise. Priority can be used only with UWCBR module */

	static int count_nodes;	/**< counter of the instantiated nodes, used for assigning node ids in default contructor*/
	static int count_token_pass_exp; /**< count token pass timer expirations */
	static int count_bus_idle_exp; /**< count bus idle timer expirations */
	constexpr int NMOD(int n) {return ((n_nodes + (n % n_nodes)) % n_nodes);} /**< given any int returns the corresponding node id via modulo operations*/
};


#endif
