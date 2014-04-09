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
 * @file   sun-ipr-node.cc
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Implements a SunIPRoutingNode.
 * 
 */

#include "sun-ipr-node.h"

extern packet_t PT_SUN_PATH_EST;
extern packet_t PT_SUN_PROBE;

long SunIPRoutingNode::number_of_pathestablishment_ = 0;
long SunIPRoutingNode::number_of_datapkt_ = 0;
long SunIPRoutingNode::number_of_ackpkt_ = 0;
long SunIPRoutingNode::number_of_drops_maxretx_ = 0;
long SunIPRoutingNode::number_of_drops_buffer_full_ = 0;
long SunIPRoutingNode::number_of_pkt_forwarded_ = 0;

/**
 * Adds the module for SunIPRoutingNode in ns2.
 */
static class SunNodeModuleClass : public TclClass {
public:

    SunNodeModuleClass() : TclClass("Module/UW/SUNNode") {
    }

    TclObject* create(int, const char*const*) {
        return (new SunIPRoutingNode());
    }
} class_module_sun_node;

// Timer. It is invoked automatically when the timer on a route expires. 
void RemoveHopTableTimer::expire(Event* e) {
    module->expTimerHopTable();
} /* RemoveHopTableTimer::expire */

// Timer. It is invoked automatically when the timer on a sink probe expires. 
void SinkProbeTimer::expire(Event* e) {
    module->lostSink();
} /* SinkProbeTimer::expire */

// Timer. It is invoked automatically when the search path timer expires. 
void SearchPathTimer::expire(Event* e) {
    module->searchPathExpire();
} /* SearchPathTimer::expire */

// Constructor for SunIPRoutingNode class
SunIPRoutingNode::SunIPRoutingNode()
: 
ipAddr_(0),
metrics_(SNR),
PoissonTraffic_(1),
period_status_(0.1),
period_data_(100),
num_hop_to_sink(0),
quality_link(0),
hop_table_length(0),
sink_associated(0),
printDebug_(0),
probe_min_snr_(10),
search_path_enable_(true),
reset_buffer_if_error_(0),
pointer_packets_(0),
pointer_acks_(0),
list_packets_max_time_(0),
list_acks_max_time_(0),
packets_array_full(false),
acks_array_full(false),
alpha_(1.0/3.0),
max_ack_error_(4),
ack_warnings_counter_(0),
ack_error_state(false),
buffer_max_size_(1),
safe_timer_buffer_(0),
timer_route_validity_(5),
timer_sink_probe_validity_(200),
timer_buffer_(15),
timer_search_path_(60),
rmhopTableTmr_(this),
sinkProbeTimer_(this),
bufferTmr_(this),
searchPathTmr_(this)
{
    if (STACK_TRACE)
        cout << "> SunIPRoutingNode()" << endl;
    bind("ipAddr_", &ipAddr_);
    bind("metrics_", &metrics_);
    bind("PoissonTraffic_", &PoissonTraffic_);
    bind("period_status_", &period_status_);
    bind("period_data_", &period_data_);
    bind("max_ack_error_", &max_ack_error_);
    bind("timer_route_validity_", &timer_route_validity_);
    bind("timer_sink_probe_validity_", &timer_sink_probe_validity_);
    bind("timer_buffer_", &timer_buffer_);
    bind("timer_search_path_", &timer_search_path_);
    bind("alpha_", &alpha_);
    bind("printDebug_", &printDebug_);
    bind("probe_min_snr_", &probe_min_snr_);
    bind("buffer_max_size_", &buffer_max_size_);
    bind("safe_timer_buffer_", &safe_timer_buffer_);
    bind("disable_path_error_", &disable_path_error_);
    bind("reset_buffer_if_error_", &reset_buffer_if_error_);
    hop_table = new nsaddr_t[MAX_HOP_NUMBER];
    bufferTmr_.resched(timer_buffer_);
    for (int i = 0; i < MAX_HOP_NUMBER; i++)
        data_and_hops[i] = 0;
    clearHops();
    trace_separator_ = '\t';
//    cout.precision(5);
//    cout.setf(ios::floatfield, ios::fixed);
} /* SunIPRoutingNode::SunIPRoutingNode */

