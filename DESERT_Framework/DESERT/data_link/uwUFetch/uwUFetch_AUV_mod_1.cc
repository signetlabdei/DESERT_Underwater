//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwUFetch_AUV_mod_1.cc
 * @author Loris Brolo
 * @version 1.0.0
 * 
 * @brief Class that provide the implementation of the AUV entity of uwUFetch protocol
 */

#include "mac.h"
#include "mmac.h"
#include "uwUFetch_AUV.h"
#include "uwUFetch_cmn_hdr.h"
#include "uwcbr-module.h"
#include <sstream>
#include <time.h>

/**
 * Class that represents the binding with the TCL configuration script 
 */
static class uwUFetchAUVModuleClass : public TclClass {
public:

    /**
     * Constructor of the class uwUFetchAUVModuleClass
     */
    uwUFetchAUVModuleClass() : TclClass("Module/UW/UFETCH/AUV") {
    }

    /**
     *  Creates the TCL object needed for the TCL language interpretation
     * 
     * @return Pointer to an TclObject
     */
    TclObject* create(int, const char*const*) {
        return (new uwUFetch_AUV());
    }
} class_module_uwufetchauv;

/**
 * Destructor of uwUFetch_AUV class
 */
uwUFetch_AUV::~uwUFetch_AUV() {
}

bool uwUFetch_AUV::initialized = false;

std::map< uwUFetch_AUV::UWUFETCH_AUV_STATUS, std::string > uwUFetch_AUV::statusInfo;
std::map< uwUFetch_AUV::UWUFETCH_AUV_STATUS_CHANGE, std::string > uwUFetch_AUV::statusChange;
std::map< uwUFetch_AUV::UWUFETCH_AUV_PACKET_TYPE, std::string > uwUFetch_AUV::packetType;
std::map< uwUFetch_AUV::UWUFETCH_TIMER_STATUS, std::string > uwUFetch_AUV::statusTimer;

/**
 * Constructor of the class uwUFetch_AUV
 */
uwUFetch_AUV::uwUFetch_AUV()
:
// TIMERS
Trigger_timer(this),
RTS_timer(this),
DATA_timer(this),
DATA_timer_first_pck(this),
// GLOBAL VARIABLES THAT SHOULD BE NEVER RESET
MAX_PAYLOAD(0),
NUM_MAX_DATA_AUV_WANT_RX(0),
T_START_PROC_TRIGGER(0),
N_RUN(0),
HEAD_NODE_1(0),
HEAD_NODE_2(0),
HEAD_NODE_3(0),
HEAD_NODE_4(0),
tx_TRIGGER_start_time(0),
tx_TRIGGER_finish_time(0),
rx_RTS_start_time(0),
rx_RTS_finish_time(0),
tx_CTS_start_time(0),
tx_CTS_finish_time(0),
rx_DATA_start_time(0),
rx_DATA_finish_time(0),
n_tot_TRIGGER_tx_by_AUV(0),
n_tot_RTS_rx_by_AUV(0),
n_tot_RTS_rx_corr_by_AUV(0),
n_tot_CTS_tx_by_AUV(0),
n_tot_DATA_rx_by_AUV(0),
n_tot_DATA_rx_corr_by_AUV(0),
number_data_pck_AUV_rx_exact(0),
mac_addr_HN_ctsed(0),
data_timeout(0),
T_tx_TRIGGER(0),
Tdata(0),
RTT(0),
mac_addr_HN_in_data(0),
hn_trigg(0),
index(0),
mac_addr_hn_triggered(0),
num_pck_hn_1(0),
num_pck_hn_2(0),
num_pck_hn_3(0),
num_pck_hn_4(0),
// VARIABLES THAT ENABLES OR NOT AN OPERATION
txTRIGGEREnabled(false),
rxRTSEnabled(false),
txCTSEnabled(false),
rxDATAEnabled(false),
// VARIABLES THAT SHOULD BE RESETED WHEN A CYCLE IS ENDED
n_TRIGGER_tx_by_AUV(0),
n_RTS_rx_by_AUV(0),
n_CTS_tx_by_AUV(0),
n_DATA_rx_by_AUV(0),
// PACKETS CREATED
curr_TRIGGER_pck_tx(NULL),
curr_RTS_pck_rx(NULL),
curr_CTS_pck_tx(NULL),
curr_DATA_pck_rx(NULL) {
    mac2phy_delay_ = 1e-19;
    bind("T_min_RTS_", (double*) & T_MIN_RTS);
    bind("T_max_RTS_", (double*) & T_MAX_RTS);
    bind("T_guard_", (double*) & T_GUARD);
    bind("t_RTS_", (double*) & T_RTS);
    bind("MAX_PAYLOAD", (double*) & MAX_PAYLOAD);
    bind("num_max_DATA_AUV_want_receive_", (int*) & NUM_MAX_DATA_AUV_WANT_RX);
    bind("TIME_BEFORE_TX_TRIGGER_PCK_", (double*) & T_START_PROC_TRIGGER);
    bind("MY_DEBUG_", (int*) & debugMio_);
    bind("NUMBER_OF_RUN_", (int*) & N_RUN);
    bind("HEAD_NODE_1_", (int*) & HEAD_NODE_1);
    bind("HEAD_NODE_2_", (int*) & HEAD_NODE_2);
    bind("HEAD_NODE_3_", (int*) & HEAD_NODE_3);
    bind("HEAD_NODE_4_", (int*) & HEAD_NODE_4);
    bind("MODE_COMM_", (int*) & mode_comm_hn_auv);
    bind("NUM_HN_NETWORK_", (int*) & NUM_HN_NET);

} //end uwUFetch_AUV()

