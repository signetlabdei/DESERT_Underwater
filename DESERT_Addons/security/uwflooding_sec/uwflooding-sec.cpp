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
 * @file   uwflooding-sec.cpp
 * @author Giovanni Toso, Alberto Signori
 * @version 1.0.0
 *
 * \brief Implements UwFlooding class.
 *
 */

#include "uwflooding-sec.h"
#include "uwsecurity_clmsg.h"
#include <clmsg-discovery.h>
#include "uwstats-utilities.h"
#include "uwphysical.h"

extern packet_t PT_UWFLOODING;

int hdr_uwflooding::offset_ = 0; /**< Offset used to access in
									<i>hdr_uwflooding</i> packets header. */

/**
 * Adds the module for SunIPRoutingSink in ns2.
 */
static class UwFloodingModuleClass : public TclClass
{
public:
	UwFloodingModuleClass()
		: TclClass("Module/UW/FLOODINGSEC")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UwFloodingSec());
	}
} class_module_uwflooding;

/**
 * Adds the header for <i>hdr_uwflooding</i> packets in ns2.
 */
static class UwFloodingPktClass : public PacketHeaderClass
{
public:
	UwFloodingPktClass()
		: PacketHeaderClass("PacketHeader/FLOODING", sizeof(hdr_uwflooding))
	{
		this->bind();
		bind_offset(&hdr_uwflooding::offset_);
	}
} class_uwflooding_pkt;


NeighborReputationHandler::NeighborReputationHandler(uint8_t neigh_addr, 
		UwFloodingSec* m, double alpha_val, int dbg)
	: TimerHandler()
	, module(m)
	, neighbor_addr(neigh_addr)
	, unconfirmed_pkts()
	, is_running(false)
	, waiting_uid(-1)
	, avg_snr(0)
	, alpha(alpha_val)
	, is_first_pkt(true)
	, debug(dbg)
{

}

NeighborReputationHandler::~NeighborReputationHandler()
{
}

bool
NeighborReputationHandler::addUnconfirmedPkt(int uid, double expire_time)
{
	std::map<int, double>::iterator it = unconfirmed_pkts.find(uid);
	if (it == unconfirmed_pkts.end()) {
		if (debug)
			std::cout << NOW << "::addUnconfirmedPkt::Node=" 
					<< (int)module->ipAddr_ << "::Neighbor=" 
					<< (int)neighbor_addr << "::uid=" << uid 
					<< "::inserted unconfirmed packet" << std::endl;
		
		unconfirmed_pkts[uid] = expire_time;
		if (!is_running) {
			if (NOW < expire_time) {
				if (debug)
					std::cout << NOW << "::addUnconfirmedPkt::Node=" 
							<< (int) module->ipAddr_ << "::Neighbor=" 
							<< (int) neighbor_addr << "::expire time=" 
							<< expire_time << std::endl;
				resched(expire_time-NOW);
				waiting_uid = uid;
				is_running = true;
			} else {
				std::cerr<<NOW << "NeighborReputationHandler::addUnconfirmedPkt"
						<< "::error:expire time already past"<< std::endl;
			}
		}
		return true;
	}
	return false;
}

bool
NeighborReputationHandler::checkUnconfirmedPkt(int uid)
{
	std::map<int,double>::iterator it_uid = unconfirmed_pkts.find(uid);
	if (it_uid != unconfirmed_pkts.end()) {
		if (debug)
			std::cout << NOW << "::checkUnconfirmedPkt::Node="
					<<(int)module->ipAddr_ << "::Neighbor=" 
					<< (int)neighbor_addr << "::uid=" << uid 
					<< "::removed from the map" << std::endl;
		
		unconfirmed_pkts.erase(it_uid);
		if (module->reputation) { 
			ChannelBasedMetricsReputation metrics = 
					ChannelBasedMetricsReputation(true, avg_snr, last_noise,
							inst_noise);
			module->reputation->updateReputation(neighbor_addr, &metrics);
		}
		if (waiting_uid == uid) { 
			int next_uid;
			double next_time;
			if (getNextPacket(next_uid, next_time)) {
				waiting_uid = next_uid;
				is_running = true;
				resched(next_time - NOW);
				if (debug)
					std::cout << NOW << "::checkUnconfirmedPkt::Node=" 
							<< (int) module->ipAddr_ << "::Neighbor=" 
							<< (int) neighbor_addr << "::next uid=" << next_uid
							<< "::expire time=" << next_time << std::endl;
			} else {
				waiting_uid = -1;
				is_running = false;
				force_cancel();
			}
		}
		return true;
	}
	return false;
}

