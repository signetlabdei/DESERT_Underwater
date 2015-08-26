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
 * @file   uwUFetch_AUV.cc
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

/******************************************************************************
 *                           TX/RX METHODS                                    *
 ******************************************************************************/
void uwUFetch_AUV::Mac2PhyStartTx_without(Packet *p) {

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

        tx_TRIGGER_start_time = NOW;

        MMac::Mac2PhyStartTx(p);
    }
} //end Mac2PhyStartTx()

void uwUFetch_AUV::Phy2MacEndTx_without(const Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);

    if (cmh->ptype() == PT_TRIGGER_UFETCH) {
        //TRIGGER section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of AUV has finished"
                << " to transmit a TRIGGER packet to the HN (" << mach->macDA() << "), now the AUV is waiting for"
            << " the reception of DATA packets from that HN." << std::endl;

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
        state_wait_first_DATA();

    }
} //end Phy2MacEndTx()

void uwUFetch_AUV::Phy2MacStartRx_without(const Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    int src_mac_addr = mach->macSA();

    if (cmh->ptype() == PT_BEACON_UFETCH) {
        //BEACON section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                << " BEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_POLL_UFETCH) {
        //POLL section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                << " POLL packet from the HN with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
        //CBEACON section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                << " CBEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_PROBE_UFETCH) {
        //PROBE section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                << " PROBE packet from the NODE with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
        //TRIGGER section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << std::endl;

    } else {
        //DATA section
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacStartRx() ---->AUV is starting to receive a"
                << " DATA packet from the HN with MAC address: " << src_mac_addr << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacStartRx()______________________Start_to_rx_DATA_pck"
                << "_from_HN(" << src_mac_addr << ")." << std::endl;
        rx_DATA_start_time = NOW;
    }
} //Phy2MacStartRx_without();

void uwUFetch_AUV::Phy2MacEndRx_without(Packet *p) {

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

            if (src_mac_addr == mac_addr_hn_triggered) {
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV reset the DATA timeout." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________RESET_DATA_timeout:BEACON_rx_from_triggered_HN." << std::endl;

                DATA_timer_first_pck.force_cancel();
                DATA_timer.force_cancel();

                n_TRIGGER_tx_by_AUV = 0;
                n_DATA_rx_by_AUV = 0;

                txTRIGGEREnabled = true;
                rxDATAEnabled = false;

                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED); //Refresh the reason for the changing of the state
                stateIdle_AUV(); //AUV return in IDLE state
            }

        } else if (cmh->ptype() == PT_POLL_UFETCH) {
            //POLL section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                    << " POLL packet from the HN with MAC address: " << src_mac_addr << " .IGNORE IT" << std::endl;

            if (src_mac_addr == mac_addr_hn_triggered) {
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV reset the DATA timeout." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________RESET_DATA_timeout:POLL_rx_from_triggered_HN." << std::endl;

                DATA_timer_first_pck.force_cancel();
                DATA_timer.force_cancel();

                n_TRIGGER_tx_by_AUV = 0;
                n_DATA_rx_by_AUV = 0;

                txTRIGGEREnabled = true;
                rxDATAEnabled = false;

                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED); //Refresh the reason for the changing of the state
                stateIdle_AUV(); //AUV return in IDLE state
            }

        } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
            //CBEACON section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                    << " CBEACON packet from the HN with MAC address: " << src_mac_addr << " .IGNORE IT." << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________Finished to rx CBEACON pck"
                    << " from HN(" << src_mac_addr << "): IGNORE IT." << std::endl;

            if (src_mac_addr == mac_addr_hn_triggered) {
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV reset the DATA timeout." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________RESET_DATA_timeout:CBEACON_rx_from_triggered_HN." << std::endl;
                DATA_timer_first_pck.force_cancel();
                DATA_timer.force_cancel();

                n_TRIGGER_tx_by_AUV = 0;
                n_DATA_rx_by_AUV = 0;

                txTRIGGEREnabled = true;
                rxDATAEnabled = false;

                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED); //Refresh the reason for the changing of the state
                stateIdle_AUV(); //AUV return in IDLE state
            }

        } else if (cmh->ptype() == PT_PROBE_UFETCH) {
            //PROBE section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                    << " PROBE packet from the NODE with MAC address: " << src_mac_addr << " .IGNORE IT." << std::endl;

        } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
            //TRIGGER section
            if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV has finished to receive a"
                    << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << " .IGNORE IT." << std::endl;

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

                    //Reset the DATA timeout within it want to receive the first packet
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________Reset_FIRST_DATA_PACKET_TIMEOUT." << std::endl;
                    DATA_timer_first_pck.force_cancel();

                    another_DATA_received();
                } else {
                    //AUV is not enabled to receive a DATA packet
                    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV is not enabled to receive the DATA"
                            << " packet: DROP IT." << std::endl;

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

                //Reset the DATA timeout within it want to receive the first packet
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________Reset FIRST DATA PACKET TIMEOUT." << std::endl;
                DATA_timer_first_pck.force_cancel();

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
            if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________POLL not addressed to me: IGNORE IT." << std::endl;

            drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
            refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            if (src_mac_addr == mac_addr_hn_triggered) {
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV reset the DATA timeout." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________RESET_DATA_timeout:POLL_rx_from_triggered_HN." << std::endl;

                DATA_timer_first_pck.force_cancel();
                DATA_timer.force_cancel();

                n_TRIGGER_tx_by_AUV = 0;
                n_DATA_rx_by_AUV = 0;

                txTRIGGEREnabled = true;
                rxDATAEnabled = false;

                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED); //Refresh the reason for the changing of the state
                stateIdle_AUV(); //AUV return in IDLE state
            }

        } else if (cmh->ptype() == PT_CBEACON_UFETCH) {

            drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
            refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

            if (src_mac_addr == mac_addr_hn_triggered) {
                if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::Phy2MacEndRx() ---->AUV reset the DATA timeout." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::Phy2MacEndRx()_________________________RESET_DATA_timeout:CBEACON_rx_from_triggered_HN." << std::endl;
                DATA_timer_first_pck.force_cancel();
                DATA_timer.force_cancel();

                n_TRIGGER_tx_by_AUV = 0;
                n_DATA_rx_by_AUV = 0;

                txTRIGGEREnabled = true;
                rxDATAEnabled = false;

                refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TO_WAIT_DATA_EXPIRED); //Refresh the reason for the changing of the state
                stateIdle_AUV(); //AUV return in IDLE state
            }

        } else if (cmh->ptype() == PT_PROBE_UFETCH) {
            drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
            refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

        } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
            drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
            refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);

        } else {
            drop(p, 1, UWFETCH_AUV_DROP_REASON_WRONG_RECEIVER);
            refreshReason(UWUFETCH_AUV_STATUS_CHANGE_PACKET_FOR_ANOTHER_NODE);
        }
    }
} //end Phy2MacEndRx_without()

