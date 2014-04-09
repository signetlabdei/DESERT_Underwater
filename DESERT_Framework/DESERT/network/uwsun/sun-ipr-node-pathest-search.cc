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
 * @file   sun-ipr-node-pathest-search.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Provides the implementation of all the methods regarding Path Establishment Search Packets.
 *
 * Provides the implementation of all the methods regarding Path Establishment Search Packets.
 */

#include "sun-ipr-node.h"

extern packet_t PT_SUN_DATA;
extern packet_t PT_SUN_PATH_EST;

/* This function is used to send a path search request.
 * This function sends a packet that will gather information about the path.
 * This function will be invoked only by a node that has to transmit to the sink.
 */
void SunIPRoutingNode::searchPath() {
    if (STACK_TRACE)
        cout << "> searchPath()" << endl;
    if (getNumberOfHopToSink() == 1) { // Do nothing, the node is directly connected with the sink
        if (printDebug_ > 10)
            cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":NODE_HC1:SEARCH_PATH:???:" << endl;
        return;
    } else if (this->getNumberOfHopToSink() == 0) {
        // Update internal variables.
        this->clearHops();
        this->setNumberOfHopToSink(0);
        // Create and send a new path establishment packet.
        Packet* p = Packet::alloc();
        this->initPktPathEstSearch(p);
        number_of_pathestablishment_++;
        if (trace_)
            this->tracePacket(p, "SEND_SRC");
        sendDown(p, this->getDelay(period_status_));
        return;
    }
} /* SunIPRoutingNode::searchPath */

/* This function initializes a new path establishment packet
 * previously allocated.
 */
void SunIPRoutingNode::initPktPathEstSearch(Packet* p) const {
    if (STACK_TRACE)
        cout << "> initPktPathEstSearch()" << endl;

    // Common header.
    hdr_cmn* ch     = HDR_CMN(p);
    ch->uid()       = sunuid_++;
    ch->ptype()     = PT_SUN_PATH_EST;
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
    //iph->ttl()   = 0; // Set by IPModule.
    
    // Path establishment header.
    hdr_sun_path_est* hpest      = HDR_SUN_PATH_EST(p);
    hpest->ptype()               = PATH_SEARCH;
    hpest->list_of_hops_length() = 0;
    for (int i = 0; i < MAX_HOP_NUMBER; i++) {
        hpest->list_of_hops()[i] = 0;
    }
    hpest->quality()             = 0;
    
    ch->size()                   += sizeof(hdr_sun_path_est);
    ch->timestamp()              = Scheduler::instance().clock();
} /* SunIPRoutingNode::initPktPathEstSearch */

/* This functions is used by a node that receives a path establishment packet.
 * This function is used to process this kind of packets.
 */
void SunIPRoutingNode::replyPathEstSearch(Packet* p) {
    if (STACK_TRACE)
        cout << "> replyPathEstSearch()" << endl;
    hdr_cmn* ch             = HDR_CMN(p);
    hdr_uwip* iph           = HDR_UWIP(p);
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);
    
    if (hpest->ptype_ == PATH_SEARCH) {
        if ((this->isMyIpInList(p)) || (iph->saddr() == ipAddr_)) { // My Ip is already in list or the packet is returned to the source: drop.
            drop(p, 1, DROP_PATH_ESTABLISHMENT_SEARCH_PACKET_ALREADY_PROCESSED);
            return;
        } else { // This is the first time that the current node sees this request.
            if (this->addMyIpInList(p) == false) { // The queue is full, impossible to add my IP.
                drop(p, 1, DROP_PATH_ESTABLISHMENT_SEARCH_PACKET_HOP_LIST_FULL);
                return;
            } else { // The IP of the current node has been added to the queue.
                this->updateQuality(p);
                if (this->getNumberOfHopToSink() == 1) { // Node directly connected with the sink: create an answer to the path establishment request.
                    this->answerPath(p);
                    Packet::free(p);
                    return;
                } else { // I'm a relay node, forward the packet in BCAST.
                    if (printDebug_ > 10)
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":REPLY_SEARCH:source:" << printIP(iph->saddr()) << "." << endl;
                    ch->next_hop() = UWIP_BROADCAST;
                    ch->prev_hop_  = ipAddr_;
                    iph->daddr()   = UWIP_BROADCAST;
                    number_of_pathestablishment_++;
                    if (trace_)
                        this->tracePacket(p, "FRWD_SRC");
                    sendDown(p, this->getDelay(period_status_));
                    return;
                }
            }
        }
    } else {
        Packet::free(p);
        return;
    }
} /* SunIPRoutingNode::replyPathEstSearch */

/* This function tries to add the IP of the current node at the end of the list
 * of IP in PathEstablishment header. It can do it if there is at least one free block.
 * If the blocks are all full the function drops the packet.
 * Packet p is a path establishment packet.
 * The function returns 1 if it added the IP, 0 otherwise.
 */
const bool SunIPRoutingNode::addMyIpInList(Packet* p) {
    if (STACK_TRACE)
        cout << "> addMyIpInList()" << endl;
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);

    if (hpest->pointer() >= MAX_HOP_NUMBER) { // Impossible to add new hops.
        return false;
    } else {
        // Time to add current node IP at the end of the list.
        hpest->list_of_hops()[hpest->pointer()] = ipAddr_;
        hpest->list_of_hops_length()++;
        hpest->pointer()++;
        return true;
    }
} /* SunIPRoutingNode::addMyIpInList */

/* This function checks if the IP of the current node is already in the list
 * inside a Path Establishment packet header.
 * If yes it returns true, otherwise it returns false.
 * It is used to check if a path is already processed.
 */
const bool SunIPRoutingNode::isMyIpInList(const Packet* p) const {
    if (STACK_TRACE)
        cout << "> isMyIpInList()" << endl;
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);

    for (int i = 0; i <= hpest->list_of_hops_length(); i++) {
        if (hpest->list_of_hops()[i] == ipAddr_)
            return true;
    }
    return false;
} /* SunIPRoutingNode::isMyIpInList */

/* This function updates the quality field in the packet p.
 * The behaviour depends on the metric.
 */
void SunIPRoutingNode::updateQuality(Packet* p){
    if (STACK_TRACE)
        cout << "> updateQuality()" << endl;
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);
    hdr_MPhy* ph            = HDR_MPHY(p);
    if (metrics_ == HOPCOUNT) {
        hpest->quality() += 1;
    } else if (metrics_ ==  SNR) {
        double snr_ = 10*log10(ph->Pr/ph->Pn);
        if (snr_ < hpest->quality()) {
            hpest->quality() = snr_;
            if (this->getNumberOfHopToSink() == 1) { // Check also for the SNR from the sink to the current node.
                if (!this->isZero(snr_to_sink_ - MIN_SNR)) {
                    if (snr_to_sink_ < hpest->quality()) {
                        hpest->quality() = snr_to_sink_;
                    }
                }
            }
        }
    } else if (metrics_ == LESSCONGESTED) {
        if (this->getPacketsLastMinute() == 0) {
            hpest->quality() = hpest->quality() + 0;
        } else if (this->getPacketsLastMinute() > 0){
            hpest->quality() = hpest->quality() + this->getLoad();
        }
    } else {
        cerr << "The metric_ field of SUN was not set up." << endl;
    }
} /* SunIPRoutingNode::updateQuality */
