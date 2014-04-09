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
 * @file   uwUFetch_NODE.cc
 * @author Loris Brolo
 * @version 1.0.0
 * 
 * @brief Class that provide the implementation of the HEAD NODE entity of uwUFetch protocol without using 
 *  RTS and CTS packets during the communication with AUV
 */

#include "mac.h"
#include "mmac.h"
#include "uwUFetch_NODE.h"
#include "uwUFetch_cmn_hdr.h"
//#include "uwmphy_modem_cmn_hdr.h"
#include "uwcbr-module.h"

void uwUFetch_NODE::Phy2MacEndRx_HN_without(Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    int src_mac_addr = mach->macSA();
    int dest_mac_addr = mach->macDA(); //Destination MAC address, the address to which the packet is addressed

    if (debug_) std::cout << NOW << "uwUFetch_NODE(" << addr << ")::Phy2MacEndRx()______________________dest_mac_addr " << dest_mac_addr << std::endl;
    //Control if the packet that the HN has received is really for him 

    if ((dest_mac_addr == addr) || (dest_mac_addr == MAC_BROADCAST)) {
        //Packet is addressed to me HN

        if (cmh->ptype() == PT_TRIGGER_UFETCH) {
            //TRIGGER section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()______________________Finished_to_rx_TRIGGER_pck"
                    << "_from_AUV(" << src_mac_addr << ")." << std::endl;

            mac_addr_AUV_in_trigger = mach->macSA(); //Save the Mac address of the 

            if (rxTRIGGEREnabled) {
                int index_q = 0;
                /**
                 * Before to start the communication between AUV, HN move the data packets from the SN queue to the HN 
                 * data queue. This operation is done only by the HN that function also as SN
                 */
                while ((!Q_data.empty()) && (index_q <= MAX_PCK_HN_WANT_RX_FROM_NODE)) {
                    //Pick up the first element of the queue
                    curr_DATA_NODE_pck_tx_HN = (Q_data.front())->copy();
                    //Remove the element from the queue that we have pick up the packet
                    Q_data.pop();
                    //Add the packet to the queue of data packets that HN will transmit to AUV when 
                    //will be required
                    Q_data_HN.push(curr_DATA_NODE_pck_tx_HN);
                    incrTotalDataPckTx_by_NODE();
                    incrTotalDataPckRx_by_HN();
                    hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_NODE_pck_tx_HN);

                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________DATA_pck_tx:_source(" << mach->macSA() << ")_"
                        << "destination(" << mach->macDA() << ")_id_pck:" << cbrh->sn() << "" << std::endl;
                    index_q++;
                }
            }

            if ((cmh->error()) || (Q_data_HN.empty())) {
                if (cmh->error()) {
                    //Packet it's in error
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->TRIGGER packet received by the HN"
                            << " is corrupted: DROP IT" << std::endl;

                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________TRIGGER_rx_from_AUV(" << mach->macSA() << ")_It_s_in_ERROR:IGNORE_It." << std::endl;

                    rx_TRIGGER_finish_HN_time = NOW;

                    incrTotalTriggerPckRx_HN();
                    incrTotalTriggerPckRx_corrupted_HN();

                    drop(p, 1, UWFETCH_NODE_DROP_REASON_ERROR); //drop the packet  
                    refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR);

                    if (rxTRIGGEREnabled) {
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN reset the TRIGGER timer" << std::endl;

                        BeaconBeforeTx_timer.force_cancel();

                        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_HN_RX_TRIGGER_FROM_AUV_CORRUPTED);
                        rxTRIGGEREnabled = true;

                        stateIdle_HN();
                    } else {
                        //HN is not enable to receive a TRIGGER packet
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN is not enabled to receive"
                                << " the TRIGGER packet: DROP IT." << std::endl;
                    }
                } else {
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->TRIGGER packet received by the HN"
                            << " is not corrupted, but the HN has 0 DATA packet enabled to tx to the AUV: DROP IT" << std::endl;

                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________TRIGGER_rx_It_s_CORRECT_but_0_DATA_to_tx:IGNORE_It." << std::endl;

                    rx_TRIGGER_finish_HN_time = NOW;

                    incrTotalTriggerPckRx_HN();

                    drop(p, 1, UWFETCH_NODE_DROP_REASON_ERROR); //drop the packet  
                    refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR);

                    if (rxTRIGGEREnabled) {
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN reset the TRIGGER timer" << std::endl;
                        BeaconBeforeTx_timer.force_cancel();

                        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_HN_RX_TRIGGER_FROM_AUV_CORRUPTED);
                        rxTRIGGEREnabled = true;

                        stateIdle_HN();
                    } else {
                        //HN is not enable to receive a TRIGGER packet
                        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN is not enabled to receive"
                                << " the TRIGGER packet: DROP IT." << std::endl;
                    }
                }
            } else {
                //Packet it's not in error 
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->Physical layer of HN with MAC"
                        << " address: " << dest_mac_addr << " has finished to receive a TRIGGER packet"
                        << " from the AUV with MAC address: " << src_mac_addr << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________TRIGGER_rx_It_s_CORRECT_>=1_DATA_to_tx." << std::endl;

                rx_TRIGGER_finish_HN_time = NOW;

                incrTriggerPckRx_HN();
                incrTotalTriggerPckRx_HN();

                curr_TRIGGER_HN_pck_rx = p->copy();

                refreshReason(UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX);

                Packet::free(p);

                TRIGGER_rx();
            }
        } else {
            //DATA section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " DATA packet from the NODE with MAC address: " << src_mac_addr << std::endl;

            rx_DATA_finish_HN_time = NOW;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________End_to_rx_DATA_pck_"
                    << "from_SN(" << src_mac_addr << ")." << std::endl;

            if (cmh->error()) {
                //Packet it's in error
                if (debug_) std::cout << NOW << " uwUFetch_NODE( " << addr << ") ::Phy2MacEndRx() ---->DATA packet received by the HN"
                        << " is corrupted: DROP IT" << std::endl;

                curr_DATA_HN_pck_rx = p->copy();

                rx_DATA_finish_HN_time = NOW;

                incrDataPckRx_by_HN();
                incrTotalDataPckRx_by_HN();
                incrTotalDataPckRx_corrupted_by_HN();

                hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_HN_pck_rx);

                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________DATA_pck_rx_by_HN(" << addr << ")_from_SN(" << src_mac_addr << ")_id_pck:" << cbrh->sn() << ""
                    << "_It_s_in_ERROR." << std::endl;

                drop(p, 1, UWFETCH_NODE_DROP_REASON_ERROR); //drop the packet  
                refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR);

            } else {
                //Packet it's not in error 

                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->Physical layer of HN with MAC"
                        << " address: " << dest_mac_addr << " has finished to receive a DATA packet"
                        << " number: " << (getDataPckRx_by_HN() + 1) << " from the NODE with MAC address: " << src_mac_addr << std::endl;

                rx_DATA_finish_HN_time = NOW;

                incrDataPckRx_by_HN();
                incrTotalDataPckRx_by_HN();

                curr_DATA_HN_pck_rx = p->copy();

                refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_RX);

                Packet::free(p);

                DATA_rx();
            }
        }
    } else {
        //Packet is not addressed to me HN
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->The packet received is not for me HN:"
                << " DROP IT." << std::endl;

        if (cmh->ptype() == PT_POLL_UFETCH) {
            //POLL section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " POLL packet from the HN with MAC address: " << src_mac_addr << " .DROP IT" << std::endl;

            drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

        } else if (cmh->ptype() == PT_PROBE_UFETCH) {
            //BEACON section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " PROBE packet from the SENSOR NODE with MAC address: " << src_mac_addr << " .DROP IT" << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_PROBE"
                    << "_pck_from_SN(" << src_mac_addr << ").Not_addressed_to_me:_IGNORE_IT." << std::endl;

            drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

        } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
            //TRIGGER section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " TRIGGER packet from the AUV with MAC address (" << src_mac_addr << ").DROP IT" << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_TRIGGER"
                    << "_pck_from_AUV(" << src_mac_addr << ")._Not_addressed_to_me:_IGNORE_IT." << std::endl;

            drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

        } else {
            //DATA section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " DATA packet from the SENSOR NODE with MAC address: " << src_mac_addr << " .DROP IT" << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________End_to_rx_DATA_pck_"
                    << "from_SN(" << src_mac_addr << ")._Not_addressed_to_me:_IGNORE_IT." << std::endl;

            drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);
        }
    }
} //end Phy2MacEndRx_HN();

