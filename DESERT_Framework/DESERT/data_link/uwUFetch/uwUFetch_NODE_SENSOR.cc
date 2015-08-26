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
 * @file   uwUFetch_NODE_SENSOR.cc
 * @author Loris Brolo
 * @version 1.0.0
 * 
 * @brief Class that provide the implementation of the SENSOR NODE entity of uwUFetch protocol
 */

#include "mac.h"
#include "mmac.h"
#include "uwUFetch_NODE.h"
#include "uwUFetch_cmn_hdr.h"
//#include "uwmphy_modem_cmn_hdr.h"
#include "uwcbr-module.h"
#include <sstream>
#include <time.h>

/**< class uwUFetchNODEModuleClass*/
static class uwUFetchNODEModuleClass : public TclClass {
public:

    /**
     * Constructor of uwUFetchNODEModuleClass class
     */
    uwUFetchNODEModuleClass() : TclClass("Module/UW/UFETCH/NODE") {
    }

    TclObject* create(int, const char*const*) {
        return (new uwUFetch_NODE());
    }
} class_module_uwufetchnode;

bool uwUFetch_NODE::initialized = false;

std::map< uwUFetch_NODE::UWUFETCH_NODE_STATUS, std::string > uwUFetch_NODE::statusInfo;
std::map< uwUFetch_NODE::UWUFETCH_NODE_STATUS_CHANGE, std::string > uwUFetch_NODE::statusChange;
std::map< uwUFetch_NODE::UWUFETCH_NODE_PACKET_TYPE, std::string > uwUFetch_NODE::packetType;
std::map< uwUFetch_NODE::UWUFETCH_TIMER_STATUS, std::string > uwUFetch_NODE::statusTimer;

/**
 * Constructor of the class uwUFetch_NODE
 */
uwUFetch_NODE::uwUFetch_NODE()
: /***SENSOR NODE***/
//timers
BCK_timer_rts(this),
CTS_timer(this),
BCK_timer_data(this),
BCK_timer_probe(this),
POLL_timer(this),
DATA_BEFORE_TX_timer(this),
CBEACON_timer(this),
// Global Variables that should be never reset
rx_BEACON_start_time(0),
rx_BEACON_finish_time(0),
tx_PROBE_start_time(0),
tx_PROBE_finish_time(0),
rx_POLL_start_time(0),
rx_POLL_finish_time(0),
tx_DATA_start_time(0),
tx_DATA_finish_time(0),
rx_CBEACON_start_time(0),
rx_CBEACON_finish_time(0),
n_tot_BEACON_pck_rx_by_NODE(0),
n_tot_BEACON_pck_rx_corr_by_NODE(0),
n_tot_PROBE_pck_tx_by_NODE(0),
n_tot_POLL_pck_rx_by_NODE(0),
n_tot_POLL_pck_rx_corr_by_NODE(0),
n_tot_DATA_pck_tx_by_NODE(0),
n_tot_CBEACON_pck_rx_by_NODE(0),
n_tot_CBEACON_pck_rx_corr_by_NODE(0),
T_min_bck_probe_node(0),
T_max_bck_probe_node(0),
num_cbeacon_node_rx(0),
mac_addr_HN_in_beacon(0),
bck_before_tx_probe(0),
Tprobe(0),
Tdata_NODE(0),
RTT(0),
mac_addr_HN_in_poll(0),
num_pck_to_tx_by_NODE(0),
num_cbeacon_at_now_HN_tx(0),
mac_addr_HN_in_cbeacon(0),
// Variables that should be reseted when a cycle is end
n_BEACON_pck_rx_by_NODE(0),
n_PROBE_pck_tx_by_NODE(0),
n_POLL_pck_rx_by_NODE(0),
n_DATA_pck_tx_by_NODE(0),
n_CBEACON_pck_rx_by_NODE(0),
dataAlreadyTransmitted(false),
// Variables that enable or not an operation
rxBEACONEnabled(false),
txPROBEEnabled(false),
rxPOLLEnabled(false),
txDATAEnabled(false),
rxCBEACONEnabled(false),
curr_BEACON_NODE_pck_rx(NULL),
curr_PROBE_NODE_pck_tx(NULL),
curr_POLL_NODE_pck_rx(NULL),
curr_DATA_NODE_pck_tx(NULL),
curr_CBEACON_NODE_pck_rx(NULL),
/***HEAD NODE***/
//Timers
BeaconBeforeTx_timer(this),
PROBE_timer(this),
DATA_timer(this),
// Global Variables that should be never reset
tx_BEACON_start_HN_time(0),
tx_BEACON_finish_HN_time(0),
rx_PROBE_start_HN_time(0),
rx_PROBE_finish_HN_time(0),
tx_POLL_start_HN_time(0),
tx_POLL_finish_HN_time(0),
rx_DATA_start_HN_time(0),
rx_DATA_finish_HN_time(0),
tx_CBEACON_start_HN_time(0),
tx_CBEACON_finish_HN_time(0),
rx_TRIGGER_start_HN_time(0),
rx_TRIGGER_finish_HN_time(0),
tx_RTS_start_HN_time(0),
tx_RTS_finish_HN_time(0),
rx_CTS_start_HN_time(0),
rx_CTS_finish_HN_time(0),
tx_DATA_start_HN_time(0),
tx_DATA_finish_HN_time(0),
n_tot_BEACON_pck_tx_by_HN(0),
n_tot_PROBE_pck_rx_by_HN(0),
n_tot_PROBE_pck_corr_rx_by_HN(0),
n_tot_POLL_pck_tx_by_HN(0),
n_tot_DATA_pck_rx_by_HN(0),
n_tot_DATA_pck_rx_corr_by_HN(0),
n_tot_CBEACON_pck_tx_by_HN(0),
n_tot_TRIGGER_pck_rx_by_HN(0),
n_tot_TRIGGER_pck_rx_corr_by_HN(0),
n_tot_DATA_pck_tx_by_HN(0),
mac_addr_NODE_polled(0),
number_data_pck_HN_rx_exact(0),
pck_number_id(1),
data_timeout(0),
Tbeacon(0),
Tpoll(0),
Tdata_HN(0),
mac_addr_NODE_in_data(0),
T_min_bck_RTS(0),
T_max_bck_RTS(0),
T_min_bck_DATA(0),
T_max_bck_DATA(0),
bck_before_tx_data(0),
mac_addr_AUV_in_trigger(0),
mac_addr_AUV_in_CTS(0),
bck_before_tx_RTS(0),
max_data_HN_can_tx(0),
max_pck_HN_can_tx(0),
Tdata_HN_pck(0),
CTSrx(false),
txRTSbeforeCBEACON(false),
sectionCBeacon(false),
// Variables that should be reseted when a cycle is end
n_BEACON_pck_tx_by_HN(0),
n_PROBE_pck_rx_by_HN(0),
n_POLL_pck_tx_by_HN(0),
n_DATA_pck_rx_by_HN(0),
n_CBEACON_pck_tx_by_HN(0),
n_TRIGGER_pck_rx_by_HN(0),
n_RTS_pck_tx_by_HN(0),
n_CTS_pck_rx_by_HN(0),
n_DATA_pck_tx_by_HN(0),
// Variables that enable or not an operation
txBEACONEnabled(false),
rxPROBEEnabled(false),
txPOLLEnabled(false),
rxDATAEnabled(false),
txCBEACONEnabled(false),
rxTRIGGEREnabled(false),
txRTSEnabled(false),
rxCTSEnabled(false),
txDATAEnabledHN(false),
// PACKETS create
curr_BEACON_HN_pck_tx(NULL),
curr_PROBE_HN_pck_rx(NULL),
curr_POLL_HN_pck_tx(NULL),
curr_DATA_HN_pck_rx(NULL),
curr_CBEACON_HN_pck_tx(NULL),
curr_TRIGGER_HN_pck_rx(NULL),
curr_RTS_HN_pck_tx(NULL),
curr_CTS_HN_pck_rx(NULL),
curr_DATA_HN_pck_tx(NULL),
curr_DATA_NODE_pck_tx_HN(NULL) {
    //variable binding
    mac2phy_delay_ = 1e-19;
    bind("TIME_BEFORE_START_COMU_HN_NODE_", (double*) & T_START_PROCEDURE_HN_NODE);
    bind("MAXIMUM_VALUE_BACKOFF_PROBE_", (double*) & T_MAX_BACKOFF_PROBE);
    bind("MINIMUM_VALUE_BACKOFF_PROBE_", (double*) & T_MIN_BACKOFF_PROBE);
    bind("MAXIMUM_NODE_POLLED_", (int*) & MAX_POLLED_NODE);
    bind("MAXIMUM_PAYLOAD_SIZE_", (int*) & MAX_PAYLOAD);
    bind("TIME_TO_WAIT_PROBES_PCK_", (double*) & T_PROBE);
    bind("TIME_TO_WAIT_POLL_PCK_", (double*) & T_POLL);
    bind("TIME_BETWEEN_2_DATA_TX_HN_", (double*) & TIME_BETWEEN_2_TX_DATA_HN_AUV);
    bind("TIME_BETWEEN_2_DATA_TX_NODE_", (double*) & TIME_BETWEEN_2_TX_DATA_NODE_HN);
    bind("SEE_THE_TRANSITIONS_STATE_", (int*) & PRINT_TRANSITIONS_INT);
    bind("GUARD_INTERVAL_", (double*) & T_GUARD);
    bind("MAXIMUM_BUFFER_SIZE_", (int*) & MAXIMUM_BUFFER_DATA_PCK_NODE);
    bind("MAXIMUM_CBEACON_TRANSMISSIONS_", (int*) & MAX_ALLOWED_CBEACON_TX);
    bind("MAXIMUM_PCK_WANT_RX_HN_FROM_NODE_", (int*) & MAX_PCK_HN_WANT_RX_FROM_NODE);
    bind("MY_DEBUG_", (int*) & debugMio_);
    bind("NUMBER_OF_RUN_", (int*) & N_RUN);
    bind("TIME_TO_WAIT_CTS_", (double*) & T_CTS);
    bind("MODE_COMM_", (int*) & MODE_COMM_HN_AUV);
    bind("BURST_DATA_", (int*) & MODE_BURST_DATA);
} //end uwUFetch_NODE()