int uwUFetch_AUV::command(int argc, const char*const*argv) {
    Tcl& tcl = Tcl::instance();

    if (argc == 2) {
        if (strcasecmp(argv[1], "initialize") == 0) {
            if (initialized == false) initInfo();
            if (debug_) fout.open("/tmp/uwUFetch_AUV_stateTransitions.txt", std::ios_base::app);

            if (debugMio_) {
                std::stringstream file_logging;
                file_logging << "uwUFetch_AUV_" << addr << "_N_RUN_" << N_RUN << ".out";
                out_file_logging.open(file_logging.str().c_str(), std::ios_base::app);
            }

            return TCL_OK;
        } else if (strcasecmp(argv[1], "printTransitions") == 0) {
            print_transitions = true;

            return TCL_OK;
        } else if (strcasecmp(argv[1], "getTRIGGERtxByAUV") == 0) {
            tcl.resultf("%d", getTotalTrigger_Tx_by_AUV());

            return TCL_OK;
        } else if (strcasecmp(argv[1], "getRTSrxByAUV") == 0) {
            tcl.resultf("%d", getTotalRts_Rx_by_AUV());

            return TCL_OK;
        } else if (strcasecmp(argv[1], "getRTSCorruptedRxByAUV") == 0) {
            tcl.resultf("%d", getTotalRts_Rx_corrupted_by_AUV());

            return TCL_OK;
        } else if (strcasecmp(argv[1], "getCTStxByAUV") == 0) {
            tcl.resultf("%d", getTotalCts_Tx_by_AUV());

            return TCL_OK;
        } else if (strcasecmp(argv[1], "getDataRxByAUV") == 0) {
            tcl.resultf("%d", getTotal_Data_Rx_by_AUV());

            return TCL_OK;
        } else if (strcasecmp(argv[1], "getDataCorruptedRxByAUV") == 0) {
            tcl.resultf("%d", getTotal_Data_Rx_corrupted_by_AUV());

            return TCL_OK;
        } else if (strcasecmp(argv[1], "AUVNodeStart") == 0) {
            stateIdle_AUV();
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "setMacAddr") == 0) {
            addr = atoi(argv[2]);
            if (debug_) std::cout << "UWFETCH_AUV MAC address is:" << addr << std::endl;

            return TCL_OK;
        }
    }

    return MMac::command(argc, argv);
}//end command

int uwUFetch_AUV::crLayCommand(ClMessage* m) {
    switch (m->type()) {
        default:
            return MMac::crLayCommand(m);
    }
} //end crLCommand()

/******************************************************************************
 *                           TX/RX METHODS                                    *
 ******************************************************************************/
void uwUFetch_AUV::Mac2PhyStartTx(Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);

    if (cmh->ptype() == PT_TRIGGER_UFETCH) {
        //TRIGGER section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the AUV passes the TRIGGER"
                << " packet to his physical layer." << std::endl;

        if (mach->macDA() == -1) {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_TRIGGER_pck_in_BROADCAST." << std::endl;
        } else {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_TRIGGER_pck_to_HN("
                    << mach->macDA() << ")." << std::endl;
        }
        //Compute the time required for transmission of the TRIGGER pck.
        computeTxTime(UWUFETCH_AUV_PACKET_TYPE_TRIGGER);

        //   if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Mac2PhyStartTx()______________________Time required for the tx: " << T_tx_TRIGGER << "[s]" << std::endl;

        tx_TRIGGER_start_time = NOW;

        MMac::Mac2PhyStartTx(p);
    } else if (cmh->ptype() == PT_CTS_UFETCH) {
        //CTS section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the AUV passes the CTS"
                << " packet to his physical layer." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_CTS_pck_to_HN("
                << mach->macDA() << ")." << std::endl;

        tx_CTS_start_time = NOW;

        MMac::Mac2PhyStartTx(p);
    }
} //end Mac2PhyStartTx()

void uwUFetch_AUV::Phy2MacEndTx(const Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);

    if (cmh->ptype() == PT_TRIGGER_UFETCH) {

        if (typeCommunication()) {
            //Communication between HN & AUV without RTS and CTS packets
            uwUFetch_AUV::Phy2MacEndTx_without(p);

        } else {
            //Communication between HN & AUV with RTS and CTS packets
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of AUV has finished"
                    << " to transmit a TRIGGER packet to the AUVs in broadcast, now the AUV is waiting for"
                    << " the reception of RTS packets from the HNs." << std::endl;

            if (mach->macDA() == -1) {
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndTx()________________________End_tx_of_TRIGGER_pck_in_BROADCAST." << std::endl;
            } else {
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndTx()________________________End_tx_of_TRIGGER_pck_to_the_HN("
                        << mach->macDA() << ")." << std::endl;
            }
            tx_TRIGGER_finish_time = NOW;

            incrTrigger_Tx_by_AUV();
            incrTotalTrigger_Tx_by_AUV();

            txTRIGGEREnabled = false;

            refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_TX_WAIT_RTS);
            state_wait_RTS();
        }

    } else if (cmh->ptype() == PT_CTS_UFETCH) {
        //CTS section 

        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of the AUV has finished"
                << " to transmit a CTS packet to the AUV with MAC address: " << mac_addr_HN_ctsed << "so wait"
                << " the DATA packets from the HN and disable the reception of other CTS" << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndTx()________________________End_tx_of_CTS_pck_to_the_HN("
                << mach->macDA() << ")." << std::endl;

        tx_CTS_finish_time = NOW;

        incrCts_Tx_by_AUV();
        incrTotalCts_Tx_by_AUV();

        txCTSEnabled = false;

        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_CTS_TX_WAIT_DATA);
        state_wait_first_DATA();

    }
} //end Phy2MacEndTx()

