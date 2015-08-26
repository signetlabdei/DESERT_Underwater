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
 * @file   uwvbr-module.cc
 * @author Giovanni Toso
 * @version 1.1.0
 *
 * \brief Provides the <i>UWVBR</i> class implementation.
 * 
 * Provides the <i>UWVBR</i> class implementation.
 */

#include "uwvbr-module.h"

#include <iostream>
#include <rng.h>

extern packet_t PT_UWVBR;

int hdr_uwvbr::offset_;         /**< Offset used to access in <i>hdr_uwvbr</i> packets header. */

/**
 * Adds the header for <i>hdr_uwvbr</i> packets in ns2.
 */
static class UwVbrPktClass : public PacketHeaderClass {
public:

    UwVbrPktClass() : PacketHeaderClass("PacketHeader/UWVBR", sizeof (hdr_uwvbr)) {
        this->bind();
        bind_offset(&hdr_uwvbr::offset_);
    }
} class_uwvbr_pkt;

/**
 * Adds the module for UwVbrModuleClass in ns2.
 */
static class UwVbrModuleClass : public TclClass {
public:

    UwVbrModuleClass() : TclClass("Module/UW/VBR") {
    }

    TclObject* create(int, const char*const*) {
        return (new UwVbrModule());
    }
} class_module_uwvbr;

void UwVbrSendTimer::expire(Event *e) {
    module->transmit();
}

void UwVbrPeriodSwitcher::expire(Event *e) {
    module->switchPeriod();
}

int UwVbrModule::uidcnt_ = 0; // unique id of the packet generated

UwVbrModule::UwVbrModule()
: dstPort_(0),
dstAddr_(0),
PoissonTraffic_(0),
debug_(0),
txsn(1),
hrsn(0),
pkts_recv(0),
pkts_ooseq(0),
pkts_lost(0),
pkts_invalid(0),
pkts_last_reset(0),
rftt(-1),
srtt(0),
sftt(0),
lrtime(0),
sthr(0),
drop_out_of_order_(0),
period_identifier_(1),
timer_switch_1_(1000),
timer_switch_2_(500),
sendTmr_(this),
period_switcher_(this),
pktSize_(0),
sumrtt(0),
sumrtt2(0),
rttsamples(0),
sumftt(0),
sumftt2(0),
fttsamples(0),
sumbytes(0),
sumdt(0),
esn(0)
{ // binding to TCL variables
    bind("period1_", &period1_);
    bind("period2_", &period2_);
    bind("timer_switch_1_", &timer_switch_1_);
    bind("timer_switch_2_", &timer_switch_2_);
    bind("destPort_", (int*) &dstPort_);
    bind("destAddr_", (int*) &dstAddr_);
    bind("packetSize_", &pktSize_);
    bind("PoissonTraffic_", &PoissonTraffic_);
    bind("debug_", &debug_);
    bind("drop_out_of_order_", &drop_out_of_order_);
    sn_check = new bool[USHRT_MAX];
    for (int i = 0; i < USHRT_MAX; i++) {
        sn_check[i] = false;
    }
}

UwVbrModule::~UwVbrModule() {
}

// TCL command interpreter

int UwVbrModule::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();
    if (argc == 2) {
        if (strcasecmp(argv[1], "start") == 0) { // TCL command to start the packet generation and transmission
            start();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "stop") == 0) {// TCL command to stop the packet generation
            stop();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getrtt") == 0) {
            tcl.resultf("%f", GetRTT());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getftt") == 0) {
            tcl.resultf("%f", GetFTT());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getper") == 0) {
            tcl.resultf("%f", GetPER());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getthr") == 0) {
            tcl.resultf("%f", GetTHR());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getvbrheadersize") == 0) {
            tcl.resultf("%d", this->getVbrHeaderSize());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getrttstd") == 0) {
            tcl.resultf("%f", GetRTTstd());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getfttstd") == 0) {
            tcl.resultf("%f", GetFTTstd());
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getsentpkts") == 0) {
            tcl.resultf("%d", txsn - 1);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "getrecvpkts") == 0) {
            tcl.resultf("%d", pkts_recv);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "sendPkt") == 0) {
            sendPkt();
            return TCL_OK;
        } else if (strcasecmp(argv[1], "resetStats") == 0) {
            resetStats();
            fprintf(stderr, "VbrModule::command() resetStats %s, pkts_last_reset=%d, hrsn=%d, txsn=%d\n", tag_, pkts_last_reset, hrsn, txsn);
            return TCL_OK;
        }
    }
    return Module::command(argc, argv);
}