int uwUFetch_NODE::crLayCommand(ClMessage* m) {
    switch (m->type()) {
        default:
            return MMac::crLayCommand(m);
    }
} //end crLCommand()

uwUFetch_NODE::~uwUFetch_NODE() {
    //nothing to do
} //end ~uwUFetch_NODE()

int uwUFetch_NODE::command(int argc, const char*const*argv) {
    Tcl& tcl = Tcl::instance();

    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            if (initialized == false) initInfo();
            if (debug_) fout.open("/tmp/uwUFetch_AUV_stateTransitions.txt", std::ios_base::app);
            if (debugMio_) {
                std::stringstream file_logging;
                file_logging << "uwUFetch_NODE_" << addr << "_N_RUN_" << N_RUN << ".out";
                out_file_logging.open(file_logging.str().c_str(), std::ios_base::app);
            }
            return TCL_OK;

        } else if (strcasecmp(argv[1], "printTransitions") == 0) {
            print_transitions = true;

            return TCL_OK;
        } else if (strcasecmp(argv[1], "getDataQueueSize") == 0) {

            return TCL_OK;

        } else if (strcasecmp(argv[1], "getBEACONrxByNODE") == 0) {
            tcl.resultf("%d", getTotalBeaconPckRx_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getBEACONrxCorruptedByNODE") == 0) {
            tcl.resultf("%d", getTotalBeaconPckRx_corrupted_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getPROBEtxByNODE") == 0) {
            tcl.resultf("%d", getTotalProbePckTx_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getPOLLrxByNODE") == 0) {
            tcl.resultf("%d", getTotalPollPckRx_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getPOLLrxCorruptedByNODE") == 0) {
            tcl.resultf("%d", getTotalPollPckRx_corrupted_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getDATAtxByNODE") == 0) {
            tcl.resultf("%d", getTotalDataPckTx_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getCBEACONrxByNODE") == 0) {
            tcl.resultf("%d", getTotalCBeaconPckRx_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getCBEACONrxCorruptedByNODE") == 0) {
            tcl.resultf("%d", getTotalCBeaconPckRx_corrupted_by_NODE());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "SimpleNodeStart") == 0) {
            stateIdle_NODE();
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getBEACONtxByHN") == 0) {
            tcl.resultf("%d", getTotalBeaconTx_by_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getPROBErxByHN") == 0) {
            tcl.resultf("%d", getTotalProbePckRx_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getPROBErxCorruptedByHN") == 0) {
            tcl.resultf("%d", getTotalProbePckRx_corrupted_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getPOLLtxByHN") == 0) {
            tcl.resultf("%d", getTotalPollPckTx_by_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getDATArxByHN") == 0) {
            tcl.resultf("%d", getTotalDataPckRx_by_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getDATArxCorruptedByHN") == 0) {
            tcl.resultf("%d", getTotalDataPckRx_corrupted_by_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getCBEACONtxbyHN") == 0) {
            tcl.resultf("%d", getTotalCBeaconPckTx_by_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getTRIGGERrxByHN") == 0) {
            tcl.resultf("%d", getTotalTriggerPckRx_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getTRIGGERrxCorrupteByHN") == 0) {
            tcl.resultf("%d", getTotalTriggerPckRx_corrupted_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getRTStxByHN") == 0) {
            tcl.resultf("%d", getTotalRtsPckTx_by_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getCTSrxByHN") == 0) {
            tcl.resultf("%d", getTotalCtsPckRx_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getCTSrxCorrupteByHN") == 0) {
            tcl.resultf("%d", getTotalCtsPckRx_corrupted_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "getDATAtxByHN") == 0) {
            tcl.resultf("%d", getTotalDataPckTx_by_HN());
            return TCL_OK;

        } else if (strcasecmp(argv[1], "HeadNodeStart") == 0) {
            stateIdle_HN();
            return TCL_OK;

        } else if (strcasecmp(argv[1], "BeHeadNode") == 0) {
            HEADNODE = 1;
            return TCL_OK;

        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "setMacAddr") == 0) {
            addr = atoi(argv[2]);
            if (debug_) std::cout << "UWFETCH_NODE MAC address is:" << addr << std::endl;
            return TCL_OK;
        }
    }
    return MMac::command(argc, argv);
}//end command

void uwUFetch_NODE::initInfo() {
    initialized = true;

    if ((print_transitions) && (system(NULL))) {
        system("rm -f /tmp/uwUFetch_NODE_stateTransitions.txt");
        system("touch /tmp/uwUFetch_NODE_stateTransitions.txt");
    }
    //State in which the NODE may be
    statusInfo[UWUFETCH_NODE_STATUS_IDLE] = "Node is in IDLE state";
    statusInfo[UWUFETCH_NODE_STATUS_BEACON_RECEIVE] = "NODE has received a BEACON packet from HN";
    statusInfo[UWUFETCH_NODE_STATUS_TRANSMIT_PROBE] = "NODE is transmitting PROBE packet to HN";
    statusInfo[UWUFETCH_NODE_STATUS_WAIT_POLL_PACKET] = "NODE is waiting for POLL packet from HN";
    statusInfo[UWUFETCH_NODE_STATUS_POLL_RECEIVE] = "NODE has received a POLL packet from HN";
    statusInfo[UWUFETCH_NODE_STATUS_TRANSMIT_DATA] = "NODE is transmitting DATA packet to HN";
    statusInfo[UWUFETCH_NODE_STATUS_TRIGGER_RECEIVE] = "HN has received a TRIGGER packet from the AUV";
    statusInfo[UWUFETCH_NODE_STATUS_RTS_TRANSMIT] = "HN has transmitted a RTS packet to the AUV";
    statusInfo[UWUFETCH_NODE_STATUS_WAIT_CTS_PACKET] = "HN is waiting to the CTS packet from the AUV";
    statusInfo[UWUFETCH_NODE_STATUS_CTS_RECEIVE] = " HN has received a CTS packet from the AUV";
    statusInfo[UWUFETCH_NODE_STATUS_TRANSMIT_BEACON] = "HN is transmitting the BEACON packet to the NODE";
    statusInfo[UWUFETCH_NODE_STATUS_WAIT_PROBE_PACKET] = "HN is waiting to receive a PROBE packet from the NODE";
    statusInfo[UWUFETCH_NODE_STATUS_PROBE_RX] = "HN has received a PROBE packet from the NODE";
    statusInfo[UWUFETCH_NODE_STATUS_POLL_TX] = "HN is transmitting the POLL packet to the NODE";
    statusInfo[UWUFETCH_NODE_STATUS_WAIT_DATA_NODE] = "HN is waiting a DATA packet from the NODE";
    statusInfo[UWUFETCH_NODE_STATUS_DATA_RX] = "HN has received a DATA packet from the NODE";
    statusInfo[UWUFETCH_NODE_STATUS_WAIT_CBEACON_PACKET] = "NODE is waiting to receive a CBEACON packet from the HN";
    statusInfo[UWUFETCH_NODE_STATUS_CBEACON_RECEIVE] = "NODE has received a CBEACON packet from the HN";
    //Reason for which the NODE change his state
    statusChange[UWFETCH_NODE_STATUS_CHANGE_STATE_IDLE] = "NODE is in IDLE STATE";
    statusChange[UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR] = "NODE has receive a packet that is corrupted";
    statusChange[UWFETCH_NODE_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE] = "NODE has received a packet that is addressed for another node";
    statusChange[UWFETCH_NODE_STATUS_CHANGE_PACKET_UNKNOWN_TYPE] = "NODE has received a packet that the type is unknown";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_BEACON_RX] = "NODE has received  a BEACON packet";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_PROBE_TX] = "NODE has transmitted a PROBE packet";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_POLL_EXPIRED] = "POLL timeout is expired";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_POLL_RX] = "NODE has received a POLL packet";
    statusChange[UWFETCH_NODE_STATUS_CHANGE_CBEACON_RX_DATA_ALREADY_TX] = "NODE has received a CBEACON packet but it has already transmit its DATA packets";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_MAX_DATA_PCK_TX] = "NODE has transmitted the maximum number of packets";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_PCK_TX] = "NODE has transmitted a DATA packets";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX] = "HN has received a TRIGGER packet";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_RTS_TX] = "HN has transmitted a RTS packet to tha AUV";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_CTS_EXPIRED] = "CTS timeout expired";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_CTS_RX] = "HN has received a CTS packet from the AUV";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_BEACON_TX] = "HN has transmitted a BEACON packet to the NODE";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_CBEACON_TX] = "HN has transmitted a CBEACON packet to the NODe";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_PROBE_RX] = "HN has received a BEACON packet from the NODE";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_CBEACON_MAX_ALLOWED_TX_0_PROBE_RX] = "HN has 0 NODE to poll and maximum number of CBEACON has been transmitted";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_CBEACON_ALLOWED_TX_0_PROBE_RX] = "HN has 0 NODE to poll and another CBEACON can be transmitted";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_MAX_PROBE_RX_PROBE_TO_NOT_EXPIRED] = "HN has received the maximum number of PROBE allowed and the PROBE timeout is not still expired";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_TO_EXPIRED_AT_LEAST_1_PROBE_RX] = "PROBE timeout is expired and at least 1 node can be polled by the HN";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_POLL_TX] = "HN has transmitted a POLL packet to the NODE";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED] = "DATA timeout is expire";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED_0_NODE_TO_POLL_NO_OTHER_CBEACON] = "DATA timeout is expire and there are 0 node to poll and other CBEACONS can not be transmitted";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED_0_NODE_TO_POLL_YES_OTHER_CBEACON] = "DATA timeout is expire and there are 0 node to poll and at least one CBEACON can be transmitted";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED_ANOTHER_NODE_TO_POLL] = "DATA timeout is expire and at least 1 node can be polled";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_RX] = "HN has received a DATA packet from the NODE";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_LAST_DATA_PCK_RX] = "HN has received the last DATA packet from the NODE";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_NO_OTHER_CBEACON] = "HN has received all the packet from the NODE, there are 0 node to polled and it's not possible to transmit other CBEACON";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_YES_OTHER_CBEACON] = "HN has received all the packet from the NODE, there are 0 node to polled and it's possible to transmit other CBEACON";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_YES_NODE_TO_POLL] = "HN has received all the packet from the NODE, there are at least one node that can be polled";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_CBEACON_MAX_ALLOWED_TX] = "HN has transmit the maximum number of CBEACON packets";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_BEACON_TO_EXPIRED] = "BEACON timeout is expired";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX_BUT_HN_NO_DATA_TO_TX] = "Trigger received by the HN, but HN has no DATA available to transmit to the AUV";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_HN_TX_ALL_DATA_TO_AUV] = "HN has transmitted all his DATA packet to the AUV";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_HN_RX_TRIGGER_FROM_AUV_CORRUPTED] = "HN has received a TRIGGER from AUV that is corrupted by the channel";
    statusChange[UWUFETCH_NODE_STATUS_CHANGE_HN_FINISH_TX_DATA_PCK_TO_AUV_RETURN_CBEACON_TX] = "HN has finished to transmit a DATA packets to the AUV, and now continue"
            " with the transmission of CBEACON packet";
    //Type of packets
    packetType[UWUFETCH_NODE_PACKET_TYPE_DATA] = "DATA packet";
    packetType[UWUFETCH_NODE_PACKET_TYPE_PROBE] = "PROBE packet";
    packetType[UWUFETCH_NODE_PACKET_TYPE_POLL] = "POLL packet";
    packetType[UWUFETCH_NODE_PACKET_TYPE_BEACON] = "BEACON packet";
    packetType[UWUFETCH_NODE_PACKET_TYPE_TRIGGER] = "TRIGGER packet";
    packetType[UWUFETCH_NODE_PACKET_TYPE_RTS] = "RTS packet";
    packetType[UWUFETCH_NODE_PACKET_TYPE_CTS] = "CTS packet";
    //Possible status in which NODE can be found
    statusTimer[UWUFETCH_TIMER_STATUS_IDLE] = "NODE is in IDLE";
    statusTimer[UWUFETCH_TIMER_STATUS_RUNNING] = "NODE is RUNNING";
    statusTimer[UWUFETCH_TIMER_STATUS_FROZEN] = "NODE is FREEZING";
    statusTimer[UWUFETCH_TIMER_STATUS_EXPIRED] = "NODE timeout is EXPIRED";
} //end initInfo();

