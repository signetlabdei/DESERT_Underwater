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
 * @file   uwranging_tdoa.cpp
 * @author Antonio Montanari
 * @version 1.0.0
 *
 * @brief Provides the implementation of the class <i>UwRangingTDOA</i>.
 *
 */

#include "uwranging_tdoa.h"
#include "uwtap_clmsg.h"
#include <clmsg-discovery.h>
#include <clmsg-stats.h>
#include <clsap.h>
#include <cmath>
#include <mac.h>
#include <mmac.h>
#include <tclcl.h>
#include <uwphysical.h>
#include <uwsmposition.h>
#include <uwstats-utilities.h>

// define a macro to print debug messages
// 0: NONE	-	1: ERR	-	2: DBG
#define DEBUG(level, text)                                                    \
	{                                                                         \
		if (debug_tdoa >= level) {                                            \
			std::cout << NOW << " UwRangingTDOA(" << node_id << "): " << text \
					  << std::endl;                                           \
		}                                                                     \
	}
/**
 * Class that represent the binding of the protocol with tcl
 */
static class UwRangingTDOAModuleClass : public TclClass
{

public:
	/**
	 * Constructor of the TDOAGenericModule class
	 */
	UwRangingTDOAModuleClass()
		: TclClass("Module/UW/RANGING_TDOA")
	{
	}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	TclObject *
	create(int, const char *const *)
	{
		return (new UwRangingTDOA());
	}

} class_module_uwrangingtdoa;

void
UwRangeTimer::expire(Event *e)
{
	module->throttleTX();
}

UwRangingTDOA::UwRangingTDOA()
	: Module()
	, scheduler_active(false)
	, debug_tdoa(0)
	, node_id(0)
	, packet_id(0)
	, n_nodes(0)
	, dist_num(0)
	, range_entries(0)
	, poisson_traffic(0)
	, range_pkts_sent(0)
	, range_bytes_sent(0)
	, range_pkts_recv(0)
	, range_pkts_err(0)
	, saved_entries(0)
	, phy_id(0)
	, queue_size(0)
	, send_timer(this)
	, mac2phy_delay(1e-6)
	, max_tt(5)
	, max_age(10e4)
	, range_period(10)
	, delay_start(0)
	, full_pkt_tx_dur(0)
	, soundspeed(1500.)
	, tof_map()
	, entries_mat()
	, times_mat()
	, last_ids()
	, time_of_flights()
	, entries_timestamps()
	, entry_last_tx()
	, tx_timestamp()
	, ber()
{
	bind("n_nodes_", (int *) &n_nodes);
	bind("debug_tdoa", (int *) &debug_tdoa);
	bind("mac2phy_delay_", (double *) &mac2phy_delay);
	bind("max_tt", (double *) &max_tt);
	bind("range_period", (double *) &range_period);
	bind("delay_start_", (double *) &delay_start);
	bind("poisson_traffic", (int *) &poisson_traffic);
	bind("range_entries", (int *) &range_entries);
	bind("soundspeed", (double *) &soundspeed);

	if (n_nodes < 0 || n_nodes > std::numeric_limits<uwrange_node_t>::max()) {
		DEBUG(0, "Negative or too big number of nodes, setting default to 0")
		n_nodes = 0;
	}

	dist_num = (n_nodes * (n_nodes - 1) / 2);
	entries_mat.resize(n_nodes,
			std::vector<std::vector<tdoa_entry>>(
					PKTIDMAX, std::vector<tdoa_entry>(n_nodes, tdoa_entry())));
	times_mat.resize(n_nodes, std::vector<double>(PKTIDMAX, -1.));
	last_ids.resize(n_nodes, -1);
	time_of_flights.resize(dist_num, -1.0);
	entries_timestamps.resize(dist_num, -0.0);
	entry_last_tx.resize(n_nodes, 0.0);
	ber.resize(n_nodes, 0.0);

	tof_map.resize(n_nodes, std::vector<int>(n_nodes, -1));
	int temp = 0;

	for (int ni = 0; ni < n_nodes; ni++) {
		for (int nj = ni + 1; nj < n_nodes; nj++) {
			tof_map[ni][nj] = temp;
			tof_map[nj][ni] = temp++;
		}
	}
}

void
UwRangingTDOA::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);

	if (ch->direction() == hdr_cmn::UP) {
		if (ch->ptype() != PT_UWRANGING_TDOA)
			sendUp(p);
		else
			Packet::free(p); // Already processed in recvSyncClMsg from UwTAP
	} else if (ch->direction() == hdr_cmn::DOWN) {
		sendDown(p);
	}
}