// Destructor for SunIPRoutingNode class
SunIPRoutingNode::~SunIPRoutingNode() {
    if (STACK_TRACE)
        cout << "> ~SunIPRoutingNode()" << endl;
    if (printDebug_ > 5)
        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":node_deleted." << endl;
    delete[] hop_table;
    delete[] trace_file_name_;
} /* SunIPRoutingNode::~SunIPRoutingNode */

int SunIPRoutingNode::recvSyncClMsg(ClMessage* m) {
    if (STACK_TRACE)
        cout << "> recvSyncClMsg()" << endl;
    return Module::recvSyncClMsg(m);
} /* SunIPRoutingNode::recvSyncClMsg */

int SunIPRoutingNode::recvAsyncClMsg(ClMessage* m) {
    if (STACK_TRACE)
        cout << "> recvAsyncClMsg()" << endl;
    if (m->type() == UWIP_CLMSG_SEND_ADDR) {
        UWIPClMsgSendAddr* m_ = (UWIPClMsgSendAddr*) m;
        ipAddr_ = m_->getAddr();
    }
    return Module::recvAsyncClMsg(m);
} /* SunIPRoutingNode::recvAsyncClMsg */

// Usually when this function is invoked you have to rest also the info about the number of hops to the sink (@see setNumberOfHopToSink)
void SunIPRoutingNode::clearHops() {
    if (STACK_TRACE)
        cout << "> clearHops()" << endl;
    for (int i = 0; i < MAX_HOP_NUMBER; i++) {
        hop_table[i] = 0;
    }
    hop_table_length = 0;
    quality_link = 0;
    sink_associated = nsaddr_t(0);
} /* SunIPRoutingNode::clearHops */

/* This function is used to initialize the node. It sends to the lower
 * layers a Sync message asking for the IP.
 * In the tcl file it is important to add a line to call the command that will invoke
 * this function.
 * Example:
 * ...
 * set ipr($id)  [new Module/UW/SUNNode]
 * set ipif($id) [new Module/UW/IP]
 * ...
 * $node($id) addModule 5 $ipr($id)   0  "IPR"
 * $node($id) addModule 4 $ipif($id)  0  "IPF"  
 * ...
 * $ipif($id) addr "1.0.0.${tmp_}"
 * ...
 * $ipr($id) initialize
 */
void SunIPRoutingNode::initialize() {
    if (STACK_TRACE)
        cout << "> initialize()" << endl;
    // Asking for the IP of the current Node.
    if (ipAddr_ == 0) {
        UWIPClMsgReqAddr* m = new UWIPClMsgReqAddr(getId());
        m->setDest(CLBROADCASTADDR);
        sendSyncClMsgDown(m);
    }
} /* SunIPRoutingNode::initialize */

// This function prints in the stdout the routing table of the current node.
void SunIPRoutingNode::printHopTable() const {
    if (STACK_TRACE)
        cout << "> printHopTable()" << endl;
    printf("NextHopList\n");
    for (int i = 0; i < hop_table_length; i++) {
        cout << this->printIP(hop_table[i]);
    }
} /* SunIPRoutingNode::printHopTable */

/* This function convert an IP from a nsaddr_t to a string in the
 * classical form: x.x.x.x. It returns a string, it doesn't print
 * the value in the stdout.
 */
string SunIPRoutingNode::printIP(const nsaddr_t& ip_) {
    if (STACK_TRACE)
        cout << "> printIP()" << endl;
    stringstream out;
    out << ((ip_ & 0xff000000)>>24);
    out << ".";
    out << ((ip_ & 0x00ff0000)>>16);
    out << ".";
    out << ((ip_ & 0x0000ff00)>>8);
    out << ".";
    out << ((ip_ & 0x000000ff));
    return out.str();
} /* SunIPRoutingNode::printIP */