/*******************************************************************************
 *                          GENERAL METHODS                                    * 
 *******************************************************************************/

void uwUFetch_NODE::Phy2MacStartRx(const Packet *p) {

    if (isHeadNode()) {
        /****************************
         *   I'm the HEAD NODE        *
         ******************************/
        uwUFetch_NODE::Phy2MacStartRx_HN(p);

    } else {
        /****************************
         *   I'm the SENSOR NODE    *
         ****************************/

        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        int src_mac_addr = mach->macSA();

        if (cmh->ptype() == PT_BEACON_UFETCH) {
            //BEACON section
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacStartRx()_________________Start_to_rx_BEACON_pck_"
                    << "from_HN(" << src_mac_addr << ")." << std::endl;

            if (rxBEACONEnabled) {
                //Node is enabled to receive BEACON
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->NODE is starting to receive a "
                        << " BEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;

                rx_BEACON_start_time = NOW;

            } else {
                //Node is not enabled to receive BEACON
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->NODE is not enabled to receive a "
                        << "BEACON packet from the HN with MAC address: " << src_mac_addr << ", so wait"
                        << " the end of the reception of the packet and after DROP IT" << std::endl;
            }
        } else if (cmh->ptype() == PT_POLL_UFETCH) {
            //POLL section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->NODE is starting to receive a "
                    << " POLL packet from the HN with MAC address: " << src_mac_addr << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacStartRx()_________________Start_to_rx_POLL_pck"
                    << "_from HN(" << src_mac_addr << ")." << std::endl;

            rx_POLL_start_time = NOW;

        } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
            //CBEACON section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->NODE is starting to receive a "
                    << " CBEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacStartRx()_________________Start_to_rx_CBEACON_pck"
                    << "_from HN(" << src_mac_addr << ")." << std::endl;

            rx_CBEACON_start_time = NOW;

        } else if (cmh->ptype() == PT_PROBE_UFETCH) {
            //PROBE section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->NODE is starting to receive a "
                    << " PROBE packet from the NODE with MAC address: " << src_mac_addr << std::endl;

        } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
            //TRIGGER section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->NODE is starting to receive a "
                    << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << std::endl;

        } else {
            //DATA section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->NODE is starting to receive a "
                    << " DATA packet from the HN or NODE with MAC address: " << src_mac_addr << std::endl;
        }
    }
} //end Phy2MacStartRx();