void
UwRangingTDOA::recv(Packet *p, Handler *h)
{
	UwRangingTDOA::recv(p);
}

int
UwRangingTDOA::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_TAP_PKT) {
		ClMsgTapPkt *msg = dynamic_cast<ClMsgTapPkt *>(m);

		if (msg->clmsg_type == clmsg_tap_type::START_TX) {
			Packet *p = msg->pkt;

			if (p) {
				hdr_cmn *ch = HDR_CMN(p);

				if (ch->ptype() == PT_UWRANGING_TDOA) {
					DEBUG(2,
							"recvSyncClMsg::updating deltas on outgoing packet")

					updateHoldoverTime(p, msg->tx_duration);
				}
			}

		} else if (msg->clmsg_type == clmsg_tap_type::END_RX) {
			const Packet *p = msg->pkt;

			if (p) {
				hdr_cmn *ch = HDR_CMN(p);
				if (ch->ptype() == PT_UWRANGING_TDOA) {
					hdr_ranging_tdoa *rangh = HDR_RANGING_TDOA(p);
					int pkt_node_id = rangh->source_node_id;
					int pkt_id = rangh->source_pkt_id;

					if (range_entries <= 0) {
						ClMsgStats stats_clmsg = ClMsgStats(phy_id, UNICAST);

						sendSyncClMsg(&stats_clmsg);

						if (stats_clmsg.getStats()->type_id ==
								(int) StatsEnum::STATS_PHY_LAYER) {
							const UwPhysicalStats *stats =
									dynamic_cast<const UwPhysicalStats *>(
											stats_clmsg.getStats());

							if (stats != 0)
								ber[pkt_node_id] = stats->last_ber;
						}
					}
					if (ch->error()) {
						DEBUG(1,
								" RX ERRORED ping with pkt_token_id "
										<< pkt_id << " from node "
										<< pkt_node_id)

						range_pkts_err++;

					} else {
						range_pkts_recv++;

						DEBUG(2,
								"recvSyncClMsg::RX ping with pkt_token_id "
										<< pkt_id << " from node "
										<< pkt_node_id)

						rangeRX(p);
					}
				}
			}
		}

		return 0;

	} else if (m->type() == CLMSG_TRIGGER_STATS) {
		return 0;
	}

	return Module::recvSyncClMsg(m);
}

double
UwRangingTDOA::entryWeight(int entry, int pkt_size, int mode) const
{
	double weight = 0;

	if (mode == 0) // weight considers only PER
	{
		weight = (pow(1 - ber[entry], pkt_size));

		DEBUG(2,
				"entry " << entry << " PER " << pow(1 - ber[entry], pkt_size)
						 << " age " << NOW - entry_last_tx[entry]
						 << " weight: " << weight)

	} else if (mode == -1) // weight considers only AGE
	{
		weight = (NOW - entry_last_tx[entry]);

		DEBUG(2,
				"entry " << entry << " PER " << pow(1 - ber[entry], pkt_size)
						 << " age " << NOW - entry_last_tx[entry]
						 << " weight: " << weight)

	} else if (mode == -2) // weight considers PER * AGE
	{
		weight = (pow(1 - ber[entry], pkt_size) * (NOW - entry_last_tx[entry]));

		DEBUG(2,
				"entry " << entry << " PER " << pow(1 - ber[entry], pkt_size)
						 << " age " << NOW - entry_last_tx[entry]
						 << " weight: " << weight)
	}

	return weight;
}

