//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwflooding-sec.h
 * @author Giovanni Toso, Alberto Signori
 * @version 1.0.0
 *
 * \brief Flooding based routing protocol with security enhanced
 *
 * Flooding based routing protocol with security mecahnism
 */

#ifndef UWFLOODINGSEC_H
#define UWFLOODINGSEC_H

#define TTL_EQUALS_TO_ZERO \
	"TEZ" /**< Reason for a drop in a <i>UWFLOODING</i> module. */

#include "uwflooding-hdr.h"

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
#include <map>
#include <list>

#include "uwbase_reputation.h"


/** Forward declaration of uwflooding.*/
class UwFloodingSec;

/**
 * This class defines the timer used to check the packet forwarding by the
 * neighbor and handle reputation
 */
class NeighborReputationHandler : public TimerHandler
{
public:

	/**
	 * Class constructor
	 * @param neigh_addr address of the neighbor
	 * @param m pointer to the UwFloding module
	 * @param alpha_val paramer to be used to combine new and old snr values
	 * @param dbg debug variable
	 */
	NeighborReputationHandler(uint8_t neigh_addr, UwFloodingSec* m, 
			double alpha_val, int dbg);

	/**
	 * Class destructor
	 */
	virtual ~NeighborReputationHandler();

	/**
	 * Add in the map the unconfirmed packets with the corresponidg timeout
	 * @param uid unique id of the analyzed packet
	 * @param expire_time expire time for the forwarding of neighbor
	 * @return true if the packet is correctly inserted, false oterwhise
	 */
	bool addUnconfirmedPkt(int uid, double expire_time);

	/**
	 * Check if the packet with the given uid is an unconfirmed one and set
	 * it as confirmed
	 * @param uid unique id if the analyzed packet
	 * @return true if the packet is present, false otherwise
	 */
	bool checkUnconfirmedPkt(int uid);

	/**
	 * Update average SNR
	 * @param val_snr new SNR value
	 * @param last_noise_val value of the noise for the last received packet
	 */
	void updateChannelMetrics(double val_snr, double last_noise_val);

	/**
	 * Update the value of the instantaneous noise
	 * @param inst_noise_val value of the instantaneous noise
	 */
	void updateInstantNoise(double inst_noise_val);

protected:

	/**
	 * Returns as reference the uid of the next packet that id going to expire.
	 * Returns false if there are no packets in the map.
	 * @param uid unique id of the next packet that is going to expire.
	 * @param exp_time expire time of the next packet that is going to expire.
	 * @return false if there is no packets in the map, true otherwise.
	 */
	bool getNextPacket(int& uid, double& exp_time) const;

	/**
	 * Removes packet with an old expire time.
	 */
	void removeOldPackets();

	/**
	 * Method called when the timer expire
	 * @param Event*  pointer to an object of type Event
	 */
	virtual void expire(Event *e);

	UwFloodingSec* module; /**< Pointer to the uwflooding module.*/
	uint8_t neighbor_addr; /**<Address of the considered neighbor.*/
	std::map<int, double> unconfirmed_pkts; /**< Map with the uid of the 
			unconfirmed packets, with the corresponding forwarding timeout.*/
	bool is_running; /**< True if a timer has been already scheduled and not 
						expired yet. */
	int waiting_uid; /** Uid of the packet that should be forwarded.*/
	double avg_snr; /**< Average SNR of the last received packets. */
	double alpha; /**< weight for the new SNR value. */
	bool is_first_pkt; /** Boolean variable to check if the packet is the first 
			received one or not. Used to correctly averaged snr.*/
	double last_noise; /**< Noise power of the last correctly received packet.*/
	double inst_noise; /**< Instantaneous noise level.*/
	int debug; /** Debug variable.*/
};

/**
 * UwFloodingSec class is used to represent the routing layer of a node.
 */
class UwFloodingSec : public Module
{
	friend class NeighborReputationHandler;

public:
	/**
	 * Constructor of UwFloodingSec class.
	 */
	UwFloodingSec();

	/**
	 * Destructor of UwFloodingSec class.
	 */
	virtual ~UwFloodingSec();

protected:
	/*****************************
	 |     Internal Functions    |
	 *****************************/
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not.
	 *
	 */
	virtual int command(int, const char *const *);

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 */
	virtual void recv(Packet *);

	/**
	 * Cross-Layer messages synchronous interpreter.
	 *
	 * @param ClMessage* an instance of ClMessage that represent the message
	 * received
	 * @return <i>0</i> if successful.
	 */
	virtual int recvSyncClMsg(ClMessage *);

	/**
	 * Cross-Layer messages asynchronous interpreter. Used to retrive the IP
	 * od the current node from the IP module.
	 *
	 * @param ClMessage* an instance of ClMessage that represent the message
	 * received and used for the answer.
	 * @return <i>0</i> if successful.
	 */
	virtual int recvAsyncClMsg(ClMessage *);