int UwVbrModule::crLayCommand(ClMessage* m) {
    switch (m->type()) {
        default:
            return Module::crLayCommand(m);
    }
}

void UwVbrModule::initPkt(Packet* p) {
    hdr_cmn* ch = hdr_cmn::access(p);
    ch->uid()   = uidcnt_++;
    ch->ptype() = PT_UWVBR;
    ch->size()  = pktSize_;

    hdr_uwip* uwiph  = hdr_uwip::access(p);
    uwiph->daddr()   = dstAddr_;
    
    hdr_uwudp* uwudp = hdr_uwudp::access(p);
    uwudp->dport()   = dstPort_;

    hdr_uwvbr* uwvbrh = HDR_UWVBR(p);
    uwvbrh->sn()      = txsn++;
    ch->timestamp()   = Scheduler::instance().clock();

    if (rftt >= 0) {
        uwvbrh->rftt() = rftt;
        uwvbrh->rftt_valid() = true;
    } else {
        uwvbrh->rftt_valid() = false;
    }
}

void UwVbrModule::start() {
    sendTmr_.resched(getTimeBeforeNextPkt());
    period_switcher_.resched(timer_switch_1_);
}

void UwVbrModule::sendPkt() {
    double delay = 0;
    Packet* p = Packet::alloc();
    initPkt(p);
    hdr_cmn* ch = hdr_cmn::access(p);
    hdr_uwvbr* uwvbrh = HDR_UWVBR(p);
    if (debug_ > 10)
        printf("VbrModule(%d)::sendPkt, send a pkt (%d) with sn: %d\n", getId(), ch->uid(), uwvbrh->sn());
    sendDown(p, delay);
}

void UwVbrModule::transmit() {
    sendPkt();
    // Schedule next transmission.
    sendTmr_.resched(getTimeBeforeNextPkt());
}

void UwVbrModule::switchPeriod() {
    if (period_identifier_ == 1) {
        period_identifier_ = 2;
        period_switcher_.resched(timer_switch_2_);
    } else {
        period_identifier_ = 1;
        period_switcher_.resched(timer_switch_1_);
    }
}

void UwVbrModule::stop() {
    sendTmr_.force_cancel();
}

void UwVbrModule::recv(Packet* p, Handler* h) {
    recv(p);
}

void UwVbrModule::recv(Packet* p) {
    hdr_cmn* ch = hdr_cmn::access(p);
    if (debug_ > 10)
        printf("VbrModule(%d)::recv(Packet*p,Handler*) pktId %d\n", getId(), ch->uid());

    if (ch->ptype() != PT_UWVBR) {
        drop(p, 1, UWVBR_DROP_REASON_UNKNOWN_TYPE);
        incrPktInvalid();
        return;
    }

    hdr_uwvbr* uwvbrh = HDR_UWVBR(p);

    esn = hrsn + 1; // expected sn
    
    if (!drop_out_of_order_) {
        if (sn_check[uwvbrh->sn()]) { // Packet already processed: drop it
            incrPktInvalid();
            drop(p, 1, UWVBR_DROP_REASON_DUPLICATED_PACKET);
            return;
        }
    }
    
    sn_check[uwvbrh->sn()] = true;

    if (drop_out_of_order_) {
        if (uwvbrh->sn() < esn) { // packet is out of sequence and is to be discarded
            incrPktOoseq();
            if (debug_ > 1) {
                printf("VbrModule::recv() Pkt out of sequence! vbrh->sn=%d\thrsn=%d\tesn=%d\n", uwvbrh->sn(), hrsn, esn);
            }
            drop(p, 1, UWVBR_DROP_REASON_OUT_OF_SEQUENCE);
            return;
        }
    }

    rftt = Scheduler::instance().clock() - ch->timestamp();

    if (uwvbrh->rftt_valid()) {
        double rtt = rftt + uwvbrh->rftt();
        updateRTT(rtt);
    }

    updateFTT(rftt);

    /* a new packet has been received */
    incrPktRecv();

    hrsn = uwvbrh->sn();
    if (drop_out_of_order_) {
        if (uwvbrh->sn() > esn) { // packet losses are observed
            incrPktLost(uwvbrh->sn() - (esn));
        }
    }

    double dt = Scheduler::instance().clock() - lrtime;
    updateThroughput(ch->size(), dt);

    lrtime = Scheduler::instance().clock();

    Packet::free(p);

    if (drop_out_of_order_) {
        if (pkts_lost + pkts_recv + pkts_last_reset != hrsn) {
            fprintf(stderr, "ERROR VbrModule::recv() pkts_lost=%d  pkts_recv=%d  hrsn=%d\n", pkts_lost, pkts_recv, hrsn);
        }
    }
}

