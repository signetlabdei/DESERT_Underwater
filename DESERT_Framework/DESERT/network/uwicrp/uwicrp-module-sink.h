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

/**
 * @file   uwicrp-module-sink.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Provides a module for sinks that needs a simple and dynamic routing protocol.
 *
 */

#ifndef UWICRP_MODULE_SINK_H
#define	UWICRP_MODULE_SINK_H

#include "uwicrp-hdr-ack.h"
#include "uwicrp-hdr-data.h"
#include "uwicrp-hdr-status.h"
#include "uwicrp-common.h"
#include <uwip-module.h>
#include <uwip-clmsg.h>
#include "packet.h"

#include <sstream>
#include <string>
#include <iostream>
#include <rng.h>
#include <ctime>
#include <module.h>
#include <tclcl.h>
#include <vector>

using namespace std;

/**
 * UwIcrpSink class is used to represent the routing layer of a node.
 */
class UwIcrpSink : public Module {
   
public:
    
    /**
     * Constructor of UwIcrpSink class.
     */
    UwIcrpSink();
    
    /**
     * Destructor of UwIcrpSink class.
     */
    ~UwIcrpSink();
    
protected:
    
    /**
     * Cross-Layer messages synchronous interpreter.
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received
     * @return <i>0</i> if successful.
     */
    virtual int recvSyncClMsg(ClMessage*);
    
    /**
     * Initializes a UwIcrpSink node.
     * It sends to the lower layers a Sync message asking for the IP of the node.
     * 
     * @see UWIPClMsgReqAddr(int src)
     * @see sendSyncClMsgDown(ClMessage* m)
     */
    virtual int recvAsyncClMsg(ClMessage*);
    
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
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet will be received.
     */
    virtual void recv(Packet*);

    /**
     * Initializes a Status Packet previously allocated.
     * 
     * @param Packet* Pointer to a PT_UWICRP_STATUS Packet to use to retrieve the information to initialize the new packet.
     * @param Packet* Pointer to a PT_UWICRP_STATUS Packet to initialize.
     */
    virtual void initStatusPkt(Packet*, Packet*);
    
    /**
     * Initializes a UwIcrpSink node.
     * It sends to the lower layers a Sync message asking for the IP of the node.
     * 
     * @see UWIPClMsgReqAddr(int src)
     * @see sendSyncClMsgDown(ClMessage* m)
     */
    virtual void initialize();
    
    /**
     * Creates an ack packet and sends it to the previous hop using the information
     * contained in the header of the data packet passed as input parameter. It is
     * an ack to the previous hop, and not to the source of the packet.
     * 
     * @param Packet* Pointer to a Data packet to acknowledge.
     */
    virtual void sendBackAck(const Packet* p);
    
    /**
     * Return a string with an IP in the classic form "x.x.x.x" converting an ns2 nsaddr_t address.
     * 
     * @param nsaddr_t& ns2 address
     * @return String that contains a printable IP in the classic form "x.x.x.x"
     */
    virtual string printIP(const uint8_t);
    
    /**
     * Returns the size in byte of a <i>hdr_uwicrp_ack</i> packet header.
     * 
     * @return The size of a <i>hdr_uwicrp_ack</i> packet header.
     */
    static inline const int getAckPktHeaderSize() { return sizeof(hdr_uwicrp_ack); }
    
    /**
     * Returns the size in byte of a <i>hdr_uwicrp_data</i> packet header.
     * 
     * @return The size of a <i>hdr_uwicrp_data</i> packet header.
     */
    static inline const int getDataPktHeaderSize() { return sizeof(hdr_uwicrp_data); }
    
    /**
     * Returns the size in byte of a <i>hdr_uwicrp_status</i> packet header.
     * 
     * @return The size of a <i>hdr_uwicrp_status</i> packet header.
     */
    static inline const int getStatusPktHeaderSize() { return sizeof(hdr_uwicrp_status); }
    
    uint8_t ipAddr_;                   /**< IP of the current node. */
    int printDebug_;                    /**< Flag to enable or disable dirrefent levels of debug. */

private:
    
    static long numberofackpkt_;        /**< Comulative number of Ack packets processed by UwIcrpSink objects. */
    static long numberofstatuspkt_;     /**< Comulative number of Status packets processed by UwIcrpSink objects. */
};

#endif // UWICRP_MODULE_SINK_H
