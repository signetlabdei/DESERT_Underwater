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
 * \brief Provides the definition of uwApplicationmodule class
 *
 */

#ifndef UWAPPLICATION_MODULE_H
#define	UWAPPLICATION_MODULE_H

#include <uwip-module.h>
#include <uwudp-module.h>

#include <module.h>
#include <iostream>
#include <sstream>
#include <climits>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>

#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <deque>
#include <list>
#include <queue>
#include <rng.h>
#include <fstream>
#include <ostream>


#define UWAPPLICATION_DROP_REASON_UNKNOWN_TYPE "DPUT"   /**< Drop the packet. Packet received is an unknown type*/
#define UWAPPLICATION_DROP_REASON_DUPLICATED_PACKET "DPD" /**< Drop the packet. Packet received is already analyzed*/
#define UWAPPLICATION_DROP_REASON_OUT_OF_SEQUENCE "DOOS"    /**< Drop the packet. Packet received is out of sequence. */

using namespace std;
extern packet_t PT_DATA_APPLICATION; /**< Trigger packet type for UFetch protocol */

class uwApplicationModule : public Module {
    //friend class uwSendTimerAppl;
public:
    /**
     * Constructor of uwApplicationModule class
     */
    uwApplicationModule();
    /**
     * Destructor of uwApplicationModule class
     */
    virtual ~uwApplicationModule();
    /**
     * TCL command interpreter. It implements the following OTCL methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     **/
    virtual int command(int argc, const char*const* argv);

    /**
     * Cross-Layer messages interpreter
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received
     * @return <i>0</i> if successful.
     */
    virtual int crLayCommand(ClMessage* m);
    
    /**
     * Handle the communication between server and client
     * 
     * @param clnSock socket obtained after the accept function and use for the 
     *                communication between server and client
     */
    virtual void handleTCPclient(int clnSock); 

    //virtual void handleUDPclient(int clnSock);

        /**
     * Increase the number of DATA packets stored in the Server queue. This DATA
     * packets will be sent to the below levels of ISO/OSI stack protocol. 
     */
    virtual void incrPktsPushQueue() { pkts_push_queue++; }

    /**
     * Calculate the epoch of the event. Used in sea-trial mode
     * @return the epoch of the system
     */
    inline unsigned long int getEpoch() {return time(NULL);}


    
    int servSockDescr; /**< socket descriptor for server */
    int clnSockDescr; /**< *socket descriptor for client */
    struct sockaddr_in servAddr; /**< Server address */
    struct sockaddr_in clnAddr; /**< Client address */
    int servPort; /**< Server port*/
    std::queue<Packet*> queuePckReadTCP; /**< Queue that store the DATA packets recevied from the client by the server using a TCP protocol*/ 
    std::queue<Packet*> queuePckReadUDP; /**< Queue that store the DATA packets recevied from the client by the server using a UDP protocol*/
    std::ofstream out_log; /**< Variable that handle the file in which the protocol write the statistics */
    bool logging;
    int node_id;
    int exp_id;
protected:
    /**< uwSenderTimer class that manage the timer */
    class uwSendTimerAppl : public TimerHandler {
    public:
        uwSendTimerAppl(uwApplicationModule *m) : TimerHandler() {
            m_ = m;
        }

        virtual ~uwSendTimerAppl() {

        }

    protected:
        virtual void expire(Event *e);
        uwApplicationModule* m_;
    }; //End uwSendTimer class

