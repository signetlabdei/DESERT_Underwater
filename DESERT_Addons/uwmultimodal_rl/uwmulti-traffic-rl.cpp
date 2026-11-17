//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
 * @file uwmulti-traffic-rl.cpp
 * @author Filippo Donegà and Filippo Bragato
 * @version 1.0.0
 */

#include "uwmulti-traffic-rl.h"
#include "rl/Agent.h"
#include "rl/DDQNAgent.h"
#include "rl/DSGQlearningAgent.h"
#include "rl/DSGSARSAAgent.h"
#include "rl/PPOAgent.h"
#include "uwmulti-traffic-rl-clmsg.h"
#include <torch/torch.h>

#include <algorithm>
#include <clmsg-discovery.h>
#include <clmsg-stats.h>
#include <ip.h>
#include <mac.h>
#include <memory>
#include <mmac-clmsg.h>
#include <mphy_pktheader.h>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <uwcbr-module.h>
#include <uwip-clmsg.h>
#include <uwip-module.h>
#include <uwopticalstats_phy-clmsg.h>
#include <uwphysical.h>
#include <uwstats-utilities.h>

/**
 * Class that represents the binding with the Tcl configuration script
 */
static class UwMultiTrafficRlClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UwMultiTrafficRlClass()
		: TclClass("Module/UW/MULTIMODAL_RL")
	{
	}
	/**
	 * Creates the TCL object needed for the Tcl language interpretation
	 * @return Pointer to a TclObject
	 */
	virtual TclObject *
	create(int, const char *const *)
	{
		return (new UwMultiTrafficRl);
	}
} class_multitraffic_rl;

UwMultiTrafficRl::UwMultiTrafficRl()
	: UwMultiTrafficControl()
	, debug_(0)
	, phy_ids()
	, agent()
	, state_ptr()
	, state_ptr_other()
	, mll_ids()
	, acoustic_queue_pointer()
	, optical_queue_pointer()
	, wait_queue()
	, alpha_(0.0)
	, beta_(0.0)
	, epsilon_(0.0)
	, expire_time_(30.0)
	, mac_addr(-1)
	, info_vec()
	, info_vec_src()
	, tmr(this)
	, probe_tmr(this)
	, finalProcedureTimerPeriod(0.0)
	, mode_(Mode::Multimodal)
	, handled_traffic_type_(-1)
	, rng_seed(0)
	, probing_period_(60.0)
	, mask_type_(0)
	, range_based_distance_(0.0)
	, other_addr_(-1)
	, agent_type_("dsgsarsa")
	, output_path_()
	, learning_index_(0)
	, loss_timeouts_(3, 30.0) // Initialize 3 timeouts (one per traffic type
							  // 0,1,2) with default 30s
	, probing_best_medium_(0)
	, jitter_received_(0.0)
	, jitter_count_(0.0)
{
	info_vec_addr = (uintptr_t) &info_vec;
	state_ptr_addr = (uintptr_t) &state_ptr;
	bind("debug_", &debug_);
	bind("alpha_", &alpha_);
	bind("beta_", &beta_);
	bind("epsilon_", &epsilon_);
	bind("expire_time_", &expire_time_);
	bind("handled_traffic_type_", &handled_traffic_type_);
	bind("rng_seed_", &rng_seed);
	bind("probing_period_", &probing_period_);
	bind("mask_type_", &mask_type_);
	bind("range_based_distance_", &range_based_distance_);
	bind("other_addr_", &other_addr_);
}