void uwUFetch_AUV::Phy2MacStartRx(const Packet *p) {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_AUV::Phy2MacStartRx_without(p);

    } else {
        //Communication between HN & AUV with RTS and CTS packets
        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        int src_mac_addr = mach->macSA();

        if (cmh->ptype() == PT_BEACON_UFETCH) {
            //BEACON section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                    << " BEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;
            // if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start to rx BEACON pck"
            //         << " from HN(" << src_mac_addr << ")." << std::endl;

        } else if (cmh->ptype() == PT_POLL_UFETCH) {
            //POLL section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                    << " POLL packet from the HN with MAC address: " << src_mac_addr << std::endl;
            // if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start to rx POLL pck"
            //         << " from HN(" << src_mac_addr << ")." << std::endl;

        } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
            //CBEACON section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                    << " CBEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;
            // if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start to rx CBEACON pck"
            //          << " from HN(" << src_mac_addr << ")." << std::endl;

        } else if (cmh->ptype() == PT_PROBE_UFETCH) {
            //PROBE section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                    << " PROBE packet from the NODE with MAC address: " << src_mac_addr << std::endl;
            //  if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start to rx PROBE pck"
            //          << " from NODE(" << src_mac_addr << ")." << std::endl;

        } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
            //TRIGGER section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                    << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << std::endl;
            //  if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start to rx TRIGGER pck"
            //          << " from AUV(" << src_mac_addr << ")." << std::endl;

        } else if (cmh->ptype() == PT_RTS_UFETCH) {
            //RTS section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                    << " RTS packet from the HN with MAC address: " << src_mac_addr << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start_to_rx_RTS_pck"
                    << "_from_HN(" << src_mac_addr << ")." << std::endl;

            rx_RTS_start_time = NOW;

        } else {
            //DATA section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                    << " DATA packet from the HN with MAC address: " << src_mac_addr << std::endl;
            // if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD" << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start_to_rx_DATA_pck"
                    << "_from_HN(" << src_mac_addr << ")." << std::endl;

            rx_DATA_start_time = NOW;
        }
    }
} //Phy2MacStartRx();

void uwUFetch_AUV::Phy2MacEndRx(Packet *p) {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_AUV::Phy2MacEndRx_without(p);

    } else {
        //Communication between HN & AUV with RTS and CTS packets
        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        int src_mac_addr = mach->macSA();
        int dest_mac_addr = mach->macDA(); //Destination MAC address, the address to which the packet is addressed

        //Control if the packet that the HN has received is really for him                           
        if ((dest_mac_addr == addr) || (dest_mac_addr == MAC_BROADCAST)) {

            if (cmh->ptype() == PT_BEACON_UFETCH) {
                //BEACON section
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                        << " BEACON packet from the HN with MAC address: " << src_mac_addr << " .IGNORE IT" << std::endl;

            } else if (cmh->ptype() == PT_POLL_UFETCH) {
                //POLL section
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                        << " POLL packet from the HN with MAC address: " << src_mac_addr << " .IGNORE IT" << std::endl;

            } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
                //CBEACON section
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                        << " CBEACON packet from the HN with MAC address: " << src_mac_addr << " .IGNORE IT." << std::endl;

            } else if (cmh->ptype() == PT_PROBE_UFETCH) {
                //PROBE section
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                        << " PROBE packet from the NODE with MAC address: " << src_mac_addr << " .IGNORE IT." << std::endl;

            } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
                //TRIGGER section
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                        << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << " .IGNORE IT." << std::endl;

            } else if (cmh->ptype() == PT_RTS_UFETCH) {
                //RTS section
                rx_RTS_finish_time = NOW;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_RTS_pck"
                        << "_from_HN(" << src_mac_addr << ")." << std::endl;
                incrTotalRts_Rx_by_AUV();

                if ((cmh->error()) || (!rxRTSEnabled)) {
                    if (cmh->error()) {
                        //Packet it's in error
                        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->RTS packet received by the AUV"
                                << " is corrupted: DROP IT, and continue with the processing." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________RTS_pck_rx"
                                << "_from HN(" << src_mac_addr << ")_It_s_in_ERROR." << std::endl;

                        incrTotalRts_Rx_corrupted_by_AUV();

                        drop(p, 1, UWFETCH_AUV_DROP_REASON_ERROR); //drop the packet  

                    } else {
                        //AUV is not enabled to receive RTS
                        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                                << " RTS packet from the HN with MAC address: " << src_mac_addr << ", but it's not "
                                << " enabled to receive It, so DROP the packet and continue the processing." << std::endl;
                        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________RTS_pck_rx"
                                << "_from_HN(" << src_mac_addr << ")_NOT_ENABLED_to_rx_It." << std::endl;

                        drop(p, 1, UUFETCH_AUV_DROP_REASON_NOT_ENABLE);
                    }
                } else {
                    //AUV is enabled to receive RTS
                    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                            << " RTS packet from the HN with MAC address: " << src_mac_addr << "." << std::endl;

                    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________RTS_pck_rx"
                            << "_from_HN(" << src_mac_addr << ")_It_s_CORRECT." << std::endl;

                    if (getTrigger_Tx_by_AUV() == 1) {
                        //The RTS is received after a transmission of TRIGGER packet
                        incrRts_Rx_by_AUV();

                        curr_RTS_pck_rx = p->copy();

                        Packet::free(p);

                        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_RTS_FINISHED_TO_RX);
                        RTS_rx();

                    } else {
                        //The RTS is received without sending previous TRIGGER packets
                        incrRts_Rx_by_AUV();

                        curr_RTS_pck_rx = p->copy();

                        Packet::free(p);

                        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_RTS_FINISHED_TO_RX);
                        RTS_rx();

                    }
                }
            } else {
                //DATA section
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                        << " DATA packet from the HEAD NODE with MAC address: " << src_mac_addr << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_DATA_pck"
                        << "_from_HN(" << src_mac_addr << ")." << std::endl;

                rx_DATA_finish_time = NOW;

                incrTotal_Data_Rx_by_AUV();

                if ((cmh->error()) || (!rxDATAEnabled)) {
                    if (cmh->error()) {
                        //Packet it's in error
                        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->DATA packet received by the AUV"
                                << " is corrupted: DROP IT and continue with the actual processing." << std::endl;

                        incrData_Rx_by_AUV();
                        incrTotal_Data_Rx_corrupted_by_AUV();

                        curr_DATA_pck_rx = p->copy();

                        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_pck_rx);

                        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________DATA_pck_rx_by_AUV(" << addr << ")_from_HN(" << mach->macSA() << ")_id_pck:" << cbrh->sn() << ""
                            << "_It_s_in_ERROR." << std::endl;

                        drop(p, 1, UWFETCH_AUV_DROP_REASON_ERROR); //drop the packet  

                        another_DATA_received();
                    } else {
                        //AUV is not enabled to receive a DATA packet
                        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV is not enabled to receive the DATA"
                                << " packet from HN(" << mach->macSA() << "): DROP IT." << std::endl;

                        curr_DATA_pck_rx = p->copy();

                        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_pck_rx);

                        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________DATA_pck_rx_by_AUV(" << addr << ")_from_HN(" << mach->macSA() << ")_id_pck:" << cbrh->sn() << ""
                            << "_NOT_ENABLED_TO_RX_IT." << std::endl;

                        drop(p, 1, UUFETCH_AUV_DROP_REASON_NOT_ENABLE);
                    }
                } else {
                    //Packet it's not in error
                    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->Physical layer of AUV with MAC"
                            << " address: " << dest_mac_addr << " has finished to receive a DATA packet"
                            << " number: " << (getData_Rx_by_AUV() + 1) << " from the HEAD NODE with MAC address: " << src_mac_addr << std::endl;

                    incrData_Rx_by_AUV();

                    curr_DATA_pck_rx = p->copy();

                    hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_pck_rx);

                    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()________________________DATA_pck_rx_by_AUV(" << addr << ")_from_HN(" << mach->macSA() << ")_id_pck:" << cbrh->sn() << ""
                        << "_IT_S_CORRECT." << std::endl;

                    Packet::free(p);

                    refreshReason(UWUFETCH_AUV_STATUS_CHANGE_DATA_PCK_FINISHED_TO_RX);
                    DATA_rx();
                }
            }
        } else {
            //Packet is not addressed to me AUV
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->The packet received is not for me AUV:"
                    << " DROP IT" << std::endl;

            if (cmh->ptype() == PT_BEACON_UFETCH) {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            } else if (cmh->ptype() == PT_POLL_UFETCH) {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            } else if (cmh->ptype() == PT_PROBE_UFETCH) {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            } else if (cmh->ptype() == PT_RTS_UFETCH) {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            } else if (cmh->ptype() == PT_CTS_UFETCH) {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            } else {
                drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);
            }
        }
    }
} //end Phy2MacEndRx()

