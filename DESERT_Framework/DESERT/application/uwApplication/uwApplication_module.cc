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
//

/**
 * @file   uwApplication_module.cc
 * @author Federico Favaro
 * @version 1.1.0
 *
 * \brief Provides the implementation of uwApplicationmodule class
 *
 */

#include <sstream>
#include <time.h>
#include "uwApplication_cmn_header.h"
#include "uwApplication_module.h"

static class uwApplicationModuleClass : public TclClass {
public:

    /**
     * Constructor of uwApplicationModuleClass class 
     */
    uwApplicationModuleClass() : TclClass("Module/UW/APPLICATION") {
    }

    /**
     *  Creates the TCL object needed for the TCL language interpretation
     * 
     * @return Pointer to an TclObject
     */
    TclObject* create(int, const char*const*) {
        return (new uwApplicationModule());
    }
} class_module_uwapplicationmodule;

uwApplicationModule::uwApplicationModule()
: chkTimerPeriod(this),
uidcnt(0),
txsn(1),
rftt(0),
pkts_lost(0),
pkts_recv(0),
pkts_ooseq(0),
pkts_invalid(0),
pkts_last_reset(0),
lrtime(0),
sumrtt(0),
sumrtt2(0),
rttsamples(0),
sumftt(0),
sumftt2(0),
fttsamples(0),
esn(0),
sumbytes(0),
sumdt(0),
hrsn(0),
servSockDescr(0),
servPort(0),
tcp_udp(-1),
logging(false),
node_id(0),
exp_id(0)
{
    bind("debug_", (int*) &debug_);
    bind("period_", (int*) &PERIOD);
    bind("node_ID_", (int*) &node_id);
    bind("EXP_ID_", (int*) &exp_id);
    bind("PoissonTraffic_", (int*) &poisson_traffic);
    bind("Payload_size_", (int*) &payloadsize);
    bind("destAddr_", (int*) &dst_addr);
    bind("destPort_", (int*) &port_num);
    bind("Socket_Port_", (int*) &servPort);
    bind("drop_out_of_order_", (int*) &drop_out_of_order);

    sn_check = new bool[USHRT_MAX];
    for (int i = 0; i < USHRT_MAX; i++) {
        sn_check[i] = false;
    }
    //servPort = port_num;
} //end uwApplicationModule() Method

uwApplicationModule::~uwApplicationModule() {

}

int uwApplicationModule::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();

    if (argc == 2) {
        if (strcasecmp(argv[1], "start") == 0) {
            if (withoutSocket()) {
                //Generate DATA packets without the use of sockets
                start_generation();
            } else {
                //The communication take place with the use of sockets 
                if (useTCP()) {
                    //Generate DATA packets using TCP connection
                    uwApplicationModule::openConnectionTCP();
                } else {
                    //Generate DATA packets using UDP connection
                    uwApplicationModule::openConnectionUDP();
                }
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "stop") == 0) {
            stop();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getsentpkts") == 0) {
            tcl.resultf("%d", getPktSent());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "lostpkts") == 0) {
            tcl.resultf("%f", getPktLost());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getrecvpkts") == 0) {
            tcl.resultf("%d", getPktRecv());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "outofsequencepkts") == 0) {
            tcl.resultf("%f", getPktsOOSequence());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "notknownpktrx") == 0) {
            tcl.resultf("%f", getPktsInvalidRx());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getrecvpktsqueue") == 0) {
            tcl.resultf("%d", getPktsPushQueue());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getrtt") == 0) {
            tcl.resultf("%f", GetRTT());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getrttstd") == 0) {
            tcl.resultf("%f", GetRTTstd());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getftt") == 0) {
            tcl.resultf("%f", GetFTT());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getfttstd") == 0) {
            tcl.resultf("%f", GetFTTstd());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getper") == 0) {
            tcl.resultf("%f", GetPER());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getthr") == 0) {
            tcl.resultf("%f", GetTHR());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "print_log") == 0) {
            std::stringstream stat_file;
            stat_file << "UWAPPLICATION_LOG_NODE_ID_" << node_id << "_EXP_ID_" << exp_id << ".out";
            out_log.open(stat_file.str().c_str(),std::ios_base::app);
            out_log << left <<  "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::FILE_CREATED" << endl;
            logging = true;
            return TCL_OK;
        } 
    } else if (argc == 3) {
        if (strcasecmp(argv[1],"SetSocketProtocol") == 0) {
            string protocol = argv[2];
            if (strcasecmp(protocol.c_str(),"UDP") == 0)
            {
                socket_active = true;
                tcp_udp = 0;
            } else if (strcasecmp(protocol.c_str(),"TCP") == 0) {
                socket_active = true;
                tcp_udp = 1;
            } else {
                socket_active = false;
                tcp_udp = -1;
            }
            return TCL_OK;
        }
    }
    return Module::command(argc, argv);
}//end command() Method