static std::unique_ptr<rl::Agent>
createAgent(const std::string &agent_type, int64_t state_dim,
		int64_t action_dim, double alpha, double beta, double epsilon,
		int mac_addr, const std::string &output_path)
{

	rl::AgentConfig config;
	config.state_dim = state_dim;
	config.action_dim = action_dim;
	config.output_path = output_path;
	config.mac_addr = mac_addr;
	// TODO: What's the difference between config.learning_rate and
	// config.alpha? Naming should be changed, we're assigning alpha to
	// learning_rate in the next two cases...

	if (strcasecmp(agent_type.c_str(), "ddqn") == 0) {
		config.learning_rate = alpha;
		return std::make_unique<rl::DDQNAgent>(config);
	}

	if (strcasecmp(agent_type.c_str(), "ppo") == 0) {
		config.learning_rate = alpha;
		return std::make_unique<rl::PPOAgent>(config);
	}

	if (strcasecmp(agent_type.c_str(), "dsgsarsa") == 0) {
		config.alpha = alpha;
		config.epsilon = epsilon;
		return std::make_unique<rl::DSGSARSAAgent>(config);
	}

	if (strcasecmp(agent_type.c_str(), "dsgqlearning") == 0) {
		config.alpha = alpha;
		config.epsilon = epsilon;
		return std::make_unique<rl::DSGQlearningAgent>(config);
	}

	return nullptr;
}

void
UwMultiTrafficRl::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwcbr *cbrh = HDR_UWCBR(p);
	hdr_mac *mach = HDR_MAC(p);
	if (handled_traffic_type_ == cbrh->traffic_type()) {
		// Packet is of the handled traffic type
		if (ch->direction() == hdr_cmn::DOWN) {
			// Packet incoming from the upper layer
			if (ch->next_hop() == other_addr_) {
				// Packet is directed towards the node of my competence
				///////////////////////////////////////////////////////
				// TODO: The ts_ field is already used by packer-common
				// and packer-uwpolling. Another field must be used to
				// make this module work with real modems
				ch->ts_ = NOW;
				///////////////////////////////////////////////////////
				wait_queue.push(p);
				processWaitQueue();
			}
		} else if (ch->direction() == hdr_cmn::UP) {
			// Packet incoming from the lower layer
			if (debug_) {
				std::cout << "UwMultiTrafficRl (" << mac_addr
						  << ")::Received packet " << ch->uid() << " from "
						  << (int) mach->macSA()
						  << ", other node: " << other_addr_ << std::endl;
			}
			if (mach->macSA() == other_addr_) {
				// Packet came from the node of my competence
				for (auto it = info_vec_src->begin(); it != info_vec_src->end();
						it++) {
					if (it->packet_id_ == ch->uid() && it->action_ != 2) {
						it->delay_ = NOW - it->generation_time_;
						it->rx_time_ = NOW;
						it->received_ = true;
						it->completed_ = true;
						it->completion_time_ = NOW;
						it->was_timeout_ = false;
					}
				}
				sendUp(p);
			}
		}
	}
	if (cbrh->traffic_type() == 4) {
		// Packet type: optical probe
		if (ch->direction() == hdr_cmn::DOWN) {
			if (ch->next_hop() == other_addr_) {
				// Packet is directed towards the node of my competence
				// Packet incoming from the upper layer
				// Send to optical PHY
				sendDown(mll_ids[1], p);
			}
		} else if (ch->direction() == hdr_cmn::UP) {
			// Packet incoming from the lower layer
			if (mach->macSA() == other_addr_) {
				// Packet came from the node of my competence
				// Set optical as the best medium for other probing_period_
				// seconds
				probing_best_medium_ = 1;
				probe_tmr.resched(probing_period_);
			}
		}
	}
}

