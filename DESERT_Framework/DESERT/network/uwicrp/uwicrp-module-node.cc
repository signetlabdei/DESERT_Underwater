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
 * @file   uwicrp-module-node.cc
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Implements UWIcrpNode.
 *
 * Implements UWIcrpNode.
 */

#include "uwicrp-module-node.h"

extern packet_t PT_UWICRP_ACK;
extern packet_t PT_UWICRP_STATUS;
extern packet_t PT_UWICRP_DATA;

/**
 * Adds the module for UwIcrpNodeModuleClass in ns2.
 */
static class UwIcrpNodeModuleClass : public TclClass {
public:

    UwIcrpNodeModuleClass() : TclClass("Module/UW/ICRPNode") {
    }

    TclObject* create(int, const char*const*) {
        return (new UwIcrpNode());
    }
} class_module_uwicrp_node;

long UwIcrpNode::numberofstatuspkt_ = 0;
long UwIcrpNode::numberofdatapkt_ = 0;
long UwIcrpNode::numberofackpkt_ = 0;

void AckWaiting::expire(Event* e) {
    module->ackLost();
} /* AckWaiting::expire */

UwIcrpNode::UwIcrpNode()
: ipAddr_(0),
printDebug_(0),
timer_ack_waiting_(10),
ackwaitingTmr_(this)
{
    bind("printDebug_", &printDebug_);
    bind("maxvaliditytime_", &max_validity_time_);
    bind("timer_ack_waiting_", &timer_ack_waiting_);
    clearAllRouteTable();
    cout.precision(2);
    cout.setf(ios::floatfield, ios::fixed);
    for (int i = 0; i < HOP_TABLE_LENGTH; i++) {
        route_table[i].destination = 0;
        route_table[i].hopcount = INT_MAX;
        route_table[i].creationtime = 0;
        route_table[i].next_hop = -1;
        route_table[i].isValid = false;
    }
}

UwIcrpNode::~UwIcrpNode() {
}

int UwIcrpNode::recvSyncClMsg(ClMessage* m) {
    return Module::recvSyncClMsg(m);
}

int UwIcrpNode::recvAsyncClMsg(ClMessage* m) {
    if (m->type() == UWIP_CLMSG_SEND_ADDR) {
        UWIPClMsgSendAddr* m_ = (UWIPClMsgSendAddr*) m;
        ipAddr_ = m_->getAddr();
    }
    return Module::recvAsyncClMsg(m);
}

void UwIcrpNode::initialize() {
    if (ipAddr_ == 0) {
        UWIPClMsgReqAddr* m = new UWIPClMsgReqAddr(getId());
        sendSyncClMsgDown(m);
    }
}

void UwIcrpNode::clearRouteTable(const int& i) {
    if (i < HOP_TABLE_LENGTH) {
        route_table[i].destination = 0;
        route_table[i].hopcount = INT_MAX;
        route_table[i].creationtime = 0;
        route_table[i].next_hop = -1;
        route_table[i].isValid = false;
    }
}

void UwIcrpNode::clearAllRouteTable() {
    for (int i = 0; i < HOP_TABLE_LENGTH; i++) {
        route_table[i].destination = 0;
        route_table[i].hopcount = INT_MAX;
        route_table[i].creationtime = 0;
        route_table[i].next_hop = -1;
        route_table[i].isValid = false;
    }
}