/******************************************************************************
 *                                      AUV METHODS                                                   *
 ******************************************************************************/
void uwUFetch_AUV::stateIdle_AUV() {

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::stateIdle_AUV() ---->AUV is in IDLE state." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::stateIdle_AUV()_______________________AUV_is_in_IDLE_STATE." << std::endl;

    txTRIGGEREnabled = false;

    refreshState(UWUFETCH_AUV_STATUS_IDLE);
    if (print_transitions) printStateInfo();

    if (last_reason == UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED) {
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::stateIdle_AUV() ---->AUV start immediately the transmission of TRIGGER packet"
                << " to a specific HN." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::stateIdle_AUV()_______________________Start_immediately_the_tx_of_TRIGGER_pck." << std::endl;

        txTRIGGEREnabled = true;
        state_TRIGGER_tx();

    } else {
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::stateIdle_AUV() ---->AUV before to transmit a TRIGGER packet"
                << " to a specific HNs waits an interval time equal to: " << T_START_PROC_TRIGGER << "[s]." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::stateIdle_AUV()_______________________Wait_an_interval_time_equal_to:_"
                << T_START_PROC_TRIGGER << "_before_to_tx_TRIGGER_pck." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::stateIdle_AUV()_______________________.......Waiting" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::stateIdle_AUV()_______________________.......Waiting" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::stateIdle_AUV()_______________________.......Waiting" << std::endl;

        Trigger_timer.schedule(T_START_PROC_TRIGGER);
    }
} //end stateIdle_AUV()

void uwUFetch_AUV::TriggerTOExpired() {
    //The timeout is expired, so now the AUV node can transmit a TRIGGER packet. Before to transmit a TRIGGER packet, AUV 
    //must be created It.
    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::TriggerTOExpired() --->AUV now can start the transmission of the"
            << " TRIGGER packet in broadcast to the HNs." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::TriggerTOExpired()____________________Start_the_communication_with_HNs." << std::endl;

    txTRIGGEREnabled = true;
    rxRTSEnabled = false;

    refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_TO_EXPIRED);
    state_TRIGGER_tx();

} //end TRIGGERTOExpired()

void uwUFetch_AUV::state_TRIGGER_tx() {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_AUV::state_TRIGGER_tx_without();

    } else {
        //Communication between HN & AUV with RTS and CTS packets
        refreshState(UWUFETCH_AUV_STATUS_TRANSMIT_TRIGGER);
        if (print_transitions) printStateInfo();

        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_TRIGGER_tx() ---->AUV is initializing the TRIGGER packet that"
                << " will then transmitted to the HNs in broadcast. Only after the end of transmission of TRIGGER"
                << " the AUV will enabled to receive a RTS packet." << std::endl;

        Packet* p = Packet::alloc(); //allocate the memory for the packet
        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        hdr_TRIGGER_UFETCH* triggerh = HDR_TRIGGER_UFETCH(p);

        cmh->ptype() = PT_TRIGGER_UFETCH; //assign the type of the packet

        mach->set(MF_CONTROL, addr, MAC_BROADCAST); //set the information of the packet (type,source address,destination address)
        mach->macSA() = addr; //set the source address as AUV MAC address
        mach->macDA() = MAC_BROADCAST; //set the destination address as BROADCAST address

        //Filling the header of TRIGGER packet e determine the size of the TRIGGER packet
        triggerh->t_min() = (int) (T_MIN_RTS * 1000); //minimum value for to choose the back-off time for tx RTS pck
        triggerh->t_max() = (int) (T_MAX_RTS * 1000); //maximum value for to choose the back-off time for tx RTS pck
        triggerh->max_pck_want_rx() = NUM_MAX_DATA_AUV_WANT_RX; //maximum number of DATA packets that the AUV want to receive from the HN
        cmh->size() = sizeof (hdr_TRIGGER_UFETCH); //set the size of TRIGGER header packet

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________PARAMETERS_of_TRIGGER_PACKET." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________Source_address:_" << mach->macSA() << std::endl;
        if (mach->macDA() == -1) {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________Destination_Address:_BROADCAST." << std::endl;
        } else {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________Destination_Address:_" << mach->macDA() << std::endl;
        }
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________MIN_backoff_time_DATA_pck:_" << T_MIN_RTS << "[s]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________MAX_backoff_time_DATA_pck:_" << T_MAX_RTS << "[s]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________MAX_DATA_pck_want_rx_from_HN:_" << triggerh->max_pck_want_rx() << "[pck]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()____________________Size_of_TRIGGER_pck:_" << cmh->size() << "[byte]." << std::endl;

        //Create a copy f the TRIGGER packet
        curr_TRIGGER_pck_tx = p->copy();

        //DE-allocate the memory occupied by TRIGGER packet
        Packet::free(p);

        //Recall the function that simulate the transmission of the trigger
        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_INITIALIZE_TX_TRIGGER);
        TRIGGER_tx();
    }

} //end state_TRIGGER_tx()