/* This function convert an IP from a ns_addr_t to a string in the
 * classical form: x.x.x.x
 */
string SunIPRoutingNode::printIP(const ns_addr_t& ipt_) {
    return SunIPRoutingNode::printIP(ipt_.addr_);
} /* SunIPRoutingNode::printIP */

nsaddr_t SunIPRoutingNode::str2addr(const char *str) {
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
} /* SunIPRoutingNode::str2addr */

// Command implementation from nsmiracle. */
int SunIPRoutingNode::command(int argc, const char*const* argv) {
    if (STACK_TRACE)
        cout << "> command()" << endl;
    Tcl& tcl = Tcl::instance();
    
    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            this->initialize();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "clearhops") == 0) {
            this->clearHops();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "printhopcount") == 0) {
            if (num_hop_to_sink <= 0)
                cout << "N: " << this->printIP(ipAddr_) <<  " not connected to the sink." << endl;
            else
                cout << "N: " << this->printIP(ipAddr_) <<  " # hops to sink = " << num_hop_to_sink << endl;
            return TCL_OK;
        } else if (strcasecmp(argv[1], "printhops") == 0) {
            this->printHopTable();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getackcount") == 0) {
            tcl.resultf("%lu", this->getAckCount());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getdatapktcount") == 0) {
            tcl.resultf("%lu", this->getDataCount());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getforwardedcount") == 0) {
            tcl.resultf("%lu", this->getForwardedCount());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getdatapktdroppedbuffer") == 0) {
            tcl.resultf("%lu", this->getDataDropsCountBuffer());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getdatapktdroppedmaxretx") == 0) {
            tcl.resultf("%lu", this->getDataDropsCountMaxRetx());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getpathestablishmentpktcount") == 0) {
            tcl.resultf("%lu", this->getPathEstablishmentCount());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getackheadersize") == 0) {
            tcl.resultf("%d", this->getAckHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getdatapktheadersize") == 0) {
            tcl.resultf("%d", this->getDataPktHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getpathestheadersize") == 0) {
            tcl.resultf("%d", this->getPathEstHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getmeanretx") == 0) {
//            double num = double(pkt_tx_) - double(pkt_stored_ - buffer_data.size());
//            double den = double(pkt_stored_ - buffer_data.size());
//            cout << ":ip:" << printIP(ipAddr_);
//            cout << ":pkt_tx_:" << pkt_tx_;
//            cout << ":pkt_stored_:" << pkt_stored_;
//            cout << ":pkts_still_in_buffer:" << buffer_data.size();
//            cout << ":meanretx:" << (double(num)/double(den)) << endl;
            if (pkt_stored_ == 0 || static_cast<ulong>(pkt_stored_) == buffer_data.size()) {
                tcl.resultf("%d", 0);
            } else {
                double num = double(pkt_tx_) - double(pkt_stored_ - buffer_data.size());
                double den = double(pkt_stored_ - buffer_data.size());
                double retx = std::max(double(num)/double(den), double(0));
                tcl.resultf("%f", retx);
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "gettransmittedpackets") == 0) {
            tcl.resultf("%d", pkt_tx_);
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "getstats") == 0) {
            if ((atoi(argv[2]) >= 0) && (atoi(argv[2]) < MAX_HOP_NUMBER)) {
                tcl.resultf("%d", data_and_hops[atoi(argv[2])]);
                return TCL_OK;
            } else {
                tcl.resultf(0);
                return TCL_OK;
            }
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
        } 
    }
    return Module::command(argc, argv);
} /* SunIPRoutingNode::command */

