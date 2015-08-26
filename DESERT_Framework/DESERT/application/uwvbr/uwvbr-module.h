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
 * @file   uwvbr-module.h
 * @author Giovanni Toso
 * @version 1.1.0
 * 
 * \brief Provides the <i>UWVBR</i> packets header description and the definition of the class <i>UWVBR</i>.
 * 
 * Provides the <i>UWVBR</i> packets header description and the definition of the class <i>UWVBR</i>.
 * <i>UWVBR</i> can manage no more than 2^16 packets. If a module generates more
 * than 2^16 packets, they will be dropped.
 */

#ifndef UWVBR_MODULE_H
#define UWVBR_MODULE_H

#include <uwip-module.h>
#include <uwudp-module.h>

#include <module.h>
#include <iostream>
#include <string>
#include <sstream>
#include <climits>

#define UWVBR_DROP_REASON_UNKNOWN_TYPE "UKT"      /**< Reason for a drop in a <i>UWVBR</i> module. */
#define UWVBR_DROP_REASON_OUT_OF_SEQUENCE "OOS"   /**< Reason for a drop in a <i>UWVBR</i> module. */
#define UWVBR_DROP_REASON_DUPLICATED_PACKET "DPK" /**< Reason for a drop in a <i>UWVBR</i> module. */

#define HDR_UWVBR(P)      (hdr_uwvbr::access(P))

using namespace std;

extern packet_t PT_UWVBR;

/**
 * <i>hdr_uwvbr</i> describes <i>UWVBR</i> packets.
 */
typedef struct hdr_uwvbr {
    u_int16_t sn_;      /**< Serial number of the packet. */
    float rftt_;        /**< Forward Trip Time of the packet. */
    bool rftt_valid_;   /**< Flag used to set the validity of the fft field. */

    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset_ variable.
     */
    inline static int& offset() {
        return offset_;
    }

    inline static struct hdr_uwvbr * access(const Packet * p) {
        return (struct hdr_uwvbr*) p->access(offset_);
    }

    /**
     * Reference to the sn_ variable.
     */
    inline u_int16_t& sn() {
        return sn_;
    }
    
    /**
     * Reference to the rftt_valid_ variable.
     */
    inline bool& rftt_valid() {
        return rftt_valid_;
    }
    
    /**
     * Reference to the rftt_ variable.
     */
    inline float& rftt() {
        return (rftt_);
    }
} hdr_uwvbr;


class UwVbrModule;

/**
 * UwVbrSendTimer class is used to handle the scheduling period of <i>UWVBR</i> packets.
 */
class UwVbrSendTimer : public TimerHandler {
public:

    UwVbrSendTimer(UwVbrModule *m) : TimerHandler() {
        module = m;
    }

protected:
    virtual void expire(Event *e);
    UwVbrModule* module;
};

/**
 * UwVbrPeriodSwitcher class is used to handle the switch between different period used by a UwVbrModule.
 */
class UwVbrPeriodSwitcher : public TimerHandler {
public:

    UwVbrPeriodSwitcher(UwVbrModule *m) : TimerHandler() {
        module = m;
    }

protected:
    virtual void expire(Event *e);
    UwVbrModule* module;
};

/**
 * UwVbrModule class is used to manage <i>UWVBR</i> packets, and to collect statistics about them.
 */
class UwVbrModule : public Module {
    friend class UwVbrSendTimer;
    friend class UwVbrPeriodSwitcher;

public:

    /**
     * Constructor of UwVbrModule class.
     */
    UwVbrModule();
    
    /**
     * Destructor of UwVbrModule class.
     */
    virtual ~UwVbrModule();

    /**
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet will be received.
     */
    virtual void recv(Packet*);
    
    /**
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet will be received.
     * @param Handler* Handler.
     */
    virtual void recv(Packet* p, Handler* h);
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int argc, const char*const* argv);
    
    
    virtual int crLayCommand(ClMessage* m);

    /**
     * Returns the mean Round Trip Time.
     * @return Round Trip Time.
     */
    virtual double GetRTT() const;
    
    /**
     * Returns the mean Forward Trip Time.
     * @return Forward Trip Time.
     */
    virtual double GetFTT() const;

    /**
     * Returns the mean Packet Error Rate.
     * @return Packet Error Rate.
     */
    virtual double GetPER() const;
    
    /**
     * Returns the mean Throughput.
     * @return Throughput.
     */
    virtual double GetTHR() const;

    /**
     * Returns the Round Trip Time Standard Deviation.
     * @return Round Trip Time Standard Deviation.
     */
    virtual double GetRTTstd() const;
    
    /**
     * Returns the mean Forward Trip Time Standard Deviation.
     * @return Forward Trip Time Standard Deviation.
     */
    virtual double GetFTTstd() const;

    /**
     * Resets all the statistics of the UWVBR module.
     */
    virtual void resetStats();