void uwUFetch_AUV::TRIGGER_tx() {
    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_AUV::Mac2PhyStartTx_without(curr_TRIGGER_pck_tx);

    } else {
        //Communication between HN & AUV with RTS and CTS packets
        Mac2PhyStartTx(curr_TRIGGER_pck_tx);
    }
} //end TRIGGER_tx()

void uwUFetch_AUV::state_wait_RTS() {
    if (T_RTS < 0) {
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_wait_RTS() ---->The waiting time for RTS packets is negative"
                << " so set the default value: 10[s]." << std::endl;
        T_RTS = 10;
    }

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_wait_RTS() ---->AUV is scheduling RTS TIMER T= "
            << T_RTS << "[s]. In this interval AUV waiting the RTS packets." << std::endl;

    refreshState(UWUFETCH_AUV_STATUS_WAIT_RTS_PACKET);
    if (print_transitions) printStateInfo();

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_RTS()______________________Waiting_RTS_pck." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_RTS()______________________Timeout_within_rx_RTS_pck:_" << T_RTS << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_RTS()______________________........Waiting_RTS." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_RTS()______________________........Waiting_RTS." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_RTS()______________________........Waiting_RTS." << std::endl;

    rxRTSEnabled = true;

    RTS_timer.schedule(T_RTS);

} //end state_wait_RTS();

void uwUFetch_AUV::RTS_rx() {

    hdr_mac* mach = HDR_MAC(curr_RTS_pck_rx);
    hdr_RTS_UFETCH* rtsh = HDR_RTS_UFETCH(curr_RTS_pck_rx);

    refreshState(UWUFETCH_AUV_STATUS_RECEIVE_RTS_PACKET);
    if (print_transitions) printStateInfo();
    /**
     *  Save the information of the HN that has sent the RTS packet correctly 
     *  to the AUV
     *
     */
    double bck_time_choice_rts_by_HN = (double) rtsh->backoff_time_RTS() / 1000;

    Q_rts_mac_HN.push(mach->macSA()); //store the MAC address of the HN that
    //has sent the RTS packet to the AUV
    Q_rts_n_pcks_HN_want_tx_AUV.push(rtsh->num_DATA_pcks()); //store the number of DATA packets that the HN want to tx to
    //the AUV
    Q_rts_backoff_time.push(bck_time_choice_rts_by_HN); //store the back-off time choice by the HN before to transmit
    //a RTS packet          

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RTS_rx()______________________________PARAMETERS_of_RTS_PACKET." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RTS_rx()______________________________Source_address:_" << mach->macSA() << "." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RTS_rx()______________________________Destination_address:_" << mach->macDA() << "." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RTS_rx()______________________________RTS_backoff_time_choice_by_HN:_" << bck_time_choice_rts_by_HN << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RTS_rx()______________________________Number_of_pck_HN_would_like_to_tx: " << rtsh->num_DATA_pcks() << "[pck]." << std::endl;

    Packet::free(curr_RTS_pck_rx);

    if (getTrigger_Tx_by_AUV() == 1) {
        //This mean that the AUV has received the RTS after the sending of trigger,
        //so AUV before proceeding with the transmission of CTS it must reset the RTS timer. 
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RTS_rx()______________________________RESET_RTS_timeout." << std::endl;
        RTS_timer.force_cancel();
        rxRTSEnabled = false;
        txCTSEnabled = true;

        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_RTS_FINSHED_TO_STORE);
        state_CTS_tx();
    } else {
        //This mean that the AUV has received the RTS without the sending of trigger,
        //so AUV before proceeding with the transmission of CTS it must reset the RTS timer and Trigger timer. 
        Trigger_timer.force_cancel();
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RTS_rx()______________________________RESET_RTS_timeout." << std::endl;
        RTS_timer.force_cancel();
        rxRTSEnabled = false;
        txCTSEnabled = true;

        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_RTS_FINSHED_TO_STORE);
        state_CTS_tx();
    }
} //end RTS_rx();

void uwUFetch_AUV::RtsTOExpired() {

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::RtsTOExpired() ---->RTS timeout is expired: other"
            << " RTS packets can not be received by the AUV." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::RtsTOExpired()________________________RTS_timeout_is_expired." << std::endl;

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::RtsTOExpired() ---->AUV has received 0 RTS packets within the interval"
            << " time pre-established, so return in IDLE STATE." << std::endl;

    n_TRIGGER_tx_by_AUV = 0;
    n_RTS_rx_by_AUV = 0;
    n_CTS_tx_by_AUV = 0;
    n_DATA_rx_by_AUV = 0;

    txTRIGGEREnabled = true;
    rxRTSEnabled = false;

    refreshReason(UWUFETCH_AUV_STATUS_CHANGE_RTS_TO_EXPIRED_0_RTS_RX);
    stateIdle_AUV(); //AUV return in IDLE state

} //end RtsTOExpired

