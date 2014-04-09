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
 * @file   uwip-module.h
 * @author Giovanni Toso
 * @version 1.0.1
 * 
 * \brief Provides the <i>UWIP</i> packets header description. Definition of the class that define the network layer.
 *
 */

#ifndef _UWIPMODULE_
#define _UWIPMODULE_

#include <module.h>

#include <iostream>
#include <string>
#include <sstream>
                       
#define NOT_FOR_ME_REASON "NFM"         /**< Reason for a drop in a <i>UWIP</i> module. */
#define DESTINATION_ADDR_UNSET "DAU"    /**< Reason for a drop in a <i>UWIP</i> module. */
#define ORIGINATED_BY_ME "OBM"          /**< Reason for a drop in a <i>UWIP</i> module. */
#define INVALID_DESTINATION_ADDR "IDA"  /**< Reason for a drop in a <i>UWIP</i> module. */

#define HDR_UWIP(P)      (hdr_uwip::access(P))

using namespace std;

static const uint8_t UWIP_BROADCAST        = static_cast<uint8_t>(0x000000ff);  /**< Variable used to represent a broadcast UWIP. */

extern packet_t PT_UWIP;

/**
 * <i>hdr_uwip</i> describes <i>UWIP</i> packets.
 */
typedef struct hdr_uwip {
    uint8_t saddr_;    /**< IP of the source. */
    uint8_t daddr_;    /**< IP of the destination. */

    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset_ variable.
     */
    inline static int& offset() {
        return offset_;
    }
    
    
    inline static hdr_uwip * access(const Packet * p) {
        return (hdr_uwip*) p->access(offset_);
    }
    
    /**
     * Reference to the saddr_ variable.
     */
    uint8_t& saddr() {
        return saddr_;
    }
    
    /**
     * Reference to the daddr_ variable.
     */
    uint8_t& daddr() {
        return daddr_;
    }
    
} hdr_uwip;

/**
 * UWIPModule class is used to define the Internet Protocol (IP) layer of a node.
 */
class UWIPModule : public Module {
    
public:
    
    /**
     * Constructor of UWIPModule class.
     */
    UWIPModule();
    
    /**
     * Destructor of UWIPModule class.
     */
    virtual ~UWIPModule();
    
    /**
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet received.
     */
    virtual void recv(Packet *p);
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int argc, const char*const* argv);
    
    /**
     * Cross-Layer messages synchronous interpreter.
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received
     * @return <i>0</i> if successful.
     */
    virtual int recvSyncClMsg(ClMessage* m);
    
    /**
     * Returns a nsaddr_t address from an IP written as a string in the form "x.x.x.x".
     * 
     * @param char* IP in string form
     * @return nsaddr_t that contains the IP converter from the input string
     */
    static nsaddr_t str2addr(const char* str);
    
    /**
     * Returns a string with an IP in the classic form "x.x.x.x" converting an ns2 nsaddr_t address.
     * 
     * @param nsaddr_t& ns2 address
     * @return String that contains a printable IP in the classic form "x.x.x.x"
     */
    static const string printIP(const nsaddr_t&);
    
    /**
     * Returns a string with an IP in the classic form "x.x.x.x" converting an uint8_t address.
     * 
     * @param uint8_t& address
     * @return String that contains a printable IP in the classic form "x.x.x.x"
     */
    static const string printIP(const uint8_t&);
    
    /**
     * Prints the IDs of the packet's headers defined by UWIP.
     */
    inline void printIdsPkts() const {
        std::cout << "UWIP packets IDs:" << std::endl;
        std::cout << "PT_UWIP: \t\t" << PT_UWIP << std::endl;
    }
    
protected:
    static uint8_t lastIP;     /**< Used to set a default IP address. */
    uint8_t ipAddr_;            /**< IP address of the node. */
    int debug_;                 /**< Flag to enable or disable dirrefent levels of debug. */
    bool addr_type_inet;        /**< <i>true</i> if the addressing type is <i>INET</i>, <i>false</i> if it is <i>ILINK</i>. */
    
    /**
     * Returns the size in byte of a <i>hdr_sun_data</i> packet header.
     * 
     * @return The size of a <i>hdr_sun_data</i> packet header.
     */
    static inline int getIpHeaderSize() { return sizeof(hdr_uwip); }
};

#endif // _UWIPMODULE_
