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
 * @file   uwflooding.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Flooding based routing protocol.
 * 
 * Flooding based routing protocol.
 */

#ifndef UWFLOODING_H
#define	UWFLOODING_H

#define TTL_EQUALS_TO_ZERO   "TEZ"                     /**< Reason for a drop in a <i>UWFLOODING</i> module. */

#include "uwflooding-hdr.h"

#include <uwip-module.h>
#include <uwip-clmsg.h>
#include <uwcbr-module.h>

#include "mphy.h"
#include "packet.h"
#include <module.h>
#include <tclcl.h>

#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <iostream>
#include <rng.h>
#include <ctime>
#include <vector>
#include <fstream>
#include <map>
#include <list>

using namespace std;

/**
 * UwFlooding class is used to represent the routing layer of a node.
 */
class UwFlooding : public Module {
    
public:
    /**
     * Constructor of UwFlooding class.
     */
    UwFlooding();
    
    /**
     * Destructor of UwFlooding class.
     */
    virtual ~UwFlooding();
    
protected:
    /*****************************
     |     Internal Functions    |
     *****************************/
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
     * Cross-Layer messages synchronous interpreter.
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received
     * @return <i>0</i> if successful.
     */
    virtual int recvSyncClMsg(ClMessage*);
    
    /**
     * Cross-Layer messages asynchronous interpreter. Used to retrive the IP
     * od the current node from the IP module.
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received and used for the answer.
     * @return <i>0</i> if successful.
     */
    virtual int recvAsyncClMsg(ClMessage*);
    
    /**
     * Returns a nsaddr_t address from an IP written as a string in the form "x.x.x.x".
     * 
     * @param char* IP in string form
     * @return nsaddr_t that contains the IP converter from the input string
     */
    static nsaddr_t str2addr(const char*);
    
    /**
     * Writes in the Path Trace file the path contained in the Packet
     * 
     * @param Packet to analyze.
     */
    virtual void writePathInTrace(const Packet*, const string&);
    
    /**
     * Return a string with an IP in the classic form "x.x.x.x" converting an ns2 nsaddr_t address.
     * 
     * @param nsaddr_t& ns2 address
     * @return String that contains a printable IP in the classic form "x.x.x.x"
     */
    static string printIP(const nsaddr_t&);
    
private:
    // Variables
    
    uint8_t ipAddr_;
    int ttl_;                                   /**< Time to leave of the <i>UWFLOODING</i> packets. */
    double maximum_cache_time_;                 /**< Validity time of a packet entry. */
    int optimize_;                              /**< Flag used to enable the mechanism to drop packets processed twice. */
    long packets_forwarded_;                    /**< Number of packets forwarded by this module. */
    bool trace_path_;                           /**< Flag used to enable or disable the path trace file for nodes, */
    char* trace_file_path_name_;                /**< Name of the trace file that contains the list of paths of the data packets received. */
    ofstream trace_file_path_;                  /**< Ofstream used to write the path trace file in the disk. */
    ostringstream osstream_;                    /**< Used to convert to string. */
    
    typedef std::map<uint16_t, double> map_packets;               /**< Typedef for a packet id: (serial_number, timestamp). */
    typedef std::map<uint8_t, map_packets> map_forwarded_packets; /**< Typedef for a map of the packet forwarded (saddr, map_packets). */
    map_forwarded_packets my_forwarded_packets_;                  /**< Map of the packet forwarded. */
    
    /**
     * Copy constructor declared as private. It is not possible to create a new UwFlooding object passing to its constructor another UwFlooding object. 
     * 
     * @param UwFlooding& UwFlooding object.
     */
    UwFlooding(const UwFlooding&);
};

#endif // UWFLOODING_H