int UwIcrpNode::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    
    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            this->initialize();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "clearhops") == 0) {
            this->clearAllRouteTable();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "printhops") == 0) {
            this->printHopTable();
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
        } else if (strcasecmp(argv[1], "getdatapktcount") == 0) {
            tcl.resultf("%lu", numberofdatapkt_);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getstatuspktcount") == 0) {
            tcl.resultf("%lu", numberofstatuspkt_);
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "ipsink") == 0) {
            ipSink_ = static_cast<uint8_t>(atoi(argv[2]));
            if (ipSink_ == 0) {
                fprintf(stderr, "0 is not a valid IP address");
                return TCL_ERROR;
            }
            if (printDebug_)
                cout << "-> Node linked with Sink: " << printIP(ipSink_) << endl;
            return TCL_OK;
        } else if (strcasecmp(argv[1], "addr") == 0) {
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

void UwIcrpNode::recv(Packet *p) {
    hdr_cmn* ch = HDR_CMN(p);
    hdr_uwip* iph = HDR_UWIP(p);
    hdr_uwicrp_data* icrp_datah;
    hdr_uwicrp_status* icrp_statush;
    
    if (!ch->error()) {
        // Checks the packet type
        if (ch->ptype_ == PT_UWICRP_STATUS) {
            icrp_statush = HDR_UWICRP_STATUS(p);
        } else {
            icrp_datah = HDR_UWICRP_DATA(p);
        }
        if (ch->direction() == hdr_cmn::UP) {
            if (ch->ptype_ == PT_UWICRP_STATUS) {
                if (iph->daddr() == ipAddr_) { // The packet is for me, process it and drop
                    if (printDebug_ > 10)
                        cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Status Packet for me." << endl;
                    this->addRouteEntry(p);
                    Packet::free(p);
                    return;
                } else { // This node has to forward the status packet to the next hop
                    if (printDebug_ > 10)
                        cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Status Packet, process and forward it." << endl;
                    // Add the entry
                    this->addRouteEntry(p);
                    // Forward the packet
                    Packet* p_new = Packet::alloc();
                    p_new = p->copy();
                    hdr_cmn* ch_new = HDR_CMN(p_new);
                    hdr_uwicrp_status* icrp_statush_new = HDR_UWICRP_STATUS(p_new);
                    ch_new->prev_hop_ = ipAddr_;
                    ch_new->next_hop() = icrp_statush_new->list_of_hops()[--icrp_statush_new->pointer_to_list_of_hops()]; // TODO: check
                    numberofstatuspkt_++;
                    sendDown(p_new);
                    Packet::free(p);
                    return;
                }             
            } else if (ch->ptype_ == PT_UWICRP_ACK) {
                if (iph->daddr() == ipAddr_) {
                    ackwaitingTmr_.force_cancel();
                }
            } else { // Data packet to forward
                if (iph->daddr() != ipAddr_) { // The destination (sink) must be different from my IP, this is a node
                    if (printDebug_ > 10)
                        cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Data Packet Received." << endl;
                    if (ch->next_hop() != UWIP_BROADCAST) { // The packet wasn't sent in bcast by the source
                        if (this->addIpInList(p, ipAddr_)) { // The IP of the current node is now in the list
                            int pointer_ = this->findInRouteTable(ipSink_);
                            if (pointer_ != -1 && route_table[pointer_].next_hop != 0) { // I have a valid next hop
                                ch->next_hop() = route_table[pointer_].next_hop;
                                ackwaitingTmr_.resched(timer_ack_waiting_);
                            } else { // Otherwise send the packet in broadcast
                                ch->next_hop() = UWIP_BROADCAST;
                            }
                            if (printDebug_ > 10)
                                cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Data Packet to Forward - IP inserted." << endl;
                            this->sendBackAck(p);
                            ch->prev_hop_ = ipAddr_;
                            numberofdatapkt_++;
                            sendDown(p);
                            return;
                        } else {
                            Packet::free(p);
                            return;
                        }
                    } else { // The packet was sent in bcast by the source: add my IP to the list and forward it in bcast
                        if (!this->isIpInList(p, ipAddr_)) { // If my IP is not already in the list
                            if (this->addIpInList(p, ipAddr_)) { // The IP of the current node is now in the list
                                ch->prev_hop_ = ipAddr_;
                                ch->next_hop() = UWIP_BROADCAST;
                                if (printDebug_ > 10)
                                    cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Data Packet in Bcast to Forward - IP inserted." << endl;
                                numberofdatapkt_++;
                                sendDown(p);
                                return;
                            } else {
                                Packet::free(p);
                                return;
                            }
                        } else {
                            if (printDebug_ > 10)
                                cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - UP - Data Packet already processed -> Discard it." << endl;
                            Packet::free(p);
                            return;
                        }
                    }
                } else {
                    Packet::free(p);
                    return;
                }
            }
        } else if (ch->direction() == hdr_cmn::DOWN) { // The node received a packet from an application, it has to send it to the right destination
            Packet* p_new = Packet::alloc();
            p_new = p->copy();
            this->initPkt(p_new);
            hdr_cmn* ch_new   = HDR_CMN(p_new);
            // Do I have a path to the sink?
            int pointer_ = this->findInRouteTable(ipSink_);
            if (pointer_ != -1 && route_table[pointer_].next_hop != 0) {
                if (ch->next_hop() != 0) {
                    ch_new->next_hop() = route_table[pointer_].next_hop;
                    ackwaitingTmr_.resched(timer_ack_waiting_);
                }
            }
            if (printDebug_ > 10)
                cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - DOWN - New Data Packet. Next hop: " << printIP(ch_new->next_hop()) << endl;
            numberofdatapkt_++;
            sendDown(p_new);
            Packet::free(p);
            return;
        } else {
            if (printDebug_ > 10)
                cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) <<  " - P: " << ch->uid() << " - ??? - NO_DIRECTION - Garbage." << endl;
            Packet::free(p);
            return;
        }
    }
}

