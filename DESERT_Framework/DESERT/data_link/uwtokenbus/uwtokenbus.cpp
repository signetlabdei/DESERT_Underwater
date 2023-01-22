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
 * @file   uwtokenbus.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UwTokenBus</i>.
 *
 */

#include "uwtokenbus.h"
#include "uwtokenbus_hdr.h"
#include <iostream>
#include <mac.h>
#include <uwcbr-module.h>
#include <tclcl.h>

extern packet_t PT_UWTOKENBUS;

//define a macro to print debug messages
#define DEBUG(level,text) {if (debug >= level) {std::cout << NOW << " UwTokenBus(" << node_id << "): " << text << std::endl;}}

/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwTokenBusModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the UwTokenBusModule class
	 */
	UwTokenBusModuleClass()
		: TclClass("Module/UW/TOKENBUS")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int argc, const char *const * argv)
	{
		return (new UwTokenBus());
	}

} class_module_uwtokenbus;

void UwTokenBus::TimerBusIdle::expire(Event *e)
{
	((UwTokenBus *)module)->expireBusIdle();
}

void UwTokenBus::TimerTokenPass::expire(Event *e)
{
	((UwTokenBus *)module)->expireTokenPass();
}

//initializing static variables
int UwTokenBus::count_nodes = 0;
int UwTokenBus::count_token_pass_exp = 0;
int UwTokenBus::count_bus_idle_exp = -1;	//start from -1 because initRing will do an increment on startup

//default constructor
UwTokenBus::UwTokenBus()
	: MMac(), 
	node_id(count_nodes++),
	n_nodes(0),
	last_token_id_heard(0),
	last_token_id_owned(0),
	max_token_hold_time(0),
	min_token_hold_time(0),
	token_rx_time(0),
	max_queue_size(0),
	rtx_status(IDLE),  
	got_token(false), 
	slot_time(0),
	token_pass_timeout(0),
	bus_idle_timeout(0),
	token_pass_timer(this), 
	bus_idle_timer(this),
	count_token_resend(0),
	count_token_regen(0),
	count_token_invalid(0),
	debug(100),
	drop_old_(0),
	checkPriority(0)
{
	bind("n_nodes_", (int *)&n_nodes);
	bind("slot_time_", (double *)&slot_time);
	bind("queue_size_", (int *)&max_queue_size);
	bind("debug_tb", (int *)&debug);
	bind("debug_", (int *)&debug_);
	bind("max_token_hold_time_", (double *)&max_token_hold_time);
	bind("min_token_hold_time_", (double *)&min_token_hold_time);
	bind("drop_old_", (int *)&drop_old_);
	bind("checkPriority_", (int *)&checkPriority);
	bind("mac2phy_delay_", (double *)&mac2phy_delay_);

	if (slot_time < 0)
	{
		DEBUG(1," UwTokenBus() not valid slot_time_ < 0!! set to 1 by default ")
		slot_time = 1;
	}
	if (n_nodes < 0)
	{
		DEBUG(1," UwTokenBus() not valid token_pass_timeout_ < 0!! set to 1 by default ")
		token_pass_timeout = 1;
	}
	if (max_queue_size < 0)
	{
		DEBUG(1," UwTokenBus() not valid queue_size_ < 0!! set to 1000 by default ")
		max_queue_size = 1000;
	}		
	if (max_token_hold_time < 0)
	{
		DEBUG(1," UwTokenBus() not valid max_token_hold_time_ < 0!! set to 10 by default ")
		max_token_hold_time = 10;
	}
		if (min_token_hold_time < 0)
	{
		DEBUG(1," UwTokenBus() not valid min_token_hold_time_ < 0!! set to 1 by default ")
		max_token_hold_time = 1;
	}
	if (drop_old_ == 1 && checkPriority == 1)
	{
		DEBUG(1," UwTokenBus() drop_old_ and checkPriority cannot be set both to 1!! "
			 << "checkPriority set to 0 by default ")
		checkPriority = 0;
	}
	if (mac2phy_delay_ <= 0)
	{
		DEBUG(1,"mac2phy_delay_ < 0!! set to 1e-9 by default")
		mac2phy_delay_ = 1e-9;
	}

	token_pass_timeout = 2*slot_time+min_token_hold_time;
	bus_idle_timeout = slot_time+min_token_hold_time;
	initRing();
}

UwTokenBus::~UwTokenBus()
{
}

void UwTokenBus::expireTokenPass() {
	DEBUG(4," TOKEN PASS timer expired" )
	got_token = true;
	count_token_pass_exp++;	
	sendToken(normId(last_token_id_owned + 1));
}

void UwTokenBus::expireBusIdle() {
	DEBUG(1," BUS IDLE timer expired" )
	got_token = true;
	token_rx_time = NOW;
	last_token_id_owned = normId(last_token_id_owned + n_nodes);
	count_bus_idle_exp++;
	token_pass_timer.resched(min_token_hold_time);
	txData();
}