void uwUFetch_NODE::Phy2MacEndRx(Packet *p) {

    if (isHeadNode()) {
        /*********************
         * I'm the HEAD NODE *
         *********************/
        uwUFetch_NODE::Phy2MacEndRx_HN(p);

    } else {
        /****************************
         *   I'm the SENSOR NODE     *
         *****************************/
        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        int src_mac_addr = mach->macSA();
        int dest_mac_addr = mach->macDA(); //Destination MAC address, the address to which the packet is addressed
        hdr_MPhy* ph = HDR_MPHY(p);
        double gen_time = ph->txtime;
        double received_time = ph->rxtime;
        double diff_time = received_time - gen_time;
        double distance = diff_time * v_speed_sound;

        //Control if the packet that the HN has received is really for him                           
        if ((dest_mac_addr == addr) || (dest_mac_addr == MAC_BROADCAST)) {
            //Packet is addressed to me NODE

            if (cmh->ptype() == PT_BEACON_UFETCH) {
                //BEACON section
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________Finished_to_rx_BEACON"
                        << "_pck_from_HN(" << src_mac_addr << ")" << std::endl;

                if (((cmh->error()) || (!rxBEACONEnabled))) {
                    if ((cmh->error()) && (rxBEACONEnabled)) {
                        //BEACON is corrupted by the channel
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->Node is enabled to rx BEACON packet, but the packet received"
                                << " is corrupted: DROP IT and return in IDLE STATE." << std::endl;

                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________ENABLED_to_rx_BEACON_pck_from_HN(" << mach->macSA() << ")_"
                            << "but_pck_It_s_in_ERROR." << std::endl;

                        rx_BEACON_finish_time = NOW;

                        incrTotalBeaconPckRx_by_NODE();
                        incrTotalBeaconPckRx_corrupted_by_NODE();

                        rxBEACONEnabled = true;
                        n_BEACON_pck_rx_by_NODE = 0;

                        stateIdle_NODE();
                    } else if ((cmh->error()) && (!rxBEACONEnabled)) {
                        //Node is not enabled to receive BEACON and packet it's in error

                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                << "BEACON packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled"
                                << " to receive It, so DROP IT." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________NOT_ENABLED_to_rx_BEACON_pck_from_HN(" << mach->macSA() << ")"
                            << "_and_pck_It_s_in_ERROR." << std::endl;

                        /**Se il beacon è ricevuto dal nodo da cui sto aspettando il POLL, allora considera il BEACON
                         *  anche se é in errore e ritorna nello stato di IDLE resettando il POLL timeout, altrimenti droppa semplicemente il pacchetto*/
                        incrTotalBeaconPckRx_by_NODE();
                        incrTotalBeaconPckRx_corrupted_by_NODE();

                        if (mach->macSA() == mac_addr_HN_in_beacon) {
                            if (rxPOLLEnabled) {
                                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                        << "BEACON packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled to rx It, but SN it's waiting a POLL packet"
                                        << " from this HN, so return in IDLE state and wait a BEACON from another node." << std::endl;
                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________SN(" << addr << ")_is_waiting_a_POLL_from_HN(" << mach->macSA() << ")"
                                    << ":return_in_IDLE_STATE." << std::endl;

                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________RESET_POLL_timeout." << std::endl;

                                rxPOLLEnabled = false;
                                POLL_timer.force_cancel();

                                rxBEACONEnabled = true;
                                n_BEACON_pck_rx_by_NODE = 0;

                                n_BEACON_pck_rx_by_NODE = 0;
                                n_PROBE_pck_tx_by_NODE = 0;
                                n_POLL_pck_rx_by_NODE = 0;
                                n_DATA_pck_tx_by_NODE = 0;
                                n_CBEACON_pck_rx_by_NODE = 0;

                                rxBEACONEnabled = true;
                                txPROBEEnabled = false;
                                rxPOLLEnabled = false;
                                txDATAEnabled = false;
                                rxCBEACONEnabled = false;

                                dataAlreadyTransmitted = false;
                                stateIdle_NODE();
                            } else if (rxCBEACONEnabled) {
                                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                        << "BEACON packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled to rx It, but SN it's waiting a CBEACON packet"
                                        << " from this HN, so return in IDLE state and wait a BEACON from another node." << std::endl;

                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________SN(" << addr << ")_is_waiting_a_CBEACON_pck_from_HN(" << mach->macSA() << ")_"
                                    << ":return_in_IDLE_STATE." << std::endl;

                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________RESET_CBEACON_timeout." << std::endl;

                                rxPOLLEnabled = false;
                                rxCBEACONEnabled = false;
                                POLL_timer.force_cancel();
                                CBEACON_timer.force_cancel();

                                rxBEACONEnabled = true;
                                n_BEACON_pck_rx_by_NODE = 0;

                                n_BEACON_pck_rx_by_NODE = 0;
                                n_PROBE_pck_tx_by_NODE = 0;
                                n_POLL_pck_rx_by_NODE = 0;
                                n_DATA_pck_tx_by_NODE = 0;
                                n_CBEACON_pck_rx_by_NODE = 0;

                                rxBEACONEnabled = true;
                                txPROBEEnabled = false;
                                rxPOLLEnabled = false;
                                txDATAEnabled = false;
                                rxCBEACONEnabled = false;

                                dataAlreadyTransmitted = false;
                                stateIdle_NODE();
                            }
                        }
                        drop(p, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);
                    } else if (!(cmh->error()) && (!rxBEACONEnabled)) {
                        //Node is not enabled to receive BEACON and Beacon it's not in error
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                << "BEACON packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled"
                                << " to receive It, so DROP IT." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________NOT_ENABLED_to_rx_BEACON_pck_from_HN(" << mach->macSA() << ")_"
                            << "_and_BEACON_It_s_CORRECT:IGNORE_IT." << std::endl;

                        incrTotalBeaconPckRx_by_NODE();

                        if (mach->macSA() == mac_addr_HN_in_beacon) {
                            if (rxPOLLEnabled) {
                                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                        << "BEACON packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled to rx It, but SN it's waiting a POLL packet"
                                        << " from this HN, so return in IDLE state and wait a BEACON from another node." << std::endl;

                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________SN(" << addr << ")_is_waiting_a_POLL_pck_from_HN(" << mach->macSA() << ")_"
                                    << ":return_in_IDLE_STATE." << std::endl;
                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________RESET_POLL_timeout." << std::endl;

                                rxPOLLEnabled = false;
                                POLL_timer.force_cancel();

                                rxBEACONEnabled = true;
                                n_BEACON_pck_rx_by_NODE = 0;

                                n_BEACON_pck_rx_by_NODE = 0;
                                n_PROBE_pck_tx_by_NODE = 0;
                                n_POLL_pck_rx_by_NODE = 0;
                                n_DATA_pck_tx_by_NODE = 0;
                                n_CBEACON_pck_rx_by_NODE = 0;

                                rxBEACONEnabled = true;
                                txPROBEEnabled = false;
                                rxPOLLEnabled = false;
                                txDATAEnabled = false;
                                rxCBEACONEnabled = false;

                                dataAlreadyTransmitted = false;
                                stateIdle_NODE();
                            } else if (rxCBEACONEnabled) {

                                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                        << "BEACON packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled to rx It, but SN it's waiting a CBEACON packet"
                                        << " from this HN, so return in IDLE state and wait a BEACON from another node." << std::endl;
                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________SN(" << addr << ")_is_waiting_a_CBEACON_pck_from_HN(" << mach->macSA() << ")"
                                    << ":return in IDLE STATE." << std::endl;
                                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________RESET_CBEACON_timeout." << std::endl;

                                rxPOLLEnabled = false;
                                POLL_timer.force_cancel();

                                rxCBEACONEnabled = false;
                                CBEACON_timer.force_cancel();

                                rxBEACONEnabled = true;
                                n_BEACON_pck_rx_by_NODE = 0;

                                n_BEACON_pck_rx_by_NODE = 0;
                                n_PROBE_pck_tx_by_NODE = 0;
                                n_POLL_pck_rx_by_NODE = 0;
                                n_DATA_pck_tx_by_NODE = 0;
                                n_CBEACON_pck_rx_by_NODE = 0;

                                rxBEACONEnabled = true;
                                txPROBEEnabled = false;
                                rxPOLLEnabled = false;
                                txDATAEnabled = false;
                                rxCBEACONEnabled = false;

                                dataAlreadyTransmitted = false;
                                stateIdle_NODE();
                            }
                        }
                        drop(p, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);
                    }
                } else {
                    //Node is enabled to receive BEACON
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                            << "BEACON packet from the HN with MAC address: " << src_mac_addr << " and it's enabled"
                            << " to receive It." << std::endl;
                    rxBEACONEnabled = false;

                    if (debug_) std::cout << NOW << " uwUFetch_NODE( " << addr << ") ::Phy2MacEndRx() ---->BEACON packet received by the NODE"
                            << " is CORRECT, so COMPUTE IT" << std::endl;

                    rx_BEACON_finish_time = NOW;

                    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________BEACON_rx_from_HN(" << mach->macSA() << ")_:It_s_CORRECT." << std::endl;

                    incrBeaconPckRx_by_NODE();
                    incrTotalBeaconPckRx_by_NODE();

                    curr_BEACON_NODE_pck_rx = p->copy();

                    Packet::free(p);
                    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_BEACON_RX);

                    BEACON_rx();
                }
            } else if (cmh->ptype() == PT_POLL_UFETCH) {
                //POLL section
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a"
                        << " POLL packet from the HN with MAC address: " << src_mac_addr << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________Finished_to_rx_POLL_pck"
                        << "_from_HN(" << src_mac_addr << ")." << std::endl;

                if ((src_mac_addr == mac_addr_HN_in_beacon)) {
                    if (cmh->error()) {
                        //POLL is corrupted
                        if (debug_) std::cout << NOW << " uwUFetch_NODE( " << addr << ") ::Phy2MacEndRx() ---->POLL packet received by the NODE"
                                << " is corrupted: DROP IT." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________POLL_pck_rx_from_HN(" << src_mac_addr << ")_It_s_in_ERROR." << std::endl;

                        rx_POLL_finish_time = NOW;

                        incrTotalPollPckRx_by_NODE();
                        incrTotalPollPckRx_corrupted_by_NODE(); //increment the number of packets in error

                        drop(p, 1, UWFETCH_NODE_DROP_REASON_ERROR); //drop the packet  
                        refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR);

                        n_PROBE_pck_tx_by_NODE = 0;
                        n_POLL_pck_rx_by_NODE = 0;
                        n_DATA_pck_tx_by_NODE = 0;

                        rxPOLLEnabled = false;
                        dataAlreadyTransmitted = false;

                    } else {
                        //POLL is not corrupted
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->POLL packet received by the NODE"
                                << " is CORRECT." << std::endl;

                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________POLL_pck_rx_from_HN(" << src_mac_addr << ")_It_s_CORRECT." << std::endl;

                        rx_POLL_finish_time = NOW;

                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->POLL timer is resetted." << std::endl;
                        POLL_timer.force_cancel();

                        incrPollPckRx_by_NODE();
                        incrTotalPollPckRx_by_NODE();

                        curr_POLL_NODE_pck_rx = p->copy();

                        Packet::free(p);
                        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_BEACON_RX);

                        POLL_rx();
                    }
                } else {
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has received a POLL packet"
                            << " from the HN from which it's not waiting a POLL, so DROP IT." << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________POLL_pck_rx_from_HN(" << src_mac_addr << ")_It_s_not_addressed_to_me:_DROP_IT." << std::endl;

                    drop(p, 1, UWFETCH_NODE_DROP_REASON_ERROR); //drop the packet  
                    refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR);
                }
            } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
                //CBEACON section
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a"
                        << " CBEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________NODE_has_finished_to_rx_CBEACON_packet"
                        << "_from HN(" << src_mac_addr << ")." << std::endl;

                mac_addr_HN_in_cbeacon = mach->macSA();

                if (mac_addr_HN_in_cbeacon == mac_addr_HN_in_beacon) {
                    //CBEACON is received from the HN from which I have received the BEACON
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->CBEACON rx from the correct HN" << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________CBEACON_rx_from_the_correct_HN(" << src_mac_addr << ")." << std::endl;

                    incrTotalCBeaconPckRx_by_NODE(); //increment the total number of CBEACON packets received.
                    CBEACON_timer.force_cancel();
                    if (cmh->error()) {
                        //The CBEACON is corrupted 
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->CBEACON received by the NODE"
                                << " is corrupted: DROP IT." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________CBEACON_rx_from_HN(" << mach->macSA() << ")_It_s_in_ERROR." << std::endl;

                        incrCBeaconPckRx_by_NODE();
                        incrTotalCBeaconPckRx_corrupted_by_NODE();

                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->CBEACON reset the timer" << std::endl;
                        CBEACON_timer.force_cancel();

                        if (getCBeaconPckRx_by_NODE() == num_cbeacon_node_rx) {
                            //Node has received the maximum number of CBEACON
                            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has received the maximum"
                                    << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                                    << " return in IDLE STATE." << std::endl;
                            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________MAX_number_of_CBEACON_received." << std::endl;

                            n_BEACON_pck_rx_by_NODE = 0;
                            n_PROBE_pck_tx_by_NODE = 0;
                            n_POLL_pck_rx_by_NODE = 0;
                            n_DATA_pck_tx_by_NODE = 0;
                            n_CBEACON_pck_rx_by_NODE = 0;

                            rxBEACONEnabled = true;
                            txPROBEEnabled = false;
                            rxPOLLEnabled = false;
                            txDATAEnabled = false;
                            rxCBEACONEnabled = false;
                            POLL_timer.force_cancel();

                            dataAlreadyTransmitted = false;
                            stateIdle_NODE();
                        } else {
                            //Node can receive another CBEACON packet
                            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has not received the maximum"
                                    << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                                    << " wait other CBEACON packets." << std::endl;
                            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________Other_CBEACON_can_be_rx." << std::endl;

                            n_PROBE_pck_tx_by_NODE = 0;
                            n_POLL_pck_rx_by_NODE = 0;
                            rxPOLLEnabled = false;
                            POLL_timer.force_cancel();
                            CBEACON_timer.force_cancel();

                            state_wait_CBEACON();
                        }
                    } else {
                        //The CBEACON is not corrupted
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->CBEACON packet received by the NODE"
                                << " is CORRECT." << std::endl;
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->CBEACON received is the number: "
                                << getCBeaconPckRx_by_NODE() + 1 << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________CBECON_rx_from_HN(" << mach->macSA() << ")_It_s_CORRECT." << std::endl;

                        curr_CBEACON_NODE_pck_rx = p->copy();

                        Packet::free(p);
                        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_BEACON_RX);

                        CBEACON_rx();
                    }
                } else {
                    //CBEACON is received from the HN from which I don't never received the BEACON
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->CBEACON packet received by the NODE"
                            << " is received from the HN from which I never received the BEACON: IGNORE IT" << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________CBEACON_rx_It_s_not_for_me_SENSOR_NODE." << std::endl;

                    incrTotalCBeaconPckRx_by_NODE(); //increment the number of packets in error
                }
            } else if (cmh->ptype() == PT_PROBE_UFETCH) {
                //PROBE section
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a"
                        << " PROBE packet from the NODE with MAC address: " << src_mac_addr << ". DROP IT" << std::endl;

                drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

            } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
                //TRIGGER section
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a"
                        << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << std::endl;

                drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);
            } else {
                //DATA section
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a"
                        << " DATA packet from the HN or NODE with MAC address: " << src_mac_addr << " .DROP IT" << std::endl;

                if (mach->macSA() == mac_addr_HN_in_beacon) {
                    if (rxPOLLEnabled) {
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                << "DATA packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled to rx It, but SN it's waiting a POLL packet"
                                << " from this HN, so return in IDLE state and wait a BEACON from another node." << std::endl;

                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________Received_DATA_from_HN_from_which_"
                                << "SN(" << addr << ")_is_waiting_a_POLL." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________RESET_POLL_timeout_and_return_in_IDLE_STATE." << std::endl;

                        rxPOLLEnabled = false;
                        POLL_timer.force_cancel();

                        rxBEACONEnabled = true;
                        n_BEACON_pck_rx_by_NODE = 0;

                        n_BEACON_pck_rx_by_NODE = 0;
                        n_PROBE_pck_tx_by_NODE = 0;
                        n_POLL_pck_rx_by_NODE = 0;
                        n_DATA_pck_tx_by_NODE = 0;
                        n_CBEACON_pck_rx_by_NODE = 0;

                        rxBEACONEnabled = true;
                        txPROBEEnabled = false;
                        rxPOLLEnabled = false;
                        txDATAEnabled = false;
                        rxCBEACONEnabled = false;

                        dataAlreadyTransmitted = false;
                        stateIdle_NODE();

                    } else if (rxCBEACONEnabled) {

                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has finished to receive a "
                                << "DATA packet from the HN with MAC address: " << src_mac_addr << " and it's not enabled to rx It, but SN it's waiting a CBEACON packet"
                                << " from this HN, so return in IDLE state and wait a BEACON from another node." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________Received_DATA_from_HN_from_which_"
                                << "SN(" << addr << ")_is_waiting_a_CBEACON." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________RESET_CBEACON_timeout_and_return_in_IDLE_STATE." << std::endl;

                        rxPOLLEnabled = false;
                        POLL_timer.force_cancel();

                        rxCBEACONEnabled = false;
                        CBEACON_timer.force_cancel();

                        rxBEACONEnabled = true;
                        n_BEACON_pck_rx_by_NODE = 0;

                        n_BEACON_pck_rx_by_NODE = 0;
                        n_PROBE_pck_tx_by_NODE = 0;
                        n_POLL_pck_rx_by_NODE = 0;
                        n_DATA_pck_tx_by_NODE = 0;
                        n_CBEACON_pck_rx_by_NODE = 0;

                        rxBEACONEnabled = true;
                        txPROBEEnabled = false;
                        rxPOLLEnabled = false;
                        txDATAEnabled = false;
                        rxCBEACONEnabled = false;

                        dataAlreadyTransmitted = false;
                        stateIdle_NODE();
                    }
                }
                drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);
            }
        } else {
            //Packet not addressed to the exactly sensor node
            if (cmh->ptype() == PT_TRIGGER_UFETCH) {

                drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

            } else if (cmh->ptype() == PT_PROBE_UFETCH) {

                drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

            } else if (cmh->ptype() == PT_POLL_UFETCH) {
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________Finished_to_rx_POLL"
                        << "_pck_from_HN(" << src_mac_addr << ")" << std::endl;
                if ((cmh->ptype() == PT_POLL_UFETCH) && (src_mac_addr == mac_addr_HN_in_beacon) && (getProbePckTx_by_NODE() >= 1)) {
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->NODE has received a POLL packet"
                            << " from the HN from which it waiting a POLL, but it's not my turn, so reset the POLL timer and"
                            << " restart to waiting a POLL." << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________SN(" << addr << ")_is_waiting_a_POLL_from"
                            << "_HN(" << src_mac_addr << ")_but_It_s_NOT_ITS_TURN." << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________RESET_and_RESTART_POLL_timeout." << std::endl;

                    //POLL_timer.force_cancel();
                    hdr_mac* mach = HDR_MAC(p);
                    hdr_POLL_UFETCH* pollh = HDR_POLL_UFETCH(p);

                    /**
                     * Save the information of POLL HEADER
                     */
                    num_pck_to_tx_by_NODE = pollh->num_DATA_pcks_MAX_rx(); //Number of DATA packets that the node will transmit to the HN  
                    double poll_waiting_time = num_pck_to_tx_by_NODE * (T_GUARD) + T_POLL;
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->The new waiting time is:"
                            << poll_waiting_time << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndRx()___________________POLL_waiting_time_is:_" << poll_waiting_time << "_[s]." << std::endl;

                    drop(p, 1, UWFETCH_NODE_DROP_REASON_WRONG_RECEIVER);

                    POLL_timer.schedule(poll_waiting_time);

                } else {
                    //Packet is not addressed to me NODE
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->The packet received is not for me NODE:"
                            << " DROP IT." << std::endl;

                    drop(p, 1, UWFETCH_NODE_DROP_REASON_WRONG_RECEIVER);
                    refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

                }
            } else {

                drop(p, 1, UWFETCH_NODE_DROP_REASON_WRONG_RECEIVER);
            }
        }

    }
} //end Phy2MacEndRx();