void UwIcrpNode::initPkt(Packet* p) {
    // Common header.
    hdr_cmn* ch       = HDR_CMN(p);
    //ch->uid()       = 0; // Set by the Application above.
    //ch->ptype()     = 0; // Set by the Application above.
    //ch->size()      = 0; // Look Down;
    ch->direction()   = hdr_cmn::DOWN;
    //ch->next_hop()  = 0; // Set by SuwIPRoutingNode::forwardDataPacket.
    ch->next_hop()    = UWIP_BROADCAST;
    ch->prev_hop_     = ipAddr_;
    
    // IP Header.
    hdr_uwip* iph = HDR_UWIP(p);
    iph->daddr()  = ipSink_; // The destination is always the Sink associated.
    //iph->dport() = 0; // Set by the Application above.
    //iph->saddr() = 0; // Set by IPModule
    //iph->sport() = 0; // Set by the Application above.
    //iph->ttl()   = 0; // Set by IPModule
    
    // Data header.
    hdr_uwicrp_data* icrpdatah           = HDR_UWICRP_DATA(p);
    icrpdatah->list_of_hops()[0]         = ipAddr_;
    icrpdatah->list_of_hops_length()++;
    icrpdatah->pointer_to_list_of_hops() = 0;
    
    ch->size()        += sizeof(hdr_uwicrp_data);
    ch->timestamp()   = Scheduler::instance().clock();
}

void UwIcrpNode::addRouteEntry(Packet* p) {
    // FIX this: it works only with one sink!
    // This version doesn't take count of new best routes, it always override the olds
    hdr_cmn* ch = HDR_CMN(p);
    hdr_uwip* iph = HDR_UWIP(p);
    hdr_uwicrp_status* icrp_statush = HDR_UWICRP_STATUS(p);
    if (route_table[0].destination != 0) { // I have already a route to the destination
        int new_nop_count_ = icrp_statush->list_of_hops_length() - icrp_statush->pointer_to_list_of_hops();
        if (route_table[0].isValid == false || new_nop_count_ <= route_table[0].hopcount) { // I have to update the old route
            route_table[0].destination = iph->saddr();
            route_table[0].hopcount = icrp_statush->list_of_hops_length() - icrp_statush->pointer_to_list_of_hops();
            route_table[0].isValid = true;
            route_table[0].creationtime = Scheduler::instance().clock();
            route_table[0].next_hop = ch->prev_hop_;
        }
    } else { // I don't have any route to the destination, save it
        route_table[0].destination = iph->saddr();
        route_table[0].hopcount = icrp_statush->list_of_hops_length() - icrp_statush->pointer_to_list_of_hops();
        route_table[0].isValid = true;
        route_table[0].creationtime = Scheduler::instance().clock();
        route_table[0].next_hop = ch->prev_hop_;
    }
    return;
}

int UwIcrpNode::findInRouteTable(nsaddr_t ip_) {
    for (int i = 0; i < HOP_TABLE_LENGTH; i++) {
        if (route_table[i].creationtime != 0) {
            if ((Scheduler::instance().clock() - route_table[i].creationtime) > max_validity_time_) {
                this->clearRouteTable(i);
            }
        }
        if (route_table[i].destination == ip_ && route_table[i].isValid) {
            route_table[i].creationtime = Scheduler::instance().clock();
            return i;
        }
    }
    return -1;
}