void UwTokenBus::initRing() {
	last_token_id_owned = int(-n_nodes + node_id); //just to have the first tokenID round starting at 0 instead of n_nodes
	bus_idle_timer.resched(3*(node_id)*bus_idle_timeout);
}

void UwTokenBus::recvFromUpperLayers(Packet *p)
{
	incrUpperDataRx();
	DEBUG(10, " recv packet from upperLayer: " << p << " queue size: " << buffer.size()+1)
	bool is_cbr = (((hdr_cmn*)HDR_CMN(p))->ptype() == PT_UWCBR);
	initPkt(p);
	if (buffer.size() < (size_t) max_queue_size)
	{
		if (is_cbr && checkPriority) 
		{
			hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
			if (uwcbrh->priority() == 0)
			{
				buffer.push_back(p);
			}
			else
			{
				buffer.push_front(p);
			}
		}
		else buffer.push_back(p);
	}
	else
	{
		incrDiscardedPktsTx();
		DEBUG(1," recvFromUpperLayers() dropping pkt due to buffer full ")
		if (drop_old_)
		{
			Packet *p_old = buffer.front();
			buffer.pop_front();
			Packet::free(p_old);
			buffer.push_back(p);
		}
		else if (is_cbr && checkPriority)
		{
			hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
			if (uwcbrh->priority() == 1)
			{
				Packet *p_old = buffer.back();
				buffer.pop_back();
				Packet::free(p_old);
				buffer.push_front(p);
			}
		}
		else
			{Packet::free(p);}
	}
	if (got_token)
		txData();
}

int UwTokenBus::normId(int id) const
{
	if (id > TOKENIDMAX)
		return ((id % TOKENIDMAX) + (TOKENIDMAX % n_nodes));
	else 
		return id;	
}

int UwTokenBus::nextId(int id) const
{
	if (id + 1 > TOKENIDMAX) //OVERFLOW 
		return (id % n_nodes) + 1;	
	return id + 1;
}

int UwTokenBus::nextIdOwned(int id) const
{
	if (id + n_nodes > TOKENIDMAX) //OVERFLOW
		return node_id; 
	else
		return id + n_nodes;
}

void UwTokenBus::txData()
{
	if (got_token && rtx_status == IDLE)
	{
		if (buffer.size() > 0)
		{
			Packet *p = buffer.front();
			if (NOW - token_rx_time + Mac2PhyTxDuration(p) < max_token_hold_time) //if true I have time to send this packet
			{
				DEBUG(10," sending a data packet from queue: " << p)
				token_pass_timer.force_cancel();
				buffer.pop_front();
				incrDataPktsTx();
				(HDR_TOKENBUS(p)->tokenId()) = last_token_id_owned; 
				Mac2PhyStartTx(p);
			}
			else
			{// max token hold timeout is expired
				DEBUG(3, " UwTokenBus ID " << node_id
						 << " max token hold timeout expired with " << buffer.size()
						 << " packets still in queue")
				token_pass_timer.resched(token_pass_timeout);
				sendToken(normId(last_token_id_owned + 1));
			}
		}
		else { 
			if (NOW - token_rx_time >= min_token_hold_time) {
				token_pass_timer.resched(token_pass_timeout);
				sendToken(normId(last_token_id_owned + 1));
			}
			else
				token_pass_timer.resched(min_token_hold_time - (NOW - token_rx_time));
		}
	}
	else
	{
		if (!got_token) 
			DEBUG(4," Waiting the token to start tx")
		else if(rtx_status == TRANSMITTING)
			DEBUG(4," Wait earlier packet expires to send the current one")
		else if(rtx_status == RECEIVING)
			DEBUG(1," txData() called while in RX status ")
	}
}

void UwTokenBus::Mac2PhyStartTx(Packet *p)
{
	if (rtx_status == TRANSMITTING)
		DEBUG(1," called Mac2PhyStartTx() while already transmitting ")
	if (rtx_status == RECEIVING)
		DEBUG(1," called Mac2PhyStartTx() while receiving ")
	bus_idle_timer.force_cancel(); //will resched in Phy2MacEndTx
	rtx_status = TRANSMITTING;
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_tokenbus *tbh = HDR_TOKENBUS(p);
	last_token_id_heard = tbh->tokenId();
	DEBUG(10, " start tx of pkt " << p << " of type " << ch->ptype() << " size: " << ch->size() << " tx duration: " << Mac2PhyTxDuration(p)
		<< " tokenid: " << last_token_id_heard << " to mac: " << mach->macDA());
	MMac::Mac2PhyStartTx(p);
}

void UwTokenBus::Phy2MacEndTx(const Packet *p)
{
	//DEBUG(4, " ended tx of pkt: " << p)
	rtx_status = IDLE;
	bus_idle_timer.resched(3 * n_nodes * bus_idle_timeout);	
	if (got_token)
		txData();
}

void UwTokenBus::Phy2MacStartRx(const Packet *p)
{
	//DEBUG(4," Phy2MacStartRx() packet: " << p)
	if (rtx_status == TRANSMITTING)
		DEBUG(1," Phy2MacStartRx() while in TX status")
	else if (rtx_status == RECEIVING)
		DEBUG(1," Phy2MacStartRx() while already in RX status")
	else if (rtx_status == IDLE)
		rtx_status = RECEIVING;
}