void SunIPRoutingNode::recv(Packet* p) {
    if (STACK_TRACE)
        cout << "> recv()" << endl;
    hdr_uwip* iph           = HDR_UWIP(p);
    hdr_cmn* ch             = HDR_CMN(p);
    hdr_sun_path_est* hpest = HDR_SUN_PATH_EST(p);
    hdr_sun_ack* hack       = HDR_SUN_ACK(p);
    hdr_uwcbr* uwcbrh       = HDR_UWCBR(p);

    if (!ch->error()) { // Process the packet only if it is free from errors.
        if (trace_)
            this->tracePacket(p, "RECV_PKT");
        if (ch->direction() == hdr_cmn::UP) {
            if ((ch->next_hop() == UWIP_BROADCAST) || (ch->next_hop() == ipAddr_) || (iph->daddr() == UWIP_BROADCAST) || (iph->daddr() == ipAddr_)) { // Redundant check already made in UWIP (just in case).
                if (ch->ptype() == PT_SUN_PATH_EST) { // Path Establishment Packet.
                    if (hpest->ptype() == PATH_SEARCH) { // Path Search.
                        if (iph->saddr() == ipAddr_) { // Garbage: the node received a request from itself.
                            Packet::free(p);
                            return;
                        } else {
                            if (printDebug_ > 5) {
                                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":UP:PATH_EST:PATH_REQUEST:source:" << printIP(iph->saddr()) << "." << endl;
                                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":replyPathEstSearch." << endl;
                            }
                            if(trace_)
                                this->tracePacket(p, "RECV_SRC");
                            this->replyPathEstSearch(p->copy());
                            Packet::free(p);
                            return;
                        }
                    } else if (hpest->ptype() == PATH_ANSWER) { // Path Answer.
                        if (iph->saddr() == ipAddr_) { // Garbage: the node received an answer from itself.
                            Packet::free(p);
                            return;
                        } else {
                            if (iph->daddr() == ipAddr_) { // The packet is arrived to the destination.
                                if (printDebug_ > 5) {
                                    cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":UP:PATH_EST:PATH_ANSWER:source:" << printIP(iph->saddr()) << "." << endl;
                                    cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":evaluatePath." << endl;
                                }
                                if(trace_)
                                    this->tracePacket(p, "RECV_PTH");
                                this->evaluatePath(p);
                                Packet::free(p);
                                return;
                            } else {
                                if (printDebug_ > 5) {
                                    cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":UP:PATH_EST:PATH_ANSWER:source:" << printIP(iph->saddr()) << "." << endl;
                                    cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":sendRouteBack." << endl;
                                }
                                this->sendRouteBack(p->copy());
                                Packet::free(p);
                                return;
                            }
                        }
                    } else if (hpest->ptype() == PATH_ERROR) {
                        if (iph->saddr() == ipAddr_) { // Garbage: the node received an error from itself.
                            Packet::free(p);
                            return;
                        } else { // Path error: reset the route information and send back the route error packet.
                            if (printDebug_ > 5) {
                                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":UP:PATH_EST:PATH_ERROR:source:" << printIP(iph->saddr()) << "." << endl;
                                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":clearHops,setNumberOfHopToSink(0)." << endl;
                                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":sendRouteErrorBack." << endl;
                            }
                            if(trace_)
                                this->tracePacket(p, "RECV_ERR");
                            this->clearHops();
                            this->setNumberOfHopToSink(0);
                            if (iph->daddr() != ipAddr_) {
                                this->sendRouteErrorBack(p->copy());
                                if (reset_buffer_if_error_) {
                                    while(!buffer_data.empty()) {
                                        Packet::free(((buffer_element) buffer_data.front()).p_);
                                        buffer_data.erase(buffer_data.begin());
                                        Packet::free(p);
                                        return;
                                    }
                                }
                            }
                            Packet::free(p);
                            return;
                        }
                    } else {
                        Packet::free(p);
                        return;
                    }
                } else if (ch->ptype() == PT_SUN_ACK) { // Ack Packet.
                    if (iph->saddr() == ipAddr_) { // Garbage: the node received an ack from itself.
                        Packet::free(p);
                        return;
                    } else {
                        if (printDebug_ > 5)
                            cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":UP:ACK:source:" << printIP(iph->saddr()) << "." << endl;
                        if(trace_)
                            this->tracePacket(p, "RECV_ACK");
                        this->updateAcksCount();
                        buffer_element _tmp = buffer_data.front();
                        if (hack->uid() == _tmp.id_pkt_) { // Ack for the first packet in the buffer.
                            ack_warnings_counter_ = 0;
                            ack_error_state = false;
                            if (buffer_data.size() > 0) { // There is at least one packet in the buffer.
                                if (safe_timer_buffer_) {
                                    timer_buffer_ = timer_buffer_ * 0.99 + (Scheduler::instance().clock() - ch->timestamp()) * 2 * (1 - 0.99);
                                    if (timer_buffer_ < (Scheduler::instance().clock() - ch->timestamp())*2) {
                                        timer_buffer_ = (Scheduler::instance().clock() - ch->timestamp())*2;
                                    }
                                }
                                if (printDebug_ > 5)
                                    cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":buf_size:" << buffer_data.size() << ":packet_removed." << endl;
                                Packet::free(((buffer_element) buffer_data.front()).p_);
                                buffer_data.erase(buffer_data.begin()); // Remove the first packet.
                            }
                        } else {
                            ;
                        }
                        Packet::free(p);
                    }
                } else if (ch->ptype() == PT_SUN_PROBE) { // Probe Packet.
                    if (printDebug_ > 5) {
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":UP:PROBE:source:" << printIP(iph->saddr()) << "." << endl;
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":receiveProbe." << endl;
                    }
                    if(trace_)
                        this->tracePacket(p, "RECV_PRB");
                    this->receiveProbe(p);
                    Packet::free(p);
                    return;
                } else { //ch->ptype_ == Data packet.
                    if(trace_)
                        this->tracePacket(p, "RECV_DTA");
                    this->updatePacketsCount();
                    if (iph->daddr() == ipAddr_) { // The packet is arrived at destination: send it up.
                        // Send back an ack
                        if (printDebug_ > 5)
                            cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":sendBackAck." << endl;
                        this->sendBackAck(p);
                        
                        if (printDebug_ > 5)
                            cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":UP:DATA:DESTINATION_REACHED." << endl;
                        ch->size() -= sizeof(hdr_sun_data);
                        sendUp(p->copy());
                        Packet::free(p);
                        return;
                    } else if ((ch->next_hop() == ipAddr_) && (iph->daddr() != ipAddr_)) { //The node is a relay, forward the Data packet.
                        // Send back an ack
                        if (printDebug_ > 5)
                            cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":sendBackAck." << endl;
                        this->sendBackAck(p); // Send back an ack.
                        
                        if (buffer_data.size() < buffer_max_size_) {
                            if (printDebug_ > 5)
                                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":UP:DATA:PKT_BUFFERED." << endl;
//                            if (buffer_data.size() == 0) { //TODO: enable this?
//                                this->searchPath(); // Node not connected with any sink: send a path establishment request.
//                                search_path_enable_ = false;
//                                searchPathTmr_.resched(timer_search_path_);
//                            }
                            pkt_stored_++;
                            buffer_data.push_back(buffer_element(p->copy(), uwcbrh->sn(), Scheduler::instance().clock()));
                            Packet::free(p);
                        } else {
                            if (printDebug_ > 5)
                                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":UP:DATA:DROP:BUFFER_IS_FULL." << endl;
                            number_of_drops_buffer_full_++;
                            drop(p, 1, DROP_BUFFER_IS_FULL);
                        }
                        return;
                    }
                }
            } else { // Wrong destination.
                if (printDebug_ > 5)
                    cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":Packet:" << ch->uid() << ":UP:DATA:DROP:wrong_ip." << endl;
                drop(p, 1, DROP_PACKET_NOT_FOR_ME);
                return;
            }
        } else if (ch->direction() == hdr_cmn::DOWN) { // The packet is arrived from the upper layers.
            this->updatePacketsCount();
            if (this->getNumberOfHopToSink() == 1) { // The node is connected directly with the sink.
                iph->saddr() = ipAddr_;
                iph->daddr() = 0; // Used to set that the packet is not initialized.
                if (buffer_data.size() < buffer_max_size_) { // There is space to buffer the packet.
                    if (printDebug_ > 5)
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":DOWN:DATA:PKT_BUFFERED." << endl;
                    pkt_stored_++;
                    buffer_data.push_back(buffer_element(p->copy(), uwcbrh->sn(), Scheduler::instance().clock()));
                    Packet::free(p);
                } else {
                    if (printDebug_ > 5)
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":DOWN:DATA:DROP:BUFFER_IS_FULL." << endl;
                    number_of_drops_buffer_full_++;
                    drop(p, 1, DROP_BUFFER_IS_FULL);
                }
                return;
            } else { // The current node is not directly connected with the sink.
                iph->saddr() = ipAddr_;
                iph->daddr() = 0; // Used to set that the packet is not initialized.
                if (buffer_data.size() < buffer_max_size_) {
                    if (printDebug_ > 5)
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":DOWN:DATA:PKT_BUFFERED." << endl;
                    pkt_stored_++;
                    buffer_data.push_back(buffer_element(p->copy(), uwcbrh->sn(), Scheduler::instance().clock()));
                    Packet::free(p);
                } else {
                    if (printDebug_ > 5)
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":DOWN:DATA:DROP:BUFFER_IS_FULL." << endl;
                    number_of_drops_buffer_full_++;
                    drop(p, 1, DROP_BUFFER_IS_FULL);
                }
                if (this->getNumberOfHopToSink() == 0) {
                    if (printDebug_ > 5)
                        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":searchPath." << endl;
                    if (search_path_enable_) {
                        this->searchPath(); // Node not connected with any sink: send a path establishment request.
                        search_path_enable_ = false;
                        searchPathTmr_.resched(timer_search_path_);
                    }
                }
                return;
            }
        } else {
            if (printDebug_ > 5)
                cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":Packet:" << ch->uid() << ":DOWN:DATA:DROP:NO_DIRECTION." << endl;
            drop(p, 1, DROP_PACKET_WITHOUT_DIRECTION);
            return;
        }
    }
} /* SunIPRoutingNode::recv */

