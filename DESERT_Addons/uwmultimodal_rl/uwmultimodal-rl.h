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
 * @file uwmultimodal-rl.h
 * @author Filippo Donegà and Filippo Bragato
 * @version 1.0.0
 */

#ifndef UWMULTIMODAL_RL_H
#define UWMULTIMODAL_RL_H

#include <timer-handler.h>
#include "rl/Agent.h"
#include "uwmulti-traffic-control.h"
#include "uwopticalstats-phy.h"
#include <cstdint>
#include <memory>
#include <string>
#include <torch/torch.h>

class UwMultiModalRl; // Forward declaration of the UwMultiModalRl class

/**
 * Class used to schedule periodic processing of the wait queue
 */
class UwFinalProcedureTimer : public TimerHandler
{
public:
	UwFinalProcedureTimer(UwMultiModalRl *m)
		: TimerHandler()
	{
		module = m;
	}

protected:
	virtual void expire(Event *e);
	UwMultiModalRl *module;
};

/**
 * Class used to restore the best medium to acoustic when some time has passed
 * since the last optical probe reception
 */
class UwOpticalProbeRxTimer : public TimerHandler
{
public:
	UwOpticalProbeRxTimer(UwMultiModalRl *m)
		: TimerHandler()
	{
		module = m;
	}

protected:
	virtual void expire(Event *e);
	UwMultiModalRl *module;
};

/**
 * Enum representing the operating modes
 * 0: Always choose acoustic
 * 1: Always choose optical
 * 2: Choose the best medium using RL
 * 3: Switch medium based on distance
 * 4: Switch medium based on optical probe
 */
enum Mode { AcousticOnly, OpticalOnly, Multimodal, RangeBased, OpticalProbing };

/**
 * Struct that represents an info_vec entry used for learning
 */
struct Info {

	/**
	 * Constructor of the Info class
	 */
	Info(int mask_type);
	/**
	 * Destructor of the Info class
	 */
	~Info() = default;

	int packet_id_; /**< Unique packet identifier */
	State state_; /**< State of the node */
	int action_; /**< Action chosen by the RL algorithm */
	bool completed_; /**< Flag indicating whether the transition is completed */
	bool received_; /**< Flag indicating whether the packet was received */
	bool was_timeout_; /**< Flag indicating if packet was marked complete due to
						  timeout (lost) */
	double generation_time_; /**< Timestamp of the moment the packet related to
								this transition was generated */
	double action_time_; /**< Timestamp of the moment this action is chosen */
	double rx_time_; /**< Timestamp of the moment the packet related to this
						transition was received, -1 otherwise */
	double completion_time_; /**< Timestamp when the Info was marked as
								completed */
	double loss_timeout_; /**< Timeout period for marking packet as lost (in
							 seconds) */
	double delay_; /**< Delay of the packet or -1 when lost */

	torch::Tensor log_prob_; /**< Log probability of the action chosen by the RL
								algorithm */
	torch::Tensor value_; /**< Value of the state chosen by the RL algorithm */
};

class UwMultiModalRl : public UwMultiTrafficControl
{

	friend class UwFinalProcedureTimer;
	friend class UwOpticalProbeRxTimer;

public:
	/**
	 * Constructor of the UwMultiModalRl class
	 */
	UwMultiModalRl();
	/**
	 * Destructor of the UwMultiModalRl class
	 */
	virtual ~UwMultiModalRl() = default;
	/**
	 * TCL command interpreter
	 *
	 * @param argc Number of arguments in <i>argv</i>
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i> is the name of the object)
	 *
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not
	 */
	virtual int command(int argc, const char *const *argv);
	/**
	 * Handle a received packet
	 *
	 * @param p pointer to the packet
	 */
	virtual void recv(Packet *p);
	/**
	 * Get the underlying PHY layers IDs
	 */
	virtual void discoverPhyLayers();
	/**
	 * Get the underlying MLL layers IDs
	 */
	virtual void discoverMllLayers();
	/**
	 * Handle a received synchronous cross-layer message
	 */
	virtual int recvSyncClMsg(ClMessage *m);
	/**
	 * Send a cross-layer message containing a pointer to info_vec
	 */
	void sendInfoVecPtrClmsg();
	/**
	 * Call processWaitQueue iteratively
	 */
	void finalProcedure();
	/**
	 * Iterate over wait_queue starting from the oldest entry and send
	 * packets until action 2 (wait) is chosen or the queue is empty
	 */
	void processWaitQueue();
	/**
	 * Set the best medium back to acoustic (probing mode only)
	 */
	void setBestMediumToAcoustic();
	/**
	 * Update completion status of Info entries based on timeouts
	 */
	void updateInfoCompletion();
	/**
	 * Check for completed consecutive Info entries and dispatch transitions to
	 * agent
	 */
	void checkAndLearnFromInfo();

