//
// Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwranging_tdoa.h
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UwRangingTDOA</i>.
 *
 */

#ifndef UWRANGINGTDOA_H
#define UWRANGINGTDOA_H

#include "uwranging_tdoa_hdr.h"
#include <limits>
#include <module.h>
#include <vector>

class UwRangingTDOA;

/**
 * UwRangeTimer class is used to schedule the transmission of ranging packets
 */
class UwRangeTimer : public TimerHandler
{
public:
	UwRangeTimer(UwRangingTDOA *m)
		: TimerHandler()
	{
		module = m;
	}

protected:
	virtual void expire(Event *e);
	UwRangingTDOA *module;
};

/**
 * Class that represents a UwRangingTDOA Node
 */
class UwRangingTDOA : public Module
{

	friend class UwRangeTimer;

public:
	/**
	 * Constructor of the UwRangingTDOA class
	 */
	UwRangingTDOA();

	/**
	 * Destructor of the UwRangingTDOA class
	 */
	virtual ~UwRangingTDOA() = default;

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 */
	virtual void recv(Packet *) override;

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 * @param Handler* Handler.
	 */
	virtual void recv(Packet *p, Handler *h) override;

	/**
	 * Retrieve packets from other modules
	 * @param m ClMessage
	 */
	virtual int recvSyncClMsg(ClMessage *m) override;

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
	 * @return TCL_OK or TCL_ERROR
	 */
	virtual int command(int argc, const char *const *argv) override;

	/**
	 * Start to send packets.
	 */
	virtual void start();

	/**
	 * Stop to send packets.
	 */
	virtual void stop();

protected:
	/**
	 * Check if value is not negative.
	 * @param double Value to check.
	 * @return true if valid, false otherwise
	 */
	virtual bool isValid(double value) const;

	/**
	 * Check if all elements of a tdoa entry are not negative.
	 * @param tdoa_entry Entry to check.
	 * @return true if valid, false otherwise
	 */
	virtual bool isValid(const tdoa_entry &entry) const;

	/**
	 * Send a ranging message down to the MAC layer.
	 */
	virtual void rangeTX();

	/* Performs the reception of a ranging message.
	 * @param Packet* Pointer to the packet received.
	 */
	virtual void rangeRX(const Packet *p);

	/** Update the holdover time with the actual time of transmission.
	 * @param Packet* Pointer to the packet received.
	 * @param double Transmission duration computed by the physical layer.
	 */
	virtual void updateHoldoverTime(Packet *p, double tx_duration);

	/** Compute the optimal number of entries for the ranging message.
	 * @param std::vector<int> Vector of entries to sort.
	 * @return int Number of optimal entries.
	 */
	virtual int calcOptEntries(
			std::vector<int> *sorted_entries = nullptr) const;

	/** Compute the weight of the entry given the packet size in bits.
	 * @param int n-th entries.
	 * @param int size of the n-th entries in bit
	 * @param int mode to use to compute the optimal number of entries
	 */
	virtual double entryWeight(int entry, int pkt_size, int mode) const;

	/** Schedule the transmission of a rangig message. */
	virtual void throttleTX();

	bool scheduler_active; /**< True if the ranging scheduler is been activated
							  with Tcl "start" command. */
	int debug_tdoa;
	int node_id; /**< Id of the current node. */
	int packet_id; /**< id of the last ranging packet sent (= node_id +
					  k*n_nodes)*/
	int n_nodes; /**< Number of nodes. */
	int dist_num; /**< Number of distances each node computes. */
	int range_entries; /**< Number of entries per ranging packet, if <= 0 enable
						  adaptive payload size. */
	int poisson_traffic; /**< If true the ranging packets are sent according to
							a poisson process with rate range_period. */
	int range_pkts_sent; /**< Counter of sent ranging packets. */
	int range_bytes_sent; /**< Counter of sent ranging bytes. */
	int range_pkts_recv; /**< Counter of received ranging packets. */
	int range_pkts_err; /**< Counter of lost ranging packets. */
	int saved_entries; /**< Total number of entries not sent in the ranging
						  packets. */
	int phy_id; /**< Id of the PHY module. */
	int queue_size; /**< Counts the packet sent to the MAC waiting to be
					   transmitted. */
	UwRangeTimer send_timer; /**< Timer that schedules ranging packet
								transmissions. */
	double mac2phy_delay; /** Floating numbers within this value won't be
							 discarded by NNLS. */
	double max_tt; /**< Max travel time between nodes in seconds. */
	double max_age; /**< Max age of a ranging packet in seconds. */
	double range_period; /**< Ranging period in seconds. */
	double delay_start; /**< Starting delay in seconds. */
	double full_pkt_tx_dur; /**< TX duration for a full range packet. */
	double soundspeed; /**< Speed of sound in m/s */

	std::vector<std::vector<int>>
			tof_map; /*< Map between couples of nodes to a distance. */
	std::vector<std::vector<std::vector<tdoa_entry>>>
			entries_mat; /*< Matrix containing the entries for each packet. */
	std::vector<std::vector<double>>
			times_mat; /*< Matrix containing the tx/rx times of each packet. */
	std::vector<int> last_ids; /*< Vector of ids of the last rx/tx packet. */
	std::vector<double>
			time_of_flights; /*< Vector of one way travel times between each
								nodes in the network. */
	std::vector<double>
			entries_timestamps; /*< Vector of the times at which each one way
								   travel time is computed. */
	std::vector<double> entry_last_tx; /**< Vector of the last (actual) TX times
										  of each entry. */
	std::vector<uwrange_time_t> tx_timestamp; /**< Vector of the TX timestamp
												 for each packet to be sent. */
	std::vector<double>
			ber; /*< Vector fo the BER of each channel (symmetry assumption). */

	const size_t PKTIDMAX = std::numeric_limits<uwrange_pkt_t>::
			max(); /**< Max number of ranging packets that can be saved. */
};

#endif