/******************************************************************************
 *                              AUV METHODS                                   *
 ******************************************************************************/
void uwUFetch_AUV::state_TRIGGER_tx_without() {

    refreshState(UWUFETCH_AUV_STATUS_TRANSMIT_TRIGGER);
    if (print_transitions) printStateInfo();

    Packet* p = Packet::alloc(); //allocate the memory for the packet
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    hdr_TRIGGER_UFETCH* triggerh = HDR_TRIGGER_UFETCH(p);

    cmh->ptype() = PT_TRIGGER_UFETCH; //assign the type of the packet

    if (index == 0) {
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_TRIGGER_tx() ---->Initialization TRIGGER packet." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Initialize_TRIGGER_pck_that"
                << "_will_be_tx_to_the_HN(" << HEAD_NODE_1 << ")." << std::endl;

        mac_addr_hn_triggered = HEAD_NODE_1;
        mach->set(MF_CONTROL, addr, HEAD_NODE_1); //set the information of the packet (type,source address,destination address)
        mach->macSA() = addr; //set the source address as AUV MAC address
        mach->macDA() = HEAD_NODE_1; //set the destination address as BROADCAST address
        if (index == NUM_HN_NET)
            index = index + NUM_HN_NET + 1;
        else {
            index = index + 1;
        }
    } else if (index == 1) {

        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_TRIGGER_tx() ---->Initialization TRIGGER packet." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Initialize_TRIGGER_pck_that"
                << "_will_be_tx_to_the_HN(" << HEAD_NODE_2 << ")." << std::endl;

        mac_addr_hn_triggered = HEAD_NODE_2;
        mach->set(MF_CONTROL, addr, HEAD_NODE_2); //set the information of the packet (type,source address,destination address)
        mach->macSA() = addr; //set the source address as AUV MAC address
        mach->macDA() = HEAD_NODE_2; //set the destination address as BROADCAST address

        if (index == NUM_HN_NET)
            index = index + NUM_HN_NET + 1;
        else {
            index = index + 1;
        }

    } else if (index == 2) {

        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_TRIGGER_tx() ---->Initialization TRIGGER packet." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Initialize_TRIGGER_pck_that"
                << "_will_be_tx_to_the_HN(" << HEAD_NODE_3 << ")." << std::endl;

        mach->set(MF_CONTROL, addr, HEAD_NODE_3); //set the information of the packet (type,source address,destination address)
        mach->macSA() = addr; //set the source address as AUV MAC address
        mach->macDA() = HEAD_NODE_3; //set the destination address as BROADCAST address
        mac_addr_hn_triggered = HEAD_NODE_3;
        if (index == NUM_HN_NET)
            index = index + NUM_HN_NET + 1;
        else {
            index = index + 1;
        }

    } else if (index == 3) {

        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_TRIGGER_tx() ---->Initialization TRIGGER packet." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Initialize_TRIGGER_pck_that"
                << "_will_be_tx_to_the_HN(" << HEAD_NODE_4 << ")." << std::endl;

        mach->set(MF_CONTROL, addr, HEAD_NODE_4); //set the information of the packet (type,source address,destination address)
        mach->macSA() = addr; //set the source address as AUV MAC address
        mach->macDA() = HEAD_NODE_4; //set the destination address as BROADCAST address
        mac_addr_hn_triggered = HEAD_NODE_4;
        if (index == NUM_HN_NET)
            index = index + NUM_HN_NET + 1;
        else {
            index = index + 1;
        }

    } else {
        if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_TRIGGER_tx() ---->Initialization TRIGGER packet." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Initialize_TRIGGER_pck_that"
                << "_will_be_tx_to_the_HN(" << HEAD_NODE_1 << ")." << std::endl;

        mac_addr_hn_triggered = HEAD_NODE_1;
        mach->set(MF_CONTROL, addr, HEAD_NODE_1); //set the information of the packet (type,source address,destination address)
        mach->macSA() = addr; //set the source address as AUV MAC address
        mach->macDA() = HEAD_NODE_1; //set the destination address as BROADCAST address 
        index = 1;

    }

    //Filling the header of TRIGGER packet e determine the size of the TRIGGER packet
    triggerh->t_min() = (int) (T_MIN_RTS * 1000); //minimum value for to choose the back-off time for tx RTS pck
    triggerh->t_max() = (int) (T_MAX_RTS * 1000); //maximum value for to choose the back-off time for tx RTS pck
    triggerh->max_pck_want_rx() = NUM_MAX_DATA_AUV_WANT_RX; //maximum number of DATA packets that the AUV want to receive from the HN
    cmh->size() = sizeof (hdr_TRIGGER_UFETCH); //set the size of TRIGGER header packet

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________PARAMETERS_of_TRIGGER_PACKET." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Source_address:_" << mach->macSA() << std::endl;
    if (mach->macDA() == -1) {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Destination_Address:_BROADCAST." << std::endl;
    } else {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Destination_Address:_" << mach->macDA() << std::endl;
    }
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________MIN_backoff_time_DATA_pck:_" << T_MIN_RTS << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________MAX_backoff_time_DATA_pck:_" << T_MAX_RTS << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________MAX_DATA_pck_want_rx_from_HN:_" << triggerh->max_pck_want_rx() << "[pck]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_TRIGGER_tx()_____________________Size_of_TRIGGER_pck:_" << cmh->size() << "[byte]." << std::endl;

    //Create a copy f the TRIGGER packet
    curr_TRIGGER_pck_tx = p->copy();

    //DE-allocate the memory occupied by TRIGGER packet
    Packet::free(p);

    //Recall the function that simulate the transmission of the trigger
    refreshReason(UWUFETCH_AUV_STATUS_CHANGE_TRIGGER_INITIALIZE_TX_TRIGGER);
    TRIGGER_tx();
} //end state_TRIGGER_tx_without();

void uwUFetch_AUV::state_wait_first_DATA_without() {

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_________________Waiting_DATA"
            << "_pck_number_" << (getData_Rx_by_AUV() + 1) << "_from_HN(" << mac_addr_hn_triggered << ")" << std::endl;

    refreshState(UWUFETCH_AUV_STATUS_WAIT_DATA_HN);
    if (print_transitions) printStateInfo();

    data_timeout = getDataTimerValue(); //Compute the timeout interval within 
    //AUV want to receive all DATA packets from HN 
    if (debug_) std::cout << NOW << " uwUFetch_AUV (" << addr << ") ::state_wait_first_DATA() ---->AUV want to receive all the DATA packets"
            << " from the HN  (" << mac_addr_hn_triggered << ") within an interval time: " << data_timeout << " [s]." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________Timeout_within_rx_all_DATA"
            << "_pcks_" << data_timeout << "[s]." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_AUV(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;

    rxDATAEnabled = true;

    DATA_timer.schedule(data_timeout);
    DATA_timer_first_pck.schedule(data_timeout / 8);

} //end state_wait_first_DATA();




