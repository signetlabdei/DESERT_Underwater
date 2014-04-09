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
 * @file   uwip-clmsg.h
 * @author Giovanni Toso
 * @version 1.0.1
 * 
 * \brief Cross layer messages definition for the UWIP Module.
 *
 */

#ifndef UWIP_CLMSG_H
#define UWIP_CLMSG_H

#include <clmessage.h>

#define UWIP_CLMSG_VERBOSITY 5                  /**< Verbosity level. */
#define UWIP_CLMSG_UPD_ROUTE_VERBOSITY 2        /**< Verbosity level. */

extern ClMessage_t UWIP_CLMSG_SEND_ADDR;
extern ClMessage_t UWIP_CLMSG_REQ_ADDR;
extern ClMessage_t UWIP_CLMSG_UPD_ROUTE;

/**
 * Class that manages cross layer messages that require the IP of the node.
 * 
 * @param src
 */
class UWIPClMsgReqAddr : public ClMessage {
public:
    UWIPClMsgReqAddr(int src);
    UWIPClMsgReqAddr(UWIPClMsgReqAddr *m);
    ClMessage* copy();
};

/**
 * Class used to answer to UWIPClMsgReqAddr cross layer messages.
 */
class UWIPClMsgSendAddr : public ClMessage {
public:
    UWIPClMsgSendAddr();
    UWIPClMsgSendAddr(DestinationType dtype, int value);
    UWIPClMsgSendAddr(UWIPClMsgSendAddr *m);
    ClMessage* copy();
    void setAddr(nsaddr_t addr);
    nsaddr_t getAddr();
private:
    nsaddr_t addr_;
};

class Packet;

/**
 * Class that manages cross layer messages that contain route updates.
 * 
 * @param p Packet
 */
class UWIpClMsgUpdRoute : public ClMessage {
public:
    UWIpClMsgUpdRoute(Packet* p);

    virtual ~UWIpClMsgUpdRoute() {}
    virtual ClMessage* copy(); // copy the message

    Packet* getPacket() {return packet;}

    void setPacket(Packet* p) {packet = p;}
    
protected:
    Packet* packet;
};

#endif // UWIP_CLMSG_H