/******************************************************************************
 *                          HEAD NODE METHODS                                 *
 ******************************************************************************/

void uwUFetch_NODE::TRIGGER_rx_without() {

    if (rxTRIGGEREnabled) {
        // HN is enabled to receive a TRIGGER
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::TRIGGER_rx() ---->HN reset the TRIGGER timer" << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()_________________________TRIGGER_rx_It_s_CORRECT_and_>= 1_DATA_to_tx_and_enabled_to_rx_It." << std::endl;

        BeaconBeforeTx_timer.force_cancel(); //reset the timer in which the HN wait a TRIGGER packet 

        refreshState(UWUFETCH_NODE_STATUS_TRIGGER_RECEIVE);
        if (print_transitions) printStateInfo();

        hdr_mac* mach = HDR_MAC(curr_TRIGGER_HN_pck_rx);
        hdr_TRIGGER_UFETCH* triggerh = HDR_TRIGGER_UFETCH(curr_TRIGGER_HN_pck_rx);

        /*
         * Save the header data of the trigger
         */

        T_min_bck_DATA = (double) triggerh->t_min() / 1000; //Minimum value time before that HN transmit a DATA packet
        T_max_bck_DATA = (double) triggerh->t_max() / 1000; //Maximum value time before that HN transmit a DATA packet
        max_pck_HN_can_tx = triggerh->max_pck_want_rx(); //Maximum number of DATA packet that HN can transmit to the AUV   

        mac_addr_AUV_in_trigger = mach->macSA(); //Save the MAC address of the AUV node from which the HN has received a trigger packet

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________PARAMETERS_of_TRIGGER_PACKET: " << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Source_address:_" << mach->macSA() << "." << std::endl;
        if (mach->macDA() == -1) {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Destination_Address:_BROADCAST." << std::endl;
        } else {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Destination_Address:_" << mach->macDA() << std::endl;
        }
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________MIN_backoff_time_DATA_pck:_" << T_min_bck_DATA << "[s]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________MAX_backoff_time_DATA_pck:_" << T_max_bck_DATA << "[s]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________MAX_DATA_pck_want_rx_from_HN:_" << max_pck_HN_can_tx << "[pck]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________DATA_pck_would_like_to_tx_to_the_AUV:_" << Q_data_HN.size() << "[pck]." << std::endl;

        rxTRIGGEREnabled = false;
        txBEACONEnabled = false;

        bck_before_tx_data = choiceBackOffTimer_HN();
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Backoff_timeout_choice:_" << bck_before_tx_data << "[s]." << std::endl;

        BCK_timer_data.schedule(bck_before_tx_data);

    } else {
        //HN is not enable to receive a TRIGGER packet
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::TRIGGER_rx() ---->HN is not enabled to receive"
                << " the TRIGGER packet: DROP IT." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()_________________________TRIGGER_rx_It_s_CORRECT_and_>= 1_DATA_to_tx_and_not_enabled_to_rx_It:IGNORE_It." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Not_ENABLED"
                << "_to_rx_TRIGGER_pck." << std::endl;

        drop(curr_TRIGGER_HN_pck_rx, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);
    }
} //end TRIGGER_rx();