int
UwRangingTDOA::calcOptEntries(std::vector<int> *sorted_entries) const
{
	int opt_n_entries = (range_entries > 0 ? range_entries : n_nodes - 1);

	if (range_entries <= 0) {
		std::multimap<double, int, std::greater<double>> sorted_pairs;
		double max_weight = 0.0;

		for (int n_entries = 1; n_entries < n_nodes; n_entries++) {
			sorted_pairs.clear();

			int pkt_size = 8 * hdr_ranging_tdoa::getSize(n_entries);

			for (int entry = 0; entry < n_nodes; entry++) {
				if (entry == node_id)
					continue;

				double w =
						entryWeight(entry, pkt_size, 0); // weight is just PDR

				sorted_pairs.insert(std::pair<double, int>(w, entry));
				DEBUG(2,
						"entryWeight(" << entry << "," << n_entries
									   << ")= " << w << "")
			}

			double weight = 0.0;
			std::multimap<double, int>::iterator iter = sorted_pairs.begin();

			for (int i = 0; i < n_entries;
					i++) // was i < n_entries for considering only best entries
			{
				weight += 1 * (iter->first); // was without n_entries for
											 // considering only best entries
				iter++;
				if (iter == sorted_pairs.end())
					break;
			}

			if (weight > max_weight) {
				max_weight = weight;
				opt_n_entries = n_entries;
			}

			DEBUG(2, "N entries: " << n_entries << " weight: " << weight << "")
		}
	}

	int pkt_size = 8 * hdr_ranging_tdoa::getSize(opt_n_entries);

	std::multimap<double, int, std::greater<double>> sorted_pairs;

	for (int entry = 0; entry < n_nodes; entry++) {
		if (entry == node_id)
			continue;

		sorted_pairs.insert(std::pair<double, int>(
				entryWeight(entry, pkt_size, range_entries), entry));
	}

	std::multimap<double, int>::iterator iter = sorted_pairs.begin();

	if (sorted_entries) {
		for (int i = 0; i < opt_n_entries; i++) {
			sorted_entries->push_back(iter->second);
			iter++;
		}
	}

	DEBUG(2, "calcOptEntries::opt_n_entries = " << opt_n_entries)

	return opt_n_entries;
}

