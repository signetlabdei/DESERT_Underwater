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
 * @file   uw-ofdm-aloha.cpp
 * @author Sara Falleni
 * @version 1.0.0
 *
 * @brief Class that provide the implementation of ALOHA protocol. By default all carriers are used
 * not to be used carriers can be added to the nouse_carriers as in smart_ofdm
 *
 */

#include "uw-ofdm-aloha.h"
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
static class UWOFDMALOHAModuleClass : public TclClass
{
public:
    /**
     * Constructor of the class
     */
    UWOFDMALOHAModuleClass()
        : TclClass("Module/UW/OFDM_ALOHA")
    {
    }

    TclObject *
    create(int, const char *const *)
    {
        return (new UWOFDMAloha());
    }
} class_module_uwofdmaloha;

void UWOFDMAloha::AckTimer::expire(Event *e)
{
    timer_status = UWOFDMALOHA_EXPIRED;
    if (module->curr_state == UWOFDMALOHA_STATE_WAIT_ACK)
    {

        if (module->uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << module->addr
                 << ") timer expire() current state = "
                 << module->status_info[module->curr_state]
                 << "; ACK not received, next state = "
                 << module->status_info[UWOFDMALOHA_STATE_BACKOFF] << endl;

        module->refreshReason(UWOFDMALOHA_REASON_ACK_TIMEOUT);
        module->stateBackoff();
    }
    else
    {
        if (module->uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << module->addr
                 << ")::AckTimer::expired() " << endl;
    }
}

void UWOFDMAloha::BackOffTimer::expire(Event *e)
{
    timer_status = UWOFDMALOHA_EXPIRED;
    if (module->curr_state == UWOFDMALOHA_STATE_BACKOFF)
    {

        if (module->uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << module->addr
                 << ") timer expire() current state = "
                 << module->status_info[module->curr_state]
                 << "; backoff expired, next state = "
                 << module->status_info[UWOFDMALOHA_STATE_IDLE] << endl;

        module->refreshReason(UWOFDMALOHA_REASON_BACKOFF_TIMEOUT);
        module->exitBackoff();
        module->stateIdle();
    }
    else
    {
        if (module->uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << module->addr
                 << ")::BackOffTimer::expired() " << endl;
    }
}

const double UWOFDMAloha::prop_speed = 1500.0;
bool UWOFDMAloha::initialized = false;

map<UWOFDMAloha::UWOFDMALOHA_STATUS, string> UWOFDMAloha::status_info;
map<UWOFDMAloha::UWOFDMALOHA_REASON_STATUS, string> UWOFDMAloha::reason_info;
map<UWOFDMAloha::UWOFDMALOHA_PKT_TYPE, string> UWOFDMAloha::pkt_type_info;

UWOFDMAloha::UWOFDMAloha()
    : ack_timer(this), 
    backoff_timer(this), 
    txsn(1), 
    last_sent_data_id(-1), 
    curr_data_pkt(0), 
    last_data_id_rx(NOT_SET), 
    curr_tx_rounds(0), 
    print_transitions(false), 
    has_buffer_queue(true), 
    curr_state(UWOFDMALOHA_STATE_IDLE), 
    prev_state(UWOFDMALOHA_STATE_IDLE), 
    prev_prev_state(UWOFDMALOHA_STATE_IDLE), 
    ack_mode(UWOFDMALOHA_NO_ACK_MODE), 
    last_reason(UWOFDMALOHA_REASON_NOT_SET), 
    start_tx_time(0), 
    recv_data_id(-1), 
    srtt(0), 
    sumrtt(0), 
    sumrtt2(0), 
    rttsamples(0), 
    n_receptions(0), 
    disturbanceNode(false)
{
    mac2phy_delay_ = 1e-19;

    bind("HDR_size_", (int *)&HDR_size);
    bind("ACK_size_", (int *)&ACK_size);
    bind("max_tx_tries_", (int *)&max_tx_tries);
    bind("wait_constant_", (double *)&wait_constant);
    bind("uwofdmaloha_debug_", (int *)&uwofdmaloha_debug); // debug mode
    bind("max_payload_", (int *)&max_payload);
    bind("ACK_timeout_", (double *)&ACK_timeout);
    bind("alpha_", (double *)&alpha_);
    bind("buffer_pkts_", (int *)&buffer_pkts);
    bind("backoff_tuner_", (double *)&backoff_tuner);
    bind("max_backoff_counter_", (int *)&max_backoff_counter);
    bind("MAC_addr_", (int *)&addr);

    if (max_tx_tries <= 0)
        max_tx_tries = INT_MAX;
    if (buffer_pkts > 0)
        has_buffer_queue = true;
}

UWOFDMAloha::~UWOFDMAloha()
{
}

// // TCL command interpreter

int UWOFDMAloha::command(int argc, const char *const *argv)
{
    Tcl &tcl = Tcl::instance();
    if (argc == 2)
    {
        if (strcasecmp(argv[1], "setAckMode") == 0)
        {
            ack_mode = UWOFDMALOHA_ACK_MODE;
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "setNoAckMode") == 0)
        {
            ack_mode = UWOFDMALOHA_NO_ACK_MODE;
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "setDisturbanceNode") == 0)
        {
            disturbanceNode = true;
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "initialize") == 0)
        {
            if (initialized == false)
                initInfo();
            if (print_transitions)
                fout.open("/tmp/OFDMALOHAstateTransitions.txt", ios_base::app);
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
				cout << "Carrier " << atoi(argv[2]) << " not usable " <<  endl;
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

void UWOFDMAloha::initInfo()
{

    initialized = true;

    if ((print_transitions) && (system(NULL)))
    {
        system("rm -f /tmp/OFDMALOHAstateTransitions.txt");
        system("touch /tmp/OFDMALOHAstateTransitions.txt");
    }

    status_info[UWOFDMALOHA_STATE_IDLE] = "Idle state";
    status_info[UWOFDMALOHA_STATE_TX_DATA] = "Transmit DATA state";
    status_info[UWOFDMALOHA_STATE_TX_ACK] = "Transmit ACK state";
    status_info[UWOFDMALOHA_STATE_WAIT_ACK] = "Wait for ACK state";
    status_info[UWOFDMALOHA_STATE_DATA_RX] = "DATA received state";
    status_info[UWOFDMALOHA_STATE_ACK_RX] = "ACK received state";
    status_info[UWOFDMALOHA_STATE_RX_IDLE] = "Start rx Idle state";
    status_info[UWOFDMALOHA_STATE_RX_WAIT_ACK] = "Start rx Wait ACK state";
    status_info[UWOFDMALOHA_STATE_CHK_ACK_TIMEOUT] = "Check Wait ACK timeout state";
    status_info[UWOFDMALOHA_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";
    status_info[UWOFDMALOHA_STATE_BACKOFF] = "Backoff state";
    status_info[UWOFDMALOHA_STATE_RX_BACKOFF] = "Start rx Backoff state";
    status_info[UWOFDMALOHA_STATE_CHK_BACKOFF_TIMEOUT] =
        "Check Backoff timeout state";
    status_info[UWOFDMALOHA_STATE_RX_IN_PROGRESS] = "Receiving Data";
    status_info[UWOFDMALOHA_STATE_MULTIPLE_RX_IN_PROGRESS] = "Receiving Multiple Data";

    reason_info[UWOFDMALOHA_REASON_DATA_PENDING] = "DATA pending from upper layers";
    reason_info[UWOFDMALOHA_REASON_DATA_RX] = "DATA received";
    reason_info[UWOFDMALOHA_REASON_DATA_TX] = "DATA transmitted";
    reason_info[UWOFDMALOHA_REASON_ACK_TX] = "ACK tranmsitted";
    reason_info[UWOFDMALOHA_REASON_ACK_RX] = "ACK received";
    reason_info[UWOFDMALOHA_REASON_ACK_TIMEOUT] = "ACK timeout";
    reason_info[UWOFDMALOHA_REASON_DATA_EMPTY] = "DATA queue empty";
    reason_info[UWOFDMALOHA_REASON_MAX_TX_TRIES] =
        "DATA dropped due to max tx rounds";
    reason_info[UWOFDMALOHA_REASON_START_RX] = "Start rx pkt";
    reason_info[UWOFDMALOHA_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
    reason_info[UWOFDMALOHA_REASON_WAIT_ACK_PENDING] = "Wait for ACK timer pending";
    reason_info[UWOFDMALOHA_REASON_PKT_ERROR] = "Erroneous pkt";
    reason_info[UWOFDMALOHA_REASON_BACKOFF_TIMEOUT] = "Backoff expired";
    reason_info[UWOFDMALOHA_REASON_BACKOFF_PENDING] = "Backoff timer pending";

    pkt_type_info[UWOFDMALOHA_ACK_PKT] = "ACK pkt";
    pkt_type_info[UWOFDMALOHA_DATA_PKT] = "DATA pkt";
    pkt_type_info[UWOFDMALOHA_DATAMAX_PKT] = "MAX payload DATA pkt";
}

// Initialize subCarriers parameters inside a node, default all carriers are used
void UWOFDMAloha::init_macofdm_node(int subCarNum, double subCarSize, int ctrl_subCar, string modulation)
{
    mac_ncarriers = subCarNum;
    ctrl_car = ctrl_subCar;
    mac_carrierSize = subCarSize;

    for (int i = 0; i < mac_ncarriers; i++)
    {
        mac_carVec.push_back(1);
        mac_carMod.push_back("BPSK");
    }
    for (int i = 0; i < nouse_carriers.size(); i++)
    {
        mac_carVec[nouse_carriers[i]] = 0;
    }
    return;
}

void UWOFDMAloha::updateRTT(double curr_rtt)
{
    srtt = alpha_ * srtt + (1 - alpha_) * curr_rtt;
    sumrtt += curr_rtt;
    sumrtt2 += curr_rtt * curr_rtt;
    rttsamples++;
    ACK_timeout = (sumrtt / rttsamples);
}

void UWOFDMAloha::updateAckTimeout(double rtt)
{
    updateRTT(rtt);

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::updateAckTimeout() curr ACK_timeout = " << ACK_timeout
             << endl;
}

void UWOFDMAloha::exitBackoff()
{
    backoff_timer.stop();
}

double
UWOFDMAloha::getBackoffTime()
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

    if (uwofdmaloha_debug)
    {
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::getBackoffTime() backoff time = " << backoff_duration
             << " s" << endl;
    }
    return backoff_duration;
}

double
UWOFDMAloha::computeTxTime(UWOFDMALOHA_PKT_TYPE type)
{
    double duration;
    Packet *temp_data_pkt;
    map<pktSeqNum, Packet *>::iterator it_p;

    if (type == UWOFDMALOHA_DATA_PKT)
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
    else if (type == UWOFDMALOHA_ACK_PKT)
    {
        temp_data_pkt = Packet::alloc();
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = ACK_size;
    }
    duration = Mac2PhyTxDuration(temp_data_pkt);
    Packet::free(temp_data_pkt);
    return (duration);
}

void UWOFDMAloha::recvFromUpperLayers(Packet *p)
{
    if(uwofdmaloha_debug)
        std::cout << NOW << "  UWOFDMAloha (" << addr
                 << ")::recvFromUpperLayers()"; 
    if (((has_buffer_queue == true) && (mapPacket.size() < buffer_pkts)) ||
        (has_buffer_queue == false))
    {
        initPkt(p, UWOFDMALOHA_DATA_PKT);
        // Q.push(p);
        putPktInQueue(p);
        incrUpperDataRx();
        waitStartTime();
        if (uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << addr
                 << ")::recvFromUpperLayers() current status "
                 << status_info[curr_state] << " prev state " << status_info[prev_state]
                 << " prev prev state " << status_info[prev_prev_state] << endl;
        if (uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << addr
                 << ")::recvFromUpperLayers() recv queue size: " << pkt_rcv_queue.size() << endl;

        if (curr_state == UWOFDMALOHA_STATE_IDLE && n_receptions == 0)
        {

            refreshReason(UWOFDMALOHA_REASON_DATA_PENDING);
            stateTxData();
        }
    }
    else
    {
        incrDiscardedPktsTx();
        drop(p, 1, UWOFDMALOHA_DROP_REASON_BUFFER_FULL);
    }
}

void UWOFDMAloha::initPkt(Packet *p, UWOFDMALOHA_PKT_TYPE type, int dest_addr)
{
    hdr_cmn *ch = hdr_cmn::access(p);
    hdr_mac *mach = HDR_MAC(p);
    hdr_MPhy *ph = HDR_MPHY(p);
    hdr_OFDM *ofdmph = HDR_OFDM(p);
    // use MF_BEACON for disturb packets

    ofdmph->carrierSize = mac_carrierSize;
    ofdmph->nativeOFDM = true;
    ofdmph->carrierNum = mac_ncarriers;

    if (uwofdmaloha_debug)
    {
        std::cout << NOW << "  UWOFDMAloha (" << addr
                  << ")::initPkt(), pkt destination " << dest_addr << std::endl;
    }

    for (std::size_t i = 0; i < mac_carVec.size(); ++i)
    {
        ofdmph->carriers[i] = mac_carVec[i];
        ofdmph->carMod[i] = mac_carMod[i];
    }
    if(uwofdmaloha_debug)
    displayCarriers(p);

    int curr_size = ch->size();

    switch (type)
    {

    case (UWOFDMALOHA_DATA_PKT):
    {
        ch->size() = curr_size + HDR_size;
        if (disturbanceNode)
            mach->ftype() = MF_BEACON;
        else
            mach->ftype() = MF_DATA;
    }
    break;

    case (UWOFDMALOHA_ACK_PKT):
    {
        ch->ptype() = PT_MMAC_ACK;
        ch->size() = ACK_size;
        ch->uid() = recv_data_id;
        mach->macSA() = addr;
        mach->macDA() = dest_addr;
    }
    break;
    }
    if (uwofdmaloha_debug)
    {
        std::cout << NOW << "  UWOFDMAloha (" << addr
                  << ")::initPkt() Packet SEQ NUM " << getPktSeqNum(p) << " initialized like:" << std::endl;
    }
}

void UWOFDMAloha::Mac2PhyStartTx(Packet *p)
{

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::Mac2PhyStartTx() start tx packet" << endl;

    hdr_OFDM *ofdmph = HDR_OFDM(p);

    hdr_mac *mach = HDR_MAC(p);
    MMac::Mac2PhyStartTx(p);

    if (uwofdmaloha_debug)
        std::cout << NOW << "  UWOFDMAloha (" << addr
             << ")::Mac2PhyStartTx() Sent down packet SEQ NUM " 
             << getPktSeqNum(p) << " to " << mach->macDA() << endl;
}

void UWOFDMAloha::Phy2MacEndTx(const Packet *p)
{
    hdr_mac *mach = HDR_MAC(p);

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::Phy2MacEndTx() end tx packet for node " << mach->macDA() << endl;

    switch (curr_state)
    {

    case (UWOFDMALOHA_STATE_TX_DATA):
    {
        refreshReason(UWOFDMALOHA_REASON_DATA_TX);
        if (ack_mode == UWOFDMALOHA_ACK_MODE)
        {

            if (uwofdmaloha_debug)
                cout << NOW << "  UWOFDMAloha (" << addr
                     << ")::Phy2MacEndTx() DATA sent,from "
                     << status_info[curr_state] << " to "
                     << status_info[UWOFDMALOHA_STATE_WAIT_ACK] << endl;

            stateWaitAck();
        }
        else
        {

            if (uwofdmaloha_debug)
                cout << NOW << "  UWOFDMAloha (" << addr
                     << ")::Phy2MacEndTx() DATA sent, from "
                     << status_info[curr_state] << " to "
                     << status_info[UWOFDMALOHA_STATE_IDLE] << endl;

            stateIdle();
        }
    }
    break;

    case (UWOFDMALOHA_STATE_TX_ACK):
    {
        refreshReason(UWOFDMALOHA_REASON_ACK_TX);

        if (uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << addr
                 << ")::Phy2MacEndTx() ack sent, from "
                 << status_info[curr_state] << " to "
                 << status_info[UWOFDMALOHA_STATE_IDLE] << endl;
        stateIdle();
    }
    break;

    default:
    {
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::Phy2MacEndTx() logical error, current state = "
             << status_info[curr_state] << " prev state " << status_info[prev_state] 
             << " reason " << reason_info[last_reason] << endl;
        stateIdle();
        // exit(1);
    }
    break;
    }
}

void UWOFDMAloha::Phy2MacStartRx(const Packet *p)
{
    hdr_mac *mach = HDR_MAC(p);
    Packet *temp_packet = p->copy();
    n_receptions++;
    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr << ")::Phy2MacStartRx() rx Packet SEQ NUM " 
        << getPktSeqNum(temp_packet) << " from " << mach->macSA() << endl;
}

void UWOFDMAloha::Phy2MacEndRx(Packet *p)
{
    hdr_cmn *ch = HDR_CMN(p);
    packet_t rx_pkt_type = ch->ptype();
    hdr_mac *mach = HDR_MAC(p);
    MacFrameType frame_type = mach->ftype();
    hdr_MPhy *ph = HDR_MPHY(p);

    int source_mac = mach->macSA();
    int dest_mac = mach->macDA();

    double gen_time = ph->txtime;
    double received_time = ph->rxtime;
    double diff_time = received_time - gen_time;

    double distance = diff_time * prop_speed;

    n_receptions--;

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr << ")::Phy2MacEndRx() "
             << status_info[curr_state]
             << ", received a pkt type = " << ch->ptype()
             << ", src addr = " << mach->macSA()
             << " dest addr = " << mach->macDA()
             << ", estimated distance between nodes = " << distance << " m "
             << endl;

    if (ch->error())
    {

        if (uwofdmaloha_debug)
            cout << NOW << "  UWOFDMAloha (" << addr
                 << ")::Phy2MacEndRx() dropping corrupted pkt " << endl;
        incrErrorPktsRx();

        refreshReason(UWOFDMALOHA_REASON_PKT_ERROR);
        drop(p, 1, UWOFDMALOHA_DROP_REASON_ERROR);
        return;
    }
    else
    {
        if (dest_mac == addr || dest_mac == MAC_BROADCAST)
        {
            if (frame_type == MF_ACK)
            {
                if (ack_mode == UWOFDMALOHA_NO_ACK_MODE)
                {
                    refreshReason(UWOFDMALOHA_REASON_PKT_NOT_FOR_ME);
                    // Packet::free(p);
                    if (uwofdmaloha_debug)
                        cout << NOW << "  UWOFDMAloha (" << addr
                             << ")::Phy2MacEndRx() PKT NOT FOR ME 111 (wasn't waiting ACKS)" << endl;
                    drop(p, 1,  UWOFDMALOHA_DROP_REASON_WRONG_RECEIVER);
                    return;
                }
                if (curr_state != UWOFDMALOHA_STATE_WAIT_ACK)
                {
                    cout << NOW << "  UWOFDMAloha (" << addr
                         << ")::Phy2MacEndRx() PKT NOT FOR ME 333 (wasn't waiting ACKS)" << endl;

                    drop(p, 1,  UWOFDMALOHA_DROP_REASON_WRONG_RECEIVER);

                    return;
                }
                if (getPktSeqNum(p) != waitforpktnum)
                {
                    cout << NOW << "  UWOFDMAloha (" << addr
                         << ")::Phy2MacEndRx() PKT NOT FOR ME 444 (Previous ACK)" << endl;
                    drop(p, 1,  UWOFDMALOHA_DROP_REASON_WRONG_RECEIVER);
                    return;
                }
                refreshReason(UWOFDMALOHA_REASON_ACK_RX);
                stateRxAck(p);
            }
            else if (frame_type == MF_DATA && curr_state == UWOFDMALOHA_STATE_IDLE)
            {
                refreshReason(UWOFDMALOHA_REASON_DATA_RX);
                if (uwofdmaloha_debug)
                    cout << NOW << "  UWOFDMAloha (" << addr
                         << ")::Phy2MacEndRx() going to stateRxData " << endl;

                stateRxData(p);
            }
            else
            {
                refreshReason(UWOFDMALOHA_REASON_PKT_NOT_FOR_ME);

                if (uwofdmaloha_debug)
                    std::cout << NOW << "  UWOFDMAloha (" << addr
                         << ")::Phy2MacEndRx() NOT IN CONDITION TO RECEIVE FOR NOW " << endl;
                drop(p, 1,  UWOFDMALOHA_DROP_REASON_WRONG_RECEIVER);
                return;
            }
        }
        else
        {
            refreshReason(UWOFDMALOHA_REASON_PKT_NOT_FOR_ME);
            if(frame_type != MF_RTS && frame_type != MF_CTS && frame_type != MF_ACK && frame_type != MF_DATA){
					cerr << NOW << "  UWSmartOFDM (" << addr << ")::Phy2MacEndRx Unrecognized packet " << std::endl;
					drop(p, 1,  UWOFDMALOHA_DROP_REASON_WRONG_RECEIVER);
			}
            else {
                
            if (uwofdmaloha_debug)
                cout << NOW << "  UWOFDMAloha (" << addr
                     << ")::Phy2MacEndRx() PKT NOT FOR ME 222x " << endl;

            drop(p, 1,  UWOFDMALOHA_DROP_REASON_WRONG_RECEIVER);
            }
            return;
        }
    }
}

void UWOFDMAloha::txData()
{
    Packet *data_pkt = curr_data_pkt->copy();

    if ((ack_mode == UWOFDMALOHA_NO_ACK_MODE))
    {
        eraseItemFromPktQueue(getPktSeqNum(data_pkt));
    }
    waitforpktnum = getPktSeqNum(data_pkt);
    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::txData sending SEQ NUM PACKET " << getPktSeqNum(data_pkt) << std::endl;

    incrDataPktsTx();
    incrCurrTxRounds();
    Mac2PhyStartTx(data_pkt);
}

void UWOFDMAloha::txAck(int dest_addr)
{
    Packet *ack_pkt = Packet::alloc();
    initPkt(ack_pkt, UWOFDMALOHA_ACK_PKT, dest_addr);

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::txAck sending ACK to " << dest_addr << std::endl;

    incrAckPktsTx();
    Mac2PhyStartTx(ack_pkt);
}

void UWOFDMAloha::stateCheckAckExpired()
{
    refreshState(UWOFDMALOHA_STATE_CHK_ACK_TIMEOUT);

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr << ")::stateCheckAckExpired()"
             << endl;

    map<pktSeqNum, AckTimer>::iterator it_a;
    it_a = mapAckTimer.begin();

    if (print_transitions)
        printStateInfo();
    if (((*it_a).second).isActive())
    {
        refreshReason(UWOFDMALOHA_REASON_WAIT_ACK_PENDING);
        refreshState(UWOFDMALOHA_STATE_WAIT_ACK);
    }
    else if (((*it_a).second).isExpired())
    {
        refreshReason(UWOFDMALOHA_REASON_ACK_TIMEOUT);
        stateBackoff();
    }
    else
    {
        cerr << NOW << "  UWOFDMAloha (" << addr
             << ")::stateCheckAckExpired() ack_timer logical error, current "
                "timer state = "
             << status_info[curr_state] << endl;
        stateIdle();
    }
}

void UWOFDMAloha::stateCheckBackoffExpired()
{
    refreshState(UWOFDMALOHA_STATE_CHK_BACKOFF_TIMEOUT);

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr << ")::stateCheckBackoffExpired()"
             << endl;
    if (print_transitions)
        printStateInfo();
    if (backoff_timer.isActive())
    {
        refreshReason(UWOFDMALOHA_REASON_BACKOFF_PENDING);
        stateBackoff();
    }
    else if (backoff_timer.isExpired())
    {
        refreshReason(UWOFDMALOHA_REASON_BACKOFF_TIMEOUT);
        exitBackoff();
        stateIdle();
    }
    else
    {
        cerr << NOW << "  UWOFDMAloha (" << addr
             << ")::stateCheckBackoffExpired() backoff_timer logical error, "
                "current timer state = "
             << status_info[curr_state] << endl;
        stateIdle();
    }
}

void UWOFDMAloha::stateIdle()
{
    mapAckTimer.clear();
    backoff_timer.stop();

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::stateIdle() reception queue size = " << pkt_rcv_queue.size() << endl;

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr
             << ")::stateIdle() transmission queue size = " << mapPacket.size() << endl;

    refreshState(UWOFDMALOHA_STATE_IDLE);

    if (print_transitions)
        printStateInfo();

  if (!mapPacket.empty() && n_receptions == 0)
    {
        stateTxData();
    }
}

void UWOFDMAloha::stateRxIdle()
{
    refreshState(UWOFDMALOHA_STATE_RX_IDLE);

    if (print_transitions)
        printStateInfo();
}

void UWOFDMAloha::stateBackoff()
{
    backoff_timer.force_cancel();
    refreshState(UWOFDMALOHA_STATE_BACKOFF);

    if (backoff_timer.isFrozen())
        backoff_timer.unFreeze();
    else
        backoff_timer.schedule(getBackoffTime());

    if (uwofdmaloha_debug)
        std::cout << NOW << "  UWOFDMAloha (" << addr << ")::stateBackoff() " << endl;
    if (uwofdmaloha_debug && mapPacket.size() > 0)
    {

        map<pktSeqNum, Packet *>::iterator it_p;
        it_p = mapPacket.begin();
        if (uwofdmaloha_debug)
            displayCarriers((*it_p).second);
    }
    if (print_transitions)
        printStateInfo(backoff_timer.getDuration());
}

void UWOFDMAloha::stateRxBackoff()
{
    backoff_timer.freeze();
    refreshState(UWOFDMALOHA_STATE_RX_BACKOFF);

    if (print_transitions)
        printStateInfo();
}

void UWOFDMAloha::stateTxData()
{
    refreshState(UWOFDMALOHA_STATE_TX_DATA);

    if (uwofdmaloha_debug)
        std::cout << NOW << "  UWOFDMAloha (" << addr << ")::stateTxData() " << endl;
    if (print_transitions)
        printStateInfo();

    map<pktSeqNum, Packet *>::iterator it_p;
    it_p = mapPacket.begin();
    curr_data_pkt = (*it_p).second;
    int seq_num;
    seq_num = getPktSeqNum(curr_data_pkt);

    hdr_OFDM *ofdmph = HDR_OFDM(curr_data_pkt);
    if (uwofdmaloha_debug)
        for (int i = 0; i < mac_ncarriers; i++)
        {
            std::cout << "carrier[" << i << "] = " << ofdmph->carMod[i] << std::endl;
        }

    map<pktSeqNum, AckTimer>::iterator it_a;

    if (seq_num != last_sent_data_id)
    {
        putAckTimerInMap(seq_num);
        it_a = mapAckTimer.find(seq_num);
        resetCurrTxRounds();
        backoff_timer.resetCounter();
        ((*it_a).second).resetCounter();
        hdr_mac *mach = HDR_MAC(curr_data_pkt);
        start_tx_time = NOW; // we set curr RTT
        last_sent_data_id = seq_num;
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
                txData();
            }
            else
            {
                eraseItemFromPktQueue(seq_num);
                incrDroppedPktsTx();

                refreshReason(UWOFDMALOHA_REASON_MAX_TX_TRIES);

                if (uwofdmaloha_debug)
                    cout << NOW << "  UWOFDMAloha (" << addr
                         << ")::statePreTxData() curr_tx_rounds "
                         << curr_tx_rounds
                         << " > max_tx_tries = " << max_tx_tries << endl;
                waitforpktnum = -1;
                stateIdle();
            }
        } else {
            stateCheckAckExpired();
        }
    }
}

void UWOFDMAloha::stateWaitAck()
{

    map<pktSeqNum, AckTimer>::iterator it_a;
    it_a = mapAckTimer.begin();

    ((*it_a).second).stop();
    refreshState(UWOFDMALOHA_STATE_WAIT_ACK);

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr << ")::stateWaitAck() " << endl;
    if (print_transitions)
        printStateInfo();

    ((*it_a).second).incrCounter();
    ((*it_a).second).schedule(ACK_timeout + 2 * wait_constant);
}

void UWOFDMAloha::stateRxWaitAck()
{
    refreshState(UWOFDMALOHA_STATE_RX_WAIT_ACK);

    if (print_transitions)
        printStateInfo();
}

void UWOFDMAloha::stateTxAck(int dest_addr)
{
    refreshState(UWOFDMALOHA_STATE_TX_ACK);

    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr << ")::stateTxAck() dest addr "
             << dest_addr << endl;
    if (print_transitions)
        printStateInfo();

    txAck(dest_addr);
}

void UWOFDMAloha::stateRxData(Packet *data_pkt)
{
    ack_timer.stop();
    refreshState(UWOFDMALOHA_STATE_DATA_RX);

    refreshReason(UWOFDMALOHA_REASON_DATA_RX);

    hdr_mac *mach = HDR_MAC(data_pkt);
    int dst_addr = mach->macSA();

    hdr_cmn *ch = hdr_cmn::access(data_pkt);
    recv_data_id = ch->uid();
    ch->size() = ch->size() - HDR_size;
    incrDataPktsRx();
    sendUp(data_pkt);
    if (uwofdmaloha_debug)
        cout << NOW << "  UWOFDMAloha (" << addr << ")::stateRxData() SEQ NUM " 
        << getPktSeqNum(data_pkt) << " from " << dst_addr << endl;
    if (ack_mode == UWOFDMALOHA_ACK_MODE)
        stateTxAck(dst_addr);
    else
        stateIdle();
}

void UWOFDMAloha::stateRxAck(Packet *p)
{

    map<pktSeqNum, AckTimer>::iterator it_a;
    it_a = mapAckTimer.begin();

    ((*it_a).second).stop();
    refreshState(UWOFDMALOHA_STATE_ACK_RX);
    if (uwofdmaloha_debug)
        cout << NOW << " UWOFDMAloha (" << addr << ")::stateRxAck() " << endl;

    int seq_num;
    seq_num = getPktSeqNum(p);

    if (uwofdmaloha_debug)
        cout << NOW << " UWOFDMAloha (" << addr << ")::stateRxAck() seq number " << seq_num << endl;
    Packet::free(p);

    refreshReason(UWOFDMALOHA_REASON_ACK_RX);
    if (uwofdmaloha_debug)
        cout << NOW << " UWOFDMAloha (" << addr << ")::stateRxAck() refreshReason " << endl;

    eraseItemFromPktQueue(seq_num);
    if (uwofdmaloha_debug)
        cout << NOW << " UWOFDMAloha (" << addr << ")::stateRxAck() erase from pkt queue" << endl;
    eraseItemFrommapAckTimer(seq_num);
    if (uwofdmaloha_debug)
        cout << NOW << " UWOFDMAloha (" << addr << ")::stateRxAck() erase from ACK timer" << endl;
    updateAckTimeout(NOW - start_tx_time);
    incrAckPktsRx();
    if (uwofdmaloha_debug)
        cout << NOW << " UWOFDMAloha (" << addr << ")::stateRxAck() Going back to Idle" << endl;
    stateIdle();
}

void UWOFDMAloha::printStateInfo(double delay)
{
    if (uwofdmaloha_debug)
        cout << NOW << " UWOFDMAloha (" << addr << ")::printStateInfo() "
             << "from " << status_info[prev_state] << " to "
             << status_info[curr_state]
             << ". Reason: " << reason_info[last_reason] << endl;

    if (curr_state == UWOFDMALOHA_STATE_BACKOFF)
    {
        fout << left << setw(10) << NOW << "  UWOFDMAloha (" << addr
             << ")::printStateInfo() "
             << "from " << status_info[prev_state] << " to "
             << status_info[curr_state]
             << ". Reason: " << reason_info[last_reason]
             << ". Backoff duration = " << delay << endl;
    }
    else
    {
        fout << left << setw(10) << NOW << "  UWOFDMAloha (" << addr
             << ")::printStateInfo() "
             << "from " << status_info[prev_state] << " to "
             << status_info[curr_state]
             << ". Reason: " << reason_info[last_reason] << endl;
    }
}