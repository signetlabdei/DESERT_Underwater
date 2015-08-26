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
 * @file   uwicrp-module-sink.cc
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Implements UWIcrpSink.
 *
 */

#include "uwicrp-module-sink.h"

extern packet_t PT_UWICRP_ACK;
extern packet_t PT_UWICRP_STATUS;
extern packet_t PT_UWICRP_DATA;

/**
 * Adds the module for UwIcrpSinkModuleClass in ns2.
 */
static class UwIcrpSinkModuleClass : public TclClass {
public:

    UwIcrpSinkModuleClass() : TclClass("Module/UW/ICRPSink") {
    }

    TclObject* create(int, const char*const*) {
        return (new UwIcrpSink());
    }
} class_module_uwicrp_sink;

long UwIcrpSink::numberofstatuspkt_ = 0;
long UwIcrpSink::numberofackpkt_ = 0;

UwIcrpSink::UwIcrpSink()
: ipAddr_(0),
printDebug_(0)
{
    bind("printDebug_", &printDebug_);
    cout.precision(2);
    cout.setf(ios::floatfield, ios::fixed);
}

UwIcrpSink::~UwIcrpSink() {
}

int UwIcrpSink::recvSyncClMsg(ClMessage* m) {
    return Module::recvSyncClMsg(m);
}

int UwIcrpSink::recvAsyncClMsg(ClMessage* m) {
    if (m->type() == UWIP_CLMSG_SEND_ADDR) {
        UWIPClMsgSendAddr* m_ = (UWIPClMsgSendAddr*) m;
        ipAddr_ = m_->getAddr();
    }
    return Module::recvAsyncClMsg(m);
}

void UwIcrpSink::initialize() {
    if (ipAddr_ == 0) {
        UWIPClMsgReqAddr* m = new UWIPClMsgReqAddr(getId());
        sendSyncClMsgDown(m);
    }
}