void
NeighborReputationHandler::expire(Event *e)
{
	if (debug)
		std::cout << NOW << "::NeighborReputationHandler::expire::Node=" 
				<< (int)module->ipAddr_ << "::Neighbor=" << (int)neighbor_addr 
				<< "::expected_uid=" << waiting_uid << "::timer expired" 
				<< std::endl;
	
	unconfirmed_pkts.erase(waiting_uid);
	module->retrieveInstantNoise(neighbor_addr);
	if (module->reputation) {
		ChannelBasedMetricsReputation metrics = 
				ChannelBasedMetricsReputation(false, avg_snr, last_noise,
						inst_noise);
		module->reputation->updateReputation(neighbor_addr, &metrics);
	}
	removeOldPackets();
	int next_uid;
	double next_time;
	if (getNextPacket(next_uid, next_time)) {
		waiting_uid = next_uid;
		is_running = true;
		resched(next_time-NOW);
		if (debug)
			std::cout << NOW << "::NeighborReputationHandler::expire::Node=" 
					<< (int)module->ipAddr_  << "::Neighbor=" 
					<< (int)neighbor_addr << "::next uid=" << next_uid 
					<< "::expire time=" << next_time << std::endl;
	} else {
		waiting_uid = -1;
		is_running = false;
		if (debug)
			std::cout << NOW << "::NeighborReputationHandler::expire::Node=" 
					<< (int)module->ipAddr_ << "::Neighbor=" 
					<< (int)neighbor_addr << "::no packet to schedule" 
					<< std::endl;
	}
}

bool
NeighborReputationHandler::getNextPacket(int& uid, double& exp_time) const
{
	std::map<int, double>::const_iterator it = unconfirmed_pkts.begin();
	if (it != unconfirmed_pkts.end()) {
		for (std::map<int, double>::const_iterator it_f = unconfirmed_pkts.begin();
				it_f != unconfirmed_pkts.end(); it_f++) {
			if (it_f->second < it->second) {
				it = it_f;
			}			
		} 
		if (NOW <= it->second) {
			uid = it->first;
			exp_time = it->second;
			return true;
		}
		return false;
	} else {
		return false;
	}
}

void
NeighborReputationHandler::removeOldPackets()
{
	for (std::map<int, double>::iterator it = unconfirmed_pkts.begin();
			it != unconfirmed_pkts.end(); it++) {
		if (it->second < NOW ) {
			if (debug)
				std::cout << NOW << "::removeOldPackets::Node=" 
						<< (int)module->ipAddr_ << "::Neighbor="
						<< (int)neighbor_addr << "::packet with uid=" 
						<< it->first << "::not forwarded" << std::endl;
		
			unconfirmed_pkts.erase(it);
			if (module->reputation) {
				ChannelBasedMetricsReputation metrics = 
						ChannelBasedMetricsReputation(false, avg_snr,last_noise,
								inst_noise);
				module->reputation->updateReputation(neighbor_addr, &metrics);
			}
		}
	}	
}

void
NeighborReputationHandler::updateChannelMetrics(double val_snr, 
		double last_noise_val)
{
	if (is_first_pkt) {
		avg_snr = val_snr;
		is_first_pkt = false;
	}
	else {
		avg_snr = (1-alpha)*avg_snr + alpha*val_snr;
	}
	last_noise = last_noise_val;
	inst_noise = last_noise_val;
	if (debug)
		std::cout << NOW << "::Node=" << (int)module->ipAddr_ << "::Neighbor=" 
				<< (int)neighbor_addr  <<"::average snr=" << avg_snr 
				<< "::val snr=" << val_snr << std::endl;
}

void
NeighborReputationHandler::updateInstantNoise(double inst_noise_val)
{
	inst_noise = inst_noise_val;
	if (debug)
		std::cout << NOW << "::Node=" << (int)module->ipAddr_ << "::Neighbor=" 
				<< (int)neighbor_addr  << "::last rx noise=" << last_noise
				<<"::inst noise =" << inst_noise << std::endl;
}

