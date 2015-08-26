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
 * @file   sun-ipr-node.h
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Dinamic source routing protocol, this file contains Nodes specifications.
 * 
 * Dinamic source routing protocol, this file contains Nodes specifications.
 */

#ifndef SUN_NODE_H
#define	SUN_NODE_H

#include "sun-ipr-common-structures.h"
#include "sun-hdr-ack.h"
#include "sun-hdr-data.h"
#include "sun-hdr-pathestablishment.h"

#include <uwip-module.h>
#include <uwip-clmsg.h>
#include <uwcbr-module.h>

#include "mphy.h"
#include "packet.h"
#include <module.h>
#include <tclcl.h>

#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <iostream>
#include <rng.h>
#include <ctime>
#include <vector>
#include <fstream>

using namespace std;

class SunIPRoutingNode;

/**
 * SinkProbeTimer class is used to handle the validity time of a probe message.
 * When a node receives a probe from a sink this means that it is in the coverage
 * are of the sink. This assumption is valid for a timer managed by a SinkProbeTimer
 * object.
 */
class SinkProbeTimer : public TimerHandler {
public:

    SinkProbeTimer(SunIPRoutingNode* m) : TimerHandler() {
        module = m;
    }

protected:
    
    /**
     * Method invoked when the SinkProbeTimer timer expires.
     */
    virtual void expire(Event *e);
    
    SunIPRoutingNode* module; /**< Pointer to an objet of type SunIPRoutingNode. */
};

/**
 * RemoveHopTableTimer class is used to handle the validity time of hop tables.
 * When a node creates an entry in its routing table, its maximum validity time
 * is managed by a timer. The timer is managed by a RemoveHopTableTimer object.
 */
class RemoveHopTableTimer : public TimerHandler {
    
public:

    RemoveHopTableTimer(SunIPRoutingNode* m) : TimerHandler() {
        module = m;
    }
    
protected:
    SunIPRoutingNode* module; /**< Pointer to an objet of type SunIPRoutingNode. */
    
    /**
     * Method invoked when the RemoveHopTableTimer timer expires.
     */
    virtual void expire(Event *e);
};

///**
// * AckWaiting class is used to handle the timer of acks.
// * When a node sends a packet it waits for and ack
// * for a period managed by an AckWaiting object.
// */
//class AckWaiting : public TimerHandler {
//    
//public:
//    AckWaiting(SunIPRoutingNode* m) : TimerHandler() {
//        module = m;
//    }
//
//protected:
//    SunIPRoutingNode* module; /**< Pointer to an objet of type SunIPRoutingNode. */
//    
//    /**
//     * Method invoked when the AckWaiting timer expires.
//     */
//    virtual void expire(Event *e);
//};

/**
 * BufferTimer class is used to handle the timer of the Buffer.
 */
class BufferTimer : public TimerHandler {
    
public:
    BufferTimer(SunIPRoutingNode* m) : TimerHandler() {
        module = m;
    }

protected:
    SunIPRoutingNode* module; /**< Pointer to an objet of type SunIPRoutingNode. */
    
    /**
     * Method invoked when the BufferTimer timer expires.
     */
    virtual void expire(Event *e);
};

/**
 * SearchPathTimer class is used to handle the timer of Search Path requests.
 */
class SearchPathTimer : public TimerHandler {
    
public:
    SearchPathTimer(SunIPRoutingNode* m) : TimerHandler() {
        module = m;
    }

protected:
    SunIPRoutingNode* module; /**< Pointer to an objet of type SunIPRoutingNode. */
    
    /**
     * Method invoked when the SearchPathTimer timer expires.
     */
    virtual void expire(Event *e);
};

/**
 * SunIPRoutingNode class is used to represent the routing layer of a node.
 */
class SunIPRoutingNode : public Module {
    
    /**
     * Friend class used to implement the timer on Hop Tables.
     * 
     * @see SinkProbeTimer
     */
    friend class RemoveHopTableTimer;
    
    /**
     * Friend class used to implement the timer on Acks.
     * 
     * @see SinkProbeTimer
     */
    friend class AckWaiting;
    
    /**
     * Friend class used to implement the timer on Probes.
     * 
     * @see SinkProbeTimer
     */
    friend class SinkProbeTimer;
    
    /**
     * Friend class used to implement the timer on the Buffer.
     * 
     * @see BufferTimer
     */
    friend class BufferTimer;
    