void uwUFetch_NODE::Mac2PhyStartTx(Packet *p) {

    if (isHeadNode()) {
        /*********************
         * I'm the HEAD NODE *
         *********************/
        uwUFetch_NODE::Mac2PhyStartTx_HN(p);
    } else {
        /****************************
         *    I'm the SENSOR NODE   *
         ****************************/
        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);

        if (cmh->ptype() == PT_PROBE_UFETCH) {
            //PROBE section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the NODE passes the PROBE"
                    << " packet to its physical layer." << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Mac2PhyStartTx()_________________Start_tx_PROBE_pck"
                    << "_to_the_HN(" << mac_addr_HN_in_beacon << ")." << std::endl;

            tx_PROBE_start_time = NOW;

            MMac::Mac2PhyStartTx(p);
        } else {
            //DATA section

            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the NODE passes the DATA"
                    << " packet to its physical layer." << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Mac2PhyStartTx()_________________Start_tx_DATA_pck_"
                    << "to_the_HN(" << mach->macDA() << ")." << std::endl;

            tx_DATA_start_time = NOW;

            Tdata_NODE_pck = Mac2PhyTxDuration(p);
            Tdata_NODE_pck = TIME_BETWEEN_2_TX_DATA_NODE_HN;

            MMac::Mac2PhyStartTx(p);
        }
    }
} //end Mac2PhyStartTx();

void uwUFetch_NODE::Phy2MacEndTx(const Packet *p) {
    if (isHeadNode()) {
        /*********************
         * I'm the HEAD NODE *
         *********************/
        uwUFetch_NODE::Phy2MacEndTx_HN(p);

    } else {
        /****************************
         *    I'm the SENSOR NODE   *
         ****************************/
        hdr_cmn* cmh = hdr_cmn::access(p);

        if (cmh->ptype() == PT_PROBE_UFETCH) {
            //PROBE section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of the NODE has finished"
                    << " to transmit a PROBE packet to the HN with MAC address: " << mac_addr_HN_in_beacon << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndTx()___________________Finished_to_tx_PROBE_pck"
                    << "_to_the_HN(" << mac_addr_HN_in_beacon << ")." << std::endl;

            tx_PROBE_finish_time = NOW;

            incrProbePckTx_by_NODE();
            incrTotalProbePckTx_by_NODE();

            txPROBEEnabled = false;

            state_wait_POLL();

        } else {
            //DATA section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of the NODE has finished"
                    << " to transmit a DATA packet to the HN with MAC address: " << mac_addr_HN_in_poll << std::endl;

            tx_DATA_finish_time = NOW;

            incrDataPckTx_by_NODE();
            incrTotalDataPckTx_by_NODE();

            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::Phy2MacEndTx()___________________Finished_to_tx_DATA_pck"
                    << "_to_the_HN(" << mac_addr_HN_in_beacon << ")." << std::endl;

            dataAlreadyTransmitted = true;
            rxBEACONEnabled = false;
            if (getDataPckTx_by_NODE() < (num_pck_to_tx_by_NODE)) {
                DATA_BEFORE_TX_timer.schedule(Tdata_NODE_pck);
            } else {
                state_DATA_NODE_finish_tx();
            }
        }
    }
} //end Phy2MacEndTx();

/******************************************************************************
 *                            SENSOR NODE                                     *      
 ******************************************************************************/
void uwUFetch_NODE::stateIdle_NODE() {
    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_NODE() ---->NODE is in IDLE STATE" << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::stateIdle_NODE()_________________NODE_is_IDLE_STATE." << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_IDLE);
    if (print_transitions) printStateInfo();

    rxBEACONEnabled = true;

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_NODE() ---->NODE is waiting a BEACON packet"
            << " from an HN." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::stateIdle_NODE()_________________....Waiting_BEACON_pck." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::stateIdle_NODE()_________________....Waiting_BEACON_pck." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::stateIdle_NODE()_________________....Waiting_BEACON_pck." << std::endl;

} //end stateIdle_NODE();

