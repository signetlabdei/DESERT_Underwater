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

/**
 * @file   uw-smart-ofdm.cpp
 * @author Sara Falleni
 * @version 1.0.0
 *
 * @brief SMART_OFDM can work as MACA full bandwidth or MACA with control carriers divided
 * from data carriers.
 */

#include "uw-smart-ofdm.h"
#include <mac.h>
#include <cmath>
#include <climits>
#include <iomanip>
#include <rng.h>
#include "uwphy-clmsg.h"

enum
{
	NOT_SET = -1,
	SESSION_DISTANCE_NOT_SET = 0
};

/**
 * Class that describe the binding with tcl scripting language
 */
static class UWSMARTOFDMModuleClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UWSMARTOFDMModuleClass()
		: TclClass("Module/UW/SMART_OFDM")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UWSmartOFDM());
	}
} class_module_uwsmartofdm;

void UWSmartOFDM::AckTimer::expire(Event *e)
{
	timer_status = UWSMARTOFDM_EXPIRED;
	if (module->curr_state == UWSMARTOFDM_STATE_WAIT_ACK)
	{

		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ") ACK_timer expire() current state = "
				 << module->status_info[module->curr_state]
				 << "; ACK not received, next state = "
				 << module->status_info[UWSMARTOFDM_STATE_BACKOFF] << endl;

		module->refreshReason(UWSMARTOFDM_REASON_ACK_TIMEOUT);
		module->stateBackoff();
	}
	else
	{
		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ")::AckTimer::expired() " << endl;
	}
}

void UWSmartOFDM::BackOffTimer::expire(Event *e)
{
	timer_status = UWSMARTOFDM_EXPIRED;
	if (module->curr_state == UWSMARTOFDM_STATE_BACKOFF)
	{

		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ") timer expire() current state = "
				 << module->status_info[module->curr_state]
				 << "; backoff expired, next state = "
				 << module->status_info[UWSMARTOFDM_STATE_IDLE] << endl;

		module->refreshReason(UWSMARTOFDM_REASON_BACKOFF_TIMEOUT);
		module->exitBackoff();
		if (module->current_rcvs == 0)
			module->stateIdle();
		else
			module->refreshState(UWSMARTOFDM_STATE_IDLE);
	}
	else
	{
		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ")::BackOffTimer::expired() " << endl;
	}
}

// Packets can't be sent while waiting for CTS
// The only thing that can happen is that something is being received
// When this expires the state always has to be UWSMARTOFDM_STATE_CTRL_BACKOFF
void UWSmartOFDM::CTSTimer::expire(Event *e)
{
	if (module->uwsmartofdm_debug)
		cout << NOW << "  UWSmartOFDM (" << module->addr
			 << ") CTS TIMER EXPIRED" << endl;
	timer_status = UWSMARTOFDM_EXPIRED;
	module->RTSvalid = true;
	module->refreshReason(UWSMARTOFDM_REASON_CTS_BACKOFF_TIMEOUT);
	if (module->current_rcvs == 0)
	{

		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ") CTStimer expire() current state = "
				 << module->status_info[module->curr_state]
				 << "; backoff expired, next state = "
				 << module->status_info[UWSMARTOFDM_STATE_TX_RTS] << endl;

		module->exitCTSBackoff();
		module->Mac2PhySetTxBusy(1);
		module->stateSendRTS();
	}
	else
	{
		module->refreshState(UWSMARTOFDM_STATE_IDLE);
		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ") CTStimer expire() current state = "
				 << module->status_info[module->curr_state]
				 << "; backoff expired, next state = "
				 << module->status_info[UWSMARTOFDM_STATE_IDLE] << endl;
	}
}

