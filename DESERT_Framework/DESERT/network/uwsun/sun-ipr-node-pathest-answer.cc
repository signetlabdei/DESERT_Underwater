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
 * @file   sun-ipr-node-pathest-answer.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Provides the implementation of all the methods regarding Path Establishment Answer Packets.
 *
 * Provides the implementation of all the methods regarding Path Establishment Answer Packets.
 */

#include "sun-ipr-node.h"

extern packet_t PT_SUN_DATA;
extern packet_t PT_SUN_PATH_EST;

/* This function is invoked by a node with hop count = 1.
 * This function creates an Path Establishment Answer packet and, using other functions it sends
 * the packet back.
 */
void SunIPRoutingNode::answerPath(const Packet* p_old) {
    if (STACK_TRACE)
        cout << "> answerPath()" << endl;
    // This node is a node with hop count = 1. Checks anyway for errors.
    // The packet p_old is removed by the method that invokes this one.
    if (this->getNumberOfHopToSink() == 1) {
        Packet* p_answer    = Packet::alloc();
        this->initPktPathEstAnswer(p_answer, p_old);
        hdr_cmn* ch_answer      = HDR_CMN(p_answer);
        hdr_uwip* iph           = HDR_UWIP(p_answer);
        hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p_answer);
        if (printDebug_ > 10)
            cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) <<  " - CP: " << ch_answer->uid() << " - Answer Path sent. Hop Count: " << (hpest->list_of_hops_length() + 1) << " (destination: " << printIP(iph->daddr()) << ")." << endl;
        number_of_pathestablishment_++;
        if (trace_)
            this->tracePacket(p_answer, "SEND_PTH");
        sendDown(p_answer, this->getDelay(period_status_));
        return;
    } else {
        return;
    }
} /* SunIPRoutingNode::answerPath */

/* This function initialize a Path Establishment Answer packet passed
 * as first argument with the values contained in the Path Establishment Request
 * packet passed as second argument.
 */
void SunIPRoutingNode::initPktPathEstAnswer(Packet* new_pkt, const Packet* old_pkt) {
    if (STACK_TRACE)
        cout << "> initPktPathEstAnswer()" << endl;
    // Common header.
    hdr_cmn* ch_old      = HDR_CMN(old_pkt);
    hdr_cmn* ch_new      = HDR_CMN(new_pkt);
    ch_new->uid()        = sunuid_++;
    ch_new->ptype()      = PT_SUN_PATH_EST;
    //ch_new->size()     = 0; // Look Down.
    ch_new->direction()  = hdr_cmn::DOWN;
    //ch_new->next_hop() = 0; // Look Down.
    ch_new->prev_hop_    = ipAddr_;

    // IP Header
    hdr_uwip* iph_old    = HDR_UWIP(old_pkt);
    hdr_uwip* iph_new    = HDR_UWIP(new_pkt);
    iph_new->saddr()   = ipAddr_; // The relay with hop count 1 will set as source it's own IP.
    //iph_new->sport() = 0; // Set by the Application above.
    iph_new->daddr()   = iph_old->saddr(); // The daddr is the IP of the node that sent the path request.
    //iph_new->dport() = 0; // Set by the Application above.
    //iph_new->ttl()   = 0; // Set by IPModule.

    // Path establishment header.
    hdr_sun_path_est* hpest_new      = HDR_SUN_PATH_EST(new_pkt);
    hdr_sun_path_est* hpest_old      = HDR_SUN_PATH_EST(old_pkt);
    hpest_new->sinkAssociated()      = sink_associated;
    hpest_new->ptype()               = PATH_ANSWER;
    hpest_new->list_of_hops_length() = hpest_old->list_of_hops_length();
    hpest_new->pointer()             = hpest_new->list_of_hops_length() - 2; // -1 because the length starts from 1, -1 because the next hop is the previous node in the list 
//    cout << "pointer: " << (int) hpest_new->pointer() << endl;
//    cout << ">-----------begin initPathAnswer--------------<" << endl;
//    cout << "my hop count must be 1 and it is: " << this->getNumberOfHopToSink() << endl;
//    cout << "Node IP: " << printIP(ipAddr_) << endl;
//    cout << "Requesting node: " << printIP(iph_old->saddr()) << endl;
//    cout << "Previous node: " << printIP(ch_old->prev_hop_) << endl;
    for (int i = 0; i < hpest_new->list_of_hops_length(); i++) {
        hpest_new->list_of_hops()[i] = hpest_old->list_of_hops()[i];
//        cout << "hop[" << i << "]: " << printIP(hpest_new->list_of_hops()[i]) << endl;
    }
//    cout << ">------------end initPathAnswer---------------<" << endl;
    hpest_new->quality()             = hpest_old->quality();
    
    ch_new->size()                   += sizeof(hdr_sun_path_est);
//    cout << "list of hops length: " << (int) hpest_old->list_of_hops_length() << endl;
//    cout << "pointer: " << (int) hpest_new->pointer() << endl;
    if (hpest_new->pointer() < 0) {
        ch_new->next_hop()           = ch_old->prev_hop_; //iph_old->saddr();
    } else {
        ch_new->next_hop()           = hpest_new->list_of_hops()[hpest_new->pointer()];
    }
//    cout << "next hop: " << printIP(ch_new->next_hop()) << endl;
    ch_new->timestamp()              = Scheduler::instance().clock();
} /* SunIPRoutingNode::initPktPathEstAnswer */

