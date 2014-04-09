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
 * @file   uwip-module.cc
 * @author Giovanni Toso
 * @version 1.0.1
 * 
 * \brief Implements UWIPModule.
 *
 */

#include "uwip-module.h"
#include "uwip-clmsg.h"

extern packet_t PT_UWIP;

int hdr_uwip::offset_;

uint8_t UWIPModule::lastIP = 0;

/**
 * Adds the header for <i>hdr_uwip</i> packets in ns2.
 */
static class UwIpPktClass : public PacketHeaderClass {
public:

    UwIpPktClass() : PacketHeaderClass("PacketHeader/UWIP", sizeof (hdr_uwip)) {
        this->bind();
        bind_offset(&hdr_uwip::offset_);
    }
} class_uwip_pkt;

/**
 * Adds the module for UWIPModuleClass in ns2.
 */
static class UWIPModuleClass : public TclClass {
public:

    UWIPModuleClass() : TclClass("Module/UW/IP") {
    }

    TclObject* create(int, const char*const*) {
        return (new UWIPModule());

    }
} class_uwipmodule;

UWIPModule::UWIPModule()
:
ipAddr_(0),
debug_(0),
addr_type_inet(true)
{
    bind("debug_", &debug_);
    ipAddr_ = ++lastIP;
}

UWIPModule::~UWIPModule() {
}

int UWIPModule::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "addr") == 0) {
            tcl.resultf("%d", ipAddr_);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setaddrinet") == 0) {
            addr_type_inet = true;
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setaddrilink") == 0) {
            addr_type_inet = false;
            return TCL_OK;
        } else if (strcasecmp(argv[1], "addr-string") == 0) {
            tcl.resultf("%d.%d.%d.%d", (ipAddr_ & 0xff000000) >> 24, (ipAddr_ & 0x00ff0000) >> 16, (ipAddr_ & 0x0000ff00) >> 8, (ipAddr_ & 0x000000ff));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getipheadersize") == 0) {
            tcl.resultf("%d", this->getIpHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "printidspkts") == 0) {
            this->printIdsPkts();
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "addr") == 0) {
            ipAddr_ = static_cast<uint8_t>(atoi(argv[2]));
            if (ipAddr_ == 0) {
                fprintf(stderr, "0 is not a valid IP address");
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }
    return Module::command(argc, argv);
}

void UWIPModule::recv(Packet *p) {
    hdr_cmn* ch    = HDR_CMN(p);
    hdr_uwip* iph  = HDR_UWIP(p);
    
    if (ch->direction() == hdr_cmn::UP) {
        if (iph->saddr() == ipAddr_) {
            drop(p, 1, ORIGINATED_BY_ME);
            return;
        } else if (\
                (ch->next_hop() & 0x000000ff) == (ipAddr_ & 0x000000ff) || \
                (ch->next_hop() & 0x000000ff) == (UWIP_BROADCAST & 0x000000ff) || \
                (iph->daddr() & 0x000000ff) == (ipAddr_ & 0x000000ff) || \
                (iph->daddr() & 0x000000ff) == (UWIP_BROADCAST & 0x000000ff)) { // Right destination.
            ch->size() -= sizeof(hdr_uwip);
            sendUp(p);
            return;
        } else {
            drop(p, 1, NOT_FOR_ME_REASON);
            return;
        }
    } else if (ch->direction() == hdr_cmn::DOWN) { // Direction DOWN
        if (iph->daddr() == ipAddr_ || ch->next_hop() == ipAddr_) { // The node is sending a packet to itself, drop it.
            drop(p, 1, INVALID_DESTINATION_ADDR);
            return;
        }
        if (addr_type_inet) {
            ch->addr_type() = NS_AF_INET;
        } else {
            ch->addr_type() = NS_AF_ILINK;
        }
        
        ch->size()      += sizeof(hdr_uwip);
        
        if (iph->saddr() == 0) {
            iph->saddr() = ipAddr_;
        }
        
        if (ch->prev_hop_ == 0) {
            ch->prev_hop_ = ipAddr_;
        }
        
        if (iph->daddr() == 0) {
            drop(p, 1, DESTINATION_ADDR_UNSET);
            return;
        }

        if (ch->next_hop() == 0 && iph->daddr() != 0) {
            if (debug_)
                cerr << "Node:" << this->printIP(ipAddr_) << ":Warning:Packet sent with next_hop equals to 0" << endl;
            ch->next_hop() = iph->daddr();
        }
        
        sendDown(p);
    } else {
        Packet::free(p);
        return;
    }
}

nsaddr_t UWIPModule::str2addr(const char *str) {
    int level[4] = {0, 0, 0, 0};
    char tmp[20];
    strncpy(tmp, str, 19);
    tmp[19] = '\0';
    char *p = strtok(tmp, ".");
    for (int i = 0; p && i < 4; p = strtok(NULL, "."), i++) {
        
        /*FF: Let's take in consideration only the last value of the IP address
         e.g.: x.x.x.1 --> the address will be 1
         since the netmask is not considered anymore and the address is 8 bit 
         long, take in consideration the initial part of the IP doesn't make sense
         anymore */
        /*level[i] = atoi(p);
        if (level[i] > 255)
            level[i] = 255;
        else if (level[i] < 0)
            level[i] = 0;*/
        
        if (i == 3) {
            level[i] = atoi(p);
            if (level[i] > 255)
                level[i] = 255;
            else if (level[i] < 0)
                level[i] = 0;
        }
        
    }
    nsaddr_t addr = 0;
    //for (int i = 0; i < 4; i++) {
    //    addr += (level[i] << 8 * (3 - i));
    //}
    
    addr = level[3];
    
    return addr;
}

int UWIPModule::recvSyncClMsg(ClMessage* m) {
    if (m->type() == UWIP_CLMSG_SEND_ADDR) {
        UWIPClMsgSendAddr *c = new UWIPClMsgSendAddr(UNICAST, m->getSource());
        c->setAddr(ipAddr_);
        sendAsyncClMsg(c);

        ((UWIPClMsgSendAddr*) m)->setAddr(ipAddr_);
        return 0;
    }
    return Module::recvSyncClMsg(m);
}

const string UWIPModule::printIP(const nsaddr_t& ip_) {
    stringstream out;
    out << ((ip_ & 0xff000000)>>24);
    out << ".";
    out << ((ip_ & 0x00ff0000)>>16);
    out << ".";
    out << ((ip_ & 0x0000ff00)>>8);
    out << ".";
    out << ((ip_ & 0x000000ff));
    return out.str();
}

const string UWIPModule::printIP(const uint8_t& ip_) {
    std::stringstream out;
    out << ((ip_ & 0xff000000)>>24);
    out << ".";
    out << ((ip_ & 0x00ff0000)>>16);
    out << ".";
    out << ((ip_ & 0x0000ff00)>>8);
    out << ".";
    out << ((ip_ & 0x000000ff));
    return out.str();
}