// While RTSvalid == false I can't be sending DATA
// can only send CTS but in that case state is not IDLE
void UWSmartOFDM::RTSTimer::expire(Event *e)
{
	module->RTSvalid = true;
	timer_status = UWSMARTOFDM_EXPIRED;
	if (module->curr_state == UWSMARTOFDM_STATE_IDLE && module->current_rcvs == 0)
	{
		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ") RTStimer expire(), stateIDLE, sending RTS again " << std::endl;
		module->Mac2PhySetTxBusy(1);
		module->stateSendRTS();
	}
	else
	{
		if (module->uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << module->addr
				 << ") RTStimer expire(), BUT doing other stuff, keep going " << std::endl;
	}
}

void UWSmartOFDM::AssignmentTimer::expire(Event *e)
{
	// cout << NOW << "  UWSmartOFDM (" << module->addr << ") Assignment TIMER EXPIRED" << endl;
	timer_status = UWSMARTOFDM_EXPIRED;

	module->clearOccTable();
	if (module->curr_state == UWSMARTOFDM_STATE_IDLE && module->current_rcvs == 0)
	{
		module->stateIdle();
	}
}

void UWSmartOFDM::AssignmentValidTimer::expire(Event *e)
{
	timer_status = UWSMARTOFDM_EXPIRED;

	module->resetAssignment();
}

// While DATAtimer is active, the node can't send packets because it's waiting to receive them
void UWSmartOFDM::DATATimer::expire(Event *e)
{
	timer_status = UWSMARTOFDM_EXPIRED;
	module->refreshReason(UWSMARTOFDM_REASON_DATAT_EXPIRED);
	if (module->uwsmartofdm_debug)
		cout << NOW << "  UWSmartOFDM (" << module->addr
			 << ") DATATimer expire() " << std::endl;

	if (module->current_rcvs == 0)
		module->stateIdle();
	else
		module->refreshState(UWSMARTOFDM_STATE_IDLE);
}

const double UWSmartOFDM::prop_speed = 1500.0;
bool UWSmartOFDM::initialized = false;

map<UWSmartOFDM::UWSMARTOFDM_STATUS, string> UWSmartOFDM::status_info;
map<UWSmartOFDM::UWSMARTOFDM_REASON_STATUS, string> UWSmartOFDM::reason_info;
map<UWSmartOFDM::UWSMARTOFDM_PKT_TYPE, string> UWSmartOFDM::pkt_type_info;

UWSmartOFDM::UWSmartOFDM()
	: ack_timer(this), backoff_timer(this), CTS_timer(this),
	  assignment_timer(this), assignment_valid_timer(this),
	  RTS_timer(this), DATA_timer(this), txsn(1), last_sent_data_id(-1),
	  curr_data_pkt(0), last_data_id_rx(NOT_SET), curr_tx_rounds(0),
	  has_buffer_queue(true), curr_state(UWSMARTOFDM_STATE_IDLE),
	  prev_state(UWSMARTOFDM_STATE_IDLE), prev_prev_state(UWSMARTOFDM_STATE_IDLE),
	  ack_mode(UWSMARTOFDM_NO_ACK_MODE), last_reason(UWSMARTOFDM_REASON_NOT_SET),
	  start_tx_time(0), recv_data_id(-1), srtt(0), sumrtt(0), sumrtt2(0), rttsamples(0),
	  current_timeslot(0), current_rcvs(0), curr_rts_tries(0), timeslots(0), timeslot_length(0),
	  max_car_reserved(0), req_tslots(0), max_burst_size(0), curr_pkt_batch(0),
	  batch_sending(false), RTSvalid(true), max_rts_tries(0), nextFreeTime(0),
	  ackToSend(false), waitPkt(0), nextRTSts(0), nextRTS(0), fullBand(false)
{
	bind("HDR_size_", (int *)&HDR_size);
	bind("ACK_size_", (int *)&ACK_size);
	bind("RTS_size_", (int *)&RTS_size);
	bind("CTS_size_", (int *)&CTS_size);
	bind("DATA_size_", (int *)&DATA_size);
	bind("bitrateCar_", (double *)&bitrateCar);
	bind("max_tx_tries_", (int *)&max_tx_tries);
	bind("max_rts_tries_", (int *)&max_rts_tries);
	bind("wait_constant_", (double *)&wait_constant);
	bind("uwsmartofdm_debug_", (int *)&uwsmartofdm_debug); // debug mode
	bind("max_payload_", (int *)&max_payload);
	bind("ACK_timeout_", (double *)&ACK_timeout);
	bind("alpha_", (double *)&alpha_);
	bind("buffer_pkts_", (int *)&buffer_pkts);
	bind("backoff_tuner_", (double *)&backoff_tuner);
	bind("max_backoff_counter_", (int *)&max_backoff_counter);
	bind("nodenum_", (int *)&nodeNum);
	bind("timeslots_", (int *)&timeslots);
	bind("timeslot_length_", (double *)&timeslot_length);
	bind("max_car_reserved_", (int *)&max_car_reserved);
	bind("req_tslots_", (int *)&req_tslots);
	bind("MAC_addr_", (int *)&addr);
	bind("print_transitions_", (int *)&print_transitions);
	bind("max_burst_size_", (int *)&max_burst_size);
	bind("fullBand_", (int *)&fullBand);

	if (max_tx_tries <= 0)
		max_tx_tries = INT_MAX;
	if (buffer_pkts > 0)
		has_buffer_queue = true;

	msgDisp.initDisplayer(addr, "UWSmartOFDM", uwsmartofdm_debug);
}

UWSmartOFDM::~UWSmartOFDM()
{
}

// TCL command interpreter

int UWSmartOFDM::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2)
	{
		if (strcasecmp(argv[1], "setAckMode") == 0)
		{
			ack_mode = UWSMARTOFDM_ACK_MODE;
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "setNoAckMode") == 0)
		{
			ack_mode = UWSMARTOFDM_NO_ACK_MODE;
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "initialize") == 0)
		{
			if (initialized == false)
				initInfo();
			if (print_transitions)
				fout.open("/tmp/SMARTOFDMstateTransitions.txt", ios_base::app);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "printTransitions") == 0)
		{
			print_transitions = true;
			return TCL_OK;
		} // stats functions
		else if (strcasecmp(argv[1], "getQueueSize") == 0)
		{
			tcl.resultf("%d", mapPacket.size());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getUpLayersDataRx") == 0)
		{
			tcl.resultf("%d", getUpLayersDataPktsRx());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getAckPktsTx") == 0)
		{
			tcl.resultf("%d", getAckPktsTx());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getHighPrioPktsSent") == 0)
		{
			tcl.resultf("%d", getHighPrioPktsSent());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getHighPrioPktsRecv") == 0)
		{
			tcl.resultf("%d", getHighPrioPktsRecv());
			return TCL_OK;
		}
	}
	else if (argc == 3)
	{
		if (strcasecmp(argv[1], "setMacAddr") == 0)
		{
			addr = atoi(argv[2]);
			if (debug_)
				cout << "OFDM Aloha MAC address of current node is " << addr << endl;
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "addInvalidCarriers") == 0)
		{
			addInvalidCarriers(atoi(argv[2]));
			if (debug_)
				cout << "Carrier " << atoi(argv[2]) << " not usable " << endl;
			return TCL_OK;
		}
	}
	else if (argc == 6)
	{
		if (strcasecmp(argv[1], "init_macofdm_node") == 0)
		{
			init_macofdm_node(atoi(argv[2]), atof(argv[3]), atoi(argv[4]), argv[5]);
			return TCL_OK;
		}
	}
	return MMac::command(argc, argv);
}

void UWSmartOFDM::initInfo()
{

	initialized = true;

	if ((print_transitions) && (system(NULL)))
	{
		system("rm -f /tmp/SMARTOFDMstateTransitions.txt");
		system("touch /tmp/SMARTOFDMstateTransitions.txt");
	}

	status_info[UWSMARTOFDM_STATE_IDLE] = "Idle state";
	status_info[UWSMARTOFDM_STATE_TX_DATA] = "Transmit DATA state";
	status_info[UWSMARTOFDM_STATE_TX_ACK] = "Transmit ACK state";
	status_info[UWSMARTOFDM_STATE_WAIT_ACK] = "Wait for ACK state";
	status_info[UWSMARTOFDM_STATE_DATA_RX] = "DATA received state";
	status_info[UWSMARTOFDM_STATE_ACK_RX] = "ACK received state";
	status_info[UWSMARTOFDM_STATE_RX_IDLE] = "Start rx Idle state";
	status_info[UWSMARTOFDM_STATE_RX_WAIT_ACK] = "Start rx Wait ACK state";
	status_info[UWSMARTOFDM_STATE_CHK_ACK_TIMEOUT] = "Check Wait ACK timeout state";
	status_info[UWSMARTOFDM_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";
	status_info[UWSMARTOFDM_STATE_BACKOFF] = "Backoff state";
	status_info[UWSMARTOFDM_STATE_RX_BACKOFF] = "Start rx Backoff state";
	status_info[UWSMARTOFDM_STATE_CHK_BACKOFF_TIMEOUT] = "Check Backoff timeout state";
	status_info[UWSMARTOFDM_STATE_TX_RTS] = "Transmit RTS state";
	status_info[UWSMARTOFDM_STATE_RX_RTS] = "Receive RTS state";
	status_info[UWSMARTOFDM_STATE_WAIT_CTS] = "Wait for CTS state";
	status_info[UWSMARTOFDM_STATE_CTRL_BACKOFF] = "Start CTRL backoff state";
	status_info[UWSMARTOFDM_STATE_TX_CTS] = "Transmit CTS state";
	status_info[UWSMARTOFDM_STATE_RX_CTS] = "Receive CTS state";
	status_info[UWSMARTOFDM_STATE_RX_ACTIVE] = "Active Reception state";
	status_info[UWSMARTOFDM_STATE_TX_ACTIVE] = "Active Transmission state";
	status_info[UWSMARTOFDM_STATE_WAIT_DATA] = "Wait for DATA state";

	reason_info[UWSMARTOFDM_REASON_DATA_PENDING] = "DATA pending from upper layers";
	reason_info[UWSMARTOFDM_REASON_DATA_RX] = "DATA received";
	reason_info[UWSMARTOFDM_REASON_DATA_TX] = "DATA transmitted";
	reason_info[UWSMARTOFDM_REASON_ACK_TX] = "ACK tranmsitted";
	reason_info[UWSMARTOFDM_REASON_ACK_RX] = "ACK received";
	reason_info[UWSMARTOFDM_REASON_DATA_NOCAR] = "DATA to send & carriers not assigned";
	reason_info[UWSMARTOFDM_REASON_DATA_CARASSIGNED] = "DATA to send & carriers already assigned";
	reason_info[UWSMARTOFDM_REASON_RTS_RX] = "RTS received";
	reason_info[UWSMARTOFDM_REASON_CTS_RX] = "CTS received";
	reason_info[UWSMARTOFDM_REASON_ACK_TIMEOUT] = "ACK timeout";
	reason_info[UWSMARTOFDM_REASON_DATA_EMPTY] = "DATA queue empty";
	reason_info[UWSMARTOFDM_REASON_MAX_TX_TRIES] =
		"DATA dropped due to max tx rounds";
	reason_info[UWSMARTOFDM_REASON_START_RX] = "Start rx pkt";
	reason_info[UWSMARTOFDM_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
	reason_info[UWSMARTOFDM_REASON_WAIT_ACK_PENDING] = "Wait for ACK timer pending";
	reason_info[UWSMARTOFDM_REASON_PKT_ERROR] = "Pkt corrupted at PHY layer";
	reason_info[UWSMARTOFDM_REASON_BACKOFF_TIMEOUT] = "Backoff expired";
	reason_info[UWSMARTOFDM_REASON_BACKOFF_PENDING] = "Backoff timer pending";
	reason_info[UWSMARTOFDM_REASON_WAIT_CTS_PENDING] = "CTS Backoff timer pending";
	reason_info[UWSMARTOFDM_REASON_CTS_TX] = "CTS transmitted";
	reason_info[UWSMARTOFDM_REASON_RTS_TX] = "RTS transmitted";
	reason_info[UWSMARTOFDM_REASON_CTS_BACKOFF_TIMEOUT] = "CTS backoff expired";
	reason_info[UWSMARTOFDM_REASON_PHY_LAYER_RECEIVING] = "Phy Layer receiving something";
	reason_info[UWSMARTOFDM_REASON_PHY_LAYER_SENDING] = "Phy Layer sending something";
	reason_info[UWSMARTOFDM_REASON_MAX_RTS_TRIES] = "Max RTS tries reached";
	reason_info[UWSMARTOFDM_REASON_WAIT_DATA] = "CTS Sent, waiting for DATA to arrive";
	reason_info[UWSMARTOFDM_REASON_DATAT_EXPIRED] = "DATA Timer Expired";
	reason_info[UWSMARTOFDM_REASON_PREVIOUS_RTS] = "Previously received and RTS";

	pkt_type_info[UWSMARTOFDM_ACK_PKT] = "ACK pkt";
	pkt_type_info[UWSMARTOFDM_DATA_PKT] = "DATA pkt";
	pkt_type_info[UWSMARTOFDM_DATAMAX_PKT] = "MAX payload DATA pkt";
	pkt_type_info[UWSMARTOFDM_RTS_PKT] = "RTS pkt";
	pkt_type_info[UWSMARTOFDM_CTS_PKT] = "CTS PKT";
}

// Initialize subCarriers parameters inside a node, default all carriers are used
void UWSmartOFDM::init_macofdm_node(int subCarNum, double carSize, int ctrl_subCar, std::string modulation)
{

	mac_ncarriers = subCarNum;
	ctrl_car = ctrl_subCar;
	data_car = mac_ncarriers - ctrl_car;

	mac_carrierSize = carSize;

	assignment_timer.schedule(timeslot_length);

	// Occupancy_table initialization with all 0s (free)
	for (int i = 0; i < data_car; i++)
	{

		std::vector<int> temp;
		for (int j = 0; j < timeslots; j++)
			temp.push_back(0);

		otabmtx.lock();
		occupancy_table.push_back(temp);
		otabmtx.unlock();
	}

	for (int i = 0; i < mac_ncarriers; i++)
	{
		std::vector<double> temp;
		temp.push_back(0);
		interf_table.push_back(temp);
	}

	// This is kept for carriers not to be used at all
	for (int i = 0; i < nouse_carriers.size(); i++)
	{
		for (int j = 0; j < timeslots; j++)
			occupancy_table[nouse_carriers[i]][j] = 1;
	}
	// mac_carVec initialization (since it's a vector!)
	for (int i = 0; i < data_car; i++)
	{
		mac_carVec.push_back(0);
	}
	// mac_carMod initialization (since it's a vector!)
	for (int i = 0; i < mac_ncarriers; i++)
	{
		mac_carMod.push_back("BPSK");
	}
	return;
}

void UWSmartOFDM::updateRTT(double curr_rtt)
{
	srtt = alpha_ * srtt + (1 - alpha_) * curr_rtt;
	sumrtt += curr_rtt;
	sumrtt2 += curr_rtt * curr_rtt;
	rttsamples++;
	ACK_timeout = (sumrtt / rttsamples);
}

void UWSmartOFDM::updateAckTimeout(double rtt)
{
	updateRTT(rtt);
	//   double curr_rtt = getRTT();

	//   if (curr_rtt > 0) ACK_timeout = min(ACK_timeout, getRTT() );

	if (uwsmartofdm_debug)
		cout << NOW << "  UWSmartOFDM (" << addr
			 << ")::updateAckTimeout() curr ACK_timeout = " << ACK_timeout
			 << endl;
	//   waitForUser();
}

void UWSmartOFDM::exitBackoff()
{
	backoff_timer.stop();
}

void UWSmartOFDM::exitCTSBackoff()
{
	CTS_timer.stop();
}

double
UWSmartOFDM::getBackoffTime()
{
	incrTotalBackoffTimes();
	double random = RNG::defaultrng()->uniform_double();

	backoff_timer.incrCounter();
	double counter = backoff_timer.getCounter();
	if (counter > max_backoff_counter)
		counter = max_backoff_counter;

	double backoff_duration =
		backoff_tuner * random * 2.0 * ACK_timeout * pow(2.0, counter);

	backoffSumDuration(backoff_duration);

	if (uwsmartofdm_debug)
	{
		cout << NOW << "  UWSmartOFDM (" << addr
			 << ")::getBackoffTime() backoff time = " << backoff_duration
			 << " s" << endl;
	}
	return backoff_duration;
}

double
UWSmartOFDM::computeTxTime(UWSMARTOFDM_PKT_TYPE type)
{
	double duration;
	Packet *temp_data_pkt;
	map<pktSeqNum, Packet *>::iterator it_p;

	if (type == UWSMARTOFDM_DATA_PKT)
	{
		if (!mapPacket.empty())
		{
			it_p = mapPacket.begin();
			temp_data_pkt = ((*it_p).second)->copy();
			// temp_data_pkt = (Q.front())->copy();
			hdr_cmn *ch = HDR_CMN(temp_data_pkt);
			ch->size() = HDR_size + ch->size();
		}
		else
		{
			temp_data_pkt = Packet::alloc();
			hdr_cmn *ch = HDR_CMN(temp_data_pkt);
			ch->size() = HDR_size + max_payload;
		}
	}
	else if (type == UWSMARTOFDM_ACK_PKT)
	{
		temp_data_pkt = Packet::alloc();
		hdr_cmn *ch = HDR_CMN(temp_data_pkt);
		ch->size() = ACK_size;
	}
	duration = Mac2PhyTxDuration(temp_data_pkt);
	Packet::free(temp_data_pkt);
	return (duration);
}

void UWSmartOFDM::recvFromUpperLayers(Packet *p)
{
	msgDisp.printStatus("", "recvFromUpperLayers", NOW, addr);
	if (((has_buffer_queue == true) && (mapPacket.size() < buffer_pkts)) ||
		(has_buffer_queue == false))
	{
		hdr_cmn *ch = hdr_cmn::access(p);
		if (uwsmartofdm_debug)
			std::cout << NOW << "  UWSmartOFDM (" << addr << ")::recvFromUpperLayers received packet seq_num = " << ch->uid() << std::endl;
		initPkt(p, UWSMARTOFDM_DATA_PKT);
		putPktInQueue(p);
		incrUpperDataRx();
		waitStartTime();

		if (curr_state == UWSMARTOFDM_STATE_IDLE && !current_rcvs)
		{ // Basically not doing anything

			refreshReason(UWSMARTOFDM_REASON_DATA_PENDING);
			hdr_mac *mach = HDR_MAC(p);
			current_macDA = mach->macDA();
			if (!car_assigned && RTSvalid)
			{
				msgDisp.printStatus("car_assigned = FALSE sending RTS", "recvFromUpperLayers", NOW, addr);
				Mac2PhySetTxBusy(1);
				stateSendRTS();
			}
			else if (car_assigned)
			{
				msgDisp.printStatus("New data but car_assigned = TRUE", "recvFromUpperLayers", NOW, addr);
				stateTxData();
			}
			else
			{
				msgDisp.printStatus("New data but RTS_valid = FALSE and car_assigned = FALSE", "recvFromUpperLayers", NOW, addr);
			}
		}
		else
		{
			string st = "Not proceeding with RTS because in STATE: " + status_info[curr_state] + "or current_rcvs = " + std::to_string(current_rcvs);
			msgDisp.printStatus(st, "recvFromUpperLayers", NOW, addr);
		}
	}
	else
	{
		incrDiscardedPktsTx();
		msgDisp.printStatus("Dropping packet", "recvFromUpperLayers", NOW, addr);
		drop(p, 1, UWSMARTOFDM_DROP_REASON_BUFFER_FULL);
	}
}

void UWSmartOFDM::initPkt(Packet *p, UWSMARTOFDM_PKT_TYPE type, int dest_addr)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);
	hdr_OFDMMAC *ofdmmac = HDR_OFDMMAC(p);

	if (uwsmartofdm_debug)
	{
		std::cout << NOW << "  UWSmartOFDM (" << addr << ")::initPkt() for a " << pkt_type_info[type] << " packet " << p << " seq_num " << ch->uid() << " size " << ch->size() << std::endl;
	}
	for (int i = 0; i < mac_carMod.size(); i++)
		ofdmph->carMod[i] = mac_carMod[i];
	ofdmph->carrierNum = mac_ncarriers;
	ofdmph->carrierSize = mac_carrierSize;
	ofdmph->nativeOFDM = true;

	int curr_size = ch->size();

	switch (type)
	{

	case (UWSMARTOFDM_DATA_PKT):
	{
		ch->size() = curr_size + HDR_size;
		mach->ftype() = MF_DATA;
		// std::cout << "DATA PKT, MAC DEST " << mach->macDA() << std::endl;
	}
	break;

	case (UWSMARTOFDM_ACK_PKT):
	{
		ch->ptype() = PT_MMAC_ACK;
		ch->size() = ACK_size;
		ch->uid() = recv_data_id;
		mach->ftype() = MF_ACK;
		mach->macSA() = addr;
		mach->macDA() = dest_addr;
	}
	break;

	case (UWSMARTOFDM_RTS_PKT):
	{
		ch->ptype() = PT_MMAC_RTS;
		ch->size() = RTS_size;
		ch->uid() = recv_data_id;
		mach->ftype() = MF_RTS;
		mach->macSA() = addr;
		mach->macDA() = dest_addr;
		if (mapPacket.size() > max_burst_size)
			ofdmmac->bytesToSend = DATA_size * max_burst_size;
		else
			ofdmmac->bytesToSend = DATA_size * mapPacket.size();
	}
	break;

	case (UWSMARTOFDM_CTS_PKT):
	{
		ch->ptype() = PT_MMAC_CTS;
		ch->size() = CTS_size;
		ch->uid() = recv_data_id;
		mach->ftype() = MF_CTS;
		mach->macSA() = addr;
		mach->macDA() = dest_addr;
	}
	break;
	}
	if (type != UWSMARTOFDM_DATA_PKT)
	{
		if (fullBand == false)
		{
			for (std::size_t i = 0; i < ctrl_car; ++i)
				ofdmph->carriers[i] = 1;

			for (std::size_t i = ctrl_car; i < mac_ncarriers; ++i)
				ofdmph->carriers[i] = 0;
		}
		else
		{
			for (std::size_t i = 0; i < mac_ncarriers; ++i)
				ofdmph->carriers[i] = 1;
		}

		for (std::size_t i = 0; i < mac_ncarriers; ++i)
			ofdmph->carMod[i] = "BPSK";
	}
}

void UWSmartOFDM::Mac2PhyStartTx(Packet *p)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	hdr_mac *mach = HDR_MAC(p);

	if (uwsmartofdm_debug)
		cout << NOW << "  UWSmartOFDM (" << addr
			 << ")::Mac2PhyStartTx() _DBG_ start tx packet type " << ch->ptype()
			 << ", seq num = " << ch->uid()
			 << ", source addr = " << mach->macSA()
			 << ", dest addr = " << mach->macDA()
			 << ", current_rcvs = " << current_rcvs << std::endl;

	MMac::Mac2PhyStartTx(p);

	msgDisp.printStatus("Sent down packet, current_rcvs = " + std::to_string(current_rcvs), "Mac2PhyStartTx", NOW, addr);
}