/* This function receives a Path Establishment Answer Packet and checks
 * if the IP of the current node is in the list.
 * If no it drops the packet, otherwise it sends to the previous node the packet,
 * and adds the information about the route in its the routing table.
 * Returns 0 if it drops the packet, 1 if it sends back the packet.
 */
void SunIPRoutingNode::sendRouteBack(Packet* p) {
    if (STACK_TRACE)
        cout << "> sendRouteBack()" << endl;
    hdr_cmn* ch         = HDR_CMN(p);
    hdr_uwip* iph       = HDR_UWIP(p);
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);

    if (hpest->ptype() == PATH_ANSWER) { // This function processes only PATH_ANSWER packets
        if (hpest->list_of_hops_length() == 0) {
            drop(p, 1, DROP_PATH_ESTABLISHMENT_ANSWER_PACKET_GARBAGE);
            return;
        } else {
            short j_ = hpest->pointer();
            if (hpest->list_of_hops()[j_] == ipAddr_) { // The current node is the right next hop.
                // Update the hop table of the current node only if the node receives a new best path or it doesn't have any path.
                /* The number of hops from the node to the sink are:
                 * list_of_hops_length - 1 + pointer + 1 = list_of_hops_length - pointer
                 */
                if (metrics_ == HOPCOUNT) {
                    if (((hpest->list_of_hops_length() - hpest->pointer()) <= this->getNumberOfHopToSink()) ||
                            (this->getNumberOfHopToSink() == 0)) { //Attention: the pointer is a decreasing value.
                        // Reset of the routing information.
                        this->clearHops();
                        this->setNumberOfHopToSink(0);
                        // Update the routing table of the current node.
                        for (int i = j_ + 1, w = 0; i < hpest->list_of_hops_length(); i++, w++) {
                            hop_table[w] = hpest->list_of_hops()[i];
                        }
                        hop_table_length = hpest->list_of_hops_length() - hpest->pointer() - 1;
                        this->setNumberOfHopToSink(hop_table_length + 1);
                        sink_associated = hpest->sinkAssociated();
                        rmhopTableTmr_.resched(timer_route_validity_);
                        ack_warnings_counter_ = 0;
                        ack_error_state = false;
                    }
                } else if (metrics_ == SNR) {
                    double tmp_ = hpest->quality();
                    if (this->isZero(quality_link) || (quality_link < tmp_) || (this->getNumberOfHopToSink() == 0)) {
                        // Reset of the routing information.
                        this->clearHops();
                        this->setNumberOfHopToSink(0);
                        // Update the routing table of the current node.
                        for (int i = j_ + 1, w = 0; i < hpest->list_of_hops_length(); i++, w++) {
                            hop_table[w] = hpest->list_of_hops()[i];
                        }
                        hop_table_length = hpest->list_of_hops_length() - hpest->pointer() - 1;
                        this->setNumberOfHopToSink(hop_table_length + 1);
                        quality_link = tmp_;
                        sink_associated = hpest->sinkAssociated();
                        rmhopTableTmr_.resched(timer_route_validity_);
                        ack_warnings_counter_ = 0;
                        ack_error_state = false;
                    }
                } else if (metrics_ == LESSCONGESTED) {
                    double tmp_ = hpest->quality() / hpest->list_of_hops_length();
                    if ((!this->isZero(quality_link) && (quality_link > tmp_)) || (this->getNumberOfHopToSink() == 0)) {
                        // Reset of the routing information.
                        this->clearHops();
                        this->setNumberOfHopToSink(0);
                        // Update the routing table of the current node.
                        for (int i = j_ + 1, w = 0; i < hpest->list_of_hops_length(); i++, w++) {
                            hop_table[w] = hpest->list_of_hops()[i];
                        }
                        hop_table_length = hpest->list_of_hops_length() - hpest->pointer() - 1;
                        this->setNumberOfHopToSink(hop_table_length + 1);
                        quality_link = tmp_;
                        sink_associated = hpest->sinkAssociated();
                        rmhopTableTmr_.resched(timer_route_validity_);
                        ack_warnings_counter_ = 0;
                        ack_error_state = false;
                    }
                }
                // Update the information in the Path Establishment Answer packet.
                hpest->pointer()--;
                if (hpest->pointer() < 0) { // The list is over, the next hop will be the destination.
                    ch->next_hop() = iph->daddr();
                    ch->prev_hop_  = ipAddr_;
                } else { // Look in the hop list for the next hop.
                    ch->next_hop() = hpest->list_of_hops()[hpest->pointer()];
                    ch->prev_hop_  = ipAddr_;
                }
                if (printDebug_ > 10)
                    cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - Send Answer Path reply. Next hop: " << printIP(ch->next_hop()) << " (destination: " << printIP(iph->saddr()) << ")." << endl;
                number_of_pathestablishment_++;
                if (trace_)
                    this->tracePacket(p, "FRWD_PTH");
                sendDown(p, this->getDelay(period_status_));
                return;
            } else {
                drop(p, 1, DROP_PATH_ESTABLISHMENT_ANSWER_PACKET_GARBAGE);
                return;
            }
        }
    } else {
        Packet::free(p);
        return;
    }
} /* SunIPRoutingNode::sendRouteBack */

