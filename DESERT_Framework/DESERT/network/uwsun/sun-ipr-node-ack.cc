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
 * @file   sun-ipr-node-ack.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Provides the implementation of all the methods regarding Ack Packets.
 *
 * Provides the implementation of all the methods regarding Ack Packets.
 */

#include "sun-ipr-node.h"

/* This function uses the value contained in the header of a uwcbr packet.
 * TODO: try to modifies this in order to use the ch->uid() field instead.
 */
void SunIPRoutingNode::sendBackAck(const Packet* p) {
    if (STACK_TRACE)
        cout << "> sendBackAck()" << endl;
    hdr_cmn* ch         = HDR_CMN(p);
    hdr_uwcbr* uwcbrh   = HDR_UWCBR(p);

    Packet* p_ack       = Packet::alloc();
    this->initPktAck(p_ack);
    
    hdr_cmn* ch_ack     = HDR_CMN(p_ack);
    hdr_uwip* iph_ack   = HDR_UWIP(p_ack);
    hdr_sun_ack* hack   = HDR_SUN_ACK(p_ack);
    
    ch_ack->next_hop()  = ch->prev_hop_;
    iph_ack->daddr()    = ch->prev_hop_;
    hack->uid()         = uwcbrh->sn();
    
    if (printDebug_ > 5)
        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":SEND:ACK_PKT:to:" << printIP(ch_ack->next_hop()) << ":id:" << hack->uid() << endl;
    number_of_ackpkt_++;
    if (trace_)
        this->tracePacket(p_ack, "SEND_ACK");
    sendDown(p_ack, this->getDelay(period_status_));
} /* SunIPRoutingNode::sendBackAck */

void SunIPRoutingNode::initPktAck(Packet* p) {
    if (STACK_TRACE)
        cout << "> initPktPathEstSearch()" << endl;

    // Common header.
    hdr_cmn* ch     = HDR_CMN(p);
    ch->uid()       = sunuid_++;
    ch->ptype()     = PT_SUN_ACK;
    //ch->size()    = 0; // Look down.
    ch->direction() = hdr_cmn::DOWN;
    //ch->next_hop()  = 0;
    ch->prev_hop_   = ipAddr_;
    
    // IP header.
    //hdr_uwip* iph  = HDR_UWIP(p);
    //iph->daddr() = 0;
    //iph->dport() = 0;
    //iph->saddr() = 0;
    //iph->sport() = 0;
    
    ch->size()                  += sizeof(hdr_sun_ack);
    ch->timestamp()             = Scheduler::instance().clock();
} /* SunIPRoutingNode::initPktAck */

/* This function is used to create a route error packet. This packet will be
 * sent by the current node to the source of the data packet that generated the
 * error. The packet created by this function is a path establishment packet
 * with the type field set to PATH_ERROR.
 */
void SunIPRoutingNode::createRouteError(const Packet* p_data, Packet* p) { // p is a path establishment packet beacause it contains a list of hops.
    if (STACK_TRACE)
        cout << "> createRouteError()" << endl;
    this->initPktPathEstSearch(p);
    hdr_cmn* ch_data        = HDR_CMN(p_data);
    hdr_uwip* iph_data      = HDR_UWIP(p_data);
    hdr_sun_data* hdata     = HDR_SUN_DATA(p_data);
    hdr_cmn* ch             = HDR_CMN(p);
    hdr_uwip* iph           = HDR_UWIP(p);
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);
    
    ch->prev_hop_       = ipAddr_;
    if (ch_data->prev_hop_ == iph_data->saddr()) { // Trick to fix the case in which the node that generates the error is one hop distant from the source.
        ch->next_hop()      = ch_data->prev_hop_;
    } else {
        ch->next_hop()       = hdata->list_of_hops()[hdata->pointer() - 1];
    }
    
    iph->saddr()        = ipAddr_;
    iph->daddr()        = iph_data->saddr();
    
    hpest->ptype()               = PATH_ERROR;
    hpest->list_of_hops_length() = hdata->pointer(); // Not + 1 because the current node is excluded.
    hpest->pointer()             = 1;
    
    // Hops from the current node to the source of the data packet.
    int w_ = hdata->pointer() - 1;
//    cout << "createRouteError(), node:" << printIP(ipAddr_) << ":" << printIP(iph->daddr()) << endl;
    for (int i = 0; i < hdata->pointer(); i++) {
        hpest->list_of_hops()[i] = hdata->list_of_hops()[w_];
//        cout << i << ":" << printIP(hpest->list_of_hops()[i]) << endl;
        w_--;
    }
} /* SunIPRoutingNode::createRouteError */

void SunIPRoutingNode::sendRouteErrorBack(Packet* p) {
    if (STACK_TRACE)
        cout << "> sendRouteErrorBack()" << endl;
    hdr_cmn* ch         = HDR_CMN(p);
    hdr_uwip* iph       = HDR_UWIP(p);
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);
    
    if (hpest->pointer() < hpest->list_of_hops_length()) {
        ch->next_hop() = hpest->list_of_hops()[hpest->pointer()];
    } else {
        ch->next_hop() = iph->daddr();
    }
//    cout << "ch->next_hop():" << printIP(ch->next_hop()) << endl;
    hpest->pointer()--;
    number_of_pathestablishment_++;
    if (trace_)
        this->tracePacket(p, "FRWD_ERR");
    sendDown(p, this->getDelay(period_status_));
} /* SunIPRoutingNode::sendRouteErrorBack */