bool UwTokenBus::validToken(Packet *p) const
{
	hdr_tokenbus *tbh = HDR_TOKENBUS(p);
	int id = tbh->tokenId();
	if (id >= last_token_id_heard
		|| (id <= n_nodes && (last_token_id_heard + n_nodes > TOKENIDMAX))) //UINT16 OVERFLOW
		return true;
	else
		DEBUG(1," received a token with invalid token id: " << id)
	return false;
}

void UwTokenBus::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	int dest_mac = mach->macDA();
	hdr_tokenbus *tbh = HDR_TOKENBUS(p);
	int pkt_token_id = tbh->tokenId();
	int pkt_node_id = (pkt_token_id % n_nodes); //find which node the token is meant to

	if (rtx_status != TRANSMITTING)
	{
		rtx_status = IDLE;

		if (ch->error())
		{
			/* in case of errors I assume the token_id in the header might be corrupted 
			* so I won't consider it for rescheduling the timers
			*/
			DEBUG(1," Phy2MacEndRx() dropping corrupted pkt ")
			incrErrorPktsRx();
			Packet::free(p);
		}
		else
		{
			if (validToken(p))
			{
				token_pass_timer.force_cancel();
				bus_idle_timer.force_cancel();
				last_token_id_heard = pkt_token_id;
				
				if (pkt_node_id == node_id)
				{
					got_token = true;
					last_token_id_owned = pkt_token_id;
					token_rx_time = NOW;
					token_pass_timer.resched(min_token_hold_time);
					DEBUG(3," Received the token ")
					txData();
				}
				else if (pkt_node_id < node_id)
				{ //pkt from previous node in the ring
					bus_idle_timer.resched((3 * (node_id - pkt_node_id + 1) * bus_idle_timeout));
				}
				else //consider ring turnaround
					bus_idle_timer.resched((3 * (n_nodes - pkt_node_id + node_id + 1) * bus_idle_timeout));
			}
			else { 
				DEBUG(1," received a pkt with invalid token id ")
			}

			if (ch->ptype() == PT_UWTOKENBUS)
			{
				Packet::free(p);
				incrCtrlPktsRx();
			}
			else if ((dest_mac != addr) && (dest_mac != BCAST_ADDR))
			{
				Packet::free(p);
				incrXDataPktsRx();
			}
			else
			{
				sendUp(p);
				incrDataPktsRx();
			}
		}			
	}
	else
	{
		DEBUG(1," discard packet rx while tx; pkt token_id : " << pkt_token_id << " macSA: " << mach->macSA() << " macDA: " << mach->macDA()) 
		Packet::free(p); //assume the packet got corrupted in collision with tx packet
	}
}

void UwTokenBus::initPkt(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_tokenbus *tbh = HDR_TOKENBUS(p);
	(ch->size()) += tbh->getSize();
}

void UwTokenBus::sendToken(int token_id)
{
	if(!got_token) {
		DEBUG(1," attempt to send token without owning it; tokenid: " << token_id)
		return;
	}
	//build the token packet
	Packet *p = Packet::alloc();
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_tokenbus *tbh = HDR_TOKENBUS(p);
	(ch->size()) += tbh->getSize();
	ch->ptype() = PT_UWTOKENBUS;
	mach->set(MF_CONTROL, addr, BCAST_ADDR);
	mach->macDA() = BCAST_ADDR;
	tbh->tokenId() = token_id;
	got_token = false;
	DEBUG(3," pass TOKEN to node: " << (token_id % n_nodes) << " with tokenid: " << token_id)
	incrCtrlPktsTx();
	Mac2PhyStartTx(p);
}


int UwTokenBus::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2)
	{
		if (strcasecmp(argv[1], "get_buffer_size") == 0)
		{
			tcl.resultf("%d", buffer.size());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "get_count_token_resend") == 0)
		{
			tcl.resultf("%d", count_token_resend);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "get_count_token_regen") == 0)
		{
			tcl.resultf("%d", count_token_regen);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "get_count_token_invalid") == 0)
		{
			tcl.resultf("%d", count_token_invalid);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "get_bus_idle_exp") == 0)
		{
			tcl.resultf("%d", count_bus_idle_exp);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "get_token_pass_exp") == 0)
		{
			tcl.resultf("%d", count_token_pass_exp);
			return TCL_OK;
		}
	}
	else if (argc == 3)
	{
		if (strcasecmp(argv[1], "set_slot_time") == 0)
		{
			slot_time = atof(argv[2]);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "set_token_pass_timeout") == 0)
		{
			token_pass_timeout = atof(argv[2]);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "set_bus_idle_timeout") == 0)
		{
			bus_idle_timeout = atof(argv[2]);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "setMacAddr") == 0)
		{
			addr = atoi(argv[2]);
			DEBUG(4, " MAC address: " << addr)
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}
