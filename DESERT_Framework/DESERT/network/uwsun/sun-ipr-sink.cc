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
 * @file   sun-ipr-sink.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Implements a SunIPRoutingSink.
 * 
 */

#include "sun-ipr-sink.h"
#include "mphy_pktheader.h"

extern packet_t PT_SUN_ACK;
extern packet_t PT_SUN_DATA;
extern packet_t PT_SUN_PROBE;
extern packet_t PT_SUN_PATH_EST;

long SunIPRoutingSink::probe_count_ = 0;
long SunIPRoutingSink::number_of_ackpkt_ = 0;

/**
 * Adds the module for SunIPRoutingSink in ns2.
 */
static class SunSinkModuleClass : public TclClass {
public:

    SunSinkModuleClass() : TclClass("Module/UW/SUNSink") {
    }

    TclObject* create(int, const char*const*) {
        return (new SunIPRoutingSink());
    }
} class_module_sun_sink;

// Timer.
void SunIPRoutingSink::SendTimer::expire(Event *e) {
    module->transmit();
}

SunIPRoutingSink::SunIPRoutingSink()
: t_probe(30),
numberofnodes_(0),
sendTmr_(this),
trace_(false),
trace_path_(false)
{ // Binding to TCL variables.
    if (STACK_TRACE)
        cout << "> SunIPRoutingSink()" << endl;
    bind("t_probe", &t_probe);
    bind("ipAddr_", &ipAddr_);
    bind("PoissonTraffic_", &PoissonTraffic_);
    bind("periodPoissonTraffic_", &periodPoissonTraffic_);
    bind("printDebug_", &printDebug_);
    trace_separator_ = '\t';
}

SunIPRoutingSink::~SunIPRoutingSink() {
    if (STACK_TRACE)
        cout << "> ~SunIPRoutingSink()" << endl;
    if (printDebug_ > 10)
        cout << "S: " << this->printIP(ipAddr_) << " deleted." << endl;
}

int SunIPRoutingSink::recvSyncClMsg(ClMessage* m) {
    if (STACK_TRACE)
        cout << "> recvSyncClMsg()" << endl;
    return Module::recvSyncClMsg(m);
}

int SunIPRoutingSink::recvAsyncClMsg(ClMessage* m) {
    if (STACK_TRACE)
        cout << "> recvAsyncClMsg()" << endl;
    if (m->type() == UWIP_CLMSG_SEND_ADDR) {
        UWIPClMsgSendAddr* m_ = (UWIPClMsgSendAddr*) m;
        ipAddr_ = m_->getAddr();
    }
    return Module::recvAsyncClMsg(m);
}

int SunIPRoutingSink::command(int argc, const char*const* argv) {
    if (STACK_TRACE)
        cout << "> command()" << endl;
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            this->initialize();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "start") == 0) {// TCL command to start the packet generation and transmission.
            this->start();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "stop") == 0) { // TCL command to stop the packet generation.
            this->stop();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "sendprobe") == 0) {
            this->sendProbe();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getprobetimer") == 0) {
            this->getProbeTimer();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getprobepktcount") == 0) {
            tcl.resultf("%lu", this->getProbeCount());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getackcount") == 0) {
            tcl.resultf("%lu", this->getAckCount());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getprobepktheadersize") == 0) {
            tcl.resultf("%i", this->getProbePktHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getackheadersize") == 0) {
            tcl.resultf("%d", this->getAckHeaderSize());
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "setnumberofnodes") == 0) {
            numberofnodes_ = atoi(argv[2]);
            arrayofstats_ = new unsigned int*[numberofnodes_];
            for (int i = 0; i < numberofnodes_; i++) {
                arrayofstats_[i] = new unsigned int[MAX_HOP_NUMBER];
                for (int j = 0; j < MAX_HOP_NUMBER; j++)
                    arrayofstats_[i][j] = 0;
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "addr") == 0) {
            ipAddr_ = str2addr((char *) argv[2]);
            if (ipAddr_ == 0) {
                fprintf(stderr, "0.0.0.0 is not a valid IP address");
                return TCL_ERROR;
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "trace") == 0) {
            string tmp_ = ((char *) argv[2]);
            trace_file_name_ = new char[tmp_.length() + 1];
            strcpy(trace_file_name_, tmp_.c_str());
            if (trace_file_name_ == NULL) {
                fprintf(stderr, "Empty string for the trace file name");
                return TCL_ERROR;
            }
            trace_ = true;
            remove(trace_file_name_);
            trace_file_.open(trace_file_name_);
            trace_file_.close();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "tracepaths") == 0) {
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
    } else if (argc == 4) {
        if (strcasecmp(argv[1], "getstats") == 0) {
            if (numberofnodes_ != 0) {
                if ((atoi(argv[2])) < numberofnodes_) {
                    if ((atoi(argv[3])) < MAX_HOP_NUMBER) {
                        tcl.resultf("%d", arrayofstats_[atoi(argv[2])][atoi(argv[3])]);
                        return TCL_OK;
                    } else {
                        tcl.resultf(0);
                        return TCL_OK;
                    }
                }
            }
        }
    }
    return Module::command(argc, argv);
}