void
UwMultiTrafficRl::processWaitQueue()
{
	// Iterate over wait_queue until it is empty or a packet is re-queued
	// (action 2 = wait is chosen)
	bool re_queue = false;
	while (wait_queue.size() > 0 && re_queue == false) {
		// Get front packet and access its header fields
		Packet *front_packet = wait_queue.front();
		hdr_cmn *cmnh = HDR_CMN(front_packet);
		hdr_uwcbr *cbrh = HDR_UWCBR(front_packet);
		hdr_mac *mach = HDR_MAC(front_packet);
		int daddr = cmnh->next_hop();
		if (debug_) {
			std::cout << "UwMultiTrafficRl (" << mac_addr << ")::Packet "
					  << cmnh->uid()
					  << " incoming from the upper layer directed to " << daddr
					  << std::endl;
		}
		// Ask all discovered physical layers for stats
		for (unsigned i = 0; i < phy_ids.size(); i++) {
			ClMsgStats m = ClMsgStats(phy_ids[i], UNICAST);
			sendSyncClMsg(&m);
			if (debug_) {
				switch (i) {
					case 0:
						std::cout << "UwMultiTrafficRl (" << mac_addr
								  << ")::Asked UwPhysical for stats"
								  << std::endl;
						break;
					case 1:
						std::cout << "UwMultiTrafficRl (" << mac_addr
								  << ")::Asked UwOpticalStats for stats"
								  << std::endl;
						break;
					default:
						break;
				}
			}
			// Stats received from UwPhysical
			if (const UwPhysicalStats *stats =
							dynamic_cast<const UwPhysicalStats *>(
									m.getStats())) {
				if (debug_) {
					std::cout << "UwMultiTrafficRl (" << mac_addr
							  << ")::Received UwPhysicalStats" << std::endl;
				}
				if (*state_ptr_other != nullptr) {
					(*state_ptr_other)
							->setAcousticLastNoise(stats->last_noise_power);
					(*state_ptr_other)
							->setAcousticInstantNoise(
									stats->instant_noise_power);
					(*state_ptr_other)
							->setAcousticLastRxPower(stats->last_rx_power);
				}
			}
			// Stats received from UwOpticalStatsPhy
			if (const UwOpticalStats *stats =
							dynamic_cast<const UwOpticalStats *>(
									m.getStats())) {
				if (debug_) {
					std::cout << "UwMultiTrafficRl (" << mac_addr
							  << ")::Received UwOpticalStats" << std::endl;
				}
				if (*state_ptr_other != nullptr) {
					(*state_ptr_other)
							->setOpticalLastNoise(stats->last_noise_power);
					(*state_ptr_other)
							->setOpticalInstantNoise(
									stats->instant_noise_power);
					(*state_ptr_other)
							->setOpticalLastRxPower(stats->last_rx_power);
					(*state_ptr_other)->setTemperature(stats->temperature);
				}
			}
		}
		// Here, each node updates its own packet-related state entries
		int data_type = cbrh->traffic_type();
		int packet_size = cmnh->size();
		if (state_ptr != nullptr) {
			state_ptr->setDataType(data_type);
			state_ptr->setPacketSize(packet_size);
			if (acoustic_queue_pointer != nullptr)
				state_ptr->setAcousticQueueLength(
						acoustic_queue_pointer->size());
			if (optical_queue_pointer != nullptr)
				state_ptr->setOpticalQueueLength(optical_queue_pointer->size());
			state_ptr->setWaitQueueLength(wait_queue.size());
		}
		Info info = Info(mask_type_);
		info.packet_id_ = cmnh->uid();
		///////////////////////////////////////////////////////
		// TODO: The ts_ field is already used by packer-common
		// and packer-uwpolling. Another field must be used to
		// make this module work with real modems
		info.generation_time_ = cmnh->ts_;
		///////////////////////////////////////////////////////
		info.action_time_ = NOW;
		State s = State(mask_type_);
		memcpy(&s, state_ptr, sizeof(*(state_ptr)));
		info.state_ = s;
		int action = 1;
		if (state_ptr != nullptr) {
			switch (mode_) {
				case Mode::AcousticOnly:
					action = 0;
					break;
				case Mode::OpticalOnly:
					action = 1;
					break;
				case Mode::Multimodal:
					if (agent != nullptr) {
						rl::ActionResult result =
								agent->act(state_ptr->toTensor());
						action = result.action;
						info.log_prob_ = result.log_prob;
						info.value_ = result.value;
					}
					break;
				case Mode::RangeBased:
					if (state_ptr->getDistance() < range_based_distance_)
						action = 1;
					else
						action = 0;
					break;
				case Mode::OpticalProbing:
					action = probing_best_medium_;
					break;
				default:
					action = 0;
					break;
			}
		}
		if (debug_) {
			std::cout << "UwMultiTrafficRl (" << mac_addr
					  << ")::Chosen action: " << action << std::endl;
		}
		// Set reward of the packet to 0 if wait action is chosen for it
		if (action == 2) {
			info.delay_ = 0.0;
		} else {
			if (action == 3) {
				info.delay_ = 0.0;
			}
		}

		info.action_ = action;
		info_vec.push_back(info);

		// Mark dropped and waited packets as immediately completed
		if (action == 2 || action == 3) {
			info_vec.back().completed_ = true;
			info_vec.back().completion_time_ = NOW;
			info_vec.back().was_timeout_ = false;
			info_vec.back().delay_ = 0.0;

			if (debug_) {
				printf("[%f] UwMultiTrafficRl: Packet %d marked as completed "
					   "(%s)\n",
						NOW,
						info_vec.back().packet_id_,
						action == 2 ? "wait" : "drop");
			}
		}

		if (action < 2) {
			// Send down to the chosen lower mll layer
			sendDown(mll_ids[action], front_packet);
			wait_queue.pop();
		}
		if (action == 2) {
			re_queue = true;
		}
		if (action == 3) {
			wait_queue.pop();
		}
	}

	// Update completion status of pending packets due to timeouts
	updateInfoCompletion();

	// Check for completed consecutive Info entries and dispatch transitions to
	// agent
	checkAndLearnFromInfo();
}