UwFloodingSec::UwFloodingSec()
	: ipAddr_(0)
	, ttl_(10)
	, maximum_cache_time_(60)
	, optimize_(1)
	, packets_forwarded_(0)
	, trace_path_(false)
	, trace_file_path_name_((char *) "trace")
	, ttl_traffic_map()
	, use_reputation(false)
	, neighbor()
	, neighbor_tmr()
	, fwd_to(30)
	, reputation(nullptr)
	, alpha_snr(0.5)
	, valid_phy_id(false)
	, stats_phy_id(0)
{ // Binding to TCL variables.
	bind("ttl_", &ttl_);
	bind("maximum_cache_time_", &maximum_cache_time_);
	bind("optimize_", &optimize_);
	bind("forward_timeout_", &fwd_to);
	bind("alpha_snr_", &alpha_snr);
} /* UwFlooding::UwFlooding */

UwFloodingSec::~UwFloodingSec()
{
} /* UwFloodingSec::~UwFloodingSec */

int
UwFloodingSec::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_OVERHEARING_PKT) {
		ClMsgOverhearingPkt *msg = dynamic_cast<ClMsgOverhearingPkt *>(m);
		Packet* pkt;
		if (msg->getPacket(pkt)) {
			hdr_cmn* ch = HDR_CMN(pkt);
			if (use_reputation && !ch->error()) {
				sendStatsClMsg(ch->prev_hop_);
				checkUnconfirmedPkt(ch->prev_hop_, ch->uid());
			}
		}
		return 0;
	}
	return Module::recvSyncClMsg(m);
} /* UwFlooding::recvSyncClMsg */

void 
UwFloodingSec::sendStatsClMsg(int neighbor_addr)
{
	if (valid_phy_id) {
		ClMsgStats stats_clmsg = ClMsgStats(stats_phy_id, UNICAST);
		sendSyncClMsg(&stats_clmsg);
		if (stats_clmsg.getStats()->type_id == (int)StatsEnum::STATS_PHY_LAYER){
			const UwPhysicalStats* stats = dynamic_cast<const UwPhysicalStats*>(
					stats_clmsg.getStats());
			if (stats != 0) {
				if (!stats->has_error) {
					double snr_db = 10 * log10(stats->last_rx_power / 
						(stats->last_noise_power + stats->last_interf_power));
					if (debug_)
						std::cout << NOW << "::Node=" << (int) ipAddr_ 
								<< "::Neighbor=" << neighbor_addr 
								<< "::Received packet with" 
								<< "::rx power=" << stats->last_rx_power 
								<< "::noise=" << stats->last_noise_power 
								<< "::interference=" 
								<< stats->last_interf_power 
								<< "::SNR=" << snr_db << std::endl;
					if (use_reputation) {
						addToNeighbor(neighbor_addr);
						neighbor_timer_map::iterator it = 
								neighbor_tmr.find(neighbor_addr);
						it->second.updateChannelMetrics(snr_db, 
								stats->last_noise_power);
					}
				}
			}
		}
	}
}

void
UwFloodingSec::retrieveInstantNoise(int neighbor_addr)
{
	if (valid_phy_id) {
		ClMsgStats stats_clmsg = ClMsgStats(stats_phy_id, UNICAST);
		sendSyncClMsg(&stats_clmsg);
		if (stats_clmsg.getStats()->type_id == (int)StatsEnum::STATS_PHY_LAYER){
			const UwPhysicalStats* stats = dynamic_cast<const UwPhysicalStats*>(
					stats_clmsg.getStats());
			if (stats != 0) {
				if (use_reputation) {
					neighbor_timer_map::iterator it = 
							neighbor_tmr.find(neighbor_addr);
					if (it != neighbor_tmr.end())
						it->second.updateInstantNoise(stats->instant_noise_power);
				}
			}
		}
	}
}

int
UwFloodingSec::recvAsyncClMsg(ClMessage *m)
{
	return Module::recvAsyncClMsg(m);
} /* UwFlooding::recvAsyncClMsg */