void uwUFetch_NODE::BEACON_rx() {

    refreshState(UWUFETCH_NODE_STATUS_BEACON_RECEIVE);
    if (print_transitions) printStateInfo();

    hdr_mac* mach = HDR_MAC(curr_BEACON_NODE_pck_rx);
    hdr_BEACON_UFETCH* beaconh = HDR_BEACON_UFETCH(curr_BEACON_NODE_pck_rx);

    /*
     * Save the header data of the beacon
     */
    T_min_bck_probe_node = (double) beaconh->t_min_bc() / 1000;
    T_max_bck_probe_node = (double) beaconh->t_max_bc() / 1000;
    num_cbeacon_node_rx = beaconh->num_Max_CBEACON_tx_by_HN(); //Number of CBEACON that the SENSOR NODE will be received from  the HN  from which it has 
    //received the BEACON 
    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::BEACON_rx() ---->NUMERO DI CBEACON: " << num_cbeacon_node_rx << std::endl;

    mac_addr_HN_in_beacon = mach->macSA(); //Mac address from which the node has received a BEACON

    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________PARAMETERS_of_BEACON_PACKET:" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________Source_address:_" << mach->macSA() << "." << std::endl;
    if (mach->macDA() == -1) {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________Destination_address:_BROADCAST." << std::endl;
    } else {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________Destination_address:_" << mach->macDA() << "." << std::endl;
    }
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________MIN_backoff_time_PROBE_pck:_" << T_min_bck_probe_node << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________MAX_backoff_time_PROBE_pck:_" << T_max_bck_probe_node << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________MAX_number_of_CBEACON_that_HN_will_tx:_" << num_cbeacon_node_rx << "." << std::endl;

    rxBEACONEnabled = false;

    bck_before_tx_probe = choiceBackOffTimer();
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BEACON_rx()______________________Backoff_choice_before_tx_PROBE:_" << bck_before_tx_probe << "[s]." << std::endl;

    BCK_timer_probe.schedule(bck_before_tx_probe);

} //end BEACON_rx();

void uwUFetch_NODE::BCKTOExpired() {
    if (isHeadNode()) {
        /*********************
         * I'm the HEAD NODE *
         *********************/
        BCKTOExpired_HN();

    } else {

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::BCKTOExpired() ---->Back-off timeout is expired:"
                << " so the PROBE packet now can be transmitted by the NODE to the HN." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::BCKTOExpired()___________________PROBE_Backoff_expired." << std::endl;
        state_PROBE_tx();
    }
} //end BCKTOExpired();

void uwUFetch_NODE::state_PROBE_tx() {

    if (!Q_data.empty()) {
        //QUEUE is not empty so transmit the PROBE packet
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_PROBE_tx() ---->NODE create a PROBE packet"
                << " that will then transmit at the HN with MAC address: " << mac_addr_HN_in_beacon << std::endl;

        refreshState(UWUFETCH_NODE_STATUS_TRANSMIT_PROBE);
        if (print_transitions) printStateInfo();

        Packet* p = Packet::alloc();
        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        hdr_PROBE_UFETCH* probeh = HDR_PROBE_UFETCH(p);

        cmh->ptype() = PT_PROBE_UFETCH;
        cmh->size() = sizeof (hdr_PROBE_UFETCH);

        mach->set(MF_CONTROL, addr, mac_addr_HN_in_beacon);
        mach->macSA() = addr;
        mach->macDA() = mac_addr_HN_in_beacon;

        //Filling the HEADER of the PROBE packet
        probeh->backoff_time_PROBE() = (int) (bck_before_tx_probe * 1000); //Back off value choose by the NODE before to transmit the PROBE packet
        probeh->n_DATA_pcks_Node_tx() = Q_data.size(); //Maximum quantity of DATA packets that the NODE would like to transmit at the HN

        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_PROBE_tx()_________________PARAMETER_of_PROBE_pck:" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_PROBE_tx()_________________Source_address:_" << mach->macSA() << "." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_PROBE_tx()_________________Destination_address:_" << mach->macDA() << "." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_PROBE_tx()_________________DATA_pck_would_be_tx:_" << probeh->n_DATA_pcks_Node_tx() << "[pck]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_PROBE_tx()_________________Backoff_time_choice:_" << bck_before_tx_probe << "[s]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_PROBE_tx()_________________Size_of_PROBE_pck:_" << cmh->size() << "[byte]." << std::endl;

        curr_PROBE_NODE_pck_tx = p->copy();

        /*hdr_uwmphy_modem* modemh = HDR_UWMPHY_MODEM(curr_PROBE_NODE_pck_tx);
        modemh->use_burst_data() = 0;*/

        Packet::free(p);

        txPROBEEnabled = true;

        PROBE_tx();

    } else {
        //QUEUE is empty, so the NODE doesn't transmit a PROBE packet to the HN.
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_PROBE_tx() ----> NODE has no DATA packet to be transmitted"
                << " to the HN, then ignore the BEACON received from the HN with MAC address: " << mac_addr_HN_in_beacon << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_PROBE_tx()_________________0_DATA_to_tx_so_NO_tx_PROBE_pck." << std::endl;

        if (getCBeaconPckRx_by_NODE() == num_cbeacon_node_rx) {
            //Node has received the maximum number of CBEACON
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_PROBE_tx() ---->NODE has received the maximum"
                    << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                    << " return in IDLE STATE." << std::endl;

            n_BEACON_pck_rx_by_NODE = 0;
            n_PROBE_pck_tx_by_NODE = 0;
            n_POLL_pck_rx_by_NODE = 0;
            n_DATA_pck_tx_by_NODE = 0;
            n_CBEACON_pck_rx_by_NODE = 0;

            rxBEACONEnabled = true;
            txPROBEEnabled = false;

            dataAlreadyTransmitted = false;

            stateIdle_NODE();

        } else {
            //Node can received another CBEACON
            txPROBEEnabled = false;
            rxCBEACONEnabled = true;

            n_PROBE_pck_tx_by_NODE = 0;
            n_POLL_pck_rx_by_NODE = 0;
            n_DATA_pck_tx_by_NODE = 0;

            state_wait_CBEACON();
        }
    }
} //end state_PROBE_tx();

void uwUFetch_NODE::PROBE_tx() {
    //NODE is enabled to transmit PROBE packet
    if (debug_) std::cout << NOW << " uwUFetch_NODE(" << addr << ") ::PROBE_tx() ---->NODE is transmitting a PROBE packet to the HN"
            << " with MAC address: " << mac_addr_HN_in_beacon << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_PROBE_TX);

    computeTxTime(UWUFETCH_NODE_PACKET_TYPE_PROBE);

    Mac2PhyStartTx(curr_PROBE_NODE_pck_tx);

} //end PROBE_tx();

void uwUFetch_NODE::state_wait_POLL() {

    computeTxTime(UWUFETCH_NODE_PACKET_TYPE_DATA);

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_POLL() ---->NODE is waiting for a POLL packet from HN"
            << " with MAC address: " << mac_addr_HN_in_beacon << ". The maximum waiting time is: " << T_POLL << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_POLL()________________Waiting_POLL_pck"
            << "_from HN(" << mac_addr_HN_in_beacon << ")." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_POLL()________________Waiting_time:_" << T_POLL << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_POLL()________________....Waiting_POLL." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_POLL()________________....Waiting_POLL." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_POLL()________________....Waiting_POLL." << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_WAIT_POLL_PACKET);
    if (print_transitions) printStateInfo();

    rxPOLLEnabled = true;

    POLL_timer.schedule(T_POLL);

} //end state_wait_POLL();

void uwUFetch_NODE::PollTOExpired() {

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_POLL_EXPIRED);

    if (getCBeaconPckRx_by_NODE() >= num_cbeacon_node_rx) {//|| (getCBeaconPckRx_by_NODE() == 0)) {
        //Node has received the maximum number of CBEACON
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::PollTOExpired() ---->NODE has received the maximum number of"
                << " CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                << " return in IDLE STATE." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::PollTOExpired()_______________POLL_timeout_EXPIRED:_Max_CBEACON_pck_rx." << std::endl;

        n_BEACON_pck_rx_by_NODE = 0;
        n_PROBE_pck_tx_by_NODE = 0;
        n_POLL_pck_rx_by_NODE = 0;
        n_DATA_pck_tx_by_NODE = 0;
        n_CBEACON_pck_rx_by_NODE = 0;

        rxBEACONEnabled = true;
        rxPOLLEnabled = false;
        txDATAEnabled = false;
        rxCBEACONEnabled = false;
        mac_addr_HN_in_beacon = addr;

        dataAlreadyTransmitted = false;

        stateIdle_NODE();

    } else {
        //Node wait the successive CBEACON
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::PollTOExpired()_______________POLL_timeout_EXPIRED:_another_CBEACON_can_be_rx." << std::endl;

        rxPOLLEnabled = false;

        n_PROBE_pck_tx_by_NODE = 0;
        n_POLL_pck_rx_by_NODE = 0;
        n_DATA_pck_tx_by_NODE = 0;

        state_wait_CBEACON();
    }
} //end PollTOExpired();

void uwUFetch_NODE::POLL_rx() {

    if (rxPOLLEnabled) {

        refreshState(UWUFETCH_NODE_STATUS_POLL_RECEIVE);
        if (print_transitions) printStateInfo();

        hdr_mac* mach = HDR_MAC(curr_POLL_NODE_pck_rx);
        hdr_POLL_UFETCH* pollh = HDR_POLL_UFETCH(curr_POLL_NODE_pck_rx);

        /**
         * Save the information of POLL HEADER
         */
        num_pck_to_tx_by_NODE = pollh->num_DATA_pcks_MAX_rx(); //Number of DATA packets that the node will transmit to the HN    

        mac_addr_HN_in_poll = mach->macSA(); //MAC address of the HN from which the node has received the POLL packet

        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::POLL_rx()________________________PARAMETERS_of_POLL_PACKET:" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::POLL_rx()________________________Source_address:_" << mach->macSA() << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::POLL_rx()________________________Destination_address:_" << mach->macDA() << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::POLL_rx()________________________DATA_pck_want_rx_from_SN:_" << num_pck_to_tx_by_NODE << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::POLL_rx()________________________MAC_address_of_POLL_rx_(" << mac_addr_HN_in_poll << ")." << std::endl;

        rxPOLLEnabled = false;

        state_DATA_NODE_first_tx();
    } else {
        //POLL reception is not enabled
        hdr_mac* mach = HDR_MAC(curr_POLL_NODE_pck_rx);

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::POLL_rx() ---->NODE is not enabled to receive the"
                << " POLL packet: DROP IT." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::POLL_rx()________________________Not_ENABLED_to_rx_POLL_pck_from_HN(" << mach->macSA() << ")." << std::endl;
        drop(curr_POLL_NODE_pck_rx, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);
    }
} //end POLL_rx();