void uwUFetch_AUV::state_CTS_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_CTS_tx() ---->AUV is creating a CTS packet that"
            << " will then transmit at the first node of the queue that contain the RTS'received." << std::endl;

    refreshState(UWUFETCH_AUV_STATUS_TRANSMIT_CTS_PACKET);
    if (print_transitions) printStateInfo();

    Packet* p = Packet::alloc();
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    hdr_CTS_UFETCH* ctsh = HDR_CTS_UFETCH(p);

    cmh->ptype() = PT_CTS_UFETCH;
    cmh->size() = sizeof (hdr_CTS_UFETCH);

    mach->set(MF_CONTROL, addr, Q_rts_mac_HN.front());
    mach->macSA() = addr;
    mach->macDA() = Q_rts_mac_HN.front();

    //Filling the HEADER of the CTS packet
    if (Q_rts_n_pcks_HN_want_tx_AUV.front() <= NUM_MAX_DATA_AUV_WANT_RX) {
        ctsh->num_DATA_pcks_MAX_rx() = Q_rts_n_pcks_HN_want_tx_AUV.front(); //Maximum number of DATA packets that the AUV want to 
        //receive from the HN that is being to cts
    } else {
        ctsh->num_DATA_pcks_MAX_rx() = NUM_MAX_DATA_AUV_WANT_RX; //Maximum number of DATA packets that the AUV want to 
        //receive from the HN that is being to cts 
    }

    number_data_pck_AUV_rx_exact = ctsh->num_DATA_pcks_MAX_rx();
    ctsh-> mac_addr_HN_ctsed() = Q_rts_mac_HN.front(); //Mac address of the HN that the AUV is being to cts
    mac_addr_HN_ctsed = Q_rts_mac_HN.front(); //Store the mac address of the HN that the AUV is being to cts

    curr_CTS_pck_tx = p->copy();

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_CTS_tx()________________________PARAMETERS_OF_CTS_PCK." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_CTS_tx()________________________Source_address:_" << mach->macSA() << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_CTS_tx()________________________Destination_address:_" << mach->macDA() << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_CTS_tx()________________________DATA_pck_want_rx_from_HN:_" << number_data_pck_AUV_rx_exact << "[pck]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_CTS_tx()________________________Size_of_CTS_pck:_" << cmh->size() << "[byte]." << std::endl;

    Packet::free(p); //de-allocate the memory

    txCTSEnabled = true;

    refreshReason(UWUFETCH_AUV_STATUS_CHANGE_CTS_INITIALIZED_TX_CTS);
    CTS_tx();
} //end state_CTS_tx();

void uwUFetch_AUV::CTS_tx() {

    Mac2PhyStartTx(curr_CTS_pck_tx);

} //end state_CTS_tx();

void uwUFetch_AUV::state_wait_first_DATA() {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_AUV::state_wait_first_DATA_without();

    } else {
        //Communication between HN & AUV with RTS and CTS packets
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________Waiting_DATA"
                << "_pck_number_" << (getData_Rx_by_AUV() + 1) << "_from_HN(" << mac_addr_HN_ctsed << ")" << std::endl;

        refreshState(UWUFETCH_AUV_STATUS_WAIT_DATA_HN);
        if (print_transitions) printStateInfo();

        data_timeout = getDataTimerValue(); //Compute the timeout interval within AUV want to receive all DATA packets from HN 
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_wait_first_DATA() ---->AUV want to receive all the DATA packets"
                << " from the HN  (" << mac_addr_hn_triggered << ") within an interval time: " << data_timeout << " [s]." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________Timeout_within_rx_all_DATA"
                << "_pcks_" << data_timeout << "[s]." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;

        rxDATAEnabled = true;

        DATA_timer.schedule(data_timeout);
    }

} //end state_wait_first_DATA();

void uwUFetch_AUV::DATA_rx() {

    refreshState(UWUFETCH_AUV_STATUS_RECEIVE_DATA_PACKET);
    if (print_transitions) printStateInfo();

    hdr_mac* mach = HDR_MAC(curr_DATA_pck_rx);

    /**
     *  Save the information of the HN that has sent the DATA packet correctly 
     *  to the AUV
     */
    mac_addr_HN_in_data = mach->macSA(); //Source MAC address of the HN that has transmitted the DATA packet to the AUV

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::DATA_Rx() ---->AUV has received the DATA packet"
            << " number: " << getData_Rx_by_AUV() << " from the HN with"
        << " MAC address: " << mac_addr_HN_in_data << "." << std::endl;

    /**
     * save the data packet in the QUEUE of the AUV
     * an pass it to the application layer
     */
    Q_data_AUV.push(curr_DATA_pck_rx->copy()); //Save the data packets in the QUEUE of the AUV
    sendUp(curr_DATA_pck_rx->copy()); //Pass the DATA packets received from the AUV at the CBR level
    Q_data_AUV.pop();

    hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_pck_rx);

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::DATA_RX()____________________________DATA_pck_rx_by_AUV(" << addr << ")_from_HN(" << mac_addr_HN_in_data << ")_id_pck:" << cbrh->sn() << ""
        << "_IS_PASSED_TO_APP_LAYER." << std::endl;

    Packet::free(curr_DATA_pck_rx);

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::DATA_Rx() ----> The DATA packet is passed to the APPLICATION"
            << " layer of the AUV." << std::endl;

    if (mac_addr_HN_in_data == HEAD_NODE_1) {
        num_pck_hn_1++;

    } else if (mac_addr_HN_in_data == HEAD_NODE_2) {
        num_pck_hn_2++;

    } else if (mac_addr_HN_in_data == HEAD_NODE_3) {
        num_pck_hn_3++;

    } else if (mac_addr_HN_in_data == HEAD_NODE_4) {
        num_pck_hn_4++;

    }

    another_DATA_received();
} //end DATA_rx();

void uwUFetch_AUV::state_wait_DATA() {

    refreshState(UWUFETCH_AUV_STATUS_WAIT_DATA_HN);
    if (print_transitions) printStateInfo();

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_Wait_DATA() ---->AUV is waiting to receive the DATA packet"
            << " number: " << (getData_Rx_by_AUV() + 1) << " from HN with MAC address: " << mac_addr_HN_ctsed << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_DATA()____________________Waiting_DATA"
            << "_pck_number_" << (getData_Rx_by_AUV() + 1) << "_from_HN(" << mac_addr_HN_ctsed << ")" << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_DATA()____________________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_DATA()____________________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_DATA()____________________.......Waiting_DATA" << std::endl;

} //end state_wait_DATA();

