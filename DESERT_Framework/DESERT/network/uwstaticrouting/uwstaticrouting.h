//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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

/**
 * @file   uwstaticrouting.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Provides basic routing functionalities.
 *
 * Provides basic routing functionalities.
 */

#ifndef _STATIC_ROUTING_H_
#define _STATIC_ROUTING_H_

#define DROP_DEST_NO_ROUTE "DNR"      /**< Reason for a drop in a <i>UWVBR</i> module. */

#include <uwip-module.h>
#include <map>

namespace {
    static const uint16_t IP_ROUTING_MAX_ROUTES = 100; /**< Maximum number of entries in the routing table of a node. */
}

/**
 * UwStaticRoutingModule class implements basic routing functionalities.
 */
class UwStaticRoutingModule : public UWIPModule {
public:
    
    /**
     * Constructor of UwStaticRoutingModule class.
     */
    UwStaticRoutingModule();
    
    /**
     * Destructor of UwStaticRoutingModule class.
     */
    virtual ~UwStaticRoutingModule();
    
    /**
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet will be received.
     */
    virtual void recv(Packet*);
    
    /**
     * Cross-Layer messages synchronous interpreter.
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received
     * @return <i>0</i> if successful.
     */
    virtual int recvSyncClMsg(ClMessage*);
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int, const char*const*);
    
    /**
     * Returns the next hop address of a packet passed as input.
     * 
     * @param Packet* Packet to process.
     * @return IP of the next hop.
     */
    virtual uint8_t getNextHop(const Packet*) const;
    
    /**
     * Returns the next hop address of an address passed as input.
     * 
     * @param nsaddr_t Address to process.
     * @return IP of the next hop.
     */
    virtual uint8_t getNextHop(const uint8_t&) const;
    
    /**
     * Removes all the routing information.
     */
    virtual void clearRoutes();
    
    /**
     * Adds a new entry in the routing table.
     * 
     * @param nsaddr_t Address of the destination.
     * @param nsaddr_t Address of the next hop.
     */
    virtual void addRoute(const uint8_t&, const uint8_t&);

private:
    
    std::map<uint8_t, uint8_t> routing_table;          /**< Routing table: destination - next hop. */
    uint8_t default_gateway;                           /**< Default gateway. */
};

#endif // _STATIC_ROUTING_H_
