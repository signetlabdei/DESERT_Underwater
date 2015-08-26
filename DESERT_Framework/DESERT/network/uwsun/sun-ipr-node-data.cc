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
 * @file   sun-ipr-node-data.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Provides the implementation of all the methods regarding Data Packets.
 *
 * Provides the implementation of all the methods regarding Data Packets.
 */

#include "sun-ipr-node.h"

extern packet_t PT_SUN_DATA;
extern packet_t PT_SUN_PATH_EST;

/* This function initialize a Data Packet passed as argument with the values
 * contained in the routing table.
 */
void SunIPRoutingNode::initPktDataPacket(Packet* p) {
    if (STACK_TRACE)
        cout << "> initPktDataPacket()" << endl;

    // Common header.
    hdr_cmn* ch       = HDR_CMN(p);
    //ch->uid()       = 0; // Set by the Application above.
    //ch->ptype()     = 0; // Set by the Application above.
    //ch->size()      = 0; // Look Down;
    ch->direction()   = hdr_cmn::DOWN;
    //ch->next_hop()  = 0; // This value must be set by the invoker.
    //ch->prev_hop_     = ipAddr_;
    
    // IP Header.
    hdr_uwip* iph = HDR_UWIP(p);
    iph->daddr()  = sink_associated; // The destination is always the Sink associated.
    //iph->dport() = 0; // Set by the Application above.
    iph->saddr()  = ipAddr_;
    //iph->sport() = 0; // Set by the Application above.
    
    // Path establishment header.
    hdr_sun_data* hdata             = HDR_SUN_DATA(p);
    hdata->list_of_hops_length()    = this->getNumberOfHopToSink() - 1;
    for (int i = 0; i < hop_table_length; i++) {
        hdata->list_of_hops()[i]= hop_table[i];
    }
    hdata->pointer()            = 0; 
    
    ch->size()                  += sizeof(hdr_sun_data);
    ch->timestamp()             = Scheduler::instance().clock();
} /* SunIPRoutingNode::initPktDataPacket */

/* Use this function only if the invoker is a node with hop_to_sink > 1,
 * otherwise: drop.
 * This function is used by a node to forward a data packet to the next hop.
 * All the information to route the packet are contained in the packet.
 */
void SunIPRoutingNode::forwardDataPacket(Packet* p) {
    if (STACK_TRACE)
        cout << "> forwardDataPacket()" << endl;
    hdr_cmn* ch     = HDR_CMN(p);
    hdr_sun_data* hdata = HDR_SUN_DATA(p);
    
    if (this->getNumberOfHopToSink() == 1) { // Useless: nodes with hop count 1 will forward packets directly to the sink
        Packet::free(p);
        return;
    } else if (hdata->list_of_hops_length() <= 0) { // Garbage Packet.
        drop(p, 1, DROP_DATA_HOPS_LENGTH_EQUALS_ZERO);
    } else if (hdata->list_of_hops_length() > 0) { // The current node acts as relay.
        int i_ = hdata->pointer();
        if (hdata->list_of_hops()[i_] == ipAddr_) { // If I'm the right next hop.
            if (this->getNumberOfHopToSink() == 1) { // Send to the sink.
                ch->next_hop() = sink_associated;
                ch->prev_hop_  = ipAddr_;
            } else { // Forward to the next node.
                ch->next_hop() = hdata->list_of_hops()[i_ + 1];
                ch->prev_hop_ = ipAddr_;
                hdata->pointer()++;
            }
            if (printDebug_ > 10)
                cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - P: " << ch->uid() << " - Forward Data Packet to: " << printIP(ch->next_hop()) << "." << endl;
            number_of_datapkt_++;
            number_of_pkt_forwarded_++;
            double delay_tx_ = this->getDelay(period_data_);
            pkt_tx_++;
            if(trace_)
                this->tracePacket(p, "FRWD_DTA");
            sendDown(p, delay_tx_);
        } else {
            Packet::free(p);
        }
    }
} /* SunIPRoutingNode::forwardDataPacket */