/* This function sets the number of hops that the current node needs to reach
 * the sink.
 * It accepts only values >= 0.
 * If the input value is accepted the function returns the input value,
 * otherwise it returns 0 and doesn't change the current hop_count.
 */
const int& SunIPRoutingNode::setNumberOfHopToSink(const int& hop) {
    if (STACK_TRACE)
        cout << "> setNumberOfHopToSink()" << endl;
    if (hop < 0) {
        num_hop_to_sink = 0;
        return this->getNumberOfHopToSink();
    } else { // (hop >= 0)
        num_hop_to_sink = hop;
        snr_to_sink_ = double(-9999.9);
        return this->getNumberOfHopToSink();
    }
} /* SunIPRoutingNode::setNumberOfHopToSink */

/* If = 1: directly connected;
 * If = 0: not connected;
 * If > 1: # of hops;
 */
const int& SunIPRoutingNode::getNumberOfHopToSink() const {
    if (STACK_TRACE)
        cout << "> getNumberOfHopToSink()" << endl;
    return num_hop_to_sink;
} /* SunIPRoutingNode::getNumberOfHopToSink */

void SunIPRoutingNode::receiveProbe(const Packet* p) {
    if (STACK_TRACE)
        cout << "> receiveProbe()" << endl;
    hdr_MPhy* ph  = HDR_MPHY(p);
    hdr_uwip* iph = HDR_UWIP(p);
    if (this->isZero(ph->Pn)) { // If the power of noise value is not initialized accept the probe anyway.
        this->clearHops();
        this->setNumberOfHopToSink(1);
        sink_associated = iph->saddr();
        sinkProbeTimer_.resched(timer_sink_probe_validity_);
    } else if (10*log10(ph->Pr/(ph->Pi + ph->Pn)) > probe_min_snr_) { // Accept the probe only if its snr is greater than a minimum value.
//        cout << "sinr probe: " << 10*log10(ph->Pr/(ph->Pi + ph->Pn)) << endl;
//        cout << "snr probe: " << 10*log10(ph->Pr/(ph->Pn)) << endl;
        this->clearHops();
        this->setNumberOfHopToSink(1);
        sink_associated = iph->saddr();
        snr_to_sink_ = 10*log10(ph->Pr/(ph->Pn));
        sinkProbeTimer_.resched(timer_sink_probe_validity_);
    }
} /* SunIPRoutingNode::receiveProbe */