    /**************************************************************************
     *                          METHODS                                       *  
     **************************************************************************/
    /**
     * Handle the transmission of DATA packets between CBR layer and the below 
     * level 
     */
    virtual void recv(Packet*);
    /**
     * Comupte some statistics as the number of packets sent and receive between
     * two layer, or control if the packet received is out of sequence.
     */
    virtual void statistics(Packet* p);
    /**
     * Start the process to generate DATA packets without sockets. In this case 
     * the payload of DATA packets are filled with a randomly sequence or with 
     * a pattern sequence.
     */
    //virtual void start_generation_pck_wth_socket();
     virtual void start_generation();
    /**
     * Set all the field of the DATA packet that must be send down after the creation
     * to the below level. In this case the payload of DATA packet are generated in
     * a random way.
     */
    //virtual void initialize_DATA_pck_wth_socket();
     virtual void init_Packet();
    /**
     * When socket communication is used, this method establish a connection 
     * between client and server. This is required because a TCP protocol is used.
     */
    virtual int openConnectionTCP();
    /**
     * Set all the field of DATA packet and take from the specific queue the 
     * payload of DATA packet that will be transmitted. After that put down to the 
     * layer below
     */
    //virtual void initialize_DATA_pck_wth_TCP();
     virtual void init_Packet_TCP();
    /**
     * When socket communication is used, this method establish a connection 
     * between client and server. This is required because a UDP protocol is used.
     */
    virtual int openConnectionUDP();
    /**
     * Set all the field of DATA packet and take from the specific queue the 
     * payload of DATA packet that will be transmitted. After that put down to the 
     * layer below
     */
    //virtual void initialize_DATA_pck_wth_UDP();
    virtual void init_Packet_UDP();
    /**
     * Close the socket connection in the case the communication take place with 
     * socket, otherwise stop the execution of the process, so force the cancellation
     * of period time generation.
     */
    virtual void stop();
    /**
     * Verify if the communication take place with socket or the data payload is
     * generated in a randomly way.
     * 
     * @return <i>true</i> communication without socket
     *          <i>false</i> communication with socket  
     */
    //virtual bool withoutSocket() {bool test;SOCKET_CMN == 0 ? test = true : test = false; return test;}
    //virtual bool withoutSocket() {bool test; socket_active == false ? test = true : test = false; return test;}
     virtual bool withoutSocket() {bool test; socket_active == false ? test = true : test = false; return test;}
    /**
     * If the communication take place using sockets verify if the protocol used
     * is TCP or UDP.
     * 
     * @return <i>true</i> socket use TCP protocol
     *          <i>false</i> socket use UDP protocol  
     */
    //virtual bool useTCP() {bool test;TCP_CMN == 1 ? test = true : test = false; return test;}
    virtual bool useTCP() {bool test; tcp_udp == 1 ? test = true : test = false; return test; }
    /**
     * If the communication take place without sockets verify if the data generation
     * period is constant or is choiche in according to a poisson process 
     * 
     * @return <i>true</i> use a Poisson process to generate data
     *          <i>false</i> use a constant period data generation 
     */
    virtual inline bool usePoissonTraffic() {bool test;poisson_traffic == 1 ? test = true : test = false; return test;}
    /**
     * If the communication take place without sockets verify if the data packets
     * received by the server is out of order or not. In the first case discard the 
     * data packet
     * 
     * @return <i>true</i> enable drop out of order
     *          <i>false</i> not enabled drop out of order 
     */
    virtual inline bool useDropOutOfOrder() {bool test;drop_out_of_order == 1 ? test = true : test = false; return test;}

    /**************************************************************************
     *                       METHODS GET and SET                              *
     **************************************************************************/
    /**
     * Increase the sequence number and so the number of packets sent by the server
     */
    virtual inline void incrPktSent() { txsn++;}
    /**
     * Increase the number of DATA packets lost by the server
     */
    virtual inline void incrPktLost(const int& npkts) { pkts_lost += npkts;}
    /**
     * Increase the number of DATA packet correctly received by the server
     */
    virtual inline void incrPktRecv() { pkts_recv++; }
    /**
     * Increase the number of DATA packets received out of order by the server
     */
    virtual inline void incrPktOoseq() { pkts_ooseq++; }
    /**
     * Increse the number of DATA packets received with error by the server
     */
    virtual inline void incrPktInvalid() { pkts_invalid++; }
    /**
     * return the number of packets sent by the server
     * 
     * @return txsn 
     */
    virtual inline int getPktSent() {return txsn - 1; }
    /**
     * return the number of DATA packets lost by the server
     * 
     * @return pkts_lost
     */
    virtual inline int getPktLost() { return pkts_lost; }
    /**
     * return the number of DATA packet correctly received by the server
     * 
     * @return pkts_recv 
     */
    virtual inline int getPktRecv() { return pkts_recv; }
    /**
     * return the number of DATA packets received out of order by the server
     * 
     * @return pkts_ooseq 
     */
    virtual inline int getPktsOOSequence() { return pkts_ooseq; }
    /**
     * return the number of DATA packets received with error by the server
     * 
     * @return pkts_invalid 
     */
    virtual inline int getPktsInvalidRx() { return pkts_invalid; }
    /**
     * return the number of DATA packets sotred in the server queue
     * 
     * @return pkts_push_queue 
     */
    virtual inline int getPktsPushQueue() { return pkts_push_queue; }
    /**
     * return period generation time
     * 
     * @return PERIOD 
     */
    virtual inline double getPeriod() { return PERIOD; }
    /**
     * return the size of DATA packet payload 
     * 
     * @return payloadsize 
     */ 
    virtual inline int getpayloadsize() { return payloadsize; }
    