bool
compareByRxTime(const Info &a, const Info &b)
{
	return a.rx_time_ < b.rx_time_;
}

static rl::Transition
buildTransition(
		const Info &current, const Info *next_info, double normalized_reward)
{
	rl::Transition transition;
	transition.state = current.state_.toTensor();
	transition.action = current.action_;
	transition.reward = static_cast<float>(
			normalized_reward); // TODO: compute normalized reward
	if (next_info != nullptr) {
		transition.next_state = next_info->state_.toTensor();
		transition.done = false;
	} else {
		transition.next_state = transition.state.clone();
		transition.done = true;
	}
	transition.has_log_prob = false;
	transition.has_value = false;
	return transition;
}

float
UwMultiTrafficRl::getReward(const Info *current, const Info *next_info,
		const Info *prev_info, int traffic_type) const
{
	switch (traffic_type) {
		case 1:
			if (current->action_ == 2) {
				return 0.0F;
			}
			if (current->received_) {
				return static_cast<float>(current->state_.getPacketSize()) /
						static_cast<float>(current->delay_);
			} else {
				if (current->action_ == 0) {
					return static_cast<float>(
								   -current->state_.getPacketSize()) /
							2500.0F;
				} else if (current->action_ == 1) {
					return static_cast<float>(
								   -current->state_.getPacketSize()) /
							4000000.0F;
				} else {
					return -10.0F;
				}
			}
		case 2: {
			double max_jitter = 0.1; // Maximum jitter threshold in seconds
			if (prev_info == nullptr) {
				return 1.0F;
			}
			if (jitter_received_ / jitter_count_ < 0.1) {
				return -100.0F;
			}
			if (current->received_ && prev_info->received_) {
				double jitter = std::abs(current->delay_ - prev_info->delay_);
				return std::max(0.1f,
						static_cast<float>(max_jitter - jitter) /
								static_cast<float>(max_jitter));
			}
			if (current->received_ && !prev_info->received_) {
				return 0.0F;
			}
			if (current->action_ == 2 || current->action_ == 3) {
				return 0.0F;
			}
			if (current->action_ == 0) {
				return static_cast<float>(-current->state_.getPacketSize()) /
						2500.0F;
			}
			if (current->action_ == 1) {
				return static_cast<float>(-current->state_.getPacketSize()) /
						4000000.0F;
			}
		}
		case 3:
			if (current->received_) {
				return 1.0F;
			}
			if (current->action_ == 2) {
				return 0.0F;
			}
			return -1.0F;

		default:
			return 0.0F;
	}
}