int
UwFloodingSec::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 2) {
		if (strcasecmp(argv[1], "getpacketsforwarded") == 0) {
			tcl.resultf("%lu", packets_forwarded_);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getfloodingheadersize") == 0) {
			tcl.resultf("%d", sizeof(hdr_uwflooding));
			return TCL_OK;
		} else if (strcasecmp(argv[1], "printNeighbor") == 0) {
			printNeighbor();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcasecmp(argv[1], "addr") == 0) {
			ipAddr_ = static_cast<uint8_t>(atoi(argv[2]));
			if (ipAddr_ == 0) {
				fprintf(stderr, "0 is not a valid IP address");
				return TCL_ERROR;
			}
			return TCL_OK;
		} else if (strcasecmp(argv[1], "trace") == 0) {
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
		} else if (strcasecmp(argv[1], "setReputation") == 0) {
			reputation = dynamic_cast<UwReputationInterface *>(TclObject::lookup(argv[2]));
			if (!reputation) {
				return TCL_ERROR;
			}
			use_reputation = true;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setPhyTag") == 0) {
			std::string tag = argv[2];
			ClMsgDiscovery disc_m;
			disc_m.addSenderData((const PlugIn*) this, getLayer(), getId(), 
					getStackId(), name() , getTag());
			sendSyncClMsg(&disc_m);
			DiscoveryStorage phy_layer_storage = disc_m.findTag(argv[2]);
			DiscoveryData phy_layer = (*phy_layer_storage.begin()).second;
			stats_phy_id = phy_layer.getId();
			valid_phy_id = true;
			return TCL_OK;
		}
	} else if (argc == 4) {
		if (strcasecmp(argv[1], "addTtlPerTraffic") == 0) {
			ttl_traffic_map[static_cast<uint16_t>(atoi(argv[2]))] = 
				static_cast<uint8_t>(atoi(argv[3]));
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
} /* UwFlooding::command */

void
UwFloodingSec::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwip *iph = HDR_UWIP(p);
	hdr_uwflooding *flh = HDR_UWFLOODING(p);

	if (!ch->error()) {
		if (ch->direction() == hdr_cmn::UP) {
			if (use_reputation) {
				prev_hop_temp = ch->prev_hop_;
				addToNeighbor(ch->prev_hop_);
			}
			if (trace_path_)
				this->writePathInTrace(p, "RECV_DTA");
			if (iph->daddr() == 0) {
				std::cerr << "Destination address not set." << std::endl;
				if (trace_path_)
					this->writePathInTrace(p, "FREE_DTA");
				Packet::free(p);
				return;
			} else if (iph->daddr() == ipAddr_) {
				flh->ttl()--;
				if (trace_path_)
					this->writePathInTrace(p, "SDUP_DTA");
				sendUp(p);
				return;
			} else if (iph->saddr() == ipAddr_) {
				//                cerr << "I am the source: free." << endl;
				if (trace_path_)
					this->writePathInTrace(p, "FREE_DTA");
				Packet::free(p);
				return;
			} else if (iph->daddr() == UWIP_BROADCAST) {
				// sendUp always: the destination is in broadcast.
				ch->size() -= sizeof(hdr_uwflooding);
				if (trace_path_)
					this->writePathInTrace(p, "SDUP_DTA");
				sendUp(p->copy());

				// SendDown
				ch->direction() = hdr_cmn::DOWN;
				ch->prev_hop_ = ipAddr_;
				ch->next_hop() = UWIP_BROADCAST;
				flh->ttl()--;
				ch->size() += sizeof(hdr_uwflooding);
				if (flh->ttl() <= 0) {
					if (trace_path_)
						this->writePathInTrace(p, "DROP_TTL");
					drop(p, 1, TTL_EQUALS_TO_ZERO);
					return;
				} else {
					if (optimize_) {
						map_forwarded_packets::iterator it2 =
								my_forwarded_packets_.find(iph->saddr());
						if (it2 != my_forwarded_packets_.end()) {
							map_packets::iterator it3 =
									it2->second.find(ch->uid());

							if (it3 == it2->second.end()) { // Known source and
															// new packet -> add
															// it to the map and
															// forward.
								it2->second.insert(std::pair<uint16_t, double>(
										ch->uid(), ch->timestamp()));
								packets_forwarded_++;
								if (trace_path_)
									this->writePathInTrace(p, "FRWD_DTA");
								sendDown(p);
								return;
							} else if (Scheduler::instance().clock() -
											it3->second >
									maximum_cache_time_) { // Packet already
														   // processed by not
														   // valid maximum
														   // cache timer ->
														   // update the cache
														   // time and forward.
								it3->second = Scheduler::instance().clock();
								packets_forwarded_++;
								if (trace_path_)
									this->writePathInTrace(p, "FRWD_DTA");
								sendDown(p);
								return;
							} else {
								if (trace_path_)
									this->writePathInTrace(p, "FREE_DTA");
								Packet::free(p);
								return;
							}
						} else {
							std::map<uint16_t, double> tmp_map;
							tmp_map.insert(std::pair<uint16_t, double>(
									ch->uid(), Scheduler::instance().clock()));
							my_forwarded_packets_.insert(
									std::pair<uint8_t, map_packets>(
											iph->saddr(), tmp_map));
							packets_forwarded_++;
							if (trace_path_)
								this->writePathInTrace(p, "FRWD_DTA");
							sendDown(p);
							return;
						}
					} else {
						packets_forwarded_++;
						if (trace_path_)
							this->writePathInTrace(p, "FRWD_DTA");
						sendDown(p);
						return;
					}
				}
			} else if (iph->daddr() != ipAddr_) {
				// SendDown
				ch->direction() = hdr_cmn::DOWN;
				ch->prev_hop_ = ipAddr_;
				ch->next_hop() = UWIP_BROADCAST;
				flh->ttl()--;
				if (flh->ttl() <= 0) {
					if (trace_path_)
						this->writePathInTrace(p, "DROP_TTL");
					drop(p, 1, TTL_EQUALS_TO_ZERO);
					return;
				} else {
					if (optimize_) {
						map_forwarded_packets::iterator it2 =
								my_forwarded_packets_.find(iph->saddr());
						if (it2 != my_forwarded_packets_.end()) {
							map_packets::iterator it3 =
									it2->second.find(ch->uid());

							if (it3 == it2->second.end()) { // Known source and
															// new packet -> add
															// it to the map and
															// forward.
								it2->second.insert(std::pair<uint16_t, double>(
										ch->uid(), ch->timestamp()));
								packets_forwarded_++;
								if (trace_path_)
									this->writePathInTrace(p, "FRWD_DTA");
								sendDown(p);
								return;
							} else if (Scheduler::instance().clock() -
											it3->second >
									maximum_cache_time_) { // Packet already
														   // processed by not
														   // valid maximum
														   // cache timer ->
														   // update the cache
														   // time and forward.
								it3->second = Scheduler::instance().clock();
								packets_forwarded_++;
								if (trace_path_)
									this->writePathInTrace(p, "FRWD_DTA");
								sendDown(p);
								return;
							} else {
								if (trace_path_)
									this->writePathInTrace(p, "FREE_DTA");
								Packet::free(p);
								return;
							}
						} else {
							std::map<uint16_t, double> tmp_map;
							tmp_map.insert(std::pair<uint16_t, double>(
									ch->uid(), Scheduler::instance().clock()));
							my_forwarded_packets_.insert(
									std::pair<uint8_t, map_packets>(
											iph->saddr(), tmp_map));
							packets_forwarded_++;
							if (trace_path_)
								this->writePathInTrace(p, "FRWD_DTA");
							sendDown(p);
							return;
						}
					} else {
						if (trace_path_)
							this->writePathInTrace(p, "FRWD_DTA");
						sendDown(p);
						return;
					}
				}
			} else {
				cerr << "State machine ERROR." << endl;
				if (trace_path_)
					this->writePathInTrace(p, "FREE_DTA");
				Packet::free(p);
				return;
			}
		} else if (ch->direction() == hdr_cmn::DOWN) {
			if (trace_path_)
				this->writePathInTrace(p, "RECV_DTA");
			if (iph->daddr() == 0) {
				std::cerr << "Destination address equals to 0." << std::endl;
				if (trace_path_)
					this->writePathInTrace(p, "FREE_DTA");
				Packet::free(p);
				return;
			}
			if (iph->daddr() == ipAddr_) {
				if (trace_path_)
					this->writePathInTrace(p, "SDUP_DTA");
				sendUp(p);
				return;
			} else { // iph->daddr() != ipAddr_
				ch->prev_hop_ = ipAddr_;
				ch->next_hop() = UWIP_BROADCAST;
				ch->size() += sizeof(hdr_uwflooding);
				flh->ttl() = getTTL(p);
				if (trace_path_)
					this->writePathInTrace(p, "FRWD_DTA");
				sendDown(p);
				return;
			}
		} else {
			cerr << "Direction different from UP or DOWN." << endl;
			if (trace_path_)
				this->writePathInTrace(p, "FREE_DTA");
			Packet::free(p);
			return;
		}
	} else {
		if (trace_path_)
			this->writePathInTrace(p, "FREE_DTA");
		Packet::free(p);
		return;
	}
} /* UwFlooding::recv */

