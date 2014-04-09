//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
// This module has only slightly modification as respect to the mll module contained  in Miracle, 
// released under the same BSD copyright and implemented by 
// Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen (Karlstad University)

/**
 * @file   uwmll-module.h
 * @author Saiful Azad
 * @version 1.0.0
 *
 * \brief Provides the declaration of UWMllModule class that represents the MLL class.
 *
 */

#ifndef UWMLL_MODULE_H
#define UWMLL_MODULE_H

#include <module.h>
#include <packet.h>
#include <arp.h>
#include <mac.h>
#include <uwip-module.h>

#include <vector>
#include <algorithm>

#include "marptable.h"

#define UWMLL_DROP_REASON_NOT_IN_ARP_LIST "NAL"

/**
 * Module for ARP-resolve.
 * Should live between one or more IPModule and one MMacModule
 * NOTE: You should only use one MMacModule below this module, else behavior is undef
 */
class UWMllModule : public Module
{
	public:
		/** Constructor */
		UWMllModule();
		/** Desctructor */
		~UWMllModule();

		/**
		 * Receive asyncronous commands
		 * In Practive only IPModule telling us its IP-address
		 * @param m Pointer to message object
		 * @return 1 on success
		 */
		virtual int crLayCommand(ClMessage* m);
		virtual int recvAsyncClMsg(ClMessage* m);
		virtual int recvSyncClMsg(ClMessage* m);

		/** TCL Command handler */
		virtual int command(int argc, const char*const* argv);

		/** Retrieve packets from other modules */
		virtual void recv(Packet* p);

		/**
		 * Retrieve packets from other modules
		 * This method is used to know which mac-module to send ARP-replies to
		 */
		virtual void recv(Packet *p, int idSrc);

	protected:
		/** Handle packet going down */
		virtual void sendDown(Packet* p);

		/** Handle packet going up */
		virtual void sendUp(Packet* p);

		/**
		 * Retrieve MAC address for lower layer
		 * @param downId id of downward module
		 * 		if> -1, only the module with id downId will be asked,
		 * 		else request is broadcasted
		 * @return the MAC address of lower layer
		 */
		virtual int getDownAddr(int downId = -1);

		/**
		 * Resolve MAC address for given dst address
		 * @param dst IP destination address
		 * @param p packet which requested the resolv, will be cached
		 * @return 0 if ARP request successfully sent, otherwise some error number
		 */
		virtual int arpResolve(nsaddr_t dst, Packet* p);
                
                inline void incrArpPktDrop() { n_arp_pkt_drop++; }
                
                inline int getArpPktDropped() { return n_arp_pkt_drop;}

		/** Pointer to an arptable */
		UWARPTable *arptable_;

		/** List of IP address to our upper layers */
		vector<nsaddr_t> netAddr;

		/** Link layer sequence number */
		int seqno_;
               
                int n_arp_pkt_drop;
                
                int enable_addr_copy; /** enable the copy of the IP address as MAC address */
                                     /** valid only if MAC_ADDR = IP on the node*/
};

#endif