void SunIPRoutingSink::recv(Packet *p) {
    if (STACK_TRACE)
        cout << "> recv()" << endl;
    hdr_cmn*  ch    = HDR_CMN(p);
    hdr_uwip* iph   = HDR_UWIP(p);
    hdr_sun_data* hdata = HDR_SUN_DATA(p);

    if (!ch->error()) {
        if (trace_)
            this->tracePacket(p, "RECV_PKT");
        if (ch->direction() == hdr_cmn::UP) {
            if ((ch->ptype() != PT_SUN_PATH_EST) && (ch->ptype() != PT_SUN_ACK) && (ch->ptype() != PT_SUN_PROBE)) { // Sink nodes accept only data packets.
//                cout << "S: " << this->printIP(ipAddr_) <<  " - P: " << ch->uid() << " - Packet Received - UP." << endl;
                if (ch->next_hop() == ipAddr_ || iph->daddr() == ipAddr_) {
                    if (printDebug_ > 5)
                        cout << "@" << Scheduler::instance().clock() << ":Sink:" << this->printIP(ipAddr_) << ":UP:DATA:SINK_REACHED." << endl;
                    
                    if (printDebug_ > 5)
                        cout << "@" << Scheduler::instance().clock() << ":Sink:" << this->printIP(ipAddr_) << ":sendBackAck." << endl;
                    this->sendBackAck(p);
                    
                    if (numberofnodes_ != 0) {
                        if (numberofnodes_ >= hdata->list_of_hops_length()) {
//                            cout << printIP(iph->saddr()) << endl;
                            int ip_ = (iph->saddr() & 0x000000ff);
                            arrayofstats_[ip_ - 1][hdata->list_of_hops_length()]++;
                        }
                    } else {
                        ;
                    }
                    if(trace_)
                        this->tracePacket(p, "RECV_DTA");
                    if(trace_path_)
                        this->writePathInTrace(p);
                    ch->size() -= sizeof (hdr_sun_data);
                    sendUp(p);
                    return;
                } else {
                    Packet::free(p);
                    return;
                }
            } else {
                Packet::free(p);
                return;
            }
        } else if (ch->direction() == hdr_cmn::DOWN) {
            return;
        }
    } else {
        Packet::free(p);
        return;
    }
}

void SunIPRoutingSink::initialize() {
    if (STACK_TRACE)
        cout << "> initialize()" << endl;
    // Asking for the IP of the current Node.
    if (ipAddr_ == 0) {
        UWIPClMsgReqAddr* m = new UWIPClMsgReqAddr(getId());
        m->setDest(CLBROADCASTADDR);
        sendSyncClMsgDown(m);
    }
}