void uwUFetch_NODE::BCKTOExpired_HN_without() {
    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::BCKTOExpired() ---->Back-off timeout is expired:"
            << " so the first DATA packet can be transmitted by the HN to the AUV." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::BCKTOExpired()________________________Backoff_timeout_expired." << std::endl;

    state_DATA_HN_first_tx();
}

void uwUFetch_NODE::state_DATA_HN_first_tx_without() {
    //Compute the maximum number of DATA packet that the HN can be transmitted to the AUV
    if (Q_data_HN.size() >= max_pck_HN_can_tx) {
        max_data_HN_can_tx = max_pck_HN_can_tx;

    } else {
        max_data_HN_can_tx = Q_data_HN.size();
    }

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_first_tx() ---->HN will be transmit " << max_data_HN_can_tx
            << " DATA packets to the AUV with MAC address: " << mac_addr_AUV_in_trigger << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_first_tx()______________DATA_pck_to_tx_"
            << max_data_HN_can_tx << "_to_the_AUV(" << mac_addr_AUV_in_trigger << ")." << std::endl;

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_first_tx() ---->HN is starting transmission of the"
            << " FIRST DATA packet to the AUV with MAC address: " << mac_addr_AUV_in_trigger << std::endl;

    //Pick up the first element of the queue
    curr_DATA_HN_pck_tx = (Q_data_HN.front())->copy();
    //Remove the element from the queue that we have pick up 
    Q_data_HN.pop();

    hdr_mac* mach = HDR_MAC(curr_DATA_HN_pck_tx);
    hdr_cmn* cmh = hdr_cmn::access(curr_DATA_HN_pck_tx);
    hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_HN_pck_tx);

    mach->set(MF_CONTROL, addr, mac_addr_AUV_in_trigger);
    mach->macSA() = addr;
    mach->macDA() = mac_addr_AUV_in_trigger;
    cmh->size() = sizeof (curr_DATA_HN_pck_tx);

    if (burstDATA()) {
        //        hdr_uwmphy_modem* modemh = HDR_UWMPHY_MODEM(curr_DATA_HN_pck_tx);
        //        modemh->use_burst_data() = 1;
        //        if (getDataPckTx_by_HN() == (max_data_HN_can_tx)) {
        //            modemh->last_packet() = 1;
        //            modemh->use_burst_data() = 1;
        //        } else {
        //            modemh->last_packet() = 0;
        //            modemh->use_burst_data() = 1;
        //        }
        txDATAEnabledHN = true;

        int old_sn = cbrh->sn();
        cbrh->sn() = pck_number_id;
        pck_number_id++;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_first_tx()______________DATA_pck_tx:_source_SN(" << Q_data_source_SN.front() << ")_source_HN(" << mach->macSA() << ")_"
            << "destination(" << mach->macDA() << ")_id_pck_original:" << old_sn << "_new_id_pck:" << cbrh->sn() << "_size:" << cmh->size() << "[byte]" << std::endl;
        Q_data_source_SN.pop();

        DATA_HN_tx();
    } else {
        txDATAEnabledHN = true;

        int old_sn = cbrh->sn();
        cbrh->sn() = pck_number_id;
        pck_number_id++;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_first_tx()______________DATA_pck_tx:_source_SN(" << Q_data_source_SN.front() << ")_source_HN(" << mach->macSA() << ")_"
            << "destination(" << mach->macDA() << ")_id_pck_original:" << old_sn << "_new_id_pck:" << cbrh->sn() << "_size:" << cmh->size() << "[byte]" << std::endl;

        Q_data_source_SN.pop();
        DATA_HN_tx();
    }
} //end state_DATA_HN_first_tx_without();