void uwUFetch_NODE::state_DATA_NODE_first_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_NODE_first_tx() ---->NODE is starting transmission of the"
            << "  FIRST DATA packet to the HN with MAC address: " << mac_addr_HN_in_poll << std::endl;

    //Pick up the first element of the queue
    curr_DATA_NODE_pck_tx = (Q_data.front())->copy();
    //Remove the element from the queue that we have pick up the packet
    Q_data.pop();

    hdr_mac* mach = HDR_MAC(curr_DATA_NODE_pck_tx);
    hdr_cmn* cmh = hdr_cmn::access(curr_DATA_NODE_pck_tx);

    mach->set(MF_CONTROL, addr, mac_addr_HN_in_poll);
    mach->macSA() = addr;
    mach->macDA() = mac_addr_HN_in_poll;
    cmh->size() = sizeof (curr_DATA_NODE_pck_tx);

    hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_NODE_pck_tx);

    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_DATA_NODE_first_tx()_______DATA_pck_tx:_source(" << mach->macSA() << ")_"
        << "destination(" << mach->macDA() << ")_id_pck:" << cbrh->sn() << "_size:" << cmh->size() << "[byte]" << std::endl;

    txDATAEnabled = true;

    DATA_NODE_tx();
} //end state_DATA_NODE_first_tx();

void uwUFetch_NODE::DATA_NODE_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_NODE_tx() ---->NODE is transmitting a DATA packet to the HN"
            << " with MAC address: " << mac_addr_HN_in_poll << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_PCK_TX);

    Mac2PhyStartTx(curr_DATA_NODE_pck_tx);

} //end DATA_NODE_tx();

void uwUFetch_NODE::DataBeforeTxTOExpired() {
    if (isHeadNode()) {
        /*********************
         * I'm the HEAD NODE *
         *********************/
        DataBeforeTxTOExpired_HN();
    } else {

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DataBeforeTxTOExpired() ---->NODE can transmit the next"
                << " DATA packet." << std::endl;

        state_DATA_NODE_tx();
    }
} //end DataBeforeTxTOExpired();

void uwUFetch_NODE::state_DATA_NODE_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_NODE_tx() ---->NODE is starting transmission of the DATA"
            << " packet number: " << (getDataPckTx_by_NODE() + 1) << " to the HN with MAC"
        << " address: " << mac_addr_HN_in_poll << std::endl;

    //Pick up the first element of the queue
    curr_DATA_NODE_pck_tx = (Q_data.front())->copy();
    //Remove the element from the queue that we have pick up the packet
    Q_data.pop();

    hdr_mac* mach = HDR_MAC(curr_DATA_NODE_pck_tx);
    hdr_cmn* cmh = hdr_cmn::access(curr_DATA_NODE_pck_tx);

    mach->set(MF_CONTROL, addr, mac_addr_HN_in_poll);
    mach->macSA() = addr;
    mach->macDA() = mac_addr_HN_in_poll;
    cmh->size() = sizeof (curr_DATA_NODE_pck_tx);

    hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_NODE_pck_tx);

    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_DATA_NODE_tx()_____________DATA_pck_tx:_source(" << mach->macSA() << ")_"
        << "destination(" << mach->macDA() << ")_id_pck:" << cbrh->sn() << "_size:" << cmh->size() << "[byte]" << std::endl;

    txDATAEnabled = true;

    DATA_NODE_tx();
} //end state_DATA_tx();

void uwUFetch_NODE::state_DATA_NODE_finish_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_NODE_finish_tx() ---->NODE has transmitted all his DATA"
            << " packets to the HN with MAC address: " << mac_addr_HN_in_poll << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_DATA_NODE_finish_tx()______NODE_has_finished_to_tx_all_DATA_packets_to_the_HN(" << mac_addr_HN_in_poll << ")." << std::endl;

    dataAlreadyTransmitted = true;

    if ((getCBeaconPckRx_by_NODE() >= num_cbeacon_node_rx)) {
        //Node has received the maximum number of CBEACON
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_NODE_finish_tx() ---->NODE has received the maximum"
                << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_poll << ", so"
                << " return in IDLE STATE." << std::endl;

        n_BEACON_pck_rx_by_NODE = 0;
        n_PROBE_pck_tx_by_NODE = 0;
        n_POLL_pck_rx_by_NODE = 0;
        n_DATA_pck_tx_by_NODE = 0;
        n_CBEACON_pck_rx_by_NODE = 0;

        rxBEACONEnabled = true;
        txDATAEnabled = false;
        rxCBEACONEnabled = false;

        dataAlreadyTransmitted = false;

        stateIdle_NODE();

    } else {
        //Node wait the successive CBEACON
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_NODE_finish_tx() ---->NODE has not received the maximum"
                << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_poll << ", so"
                << " wait a CBEACON." << std::endl;

        n_PROBE_pck_tx_by_NODE = 0;
        n_POLL_pck_rx_by_NODE = 0;
        n_DATA_pck_tx_by_NODE = 0;

        txDATAEnabled = false;
        rxCBEACONEnabled = true;

        state_wait_CBEACON();
    }
} //end state_DATA_NODE_finish_tx();

void uwUFetch_NODE::state_wait_CBEACON() {

    double T_CBEACON = 500;

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_CBEACON() ---->NODE is scheduling CBEACON TIMER T= "
            << T_CBEACON << "[s]" << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_WAIT_CBEACON_PACKET);
    if (print_transitions) printStateInfo();

    rxBEACONEnabled = false;
    rxCBEACONEnabled = true;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_CBEACON()_____________Waiting_CBEACON_pck"
            << "_from_HN(" << mac_addr_HN_in_beacon << ")" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_CBEACON()_____________....Waiting_CBEACON_pck" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_CBEACON()_____________....Waiting_CBEACON_pck" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::state_wait_CBEACON()_____________....Waiting_CBEACON_pck" << std::endl;

    CBEACON_timer.schedule(T_CBEACON);
} //end state_wait_CBEACON();

void uwUFetch_NODE::CBeaconTOExpired() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBeaconTOExpired() ---->NODE can not receive other CBEACON"
            << " packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
            << " return in IDLE STATE." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBeaconTOExpired()_______NODE_CBEACON_timeout_expired." << std::endl;

    n_BEACON_pck_rx_by_NODE = 0;
    n_PROBE_pck_tx_by_NODE = 0;
    n_POLL_pck_rx_by_NODE = 0;
    n_DATA_pck_tx_by_NODE = 0;
    n_CBEACON_pck_rx_by_NODE = 0;

    rxBEACONEnabled = true;
    txPROBEEnabled = false;
    rxPOLLEnabled = false;
    txDATAEnabled = false;
    rxCBEACONEnabled = false;

    dataAlreadyTransmitted = false;
    stateIdle_NODE();
} //end CBeaconTOExpired();