void uwUFetch_AUV::DataTOExpired_first() {

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::DataTOExpired() ---->AUV has not received the first DATA packet"
            << " within the time interval pre-established, so return in IDLE STATE." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::DataTOExpired_first()_______________________DATA_timeout_expired." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::DataTOExpired_first()_______________________Total_DATA_pck_rx_from_HN(" << mac_addr_HN_in_data << ")_is_are_" << getData_Rx_by_AUV() << std::endl;

    DATA_timer.force_cancel();
    updateQueueRTS();

    n_TRIGGER_tx_by_AUV = 0;
    n_RTS_rx_by_AUV = 0;
    n_CTS_tx_by_AUV = 0;
    n_DATA_rx_by_AUV = 0;

    txTRIGGEREnabled = true;
    rxDATAEnabled = false;

    refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED); //Refresh the reason for the changing of the state
    stateIdle_AUV(); //AUV return in IDLE state

} //end DataTOExpired();

void uwUFetch_AUV::DataTOExpired() {

    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::DataTOExpired() ---->AUV has not received all the DATA packets"
            << " within the time interval pre-established, so return in IDLE STATE." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::DataTOExpired()_______________________DATA_timeout_expired." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::DataTOExpired()_______________________Total_DATA_pck_rx_from_HN(" << mac_addr_HN_in_data << ")_is_are " << getData_Rx_by_AUV() << std::endl;

    updateQueueRTS();

    n_TRIGGER_tx_by_AUV = 0;
    n_RTS_rx_by_AUV = 0;
    n_CTS_tx_by_AUV = 0;
    n_DATA_rx_by_AUV = 0;

    txTRIGGEREnabled = true;
    rxDATAEnabled = false;

    refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED); //Refresh the reason for the changing of the state
    stateIdle_AUV(); //AUV return in IDLE state

} //end DataTOExpired();

/******************************************************************************
 *                          METODI AUSILIARI                                                        *
 ******************************************************************************/
bool uwUFetch_AUV::typeCommunication() {
    if (mode_comm_hn_auv == 1) {
        return true;
    } else {
        return false;
    }
}

double uwUFetch_AUV::getDataTimerValue() {
    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        //uwUFetch_AUV::getDataTimerValue_without();
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::getDataTimerValue() ----> AUV is computing the time interval"
                << " within it want to receive all DATA packets from the HN." << std::endl;

        RTT = 0;
        computeTxTime(UWUFETCH_AUV_PACKET_TYPE_DATA);
        RTT = getRTT();

        /**
         AUV didn't know the exactly number of DATA packets that want receive from the HN,
         so it consider the worst case, that is the maximum number of DATA packets that It want to receive from 
         the HN.
         */
        double data_timeout_cnt = NUM_MAX_DATA_AUV_WANT_RX * (Tdata + T_GUARD + RTT);

        return data_timeout_cnt;
    } else {
        //Communication between HN & AUV with RTS and CTS packets
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::getDataTimerValue() ----> AUV is computing the time interval"
                << " within it want to receive all DATA packets from the HN." << std::endl;

        RTT = 0;
        computeTxTime(UWUFETCH_AUV_PACKET_TYPE_DATA);
        RTT = getRTT();

        /**
         * AUV didn't know the exactly number of DATA packets that want receive from the HN,
         * so it consider the worst case, that is the maximum number of DATA packets that It want to receive from 
         * the HN.
         */
        double data_timeout_cnt = number_data_pck_AUV_rx_exact * (Tdata + T_GUARD + RTT);

        return data_timeout_cnt;

    }
} //end getDataTimerValue() 

void uwUFetch_AUV::computeTxTime(UWUFETCH_AUV_PACKET_TYPE tp) {

    int size_pck = 0;
    if (tp == UWUFETCH_AUV_PACKET_TYPE_TRIGGER) {
        size_pck = sizeof (hdr_TRIGGER_UFETCH);
        T_tx_TRIGGER = size_pck / BIT_RATE_SERVICE;
    } else {
        size_pck = MAX_PAYLOAD;
        Tdata = size_pck / BIT_RATE_INSTANT;
    }
} //end computeTxTime()

double uwUFetch_AUV::getRTT() {
    RTT = 4000 / v_speed_sound; //4000 because is the maximum distance between AUV and HN. 
    //The exactly distance is 3500 but I have floor this distance at 4000[m] 
    return RTT;
} //end getRTT();

void uwUFetch_AUV::updateQueueRTS() {
    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::updateQueueRTS() ---->AUV remove the HN from the list of queue"
            << "  node from which it has received the RTS packets." << std::endl;
    while (!Q_rts_mac_HN.empty()) {
        Q_rts_mac_HN.pop();
        Q_rts_n_pcks_HN_want_tx_AUV.pop();
        Q_rts_backoff_time.pop();
    }
}

void uwUFetch_AUV::another_DATA_received() {
    /**
     * Verify if another DATA packet can be received
     */
    if (getData_Rx_by_AUV() == number_data_pck_AUV_rx_exact) {
        //Other data packet can not be received by the AUV
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::another_DATA_received() ---->AUV has received all the DATA packets from HN"
                << " with MAC address: " << mac_addr_HN_in_data << ", so return in IDLE STATE." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::another_DATA_received()_______________ALL_DATA_pck_rx"
                << "_from_HN(" << mac_addr_HN_in_data << ")." << std::endl;

        updateQueueRTS();

        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::another_DATA_received() ---->AUV reset DATA timeout." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::another_DATA_received()_______________Reset_DATA_timeout." << std::endl;

        DATA_timer.force_cancel(); //reset the DATA timer

        n_TRIGGER_tx_by_AUV = 0;
        n_RTS_rx_by_AUV = 0;
        n_CTS_tx_by_AUV = 0;
        n_DATA_rx_by_AUV = 0;

        txTRIGGEREnabled = true;
        rxDATAEnabled = false;

        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_LAST_DATA_PCK_RX);
        stateIdle_AUV();

    } else {
        //Another data packet can be received
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::another_DATA_received() ---->AUV can receive another DATA packet from the "
                << " HN with MAC address: " << mac_addr_HN_in_data << ". "
                << " The DATA packet number: " << (getData_Rx_by_AUV() + 1) << std::endl;

        //  if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::another_DATA_received()_______________Another_DATA_pck_can_rx._Number " << (getData_Rx_by_AUV() + 1) << "." << std::endl;
        rxDATAEnabled = true;

        refreshReason(UWUFETCH_AUV_STATUS_CHANGE_ANOTHER_DATA_PCK_RX);
        state_wait_DATA();

    }
} //end another_DATA_received();