void uwUFetch_NODE::DATA_HN_tx_without() {
    //HN is enabled to transmit DATA packet
    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_HN_tx() ---->HN is transmitting a DATA packet to the AUV"
            << " with MAC address: " << mac_addr_AUV_in_trigger << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_PCK_TX);

    Mac2PhyStartTx(curr_DATA_HN_pck_tx);

} //end DATA_HN_tx();

void uwUFetch_NODE::state_DATA_HN_tx_without() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_tx() ---->HN is starting transmission of the"
            << " DATA packet number: " << (getDataPckTx_by_HN() + 1) << " to the AUV with"
        << " MAC address: " << mac_addr_AUV_in_trigger << std::endl;

    //Pick up the first element of the queue
    curr_DATA_HN_pck_tx = (Q_data_HN.front())->copy();
    //Remove the element from the queue that we have pick up 
    Q_data_HN.pop();

    hdr_mac* mach = HDR_MAC(curr_DATA_HN_pck_tx);
    hdr_cmn* cmh = hdr_cmn::access(curr_DATA_HN_pck_tx);
    hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_HN_pck_tx);

    mach->set(MF_CONTROL, addr, mac_addr_AUV_in_trigger);
    mach->macSA() = addr;
    mach->macDA() = mac_addr_AUV_in_trigger;
    cmh->size() = sizeof (curr_DATA_HN_pck_tx);

    if (burstDATA()) {
        //        hdr_uwmphy_modem* modemh = HDR_UWMPHY_MODEM(curr_DATA_HN_pck_tx);
        //        // modemh->use_burst_data() = 1;
        //        if (getDataPckTx_by_HN() == (max_data_HN_can_tx)) {
        //            modemh->last_packet() = 1;
        //            modemh->use_burst_data() = 1;
        //        } else {
        //            modemh->last_packet() = 0;
        //            modemh->use_burst_data() = 1;
        //        }
        txDATAEnabledHN = true;

        int old_sn = cbrh->sn();
        cbrh->sn() = pck_number_id;
        pck_number_id++;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_tx()____________________DATA_pck_tx:_source_SN(" << Q_data_source_SN.front() << ")_source_HN(" << mach->macSA() << ")_"
            << "destination(" << mach->macDA() << ")_id_pck_original:" << old_sn << "_new_id_pck:" << cbrh->sn() << "_size:" << cmh->size() << "[byte]" << std::endl;
        Q_data_source_SN.pop();

        DATA_HN_tx();
    } else {
        txDATAEnabledHN = true;

        int old_sn = cbrh->sn();
        cbrh->sn() = pck_number_id;
        pck_number_id++;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_tx()_____________________DATA_pck_tx:_source_SN(" << Q_data_source_SN.front() << ")_source_HN(" << mach->macSA() << ")_"
            << "destination(" << mach->macDA() << ")_id_pck_original:" << old_sn << "_new_id_pck:" << cbrh->sn() << "_size:" << cmh->size() << "[byte]" << std::endl;
        Q_data_source_SN.pop();

        DATA_HN_tx();
    }

} //end state_DATA_HN_tx_without();