uint8_t
UwFloodingSec::getTTL(Packet* p) const
{
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
	auto it = ttl_traffic_map.find(uwcbrh->traffic_type());
	if(it != ttl_traffic_map.end()) {
		return it->second;
	}
	return ttl_;
}

void
UwFloodingSec::writePathInTrace(const Packet *p, const string &_info)
{
	hdr_uwip *iph = HDR_UWIP(p);
	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwflooding *flh = HDR_UWFLOODING(p);

	trace_file_path_.open(trace_file_path_name_, fstream::app);
	osstream_.clear();
	osstream_.str("");
	osstream_ << _info;
	osstream_ << '\t';
	osstream_ << Scheduler::instance().clock();
	osstream_ << '\t';
	osstream_ << static_cast<uint32_t>(ch->uid() & 0x0000ffff);
	osstream_ << '\t';
	osstream_ << static_cast<uint32_t>(flh->ttl());
	osstream_ << '\t';
	osstream_ << static_cast<uint32_t>(ch->prev_hop_ & 0x000000ff);
	osstream_ << '\t';
	osstream_ << static_cast<uint32_t>(ch->next_hop() & 0x000000ff);
	osstream_ << '\t';
	osstream_ << static_cast<uint32_t>(iph->saddr());
	osstream_ << '\t';
	osstream_ << static_cast<uint32_t>(iph->daddr());
	osstream_ << '\t';
	osstream_ << ch->direction();
	osstream_ << '\t';
	osstream_ << ch->ptype();
	trace_file_path_ << osstream_.str() << endl;
	trace_file_path_.close();
} /*  UwFlooding::writePathInTrace */