Info::Info(int mask_type)
	: packet_id_(0)
	, state_(mask_type)
	, action_(-1)
	, completed_(false)
	, received_(false)
	, was_timeout_(false)
	, generation_time_(-1.0)
	, action_time_(-1.0)
	, rx_time_(-1.0)
	, completion_time_(-1.0)
	, loss_timeout_(30.0)
	, delay_(-1.0)
	, log_prob_(torch::tensor(0.0))
	, value_(torch::tensor(0.0))
{
}

int
UwMultiTrafficRl::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "getInfoVecPtr") == 0) {
			tcl.resultf("%zd", info_vec_addr);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "getStatePtr") == 0) {
			tcl.resultf("%zd", state_ptr_addr);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "stopFinalProcedure") == 0) {
			tmr.force_cancel();
			return TCL_OK;
		}
	}
	if (argc == 3) {
		if (strcasecmp(argv[1], "setOutputPath") == 0) {
			output_path_ = argv[2];
			if (auto legacy_agent =
							dynamic_cast<rl::DSGSARSAAgent *>(agent.get())) {
				legacy_agent->set_output_path(output_path_);
			}
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setAgentType") == 0) {
			agent_type_ = argv[2];
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "addInfoVecPtr") == 0) {
			std::stringstream ss;
			ss << argv[2];
			uintptr_t translated_info_vec_address;
			ss >> translated_info_vec_address;
			info_vec_src = (std::vector<Info> *) (translated_info_vec_address);
			sendInfoVecPtrClmsg();
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "saveModel") == 0) {
			if (agent != nullptr) {
				agent->save(argv[2]);
			}
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "loadModel") == 0) {
			if (agent != nullptr) {
				agent->load(argv[2]);
			}
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "startFinalProcedure") == 0) {
			finalProcedureTimerPeriod = atof(argv[2]);
			tmr.resched(finalProcedureTimerPeriod);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setMode") == 0) {
			int m = atoi(argv[2]);
			switch (m) {
				case 0:
					mode_ = Mode::AcousticOnly;
					break;
				case 1:
					mode_ = Mode::OpticalOnly;
					break;
				case 2:
					mode_ = Mode::Multimodal;
					break;
				case 3:
					mode_ = Mode::RangeBased;
					break;
				case 4:
					mode_ = Mode::OpticalProbing;
					break;
				default:
					mode_ = Mode::Multimodal;
					break;
			}
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "addStatePtr") == 0) {
			std::stringstream ss;
			ss << argv[2];
			uintptr_t translated_state_ptrs_address;
			ss >> translated_state_ptrs_address;
			state_ptr_other = (State **) (translated_state_ptrs_address);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "updateDistance") == 0) {
			state_ptr->setDistance(atof(argv[2]));
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "updateDepth") == 0) {
			state_ptr->setDepth(atof(argv[2]));
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "updateWindspeed") == 0) {
			state_ptr->setWindSpeed(atof(argv[2]));
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "updateShipping") == 0) {
			state_ptr->setShipping(atof(argv[2]));
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "updateC") == 0) {
			state_ptr->setAttenuationCoefficient(atof(argv[2]));
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setOtherAddr") == 0) {
			other_addr_ = atoi(argv[2]);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setTorchSeed") == 0) {
			torch::manual_seed(
					atoi(argv[2])); // Set a fixed seed for reproducibility
			return TCL_OK;
		}
	}
	if (argc == 6) {
		if (strcasecmp(argv[1], "initialize") == 0) {
			initialize(argv[2], atof(argv[3]), atoi(argv[4]), atoi(argv[5]));
			return TCL_OK;
		}
	}
	return UwMultiTrafficControl::command(argc, argv);
}

void
UwMultiTrafficRl::finalProcedure()
{
	processWaitQueue();
	tmr.resched(finalProcedureTimerPeriod);
}

