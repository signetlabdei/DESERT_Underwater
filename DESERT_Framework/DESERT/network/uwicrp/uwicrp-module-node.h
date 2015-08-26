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
 * @file   uwicrp-module-node.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Provides a module for nodes that needs a simple and dynamic routing protocol.
 *
 */

#ifndef UWICRP_MODULE_NODE_H
#define	UWICRP_MODULE_NODE_H

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
#include <climits>
#include <module.h>
#include <tclcl.h>
#include <vector>

using namespace std;

class UwIcrpNode;

/**
 * AckWaiting class is used to handle the timer of acks.
 * When a node sends a packet it waits for and ack
 * for a period managed by an AckWaiting object.
 */
class AckWaiting : public TimerHandler {
public:
    AckWaiting(UwIcrpNode* m) : TimerHandler() {
        module = m;
    }

protected:
    virtual void expire(Event *e);
    UwIcrpNode* module;
};

/**
 * UwIcrpNode class is used to represent the routing layer of a node.
 */
class UwIcrpNode : public Module {
    friend class AckWaiting;
   
public:
    
    /**
     * Constructor of UwIcrpNode class.
     */
    UwIcrpNode();
    
    /**
     * Destructor of UwIcrpNode class.
     */
    ~UwIcrpNode();
    
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
     * Initializes a UwIcrpNode node.
     * It sends to the lower layers a Sync message asking for the IP of the node.
     * 
     * @see UWIPClMsgReqAddr(int src)
     * @see sendSyncClMsgDown(ClMessage* m)
     */
    virtual void initialize();
    
    /**
     * Removes a specific entry in the routing table of the node.
     * 
     * @param int& Index number of the entry to remove.
     */
    virtual void clearRouteTable(const int&);
    
    /**
     * Clears completely the routing table of the node.
     */
    virtual void clearAllRouteTable();

    /**
     * Initializes a Data packet (previously allocated).
     * 
     * @param Packet* Pointer to a Data to initialize.
     */
    virtual void initPkt(Packet*);
    
    /** Adds the information received from a Status packet in the routing table of the node.
     * 
     * @param Packet* Status packet received from a sink.
     */
    virtual void addRouteEntry(Packet*);
    
    /**
     * Seeks for an entry in the routing table that contains information to a specific address passed as argument.
     * 
     * @param nsaddr_t Address of the destination to which search information.
     * @return Index value of the entry in the routing table that contains routing information to the destination.
     */
    virtual int findInRouteTable(nsaddr_t);
    
    /**
     * Checks if a specific IP is in the header of the packet passed as argument.
     * If yes it returns <i>true</i>, otherwise it return <i>false</i>.
     * 
     * @param Packet* Pointer to a packet to analyze.
     * @param nsaddr_t Address to search for.
     * @return <i>true</i> if the IP of the current node is in the header, otherwise <i>false</i>.
     */
    virtual bool isIpInList(Packet*, nsaddr_t);
    
    /**
     * Adds an IP passed as argument in the header of a Data packet passed as argument.
     * The function returns <i>true</i> if it added the IP, <i>false</i> otherwise (not enough space in the header).
     * 
     * @param Packet* Pointer to a Data packet in which to add an IP.
     * @param nsaddr_t Address to add in the header.
     * @return <i>true</i> if the IP was added, <i>false</i> otherwise.
     */
    virtual bool addIpInList(Packet*, nsaddr_t);
    
    /**
     * Prints in the stdout the routing table of the current node.
     */
    virtual void printHopTable();
    
    /**
     * Return a string with an IP in the classic form "x.x.x.x" converting an ns2 nsaddr_t address.
     * 
     * @param nsaddr_t& ns2 address
     * @return String that contains a printable IP in the classic form "x.x.x.x"
     */
    virtual string printIP(const uint8_t);
    
    /**
     * Returns a nsaddr_t address from an IP written as a string in the form "x.x.x.x".
     * 
     * @param char* IP in string form
     * @return nsaddr_t that contains the IP converter from the input string
     */
    nsaddr_t str2addr(const char*);
    
    /**
     * Creates an ack packet after the reception a data packet received correctly
     * @param p
     * @see UwIcrpNode::initPktAck
     */
    virtual void sendBackAck(const Packet* p);
    
    /**
     * Initializes an Ack packet (previously allocated).
     * 
     * @param Packet* Pointer to an Ack packet to initialize.
     */
    virtual void initPktAck(Packet* p);
    
    /**
     * Resets all the routing information because the lost of an ack.
     */
    virtual void ackLost();
    
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
    uint8_t ipSink_;                   /**< IP of the sink associated. */
    routing_table_entry route_table[HOP_TABLE_LENGTH];  /**< Node routing table. */
    double max_validity_time_;          /**< Maximum validity time of a route. */
    int printDebug_;                    /**< Flag to enable or disable dirrefent levels of debug. */
    double timer_ack_waiting_;          /**< Ack waiting timer. */
    AckWaiting ackwaitingTmr_;          /**< AckWaiting object. */
    
private:
    
    static long numberofstatuspkt_;     /**< Comulative number of Status packets processed by UwIcrpNode objects. */
    static long numberofdatapkt_;       /**< Comulative number of Data packets processed by UwIcrpNode objects. */
    static long numberofackpkt_;        /**< Comulative number of Ack packets processed by UwIcrpNode objects. */
};

#endif // UWICRP_MODULE_NODE_H