void uwUFetch_NODE::state_DATA_HN_finish_tx_without() {
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_finish_tx()_____________End_tx_DATA_pcks_to_the_AUV." << std::endl;
    if (sectionCBeacon) {
        //HN has start the communication with AUV before to start the transmission of CBEACON packet, so after the end of the communication
        //with AUV, HN return in state_CBEACON_tx().
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_finish_tx() ---->HN has transmitted all his DATA"
                << " packets to the AUV with MAC address: " << mac_addr_AUV_in_trigger << ", so return in STATE_CBEACON_TX()." << std::endl;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_HN_FINISH_TX_DATA_PCK_TO_AUV_RETURN_CBEACON_TX);
        sectionCBeacon = false;
        n_TRIGGER_pck_rx_by_HN = 0;
        n_DATA_pck_tx_by_HN = 0;

        txDATAEnabledHN = false;

        state_CBEACON_tx();

    } else {
        //HN has start the communication with AUV when it was in IDLE STATE, so after the end of the communication
        //with AUV, HN return in stateIdle_HN().
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_finish_tx() ---->HN has transmitted all his DATA"
                << " packets to the AUV with MAC address: " << mac_addr_AUV_in_trigger << ", so return in IDLE STATE." << std::endl;
        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_HN_TX_ALL_DATA_TO_AUV);
        n_TRIGGER_pck_rx_by_HN = 0;
        n_DATA_pck_tx_by_HN = 0;

        txBEACONEnabled = true;
        rxTRIGGEREnabled = true;
        txDATAEnabledHN = false;

        stateIdle_HN();
    }
} //end state_DATA_HN_finish_tx_without();