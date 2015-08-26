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
 * @file   uwflooding.cc
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Implements UwFlooding class.
 * 
 */

#include "uwflooding.h"

extern packet_t PT_UWFLOODING;

int hdr_uwflooding::offset_ = 0;     /**< Offset used to access in <i>hdr_uwflooding</i> packets header. */

/**
 * Adds the module for SunIPRoutingSink in ns2.
 */
static class UwFloodingModuleClass : public TclClass {
public:

    UwFloodingModuleClass() : TclClass("Module/UW/FLOODING") {
    }

    TclObject* create(int, const char*const*) {
        return (new UwFlooding());
    }
} class_module_uwflooding;

/**
 * Adds the header for <i>hdr_uwflooding</i> packets in ns2.
 */
static class UwFloodingPktClass : public PacketHeaderClass {
public:

    UwFloodingPktClass() : PacketHeaderClass("PacketHeader/FLOODING", sizeof(hdr_uwflooding)) {
        this->bind();
        bind_offset(&hdr_uwflooding::offset_);
    }
} class_uwflooding_pkt;

UwFlooding::UwFlooding()
:
ipAddr_(0),
ttl_(10),
maximum_cache_time_(60),
optimize_(1),
packets_forwarded_(0),
trace_path_(false),
trace_file_path_name_((char *) "trace")
{ // Binding to TCL variables.
    bind("ttl_", &ttl_);
    bind("maximum_cache_time_", &maximum_cache_time_);
    bind("optimize_", &optimize_);
} /* UwFlooding::UwFlooding */

UwFlooding::~UwFlooding() {
} /* UwFlooding::~UwFlooding */

int UwFlooding::recvSyncClMsg(ClMessage* m) {
    return Module::recvSyncClMsg(m);
} /* UwFlooding::recvSyncClMsg */

int UwFlooding::recvAsyncClMsg(ClMessage* m) {
    return Module::recvAsyncClMsg(m);
} /* UwFlooding::recvAsyncClMsg */

int UwFlooding::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    
    if (argc == 2) {
        if (strcasecmp(argv[1], "getpacketsforwarded") == 0) {
            tcl.resultf("%lu", packets_forwarded_);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getfloodingheadersize") == 0) {
            tcl.resultf("%d", sizeof(hdr_uwflooding));
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "addr") == 0) {
            //ipAddr_ = str2addr((char *) argv[2]);
            ipAddr_ = static_cast<uint8_t>(atoi(argv[2]));
            if (ipAddr_ == 0) {
                //fprintf(stderr, "0.0.0.0 is not a valid IP address");
                fprintf(stderr, "0 is not a valid IP address");
                return TCL_ERROR;
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "trace") == 0) {
            string tmp_ = ((char *) argv[2]);
            trace_file_path_name_ = new char[tmp_.length() + 1];
            strcpy(trace_file_path_name_, tmp_.c_str());
            if (trace_file_path_name_ == NULL) {
                fprintf(stderr, "Empty string for the trace file name");
                return TCL_ERROR;
            }
            trace_path_ = true;
            remove(trace_file_path_name_);
            trace_file_path_.open(trace_file_path_name_);
            trace_file_path_.close();
            return TCL_OK;
        }
    }
    return Module::command(argc, argv);
} /* UwFlooding::command */