int UwIcrpSink::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    
    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            this->initialize();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getackheadersize") == 0) {
            tcl.resultf("%d", this->getAckPktHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getdataheadersize") == 0) {
            tcl.resultf("%d", this->getDataPktHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getstatusheadersize") == 0) {
            tcl.resultf("%d", this->getStatusPktHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getackpktcount") == 0) {
            tcl.resultf("%lu", numberofackpkt_);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getstatuspktcount") == 0) {
            tcl.resultf("%lu", numberofstatuspkt_);
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "addr") == 0) {
            ipAddr_ = static_cast<uint8_t>(atoi(argv[2]));
            return TCL_OK;
            if (ipAddr_ == 0) {
                fprintf(stderr, "0 is not a valid IP address");
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }
    return Module::command(argc, argv);
}

void UwIcrpSink::recv(Packet *p) {
    hdr_cmn* ch = HDR_CMN(p);
    
    if (!ch->error()) {
        if (ch->direction() == hdr_cmn::UP) {
            if (ch->ptype_ == PT_UWICRP_STATUS) {
                if (printDebug_ > 10)
                    cout << "@" << Scheduler::instance().clock() << " -> S: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Status Packet - Garbage." << endl;
                Packet::free(p);
                return;
            } if (ch->ptype_ == PT_UWICRP_ACK) {
                if (printDebug_ > 10)
                    cout << "@" << Scheduler::instance().clock() << " -> S: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Ack Packet - Garbage." << endl;
                Packet::free(p);
                return;
            } else { // Data packet
                if (ch->next_hop() != UWIP_BROADCAST) { // The source has already a path to reach this node. length == 1 because the IP in position 0 is the IP of the source
                    if (printDebug_ > 10)
                        cout << "@" << Scheduler::instance().clock() << " -> S: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Data Packet Received. Send it up." << endl;
                    this->sendBackAck(p);
                    sendUp(p);
                    return;
                } else { // The source doesn't have a path to reach this node: create a status packet. In this case an ack is not required.
                    if (printDebug_ > 10)
                        cout << "@" << Scheduler::instance().clock() << " -> S: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Data Packet Received: Send it Up and Send back the Path." << endl;
                    Packet* p_new = Packet::alloc();
                    this->initStatusPkt(p, p_new);
                    numberofstatuspkt_++;
                    sendDown(p_new);
                    sendUp(p);
                    return;
                }
            }
        } else if (ch->direction() == hdr_cmn::DOWN) {
            if (printDebug_ > 10)
                cout << "@" << Scheduler::instance().clock() << " -> S: " << this->printIP(ipAddr_) <<  " - P: " << ch->uid() << " - DOWN - Packet from upper layer - Garbage." << endl;
            Packet::free(p);
            return;
        } else {
            if (printDebug_ > 10)
                cout << "@" << Scheduler::instance().clock() << " -> S: " << this->printIP(ipAddr_) <<  " - P: " << ch->uid() << " - ??? - NO_DIRECTION - Garbage." << endl;
            Packet::free(p);
            return;
        }
    }
}

void UwIcrpSink::initStatusPkt(Packet* p_old, Packet* p_new) {
    // Common header.
    hdr_cmn* ch_old       = HDR_CMN(p_old);
    hdr_cmn* ch_new       = HDR_CMN(p_new);
    //ch->uid()       = 0; // Set by the Application above.
    ch_new->ptype()       = PT_UWICRP_STATUS;
    //ch->size()      = 0; // Look Down;
    ch_new->direction()   = hdr_cmn::DOWN;
    //ch->next_hop()  = 0; // Set by SuwIPRoutingNode::forwardDataPacket.
    ch_new->next_hop()    = ch_old->prev_hop_;
    ch_new->prev_hop_     = ipAddr_;
    
    // IP Header.
    hdr_uwip* iph_old = HDR_UWIP(p_old);
    hdr_uwip* iph_new = HDR_UWIP(p_new);
    iph_new->daddr()  = iph_old->saddr(); // The destination is the source of the data packet
    //iph->dport() = 0; // Set by the Application above.
    //iph->saddr() = 0; // Set by IPModule
    //iph->sport() = 0; // Set by the Application above.
    //iph->ttl()   = 0; // Set by IPModule
    
    // Data header.
    hdr_uwicrp_data* icrpdatah_old             = HDR_UWICRP_DATA(p_old);
    hdr_uwicrp_status* icrpstatush_new         = HDR_UWICRP_STATUS(p_new);
    for (int i = 0; i < icrpdatah_old->list_of_hops_length(); i++) {
        icrpstatush_new->list_of_hops()[i] = icrpdatah_old->list_of_hops()[i];
    }
    icrpstatush_new->list_of_hops_length()     = icrpdatah_old->list_of_hops_length();
    icrpstatush_new->pointer_to_list_of_hops() = icrpdatah_old->pointer_to_list_of_hops();
    icrpstatush_new->creation_time() = Scheduler::instance().clock();
    
    ch_new->size()      += sizeof(hdr_uwicrp_status);
    ch_new->timestamp() = Scheduler::instance().clock();
}

string UwIcrpSink::printIP(const uint8_t _ip) {
    std::stringstream out;
    out << "0.0.0." << ((_ip & 0x000000ff));
    return out.str();
}

void UwIcrpSink::sendBackAck(const Packet* p) {
    Packet* p_ack = Packet::alloc();
    
    hdr_cmn* ch        = HDR_CMN(p);
    hdr_cmn* ch_ack    = HDR_CMN(p_ack);
    hdr_uwip* iph_ack  = HDR_UWIP(p_ack);
    
    ch_ack->ptype()    = PT_UWICRP_ACK;
    ch_ack->direction()= hdr_cmn::DOWN;
    ch_ack->next_hop() = ch->prev_hop_;
    ch_ack->prev_hop_  = ipAddr_;
    
    iph_ack->daddr()   = ch->prev_hop_;
    
    ch_ack->size()     += sizeof(hdr_uwicrp_ack);
    ch_ack->timestamp()= Scheduler::instance().clock();
    numberofackpkt_++;
    sendDown(p_ack);
} /* UwIcrpSink::sendBackAck */