void
UwMultiTrafficRl::discoverPhyLayers()
{
	// Discover all PHY layers in the stack and save their IDs
	ClMsgDiscovery m;
	m.addSenderData((const PlugIn *) this,
			getLayer(),
			getId(),
			getStackId(),
			name(),
			getTag());
	sendSyncClMsg(&m);
	DiscoveryStorage phy_layer_storage;
	std::vector<DiscoveryData> phy_layers;
	bool no_more = false;
	int i = 0;
	while (no_more == false) {
		char buffer[5];
		snprintf(buffer, 5, "PHY%d", i + 1);
		phy_layer_storage = m.findTag(buffer);
		if (phy_layer_storage.begin() != phy_layer_storage.end()) {
			phy_layers.push_back((*phy_layer_storage.begin()).second);
			phy_ids.push_back(phy_layers[i].getId());
			if (debug_) {
				std::cout << "UwMultiTrafficRl::initialize::PHY Id("
						  << phy_layers[i].getId()
						  << ")::Tcl Name = " << phy_layers[i].getTclName()
						  << "::Layer Id = " << phy_layers[i].getLayer()
						  << std::endl;
			}
			i++;
		} else {
			no_more = true;
		}
	}
}

void
UwMultiTrafficRl::discoverMllLayers()
{
	// Discover all MLL layers in the stack and save their IDs
	ClMsgDiscovery m;
	m.addSenderData((const PlugIn *) this,
			getLayer(),
			getId(),
			getStackId(),
			name(),
			getTag());
	sendSyncClMsg(&m);
	DiscoveryStorage mll_layer_storage;
	std::vector<DiscoveryData> mll_layers;
	bool no_more = false;
	int i = 0;
	while (no_more == false) {
		char buffer[5];
		snprintf(buffer, 5, "MLL%d", i + 1);
		mll_layer_storage = m.findTag(buffer);
		if (mll_layer_storage.begin() != mll_layer_storage.end()) {
			mll_layers.push_back((*mll_layer_storage.begin()).second);
			mll_ids.push_back(mll_layers[i].getId());
			if (debug_)
				std::cout << "UwMultiTrafficRl::initialize::MLL Id("
						  << mll_layers[i].getId()
						  << ")::Tcl Name = " << mll_layers[i].getTclName()
						  << "::Layer Id = " << mll_layers[i].getLayer()
						  << std::endl;
			i++;
		} else {
			no_more = true;
		}
	}
}

void
UwMultiTrafficRl::initialize(std::string output_path, double probing_period,
		int mask_type, int eval_flag)
{
	ClMsgPhy2MacAddr msg;
	sendSyncClMsg(&msg);
	mac_addr = msg.getAddr();
	discoverPhyLayers();
	discoverMllLayers();
	state_ptr = new State(mask_type);
	const int state_dim = state_ptr->getHomogeneousState().size();
	mask_type_ = mask_type;
	probing_period_ = probing_period;
	output_path_ = output_path;

	agent = createAgent(agent_type_,
			state_dim,
			4,
			alpha_,
			beta_,
			epsilon_,
			mac_addr,
			output_path_);
	if (agent != nullptr) {
		if (eval_flag) {
			agent->set_mode(rl::AgentMode::Eval);
		} else {
			agent->set_mode(rl::AgentMode::Train);
		}
	}
	agent->write_header();
}

// TODO[AGENT]: Va tutto su un file a parte
void
UwMultiTrafficRl::sendInfoVecPtrClmsg()
{
	ClMsgInfoVecPointer m = ClMsgInfoVecPointer();
	m.setInfoVecPointer(info_vec_src);
	sendSyncClMsg(&m);
}

int
UwMultiTrafficRl::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_QUEUE_POINTER) {
		// Received a pointer to a MAC layer queue
		std::queue<Packet *> *q =
				dynamic_cast<ClMsgQueuePointer *>(m)->getQueuePointer();
		int type = dynamic_cast<ClMsgQueuePointer *>(m)->getType();
		if (type == 0)
			acoustic_queue_pointer = q;
		if (type == 1)
			optical_queue_pointer = q;
	}
	return Module::recvSyncClMsg(m);
}