    /**
     * Friend class used to implement the timer on the Search Path mechanism.
     * 
     * @see SearchPathTimer
     */
    friend class SearchPathTimer;
   
public:
    /**
     * Constructor of SunIPRoutingNode class.
     */
    SunIPRoutingNode();
    
    /**
     * Destructor of SunIPRoutingNode class.
     */
    virtual ~SunIPRoutingNode();
    
    static const int LISTLENGTH = 30; /**< Used by the load metric to set the number of the acks tracked. */
    static const int MINUTE = 60;     /**< Used by the load metric to set the period to consider for the evaluation of the metric. */
    
protected:
    /*****************************
     |     Internal Functions    |
     *****************************/
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int, const char*const*);
    
    /**
     * Performs the reception of packets from upper and lower layers.
     * 
     * @param Packet* Pointer to the packet will be received.
     */
    virtual void recv(Packet*);
    
    /**
     * Cross-Layer messages synchronous interpreter.
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received
     * @return <i>0</i> if successful.
     */
    virtual int recvSyncClMsg(ClMessage*);
    
    /**
     * Cross-Layer messages asynchronous interpreter. Used to retrive the IP
     * od the current node from the IP module.
     * 
     * @param ClMessage* an instance of ClMessage that represent the message received and used for the answer.
     * @return <i>0</i> if successful.
     */
    virtual int recvAsyncClMsg(ClMessage*);
    
    /**
     * Clears all the route information of the current node.
     */
    virtual void clearHops();
    
    /**
     * Initializes a SunIPRoutingNode node.
     * It sends to the lower layers a Sync message asking for the IP of the node.
     * 
     * @see UWIPClMsgReqAddr(int src)
     * @see sendSyncClMsgDown(ClMessage* m)
     */
    virtual void initialize();
    
    /**
     * Prints in the stdout the routing table of the current node.
     */
    virtual void printHopTable() const;
    
    /**
     * Returns a string with an IP in the classic form "x.x.x.x" converting an ns2 nsaddr_t address.
     * 
     * @param nsaddr_t& ns2 address
     * @return String that contains a printable IP in the classic form "x.x.x.x"
     */
    static string printIP(const nsaddr_t&);
    
    /**
     * Returns a string with an IP in the classic form "x.x.x.x" converting an ns2 ns_addr_t address.
     * 
     * @param ns_addr_t& ns2 address
     * @return String that contains a printable IP in the classic form "x.x.x.x"
     */
    static string printIP(const ns_addr_t&);
    
    /**
     * Returns a nsaddr_t address from an IP written as a string in the form "x.x.x.x".
     * 
     * @param char* IP in string form
     * @return nsaddr_t that contains the IP converter from the input string
     */
    static nsaddr_t str2addr(const char*);
    
    /**
     * Returns a delay value to use in transmission. The delay can be
     * <i>0</i> or poissonian accordingly with the flag PoissonTraffic_.
     * 
     * @return Transmission delay.
     * @see PoissonTraffic_
     * @see periodPoissonTraffic_
     */
    inline const double getDelay(const double& period_) const {
        if (PoissonTraffic_)
            return (-log(RNG::defaultrng()->uniform_double()) / (1.0 / period_));
        else
            return 0;
    }
    
    /**
     * Evaluates is the number passed as input is equal to zero. When C++ works with
     * double and float number you can't compare them with 0. If the absolute
     * value of the number is smaller than eplison that means that the number is
     * equal to zero.
     * 
     * @param double& Number to evaluate.
     * @return <i>true</i> if the number passed in input is equal to zero, <i>false</i> otherwise.
     * @see std::numeric_limits<double>::epsilon()
     */
    inline const bool isZero(const double& value) const { return std::fabs(value) < std::numeric_limits<double>::epsilon(); }
    
    
    /**
     * Sets the number of hops that the current node needs to reach the sink.
     * 
     * @param int& Number of hops to the sink. The value must be greater or equal to 1.
     * @return Number of hops that separate the current node to the sink.
     */
    virtual const int& setNumberOfHopToSink(const int&);
    
    /**
     * Returns the number of hops that separate the node to the sink.
     * <ul>
     *  <li>0 means no routing information;
     *  <li>1 means directly connected with the sink;
     *  <li>a value > 1 correspond to the number of hops;
     * </ul>
     * @return The number of hops that separate the node to the sink.
     * @see SunIPRoutingNode::getNumberOfHopToSink()
     */
    virtual const int& getNumberOfHopToSink() const;
    
    /**
     * Invoked when the node receives a probe packet from a sink. It is used to
     * update the information about routes.
     * 
     * @param Packet* Pointer to a Probe packet.
     * @see SunIPRoutingNode::clearHops()
     * @see SunIPRoutingNode::setNumberOfHopToSink(const int&)
     */
    virtual void receiveProbe(const Packet*);

    /*****************************
     | Path Establishment Search |
     *****************************/
    /**
     * Sends a Path Establishment Packet with the option field sets to Search.
     * It also remove all the information about current routes to the sink.
     * 
     * @see SunIPRoutingNode::clearHops()
     * @see SunIPRoutingNode::setNumberOfHopToSink(const int&)
     * @see SunIPRoutingNode::initPktPathEstSearch(Packet*)
     */
    virtual void searchPath();
    
    /**
     * Initializes a Path Establishment Search packet (previously allocated).
     * 
     * @param Packet* Pointer to a Path Establishment Packet with the option field set to Search to initialize.
     */
    virtual void initPktPathEstSearch(Packet*) const;
    
    /**
     * Replies to Path Establishment Search packets.
     * It adds in the packet the IP of the current node and forwards or replies to the request.
     * 
     * @param Packet* Pointer to a Path Establishment Search packets to process.
     * @see SunIPRoutingNode::isMyIpInList(const Packet*)
     * @see SunIPRoutingNode::addMyIpInList(Packet*)
     * @see SunIPRoutingNode::updateQuality(Packet*)
     * @see SunIPRoutingNode::getNumberOfHopToSink()
     * @see SunIPRoutingNode::answerPath(const Packet*)
     */
    virtual void replyPathEstSearch(Packet*);
    
    /**
     * Adds the IP of the current node in the header of a Path Establishment
     * packet passed as argument. It can do it if there is at least one free block.
     * The function returns <i>true</i> if it added the IP, <i>false</i> otherwise.
     * 
     * @param Packet* Pointer to a Path Establishment Search packet in which to add an IP.
     * @return <i>true</i> if the IP was added, <i>false</i> otherwise.
     */
    virtual const bool addMyIpInList(Packet*);
    
    /**
     * Checks if the IP of the current node is in the header of the packet
     * passed as argument. If yes it returns <i>true</i>, otherwise it return <i>false</i>.
     * 
     * @param Packet* Pointer to a Path Establishment packet to analyze.
     * @return <i>true</i> if the IP of the current node is in the header, otherwise <i>false</i>.
     */
    virtual const bool isMyIpInList(const Packet*) const;
    
    /**
     * Updates the field quality in the packet passed as parameter.
     * The value written in the packet depends on the metric field.
     * 
     * @param Packet* Pointer to a packet in which to update the quality field.
     */
    virtual void updateQuality(Packet*);
    
    /*****************************
     | Path Establishment Answer |
     *****************************/
    /**
     * Creates and sends an Path Establishment Answer packet.
     * 
     * @param Packet* Pointer to a Path Establishment packet with the option field set to Search.
     * @see SunIPRoutingNode::getNumberOfHopToSink()
     * @see SunIPRoutingNode::initPktPathEstAnswer(Packet*, const Packet*)
     */
    virtual void answerPath(const Packet*); //invoked only by a node with ho count = 1
    
    /**
     * Initializes a Path Establishment Answer packet (previously allocated)
     * retrieving the information from a Path Establishment Request packet.
     * 
     * @param Packet* Pointer to a Path Establishment packet with the option field set to Answer to initialize.
     * @param Packet* Pointer to a Path Establishment packet with the option field set to Search.
     */
    virtual void initPktPathEstAnswer(Packet*, const Packet*); //invoked only by a node with ho count = 1
    
    /**
     * Forwards a Path Establishment Answer Packet. Adds the information about
     * the route in the routing table of the current node.
     * 
     * @param Packet* Pointer to a Path Establishment packet with option field set to ANSWER to forward.
     * @see SunIPRoutingNode::clearHops()
     * @see SunIPRoutingNode::setNumberOfHopToSink(const int&)
     */
    virtual void sendRouteBack(Packet*);
    
    /**
     * Evaluates the route information contained in a Path Establishment packet,
     * and according to different metrics it evaluates if the path contained in
     * the packet is the new best route.
     * 
     * @param Packet* Pointer to a Path Establishment packet to evaluate.
     * @return Number of hops that separate the current node to the sink.
     * @see SunIPRoutingNode::clearHops()
     * @see SunIPRoutingNode::setNumberOfHopToSink(const int&)
     * @see SunIPRoutingNode::getNumberOfHopToSink()
     */
    virtual const int& evaluatePath(const Packet*);

    /*****************************
     |           Data            |
     *****************************/
    /**
     * Initializes a data packet passed as argument with the default values.
     * 
     * @param Packet* Pointer to a packet already allocated to fill with the right values.
     */
    virtual void initPktDataPacket(Packet*);
    
    /**
     * Forwards a data packet to the next hop. All the information to route
     * the packet are contained in the packet. If the current node is in the coverage
     * are of the sink it will forward the packet directly to the sink, otherwise
     * the packet will be forwarded to the next hop.
     * 
     * @param Packet* Pointer to a Data packet to forward.
     */
    virtual void forwardDataPacket(Packet*);

    /*****************************
     |           Acks            |
     *****************************/
    /**
     * Creates an ack packet and sends it to the previous hop using the information
     * contained in the header of the data packet passed as input parameter. It is
     * an ack to the previous hop, and not to the source of the packet.
     * 
     * @param Packet* Pointer to a Data packet to acknowledge.
     * @see SunIPRoutingNode::initPktAck()
     * @see SunIPRoutingNode::clearHops()
     * @see SunIPRoutingNode::setNumberOfHopToSink(const int&)
     */
    virtual void sendBackAck(const Packet*);
    
    /**
     * Initializes an ack packet passed as argument with the default values.
     * 
     * @param Packet* Pointer to a packet already allocated to fill with the right values.
     * @see SunIPRoutingNode::initPktAck(Packet*)
     */
    virtual void initPktAck(Packet*);
    
    /**
     * Used to create a route error packet. This packet will be
     * sent by the current node to the source of the data packet that generated the
     * error.
     * 
     * @param Packet* Pointer to a Data packet that the current node is unable to forward.
     * @param Packet* Pointer to a Route error packet already allocated to initialize with proper values.
     * @see SunIPRoutingNode::initPktPathEstSearch(Packet*)
     */
    virtual void createRouteError(const Packet*, Packet*);
    
    /**
     * Send back an error packet to the previous hop. It uses the information
     * contained in the header of the packet.
     * 
     * @param Packet* Pointer to a Packet to forward to the next hop.
     */
    virtual void sendRouteErrorBack(Packet*);
    
    /*****************************
     |           Load            |
     *****************************/
    /**
     * Updates the number of packets processed by the current node.
     * To be used when the node receives or generates a packet.
     */
    virtual void updatePacketsCount();
    
    /**
     * Updates the number of acks received by the current node.
     * To be used when the node receives an ack.
     */
    virtual void updateAcksCount();
    
    /**
     * Returns the number of packets processed by the current node in the last interval of time (MINUTE).
     * 
     * @return Number of packets processed by the current node in the last interval of time (MINUTE).
     * @see MINUTE.
     */
    virtual const int getPacketsLastMinute() const;
    /**
     * Returns the number of acks received by the current node in the last interval of time (MINUTE).
     * 
     * @return Number of acks received by the current node in the last interval of time (MINUTE).
     * @see MINUTE.
     */
    virtual const int getAcksLastMinute() const;
    
    /**
     * Returns the load index of the current node combining the information from
     * getPacketsLastMinute() and getAcksLastMinute() functions.
     * 
     * @return The load index of the current node.
     * @see SunIPRoutingNode::getPacketsLastMinute()
     * @see SunIPRoutingNode::getAcksLastMinute()
     */
    virtual const double getLoad() const;
    
    /*****************************
     |         Buffering         |
     *****************************/
    /**
     * Manage the buffer of the data packets.
     */
    virtual void bufferManager();
    
    /*****************************
     |        Statistics         |
     *****************************/
    /**
     * Returns the size in byte of a <i>hdr_sun_ack</i> packet header.
     * 
     * @return The size of a <i>hdr_sun_ack</i> packet header.
     */
    static inline const int getAckHeaderSize() { return sizeof(hdr_sun_ack); }
    
    /**
     * Returns the size in byte of a <i>hdr_sun_data</i> packet header.
     * 
     * @return The size of a <i>hdr_sun_data</i> packet header.
     */
    static inline const int getDataPktHeaderSize() { return sizeof(hdr_sun_data); }
    
    /**
     * Returns the size in byte of a <i>hdr_sun_path_est</i> packet header.
     * 
     * @return The size of a <i>hdr_sun_path_est</i> packet header.
     */
    static inline const int getPathEstHeaderSize() { return sizeof(hdr_sun_path_est); }
    
    /**
     * Returns the number of Ack packets processed by the entire network.
     * 
     * @return Number of Ack packets processed by the entire network.
     */
    inline const long& getAckCount() const { return number_of_ackpkt_; }
    
    /**
     * Returns the number of Data packets processed by the entire network.
     * 
     * @return Number of Data packets processed by the entire network.
     */
    inline const long& getDataCount() const { return number_of_datapkt_; }
    
    /**
     * Returns the number of Data packets forwarded by the entire network.
     * 
     * @return Number of Data packets forwarded by the entire network.
     */
    inline const long& getForwardedCount() const { return number_of_pkt_forwarded_; }
    
    /**
     * Returns the number of packets dropped by the entire network for the reason: buffer is full.
     * 
     * @return Number of packets dropped by the entire network.
     */
    inline const long& getDataDropsCountBuffer() const { return number_of_drops_buffer_full_; }
    
    /**
     * Returns the number of packets dropped by the entire network for the reason: maximum number of retransmission reached.
     * 
     * @return Number of packets dropped by the entire network.
     */
    inline const long& getDataDropsCountMaxRetx() const { return number_of_drops_maxretx_; }
    
    /**
     * Returns the number of Path Establishment packets processed by the entire network.
     * 
     * @return Number of Path Establishment Packets processed by the entire network.
     */
    inline const long& getPathEstablishmentCount() const { return number_of_pathestablishment_; }
    
    /*****************************
     |          Timers           |
     *****************************/
    /**
     * This function is invoked when the timer of the routing table expires.
     * It removes all the information about routing information and sink association.
     * 
     * @see SunIPRoutingNode::clearHops()
     * @see SunIPRoutingNode::setNumberOfHopToSink(const int&)
     */
    virtual void expTimerHopTable();
    
    /**
     * This function removes all the information about routing information and sink association.
     * 
     * @see SunIPRoutingNode::clearHops()
     * @see SunIPRoutingNode::setNumberOfHopToSink(const int&)
     */
    virtual void lostSink();
    
    /**
     * This function enable the possibility to send a new Seath Path packet.
     * It is used to avoid too many control packets in network in a high load condition.
     */
    virtual void searchPathExpire();
    
    /*****************************
     |         Trace file         |
     *****************************/
    /**
     * Traces a packet. 
     * 
     * @param Packet to be traced.
     * @param String optional for the packet.
     */
    virtual void tracePacket(const Packet* const, const string& position = "UNDEF___");
    
    /**
     * Function that accepts a list of string and create an entry for the trace file.
     */
    virtual string createTraceString(const string&, const double&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, const double&, const int&, const int&);
    
    /**
     * Opens the trace file, writes the string passed as input and closes the file.
     * 
     * @param String to write in the trace file.
     */
    virtual void writeInTrace(const string&);
    
    enum {
        HOPCOUNT = 1,
        SNR,
        LESSCONGESTED
    };  /**< Different values of packet type for a Path Establishment packet. */
    
    // Variables
    nsaddr_t ipAddr_;                   /**< IP of the current node. */
    int metrics_;                       /**< Metric used by the current node. */
    int PoissonTraffic_;                /**< Enable (<i>1</i>) or disable (<i>0</i>) the Poisson traffic for SUN packets. */
    double period_status_;              /**< Period of the Poisson traffic for status and ack packets. */
    double period_data_;                /**< Period of the Poisson traffic for data packets in the buffer. */
    int num_hop_to_sink;                /**< Number of hops needed to reach the sink using tha path saved tn the routing table.
                                          * If the node is directly connected this value is equal to 1,
                                          * is the node doesn't have any valid path to the sink this value is equal to 0 */
    double quality_link;                /**< Quality of the link from the node to the sink. */
    nsaddr_t* hop_table;                /**< List of IPs to reach the sink. */
    int hop_table_length;               /**< Current length of the hop_table. */
    nsaddr_t sink_associated;           /**< IP of the sink associated to the node. */
    int printDebug_;                    /**< Flag to enable or disable dirrefent levels of debug. */
    double probe_min_snr_;              /**< Value below which if a node receives a probe it discards it. */
    double snr_to_sink_;                /**< SNR between the sink and the current node. */
    bool search_path_enable_;           /**< Flag to enable or disable the possibility to send search_path packets. */
    int disable_path_error_;            /**< Flag to enable or disable the possibility to send <i>Path Error</i> packets. */
    int reset_buffer_if_error_;         /**< If == 1 when a node identify a broken link it will automatically free its buffer. */
    
    // Load
    double list_packets[LISTLENGTH];    /**< List of the last LISTLENGTH temporal instants in which the node received data packets. */
    double list_acks[LISTLENGTH];       /**< List of the last LISTLENGTH temporal instants in which the node received acks. */
    int pointer_packets_;               /**< Pointer of the first avaiable space in list_packets list. */
    int pointer_acks_;                  /**< Pointer of the first avaiable space in list_acks list. */
    long list_packets_max_time_;        /**< Clock of the last packet received by the node. */
    long list_acks_max_time_;           /**< Clock of the last ack received by the node. */
    bool packets_array_full;            /**< <i>true</i> if list of packets is full, <i>false</i> otherwise. */
    bool acks_array_full;               /**< <i>true</i> if list of acks is full, <i>false</i> otherwise. */
    double alpha_;                      /**< Parameters used by Load metric. It is a correlation factor. */
    
    // Acks
    int max_ack_error_;                 /**< Maximum number of Ack errors tollerated by the node. */
    int ack_warnings_counter_;          /**< Number of acks lost since the last reset. */
    bool ack_error_state;               /**< <i>true</i> if the node is not in an error state (that means that it discovered a broken link),
                                         * <i>false</i> if the link to the next hop is considered valid.
                                         */
    
    // Buffer
    vector<buffer_element> buffer_data; /**< Buffer used to store data packets. */
    uint32_t buffer_max_size_;          /**< Maximum length of the data buffer. */
    long pkt_stored_;                   /**< Keep track of the total number of packet transmitted. */
    long pkt_tx_;                       /**< Keep track of the total number of packet retransmitted. */
    int safe_timer_buffer_;             /**< Enables a mechanism used to modify the <i>timer_buffer_</i> in case of the sending time is shorter than the time needed to receive acks. */
    
    // Timers
    double timer_route_validity_;       /**< Maximum validity time for a route entry. */
    double timer_sink_probe_validity_;  /**< Maximum validity time for a sink probe. */
    double timer_buffer_;               /**< Timer for buffer management. */
    double timer_search_path_;          /**< Timer for the search path mechanism. */
    
    RemoveHopTableTimer rmhopTableTmr_; /**< RemoveHopTableTimer object. */
    SinkProbeTimer sinkProbeTimer_;     /**< SinkProbeTimer object. */
    BufferTimer bufferTmr_;             /**< BufferTimer object. */
    SearchPathTimer searchPathTmr_;     /**< SearchPathTimer object. */
    
    // Trace file
    bool trace_;                        /**< Flag used to enable or disable the trace file for nodes, */
    char* trace_file_name_;             /**< Name of the trace file writter for the current node. */
    ostringstream osstream_;            /**< Used to convert to string. */
    ofstream trace_file_;               /**< Ofstream used to write the trace file in the disk. */
    char trace_separator_;              /**< Used as separator among elements in an entr of the tracefile. */
    
    // Statistics
    int data_and_hops[MAX_HOP_NUMBER];        /**< Structure that contains the number of data packets sent by the current node to the sink,
                                                * for different values of hop count. It is used for statistics purposes.
                                                */
    static long number_of_pathestablishment_; /**< Comulative number of Path Establishment packets processed by SunIPRoutingNode objects. */
    static long number_of_datapkt_;           /**< Comulative number of Data packets processed by SunIPRoutingNode objects. */
    static long number_of_ackpkt_;            /**< Comulative number of Ack packets processed by SunIPRoutingNode objects. */
    static long number_of_drops_buffer_full_; /**< Comulative number of packets dropped by SunIPRoutingNode objects, reason: the buffer is full. */
    static long number_of_drops_maxretx_;     /**< Comulative number of packets dropped by SunIPRoutingNode objects, reason: max number of retransmission reached. */
    static long number_of_pkt_forwarded_;     /**< Comulative number of Data packets forwarded by the network. */
    
private:
    
    /**
     * Copy constructor declared as private. It is not possible to create a new SunIPRoutingNode object passing to its constructor another SunIPRoutingNode object. 
     * 
     * @param SunIPRoutingNode& SunIPRoutingNode object.
     */
    SunIPRoutingNode(const SunIPRoutingNode&);
};

#endif // SUN_NODE_H