/* This function is invoked by the node that requested a Search Path.
 * The function accepts in input a packet path establishment with a full queue
 * of paths. Compare it with the previous saved, if it exists, and decide which
 * is the better one.
 * It returns the current value of num_hop_to_sink.
 */
const int& SunIPRoutingNode::evaluatePath(const Packet* p) {
    if (STACK_TRACE)
        cout << "> evaluatePath()" << endl;
    hdr_cmn* ch             = HDR_CMN(p);
    hdr_uwip* iph           = HDR_UWIP(p);
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);
    
    if (metrics_ == HOPCOUNT) {
        int tmp_metric_ = hpest->list_of_hops_length();
        if (tmp_metric_ > 0) { // There is at least one hop in the path.
            if (((this->getNumberOfHopToSink() == 0 ) || (this->getNumberOfHopToSink() > tmp_metric_ + 1)) && this->getNumberOfHopToSink() != 1) { // New best route. +1 because of the final hop.
                this->clearHops();
                this->setNumberOfHopToSink(0);
//                cout << "----> " << printIP(ipAddr_) << endl;
                for (int i = 0; i < hpest->list_of_hops_length(); i++) {
                    hop_table[i] = hpest->list_of_hops()[i];
//                    cout << "hop_table[i].ip_ = " << printIP(hop_table[i]) << endl;
                }
                quality_link = tmp_metric_ + 1;
                sink_associated = hpest->sinkAssociated();
                hop_table_length = hpest->list_of_hops_length();
                this->setNumberOfHopToSink(hop_table_length + 1); // The value to reach the node connected to the sink + 1 (node-node with hc 1-sink)
                if (printDebug_ > 5)
                    cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - New best Route. The new Hop Count is: " << this->getNumberOfHopToSink() << " (final hop: " << printIP(iph->saddr()) << ")." << endl;
                rmhopTableTmr_.resched(timer_route_validity_);
                ack_warnings_counter_ = 0;
                ack_error_state = false;
            }
            return this->getNumberOfHopToSink();
        } else {
            return this->getNumberOfHopToSink();
        }
    } else if (metrics_ == SNR) {
        double tmp_metric_ = hpest->quality();
        if (hpest->list_of_hops_length() > 0) { // There is at least one hop in the path.
            if (this->isZero(quality_link) || (quality_link < tmp_metric_) || (this->getNumberOfHopToSink() == 0)) {
                this->clearHops();
                this->setNumberOfHopToSink(0);
                for (int i = 0; i < hpest->list_of_hops_length(); i++) {
                    hop_table[i] = hpest->list_of_hops()[i];
//                    cout << "hop[" << i << "]: " << printIP(hop_table[i].ip_) << endl;
                }
                quality_link = tmp_metric_;
                sink_associated = hpest->sinkAssociated();
                hop_table_length = hpest->list_of_hops_length();
                this->setNumberOfHopToSink(hop_table_length + 1); // The value to reach the node connected to the sink + 1 (node-node with hc 1-sink)
                if (printDebug_ > 5)
                    cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - New best Route. The new Hop Count is: " << this->getNumberOfHopToSink() << ", with an SNR of: " << quality_link << "dB (final hop: " << printIP(iph->saddr()) << ")." << endl;
                rmhopTableTmr_.resched(timer_route_validity_);
                ack_warnings_counter_ = 0;
                ack_error_state = false;
            }
            return this->getNumberOfHopToSink();
        } else {
            return this->getNumberOfHopToSink();
        }
    } else if (metrics_ == LESSCONGESTED) {
        double tmp_metric_ = hpest->quality() / hpest->list_of_hops_length();
        if (hpest->list_of_hops_length() > 0) { // There is at least one hop in the path.
            if (this->isZero(quality_link) || (quality_link > tmp_metric_) || (this->getNumberOfHopToSink() == 0)) {
                this->clearHops();
                this->setNumberOfHopToSink(0);
                for (int i = 0; i < hpest->list_of_hops_length(); i++) {
                    hop_table[i] = hpest->list_of_hops()[i];
//                    cout << "hop[" << i << "]: " << printIP(hop_table[i].ip_) << endl;
                }
                quality_link = tmp_metric_;
                sink_associated = hpest->sinkAssociated();
                hop_table_length = hpest->list_of_hops_length();
                this->setNumberOfHopToSink(hop_table_length + 1); // The value to reach the node connected to the sink + 1 (node-node with hc 1-sink)
                if (printDebug_ > 5)
                    cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - CP: " << ch->uid() << " - New best Route. The new Hop Count is: " << this->getNumberOfHopToSink() << ", with a load of: " << quality_link << " (final hop: " << printIP(iph->saddr()) << ")." << endl;
                rmhopTableTmr_.resched(timer_route_validity_);
                ack_warnings_counter_ = 0;
                ack_error_state = false;
            }
            return this->getNumberOfHopToSink();
        } else {
            return this->getNumberOfHopToSink();
        }
    } else {
        exit(1);
    }
} /* SunIPRoutingNode::evaluatePath */