/* This function send a message to inform the other nodes involved in a path
 * that the current node lost its link with the sink. And resets the information
 * about the sink.
 */
void SunIPRoutingNode::lostSink() {
    if (STACK_TRACE)
        cout << "> lostSink()" << endl;
    if (this->getNumberOfHopToSink() == 1) { // Only a node with hc 1 can lost the sink.
        this->clearHops();
        this->setNumberOfHopToSink(0);
        sink_associated = nsaddr_t(0);
    }
} /* SunIPRoutingNode::lostSink */

void SunIPRoutingNode::searchPathExpire() {
    if (STACK_TRACE)
        cout << "> searchPathExpire()" << endl;
    search_path_enable_ = true;
    return;
} /* SunIPRoutingNode::searchPathExpire */

/* This function is called by the timer. It resets all the routing
 * information.
 */
void SunIPRoutingNode::expTimerHopTable() {
    if (STACK_TRACE)
        cout << "> expTimerHopTable()" << endl;
    if (printDebug_ > 5)
        cout << "@" << Scheduler::instance().clock() << ":Node:" << this->printIP(ipAddr_) <<  ":hc:" << this->getNumberOfHopToSink() << ":routing_table_expired." << endl;
    this->clearHops();
    this->setNumberOfHopToSink(0);
} /* SunIPRoutingNode::expTimerHopTable */