string
UwFloodingSec::printIP(const nsaddr_t &ip_)
{
	stringstream out;
	out << ((ip_ & 0xff000000) >> 24);
	out << ".";
	out << ((ip_ & 0x00ff0000) >> 16);
	out << ".";
	out << ((ip_ & 0x0000ff00) >> 8);
	out << ".";
	out << ((ip_ & 0x000000ff));
	return out.str();
} /* UwFlooding::printIP */


void
UwFloodingSec::addToNeighbor(uint8_t neighbor_addr)
{
	if (reputation) {
		reputation->addNode(neighbor_addr);
	}
	//TODO vedere se togliere neighbor map e usare solo la mappa dei timer
	if (neighbor.find(neighbor_addr) != neighbor.end()) {
		neighbor[neighbor_addr]++;
	} else {
		neighbor[neighbor_addr] = 1;
		NeighborReputationHandler tmp = NeighborReputationHandler(neighbor_addr,
				this, alpha_snr, debug_);
		neighbor_tmr.insert(std::make_pair(neighbor_addr,tmp));
	}
}

void
UwFloodingSec::printNeighbor()
{
	std::cout << "Node IP " << (int)ipAddr_ << std::endl;
	for (neighbor_map::iterator it = neighbor.begin(); it != neighbor.end(); it++) {
		RepStatus status;
		if (reputation) {
			std::cout << "Neighbor=" << (int) it->first << "::received="
					<< it->second << "::" 
					<< reputation->getStatusReport((int) it->first)
					<< std::endl;
		} else {
			std::cout << "Neighbor=" << (int) it->first << "::received="
					<< it->second << std::endl;
		}		
	}
	std::cout << std::endl;
}

void
UwFloodingSec::sendDown(Packet* p,  double delay)
{
	if (use_reputation) {
		hdr_cmn* ch = HDR_CMN(p);
		hdr_uwip* iph = HDR_UWIP(p);
		int uid = ch->uid();
		neighbor_timer_map::iterator it = neighbor_tmr.begin();
		for (; it != neighbor_tmr.end(); it++) {
			if (it->first != iph->daddr() && it->first != prev_hop_temp) {
				it->second.addUnconfirmedPkt(uid, NOW+fwd_to);
			}
		}
		prev_hop_temp = 0; //invalid ip
	}
	Module::sendDown(p);
}

bool
UwFloodingSec::checkUnconfirmedPkt(uint8_t neighbor_addr, int uid)
{
	neighbor_timer_map::iterator it = neighbor_tmr.find(neighbor_addr);
	if (it != neighbor_tmr.end()) {
		return it->second.checkUnconfirmedPkt(uid);
	}
	return false;
}