double UwVbrModule::GetRTT() const {
    return (rttsamples > 0) ? sumrtt / rttsamples : 0;
}

double UwVbrModule::GetFTT() const {
    return (fttsamples > 0) ? sumftt / fttsamples : 0;
}

double UwVbrModule::GetRTTstd() const {
    if (rttsamples > 1) {
        double var = (sumrtt2 - (sumrtt * sumrtt / rttsamples)) / (rttsamples - 1);
        return (sqrt(var));
    } else {
        return 0;
    }
}

double UwVbrModule::GetFTTstd() const {
    if (fttsamples > 1) {
        double var;
        var = (sumftt2 - (sumftt * sumftt / fttsamples)) / (fttsamples - 1);
        if (var > 0) {
            return (sqrt(var));
        } else {
            return 0;
        }
    } else { 
        return 0;
    }
}

double UwVbrModule::GetPER() const {
    if (drop_out_of_order_) {
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
}

double UwVbrModule::GetTHR() const {
    return ((sumdt != 0) ? sumbytes * 8 / sumdt : 0);
}

void UwVbrModule::updateRTT(const double& rtt) {
    sumrtt += rtt;
    sumrtt2 += rtt*rtt;
    rttsamples++;
}

void UwVbrModule::updateFTT(const double& ftt) {
    sumftt += ftt;
    sumftt2 += ftt*ftt;
    fttsamples++;
}

void UwVbrModule::updateThroughput(const int& bytes, const double& dt) {
    sumbytes += bytes;
    sumdt += dt;

    if (debug_ > 1) {
        cerr << "bytes=" << bytes << "  dt=" << dt << endl;
    }
}

void UwVbrModule::incrPktLost(const int& npkts) {
    pkts_lost += npkts;
}

void UwVbrModule::incrPktRecv() {
    pkts_recv++;
}

void UwVbrModule::incrPktOoseq() {
    pkts_ooseq++;
}

void UwVbrModule::incrPktInvalid() {
    pkts_invalid++;
}

void UwVbrModule::resetStats() {
    pkts_last_reset += pkts_lost + pkts_recv;
    pkts_recv = 0;
    pkts_ooseq = 0;
    pkts_lost = 0;
    srtt = 0;
    sftt = 0;
    sthr = 0;
    rftt = -1;
    sumrtt = 0;
    sumrtt2 = 0;
    rttsamples = 0;
    sumftt = 0;
    sumftt2 = 0;
    fttsamples = 0;
    sumbytes = 0;
    sumdt = 0;
}

double UwVbrModule::getTimeBeforeNextPkt() {
    double period_;
    if (period1_ < 0 || period2_ < 0) {
        fprintf(stderr, "%s : Error : period <= 0", __PRETTY_FUNCTION__);
        exit(1);
    }
    
    if (period_identifier_ == 1) {
        period_ = period1_;
    } else {
        period_ = period2_;
    }
    
    if (PoissonTraffic_) {
        double u = RNG::defaultrng()->uniform_double();
        double lambda = 1 / period_;
        return (-log(u) / lambda);
    } else {
        // VBR
        return period_;
    }
}