	float getReward(const Info *current, const Info *next_info,
			const Info *prev_info, int traffic_type) const;

protected:
	int debug_; /**< Flag to enable or disable debug functionalities [0 : OFF |
				   1 : ON] */
	std::vector<int>
			phy_ids; /**< Vector that stores the IDs of discovered PHY layers */
	std::unique_ptr<rl::Agent> agent; /**< Selected RL agent */
	State *state_ptr; /**< Pointer to a State object */
	State **state_ptr_other; /**< Pointer to the state_ptr of the node on the
								other side of the link */
	std::vector<int>
			mll_ids; /**< Vector that stores the IDs of discovered MLL layers */
	std::queue<Packet *>
			*acoustic_queue_pointer; /**< Pointer to the acoustic MAC queue */
	std::queue<Packet *>
			*optical_queue_pointer; /**< Pointer to the optical MAC queue */
	std::queue<Packet *> wait_queue; /**< Wait queue that stores all packets
										received from the upper layer */
	double alpha_; /**< Learning rate for weights */
	double beta_; /**< Learning rate for r_bar */
	double epsilon_; /**< Exploration probability */
	double expire_time_; /**< Time after which a packet is considered lost */
	int mac_addr; /**< MAC address of the current node */
	std::vector<Info> info_vec; /**< Vector storing SARS tuples and other packet
								   information for learning */
	std::vector<Info> *info_vec_src; /**< Pointer to the info_vec of the node on
										the other side of the link */
	uintptr_t info_vec_addr; /**< Address of the info_vec */
	uintptr_t state_ptr_addr; /**< Address of state_ptr */
	UwFinalProcedureTimer
			tmr; /**< Timer that handles periodic calls to finalProcedure */
	UwOpticalProbeRxTimer probe_tmr; /**< Timer that, once expired, switches
										probing_best_medium_ back to 0 */
	double finalProcedureTimerPeriod; /**< Period between calls to
										 finalProcedure */
	Mode mode_; /**< Operating mode [0: Acoustic only | 1: Optical only | 2:
				   Multimodal | 3: Range-based | 4: Optical probing] */
	int handled_traffic_type_; /**< Type of traffic to be handled by the agent
								*/
	int rng_seed; /**< Current simulation seed */
	double probing_period_; /**< Interval between periodic optical probes */
	int mask_type_; /**< Type of mask to be used for state selection */
	double range_based_distance_; /**< Distance threshold for the range-based
									 mode */
	int other_addr_; /**< Address of the node at the other end of the link */
	int probing_best_medium_; /**< Best medium to be used when in probing mode
							   */
	std::string agent_type_; /**< Runtime-selected RL agent type */
	std::string output_path_; /**< Output path used by the selected agent */
	size_t learning_index_; /**< Index tracking progress through info_vec for
							   learning */
	std::vector<double>
			loss_timeouts_; /**< Timeout values (in seconds) for each traffic
							   type for marking packets as lost */
	double jitter_received_; /**< Number of Jitter packets received */
	double jitter_count_; /**< Count of Jitter packets sent */
	/**
	 * Initialize the layer at the beginning of the simulation
	 */
	void initialize(std::string output_path, double probing_period,
			int mask_type, int eval_flag);
};

#endif /* UWMULTI_TRAFFIC_RL_H */