    /**
     * Compute the DATA generation rate, that can be constant and equal to the PERIOD
     * established by the user, or can occur with a Poisson process.
     * 
     * @return generation period for DATA packets 
     */
    virtual double getTimeBeforeNextPkt();
    /**
     * Returns the average Round Trip Time
     * 
     * @return the average Round Trip Time
     */
    virtual double GetRTT() const;
    /**
     * Return the standard deviation of the Round Trip Time calculated
     * 
     * @return the standard deviation of the Round Trip Time calculated 
     */
    virtual double GetRTTstd() const;
    /**
     * Update the RTT after the reception of a new packet
     * 
     * @param RTT of the current packet received
     */
    virtual void updateRTT(const double& rtt);
    /**
     * Returns the average Forward Trip Time
     * 
     * @return the average Forward Trip Time 
     * 
     */
    virtual double GetFTT() const;
    /**
    /**
     * Return the standard deviation of the Forward Trip Time calculated
     * 
     * @return the standard deviation of the Forward Trip Time calculated 
     */
    virtual double GetFTTstd() const;
    /**
     * Rerturn the Packet Error Rate calculated
     * 
     * @return the Packet Error Rate calculated
     */
    virtual double GetPER() const;
    /**
     * Return the Throughput calculated [bps]
     * 
     * @return Throughput [bps]
     */
    virtual double GetTHR() const;
    /**
     * Update the FTT after the reception of a new packet
     * 
     * @param FTT of the current packet received
     */
    virtual void updateFTT(const double& ftt);
    /**
     * Update the Throughput after the reception of a new packet
     * 
     * @param Throughput of the current packet received
     */
    virtual void updateThroughput(const int& bytes, const double& dt);
    
    /**************************************************************************
     *                          VARIABLES                                     *
     **************************************************************************/
    //TCL VARIABLES
    int debug_; /**< Used for debug purposes <i>1</i> debug activated <i>0</i> debug not activated*/
    int PERIOD; /**< Interval time between two successive generation data packets */
    //int SOCKET_CMN; /**< Enable or not the communication with socket <i>1</i> enabled <i>0</i> not enabled*/
    int poisson_traffic; /**< Enable or not the Poisson process for generation of data packets <i>1</i> enabled <i>0</i> not enabled*/
    int payloadsize; /**< Size of each data packet payaload generated */
    int port_num; /**< Number of the port in which the server provide the service */
    int drop_out_of_order; /**< Enable or not the ordering of data packet received <i>1</i> enabled <i>0</i> not enabled*/
    //int TCP_CMN; /**< Enable or not the use of TCP protocol when is used the socket communication <i>1</i> use TCP <i>0</i> use UDP*/
    uint8_t dst_addr;    /**< IP destination address. */
    
    //TIMER VARIABLES
    uwSendTimerAppl chkTimerPeriod; /**< Timer that schedule the period between two successive generation of DATA packets*/
    
    //STATISTICAL VARIABLES
    bool socket_active;
    string socket_protocol;
    int tcp_udp; //1 for tcp, 0 for udp, -1 for none
    bool* sn_check; /**< Used to keep track of the packets already received. */
    int uidcnt; /**< Identifier counter that identify uniquely the DATA packet generated*/
    int txsn; /**< Transmission sequence number of DATA packet */   
    int rftt; /**< Forward trip time*/
    int pkts_lost; /**< Counter of the packet lost during the transmission */
    int pkts_recv; /**< Counter of the packets correctly received by the server */
    int pkts_ooseq; /**< Counter of the packets received out of order by the server */
    int pkts_invalid; /**< Counter of the packets received with errors by the server */
    int pkts_push_queue; /**< Counter of DATA packets received by server and not yet passed to the below levels of ISO/OSI stack protocol*/
    int pkts_last_reset; /**< Used for error checking after stats are reset. Set to pkts_lost+pkts_recv each time resetStats is called. */
    double lrtime; /**< Time of last packet reception. */
    double sumrtt; /**< Sum of RTT samples. */
    double sumrtt2; /**< Sum of (RTT^2). */
    int rttsamples; /**< Number of RTT samples. */
    double sumftt; /**< Sum of FTT samples. */
    double sumftt2; /**< Sum of (FTT^2). */
    int fttsamples; /**< Number of FTT samples. */
    uint32_t esn; /**< Expected serial number. */
    double sumbytes; /**< Sum of bytes received. */
    double sumdt; /**< Sum of the delays. */
    int hrsn; /**< Highest received sequence number. */
   
    
    
}; //end uwApplication_module class
#endif	/* UWAPPLICATION_MODULE_H */
extern "C" {
    void *read_process_TCP(void* arg);
}

extern "C" {
    void *read_process_UDP(void* arg);
}