void SunIPRoutingNode::tracePacket(const Packet* const p, const string& position) {
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
} /* SunIPRoutingNode::tracePacket */

string SunIPRoutingNode::createTraceString(const string& info_string, const double& simulation_time_, const int& node_id_, const int& pkt_id_, const int& pkt_sn_, const int& pkt_from_, const int& pkt_next_hop, const int& pkt_source_, const int& pkt_destination_, const double& snr_, const int& direction_, const int& pkt_type) {
    if (STACK_TRACE)
        cout << "> createTraceString()" << endl;
    osstream_.clear();
    osstream_.str("");
    osstream_ << info_string << this->trace_separator_ << simulation_time_ << this->trace_separator_ << (node_id_ & 0x000000ff) << this->trace_separator_ << (pkt_id_ & 0x000000ff) << this->trace_separator_ << (pkt_sn_ & 0x000000ff) << this->trace_separator_ << (pkt_from_ & 0x000000ff) << this->trace_separator_ << (pkt_next_hop & 0x000000ff) << this->trace_separator_ << (pkt_source_ & 0x000000ff) << this->trace_separator_ << (pkt_destination_ & 0x000000ff) << this->trace_separator_ << snr_ << this->trace_separator_ << direction_ << this->trace_separator_ << pkt_type;
    return osstream_.str();
} /* SunIPRoutingNode::createTraceString */

void SunIPRoutingNode::writeInTrace(const string& string_to_write_) {
    if (STACK_TRACE)
        cout << "> writeInTrace()" << endl;
    trace_file_.open(trace_file_name_, fstream::app);
    trace_file_ << string_to_write_ << endl;
    trace_file_.close();
} /* SunIPRoutingNode::writeInTrace */