int uwApplicationModule::crLayCommand(ClMessage* m) {
    switch (m->type()) {
        default:
            return Module::crLayCommand(m);
    }
}//end crLayCommand() Method

void uwApplicationModule::recv(Packet* p) {
    if (withoutSocket()) {
        if (debug_ >=1 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::RECV_PACKET_WITHOUT_SOCKET_MODE" << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::RECV_PACKET_WITHOUT_SOCKET_MODE" << endl;
        statistics(p);
    } else {
        //Communication take place with sockets 
        if (useTCP()) {
            if (debug_ >=1 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::RECV_PACKET_USING_TCP" << endl;
            if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::RECV_PACKET_USING_TCP" << endl;
            statistics(p);
        } else {
            if (debug_ >=1 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::RECV_PACKET_USING_UDP" << endl;
            if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::RECV_PACKET_USING_UDP" << endl;
            statistics(p);
        }
    }
}; //end recv() Method

void uwApplicationModule::statistics(Packet* p) {
    hdr_cmn* ch = hdr_cmn::access(p);
    hdr_DATA_APPLICATION* uwApph = HDR_DATA_APPLICATION(p);

    if (ch->ptype_ != PT_DATA_APPLICATION) {
        if (debug_ >=0 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::DROP_PACKET_NOT_APPLICATION_TYPE" << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::DROP_PACKET_NOT_APPLICATION_TYPE" << endl;
        drop(p, 1, UWAPPLICATION_DROP_REASON_UNKNOWN_TYPE);
        incrPktInvalid(); //Increment the number of packet received invalid
        return;
    }
    esn = hrsn + 1; // Increase the expected sequence number

    //Verify if the data packet is already processed. 
    if (!useDropOutOfOrder()) {
        if (sn_check[uwApph->sn_ & 0x00ffffff]) { // Packet already processed: drop it
            if (debug_ >= 0 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::DROP_PACKET_PACKET_ALREADY_PROCESSED_ID_" << (int)uwApph->sn_ << endl;
            if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::DROP_PACKET_PACKET_ALREADY_PROCESSED_ID_" << (int)uwApph->sn_ << endl;
            incrPktInvalid();
            drop(p, 1, UWAPPLICATION_DROP_REASON_DUPLICATED_PACKET);
            return;
        }
    }
    //The data packet with this particular SN is not already processed
    //Set true sn_check. In this way we assume that these sn are already processed by the node
    sn_check[uwApph->sn_ & 0x00ffffff] = true;

    //The data packet received is out of order
    if (useDropOutOfOrder()) {
        if (uwApph->sn_ < esn) { // packet is out of sequence and is to be discarded
            incrPktOoseq(); //Increase the number of data packets receive out of sequence.
            if (debug_ >= 0 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::DROP_PACKET_PACKET_OOS_ID_" << (int)uwApph->sn_ << "_LAST_SN_" << hrsn << endl;
            if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::DROP_PACKET_PACKET_OOS_ID_" << (int)uwApph->sn_ << "_LAST_SN_" << hrsn << endl;
            drop(p, 1, UWAPPLICATION_DROP_REASON_OUT_OF_SEQUENCE);
            return;
        }
    }

    //Compute the Forward Trip time 
    rftt = Scheduler::instance().clock() - ch->timestamp();
    if ( (uwApph->rftt_valid_)/10000 ) {
        double rtt = rftt + uwApph->rftt();
        updateRTT(rtt);
    }

    updateFTT(rftt); //Update the forward trip time

    hrsn = uwApph->sn_; //Update the highest sequence number received
   
    //Verify if a packet is lost
    if (useDropOutOfOrder()) {
        if (uwApph->sn_ > esn) {
            if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::PACKET_LOST_ID_RECEIVED" << (int)uwApph->sn_ << "_ID_EXPECTED_" << esn << endl;
            if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::PACKET_LOST_ID_RECEIVED" << (int)uwApph->sn_ << "_ID_EXPECTED_" << esn << endl;
            incrPktLost(uwApph->sn_ - (esn));
        }
    }


    double dt = Scheduler::instance().clock() - lrtime;
    //updateThroughput(ch->size(), dt); //Update Throughput
    updateThroughput(uwApph->payload_size(), dt);
    incrPktRecv(); //Increase the number of data packets received

    lrtime = Scheduler::instance().clock(); //Update the time in which the last packet is received.
    if (debug_ >= 0 && socket_active) 
    {
        std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::PAYLOAD_RECEIVED--> ";
        for(int i=0;i<MAX_LENGTH_PAYLOAD;i++)
        {
            cout<<uwApph->payload_msg[i];
        }
    }
    if (debug_ >= 0 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::SN_RECEIVED_" << (int)uwApph->sn_ <<  endl;
    if (debug_ >= 0 ) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::PAYLOAD_SIZE_RECEIVED_" << (int)uwApph->payload_size() << endl;
    if (debug_ >= 1 && !withoutSocket()) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::PAYLOAD_RECEIVED_" << uwApph->payload_msg << endl;

    if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::PAYLOAD_SIZE_RECEIVED_" << (int)uwApph->payload_size() << endl;
    if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::SN_RECEIVED_" << (int)uwApph->sn_ <<  endl;
    if (logging && !withoutSocket()) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::PAYLOAD_RECEIVED_" << uwApph->payload_msg << endl;
    Packet::free(p);
}//end statistics method

void uwApplicationModule::start_generation() {
    if(debug_ >= 1) std::cout << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::START_GENERATION_DATA" << endl;
    if(logging) out_log << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::START_GENERATION_DATA" << endl;
    chkTimerPeriod.resched(getTimeBeforeNextPkt());
} //end start_generation() method

void uwApplicationModule::init_Packet() {
    Packet* p = Packet::alloc();

    double delay = 0;

    hdr_cmn* ch = hdr_cmn::access(p);
    hdr_uwudp* uwudp = hdr_uwudp::access(p);
    hdr_uwip* uwiph = hdr_uwip::access(p);
    hdr_DATA_APPLICATION* uwApph = HDR_DATA_APPLICATION(p);

    //Common header fields
    ch->uid() = uidcnt++; //Increase the id of data packet
    ch->ptype_ = PT_DATA_APPLICATION; //Assign the type of packet that is being created
    ch->size() = payloadsize; //Assign the size of data payload 
    uwApph->payload_size() = payloadsize;
    ch->direction() = hdr_cmn::DOWN; //The packet must be forward at the level above of him

    //Transport header fields
    uwudp->dport() = port_num; //Set the destination port

    //IP header fields
    uwiph->daddr() = dst_addr; //Set the IP destination address

    //Application header fields
    incrPktSent();
    uwApph->sn_ = txsn; //Sequence number to the data packet

    if (rftt >= 0) {
        uwApph->rftt_ = (int) (rftt * 10000); //Forward Trip Time
        uwApph->rftt_valid_ = true;
    } else {
        uwApph->rftt_valid_ = false;
    }
    uwApph->priority_ = 0; //Priority of the message

    //Create the payload message
    if (getpayloadsize() < MAX_LENGTH_PAYLOAD) {
        for (int i = 0; i < getpayloadsize(); i++) {
            (*uwApph).payload_msg[i] = rand() % 26 + 'a';
        }
        for (int i = getpayloadsize(); i < MAX_LENGTH_PAYLOAD; i++) {
            (*uwApph).payload_msg[i] = '0';
        }
    } else {
        for (int i = 0; i < MAX_LENGTH_PAYLOAD; i++) {
            (*uwApph).payload_msg[i] = rand() % 26 + 'a';
        }
    }

    //Show the DATA payload generated
    ch->timestamp() = Scheduler::instance().clock();

    //Show some DATA packet information 
    if (debug_ >= 2) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::UID_" << ch->uid_ << endl;
    if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::DEST_" << (int)uwiph->daddr() << endl;
    if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::SIZE_" << (int)uwApph->payload_size() << endl;
    if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::SN_" << (int)uwApph->sn_ << endl;
    if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::SEND_DOWN_PACKET" << endl;

    if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::UID_" << ch->uid_ << endl;
    if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::DEST_" << (int)uwiph->daddr() << endl;
    if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::SIZE_" << (int)uwApph->payload_size() << endl;
    if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::SN_" << (int)uwApph->sn_ << endl;
    if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET::SEND_DOWN_PACKET" << endl;

    sendDown(p, delay);
    chkTimerPeriod.resched(getTimeBeforeNextPkt()); // schedule next transmission
} //end init_Packet() method

void uwApplicationModule::stop() {
    if (withoutSocket()) {
        chkTimerPeriod.force_cancel();
    } else {
        //Close the connection
        if (useTCP()) {
            chkTimerPeriod.force_cancel();
            close(servSockDescr);
        }
    }
} //end stop() method

double uwApplicationModule::getTimeBeforeNextPkt() {
    if (getPeriod() < 0) {
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::ERROR_TIMER SET TO NEGATIVE VALUE" << endl;
        exit(1);
    }
    if (usePoissonTraffic()) {
        //Data packets are generated with a Poisson process
        double u = RNG::defaultrng()->uniform_double();
        double lambda = 1.0 / PERIOD;
        if (debug_ >= 2) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::PACKET_GENERATED_WITH_POISSON_PERIOD_"<<PERIOD<<endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::PACKET_GENERATED_WITH_POISSON_PERIOD_"<<PERIOD<<endl;
        return (-log(u) / lambda);
    } else {
        //Data packets are generated wit a costant bit rate
        if (debug_ >= 2) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::PACKET_GENERATED_WITH_FIXED_PERIOD_"<<PERIOD<<endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::PACKET_GENERATED_WITH_FIXED_PERIOD_"<<PERIOD<<endl;
        return PERIOD;
    }
} //end getTimeBeforeNextPkt() Method

double uwApplicationModule::GetRTT() const {
    return (rttsamples > 0) ? sumrtt / rttsamples : 0;
} //end GetRTT() method

double uwApplicationModule::GetRTTstd() const {
    if (rttsamples > 1) {
        double var = (sumrtt2 - (sumrtt * sumrtt / rttsamples)) / (rttsamples - 1);
        return (sqrt(var));
    } else
        return 0;
} //end GetRTTstd() method

void uwApplicationModule::updateRTT(const double& rtt) {
    sumrtt += rtt;
    sumrtt2 += rtt*rtt;
    rttsamples++;
} //end updateRTT() method

double uwApplicationModule::GetFTT() const {
    return (fttsamples > 0) ? sumftt / fttsamples : 0;
} //end getFTT() method

double uwApplicationModule::GetFTTstd() const {
    if (fttsamples > 1) {
        double var = 0;
        var = (sumftt2 - (sumftt * sumftt / fttsamples)) / (fttsamples - 1);
        if (var > 0)
            return (sqrt(var));
        else return 0;
    } else {
        return 0;
    }
} //end getFTT() method

double uwApplicationModule::GetPER() const {
    if (drop_out_of_order) {
        if ((pkts_recv + pkts_lost) > 0) {
            return ((double) pkts_lost / (double) (pkts_recv + pkts_lost));
        } else {
            return 0;
        }
    } else {
        if (esn > 1)
            return (1 - (double) pkts_recv / (double) (esn - 1));
        else
            return 0;
    }
} //end getPER() method

double uwApplicationModule::GetTHR() const {
    return ((sumdt != 0) ? sumbytes * 8 / sumdt : 0);
}//end GetTHR() method

void uwApplicationModule::updateFTT(const double& ftt) {
    sumftt += ftt;
    sumftt2 += ftt*ftt;
    fttsamples++;
}//end updateFTT() method

void uwApplicationModule::updateThroughput(const int& bytes, const double& dt) {
    sumbytes += bytes;
    sumdt += dt;
}//end updateThroughput() method




void uwApplicationModule::uwSendTimerAppl::expire(Event* e) {
    if (m_->withoutSocket()) {
        //Using a random sequence for data payload
        m_->init_Packet();
    } else {
        //Communication take placing with sockets 
        if (m_->useTCP()) {
            //The protocol that the system is using is TCP 
            m_->init_Packet_TCP();
            m_->chkTimerPeriod.resched(m_->PERIOD);
        } else {
            //The protocol that the system is using is UDP 
            m_->init_Packet_UDP();
            m_->chkTimerPeriod.resched(m_->PERIOD);
        }
    }
} //end expire();