protected:
    static int uidcnt_;         /**< Unique id of the packet generated. */
    u_int16_t dstPort_;         /**< Destination port. */
    nsaddr_t dstAddr_;          /**< IP of the destination. */
    
    bool* sn_check;             /**< Used to keep track of the packets already received. */
    
    int PoissonTraffic_;        /**< <i>1</i> if the traffic is generated according to a poissonian distribution. */
    int debug_;                 /**< Flag to enable or disable dirrefent levels of debug. */
    int drop_out_of_order_;     /**< Flag to enable or disable the check for out of order packets. */
    
    int txsn;                   /**< Sequence number of next new packet to be transmitted. */
    int hrsn;                   /**< Highest received sequence number. */
    int pkts_recv;              /**< Total number of received packets. Packet out of sequence are not counted here. */
    int pkts_ooseq;             /**< Total number of packets received out of sequence. */
    int pkts_lost;              /**< Total number of lost packets, including packets received out of sequence. */
    int pkts_invalid;           /**< Total number of invalid packets received. */
    int pkts_last_reset;        /**< for error checking after stats are reset. Set to pkts_lost+pkts_recv each time resetStats is called. */
    
    double rftt;                /**< Forward Trip Time seen for last received packet. */
    double srtt;                /**< Smoothed Round Trip Time, calculated as for TCP. */
    double sftt;                /**< Smoothed Forward Trip Time, calculated as srtt. */
    double lrtime;              /**< Time of last packet reception. */
    double sthr;                /**< Smoothed throughput calculation. */

    int period_identifier_;     /**< Flag used to identify in which one of the period the vbr module is. */
    double period1_;            /**< period between two consecutive packet transmissions (mode 1). */
    double period2_;            /**< period between two consecutive packet transmissions (mode 2). */
    double timer_switch_1_;     /**< Period in witch the node tramsmits with a packet every period1_ seconds. */
    double timer_switch_2_;     /**< Period in witch the node tramsmits with a packet every period2_ seconds. */
    UwVbrSendTimer sendTmr_;    /**< Timer which schedules packet transmissions. */
    UwVbrPeriodSwitcher period_switcher_; /**< Timer which schedules the switch between different sending periods. */
    int pktSize_;               /**< Packet size. */

    /* Cumulative statistics */
    double sumrtt;              /**< Sum of RTT samples. */
    double sumrtt2;             /**< Sum of (RTT^2). */
    int rttsamples;             /**< Number of RTT samples. */

    double sumftt;              /**< Sum of FTT samples. */
    double sumftt2;             /**< Sum of (FTT^2). */
    int fttsamples;             /**< Number of FTT samples. */

    double sumbytes;            /**< Sum of bytes received. */
    double sumdt;               /**< Sum of the delays. */

    uint32_t esn;               /**< Expected serial number. */
    
    /**
     * Initializes a data packet passed as argument with the default values.
     * 
     * @param Packet* Pointer to a packet already allocated to fill with the right values.
     */
    virtual void initPkt(Packet* p);
    
    /**
     * Allocates, initialize and sends a packet.
     * 
     * @see UwVbrModule::initPkt()
     */
    virtual void sendPkt();
    
    /**
     * Creates and transmits a packet and schedules a new transmission.
     * 
     * @see UwVbrModule::sendPkt()
     */
    virtual void transmit();
    
    /**
     * Start to send packets.
     */
    virtual void start();
    
    /**
     * Stop to send packets.
     */
    virtual void stop();
    
    /**
     * Updates the Round Trip Time.
     * 
     * @param double New Round Trip Time entry.
     */
    virtual void updateRTT(const double&);
    
    /**
     * Updates the Forward Trip Time.
     * 
     * @param double New Forward Trip Time entry.
     */
    virtual void updateFTT(const double&);
    
    /**
     * Updates the Throughput.
     * 
     * @param int Bytes of the payload of the last packet received.
     * @param double Delay Time between the last two receipts.
     */
    virtual void updateThroughput(const int&, const double&);
    
    /**
     * Increases the number of packets lost.
     * 
     * @param int Number of packets lost.
     */
    virtual void incrPktLost(const int&);
    
    /**
     * Increases by one the number of received packets.
     */
    virtual void incrPktRecv();
    
    /**
     * Increases by one the number of out of sequence packets received.
     */
    virtual void incrPktOoseq();
    
    /**
     * Increases by one the number of invalid packets.
     */
    virtual void incrPktInvalid();
    
    /**
     * Returns the amount of time to wait before the next transmission. It depends on the PoissonTraffic_ flag.
     * 
     * @return double Value to use as delay for the next transmission.
     * @see PoissonTraffic_
     */
    virtual double getTimeBeforeNextPkt();
    
    /**
     * Switches between the two different states in which UWVBR can operate. 
     */
    virtual void switchPeriod();
    
    /**
     * Returns the size in byte of a <i>hdr_uwvbr</i> packet header.
     * 
     * @return The size of a <i>hdr_uwvbr</i> packet header.
     */
    static inline const int getVbrHeaderSize() { return sizeof(hdr_uwvbr); }
};

#endif // UWVBR_MODULE_H