void
UwMultiTrafficRl::setBestMediumToAcoustic()
{
	probing_best_medium_ = 0;
}

void
UwMultiTrafficRl::updateInfoCompletion()
{
	double now = NOW;

	// Iterate through info_vec and mark packets as complete based on timeouts
	for (size_t i = 0; i < info_vec.size(); ++i) {
		Info &info = info_vec[i];

		// Skip if already completed
		if (info.completed_) {
			continue;
		}

		// If received, already marked as complete in recv()
		if (info.received_) {
			continue;
		}

		// Check if packet should be marked as lost due to timeout
		if (info.action_time_ >= 0.0) {
			double elapsed = now - info.action_time_;
			double timeout = 1.0; // Default timeout
			if (handled_traffic_type_ >= 0 &&
					handled_traffic_type_ < loss_timeouts_.size()) {
				timeout = loss_timeouts_[handled_traffic_type_];
			}

			if (elapsed > timeout) {
				// Mark as completed due to timeout (packet lost)
				info.completed_ = true;
				info.completion_time_ = now;
				info.was_timeout_ = true;
				info.received_ = false;
				info.rx_time_ = -1.0;
				info.delay_ = -1.0;

				if (debug_) {
					printf("[%f] UwMultiTrafficRl: Packet %d marked as lost "
						   "(timeout)\n",
							now,
							info.packet_id_);
				}
			}
		}
	}
}

void
UwMultiTrafficRl::checkAndLearnFromInfo()
{
	if (!agent) {
		return; // No agent initialized yet
	}

	double now = NOW;

	// Process consecutive completed Info entries for learning
	while (learning_index_ < info_vec.size() - 1) {
		Info &current_info = info_vec[learning_index_];
		Info &next_info = info_vec[learning_index_ + 1];

		// Check if both current and next Info are completed
		if (!current_info.completed_ || !next_info.completed_) {
			break; // Wait for next Info to be completed
		}

		// Compute reward - with current, next, and previous info pointers
		Info *prev_info = (learning_index_ > 0) ? &info_vec[learning_index_ - 1]
												: nullptr;

		if (handled_traffic_type_ == 2) {
			jitter_count_ += 1;
			if (current_info.received_) {
				jitter_received_ += 1;
			}
		}

		float reward = getReward(
				&current_info, &next_info, prev_info, handled_traffic_type_);

		// Build transition with next_action from next Info
		rl::Transition transition;
		transition.state = current_info.state_.toTensor();
		transition.action = current_info.action_;
		transition.next_action = next_info.action_; // Use actual next action
		transition.reward = reward;
		transition.next_state = next_info.state_.toTensor();
		transition.done = false; // Set done=true only if no more packets
								 // expected (stream learning)
		transition.has_log_prob = true;
		transition.has_value = true;
		transition.log_prob = current_info.log_prob_;
		transition.value = current_info.value_;

		// Send transition to agent for learning
		agent->observe(transition);

		// Call train_step if agent supports it
		agent->train_step();

		agent->logTransition(current_info.packet_id_,
				current_info.state_,
				current_info.action_,
				transition.reward,
				current_info.generation_time_,
				current_info.action_time_,
				current_info.rx_time_,
				current_info.received_,
				current_info.delay_);

		if (debug_) {
			printf("[%f] UwMultiTrafficRl: Learning from packet %d -> %d "
				   "(action %ld -> %ld, reward %.2f)\n",
					now,
					current_info.packet_id_,
					next_info.packet_id_,
					transition.action,
					transition.next_action,
					reward);
		}

		// Increment learning index to process next pair
		++learning_index_;
	}
}

void
UwFinalProcedureTimer::expire(Event *e)
{
	((UwMultiTrafficRl *) module)->finalProcedure();
}

void
UwOpticalProbeRxTimer::expire(Event *e)
{
	((UwMultiTrafficRl *) module)->setBestMediumToAcoustic();
}