void SunIPRoutingSink::sendProbe() {
    if (STACK_TRACE)
        cout << "> sendProbe()" << endl;
    Packet* p = Packet::alloc();

    hdr_cmn* ch = HDR_CMN(p);
    ch->uid()       = sunuid_++;
    ch->ptype()     = PT_SUN_PROBE;
    //ch->size()      = pktSize_; Look Down;
    ch->direction() = hdr_cmn::DOWN;
    ch->next_hop()  = UWIP_BROADCAST; //TODO: check these values

    // IP Header.
    hdr_uwip* iph   = HDR_UWIP(p);
    iph->daddr()    = UWIP_BROADCAST; // A Probe is always sent in broadcast.
    //iph->dport()  = 0;
    iph->saddr()    = ipAddr_;
    //iph->sport()  = 0;

    ch->size() += sizeof (hdr_sun_probe);
    ch->timestamp() = Scheduler::instance().clock();

    if (printDebug_ > 10)
        cout << "@" << Scheduler::instance().clock() << " -> S: " << this->printIP(ipAddr_) << " - P: " << ch->uid() << " - Probe Sent." << endl;
    probe_count_++; // Only for statistics
    if (trace_)
        this->tracePacket(p, "SEND_PRB");
    sendDown(p, this->getDelay(periodPoissonTraffic_));
}

string SunIPRoutingSink::printIP(const nsaddr_t& ip_) {
    if (STACK_TRACE)
        cout << "> printIP()" << endl;
    stringstream out;
    out << ((ip_ & 0xff000000) >> 24);
    out << ".";
    out << ((ip_ & 0x00ff0000) >> 16);
    out << ".";
    out << ((ip_ & 0x0000ff00) >> 8);
    out << ".";
    out << ((ip_ & 0x000000ff));
    return out.str();
}

/* This function convert an IP from a ns_addr_t to a string in the
 * classical form: x.x.x.x
 */
string SunIPRoutingSink::printIP(const ns_addr_t& ipt_) {
    return SunIPRoutingSink::printIP(ipt_.addr_);
} /* SunIPRoutingSink::printIP */

nsaddr_t SunIPRoutingSink::str2addr(const char *str) {
    if (STACK_TRACE)
        cout << "> str2addr()" << endl;
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
} /* SunIPRoutingSink::str2addr */

void SunIPRoutingSink::start() {
    if (STACK_TRACE)
        cout << "> start()" << endl;
    sendTmr_.resched(this->getProbeTimer());
}

void SunIPRoutingSink::stop() {
    if (STACK_TRACE)
        cout << "> stop()" << endl;
    sendTmr_.force_cancel();
}

void SunIPRoutingSink::transmit() {
    if (STACK_TRACE)
        cout << "> transmit()" << endl;
    this->sendProbe();
    sendTmr_.resched(this->getProbeTimer());
}

void SunIPRoutingSink::setProbeTimer(const double& t_) {
    if (STACK_TRACE)
        cout << "> setProbeTimer()" << endl;
    if (t_ > 0) {
        t_probe = t_;
    }
}

const double& SunIPRoutingSink::getProbeTimer() const {
    if (STACK_TRACE)
        cout << "> getProbeTimer()" << endl;
    return t_probe;
}

void SunIPRoutingSink::sendBackAck(const Packet* p) {
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
    
    if (printDebug_ > 10)
        cout << "@" << Scheduler::instance().clock() << " -> N: " << this->printIP(ipAddr_) << " - P: " << ch->uid() << " - Sending back Data Ack to: " << printIP(ch->prev_hop_) << "." << endl;
    number_of_ackpkt_++;
//    cout << printIP(ipAddr_) << ":ack:" << printIP(ch->prev_hop_) << endl;
    if (trace_)
        this->tracePacket(p_ack, "SEND_ACK");
    sendDown(p_ack, this->getDelay(periodPoissonTraffic_));
} /* SunIPRoutingSink::sendBackAck */

void SunIPRoutingSink::initPktAck(Packet* p) {
    if (STACK_TRACE)
        cout << "> initPktPathEstSearch()" << endl;

    // Common header.
    hdr_cmn* ch     = HDR_CMN(p);
    ch->uid()       = sunuid_++;
    ch->ptype()     = PT_SUN_ACK;
    //ch->size()     = 0; // Look down.
    ch->direction() = hdr_cmn::DOWN;
    //ch->next_hop()  = 0;
    ch->prev_hop_   = ipAddr_;
    
    // IP header.
    //iph->daddr() = 0;
    //iph->dport() = 0;
    //iph->saddr() = 0;
    //iph->sport() = 0;
    
    ch->size()      += sizeof(hdr_sun_ack);
    ch->timestamp() = Scheduler::instance().clock();
} /* SunIPRoutingSink::initPktAck */

