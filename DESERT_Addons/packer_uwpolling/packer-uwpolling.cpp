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
    uid_poll_Bits(0)
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
    this->init();
    
}

packerUwpolling::~packerUwpolling()
{
    //nothing to do
}

void packerUwpolling::init() 
{
    
    n_bits.clear();
    n_bits.assign(10,0);
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
            cout << "\033[1;37;45m (TX) UWPOLLING::TRIGGER packer hdr \033[0m"      << std::endl;
            printMyHdrFields(p);
        }
    } 
    else if ( ch->ptype() == PT_POLL) 
    {
        hdr_POLL* pollh = HDR_POLL(p);
        

        offset += put(buf, offset, &(pollh->id_), n_bits[ID_POLLED]);
        offset += put(buf, offset, &(pollh->POLL_uid_),n_bits[UID_POLL]);
        
        if (debug_)
        {
            cout << "\033[1;37;45m (TX) UWPOLLING::POLL packer hdr \033[0m"      << std::endl;
            printMyHdrFields(p);
        }
    }
    else if (ch->ptype() == PT_PROBE)
    {
        hdr_PROBE* probeh = HDR_PROBE(p);
        
        offset += put(buf, offset,&(probeh->backoff_time_), n_bits[BACKOFF_TIME]);
        offset += put(buf, offset,&(probeh->ts_), n_bits[TS_BITS]);
        offset += put(buf,offset,&(probeh->n_pkts_),n_bits[N_PKTS]);
        offset += put(buf,offset,&(probeh->id_node_),n_bits[ID_NODE]);
        offset += put(buf,offset,&(probeh->PROBE_uid_),n_bits[UID_PROBE]);
        if (debug_)
        {
            cout << "\033[1;37;45m (TX) UWPOLLING::PROBE packer hdr \033[0m"      << std::endl;
            printMyHdrFields(p);
        }
        
    }
    
    return offset;
}

size_t packerUwpolling::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    // Assumes that the CH packet type has been already unpacked!!!
    
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
            cout << "\033[1;32;40m (RX) UWPOLLING::TRIGGER packer hdr \033[0m"      << std::endl;
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
        
        if (debug_)
        {
            cout << "\033[1;32;40m (RX) UWPOLLING::POLL packer hdr \033[0m"      << std::endl;
            printMyHdrFields(p);
        }
    }
    else if (ch->ptype() == PT_PROBE)
    {
        hdr_PROBE* probeh = HDR_PROBE(p);
        
        memset(&(probeh->backoff_time_),0,sizeof(probeh->backoff_time_));
        offset += get(buf,offset,&(probeh->backoff_time_),n_bits[BACKOFF_TIME]);
        
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
            cout << "\033[1;32;40m (RX) UWPOLLING::PROBE packer hdr \033[0m"      << std::endl;
            printMyHdrFields(p);
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
                << "\033[1;37;41m 3rd field \033[0m, TRIGGER_uid: "   <<  triggerh->TRIGGER_uid_   << std::endl ;
            
    }
    else if ( ch->ptype() == PT_PROBE)
    {
        hdr_PROBE* probeh = HDR_PROBE(p);
                cout << "\033[1;37;41m 1st field \033[0m, backoff_time_: " <<  probeh->backoff_time_ << std::endl
                 << "\033[1;37;41m 2nd field \033[0m, ts_: "   <<  probeh->ts_   << std::endl
                << "\033[1;37;41m 3rd field \033[0m, n_pkts_: "   << (unsigned int) probeh->n_pkts_   << std::endl
                << "\033[1;37;41m 4th field \033[0m, id_node_: "   << (unsigned int) probeh->id_node_   << std::endl
                << "\033[1;37;41m 5th field \033[0m, PROBE_uid: "   << (unsigned int) probeh->PROBE_uid_   << std::endl;
    }
    else if ( ch ->ptype() == PT_POLL)
    {
        hdr_POLL* pollh = HDR_POLL(p);
            cout << "\033[1;37;41m 1st field \033[0m, id_polled_: " <<  pollh->id_ << std::endl;
            cout << "\033[1;37;41m 2nd field \033[0m, POLL_uid: " <<  pollh->POLL_uid_ << std::endl;
    }
        
}

void packerUwpolling::printMyHdrMap()
{
        cout << "\033[1;37;45m Packer Name \033[0m: UWPOLLING \n";
        cout << "** TRIGGER fields:\n" ;
        cout << "\033[1;37;45m Field: t_in \033[0m:"        << n_bits[T_IN]              << " bits\n" ;
        cout << "\033[1;37;45m Field: t_fin \033[0m:"        << n_bits[T_FIN]              << " bits\n" ;
        cout << "\033[1;37;45m Field: TRIGGER_u_id \033[0m:"        << n_bits[UID_TRIGGER]              << " bits\n" ;
        cout << "** PROBE fields:\n" ;
        cout << "\033[1;37;45m Field: backoff_time: \033[0m:"        << n_bits[BACKOFF_TIME]              << " bits\n" ;
        cout << "\033[1;37;45m Field: time_stamp: \033[0m:"        << n_bits[TS_BITS]              << " bits\n" ;
        cout << "\033[1;37;45m Field: n_pkts: \033[0m:"        << n_bits[N_PKTS]              << " bits\n" ;
        cout << "\033[1;37;45m Field: id_node: \033[0m:"        << n_bits[ID_NODE]              << " bits\n" ;
        cout << "\033[1;37;45m Field: PROBE_uid: \033[0m:"        << n_bits[UID_PROBE]              << " bits\n" ;
        cout << "** POLL fields:\n" ;
        cout << "\033[1;37;45m Field: id_polled: \033[0m:"        << n_bits[ID_POLLED]              << " bits\n" ;       
        cout << "\033[1;37;45m Field: POLL u_id: \033[0m:"        << n_bits[UID_POLL]              << " bits\n" ;       
}