void UWSmartOFDM::Phy2MacEndTx(const Packet *p)
{
	msgDisp.printStatus("End tx Packet", "Mac2PhyEndTx", NOW, addr);

	Mac2PhySetTxBusy(0);

	switch (curr_state)
	{

	case (UWSMARTOFDM_STATE_TX_DATA):
	{
		refreshReason(UWSMARTOFDM_REASON_DATA_TX);
		if (ack_mode == UWSMARTOFDM_ACK_MODE)
		{

			if (uwsmartofdm_debug)
				cout << NOW << "  UWSmartOFDM (" << addr
					 << ")::Phy2MacEndTx() DATA sent,from "
					 << status_info[curr_state] << " to "
					 << status_info[UWSMARTOFDM_STATE_WAIT_ACK] << endl;

			stateWaitAck();
		}
		else
		{

			if (uwsmartofdm_debug)
				cout << NOW << "  UWSmartOFDM (" << addr
					 << ")::Phy2MacEndTx() DATA sent, from "
					 << status_info[curr_state] << " to "
					 << status_info[UWSMARTOFDM_STATE_IDLE] << endl;

			stateIdle();
		}
	}
	break;

	case (UWSMARTOFDM_STATE_TX_ACK):
	{
		refreshReason(UWSMARTOFDM_REASON_ACK_TX);

		if (uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << addr
				 << ")::Phy2MacEndTx() ack sent, from "
				 << status_info[curr_state] << " to "
				 << status_info[UWSMARTOFDM_STATE_IDLE] << endl;
		// stateIdle();
		stateBackoff(0.0001); // this is for the scheduler precision
	}
	break;
	//// new smart-ofdm
	case (UWSMARTOFDM_STATE_TX_RTS):
	{
		refreshReason(UWSMARTOFDM_REASON_RTS_TX);

		if (uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << addr
				 << ")::Phy2MacEndTx() RTS sent, from "
				 << status_info[curr_state] << " to "
				 << status_info[UWSMARTOFDM_STATE_WAIT_CTS] << endl;
		stateBackoffCTS();
	}
	break;

	case (UWSMARTOFDM_STATE_TX_CTS):
	{
		refreshReason(UWSMARTOFDM_REASON_CTS_TX);

		if (uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << addr
				 << ")::Phy2MacEndTx() CTS sent, from "
				 << status_info[curr_state] << " to "
				 << status_info[UWSMARTOFDM_STATE_WAIT_DATA] << endl;
		stateWaitData(0.3);
		// stateIdle();
	}
	break;
		//// end new

	default:
	{
		cout << NOW << "  UWSmartOFDM (" << addr
			 << ")::Phy2MacEndTx() logical error, current state = "
			 << status_info[curr_state] << " prev state " << status_info[prev_state] << endl;
		stateIdle();
		// exit(1);
	}
	break;
	}
}

void UWSmartOFDM::Phy2MacStartRx(const Packet *p)
{
	current_rcvs++;
	if (uwsmartofdm_debug)
		cout << NOW << "  UWSmartOFDM (" << addr << ")::Phy2MacStartRx() rx Packet. current_rcvs = " << current_rcvs << endl;
}

void UWSmartOFDM::Phy2MacEndRx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	packet_t rx_pkt_type = ch->ptype();
	hdr_mac *mach = HDR_MAC(p);
	MacFrameType frame_type = mach->ftype();
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);

	int source_mac = mach->macSA();
	int dest_mac = mach->macDA();

	double gen_time = ph->txtime;
	double received_time = ph->rxtime;
	double diff_time = received_time - gen_time;

	double distance = diff_time * prop_speed;
	current_rcvs--;
	if (uwsmartofdm_debug)
		cout << NOW << "  UWSmartOFDM (" << addr << ")::Phy2MacEndRx() _DBG_ "
			 << status_info[curr_state]
			 << ", received a pkt type = " << ch->ptype()
			 << ", seq num = " << ch->uid()
			 << ", src addr = " << mach->macSA()
			 << " dest addr = " << mach->macDA()
			 << ", estimated distance between nodes = " << distance << " m. "
			 << "current_rcvs " << current_rcvs << endl;

	if (ch->error())
	{ // this tells if there were errors at the phy layer
		msgDisp.printStatus("dropping corrupted pkt", "Phy2MacEndRx()", NOW, addr);
		incrErrorPktsRx();

		refreshReason(UWSMARTOFDM_REASON_PKT_ERROR);
		drop(p, 1, UWSMARTOFDM_DROP_REASON_ERROR);
		if (current_rcvs == 0 && curr_state == UWSMARTOFDM_STATE_IDLE) // curr_state != UWSMARTOFDM_STATE_WAIT_DATA && curr_state != UWSMARTOFDM_STATE_CTRL_BACKOFF)
			stateIdle();
		// TODO: send ack if expected
	}
	else
	{ // no errors in the packet
		if (dest_mac == addr || dest_mac == MAC_BROADCAST)
		{
			// Organized by Packet Types, pkt is for me
			if (frame_type == MF_ACK) //(rx_pkt_type == PT_MMAC_ACK)
			{
				refreshReason(UWSMARTOFDM_REASON_ACK_RX);
				stateRxAck(p);
			}
			else if (frame_type == MF_CTS) //(rx_pkt_type == PT_MMAC_CTS)
			{
				// When receiving a CTS always go to set carriers and OTable
				// If free will proceed with sending data
				if (curr_state == UWSMARTOFDM_STATE_CTRL_BACKOFF && current_rcvs == 0)
				{
					refreshReason(UWSMARTOFDM_REASON_CTS_RX);
					msgDisp.printStatus("going to stateRxCTS", "Phy2MacEndRx", NOW, addr);
					stateRxCTS(p);
				}
				else
				{
					msgDisp.printStatus("Updating OTable but keep doing the other stuff, curr_state " + status_info[curr_state], "Phy2MacEndRx", NOW, addr);
					hdr_OFDMMAC *ofdmmac = HDR_OFDMMAC(p);
					int slots = ofdmmac->timeReserved / timeslot_length;
					updateOccupancy(ofdmmac->usage_carriers, slots + 1);
					double newNextFreeT = NOW + ofdmmac->timeReserved;
					if (newNextFreeT > nextFreeTime)
						nextFreeTime = newNextFreeT;
				}
			}
			else if (frame_type == MF_RTS) //(rx_pkt_type == PT_MMAC_RTS)
			{
				nextRTS = p->copy();
				nextRTSts = NOW;
				if (current_rcvs == 0 && ackToSend)
				{

					Mac2PhySetTxBusy(1);
					stateTxAck(HDR_MAC(pkt_to_ack)->macSA());
				}
				else if (current_rcvs == 0 && curr_state == UWSMARTOFDM_STATE_IDLE)
				{
					refreshReason(UWSMARTOFDM_REASON_RTS_RX);
					msgDisp.printStatus("going to stateRxRTS", "Phy2MacEndRx", NOW, addr);
					stateRxRTS(p);
				}
				else
				{
					msgDisp.printStatus("RTS received but curr_state " + status_info[curr_state], "Phy2MacEndRx", NOW, addr);
					drop(p, 1, UWSMARTOFDM_DROP_REASON_WRONG_RECEIVER);
				}
			}
			else
			{
				// In case an unrecognized data packet arrives
				if (frame_type != MF_RTS && frame_type != MF_CTS && frame_type != MF_ACK && frame_type != MF_DATA)
				{
					cerr << NOW << "  UWSmartOFDM (" << addr << ")::Phy2MacEndRx Unrecognized packet addressed to me" << std::endl;
					updateInterfTable(p);
					drop(p, 1, UWSMARTOFDM_DROP_REASON_WRONG_RECEIVER);
					if (current_rcvs == 0 && curr_state == UWSMARTOFDM_STATE_IDLE)
					{
						msgDisp.printStatus("Going Back To IDLE STATE", "Phy2MacEndRx", NOW, addr);
						if (ackToSend)
						{
							Mac2PhySetTxBusy(1);
							stateTxAck(HDR_MAC(pkt_to_ack)->macSA());
						}
						else
							stateIdle();
					}
				}
				// Data Packet: In this case There are no follow up operations so I receive the packet in any case
				// RxData doesn't disrupt the FSM cycle
				else if (curr_state == UWSMARTOFDM_STATE_IDLE || curr_state == UWSMARTOFDM_STATE_WAIT_DATA)
				{
					refreshReason(UWSMARTOFDM_REASON_DATA_RX);
					msgDisp.printStatus("going to stateRxData", "Phy2MacEndRx", NOW, addr);
					stateRxData(p);
				}
				else
				{
					msgDisp.printStatus("DATA received but curr_state " + status_info[curr_state], "Phy2MacEndRx", NOW, addr);
					drop(p, 1, UWSMARTOFDM_DROP_REASON_WRONG_RECEIVER);
				}
			}
		}
		else
		{
			refreshReason(UWSMARTOFDM_REASON_PKT_NOT_FOR_ME);
			if (frame_type == MF_CTS) //(rx_pkt_type == PT_MMAC_CTS)
			{
				hdr_OFDMMAC *ofdmmac = HDR_OFDMMAC(p);
				msgDisp.printStatus("PKT NOT FOR ME 555x, UPDATING TABLE drop", "Phy2MacEndRx", NOW, addr);

				int slots = ofdmmac->timeReserved / timeslot_length;
				updateOccupancy(ofdmmac->usage_carriers, slots + 1);
				double newNextFreeT = NOW + ofdmmac->timeReserved;
				if (newNextFreeT > nextFreeTime)
					nextFreeTime = newNextFreeT;
				msgDisp.printStatus("nextFreeTime=" + std::to_string(nextFreeTime), "Phy2MacEndRx", NOW, addr);
				drop(p, 1, UWSMARTOFDM_DROP_REASON_WRONG_RECEIVER);

				if (current_rcvs == 0 && curr_state == UWSMARTOFDM_STATE_IDLE)
				{
					msgDisp.printStatus("Going Back To IDLE STATE", "Phy2MacEndRx", NOW, addr);
					if (ackToSend)
					{
						Mac2PhySetTxBusy(1);
						stateTxAck(HDR_MAC(pkt_to_ack)->macSA());
					}
					else
						stateIdle();
				}
				else
				{
					msgDisp.printStatus("CTS received, not for me. Curr_state = " + status_info[curr_state], "Phy2MacEndRx", NOW, addr);
				}
			}
			else
			{
				//      Packet::free(p);
				msgDisp.printStatus("PKT NOT FOR ME 222x drop", "Phy2MacEndRx", NOW, addr);

				if (frame_type != MF_RTS && frame_type != MF_CTS && frame_type != MF_ACK && frame_type != MF_DATA)
				{
					msgDisp.printStatus("Unrecognized Packet Received", "Phy2MacEndRx", NOW, addr);
					updateInterfTable(p);
				}
				drop(p, 1, UWSMARTOFDM_DROP_REASON_WRONG_RECEIVER);
				if (current_rcvs == 0 && curr_state == UWSMARTOFDM_STATE_IDLE)
				{
					msgDisp.printStatus("Going Back To IDLE STATE 222x. Curr_state = " + status_info[curr_state], "Phy2MacEndRx", NOW, addr);
					if (ackToSend)
					{
						Mac2PhySetTxBusy(1);
						stateTxAck(HDR_MAC(pkt_to_ack)->macSA());
					}
					else
					{
						if (frame_type == MF_RTS) //(rx_pkt_type == PT_MMAC_RTS)
							stateBackoff(0.08);
						else
							stateIdle();
					}
				}
			}
		}
	}
}

void UWSmartOFDM::txData()
{
	Packet *data_pkt = curr_data_pkt->copy();

	curr_rts_tries = 0;

	if ((ack_mode == UWSMARTOFDM_NO_ACK_MODE))
	{
		eraseItemFromPktQueue(getPktSeqNum(data_pkt));
	}

	incrDataPktsTx();
	incrCurrTxRounds();
	Mac2PhyStartTx(data_pkt);
}

void UWSmartOFDM::txAck(int dest_addr)
{
	Packet *ack_pkt = Packet::alloc();
	initPkt(ack_pkt, UWSMARTOFDM_ACK_PKT, dest_addr);

	incrAckPktsTx();
	Mac2PhyStartTx(ack_pkt);
}

void UWSmartOFDM::txRTS()
{
	int nfree;

	Packet *rts_pkt = Packet::alloc();
	initPkt(rts_pkt, UWSMARTOFDM_RTS_PKT, current_macDA);

	hdr_OFDM *ofdmph = HDR_OFDM(rts_pkt);

	// incrAckPktsTx();

	hdr_OFDMMAC *ofdmmac = HDR_OFDMMAC(rts_pkt);
	nfree = pickFreeCarriers(ofdmmac->usage_carriers);
	if (nfree > 0)
	{
		msgDisp.printStatus("", "txRTS", NOW, addr);
		curr_rts_tries++;
		RTSsent++;
		Mac2PhyStartTx(rts_pkt);
	}
	else
	{
		double RTS_Backoff;
		double amp = curr_rts_tries > 1 ? (curr_rts_tries / 2) : curr_rts_tries;
		if (nextFreeTime > NOW)
			RTS_Backoff = abs(nextFreeTime - NOW + ((double)rand() / (RAND_MAX)) * amp);
		else
			RTS_Backoff = abs(((double)rand() / (RAND_MAX)) * amp);

		RTS_Backoff = RTS_Backoff == 0 ? ((double)rand() / (RAND_MAX)) : RTS_Backoff;
		msgDisp.printStatus("all carriers unavailable, back to Idle. Next RTS in " + std::to_string(RTS_Backoff), "txRTS", NOW, addr);
		RTS_timer.schedule(RTS_Backoff);
		Mac2PhySetTxBusy(0);
		RTSvalid = false;
		Packet::free(rts_pkt);
		stateIdle();
	}
}

void UWSmartOFDM::txCTS(int dest_addr, int *otherFree, int brequested)
{
	int top_car, bot_car;
	int top_avoid_car, bot_avoid_car;
	int n_match;

	Packet *cts_pkt = Packet::alloc();
	initPkt(cts_pkt, UWSMARTOFDM_CTS_PKT, dest_addr);
	hdr_OFDMMAC *ofdmmac = HDR_OFDMMAC(cts_pkt);
	hdr_OFDM *ofdmph = HDR_OFDM(cts_pkt);

	int myFree[data_car];

	pickFreeCarriers(myFree);

	string myfreecar = std::to_string(NOW) + "  UWSmartOFDM (" + std::to_string(addr) + ")::txCTS() MY FREE CARRIERS: ";
	string otherfreecar = " INCOMING FREE CARRIERS: ";
	string matchingcar = " MATCHING CARRIERS: ";

	n_match = matchCarriers(myFree, otherFree, ofdmmac->usage_carriers);

	for (int i = 0; i < data_car; i++)
	{
		myfreecar += std::to_string(myFree[i]) + " ";
		otherfreecar += std::to_string(otherFree[i]) + " ";
		matchingcar += std::to_string(ofdmmac->usage_carriers[i]) + " ";
	}
	if (uwsmartofdm_debug)
		std::cout << myfreecar << "-" << otherfreecar << "-> " << n_match << matchingcar << std::endl;
	if (nextRTS && dest_addr == HDR_MAC(nextRTS)->macSA())
	{
		nextRTS = 0;
	}
	if (n_match > 0)
	{
		double ackOffset = 0;
		if (ack_mode == UWSMARTOFDM_ACK_MODE)
			ackOffset = 0.07;
		double tneeded = (8 * (double)brequested / (n_match * bitrateCar)) + ackOffset;
		ofdmmac->timeReserved = tneeded;
		int tsl_needed = (tneeded / timeslot_length);
		// cout << NOW << "  UWSmartOFDM (" << addr << ")::txCTS() {TEMP} t needed" << tneeded << " tslotneeded " << tsl_needed << std::endl;
		updateOccupancy(ofdmmac->usage_carriers, tsl_needed + 1);
		nextFreeTime = NOW + ofdmmac->timeReserved;
		msgDisp.printStatus("nextFreeTime=" + std::to_string(nextFreeTime), "txCTS", NOW, addr);
		CTSsent++;

		Mac2PhyStartTx(cts_pkt);
	}
	else
	{
		msgDisp.printStatus("No matching carriers, back to idle", "txCTS", NOW, addr);

		Mac2PhySetTxBusy(0);
		Packet::free(cts_pkt);
		stateIdle();
	}
}

void UWSmartOFDM::stateCheckAckExpired()
{
	refreshState(UWSMARTOFDM_STATE_CHK_ACK_TIMEOUT);
	msgDisp.printStatus("", "stateCheckAckExpired", NOW, addr);

	map<pktSeqNum, AckTimer>::iterator it_a;
	it_a = mapAckTimer.begin();

	if (print_transitions)
		printStateInfo();
	if (((*it_a).second).isActive())
	{
		refreshReason(UWSMARTOFDM_REASON_WAIT_ACK_PENDING);
		refreshState(UWSMARTOFDM_STATE_WAIT_ACK);
	}
	else if (((*it_a).second).isExpired())
	{
		refreshReason(UWSMARTOFDM_REASON_ACK_TIMEOUT);
		stateBackoff();
	}
	else
	{
		cerr << NOW << "  UWSmartOFDM (" << addr
			 << ")::stateCheckAckExpired() ack_timer logical error, current "
				"timer state = "
			 << status_info[curr_state] << endl;
		stateIdle();
		// exit(1);
	}
}

void UWSmartOFDM::stateCheckBackoffExpired()
{
	refreshState(UWSMARTOFDM_STATE_CHK_BACKOFF_TIMEOUT);

	msgDisp.printStatus("", "stateCheckBackoffExpired", NOW, addr);

	if (print_transitions)
		printStateInfo();
	if (backoff_timer.isActive())
	{
		refreshReason(UWSMARTOFDM_REASON_BACKOFF_PENDING);
		stateBackoff();
	}
	else if (backoff_timer.isExpired())
	{
		refreshReason(UWSMARTOFDM_REASON_BACKOFF_TIMEOUT);
		exitBackoff();
		stateIdle();
	}
	else
	{
		cerr << NOW << "  UWSmartOFDM (" << addr
			 << ")::stateCheckBackoffExpired() backoff_timer logical error, "
				"current timer state = "
			 << status_info[curr_state] << endl;
		stateIdle();
		// exit(1);
	}
}

void UWSmartOFDM::stateCheckCTSBackoffExpired()
{
	refreshState(UWSMARTOFDM_STATE_CHK_CTS_BACKOFF_TIMEOUT);

	msgDisp.printStatus("", "stateCheckCTSBackoffExpired", NOW, addr);

	if (print_transitions)
		printStateInfo();
	if (CTS_timer.isActive())
	{
		refreshReason(UWSMARTOFDM_REASON_WAIT_CTS_PENDING);
		stateWaitCTS();
	}
	else if (CTS_timer.isExpired())
	{
		refreshReason(UWSMARTOFDM_REASON_CTS_BACKOFF_TIMEOUT);
		exitCTSBackoff();
		Mac2PhySetTxBusy(1);
		stateSendRTS();
	}
	else
	{
		cerr << NOW << "  UWSmartOFDM (" << addr
			 << ")::stateCheckCTSBackoffExpired() CTS_timer logical error, "
				"current timer state = "
			 << status_info[curr_state] << endl;
		stateIdle();
		// exit(1);
	}
}

void UWSmartOFDM::stateIdle()
{
	Packet *next_p;
	mapAckTimer.clear();
	backoff_timer.stop();
	int freec[data_car];
	refreshState(UWSMARTOFDM_STATE_IDLE);

	if (print_transitions)
		printStateInfo();

	msgDisp.printStatus("queue_size=" + std::to_string(mapPacket.size()), "stateIdle", NOW, addr);

	if (!mapPacket.empty() && ((mapPacket.size() > 0) || car_assigned) && current_rcvs == 0)
	{

		map<pktSeqNum, Packet *>::iterator it_p;
		it_p = mapPacket.begin();
		next_p = (*it_p).second;

		hdr_mac *mach = HDR_MAC(next_p);
		current_macDA = mach->macDA();
		// If carriers assignment is still valid just send data, else ask for more
		if (!car_assigned && RTSvalid)
		{
			msgDisp.printStatus("car_assigned = FALSE sending RTS", "stateIdle", NOW, addr);
			Mac2PhySetTxBusy(1);
			stateSendRTS();
		}
		else if (car_assigned)
		{
			msgDisp.printStatus("New data but car_assigned = TRUE", "stateIdle", NOW, addr);
			refreshReason(UWSMARTOFDM_REASON_DATA_CARASSIGNED);
			Mac2PhySetTxBusy(1);
			stateTxData();
		}
		else
		{
			msgDisp.printStatus("New data but RTSvalid = FALSE", "stateIdle", NOW, addr);
		}
	}
	else if (nextRTS && ((NOW - nextRTSts) < 1.5) && !current_rcvs && (pickFreeCarriers(freec) > 0))
	{
		refreshReason(UWSMARTOFDM_REASON_PREVIOUS_RTS);
		Mac2PhySetTxBusy(1);
		stateSendCTS(nextRTS);
		msgDisp.printStatus("CTS from IDLE", "stateIdle", NOW, addr);
	}
}

void UWSmartOFDM::stateSendRTS()
{
	if (curr_state != UWSMARTOFDM_STATE_TX_RTS)
	{
		refreshState(UWSMARTOFDM_STATE_TX_RTS);
		refreshReason(UWSMARTOFDM_REASON_DATA_NOCAR);

		if (curr_rts_tries == max_rts_tries)
		{
			if (uwsmartofdm_debug)
				cout << NOW << "  UWSmartOFDM (" << addr << ")::stateSendRTS() max_tries " << max_rts_tries << " curr_tries " << curr_rts_tries << std::endl;
			curr_rts_tries = 0;
			msgDisp.printStatus("Going back to Idle, rts_tries > max", "stateSendRTS", NOW, addr);

			refreshReason(UWSMARTOFDM_REASON_MAX_RTS_TRIES);
			if (print_transitions)
				printStateInfo();
			Mac2PhySetTxBusy(0);

			map<pktSeqNum, Packet *>::iterator it_p;
			it_p = mapPacket.begin();
			curr_data_pkt = (*it_p).second;
			msgDisp.printStatus("Dropping Packet seq_num " + std::to_string(getPktSeqNum(curr_data_pkt)), "stateSendRTS", NOW, addr);
			eraseItemFromPktQueue(getPktSeqNum(curr_data_pkt));

			incrDroppedPktsTx();
			stateIdle();
		}
		else
		{
			msgDisp.printStatus("Sending RTS", "stateSendRTS", NOW, addr);
			if (print_transitions)
				printStateInfo();
			txRTS();
		}
	}
}
void UWSmartOFDM::stateRxRTS(Packet *p)
{
	refreshState(UWSMARTOFDM_STATE_RX_RTS);
	refreshReason(UWSMARTOFDM_REASON_RTS_RX);

	if (current_rcvs > 0)
	{
		current_rcvs = 0;
		if (uwsmartofdm_debug)
			cout << NOW << "  UWSmartOFDM (" << addr << ")::stateRxRTS() Interrupting previous receptions " << endl;
	}

	msgDisp.printStatus("RTS received", "stateRxRTS", NOW, addr);
	Mac2PhySetTxBusy(1);
	stateSendCTS(p);

	// stateIdle();
}

void UWSmartOFDM::stateRxCTS(Packet *p)
{
	CTS_timer.force_cancel();
	refreshState(UWSMARTOFDM_STATE_RX_CTS);
	refreshReason(UWSMARTOFDM_REASON_CTS_RX);
	msgDisp.printStatus(" ", "stateRxCTS", NOW, addr);
	RTSvalid = true;

	hdr_mac *mach = HDR_MAC(p);

	nextAssignment = 50000;

	hdr_OFDMMAC *ofdmmac = HDR_OFDMMAC(p);
	int slots = ofdmmac->timeReserved / timeslot_length;
	updateOccupancy(ofdmmac->usage_carriers, slots + 1);
	assignment_valid_timer.schedule(ofdmmac->timeReserved);

	double newNextFreeT = NOW + ofdmmac->timeReserved;
	if (newNextFreeT > nextFreeTime)
		nextFreeTime = newNextFreeT;

	car_assigned = true;

	for (int i = 0; i < mac_carVec.size(); i++)
		mac_carVec[i] = 0;

	for (int i = 0; i < data_car; i++)
	{
		int val = ofdmmac->usage_carriers[i];
		if (val >= 0)
		{
			mac_carVec[val] = 1;
		}
	}
	int freec[data_car];
	if (current_rcvs == 0 && !mapPacket.empty() && pickFreeCarriers(freec) > 0)
	{
		string txcarriers = "Going to transmit. mac_carVec = ";
		for (int i = 0; i < mac_carVec.size(); i++)
		{
			txcarriers += std::to_string(mac_carVec[i]);
			txcarriers += " ";
		}
		msgDisp.printStatus(txcarriers, "stateRxCTS", NOW, addr);

		Mac2PhySetTxBusy(1);
		stateTxData();
	}
	else if ((mapPacket.empty() && current_rcvs == 0) || (current_rcvs == 0 && pickFreeCarriers(freec) <= 0))
	{
		stateIdle();
		msgDisp.printStatus("Packet queue empty, back to Idle", "stateRxCTS", NOW, addr);
	}
	else
	{
		msgDisp.printStatus("Other receptions ongoing, just keep doing them", "stateRxCTS", NOW, addr);
	}
}

void UWSmartOFDM::stateSendCTS(Packet *p)
{
	refreshState(UWSMARTOFDM_STATE_TX_CTS);
	refreshReason(UWSMARTOFDM_REASON_CTS_TX);

	msgDisp.printStatus("", "stateSendCTS", NOW, addr);

	if (print_transitions)
		printStateInfo();

	hdr_mac *mach = HDR_MAC(p);
	int dst_addr = mach->macSA();
	hdr_cmn *ch = hdr_cmn::access(p);

	hdr_OFDMMAC *ofdmmac = HDR_OFDMMAC(p);
	waitPkt = (ofdmmac->bytesToSend) / DATA_size;
	txCTS(dst_addr, ofdmmac->usage_carriers, ofdmmac->bytesToSend);
}

void UWSmartOFDM::stateWaitCTS()
{
	refreshState(UWSMARTOFDM_STATE_WAIT_CTS);

	msgDisp.printStatus("", "stateWaitCTS", NOW, addr);
	if (print_transitions)
		printStateInfo();
}

void UWSmartOFDM::stateBackoffCTS()
{
	double CTSBackoff = ((double)rand() / (RAND_MAX)) / 1.6 * (curr_rts_tries);
	CTSBackoff = CTSBackoff + 0.16;
	CTS_timer.force_cancel();
	refreshState(UWSMARTOFDM_STATE_CTRL_BACKOFF);
	RTSvalid = false;

	msgDisp.printStatus("SCHEDULING CTS BACKOFF of " + std::to_string(CTSBackoff), "stateBackoffCTS", NOW, addr);
	CTS_timer.schedule(CTSBackoff);

	if (print_transitions)
		printStateInfo();
}

void UWSmartOFDM::stateRxIdle()
{
	refreshState(UWSMARTOFDM_STATE_RX_IDLE);

	if (print_transitions)
		printStateInfo();
}

void UWSmartOFDM::stateBackoff(double bt)
{
	backoff_timer.force_cancel();
	refreshState(UWSMARTOFDM_STATE_BACKOFF);

	if (backoff_timer.isFrozen())
		backoff_timer.unFreeze();
	else if (!bt)
		backoff_timer.schedule(getBackoffTime());
	else
		backoff_timer.schedule(bt);

	msgDisp.printStatus("", "stateBackoff", NOW, addr);

	if (print_transitions)
		printStateInfo(backoff_timer.getDuration());
}

void UWSmartOFDM::stateRxBackoff()
{
	backoff_timer.freeze();
	refreshState(UWSMARTOFDM_STATE_RX_BACKOFF);

	if (print_transitions)
		printStateInfo();
}

void UWSmartOFDM::stateTxData()
{
	refreshState(UWSMARTOFDM_STATE_TX_DATA);

	msgDisp.printStatus("", "stateTxData", NOW, addr);

	if (print_transitions)
		printStateInfo();

	map<pktSeqNum, Packet *>::iterator it_p;
	it_p = mapPacket.begin();
	curr_data_pkt = (*it_p).second;
	int seq_num;
	seq_num = getPktSeqNum(curr_data_pkt);
	map<pktSeqNum, AckTimer>::iterator it_a;

	if (seq_num != last_sent_data_id)
	{
		putAckTimerInMap(seq_num);
		it_a = mapAckTimer.find(seq_num);
		resetCurrTxRounds();
		backoff_timer.resetCounter();
		((*it_a).second).resetCounter();
		hdr_mac *mach = HDR_MAC(curr_data_pkt);
		hdr_MPhy *ph = HDR_MPHY(curr_data_pkt);
		hdr_OFDM *ofdmph = HDR_OFDM(curr_data_pkt);
		start_tx_time = NOW; // we set curr RTT
		last_sent_data_id = seq_num;
		if (fullBand == false)
		{
			for (std::size_t i = 0; i < mac_carVec.size(); ++i)
				ofdmph->carriers[ctrl_car + i] = mac_carVec[i];
		}
		else
		{
			for (std::size_t i = 0; i < mac_ncarriers; ++i)
				ofdmph->carriers[i] = 1;
		}
		txData();
	}
	else
	{
		if (mapAckTimer.size() == 0)
		{
			putAckTimerInMap(seq_num);
			it_a = mapAckTimer.find(seq_num);
			((*it_a).second).resetCounter();
			incrCurrTxRounds();
			backoff_timer.incrCounter();
			if (curr_tx_rounds < max_tx_tries)
			{
				hdr_mac *mach = HDR_MAC(curr_data_pkt);
				start_tx_time = NOW; // we set curr RTT
				last_sent_data_id = seq_num;
				hdr_MPhy *ph = HDR_MPHY(curr_data_pkt);
				hdr_OFDM *ofdmph = HDR_OFDM(curr_data_pkt);
				if (fullBand == false)
				{
					for (std::size_t i = 0; i < mac_carVec.size(); ++i)
						ofdmph->carriers[ctrl_car + i] = mac_carVec[i];
				}
				else
				{
					for (std::size_t i = 0; i < mac_ncarriers; ++i)
						ofdmph->carriers[i] = 1;
				}
				txData();
			}
			else
			{
				eraseItemFromPktQueue(seq_num);
				incrDroppedPktsTx();

				refreshReason(UWSMARTOFDM_REASON_MAX_TX_TRIES);

				string s = "curr_tx_rounds " + std::to_string(curr_tx_rounds) +
						   " > max_tx_tries = " + std::to_string(max_tx_tries);
				msgDisp.printStatus(s, "stateTxData", NOW, addr);

				Mac2PhySetTxBusy(0);
				stateIdle();
			}
		}
		else
		{
			Mac2PhySetTxBusy(0);
			stateCheckAckExpired();
		}
	}
}

void UWSmartOFDM::stateWaitAck()
{

	map<pktSeqNum, AckTimer>::iterator it_a;
	it_a = mapAckTimer.begin();

	((*it_a).second).stop();
	refreshState(UWSMARTOFDM_STATE_WAIT_ACK);

	msgDisp.printStatus("", "stateWaitAck", NOW, addr);
	if (print_transitions)
		printStateInfo();

	((*it_a).second).incrCounter();
	((*it_a).second).schedule(ACK_timeout + 2 * wait_constant);
}

void UWSmartOFDM::stateRxWaitAck()
{
	refreshState(UWSMARTOFDM_STATE_RX_WAIT_ACK);

	if (print_transitions)
		printStateInfo();
}

void UWSmartOFDM::stateTxAck(int dest_addr)
{
	refreshState(UWSMARTOFDM_STATE_TX_ACK);
	ackToSend = false;

	msgDisp.printStatus("dest_addr = " + std::to_string(dest_addr), "stateTxAck", NOW, addr);

	if (print_transitions)
		printStateInfo();

	txAck(dest_addr);
}

void UWSmartOFDM::stateRxData(Packet *data_pkt)
{
	ack_timer.stop();
	DATA_timer.force_cancel();
	msgDisp.printStatus("DATA_timer canceled because DATA received", "stateRxData", NOW, addr);

	refreshState(UWSMARTOFDM_STATE_DATA_RX);

	refreshReason(UWSMARTOFDM_REASON_DATA_RX);

	hdr_mac *mach = HDR_MAC(data_pkt);
	int dst_addr = mach->macSA();

	hdr_cmn *ch = hdr_cmn::access(data_pkt);
	packet_t rx_pkt_type = ch->ptype();
	recv_data_id = ch->uid();
	ch->size() = ch->size() - HDR_size;
	incrDataPktsRx();
	waitPkt -= 1;

	sendUp(data_pkt);

	if (ack_mode == UWSMARTOFDM_ACK_MODE)
	{
		// TO DO: need to change this code to handle multiple receptions
		// if you want to use acks: What happens if data packet received, other receptions happening
		// but you want to send an ACK?
		if (current_rcvs == 0)
		{
			Mac2PhySetTxBusy(1);
			stateTxAck(dst_addr);
		}
		else
		{
			ackToSend = true;
			pkt_to_ack = data_pkt;
		}
	}
	else if (current_rcvs == 0)
	{
		if (waitPkt)
			stateBackoff(0.0002);
		else
			stateIdle();
	}
	else
	{
		refreshState(UWSMARTOFDM_STATE_IDLE);
	}
}

void UWSmartOFDM::stateRxAck(Packet *p)
{

	map<pktSeqNum, AckTimer>::iterator it_a;
	it_a = mapAckTimer.begin();

	((*it_a).second).stop();

	refreshState(UWSMARTOFDM_STATE_ACK_RX);
	refreshReason(UWSMARTOFDM_REASON_ACK_RX);

	msgDisp.printStatus("", "stateRxAck", NOW, addr);

	int seq_num;
	seq_num = getPktSeqNum(p);
	Packet::free(p);
	eraseItemFromPktQueue(seq_num);

	eraseItemFrommapAckTimer(seq_num);
	updateAckTimeout(NOW - start_tx_time);
	incrAckPktsRx();
	if (current_rcvs == 0)
		stateIdle();
	else
		refreshState(UWSMARTOFDM_STATE_IDLE);
}

void UWSmartOFDM::stateWaitData(double t)
{
	refreshState(UWSMARTOFDM_STATE_WAIT_DATA);
	refreshReason(UWSMARTOFDM_REASON_WAIT_DATA);

	msgDisp.printStatus("", "stateWaitData", NOW, addr);
	DATA_timer.schedule(t);
}

void UWSmartOFDM::printStateInfo(double delay)
{
	// if (uwsmartofdm_debug)
	cout << NOW << " UWSmartOFDM (" << addr << ")::printStateInfo() "
		 << "from " << status_info[prev_state] << " to "
		 << status_info[curr_state]
		 << ". Reason: " << reason_info[last_reason] << endl;

	// if (curr_state == UWSMARTOFDM_STATE_BACKOFF) {
	// 	fout << left << setw(10) << NOW << "  UWSmartOFDM (" << addr
	// 		 << ")::printStateInfo() "
	// 		 << "from " << status_info[prev_state] << " to "
	// 		 << status_info[curr_state]
	// 		 << ". Reason: " << reason_info[last_reason]
	// 		 << ". Backoff duration = " << delay << endl;
	// } else {
	// 	fout << left << setw(10) << NOW << "  UWSmartOFDM (" << addr
	// 		 << ")::printStateInfo() "
	// 		 << "from " << status_info[prev_state] << " to "
	// 		 << status_info[curr_state]
	// 		 << ". Reason: " << reason_info[last_reason] << endl;
	// }
}

void UWSmartOFDM::waitForUser()
{
	std::string response;
	std::cout << "Press Enter to continue";
	std::getline(std::cin, response);
}
// Remove invalid carriers
void UWSmartOFDM::removeInvalidCarrier(int c)
{
	for (int i = 0; i < nouse_carriers.size(); i++)
		if (nouse_carriers[i] == c)
			nouse_carriers.erase(nouse_carriers.begin() + i);
	return;
}

// Update Interf Table with a new unrecognized packet
void UWSmartOFDM::updateInterfTable(Packet *p)
{
	double old_thr = 10.0;
	int broken_thr = 2;
	std::vector<int> new_nouse;
	if (!fullBand)
	{
		for (int i = 0; i < mac_ncarriers; i++)
		{
			for (int j = 1; j < interf_table[i].size(); j++)
			{
				if ((NOW - interf_table[i][j]) > old_thr)
					interf_table[i].erase(interf_table[i].begin() + j);
			}
		}
		for (int i = 0; i < mac_ncarriers; i++)
		{
			if (HDR_OFDM(p)->carriers[i] == 1)
				interf_table[i].push_back(NOW);
		}
		for (int i = 0; i < mac_ncarriers; i++)
		{
			if (interf_table[i].size() > (broken_thr + 1))
			{
				new_nouse.push_back(i);
				msgDisp.printStatus(to_string(i) + " Added to InterfTable", "updateInterfTable", NOW, addr);
			}
		}
		nouse_carriers = new_nouse;
		std::string st = "nouse_carriers : ";
		for (int i = 0; i < nouse_carriers.size(); i++)
		{
			st += std::to_string(nouse_carriers[i]) + " ";
		}
		msgDisp.printStatus(st, "updateInterfTable", NOW, addr);
	}
}

// IMPORTANT NOTE: since I decided that the LOW prio nodes take double the bandwidth divided
// by the number of nodes, the bandwidth is used if there is already a user. Change the
// prev_busy parameter if it's decided to use more bandwidth
void UWSmartOFDM::carToBeUsed(criticalLevel c, int &top, int &bottom, int &avoid_top, int &avoid_bottom)
{
	// currently statistics not implemented
	avoid_top = 0;
	avoid_bottom = 0;
	int i = 0;
	int carToGive; // num of carriers to give away
	int assigned_done = 0;
	int prev_busy = 2;

	if (c == HIGH)
	{
		carToGive = floor(data_car / nodeNum);

		if (uwsmartofdm_debug)
			cout << NOW << " UWSmartOFDM (" << addr << ")::carToBeUsed() for HIGH prio " << carToGive << endl;

		for (i = ctrl_car; i < mac_ncarriers; i++)
			if (mac_carVec[i] == 0) // Free carrier
			{
				if (i < (mac_ncarriers - carToGive)) // There are enough carriers to give
				{
					bottom = i;
					top = bottom + carToGive - 1;
					assigned_done = 1;
					break;
				}
			}
		if (!assigned_done)
		{
			bottom = ctrl_car;
			top = bottom + carToGive - 1;
		}
	}
	else
	{ // priority is LOW

		carToGive = 3 * floor(data_car / nodeNum);

		if (uwsmartofdm_debug)
			cout << NOW << " UWSmartOFDM (" << addr << ")::carToBeUsed() for LOW prio " << carToGive << endl;

		for (i = mac_ncarriers - 1; i >= 0; i--)
			if (mac_carVec[i] <= prev_busy) // Free carrier or used only once
			{
				if (i >= (carToGive + ctrl_car)) // There are enough carriers to give
				{
					top = i;
					bottom = top - carToGive + 1;
					assigned_done = 1;
					break;
				}
			}
		if (!assigned_done)
		{
			top = mac_ncarriers;
			bottom = top - carToGive + 1;
		}
	}
	if (uwsmartofdm_debug)
		cout << NOW << " UWSmartOFDM (" << addr << ")::carToBeUsed() end of top_car is " << top << " bottom_car is " << bottom << endl;

	// fill the vector with the used carriers
	for (i = bottom; i <= top; i++)
		mac_carVec[i]++;
}

int UWSmartOFDM::pickFreeCarriers(int *freeCar)
{

	int nextFree = 0;
	for (int i = 0; i < data_car; i++)
		freeCar[i] = -1;

	int curTSlot = 0;
	int j;
	otabmtx.lock();
	for (int i = 0; i < data_car; i++)
	{
		if (occupancy_table[i][oTableIndex] == 0)
		{
			freeCar[nextFree] = i;
			nextFree++;
		}
	}
	otabmtx.unlock();
	return nextFree;
}

int UWSmartOFDM::matchCarriers(int *myFree, int *otherFree, int *matching)
{
	int mindex = 0;
	int foundCar = 0;
	msgDisp.printStatus("", "matchCarriers", NOW, addr);
	for (int i = 0; (i < data_car) && (foundCar < max_car_reserved); i++)
	{
		for (int j = 0; (j < data_car) && (foundCar < max_car_reserved); j++)
		{
			if ((myFree[i] == otherFree[j]) && (myFree[i] != -1))
			{
				matching[mindex] = myFree[i];
				foundCar++;
				if (mindex == (data_car - 1))
					return foundCar;
				else
					mindex++;
			}
		}
	}
	for (int k = mindex; k < data_car; k++)
		matching[k] = -1;
	return foundCar;
}

void UWSmartOFDM::updateOccupancy(int *busyCar, int ntslots)
{
	// start from the right point in the table
	otabmtx.lock();
	string st = "";
	for (int i = 0; i < data_car; i++)
	{
		if (busyCar[i] >= 0)
		{
			for (int j = 0; j < ntslots; j++)
				occupancy_table[busyCar[i]][(oTableIndex + j) % timeslots] = 1;
		}
	}
	otabmtx.unlock();
	printOccTable();
}

void UWSmartOFDM::clearOccTable()
{
	otabmtx.lock();
	for (int i = 0; i < data_car; i++)
		occupancy_table[i][oTableIndex] = 0;
	for (int i = 0; i < nouse_carriers.size(); i++)
		occupancy_table[nouse_carriers[i] - ctrl_car][oTableIndex] = 1;
	oTableIndex = (oTableIndex + 1) % timeslots;
	assignment_timer.schedule(timeslot_length);
	otabmtx.unlock();
}

void UWSmartOFDM::resetAssignment()
{
	car_assigned = false;
	if (uwsmartofdm_debug)
		std::cout << NOW << " UWSmartOFDM (" << addr << ")::resetAssignment: car_assignment = FALSE " << std::endl;
}

void UWSmartOFDM::printOccTable()
{
	string st = "";
	for (int i = 0; i < data_car; i++)
	{
		for (int j = 0; j < timeslots; j++)
			st = st + std::to_string(occupancy_table[i][j]);
		st = st + '\n';
	}
	if (uwsmartofdm_debug)
	{
		std::cout << NOW << " UWSmartOFDM (" << addr << ")::Occupancy Table: (current oTableIndex " << oTableIndex << ")" << std::endl;
		std::cout << st << std::endl;
	}
}

void UWSmartOFDM::Mac2PhySetTxBusy(int busy, int get)
{
	ClMsgUwPhyTxBusy m;
	if (get == 1)
	{
		m.setGetOp(1);
	}
	else
	{
		m.setTxBusy(busy);
	}
	sendSyncClMsg(&m);
}

bool UWSmartOFDM::batchSending()
{

	if ((mapPacket.size() > 0) || batch_sending)
		return true;
	else
		return false;
}
