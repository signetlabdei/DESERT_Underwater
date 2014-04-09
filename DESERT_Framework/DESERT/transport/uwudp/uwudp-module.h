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
 * @file   uwudp-module.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Provides the <i>UWUDP</i> packets header description and the definition of the class <i>UWUDP</i>.
 *
 * <i>UWUDP</i> implements a very simple module used to manage different application flows.
 */

#ifndef _UWUDP_H_
#define _UWUDP_H_

#include <uwip-module.h>

#include <module.h>
#include <map>
#include <set>

#define DROP_UNKNOWN_PORT_NUMBER "UPN"          /**< Reason for a drop in a <i>UWUDP</i> module. */
#define DROP_RECEIVED_DUPLICATED_PACKET "RDP"   /**< Reason for a drop in a <i>UWUDP</i> module. */

#define HDR_UWUDP(P)      (hdr_uwudp::access(P))

extern packet_t PT_UWUDP;

/**
 * <i>hdr_uwudp</i> describes <i>UWUDP</i> packets.
 */
typedef struct hdr_uwudp {
    uint8_t sport_;   /**< Source port number. */
    uint8_t dport_;   /**< Destination port number. */
    
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset_ variable.
     */
    inline static int& offset() {
        return offset_;
    }
    
    inline static struct hdr_uwudp * access(const Packet * p) {
        return (struct hdr_uwudp*) p->access(offset_);
    }
    
    /**
     * Reference to the sport_ variable.
     */
    inline uint8_t& sport() { return sport_; }
    
    /**
     * Reference to the dport_ variable.
     */
    inline uint8_t& dport() { return dport_; }
} hdr_uwudp;

/**
 * UwUdp class is used to manage UWUDP packets, and flows to and from upper modules.
 */
class UwUdp : public Module {
public:
    
    /**
     * Constructor of UwUdp class.
     */
    UwUdp();
    
    /**
     * Destructor of UwUdp class.
     */
    virtual ~UwUdp();
    
    /**
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet will be received.
     */
    virtual void recv(Packet*);
    
    /**
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet will be received.
     * @param Handler* Handler.
     */
    virtual void recv(Packet*, int);
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int , const char*const*);
    
    /**
     * Associates a module with a port.
     * 
     * @param Module* Module to which associate a port.
     * @return Id of the port associated to the module.
     */
    virtual int assignPort(Module*);
    
    /**
     * Prints the IDs of the packet's headers defined by UWUDP.
     */
    inline void printIdsPkts() const {
        std::cout << "UWUDP packets IDs:" << std::endl;
        std::cout << "PT_UWUDP: \t\t" << PT_UWUDP << std::endl;
    }

protected:
    uint16_t portcounter;                                       /**< Counter used to generate new port numbers. */
    map<int, int> port_map;                                     /**< Map: value = port;  key = id. */
    map<int, int> id_map;                                       /**< Map: value = id;    key = port. */
    
    typedef std::map<uint8_t, std::set<int> > map_packets_el; 
    std::map<uint16_t, map_packets_el> map_packets; /**< Map used to keep track of the packets received by each port. The key is the port number. The second element
                                                                               contains the saddr IP and a vector used as bucketlist.*/
    
    int drop_duplicated_packets_;       /**< Flat to enable or disable the drop of duplicated packets. */
    int debug_;                         /**< Flag to enable or disable dirrefent levels of debug. */

    /**
     * Returns the size in byte of a <i>hdr_uwudp</i> packet header.
     * 
     * @return The size of a <i>hdr_uwudp</i> packet header.
     */
    static inline int getUdpHeaderSize() { return sizeof(hdr_uwudp); }
};

#endif // _UWUDP_H_