void UwFlooding::recv(Packet *p) {
    hdr_cmn*  ch        = HDR_CMN(p);
    hdr_uwip* iph       = HDR_UWIP(p);
    hdr_uwflooding* flh = HDR_UWFLOODING(p);
    
    if (!ch->error()) {
        if (ch->direction() == hdr_cmn::UP) {
            if(trace_path_)
                this->writePathInTrace(p, "RECV_DTA");
            if (iph->daddr() == 0) {
                std::cerr << "Destination address not set." << std::endl;
                if(trace_path_)
                    this->writePathInTrace(p, "FREE_DTA");
                Packet::free(p);
                return;
            } else if (iph->daddr() == ipAddr_) {
                flh->ttl()--;
                if(trace_path_)
                    this->writePathInTrace(p, "SDUP_DTA");
                sendUp(p);
                return;
            } else if (iph->saddr() == ipAddr_) {
//                cerr << "I am the source: free." << endl;
                if(trace_path_)
                    this->writePathInTrace(p, "FREE_DTA");
                Packet::free(p);
                return;
            } else if (iph->daddr() == UWIP_BROADCAST) {
                // sendUp always: the destination is in broadcast.
                ch->size() -= sizeof(hdr_uwflooding);
                if(trace_path_)
                    this->writePathInTrace(p, "SDUP_DTA");
                sendUp(p->copy());
                
                // SendDown
                ch->direction() = hdr_cmn::DOWN;
                ch->prev_hop_   = ipAddr_;
                ch->next_hop()  = UWIP_BROADCAST;
                flh->ttl()--;
                ch->size() += sizeof(hdr_uwflooding);
                if (flh->ttl() <= 0) {
                    if(trace_path_)
                        this->writePathInTrace(p, "DROP_TTL");
                    drop(p, 1, TTL_EQUALS_TO_ZERO);
                    return;
                } else {
                    if (optimize_) {
                        map_forwarded_packets::iterator it2 = my_forwarded_packets_.find(iph->saddr());
                        if (it2 != my_forwarded_packets_.end()) {
                            map_packets::iterator it3 = it2->second.find(ch->uid());
                            
                            if (it3 == it2->second.end()) { // Known source and new packet -> add it to the map and forward.
                                it2->second.insert(std::pair<uint16_t, double>(ch->uid(), ch->timestamp()));
                                packets_forwarded_++;
                                if(trace_path_) 
                                    this->writePathInTrace(p, "FRWD_DTA");
                                sendDown(p);
                                return;
                            } else if (Scheduler::instance().clock() - it3->second > maximum_cache_time_) { // Packet already processed by not valid maximum cache timer -> update the cache time and forward.
                                it3->second = Scheduler::instance().clock();
                                packets_forwarded_++;
                                if(trace_path_) 
                                    this->writePathInTrace(p, "FRWD_DTA");
                                sendDown(p);
                                return;
                            } else {
                                if(trace_path_)
                                    this->writePathInTrace(p, "FREE_DTA");
                                Packet::free(p);
                                return;
                            }
                        } else {
                            std::map<uint16_t, double> tmp_map;
                            tmp_map.insert(std::pair<uint16_t, double>(ch->uid(), Scheduler::instance().clock()));
                            my_forwarded_packets_.insert(std::pair<uint8_t, map_packets>(iph->saddr(), tmp_map));
                            packets_forwarded_++;
                            if(trace_path_) 
                                this->writePathInTrace(p, "FRWD_DTA");
                            sendDown(p);
                            return;
                        }
                    } else {
                        packets_forwarded_++;
                        if(trace_path_) 
                            this->writePathInTrace(p, "FRWD_DTA");
                        sendDown(p);
                        return;
                    }
                }
            } else if (iph->daddr() != ipAddr_) {
                // SendDown
                ch->direction() = hdr_cmn::DOWN;
                ch->prev_hop_   = ipAddr_;
                ch->next_hop()  = UWIP_BROADCAST;
                flh->ttl()--;
                if (flh->ttl() <= 0) {
                    if(trace_path_)
                        this->writePathInTrace(p, "DROP_TTL");
                    drop(p, 1, TTL_EQUALS_TO_ZERO);
                    return;
                } else {
                    if (optimize_) {
                        map_forwarded_packets::iterator it2 = my_forwarded_packets_.find(iph->saddr());
                        if (it2 != my_forwarded_packets_.end()) {
                            map_packets::iterator it3 = it2->second.find(ch->uid());
                            
                            if (it3 == it2->second.end()) { // Known source and new packet -> add it to the map and forward.
                                it2->second.insert(std::pair<uint16_t, double>(ch->uid(), ch->timestamp()));
                                packets_forwarded_++;
                                if(trace_path_) 
                                    this->writePathInTrace(p, "FRWD_DTA");
                                sendDown(p);
                                return;
                            } else if (Scheduler::instance().clock() - it3->second > maximum_cache_time_) { // Packet already processed by not valid maximum cache timer -> update the cache time and forward.
                                it3->second = Scheduler::instance().clock();
                                packets_forwarded_++;
                                if(trace_path_) 
                                    this->writePathInTrace(p, "FRWD_DTA");
                                sendDown(p);
                                return;
                            } else {
                                if(trace_path_) 
                                    this->writePathInTrace(p, "FREE_DTA");
                                Packet::free(p);
                                return;
                            }
                        } else {
                            std::map<uint16_t, double> tmp_map;
                            tmp_map.insert(std::pair<uint16_t, double>(ch->uid(), Scheduler::instance().clock()));
                            my_forwarded_packets_.insert(std::pair<uint8_t, map_packets>(iph->saddr(), tmp_map));
                            packets_forwarded_++;
                            if(trace_path_) 
                                this->writePathInTrace(p, "FRWD_DTA");
                            sendDown(p);
                            return;
                        }
                    } else {
                        if(trace_path_) 
                            this->writePathInTrace(p, "FRWD_DTA");
                        sendDown(p);
                        return;
                    }
                }
            } else {
                cerr << "State machine ERROR." << endl;
                if(trace_path_) 
                    this->writePathInTrace(p, "FREE_DTA");
                Packet::free(p);
                return;
            }
        } else if (ch->direction() == hdr_cmn::DOWN) {
            if(trace_path_)
                this->writePathInTrace(p, "RECV_DTA");
            if (iph->daddr() == 0) {
                std::cerr << "Destination address equals to 0." << std::endl;
                if(trace_path_) 
                    this->writePathInTrace(p, "FREE_DTA");
                Packet::free(p);
                return;
            } if (iph->daddr() == ipAddr_) {
                if(trace_path_) 
                    this->writePathInTrace(p, "SDUP_DTA");
                sendUp(p);
                return;
            } else { //iph->daddr() != ipAddr_
                ch->prev_hop_  = ipAddr_;
                ch->next_hop() = UWIP_BROADCAST;
                ch->size()     += sizeof(hdr_uwflooding);
                flh->ttl()     = ttl_;
		if(trace_path_) 
		    this->writePathInTrace(p, "FRWD_DTA");
                sendDown(p);
                return;
            }
        } else {
            cerr << "Direction different from UP or DOWN." << endl;
            if(trace_path_) 
                this->writePathInTrace(p, "FREE_DTA");
            Packet::free(p);
            return;
        }
    } else {
        if(trace_path_) 
            this->writePathInTrace(p, "FREE_DTA");
        Packet::free(p);
        return;
    }
} /* UwFlooding::recv */