void
UwRangingTDOA::rangeTX()
{
	DEBUG(2, "rangeTX")
	Packet *p = Packet::alloc();
	hdr_cmn *ch = HDR_CMN(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_ranging_tdoa *rangh = HDR_RANGING_TDOA(p);
	ch->ptype() = PT_UWRANGING_TDOA;
	mach->macDA() = MAC_BROADCAST;
	mach->macSA() = node_id;
	mach->ftype() = MF_DATA;
	rangh->source_pkt_id = packet_id;
	rangh->source_node_id = node_id;

	tx_timestamp.emplace((tx_timestamp.begin() + rangh->source_pkt_id), NOW);

	std::vector<int> sorted_entries;
	int j = 0;

	DEBUG(2, "rangeTX::calling calcOptEntries rangeID: " << packet_id)

	calcOptEntries(&sorted_entries);
	saved_entries += (n_nodes - 1) - sorted_entries.size();
	rangh->times_size_ = sorted_entries.size();

	DEBUG(2,
			"rangeTX::OPT SIZE: " << sorted_entries.size()
								  << " n_nodes: " << n_nodes)

	for (auto i : sorted_entries) {
		DEBUG(2,
				"rangeTX::times_mat[" << i << "][" << last_ids[i]
									  << "] = " << times_mat[i][last_ids[i]])
		if (isValid(times_mat[i][last_ids[i]]) && isValid(last_ids[i])) {
			tdoa_entry new_entry =
					tdoa_entry(NOW - times_mat[i][last_ids[i]], i, last_ids[i]);

			(*rangh).times_[j] = new_entry;
			entries_mat[node_id][packet_id][i] = new_entry;
			j++;

			DEBUG(2,
					"rangeTX::entries_mat["
							<< node_id << "][" << packet_id << "][" << i
							<< "].time\t= "
							<< entries_mat[node_id][packet_id][i].time)
		} else {
			DEBUG(1,
					"rangeTX::entry not valid for node "
							<< i << " times_mat:" << times_mat[i][last_ids[i]]
							<< " last_ids:" << last_ids[i])
		}
	}

	ch->size() += rangh->getSize();
	DEBUG(2,
			"rangeTX::queueing range packet with pkt_id: "
					<< (int) rangh->source_pkt_id << " and size: " << ch->size()
					<< " with # entries: " << (int) rangh->times_size_);

	packet_id = (packet_id + 1) % PKTIDMAX;
	last_ids[node_id] = rangh->source_pkt_id;

	if (!isValid(rangh->source_pkt_id))
		DEBUG(1,
				"ERROR! invalid pkt_id: " << rangh->source_pkt_id
										  << " from node "
										  << rangh->source_node_id)

	++queue_size;
	sendDown(p);

	if (UwRangingTDOA::range_period < 0) {
		DEBUG(1,
				"ERROR! packet period set as: " << range_period
												<< " but must be >0!!!")
		range_period = 10;
	}
}

void
UwRangingTDOA::updateHoldoverTime(Packet *p, double tx_duration)
{
	if (p == NULL) {
		DEBUG(1, "ERROR! updateHoldoverTime() called with NULL packet!")
		return;
	}

	hdr_ranging_tdoa *rangh = HDR_RANGING_TDOA(p);
	--queue_size;
	++range_pkts_sent;
	range_bytes_sent += rangh->getSize();

	DEBUG(2,
			"updateHoldoverTime::sending range packet with pkt_id: "
					<< (int) rangh->source_pkt_id)
	for (int i = 0; i < rangh->times_size(); i++) {
		DEBUG(2, "updateHoldoverTime::elem[" << i << "]")
		auto &elem = (*rangh).times_[i];

		DEBUG(2, "updateHoldoverTime::elem.time = " << elem.time)

		elem.time += NOW + tx_duration + mac2phy_delay -
				tx_timestamp[rangh->source_pkt_id];
		entries_mat[node_id][rangh->source_pkt_id][elem.node].time = elem.time;

		DEBUG(2, "updateHoldoverTime::tx_duration = " << tx_duration)
		DEBUG(2, "updateHoldoverTime::mac2phy_delay = " << mac2phy_delay)
		DEBUG(2,
				"updateHoldoverTime::tx_timestamp = "
						<< tx_timestamp[rangh->source_pkt_id])
		DEBUG(2,
				"updateHoldoverTime::entries_mat["
						<< node_id << "][" << (int) rangh->source_pkt_id << "]["
						<< (int) elem.node << "].time = " << elem.time)

		entry_last_tx[elem.node] = NOW;
	}

	times_mat[node_id][rangh->source_pkt_id] = NOW + tx_duration;

	DEBUG(2,
			"updateHoldoverTime::times_mat["
					<< node_id << "][" << (int) rangh->source_pkt_id
					<< "] = " << times_mat[node_id][rangh->source_pkt_id])

	if (rangh->times_size() == n_nodes - 1)
		full_pkt_tx_dur = tx_duration;
}

void
UwRangingTDOA::rangeRX(const Packet *p)
{
	hdr_ranging_tdoa *rangh = HDR_RANGING_TDOA(p);
	int pkt_node_id = rangh->source_node_id;
	int pkt_id = rangh->source_pkt_id;

	times_mat[pkt_node_id][pkt_id] = NOW;
	last_ids[pkt_node_id] = pkt_id;

	if (!isValid(pkt_id))
		DEBUG(1,
				"ERROR! invalid pkt_id: " << pkt_id << " from node "
										  << pkt_node_id)

	for (size_t i = 0; i < entries_mat[pkt_node_id][pkt_id].size(); ++i) {
		// reset all entries relative to the incoming packet present in the
		// matrix
		entries_mat[pkt_node_id][pkt_id][i] = tdoa_entry(-1, i, -1);
	}

	double spherical_tof = -1.;
	double spherical_timestamp = -1.;
	for (int i = 0; i < rangh->times_size(); i++) {
		auto &elem = (*rangh).times_[i];

		entries_mat[pkt_node_id][pkt_id][elem.node] = elem;

		DEBUG(2, "rangeRX::elem[" << i << "]")
		DEBUG(2,
				"rangeRX::entries_mat[" << pkt_node_id << "][" << pkt_id << "]["
										<< (int) elem.node
										<< "].time = " << elem.time)

		if (elem.node == node_id) {
			// if packet referred in elem.id is not too old
			if (NOW - times_mat[node_id][elem.id] < max_age) {
				spherical_tof =
						(NOW - times_mat[node_id][elem.id] - elem.time) / 2.;
				spherical_timestamp = (NOW + times_mat[node_id][elem.id]) / 2.;

				DEBUG(2,
						"rangeRX::times_mat["
								<< node_id << "][" << (int) elem.id
								<< "] = " << times_mat[node_id][elem.id])
				DEBUG(2, "rangeRX::elem.time = " << elem.time)
				DEBUG(2, "rangeRX::spherical_tof = " << spherical_tof)

				if (spherical_tof >= -1e-3 && spherical_tof < max_tt) {
					time_of_flights[tof_map[pkt_node_id][node_id]] =
							std::abs(spherical_tof);
					entries_timestamps[tof_map[pkt_node_id][node_id]] =
							spherical_timestamp;
				} else {
					DEBUG(1, "Negative spherical range tt!!! ");
				}
			}
		}
	}

	for (int i = 0; i < rangh->times_size(); i++) {
		auto &elem = (*rangh).times_[i];

		if (isValid(elem) && elem.node != node_id && isValid(spherical_tof) &&
				isValid(times_mat[elem.node][elem.id]) &&
				isValid(time_of_flights[tof_map[elem.node][node_id]])) {

			DEBUG(2, "rangeRX::elem[" << i << "]")

			double hyperbolic_tof = (NOW -
					times_mat[elem.node][elem.id] // RX time from elem.node
					- spherical_tof // time distance from source node
					+ time_of_flights[tof_map[elem.node]
											 [node_id]] // time distance from
														// elem.node
					- elem.time // holdover between elem.node and source node
					+ mac2phy_delay / (n_nodes - 1));

			DEBUG(2,
					"rangeRX::times_mat[" << (int) elem.node << "]["
										  << (int) elem.id << "] "
										  << times_mat[elem.node][elem.id])
			DEBUG(2, "rangeRX::spherical_tof = " << spherical_tof)
			DEBUG(2,
					"rangeRX::time_of_flights["
							<< tof_map[elem.node][node_id] << "] = "
							<< time_of_flights[tof_map[elem.node][node_id]])
			DEBUG(2, "rangeRX::elem.time = " << elem.time)
			DEBUG(2, "rangeRX::hyperbolic_tof = " << hyperbolic_tof)

			if (hyperbolic_tof >= -1e-3) {
				time_of_flights[tof_map[pkt_node_id][elem.node]] =
						std::abs(hyperbolic_tof);
				entries_timestamps[tof_map[pkt_node_id][elem.node]] = std::min(
						spherical_timestamp,
						entries_timestamps[tof_map[elem.node][node_id]]);
			} else {
				DEBUG(1,
						"Negative hyp range from node "
								<< (int) elem.node << " to " << pkt_node_id);
			}
		}
	}
}

bool
UwRangingTDOA::isValid(double value) const
{
	return (value >= 0);
}

bool
UwRangingTDOA::isValid(const tdoa_entry &entry) const
{
	return (entry.id >= 0 && entry.node >= 0 && entry.time >= 0);
}

int
UwRangingTDOA::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			start();
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "stop") == 0) {
			stop();
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "sendRange") == 0) {
			rangeTX();
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getRangePktsTx") == 0) {
			tcl.resultf("%d", range_pkts_sent);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getRangePktsRx") == 0) {
			tcl.resultf("%d", range_pkts_recv);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getRangeBytesTx") == 0) {
			tcl.resultf("%d", range_bytes_sent);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getRangePktsErr") == 0) {
			tcl.resultf("%d", range_pkts_err);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getFullPktTime") == 0) {
			tcl.resultf("%f", full_pkt_tx_dur);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getSavedEntries") == 0) {
			tcl.resultf("%d", saved_entries);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "calcOptEntries") == 0) {
			tcl.resultf("%d", calcOptEntries());
			return TCL_OK;
		}
	}
	if (argc == 3) {
		if (strcasecmp(argv[1], "setId") == 0) {
			node_id = atoi(argv[2]);

			if (node_id < 0 ||
					node_id > std::numeric_limits<uwrange_node_t>::max())
				return TCL_ERROR;

			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setPHYId") == 0) {
			phy_id = atoi(argv[2]);

			if (phy_id < 0)
				return TCL_ERROR;

			return TCL_OK;
		}
	}
	if (argc == 4) {
		if (strcasecmp(argv[1], "get_distance") == 0) {
			int n1 = atoi(argv[2]);
			int n2 = atoi(argv[3]);

			if (n1 >= 0 && n1 < n_nodes && n2 >= 0 && n2 < n_nodes) {
				if (n1 == n2) {
					tcl.resultf("%.17f", 0.);
				} else {
					tcl.resultf("%.17f", time_of_flights[tof_map[n1][n2]]);
				}
				return TCL_OK;
			}
			return TCL_ERROR;
		}
	}
	return Module::command(argc, argv);
}

void
UwRangingTDOA::start()
{
	scheduler_active = true;
	send_timer.resched(1.0 + delay_start * node_id);
}

void
UwRangingTDOA::stop()
{
	scheduler_active = false;
	send_timer.force_cancel();
}

void
UwRangingTDOA::throttleTX()
{
	if (scheduler_active) {
		int nextTX = range_period;
		if (poisson_traffic) {
			double u = RNG::defaultrng()->uniform_double();
			double lambda = 1. / range_period;
			nextTX = (-log(u) / lambda);
		}

		if (queue_size == 0)
			rangeTX();

		send_timer.resched(nextTX);
	} else {
		send_timer.force_cancel();
	}
}
