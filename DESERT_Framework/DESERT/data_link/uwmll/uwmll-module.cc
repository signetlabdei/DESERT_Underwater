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
 * @file   uwmll-module.cc
 * @author Saiful Azad
 * @version 1.0.0
 *
 * \brief Provides the implementation of UWModuleClass that represents the implementation of MLL module.
 *
 */

#include "uwmll-module.h"
#include <mmac-clmsg.h>
#include <uwip-clmsg.h>
#include <iostream>


/**
 * Class that represents the binding with the tcl configuration script 
 */ 
static class UWMllModuleClass : public TclClass {
	public:
		/**
		 * Constructor of the class
		 */
		UWMllModuleClass() : TclClass("Module/UW/MLL") {}
		/**
	 	 * Creates the TCL object needed for the tcl language interpretation
	 	 * @return Pointer to an TclObject
	 	 */
		TclObject* create(int, const char*const*) {
			return (new UWMllModule());
		}
} class_uwll;

UWMllModule::UWMllModule() : seqno_(0), n_arp_pkt_drop(0)
{
	arptable_ = new UWARPTable();
        bind("enable_addr_copy_", (int*)& enable_addr_copy);
}

UWMllModule::~UWMllModule()
{
}

int UWMllModule::crLayCommand(ClMessage* m)
{
	return Module::crLayCommand(m);
}


int UWMllModule::recvAsyncClMsg(ClMessage* m)
{
	return Module::crLayCommand(m);
}


int UWMllModule::recvSyncClMsg(ClMessage* m) {
  return Module::recvSyncClMsg(m);
}



int UWMllModule::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "reset") == 0) {
            arptable_->clear();
            //FALL-THROUGH to give parents a chance to reset
        }
        else if (strcasecmp(argv[1], "getArpPacketDrop") == 0)
        {
            tcl.resultf("%d",getArpPktDropped());
            return TCL_OK;
        }
    }
    if (argc == 4) {
        if (strcasecmp(argv[1], "addentry") == 0) {
            nsaddr_t ipaddr = atoi(argv[2]);
            UWARPEntry* e = new UWARPEntry(ipaddr);
            e->macaddr_ = atoi(argv[3]);
            e->up_ = 1;
            arptable_->addEntry(e);
            return TCL_OK;
        }
    }
    return Module::command(argc, argv);
}

void UWMllModule::recv(Packet* p)
{
	recv(p, -1);
}

void UWMllModule::recv(Packet *p, int idSrc)
{
	hdr_cmn *ch = HDR_CMN(p);
        if(ch->direction() == hdr_cmn::UP)
        {
            sendUp(p);
        }
	else
	{
		ch->direction() = hdr_cmn::DOWN;
		sendDown(p);
	}
}

void UWMllModule::sendDown(Packet* p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwip *ih = HDR_UWIP(p);
	nsaddr_t dst = ih->daddr();
	hdr_ll *llh = HDR_LL(p);
	hdr_mac *mh = HDR_MAC(p);

	llh->seqno_ = ++seqno_;
	llh->lltype() = LL_DATA;

	mh->macSA() = getDownAddr();
	mh->hdr_type() = ETHERTYPE_IP;
	int tx = 0;

	switch(ch->addr_type()) {

		case NS_AF_ILINK:
			/* Uhh!? nsaddr_t to int!? */
			mh->macDA() = ch->next_hop();
//                        cout << "ILINK mode " << endl;
			break;

		case NS_AF_INET:
			if (ch->next_hop() == UWIP_BROADCAST) dst = ch->next_hop();
			else if (ch->next_hop() <= 0) dst = ih->daddr();
			else dst = ch->next_hop();
//                        cout << "INET mode " << endl;

		case NS_AF_NONE:
		
			if (UWIP_BROADCAST == (u_int32_t) dst)
			{
				mh->macDA() = MAC_BROADCAST;
				break;
			}

			/* Resolv ARP */
			tx = arpResolve(dst, p);
			break;

		default:
			mh->macDA() = MAC_BROADCAST;
			break;
	}

	if (tx == 0)
	{
                Module::sendDown(p);
	} else 
        {
            if (enable_addr_copy == 0)
            {
                std::cerr << NOW << "Node(" << mh->macSA() << ")::UwMLL_Module -> WARNING: Entry not found for IP address " << dst <<" Packet dropped" << endl ;
                drop(p,1,UWMLL_DROP_REASON_NOT_IN_ARP_LIST);
            }
            else
            {
                mh->macDA() = dst;
                Module::sendDown(p);
            }
        }
}

int UWMllModule::arpResolve(nsaddr_t dst, Packet* p)
{
	hdr_mac *mh = HDR_MAC(p);
	UWARPEntry *llinfo;
	llinfo = arptable_->lookup(dst);

	// Found entry, set dest and return
	if(llinfo && llinfo->up_)
	{
		mh->macDA() = llinfo->macaddr_;
		return 0;
	}
        return EADDRNOTAVAIL;
}

void UWMllModule::sendUp(Packet* p)
{
	Module::sendUp(p);
}

int UWMllModule::getDownAddr(int downId)
{
	MacClMsgGetAddr *c;
	if(downId != -1)
	{
		c = new MacClMsgGetAddr(UNICAST, downId);
		sendSyncClMsgDown(c);
	}
	else
	{
		c = new MacClMsgGetAddr(BROADCAST, CLBROADCASTADDR);
		sendSyncClMsgDown(c);
	}
	int val = c->getAddr();
	delete c;
	return val;
}