	/**
	 * Returns a nsaddr_t address from an IP written as a string in the form
	 * "x.x.x.x".
	 *
	 * @param char* IP in string form
	 * @return nsaddr_t that contains the IP converter from the input string
	 */
	static nsaddr_t str2addr(const char *);

	/**
	 * Writes in the Path Trace file the path contained in the Packet
	 *
	 * @param Packet to analyze.
	 */
	virtual void writePathInTrace(const Packet *, const string &);

	/**
	 * Return a string with an IP in the classic form "x.x.x.x" converting an
	 * ns2 nsaddr_t address.
	 *
	 * @param nsaddr_t& ns2 address
	 * @return String that contains a printable IP in the classic form "x.x.x.x"
	 */
	static string printIP(const nsaddr_t &);

	/**
	 * Adds a node in the neighbor map, updating the number of packets received
	 * from that node
	 * @param neighbor_addr IP address of the neighbor
	 */
	void addToNeighbor(uint8_t neighbor_addr);

	/**
	 * Prints the neighbor list togheter with the number of packets received
	 * from the neighbor
	 */
	void printNeighbor();

	/**
	 * Check if the received packets is an unconfirmed one.
	 * @param neighbor_addr IP address of the neighbor
	 * @param uid uinique id of the received packet
	 * @return true if the packets was between the unchecked, false otherwise
	 */
	bool checkUnconfirmedPkt(uint8_t neighbor_addr, int uid);

	/**
	 * Send down packets and start forwarding timer
	 * @param p pointer to the packet
	 * @param delay (optional) delay introduced before transmission [sec.]
	 */
	void sendDown(Packet* p,  double delay=0);

	/**
	 * Send ClMsgStats message when triggered by the reception of an overherd
	 * packets. The ClMsg is sent in unicast only to the module
	 * which sent the trigger message
	 * @param neighbor_addr IP address of the neighbor 
	 */
	virtual void sendStatsClMsg(int neighbor_addr);

	/**
	 * Send ClMsgStats to retreive instantaneous noise. The ClMsg is sent in 
	 * unicast only to the module which sent the trigger message
	 * @param neighbor_addr IP address of the neighbor
	 */
	void retrieveInstantNoise(int neighbor_addr);

private:
	// Variables

	uint8_t ipAddr_;
	int ttl_; /**< Time to leave of the <i>UWFLOODING</i> packets. */
	double maximum_cache_time_; /**< Validity time of a packet entry. */
	int optimize_; /**< Flag used to enable the mechanism to drop packets
					  processed twice. */
	long packets_forwarded_; /**< Number of packets forwarded by this module. */
	bool trace_path_; /**< Flag used to enable or disable the path trace file
						 for nodes, */
	char
			*trace_file_path_name_; /**< Name of the trace file that contains
									   the list of paths of the data packets
									   received. */
	ofstream trace_file_path_; /**< Ofstream used to write the path trace file
								  in the disk. */
	ostringstream osstream_; /**< Used to convert to string. */

	typedef std::map<uint16_t, double> map_packets; /**< Typedef for a packet
													   id: (serial_number,
													   timestamp). */
	typedef std::map<uint8_t, map_packets>
			map_forwarded_packets; /**< Typedef for a map of the packet
									  forwarded (saddr, map_packets). */
	map_forwarded_packets
			my_forwarded_packets_; /**< Map of the packet forwarded. */

	std::map<uint16_t,uint8_t> ttl_traffic_map; /**< Map with ttl per traffic.*/

	bool use_reputation; /**< True if the reputation system is used. */

	typedef std::map<uint8_t, uint> neighbor_map; /**< Typdef for a map of the
						neighbor with the number of packets received from him.*/
	neighbor_map neighbor; /**< Map with the neighbor. */

	typedef std::map<uint8_t, NeighborReputationHandler> neighbor_timer_map;/**< 
						Typedef for the forwardig timer of the neigbors.*/
	neighbor_timer_map neighbor_tmr; /**< Map with the neighbor timer. */

	double fwd_to; /**<Time out within which the forwarding is expected.*/

	uint8_t prev_hop_temp; /**< Previous hop IP address of the last received 
							packet. */

	UwReputationInterface* reputation; /**< Reputation of the neighbor. */
	
	double alpha_snr; /**< Value to be used by the NeighborReputationHandler 
			object to combine new snr values and average snr. */

	bool valid_phy_id; /**< True if the id of the phy layer from which obtain
			the statistics is a valid one.*/
	int stats_phy_id; /**< id of the physical layer from which collect the
			statistics. */
	/**
	 * Copy constructor declared as private. It is not possible to create a new
	 * UwFloodingSec object passing to its constructor another UwFloodingSec object.
	 *
	 * @param UwFloodingSec& UwFloodingSec object.
	 */
	UwFloodingSec(const UwFloodingSec &);

	/**
	 * Get the value of the TTL
	 *
	 * @param p pointer to the packet for which the ttl has to be computed.
	 *
	 * @return the ttl for that packet
	 */
	uint8_t getTTL(Packet* p) const;
};

#endif // UWFLOODING_H