void uwUFetch_NODE::CBEACON_rx() {

    if (rxCBEACONEnabled) {
        //Node is enabled to receive a CBEACON
        refreshState(UWUFETCH_NODE_STATUS_CBEACON_RECEIVE);
        if (print_transitions) printStateInfo();

        incrCBeaconPckRx_by_NODE();

        CBEACON_timer.force_cancel();

        hdr_mac* mach = HDR_MAC(curr_CBEACON_NODE_pck_rx);
        hdr_CBEACON_UFETCH* cbeaconh = HDR_CBEACON_UFETCH(curr_CBEACON_NODE_pck_rx);

        /*
         * Save the header data of the cbeacon
         */
        T_min_bck_probe_node = (double) cbeaconh->t_min_bc() / 1000;
        T_max_bck_probe_node = (double) cbeaconh->t_max_bc() / 1000;
        num_cbeacon_at_now_HN_tx = cbeaconh->num_Max_CBEACON_tx_by_HN(); //Number of CBEACON that the HN has transmitted at this time

        mac_addr_HN_in_cbeacon = mach->macSA(); //Mac address from which the node has received a BEACON

        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________PARAMETERS_of_CBEACON_pck:" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________Source_address:_" << mach->macSA() << ". " << std::endl;
        if (mach->macDA() == -1) {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________Destination_address:_BROADCAST." << std::endl;
        } else {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________Destination address:_" << mach->macDA() << ". " << std::endl;
        }
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________MIN_backoff_time_PROBE_pck:_" << T_min_bck_probe_node << "[s]. " << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________MAX_backoff_time_PROBE_pck:_" << T_max_bck_probe_node << "[s]." << std::endl;

        if ((dataAlreadyTransmitted == false)) {
            //NODE in the past cycle has not transmitted his data packet, so transmit a probe
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________NODE_has_not_yet_tx_DATA_in_this_cycle"
                    "_so_start_the_tx_of_PROBE." << std::endl;

            rxCBEACONEnabled = false; //Indicates if or not the NODE is enabled to receive a CBEACON packet to the HN

            CBEACON_timer.force_cancel();

            bck_before_tx_probe = choiceBackOffTimer();
            BCK_timer_probe.schedule(bck_before_tx_probe);

        } else {
            //Node has already transmitted his data packet or the CBEACON received is receive from an HN from which the NODE has never
            //receive a BEACON packet
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_rx() ---->NODE has received a CBEACON and it"
                    << " has already transmit his DATA packets in the previous cycle: DROP IT." << std::endl;

            drop(curr_CBEACON_NODE_pck_rx, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);

            if (getCBeaconPckRx_by_NODE() == num_cbeacon_node_rx) {
                //Node has received the maximum number of CBEACON
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_rx() ---->NODE has received the maximum"
                        << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                        << " return in IDLE STATE." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________NODE_has_already_tx_DATA_in_this_cycle_"
                        << "_and_maximum_number_of_CBEACON_rx." << std::endl;

                n_BEACON_pck_rx_by_NODE = 0;
                n_PROBE_pck_tx_by_NODE = 0;
                n_POLL_pck_rx_by_NODE = 0;
                n_DATA_pck_tx_by_NODE = 0;
                n_CBEACON_pck_rx_by_NODE = 0;

                rxBEACONEnabled = true;
                txPROBEEnabled = false;
                rxPOLLEnabled = false;
                txDATAEnabled = false;
                rxCBEACONEnabled = false;

                dataAlreadyTransmitted = false;
                stateIdle_NODE();

            } else {
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_rx() ---->NODE has not received the maximum"
                        << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                        << " wait other CBEACON packets." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________NODE_has_already_tx_DATA_in_this_cycle_"
                        << "_wait_another_CBEACON." << std::endl;

                n_PROBE_pck_tx_by_NODE = 0;
                n_POLL_pck_rx_by_NODE = 0;

                state_wait_CBEACON();
            }
        }
    } else {
        //Node is not enable to receive a CBEACON packet
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_r() ---->NODE is not enabled to receive"
                << " the CBEACON packet: DROP IT." << std::endl;
        incrCBeaconPckRx_by_NODE();

        CBEACON_timer.force_cancel();
        rxPOLLEnabled = false;
        POLL_timer.force_cancel();

        hdr_mac* mach = HDR_MAC(curr_CBEACON_NODE_pck_rx);
        hdr_CBEACON_UFETCH* cbeaconh = HDR_CBEACON_UFETCH(curr_CBEACON_NODE_pck_rx);

        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________SN_is_waiting_a_POLL_from_HN(" << mach->macSA() << ")"
            << "_but_it_has_rx_CBEACON_from_this_HN." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________RESET_POLL_timeout_and_analyzed_CBEACON_pck." << std::endl;

        /*
         * Save the header data of the cbeacon
         */
        T_min_bck_probe_node = (double) cbeaconh->t_min_bc() / 1000;
        T_max_bck_probe_node = (double) cbeaconh->t_max_bc() / 1000;
        num_cbeacon_at_now_HN_tx = cbeaconh->num_Max_CBEACON_tx_by_HN(); //Number of CBEACON that the HN has transmitted at this time

        mac_addr_HN_in_cbeacon = mach->macSA(); //Mac address from which the node has received a BEACON

        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________PARAMETERS_of_CBEACON_pck:" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________Source_address:_" << mach->macSA() << ". " << std::endl;
        if (mach->macDA() == -1) {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________Destination_address:_BROADCAST." << std::endl;
        } else {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________Destination_address:_" << mach->macDA() << ". " << std::endl;
        }
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________MIN_backoff_time_PROBE_pck:_" << T_min_bck_probe_node << "[s]. " << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________MAX_backoff_time_PROBE_pck:_" << T_max_bck_probe_node << "[s]." << std::endl;

        if ((dataAlreadyTransmitted == false)) {
            //NODE in the past cycle has not transmitted his data packet, so transmit a probe
            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________Not_yet_tx_DATA_in_this_cycle"
                    "_start_tx_of_another_PROBE_pck." << std::endl;

            rxCBEACONEnabled = false; //Indicates if or not the NODE is enabled to receive a CBEACON packet to the HN

            CBEACON_timer.force_cancel();

            bck_before_tx_probe = choiceBackOffTimer();
            BCK_timer_probe.schedule(bck_before_tx_probe);

        } else {
            //Node has already transmitted his data packet or the CBEACON received is receive from an HN from which the NODE has never
            //receive a BEACON packet
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_rx() ---->NODE has received a CBEACON and it"
                    << " has already transmit his DATA packets in the previous cycle: DROP IT." << std::endl;

            drop(curr_CBEACON_NODE_pck_rx, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);

            if (getCBeaconPckRx_by_NODE() == num_cbeacon_node_rx) {
                //Node has received the maximum number of CBEACON
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_rx() ---->NODE has received the maximum"
                        << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                        << " return in IDLE STATE." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________DATA_pck_already_tx_in_this_cycle_"
                        << "and_maximum_number_of_CBEACON_pck_rx." << std::endl;

                n_BEACON_pck_rx_by_NODE = 0;
                n_PROBE_pck_tx_by_NODE = 0;
                n_POLL_pck_rx_by_NODE = 0;
                n_DATA_pck_tx_by_NODE = 0;
                n_CBEACON_pck_rx_by_NODE = 0;

                rxBEACONEnabled = true;
                txPROBEEnabled = false;
                rxPOLLEnabled = false;
                txDATAEnabled = false;
                rxCBEACONEnabled = false;

                dataAlreadyTransmitted = false;
                stateIdle_NODE();

            } else {
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_rx() ---->NODE has not received the maximum"
                        << " number of CBEACON packet from the HN with MAC address: " << mac_addr_HN_in_beacon << ", so"
                        << " wait other CBEACON packets." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________DATA_pck_already_tx_in_this_cycle_"
                        << "and_wait_another_CBEACON_pck." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________....Waiting_CBEACON_pck." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________....Waiting_CBEACON_pck." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::CBEACON_rx()_____________________....Waiting_CBEACON_pck." << std::endl;

                n_PROBE_pck_tx_by_NODE = 0;
                n_POLL_pck_rx_by_NODE = 0;

                state_wait_CBEACON();
            }
        }
    }
} //end CBEACON_rx();

/*******************************************************************************
 *                          METODI AUSILIARI                                   * 
 ******************************************************************************/
bool uwUFetch_NODE::isHeadNode() {
    if (HEADNODE == 1) {
        return true;
    } else {
        return false;
    }
} //end isHeadNode();

double uwUFetch_NODE::choiceBackOffTimer() {
    //RNG::defaultrng()->set_seed(addr*5);
    srand(time(NULL) + addr);
    int random = rand() % (int) T_max_bck_probe_node + (int) T_min_bck_probe_node;

    if (debug_) std::cout << NOW << "uwUFetch_NODE(" << addr << ") getting Backoff time...Value = " << 0.883 * random << std::endl;
    return (0.833 * random);
} //end choiceBackoffTimer();

void uwUFetch_NODE::computeTxTime(UWUFETCH_NODE_PACKET_TYPE tp) {
    Packet* simple_pck = Packet::alloc();

    if (tp == UWUFETCH_NODE_PACKET_TYPE_PROBE) {
        hdr_cmn* cmh = HDR_CMN(simple_pck);
        hdr_mac* mach = HDR_MAC(simple_pck);
        cmh->size() = sizeof (hdr_PROBE_UFETCH);

        mach->macSA() = addr;
        mach->macDA() = mac_addr_HN_in_beacon;
        Tprobe = Mac2PhyTxDuration(simple_pck);
    } else {
        hdr_mac* mach = HDR_MAC(simple_pck);
        hdr_cmn* cmh = HDR_CMN(simple_pck);
        cmh->size() = MAX_PAYLOAD;

        mach->macSA() = addr;
        Tdata_NODE = Mac2PhyTxDuration(simple_pck);
    }
    Packet::free(simple_pck);
} //end computeTxTime();

void uwUFetch_NODE::recvFromUpperLayers(Packet* p) {
    if (isHeadNode()) {
        /**************************
         *  I'm THE HEAD NODE     *
         **************************/
        uwUFetch_NODE::recvFromUpperLayers_HN(p);
    } else {
        /**************************
         * I'm THE SENSOR NODE    *
         **************************/
        hdr_uwcbr* cbrh = HDR_UWCBR(p);

        if ((int) Q_data.size() < MAXIMUM_BUFFER_DATA_PCK_NODE) {
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::recvFromUpperLayers() ---->NODE is queuing a DATA packet"
                    << " generated by the APPLICATION layer" << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::recvFromUpperLayers()____________DATA_rx_from_upper_layer:id_pck_" << cbrh->sn() << "_STORE_IT." << std::endl;

            Q_data.push(p);
        } else {
            if (debug_) std::cout << NOW << " uwUFetch_NODE(" << addr << ") ::recvFromUpperLayers()--->NODE dropped DATA packet because"
                    << " its buffer is full" << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_SENSOR_NODE(" << addr << ")::recvFromUpperLayers()____________DATA_rx_from_upper_layer:id_pck_" << cbrh->sn() << "_MEMORY_IS_FULL:_DROP_IT." << std::endl;

            drop(p, 1, UWUFETCH_NODE_DROP_REASON_BUFFER_FULL);
        }
    }
} //end recvFromUpperLayers();

/*******************************************************************************
 *                              EXPIRE METHODS                                 *
 ******************************************************************************/
void uwUFetch_NODE::uwUFetch_BackOffTimer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << "uwUFetch_NODE (" << module->addr << ") ::uwUFetch_BackOffTimer::expire() ---->BACK-OFF timeout expired." << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->BCKTOExpired();
} //end uwUFetch_BackOffTimer::expire();

void uwUFetch_NODE::uwUFetch_POLL_timer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << " uwUFetch_NODE (" << module->addr << ")::uwUFetch_POLL_timer::expire() ---->POLL timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->PollTOExpired();
} //end uwUFetch_POLL_timer::expire();

void uwUFetch_NODE::uwUFetch_DATA_BEFORE_TX_timer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << " uwUFetch_NODE (" << module->addr << ") ::uwUFetch_DATA_BEFORE_TX_timer::expire() ---->SINGLE DATA timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->DataBeforeTxTOExpired();
} //end uwUFetch_DATA_BEFORE_TX_timer::expire();

void uwUFetch_NODE::uwUFetch_CBeacon_timer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << " uwUFetch_NODE (" << module->addr << ") ::uwUFetch_CBeacon_timer::expire() ---->CBEACON timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->CBeaconTOExpired();
} //end uwUFetch_CBeacon_timer::expire();

void uwUFetch_NODE::waitForUser() {
    std::string response;
    std::cout << "Press Enter to continue";
    std::getline(std::cin, response);
} //end waitForUser();

void uwUFetch_NODE::printStateInfo(double delay) {
    fout << NOW << " uwUFetch_NODE (" << addr << ") ::printStateInfo() " << "from " << statusInfo[prev_state]
            << " to " << statusInfo[curr_state] << ". Reason: " << statusChange[last_reason] << std::endl;
} //end printStateInfo()