void UwFlooding::writePathInTrace(const Packet* p, const string& _info) {
    hdr_uwip* iph           = HDR_UWIP(p);
    hdr_cmn* ch             = HDR_CMN(p);
    hdr_uwflooding* flh     = HDR_UWFLOODING(p);

    trace_file_path_.open(trace_file_path_name_, fstream::app);
    osstream_.clear();
    osstream_.str("");
    osstream_ << _info;
    osstream_ << '\t';
    osstream_ << Scheduler::instance().clock();
    osstream_ << '\t';
    osstream_ << static_cast<uint32_t>(ch->uid() & 0x0000ffff);
    osstream_ << '\t';
    osstream_ << static_cast<uint32_t>(flh->ttl());
    osstream_ << '\t';
    osstream_ << static_cast<uint32_t>(ch->prev_hop_ & 0x000000ff);
    osstream_ << '\t';
    osstream_ << static_cast<uint32_t>(ch->next_hop() & 0x000000ff);
    osstream_ << '\t';
    osstream_ << static_cast<uint32_t>(iph->saddr());
    osstream_ << '\t';
    osstream_ << static_cast<uint32_t>(iph->daddr());
    osstream_ << '\t';
    osstream_ << ch->direction();
    osstream_ << '\t';
    osstream_ << ch->ptype();
    trace_file_path_ << osstream_.str() << endl;
    trace_file_path_.close();
} /*  UwFlooding::writePathInTrace */

string UwFlooding::printIP(const nsaddr_t& ip_) {
    stringstream out;
    out << ((ip_ & 0xff000000) >> 24);
    out << ".";
    out << ((ip_ & 0x00ff0000) >> 16);
    out << ".";
    out << ((ip_ & 0x0000ff00) >> 8);
    out << ".";
    out << ((ip_ & 0x000000ff));
    return out.str();
} /* UwFlooding::printIP */