void SunIPRoutingSink::tracePacket(const Packet* const p, const string& position) {
    if (STACK_TRACE)
        cout << "> tracePacket()" << endl;
    hdr_MPhy* ph            = HDR_MPHY(p);
    hdr_uwip* iph           = HDR_UWIP(p);
    hdr_cmn* ch             = HDR_CMN(p);
    hdr_sun_ack* hack       = HDR_SUN_ACK(p);
    hdr_uwcbr* uwcbrh       = HDR_UWCBR(p);
    if (trace_) {
        double snr_ = 0;
        if (ph->Pn == 0) { // TODO: trick for CMRE logs
            snr_ = -999;
        } else {
            snr_ = 10*log10(ph->Pr/ph->Pn);
        }
        if (ch->ptype() == PT_SUN_ACK) {
            this->writeInTrace(this->createTraceString(position, Scheduler::instance().clock(), ipAddr_, ch->uid(), hack->uid(), ch->prev_hop_, ch->next_hop(), iph->saddr(), iph->daddr(), snr_, ch->direction(), ch->ptype()));
        } else {
            this->writeInTrace(this->createTraceString(position, Scheduler::instance().clock(), ipAddr_, ch->uid(), uwcbrh->sn(), ch->prev_hop_, ch->next_hop(), iph->saddr(), iph->daddr(), snr_, ch->direction(), ch->ptype()));
        }
    }
} /* SunIPRoutingSink::tracePacket */

string SunIPRoutingSink::createTraceString(const string& info_string, const double& simulation_time_, const int& node_id_, const int& pkt_id_, const int& pkt_sn_, const int& pkt_from_, const int& pkt_next_hop, const int& pkt_source_, const int& pkt_destination_, const double& snr_, const int& direction_, const int& pkt_type) {
    if (STACK_TRACE)
        cout << "> createTraceString()" << endl;
    osstream_.clear();
    osstream_.str("");
    osstream_ << info_string << this->trace_separator_ << simulation_time_ << this->trace_separator_ << (node_id_ & 0x000000ff) << this->trace_separator_ << (pkt_id_ & 0x000000ff) << this->trace_separator_ << (pkt_sn_ & 0x000000ff) << this->trace_separator_ << (pkt_from_ & 0x000000ff) << this->trace_separator_ << (pkt_next_hop & 0x000000ff) << this->trace_separator_ << (pkt_source_ & 0x000000ff) << this->trace_separator_ << (pkt_destination_ & 0x000000ff) << this->trace_separator_ << snr_ << this->trace_separator_ << direction_ << this->trace_separator_ << pkt_type;
    return osstream_.str();
} /* SunIPRoutingSink::createTraceString */

void SunIPRoutingSink::writeInTrace(const string& string_to_write_) {
    if (STACK_TRACE)
        cout << "> writeInTrace()" << endl;
    
    trace_file_.open(trace_file_name_, fstream::app);
    trace_file_ << string_to_write_ << endl;
    trace_file_.close();
} /* SunIPRoutingNode::writeInTrace */

void SunIPRoutingSink::writePathInTrace(const Packet* p) {
    if (STACK_TRACE)
        cout << "> writePathInTrace()" << endl;
    
    hdr_uwip* iph           = HDR_UWIP(p);
    hdr_cmn* ch             = HDR_CMN(p);
    hdr_sun_data* hdata     = HDR_SUN_DATA(p);
    
    trace_file_path_.open(trace_file_path_name_, fstream::app);
    osstream_.clear();
    osstream_.str("");
    osstream_ << Scheduler::instance().clock() << '\t' << ch->uid() << '\t' << hdata->list_of_hops_length() + 1 << '\t' << this->printIP(iph->saddr());
    for (int i = 0; i < hdata->list_of_hops_length(); i++) {
        osstream_ << '\t' << this->printIP(hdata->list_of_hops()[i]);
    }
    osstream_ << '\t' << this->printIP(iph->daddr());
    trace_file_path_ << osstream_.str() << endl;
    trace_file_path_.close();
} /*  SunIPRoutingSink::writePathInTrace */

