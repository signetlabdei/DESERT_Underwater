//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file packer-uwpolling.cpp
 * @author Fderico Favaro
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the ns2 packet of uwpolling into a bit stream, and vice-versa.
 */

 #include "packer-uwpolling.h"

 static class PackerUwpollingClass : public TclClass {
public:

    PackerUwpollingClass() : TclClass("NS2/MAC/Uwpolling/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packerUwpolling());
    }
} class_module_packerUwpolling;

packerUwpolling::packerUwpolling() : packer(false),
    t_in_Bits(0),
    t_fin_Bits(0),
    uid_trigger_Bits(0),
    id_polled_Bits(0),
    backoff_time_Bits(0),
    ts_Bits(0),
    n_pkts_Bits(0),
    uid_probe_Bits(0),
    id_node_Bits(0),
    uid_poll_Bits(0),
    poll_time_Bits(0),
    uid_sink_Bits(0),
    uid_probe_sink_Bits(0),
    uid_ack_Bits(0),
    uid_packet_Bits(0),
    uid_last_packet_Bits(0),
    uid_acks_Bits(0),
    ack_array_size_Bits(0),
    ack_array_el_Bits(0),
    ack_array_size(0),
    sink_mac(0)
{
    bind("t_in_Bits", (int*) &t_in_Bits);
    bind("t_fin_Bits", (int*) &t_fin_Bits);
    bind("uid_TRIGGER_Bits", (int*) &uid_trigger_Bits);
    bind("id_polled_Bits", (int*) &id_polled_Bits);
    bind("backoff_time_Bits", (int*) &backoff_time_Bits);
    bind("ts_Bits", (int*) &ts_Bits);
    bind("n_pkts_Bits", (int*) &n_pkts_Bits);
    bind("uid_PROBE_Bits", (int*) &uid_probe_Bits);
    bind("id_node_Bits", (int*) &id_node_Bits);
    bind("uid_POLL_Bits", (int*) &uid_poll_Bits);
    bind("poll_time_Bits", (int*) &poll_time_Bits);
    bind("uid_sink_Bits", (int*) &uid_sink_Bits);
    bind("uid_probe_sink_Bits", (int*) &uid_probe_sink_Bits);
    bind("uid_ack_Bits", (int*) &uid_ack_Bits);
    bind("uid_packet_Bits", (int*) &uid_packet_Bits);
    bind("uid_last_packet_Bits", (int*) &uid_last_packet_Bits);
    bind("uid_acks_Bits", (int*) &uid_acks_Bits);
    bind("ack_array_size_Bits", (int*) &ack_array_size_Bits);
    bind("ack_array_el_Bits", (int*) &ack_array_el_Bits);
    bind("ack_array_size", (int*) &ack_array_size);
    bind("sink_mac_", (int*) &sink_mac);
    this->init();
    
}

packerUwpolling::~packerUwpolling()
{
    //nothing to do
}

void packerUwpolling::init() 
{
	
    n_bits.clear();
    n_bits.assign(LAST_ELEM,0);
    n_bits[T_IN] = t_in_Bits;
    n_bits[T_FIN] = t_fin_Bits;
    n_bits[UID_TRIGGER] = uid_trigger_Bits;
    n_bits[ID_POLLED] = id_polled_Bits;
    n_bits[BACKOFF_TIME] = backoff_time_Bits;
    n_bits[TS_BITS] = ts_Bits;
    n_bits[N_PKTS] = n_pkts_Bits;
    n_bits[UID_PROBE] = uid_probe_Bits;
    n_bits[ID_NODE] = id_node_Bits;
    n_bits[UID_POLL] = uid_poll_Bits;
    n_bits[POLL_TIME] = poll_time_Bits;
    n_bits[UID_SINK] = uid_sink_Bits;
    n_bits[UID_PROBE_SINK] = uid_probe_sink_Bits;
    n_bits[UID_ACK] = uid_ack_Bits;
    n_bits[UID_PACKET] = uid_packet_Bits;
    n_bits[UID_LAST_PACKET] = uid_last_packet_Bits;
    n_bits[UID_ACKS] = uid_acks_Bits;
    n_bits[ACK_ARRAY_SIZE] = ack_array_size_Bits;
    n_bits[ACK_ELEM_BITS] = ack_array_el_Bits;
}