/******************************************************************************
 *                           METODI EXTRA                                     *
 ******************************************************************************/

void uwUFetch_AUV::uwUFetch_TRIGGER_timer::expire(Event* e) {
    if (module->debug_) std::cout << NOW << " uwUFetch_AUV (" << module->addr << ")::UWUFetch_TRIGGER_timer::expire() ---->TRIGGER timeout expired." << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->TriggerTOExpired();
} //end UWUFetch_TRIGGER_timer::expire()

void uwUFetch_AUV::uwUFetch_RTS_timer::expire(Event* e) {
    if (module->debug_) std::cout << NOW << " uwUFetch_AUV (" << module->addr << ") ::UWUFetch_RTS_timer::expire() ---->RTS timeout expired." << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->RtsTOExpired();
} //end uwUFetch_RTS_timer::expire()

void uwUFetch_AUV::uwUFetch_DATA_timer::expire(Event* e) {
    if (module->debug_) std::cout << NOW << " uwUFetch_AUV (" << module->addr << ")::UWUFetch_DATA_timer::expire() ---->DATA timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->DataTOExpired();
} //end UWUFetch_DATA_timer::expire() 

void uwUFetch_AUV::uwUFetch_FIRST_DATA_timer::expire(Event* e) {
    if (module->debug_) std::cout << NOW << " uwUFetch_AUV (" << module->addr << ")::UWUFetch_FIRST_DATA_timer::expire() ---->FIRST DATA timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->DataTOExpired_first();
} //end UWUFetch_DATA_timer::expire()

void uwUFetch_AUV::printStateInfo(double delay) {
    fout << NOW << "uwUFetch(" << addr << ")::printStateInfo() " << "from " << statusInfo[prev_state]
            << " to " << statusInfo[curr_state] << ". Reason: " << statusChange[last_reason] << std::endl;
} //end printStateInfo()

void uwUFetch_AUV::initInfo() {
    initialized = true;

    if ((print_transitions) && (system(NULL))) {
        system("rm -f /tmp/uwUFetch_AUV_stateTransitions.txt");
        system("touch /tmp/uwUFetch_AUV_stateTransitions.txt");
    }

    statusInfo[UWUFETCH_AUV_STATUS_IDLE] = "AUV is in IDLE state ";
    statusInfo[UWUFETCH_AUV_STATUS_TRANSMIT_TRIGGER] = "AUV is transmitting a TRIGGER packet ";
    statusInfo[UWUFETCH_AUV_STATUS_WAIT_RTS_PACKET] = "AUV is waiting a RTS packet from HN ";
    statusInfo[UWUFETCH_AUV_STATUS_RECEIVE_RTS_PACKET] = "AUV has received a RTS packet ";
    statusInfo[UWUFETCH_AUV_STATUS_TRANSMIT_CTS_PACKET] = "AUV is transmitting a CTS packet ";
    statusInfo[UWUFETCH_AUV_STATUS_WAIT_DATA_HN] = "AUV is waiting DATA packet from HN ";
    statusInfo[UWUFETCH_AUV_STATUS_RECEIVE_DATA_PACKET] = "AUV has receive a DATA packet from HN ";

    statusChange[UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_TO_EXPIRED] = "Trigger TO is expired, so the AUV start the transmission of TRIGGER packet";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_INITIALIZE_TX_TRIGGER] = "AUV has finished to initialize the TRIGGER packet, so pass "
            "the packet to the physical layer";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_TX_WAIT_RTS] = "AUV has finished to transmit a TRIGGER packet, so enter in the state in"
            " which it wait a RTS packet";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_RTS_FINISHED_TO_RX] = "AUV has finishe to receive a RTS packet, so go to the state in which"
            " the RTS is analyzed.";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_RTS_FINSHED_TO_STORE] = "AUV has finished to store the information of the RTS packet, so go to"
            " the state in which initiale the CTS packet";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_RTS_TO_EXPIRED_AT_LEAST_ONE_RTS_RX] = "RTS timeout is expired and another RTS can not be received, "
            "but at least one RTS has been received, so go to the state"
            " in which initialize the CTS packet";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_RTS_TO_EXPIRED_0_RTS_RX] = "RTS timeout is expired and another RTS can not be received, "
            "but zero RTS has been received, so go to the state idle";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_CTS_INITIALIZED_TX_CTS] = "AUV has finished to initialize the CTS packet, so go to the state"
            " in which AUV recall the physical layer";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_CTS_TX_WAIT_DATA] = "AUV has finished to transmit a CTS packet, so go to the state in which the"
            " the AUV wait a DATA packets from the HN";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_DATA_PCK_FINISHED_TO_RX] = "AUV has finished to receive the DATA packet, so go to the DATA "
            "receive method and analyzed it";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_LAST_DATA_PCK_RX] = "AUV has received the LAST DATA packet from HN, so go to the IDLE state";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_ANOTHER_DATA_PCK_RX] = "AUV has received a DATA packet, and another DATA packet can be "
            "received by It, so go to the waiting state in which it wait another"
            " DATA packet.";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED] = "DATA timeout expired, all the packets are not received by the HN, so"
            "go to the IDLE stae";
    statusChange[UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE] = "AUV has received a packet that is not addressed to It";

    packetType[UWUFETCH_AUV_PACKET_TYPE_DATA] = "DATA packet";
    packetType[UWUFETCH_AUV_PACKET_TYPE_TRIGGER] = "TRIGGER packet";
    packetType[UWUFETCH_AUV_PACKET_TYPE_RTS] = "RTS packet";
    packetType[UWUFETCH_AUV_PACKET_TYPE_CTS] = "CTS packet";

    statusTimer[UWUFETCH_TIMER_STATUS_IDLE] = "AUV is in Idle";
    statusTimer[UWUFETCH_TIMER_STATUS_RUNNING] = "AUV is running";
    statusTimer[UWUFETCH_TIMER_STATUS_FROZEN] = "AUV is Freezing";
    statusTimer[UWUFETCH_TIMER_STATUS_EXPIRED] = "AUV timeout expired";

} //end initInfo()

void uwUFetch_AUV::waitForUser() {
    std::string response;
    std::cout << "Press Enter to continue";
    std::getline(std::cin, response);
}