bool UwIcrpNode::isIpInList(Packet* p, nsaddr_t ip_) {
    hdr_uwicrp_data* icrp_datah = HDR_UWICRP_DATA(p);
    if (icrp_datah->list_of_hops_length() == 0)
        return false;
    else if (icrp_datah->list_of_hops_length() > 0) {
        for (int i = 0; i < icrp_datah->list_of_hops_length(); i++)
            if (icrp_datah->list_of_hops()[i] == ip_)
                return true;
    }
    return false;
}

bool UwIcrpNode::addIpInList(Packet* p, nsaddr_t ip_) {
    hdr_uwicrp_data* icrp_datah = HDR_UWICRP_DATA(p);
    if (icrp_datah->list_of_hops_length() < MAX_HOP_NUMBER) { // Ok, I can add another IP
        icrp_datah->list_of_hops()[icrp_datah->list_of_hops_length()] = ip_;
        icrp_datah->pointer_to_list_of_hops()++;
        icrp_datah->list_of_hops_length()++;
        return true;
    } else { // The list if full: stop!
        return false;
    }
}

void UwIcrpNode::printHopTable() {
    for (int i = 0; i < HOP_TABLE_LENGTH; i++) {
        if (route_table[i].isValid) {
            cout << "Routing table node: " << this->printIP(ipAddr_) << endl;
            cout << i << " : " << '\t' << this->printIP(route_table[i].destination)
                    << '\t' << "next hop: " << this->printIP(route_table[i].next_hop)
                    << '\t' << "hop count: " << route_table[i].hopcount
                    << endl;
        }
    }
}

string UwIcrpNode::printIP(const uint8_t _ip) {
    std::stringstream out;
    out << "0.0.0." << ((_ip & 0x000000ff));
    return out.str();
}

nsaddr_t UwIcrpNode::str2addr(const char *str) {
    int level[4] = {0, 0, 0, 0};
    char tmp[20];
    strncpy(tmp, str, 19);
    tmp[19] = '\0';
    char *p = strtok(tmp, ".");
    for (int i = 0; p && i < 4; p = strtok(NULL, "."), i++) {
        level[i] = atoi(p);
        if (level[i] > 255)
            level[i] = 255;
        else if (level[i] < 0)
            level[i] = 0;
    }
    nsaddr_t addr = 0;
    for (int i = 0; i < 4; i++) {
        addr += (level[i] << 8 * (3 - i));
    }
    return addr;
}

void UwIcrpNode::sendBackAck(const Packet* p) {
    hdr_cmn* ch   = HDR_CMN(p);

    Packet* p_ack   = Packet::alloc();
    this->initPktAck(p_ack);
    
    hdr_cmn* ch_ack      = HDR_CMN(p_ack);
    hdr_uwip* iph_ack    = HDR_UWIP(p_ack);
    
    ch_ack->next_hop() = ch->prev_hop_;
    iph_ack->daddr()   = ch->prev_hop_;
    
    numberofackpkt_++;
    sendDown(p_ack);
} /* UwIcrpNode::sendBackAck */

void UwIcrpNode::initPktAck(Packet* p) {
    // Common header.
    hdr_cmn* ch     = HDR_CMN(p);
    ch->ptype()     = PT_UWICRP_ACK;
    //ch->size()    = 0; // Look down.
    ch->direction() = hdr_cmn::DOWN;
    ch->next_hop()  = UWIP_BROADCAST;
    ch->prev_hop_   = ipAddr_;
    
    // IP header.
    hdr_uwip* iph  = HDR_UWIP(p);
    iph->daddr()   = UWIP_BROADCAST; // A path search packet is sent in broadcast.
    //iph->dport() = 0; // Not needed: no applications above.
    //iph->saddr() = 0; // Set by IPModule.
    //iph->sport() = 0; // Not needed: no applications above.
    
    ch->size()                  += sizeof(hdr_uwicrp_ack);
    ch->timestamp()             = Scheduler::instance().clock();
} /* UwIcrpNode::initPktAck */

void UwIcrpNode::ackLost() {
    this->clearAllRouteTable();
}