size_t packerUwpolling::packMyHdr(Packet* p, unsigned char* buf, size_t offset)
{
    hdr_cmn* ch = HDR_CMN(p);
    
    if (ch->ptype() == PT_TRIGGER)
    {
        hdr_TRIGGER* triggerh = HDR_TRIGGER(p);
        
        offset += put(buf, offset, &(triggerh->t_in_), n_bits[T_IN]);
        offset += put(buf, offset, &(triggerh->t_fin_),n_bits[T_FIN]);
        offset += put(buf, offset, &(triggerh->TRIGGER_uid_),n_bits[UID_TRIGGER]);
        
        if (debug_)
        {
            cout << "\033[1;37;45m (TX) UWPOLLING::TRIGGER packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    } 
    else if ( ch->ptype() == PT_POLL) 
    {
        hdr_POLL* pollh = HDR_POLL(p);
        

        offset += put(buf, offset, &(pollh->id_), n_bits[ID_POLLED]);
        offset += put(buf, offset, &(pollh->POLL_uid_),n_bits[UID_POLL]);
        offset += put(buf, offset, &(pollh->POLL_time_),n_bits[POLL_TIME]);
        
        if (debug_)
        {
            cout << "\033[1;37;45m (TX) UWPOLLING::POLL packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }
    else if (ch->ptype() == PT_PROBE)
    {
        hdr_PROBE* probeh = HDR_PROBE(p);
        
        //offset += put(buf, offset,&(probeh->backoff_time_), n_bits[BACKOFF_TIME]);
        offset += put(buf, offset,&(probeh->ts_), n_bits[TS_BITS]);
        offset += put(buf,offset,&(probeh->n_pkts_),n_bits[N_PKTS]);
        offset += put(buf,offset,&(probeh->id_node_),n_bits[ID_NODE]);
        offset += put(buf,offset,&(probeh->PROBE_uid_),n_bits[UID_PROBE]);
        if (debug_)
        {
            cout << "\033[1;37;45m (TX) UWPOLLING::PROBE packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
        
    }
    else if  (ch->ptype() == PT_PROBE_SINK)
    {
        hdr_PROBE_SINK* probes_hdr = HDR_PROBE_SINK(p);

        offset += put(buf, offset, &(probes_hdr->id_sink_), n_bits[UID_SINK]);
        offset += put(buf, offset, &(probes_hdr->PROBE_uid_), n_bits[UID_PROBE_SINK]);
        offset += put(buf, offset, &(probes_hdr->id_ack_), n_bits[UID_ACK]);
		if (debug_)
        {
            cout << "\033[1;37;45m (TX) UWPOLLING::PROBE_SINK packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }
    else if (ch->ptype() == PT_ACK_SINK) {
        hdr_ACK_SINK *ackh = HDR_ACK_SINK(p);
        uint16_t uid_acks_array[ack_array_size];
        std::vector<uint16_t>::iterator ack_id_it;
        size_t fix_array_it = 0;
        for (ack_id_it = ackh->id_ack_.begin(); ack_id_it != ackh->id_ack_.end(); ++ack_id_it) {
            uid_acks_array[fix_array_it] = (uint16_t)*ack_id_it;
            fix_array_it++;
            if ( fix_array_it >= (ack_array_size+1)) {
                break;
            }
        }
           
        size_t actual_size = fix_array_it;
        offset += put(buf, offset, &(actual_size), n_bits[ACK_ARRAY_SIZE]);
        fix_array_it = 0;
        for (fix_array_it = 0; fix_array_it < actual_size; fix_array_it++) {
            offset += put(buf, offset, &(uid_acks_array[fix_array_it]), n_bits[ACK_ELEM_BITS]);
        }
		if (debug_)
        {
            cout << "\033[1;37;45m (TX) UWPOLLING::ACK_SINK packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    } 
	else 
	{
        hdr_AUV_MULE *auv_mule_hdr = HDR_AUV_MULE(p);
        hdr_mac *mach = HDR_MAC(p);

        if (mach->macDA() == sink_mac) {
            offset += put(buf, offset, &(auv_mule_hdr->pkt_uid_), n_bits[UID_PACKET]);
            offset += put(buf, offset, &(auv_mule_hdr->last_pkt_uid_), n_bits[UID_LAST_PACKET]);
            if (debug_)
            {
                cout << "\033[1;37;45m (TX) UWPOLLING::AUV_MULE packer hdr \033[0m" << std::endl;
                printMyHdrFields(p);
            }    

        }
    }
    return offset;
}

size_t packerUwpolling::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    
    if ( ch->ptype() == PT_TRIGGER )
    {
        hdr_TRIGGER* triggerh = HDR_TRIGGER(p);
        
        memset(&(triggerh->t_in_),0,sizeof(triggerh->t_in_));
        offset += get(buf,offset,&(triggerh->t_in_),n_bits[T_IN]);
        
        memset(&(triggerh->t_fin_),0,sizeof(triggerh->t_fin_));
        offset += get(buf,offset,&(triggerh->t_fin_),n_bits[T_FIN]);
        
        memset(&(triggerh->TRIGGER_uid_),0,sizeof(triggerh->TRIGGER_uid_));
        offset += get(buf,offset,&(triggerh->TRIGGER_uid_),n_bits[UID_TRIGGER]);
        
        if (debug_)
        {
            cout << "\033[1;32;40m (RX) UWPOLLING::TRIGGER packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
     
    } 
    else if (ch->ptype() == PT_POLL)
    {
        hdr_POLL* pollh = HDR_POLL(p);
        
        memset(&(pollh->id_),0,sizeof(pollh->id_));
        offset += get(buf,offset,&(pollh->id_),n_bits[ID_NODE]);
        
        memset(&(pollh->POLL_uid_),0,sizeof(pollh->POLL_uid_));
        offset += get(buf,offset,&(pollh->POLL_uid_),n_bits[UID_POLL]);

        memset(&(pollh->POLL_time_),0,sizeof(pollh->POLL_time_));
        offset += get(buf,offset,&(pollh->POLL_time_),n_bits[POLL_TIME]);
        
        if (debug_)
        {
            cout << "\033[1;32;40m (RX) UWPOLLING::POLL packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }
    else if (ch->ptype() == PT_PROBE)
    {
        hdr_PROBE* probeh = HDR_PROBE(p);
        
        //memset(&(probeh->backoff_time_),0,sizeof(probeh->backoff_time_));
        //offset += get(buf,offset,&(probeh->backoff_time_),n_bits[BACKOFF_TIME]);
        
        memset(&(probeh->ts_),0,sizeof(probeh->ts_));
        offset += get(buf,offset,&(probeh->ts_),n_bits[TS_BITS]);
        
        memset(&(probeh->n_pkts_),0,sizeof(probeh->n_pkts_));
        offset += get(buf,offset,&(probeh->n_pkts_),n_bits[N_PKTS]);
        
        memset(&(probeh->id_node_),0,sizeof(probeh->id_node_));
        offset += get(buf,offset,&(probeh->id_node_),n_bits[ID_NODE]);
        
        memset(&(probeh->PROBE_uid_),0,sizeof(probeh->PROBE_uid_));
        offset += get(buf,offset,&(probeh->PROBE_uid_),n_bits[UID_PROBE]);
        
        if (debug_)
        {
            cout << "\033[1;32;40m (RX) UWPOLLING::PROBE packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
      
    }
    else if (ch->ptype() == PT_PROBE_SINK)
    {
        hdr_PROBE_SINK* probes_hdr = HDR_PROBE_SINK(p);

        memset(&(probes_hdr->id_sink_),0,sizeof(probes_hdr->id_sink_));
        offset += get(buf, offset, &(probes_hdr->id_sink_), n_bits[UID_SINK]);

        memset(&(probes_hdr->PROBE_uid_),0,sizeof(probes_hdr->PROBE_uid_));
        offset += get(buf, offset, &(probes_hdr->PROBE_uid_), n_bits[UID_PROBE_SINK]);

        memset(&(probes_hdr->id_ack_),0,sizeof(probes_hdr->id_ack_));
        offset += get(buf, offset, &(probes_hdr->id_ack_), n_bits[UID_ACK]);

        if (debug_)
        {
            cout << "\033[1;32;40m (RX) UWPOLLING::PROBE_SINK packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    }
    else if (ch->ptype() == PT_ACK_SINK)
    {
        hdr_ACK_SINK *ackh = HDR_ACK_SINK(p);
        int array_size = 0;
        uint16_t app;
        offset += get(buf, offset, &(array_size), n_bits[ACK_ARRAY_SIZE]);
        for (int i=0; i < array_size; i++) {
            app = 0;
            offset += get(buf, offset, &(app), n_bits[ACK_ELEM_BITS]);
            ackh->id_ack_.push_back(app);
        }

        if (debug_)
        {
            cout << "\033[1;32;40m (RX) UWPOLLING::ACK_SINK packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }
    else
    {
		hdr_AUV_MULE *auv_mule_hdr = HDR_AUV_MULE(p);
        hdr_mac *mach = HDR_MAC(p);
        if (mach->macDA() == sink_mac) {

            memset(&(auv_mule_hdr->pkt_uid_),0,sizeof(auv_mule_hdr->pkt_uid_));
            offset += get(buf, offset, &(auv_mule_hdr->pkt_uid_), n_bits[UID_PACKET]);
    
            memset(&(auv_mule_hdr->last_pkt_uid_),0,sizeof(auv_mule_hdr->last_pkt_uid_));
            offset += get(buf, offset, &(auv_mule_hdr->last_pkt_uid_), n_bits[UID_LAST_PACKET]);

            if (debug_)
            {
                cout << "\033[1;32;40m (RX) UWPOLLING::AUV_MULE packer hdr \033[0m" << std::endl;
                printMyHdrFields(p);
            }
        }
    }

    return offset;

}

void packerUwpolling::printMyHdrFields(Packet* p)
{
    hdr_cmn* ch = HDR_CMN(p);
    
    if (ch->ptype() == PT_TRIGGER)
    {
        hdr_TRIGGER* triggerh = HDR_TRIGGER(p);
        cout << "\033[1;37;41m 1st field \033[0m, t_in_: " <<  triggerh->t_in_ << std::endl
                << "\033[1;37;41m 2nd field \033[0m, t_fin_: "   <<  triggerh->t_fin_   << std::endl
                << "\033[1;37;41m 3rd field \033[0m, TRIGGER_uid: "   <<  triggerh->TRIGGER_uid_   << std::endl;
            
    }
    else if ( ch->ptype() == PT_PROBE)
    {
        hdr_PROBE* probeh = HDR_PROBE(p);
        cout << "\033[1;37;41m 2nd field \033[0m, ts_: "   <<  probeh->ts_   << std::endl
                << "\033[1;37;41m 3rd field \033[0m, n_pkts_: "   << (unsigned int) probeh->n_pkts_   << std::endl
                << "\033[1;37;41m 4th field \033[0m, id_node_: "   << (unsigned int) probeh->id_node_   << std::endl
                << "\033[1;37;41m 5th field \033[0m, PROBE_uid: "   << (unsigned int) probeh->PROBE_uid_   << std::endl;
    }
    else if ( ch ->ptype() == PT_POLL)
    {
        hdr_POLL* pollh = HDR_POLL(p);
        cout << "\033[1;37;41m 1st field \033[0m, id_polled_: " <<  pollh->id_ << std::endl
                << "\033[1;37;41m 2nd field \033[0m, POLL_uid: " <<  pollh->POLL_uid_ << std::endl
                << "\033[1;37;41m 3rd field \033[0m, POLL_time_: " <<  pollh->POLL_time_ << std::endl;
    }
    else if ( ch->ptype() == PT_PROBE_SINK)
    {
        hdr_PROBE_SINK* probe_sink_hdr = HDR_PROBE_SINK(p);
        cout << "\033[1;37;41m 1st field \033[0m, id_sink: " <<  probe_sink_hdr->id_sink_ << std::endl
                << "\033[1;37;41m 2nd field \033[0m, PROBE_uid_: " <<  probe_sink_hdr->PROBE_uid_ << std::endl
                << "\033[1;37;41m 3rd field \033[0m, id_ack_: " <<  probe_sink_hdr->id_ack_ << std::endl;
    }
    else if ( ch->ptype() == PT_ACK_SINK)
    {
        hdr_ACK_SINK* ackh = HDR_ACK_SINK(p);
        std::vector<uint16_t>::iterator ack_id_it;
        uint count = 0;
        for (ack_id_it = ackh->id_ack_.begin(); ack_id_it != ackh->id_ack_.end(); ++ack_id_it) {
            count++;
            cout << "\033[1;37;41m 1st field \033[0m, ack_id " << count << " : " 
                    << (uint16_t)*ack_id_it << std::endl;
        }	 
    }
    else// ( ch->ptype() == PT_AUV_MULE)
    {
        hdr_AUV_MULE* auv_mule_hdr = HDR_AUV_MULE(p);
        cout << "\033[1;37;41m 1st field \033[0m, pkt_uid_: " <<  auv_mule_hdr->pkt_uid_ << std::endl
                << "\033[1;37;41m 2nd field \033[0m, last_pkt_uid_: " <<  auv_mule_hdr->last_pkt_uid_ << std::endl;
    }
}

void packerUwpolling::printMyHdrMap()
{
    cout << "\033[0;46;30m Packer Name \033[0m: UWCBR" << std::endl;
    cout << "** TRIGGER header fields" << std::endl;
    cout << "\033[0;46;30m minimum backoff: \033[0m" << t_in_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m maximum backoff: \033[0m" << t_fin_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m trigger packet unique id: \033[0m" << uid_trigger_Bits << " bits" << std::endl;
    
    cout << "** POLL header fields" << std::endl;
    cout << "\033[0;46;30m polled node id: \033[0m" << id_polled_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m poll packet unique id: \033[0m" << uid_poll_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m poll time: \033[0m" << poll_time_Bits << " bits" << std::endl;

    cout << "** PROBE header fields" << std::endl;
    cout << "\033[0;46;30m timestemp: \033[0m" << ts_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m number of packets: \033[0m" << n_pkts_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m node id: \033[0m" << id_node_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m probe packets unique id: \033[0m" << uid_probe_Bits << " bits" << std::endl;
	
    cout << "** PROBE SINK header fields" << std::endl;
    cout << "\033[0;46;30m sink node id: \033[0m" << uid_sink_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m sink probe packets unique id: \033[0m" << uid_probe_sink_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m probe ack id: \033[0m" << uid_ack_Bits << " bits" << std::endl;

    cout << "** AUV header fields" << std::endl;
    cout << "\033[0;46;30m data packet unique id: \033[0m" << uid_packet_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m last tx packet unique id: \033[0m" << uid_last_packet_Bits << " bits" << std::endl;

    cout << "** ACK SINK header fields" << std::endl;
    cout << "\033[0;46;30m max number of ACKs: \033[0m" << ack_array_size_Bits << " bits" << std::endl;
    cout << "\033[0;46;30m ack id: \033[0m" << ack_array_el_Bits << " bits per element" << std::endl;
}







