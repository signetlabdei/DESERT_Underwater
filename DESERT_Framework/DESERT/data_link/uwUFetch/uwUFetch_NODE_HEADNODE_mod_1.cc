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
 * @file   uwUFetch_NODE_HEADNODE_mod_1.cc
 * @author Loris Brolo
 * @version 1.0.0
 * 
 * @brief Class that provide the implementation of  HEAD NODE entity of uwUFetch protocol using a RTS and 
 *  CTS packets when HN communicate with AUV 
 */

#include "mac.h"
#include "mmac.h"
#include "uwUFetch_NODE.h"
#include "uwUFetch_cmn_hdr.h"
//#include "uwmphy_modem_cmn_hdr.h"
#include "uwcbr-module.h"

/*******************************************************************************
 *                          GENERAL METHODS                                    * 
 *******************************************************************************/
void uwUFetch_NODE::Phy2MacStartRx_HN(const Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    int src_mac_addr = mach->macSA();

    if (cmh->ptype() == PT_BEACON_UFETCH) {
        //BEACON section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a "
                << " BEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_POLL_UFETCH) {
        //POLL section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a "
                << " POLL packet from the HN with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
        //CBEACON section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a "
                << " CBEACON packet from the HN with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_PROBE_UFETCH) {
        //PROBE section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a "
                << " PROBE packet from the NODE with MAC address: " << src_mac_addr << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacStartRx()______________________Start_to_rx_PROBE_pck"
                << "_from_SENSOR_NODE(" << src_mac_addr << ")." << std::endl;

        rx_PROBE_start_HN_time = NOW;

    } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
        //TRIGGER section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a"
                << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacStartRx()______________________Start_to_rx_TRIGGER_pck"
                << "_from_AUV(" << src_mac_addr << ")." << std::endl;

        rx_TRIGGER_start_HN_time = NOW;

    } else if (cmh->ptype() == PT_RTS_UFETCH) {
        //RTS section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a "
                << " RTS packet from the HN with MAC address: " << src_mac_addr << std::endl;

    } else if (cmh->ptype() == PT_CTS_UFETCH) {
        //CTS section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a "
                << " CTS packet from the AUV with MAC address: " << src_mac_addr << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacStartRx()______________________Start_to_rx_CTS_pck"
                << "_from_AUV(" << src_mac_addr << ")." << std::endl;

        rx_CTS_start_HN_time = NOW;

    } else {
        //DATA section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacStartRx() ---->HN is starting to receive a "
                << " DATA packet from the NODE with MAC address: " << src_mac_addr << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacStartRx()______________________Start_to_rx_DATA_pck"
                << "_from_SN(" << src_mac_addr << ")." << std::endl;

        rx_DATA_start_HN_time = NOW;
    }

} //end Phy2MacStartRx_HN();

void uwUFetch_NODE::Phy2MacEndRx_HN(Packet *p) {
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    int src_mac_addr = mach->macSA();
    int dest_mac_addr = mach->macDA(); //Destination MAC address, the address to which the packet is addressed

    if (debug_) std::cout << NOW << "uwUFetch_NODE(" << addr << ")::Phy2MacEndRx()______________________dest_mac_addr " << dest_mac_addr << std::endl;
    //Control if the packet that the HN has received is really for him 

    if ((dest_mac_addr == addr) || (dest_mac_addr == MAC_BROADCAST)) {
        //Packet is addressed to me HN

        if (cmh->ptype() == PT_BEACON_UFETCH) {
            //BEACON section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " BEACON packet from the HN with MAC address: " << src_mac_addr << " .DROP IT" << std::endl;

            drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

        } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
            //CBEACON section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " CBEACON packet from the HN with MAC address: " << src_mac_addr << " .DROP IT." << std::endl;

            drop(p, 1, UWUFETCH_NODE_DROP_CAN_NOT_RX_THIS_PCK);

        } else if (cmh->ptype() == PT_PROBE_UFETCH) {
            //PROBE section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " PROBE packet from the NODE with MAC address: " << src_mac_addr << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_PROBE"
                    << "_pck_from_SENSOR_NODE(" << src_mac_addr << ")." << std::endl;


            if (cmh->error()) {
                //Packet it's in error
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->PROBE packet received by the HN"
                        << " is corrupted: DROP IT." << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________PROBE_pck_rx_from_SN(" << src_mac_addr << ")_It_s_in_ERROR." << std::endl;

                rx_PROBE_finish_HN_time = NOW;

                incrTotalProbePckRx_HN();
                incrTotalProbePckRx_corrupted_HN();

                drop(p, 1, UWFETCH_NODE_DROP_REASON_ERROR); //drop the packet  
                refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR);

            } else {
                if (rxPROBEEnabled) {
                    //Packet is not in error
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->Physical layer of HN with MAC"
                            << " address: " << dest_mac_addr << " has finished to receive a PROBE packet"
                            << " number: " << (getProbePckRx_HN() + 1) << " from the NODE with MAC address: " << src_mac_addr << std::endl;

                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________PROBE_pck_rx_from_SN(" << src_mac_addr << ")_It_s_CORRECT." << std::endl;

                    rx_PROBE_finish_HN_time = NOW;

                    incrProbePckRx_HN();
                    incrTotalProbePckRx_HN();

                    curr_PROBE_HN_pck_rx = p->copy();

                    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_PROBE_RX);

                    Packet::free(p);

                    PROBE_rx();

                } else {
                    //HN is not enabled to receive a PROBE packet 
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN is not enabled to receive the PROBE"
                            << " packet: DROP IT." << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________HN_is_not_ENABLED_to_rx_PROBE_pck_from_SN(" << src_mac_addr << ")." << std::endl;

                    rx_PROBE_finish_HN_time = NOW;

                    drop(p, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);

                }
            }

        } else if (cmh->ptype() == PT_TRIGGER_UFETCH) {
            if (typeCommunication()) {
                //Communication between HN & AUV without RTS and CTS packets

                uwUFetch_NODE::Phy2MacEndRx_HN_without(p);
            } else {
                //Communication between HN & AUV with RTS and CTS packets
                //TRIGGER section
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                        << " TRIGGER packet from the AUV with MAC address: " << src_mac_addr << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_TRIGGER_pck"
                        << "_from_AUV(" << src_mac_addr << ")." << std::endl;

                mac_addr_AUV_in_trigger = mach->macSA(); //Save the Mac address of the 

                /**
                 * remove the DATA packet into the queue of the SENSOR NODE and put It
                 * into the queue of HN
                 * @param p
                 */
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
                        Q_data_source_SN.push(addr); //Save the MAC address from which the data packet it's arrived.
                        incrTotalDataPckTx_by_NODE();
                        incrTotalDataPckRx_by_HN();

                        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_NODE_pck_tx_HN);

                        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________DATA_pck_rx:_source(" << addr << ")_"
                                << "destination(" << addr << ")_id_pck:" << cbrh->sn() << "" << std::endl;
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
                            //if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________TRIGGER rx in ERROR && HN is enabled to rx It: IGNORE It." << std::endl;

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
            }

        } else if (cmh->ptype() == PT_CTS_UFETCH) {
            //CTS section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " CTS packet from the AUV with MAC address: " << src_mac_addr << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_CTS_pck"
                    << "_from_AUV(" << src_mac_addr << ")." << std::endl;

            if (rxCTSEnabled) {
                if (cmh->error()) {
                    //Packet it's in error
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->CTS packet received by the HN"
                            << " is corrupted: DROP IT" << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________CTS_pck_rx_from_AUV(" << src_mac_addr << ")_It_s_in_ERROR." << std::endl;

                    rx_CTS_finish_HN_time = NOW;

                    incrTotalCtsPckRx_HN();
                    incrTotalCtsPckRx_corrupted_HN();

                    drop(p, 1, UWFETCH_NODE_DROP_REASON_ERROR); //drop the packet  
                    refreshReason(UWFETCH_NODE_STATUS_CHANGE_PACKET_ERROR);

                    //return in Idle state 
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN reset the timer to wait CTS,"
                            << " and return in IDLE STATE." << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________RESET_CTS_timeout." << std::endl;
                    CTS_timer.force_cancel();
                    //Trigger_timer.force_cancel();

                    n_TRIGGER_pck_rx_by_HN = 0;
                    n_RTS_pck_tx_by_HN = 0;
                    n_CTS_pck_rx_by_HN = 0;
                    n_DATA_pck_tx_by_HN = 0;

                    rxCTSEnabled = false;
                    rxTRIGGEREnabled = true;
                    CTSrx = true;
                    stateIdle_HN();

                } else {
                    //Packet it's not in error 
                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->Physical layer of HN with MAC"
                            << " address: " << dest_mac_addr << " has finished to receive a CTS packet"
                            << " from the AUV with MAC address: " << src_mac_addr << std::endl;

                    rx_CTS_finish_HN_time = NOW;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________CTS_pck_rx_from_AUV(" << src_mac_addr << ")_It_s_CORRECT." << std::endl;

                    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN reset the timer to wait CTS,"
                            << " and COMPUTE IT." << std::endl;
                    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________RESET_CTS_timeout." << std::endl;
                    CTS_timer.force_cancel(); //reset the timer in which the HN wait a TRIGGER packet 
                    //Trigger_timer.force_cancel();

                    incrCtsPckRx_HN();
                    incrTotalCtsPckRx_HN();

                    curr_CTS_HN_pck_rx = p->copy();

                    Packet::free(p);
                    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_CTS_RX);

                    CTS_rx();
                }
            } else {
                //HN is not enable to receive a CTS packet
                CTSrx = true;

                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN is not enabled to receive"
                        << " the CTS packet: DROP IT." << std::endl;
                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndRx()________________________CTS_pck_rx_from_AUV(" << src_mac_addr << ")_It_s_not_enabled_to_rx_It." << std::endl;

                drop(curr_CTS_HN_pck_rx, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);
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

                rx_DATA_finish_HN_time = NOW;

                curr_DATA_HN_pck_rx = p->copy();

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

                hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_HN_pck_rx);

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

        } else if (cmh->ptype() == PT_CTS_UFETCH) {
            //CTS section
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndRx() ---->HN has finished to receive a"
                    << " POLL packet from the HN with MAC address: " << src_mac_addr << " .DROP IT" << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE (" << addr << ")::Phy2MacEndRx()________________________Finished_to_rx_CTS"
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

void uwUFetch_NODE::Mac2PhyStartTx_HN(Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    if (cmh->ptype() == PT_BEACON_UFETCH) {
        //BEACON section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the HN passes the BEACON"
                << " packet to his physical layer." << std::endl;

        if (mach->macDA() == -1) {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_BEACON_pck_in_BROADCAST." << std::endl;
        } else {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_BEACON_pck_to_SENSOR_NODE(" << mach->macDA() << ")." << std::endl;
        }
        tx_BEACON_start_HN_time = NOW;

        MMac::Mac2PhyStartTx(p);

    } else if (cmh->ptype() == PT_POLL_UFETCH) {
        //POLL section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of HN passes the POLL"
                << " packet to his physical layer." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_POLL_pck_to_the_SENSOR_NODE(" << mac_addr_NODE_polled << ")." << std::endl;

        tx_POLL_start_HN_time = NOW;

        MMac::Mac2PhyStartTx(p);

    } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
        //CBEACON section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the HN passes the CBEACON"
                << " packet to his physical layer." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_CBEACON_pck_in_BROADCAST." << std::endl;

        tx_CBEACON_start_HN_time = NOW;

        MMac::Mac2PhyStartTx(p);

    } else if (cmh->ptype() == PT_RTS_UFETCH) {
        //RTS section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the HN passes the RTS"
                << " packet to his physical layer." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_RTS_pck_to_the_AUV(" << mac_addr_AUV_in_trigger << ")." << std::endl;

        tx_RTS_start_HN_time = NOW;

        MMac::Mac2PhyStartTx(p);

    } else {
        //DATA section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Mac2PhyStartTx() ---->MAC layer of the HN passes the DATA"
                << " packet to his physical layer." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Mac2PhyStartTx()______________________Start_tx_of_DATA_pck"
                << "_to_the_AUV(" << mac_addr_AUV_in_CTS << ")." << std::endl;

        tx_DATA_start_HN_time = NOW;

        Tdata_HN_pck = Mac2PhyTxDuration(p);
        //Tdata_HN_pck = Tdata_HN_pck + 0.2;
        Tdata_HN_pck = TIME_BETWEEN_2_TX_DATA_HN_AUV;

        MMac::Mac2PhyStartTx(p);
    }
} //end Mac2PhyStartTx_HN();

void uwUFetch_NODE::Phy2MacEndTx_HN(const Packet *p) {

    hdr_cmn* cmh = hdr_cmn::access(p);

    if (cmh->ptype() == PT_BEACON_UFETCH) {
        //BEACON section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of HN has finished"
                << " to transmit a BEACON packet to the NODEs in broadcast." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndTx()________________________End_the_tx_of_BEACON_pck_in_BROADCAST." << std::endl;

        tx_BEACON_finish_HN_time = NOW;
        incrBeaconPckTx_by_HN();
        incrTotalBeaconPckTx_by_HN();

        txBEACONEnabled = false;

        state_wait_PROBE();

    } else if (cmh->ptype() == PT_POLL_UFETCH) {
        //POLL section 
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of the HN has finished"
                << " to transmit a POLL packet to the NODE with MAC address: " << mac_addr_NODE_polled << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndTx()________________________End_the_tx_of_POLL_pck_to_the_SENSOR NODE"
                << "(" << mac_addr_NODE_polled << ")." << std::endl;

        tx_POLL_finish_HN_time = NOW;

        incrPollPckTx_by_HN();
        incrTotalPollPckTx_by_HN();

        txPOLLEnabled = false;

        state_wait_first_DATA();

    } else if (cmh->ptype() == PT_CBEACON_UFETCH) {
        //CBEACON section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of the HN has finished"
                << " to transmit a CBEACON packet to the NODEs in broadcast." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndTx()________________________End_the_tx_of_CBEACON_pck_in_BROADCAST." << std::endl;

        tx_CBEACON_finish_HN_time = NOW;

        incrCBeaconPckTx_by_HN();
        incrTotalCBeaconPckTx_by_HN();

        txCBEACONEnabled = false;

        state_wait_PROBE();

    } else if (cmh->ptype() == PT_RTS_UFETCH) {
        //RTS section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of the HN has finished"
                << " to transmit a RTS packet to the AUV with MAC address: " << mac_addr_AUV_in_trigger << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndTx()________________________End_the_tx_of_RTS_pck_to_the_AUV(" << mac_addr_AUV_in_trigger << ")." << std::endl;

        tx_RTS_finish_HN_time = NOW;

        incrRtsPckTx_by_HN();
        incrTotalRtsPckTx_by_HN();

        txRTSEnabled = false;

        state_wait_CTS();

    } else {
        //DATA section
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->Physical layer of the HN has finished"
                << " to transmit a DATA to the AUV with MAC address: " << mac_addr_AUV_in_CTS << std::endl;

        tx_DATA_finish_HN_time = NOW;

        incrDataPckTx_by_HN();
        incrTotalDataPckTx_by_HN();

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndTx()________________________End_the_tx_of_DATA"
                << "_pck_to_the_AUV(" << mac_addr_AUV_in_CTS << ")." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::Phy2MacEndTx()________________________DATA pck"
                << " number " << getDataPckTx_by_HN() << "." << std::endl;

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::Phy2MacEndTx() ---->GET DATA PCK TX BY HN " << getDataPckTx_by_HN()
            << " to transmit a DATA to the AUV with MAC address: " << mac_addr_AUV_in_trigger << std::endl;

        if (getDataPckTx_by_HN() < (max_data_HN_can_tx)) {
            DATA_BEFORE_TX_timer.schedule(Tdata_HN_pck);
        } else {
            state_DATA_HN_finish_tx();
        }
    }

} //end Phy2MacEndTx_HN();

/******************************************************************************
 *                              HEAD NODE                                     *
 ******************************************************************************/
void uwUFetch_NODE::stateIdle_HN() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN is in IDLE STATE" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::stateIdle_HN()________________________HN_is_in_IDLE_STATE" << std::endl;
    /**
     * Before to enter in idle state and wait a trigger packet from the HN, or
     * before to start the communication with sensor nodes, HN empty his data queue
     */
    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN before to start the communication with"
            << " sensor node or AUV, empty his data queue." << std::endl;

    /**
     * Before to start the communication between AUV, HN move the data packets from the SN queue to the HN 
     * data queue. This operation is done only by the HN that function also as SN
     */
    int index_q = 0;
    while ((!Q_data.empty()) && (index_q <= MAX_PCK_HN_WANT_RX_FROM_NODE)) {
        //Pick up the first element of the queue
        curr_DATA_NODE_pck_tx_HN = (Q_data.front())->copy();
        //Remove the element from the queue that we have pick up the packet
        Q_data.pop();
        //Add the packet to the queue of data packets that HN will transmit to AUV when 
        //will be required
        Q_data_HN.push(curr_DATA_NODE_pck_tx_HN);
        Q_data_source_SN.push(addr); //Save the MAC address from which the data packet it's arrived.
        incrTotalDataPckTx_by_NODE();
        incrTotalDataPckRx_by_HN();

        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_NODE_pck_tx_HN);
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::stateIdle_HN()________________________DATA_pck_rx:_source(" << addr << ")_"
                << "destination(" << addr << ")_id_pck:" << cbrh->sn() << "" << std::endl;
        index_q++;
    }

    refreshState(UWUFETCH_NODE_STATUS_IDLE);
    if (print_transitions) printStateInfo();

    rxTRIGGEREnabled = true;

    if ((last_reason == UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX_BUT_HN_NO_DATA_TO_TX)
            || (last_reason == UWUFETCH_NODE_STATUS_CHANGE_HN_TX_ALL_DATA_TO_AUV)
            || (last_reason == UWUFETCH_NODE_STATUS_CHANGE_HN_RX_TRIGGER_FROM_AUV_CORRUPTED)
            || (last_reason == UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_CTS_EXPIRED)) {
        if (last_reason == UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX_BUT_HN_NO_DATA_TO_TX) {
            /*  if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN has analyzed the TRIGGER,"
                      << " but it hasn't DATA to transmit, so"
                      << " start immediately the communication with SENSOR NODES." << std::endl;*/
        } else if (last_reason == UWUFETCH_NODE_STATUS_CHANGE_HN_TX_ALL_DATA_TO_AUV) {
            /*  if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN has transmitted all his"
                      << " DATA packets to the AUV, so start immediately the communication with SENSOR NODES." << std::endl;*/
        } else if (last_reason == UWUFETCH_NODE_STATUS_CHANGE_HN_RX_TRIGGER_FROM_AUV_CORRUPTED) {
            /* if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN has analyzed the TRIGGER,"
                     << " but it's corrupted, so start immediately the communication with SENSOR NODES." << std::endl;*/
        } else if (last_reason == UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_CTS_EXPIRED) {
            /*  if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN has analyzed the TRIGGER,"
                      << " has transmitted a CTS packet but it has never received a CTS packet, so start immediately"
                      << " the communication with SENSOR NODES." << std::endl;*/
        }

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN before to start the communication"
                << " with SENSOR NODES wait an interval time equal to: " << 15 << "[s]." << std::endl;

        CTSrx = false;

        txBEACONEnabled = true;
        rxTRIGGEREnabled = false;

        BeaconBeforeTx_timer.schedule(15);

    } else {
        if ((!Q_data_HN.empty()) && (Q_data_HN.size() >= 1)) {
            /**
             * Time interval that will expire before that the HN start the initialization 
             * of the BEACON packet and its transmission
             * 
             */
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN waits a TRIGGER packets from the AUV"
                    << " for an interval time equal to: " << T_START_PROCEDURE_HN_NODE << " [s] before to start the communication"
                    << "  with the SENSOR NODEs." << std::endl;

            BeaconBeforeTx_timer.schedule(T_START_PROCEDURE_HN_NODE);

        } else {
            /**
             * Time interval that will expire before that the HN start the initialization 
             * of the BEACON packet and its transmission
             * 
             */
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::stateIdle_HN() ---->HN waits a TRIGGER packets from the AUV"
                    << " for an interval time equal to: " << T_START_PROCEDURE_HN_NODE << "[s] before to start the communication"
                    << " with the SENSOR NODEs." << std::endl;

            BeaconBeforeTx_timer.schedule(T_START_PROCEDURE_HN_NODE);
        }
    }
} //end stateIdle_HN();

void uwUFetch_NODE::BeaconTxTOExpire() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::BeaconTxTOExpire() ---->HN start the procedure for communicate"
            << " with the SENSOR NODEs, reception of TRIGGER packet is not enabled. " << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::BeaconTxTOExpire()____________________HN_start_communication_with_SNs." << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_BEACON_TO_EXPIRED);

    CTSrx = false;

    txBEACONEnabled = true;
    rxTRIGGEREnabled = false;

    /**
     * Before to start the communication between AUV, HN move the data packets from the SN queue to the HN 
     * data queue. This operation is done only by the HN that function also as SN
     */
    int index_q = 0;
    while ((!Q_data.empty()) && (index_q <= MAX_PCK_HN_WANT_RX_FROM_NODE)) {
        //Pick up the first element of the queue
        curr_DATA_NODE_pck_tx_HN = (Q_data.front())->copy();
        //Remove the element from the queue that we have pick up the packet
        Q_data.pop();
        //Add the packet to the queue of data packets that HN will transmit to AUV when 
        //will be required
        Q_data_HN.push(curr_DATA_NODE_pck_tx_HN);
        Q_data_source_SN.push(addr); //Save the MAC address from which the data packet it's arrived.
        incrTotalDataPckTx_by_NODE();
        incrTotalDataPckRx_by_HN();

        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_NODE_pck_tx_HN);
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::BeaconTxTOExpire()____________________DATA_pck_rx:_source(" << addr << ")_"
                << "destination(" << addr << ")_id_pck:" << cbrh->sn() << "" << std::endl;

        index_q++;
    }

    state_BEACON_tx();
} //end BeaconTxTOExpire();

void uwUFetch_NODE::TRIGGER_rx() {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets

        uwUFetch_NODE::TRIGGER_rx_without();
    } else {
        //Communication between HN & AUV with RTS and CTS packets
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
            T_min_bck_RTS = (double) triggerh->t_min() / 1000; //Minimum value time before that HN transmit a RTS packet
            T_max_bck_RTS = (double) triggerh->t_max() / 1000; //Maximum value time before that HN transmit a RTS packet
            max_pck_HN_can_tx = triggerh->max_pck_want_rx(); //Maximum number of DATA packet that HN can transmit to the AUV   

            mac_addr_AUV_in_trigger = mach->macSA(); //Save the MAC address of the AUV node from which the HN has received a trigger packet

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________PARAMETERS_of_TRIGGER_PACKET: " << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Source_address:_" << mach->macSA() << "." << std::endl;
            if (mach->macDA() == -1) {
                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Destination_Address:_BROADCAST." << std::endl;
            } else {
                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Destination_Address:_" << mach->macDA() << std::endl;
            }
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________MIN_backoff_time_RTS_pck:_" << T_min_bck_RTS << "[s]." << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________MAX_backoff_time_RTS_pck:_" << T_max_bck_RTS << "[s]." << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________MAX_DATA_pck_want_rx_from_HN:_" << max_pck_HN_can_tx << "[pck]." << std::endl;

            rxTRIGGEREnabled = false;
            txBEACONEnabled = false;

            bck_before_tx_RTS = choiceBackOffTimer_HN();
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Backoff_timeout_choice:_" << bck_before_tx_RTS << "[s]." << std::endl;
            BCK_timer_rts.schedule(bck_before_tx_RTS);

        } else {
            //HN is not enable to receive a TRIGGER packet
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::TRIGGER_rx() ---->HN is not enabled to receive"
                    << " the TRIGGER packet: DROP IT." << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()_________________________TRIGGER_rx_It_s_CORRECT_and_>= 1_DATA_to_tx_and_not_enabled_to_rx_It:IGNORE_It." << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::TRIGGER_rx()__________________________Not_ENABLED"
                    << "_to_rx_TRIGGER_pck." << std::endl;
            drop(curr_TRIGGER_HN_pck_rx, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);
        }
    }
} //end TRIGGER_rx();

void uwUFetch_NODE::BCKTOExpired_HN() {
    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_NODE::BCKTOExpired_HN_without();
    } else {
        //Communication between HN & AUV with RTS and CTS packets
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::BCKTOExpired() ---->Back-off timeout is expired:"
                << " so the first DATA packet can be transmitted by the HN to the AUV." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::BCKTOExpired()________________________Backoff_timeout_expired." << std::endl;

        state_RTS_tx();
    }
} //end BCKTOExpired();

void uwUFetch_NODE::state_RTS_tx() {
    //Transmit a RTS packet to the AUV
    if (!Q_data_HN.empty()) {
        //Queue is not empty so HN has at least one DATA packet to transmit to the AUV  
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_RTS_tx() ---->HN with MAC address " << addr << " create"
                << " a RTS packet that will then transmit to the AUV with MAC address: " << mac_addr_AUV_in_trigger << std::endl;

        refreshState(UWUFETCH_NODE_STATUS_RTS_TRANSMIT);
        if (print_transitions) printStateInfo();

        Packet* p = Packet::alloc();
        hdr_cmn* cmh = hdr_cmn::access(p);
        hdr_mac* mach = HDR_MAC(p);
        hdr_RTS_UFETCH* rtsh = HDR_RTS_UFETCH(p);

        cmh->ptype() = PT_RTS_UFETCH;
        cmh->size() = sizeof (hdr_RTS_UFETCH);

        mach->set(MF_CONTROL, addr, mac_addr_AUV_in_trigger);
        mach->macSA() = addr;
        mach->macDA() = mac_addr_AUV_in_trigger;

        //Filling the HEADER of the RTS packet 
        rtsh->num_DATA_pcks() = Q_data_HN.size(); //Number of packets that HN want to transmit at the AUV
        rtsh->backoff_time_RTS() = (int) (bck_before_tx_RTS * 1000); //Back-off timer choice before to transmit a RTS packet

        curr_RTS_HN_pck_tx = p->copy();

        txRTSEnabled = true;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________PARAMETERS_of_RTS_PACKET. " << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________Source_address:_" << mach->macSA() << "." << std::endl;
        if (mach->macDA() == -1) {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________Destination_Address:_BROADCAST." << std::endl;
        } else {
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________Destination_Address:_" << mac_addr_AUV_in_trigger << "." << std::endl;
        }
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________RTS_backoff_time_choice_by_HN: " << bck_before_tx_RTS << "[s]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________Number_of_pck_HN_would_like_to_tx:_" << rtsh->num_DATA_pcks() << "[pck]." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________Size_of_RTS_pck:_" << cmh->size() << "[byte]." << std::endl;

        Packet::free(p);

        RTS_tx();

    } else {
        //QUEUE is empty, so the HN doesn't transmit a RTS packet to the HN.
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_RTS_tx() ----> HN with MAC address" << addr << " has no DATA"
                << " packet to be transmitted to the HN, then ignore the TRIGGER received from the AUV with"
                << " MAC address: " << mac_addr_AUV_in_trigger << ", and return in IDLE STATE." << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_RTS_tx()________________________0_DATA_pck_to_tx:_NO_tx_RTS_pck." << std::endl;
        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_TRIGGER_RX_BUT_HN_NO_DATA_TO_TX);

        n_TRIGGER_pck_rx_by_HN = 0;
        n_RTS_pck_tx_by_HN = 0;
        n_CTS_pck_rx_by_HN = 0;
        n_DATA_pck_tx_by_HN = 0;

        txBEACONEnabled = true;
        rxTRIGGEREnabled = true;
        txRTSEnabled = false;

        stateIdle_HN();
    }
} //end state_RTS_tx();

void uwUFetch_NODE::RTS_tx() {
    //HN is enabled to transmit RTS packet
    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::RTS_tx() ---->HN with MAC address: " << addr << " is transmitting"
            << " a RTS packet to the AUV with MAC address: " << mac_addr_AUV_in_trigger << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_RTS_TX);

    Mac2PhyStartTx(curr_RTS_HN_pck_tx);
} //end RTS_tx();

void uwUFetch_NODE::state_wait_CTS() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_CTS() ---->HN with MAC address: " << addr << " is"
            << " waiting the CTS packet from AUV with MAC address: " << mac_addr_AUV_in_trigger << ". The maximum"
            << " waiting time is: " << T_CTS << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_WAIT_CTS_PACKET);
    if (print_transitions) printStateInfo();

    if (T_CTS < 0) {
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_CTS() ---->CTS timer insert by the user for waiting a CTS"
                << " packet is negative, so use the default value 20[s]" << std::endl;
        T_CTS = 20;
    }

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_CTS()______________________Waiting_a_CTS_pck_from_AUV(" << mac_addr_AUV_in_trigger << ")." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_CTS()______________________.......Waiting_CTS_pck." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_CTS()______________________.......Waiting_CTS_pck." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_CTS()______________________.......Waiting_CTS_pck." << std::endl;

    rxCTSEnabled = true; //enable HN to receive CTS packet

    CTS_timer.schedule(T_CTS);
} //end state_wait_CTS();

void uwUFetch_NODE::CtsTOExpired() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CtsTOExpired() ---->Timeout CTS expired. HN has not receive"
            << " CTS packet within the pre-established time interval that is equal to: " << T_CTS << " [s]."
            << " so return in IDLE STATE." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::CtsTOExpired()________________________CTS_timeout_expired." << std::endl;
    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_TO_WAIT_CTS_EXPIRED);

    n_TRIGGER_pck_rx_by_HN = 0;
    n_RTS_pck_tx_by_HN = 0;
    n_CTS_pck_rx_by_HN = 0;

    txBEACONEnabled = true;
    rxPROBEEnabled = false;
    txPOLLEnabled = false;
    rxDATAEnabled = false;
    txCBEACONEnabled = false;
    rxTRIGGEREnabled = true;
    txRTSEnabled = false;
    rxCTSEnabled = false;
    txDATAEnabledHN = false;

    CTSrx = true;
    stateIdle_HN();
} //end CtsTOExpired();

void uwUFetch_NODE::CTS_rx() {

    CTSrx = true;
    refreshState(UWUFETCH_NODE_STATUS_CTS_RECEIVE);
    if (print_transitions) printStateInfo();

    hdr_mac* mach = HDR_MAC(curr_CTS_HN_pck_rx);
    hdr_CTS_UFETCH* ctsh = HDR_CTS_UFETCH(curr_CTS_HN_pck_rx);

    /*
     * Save the header data of the CTS
     */
    max_data_HN_can_tx = ctsh->num_DATA_pcks_MAX_rx(); //Maximum number of DATA packets that the HN can transmit to the AUV

    mac_addr_AUV_in_CTS = mach->macSA(); //MAC address of the AUV node from which the HN has received a CTS packet

    rxCTSEnabled = false;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::CTS_rx()______________________________PARAMETERS_of_CTS_PACKET." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::CTS_rx()______________________________Source_address:_" << mach->macSA() << "." << std::endl;
    if (mach->macDA() == -1) {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::CTS_rx()______________________________Destination_Address:_BROADCAST." << std::endl;
    } else {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::CTS_rx()______________________________Destination_Address:_" << mach->macDA() << "." << std::endl;
    }
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::CTS_rx()______________________________DATA_pck_want_rx_from_HN: " << max_data_HN_can_tx << "[pck]." << std::endl;

    state_DATA_HN_first_tx();

    CTSrx = true;
} //end CTS_rx();

void uwUFetch_NODE::state_DATA_HN_first_tx() {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_NODE::state_DATA_HN_first_tx_without();
    } else {
        //Communication between HN & AUV with RTS and CTS packets
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_first_tx() ---->HN will be transmit " << max_data_HN_can_tx
                << " DATA packets to the AUV with MAC address: " << mac_addr_AUV_in_CTS << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_first_tx()______________DATA_pck_to_tx_"
                << max_data_HN_can_tx << "_to_the_AUV(" << mac_addr_AUV_in_CTS << ")." << std::endl;

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_first_tx() ---->HN is starting transmission of the"
                << " FIRST DATA packet to the AUV with MAC address: " << mac_addr_AUV_in_CTS << std::endl;

        //Pick up the first element of the queue
        curr_DATA_HN_pck_tx = (Q_data_HN.front())->copy();
        //Remove the element from the queue that we have pick up 
        Q_data_HN.pop();

        hdr_mac* mach = HDR_MAC(curr_DATA_HN_pck_tx);
        hdr_cmn* cmh = hdr_cmn::access(curr_DATA_HN_pck_tx);
        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_HN_pck_tx);

        mach->set(MF_CONTROL, addr, mac_addr_AUV_in_CTS);
        mach->macSA() = addr;
        mach->macDA() = mac_addr_AUV_in_CTS;
        cmh->size() = sizeof (curr_DATA_HN_pck_tx);

        if (burstDATA()) {
            // hdr_uwmphy_modem* modemh = HDR_UWMPHY_MODEM(curr_DATA_HN_pck_tx);
            // modemh->use_burst_data() = 1;
            //            if (getDataPckTx_by_HN() == (max_data_HN_can_tx)) {
            //                modemh->last_packet() = 1;
            //                modemh->use_burst_data() = 1;
            //            } else {
            //                modemh->last_packet() = 0;
            //                modemh->use_burst_data() = 1;
            //            }

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
    }
} //end state_DATA_HN_first_tx();

void uwUFetch_NODE::DATA_HN_tx() {
    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_NODE::DATA_HN_tx_without();
    } else {
        //Communication between HN & AUV with RTS and CTS packets
        //HN is enabled to transmit DATA packet
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_HN_tx() ---->HN is transmitting a DATA packet to the AUV"
                << " with MAC address: " << mac_addr_AUV_in_CTS << std::endl;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_PCK_TX);

        Mac2PhyStartTx(curr_DATA_HN_pck_tx);
    }
} //end DATA_HN_tx();

void uwUFetch_NODE::DataBeforeTxTOExpired_HN() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DataBeforeTxTOExpired() ---->HN can transmit the next"
            << " DATA packet." << std::endl;

    state_DATA_HN_tx();

} //end DataBeforeTxTOExpired();

void uwUFetch_NODE::state_DATA_HN_tx() {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_NODE::state_DATA_HN_tx_without();
    } else {
        //Communication between HN & AUV with RTS and CTS packets
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_tx() ---->HN is starting transmission of the"
                << " DATA packet number: " << (getDataPckTx_by_HN() + 1) << " to the AUV with"
            << " MAC address: " << mac_addr_AUV_in_CTS << std::endl;

        //Pick up the first element of the queue
        curr_DATA_HN_pck_tx = (Q_data_HN.front())->copy();
        //Remove the element from the queue that we have pick up 
        Q_data_HN.pop();

        hdr_mac* mach = HDR_MAC(curr_DATA_HN_pck_tx);
        hdr_cmn* cmh = hdr_cmn::access(curr_DATA_HN_pck_tx);
        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_HN_pck_tx);

        mach->set(MF_CONTROL, addr, mac_addr_AUV_in_CTS);
        mach->macSA() = addr;
        mach->macDA() = mac_addr_AUV_in_CTS;
        cmh->size() = sizeof (curr_DATA_HN_pck_tx);

        if (burstDATA()) {
            //            hdr_uwmphy_modem* modemh = HDR_UWMPHY_MODEM(curr_DATA_HN_pck_tx);
            //
            //            if (getDataPckTx_by_HN() == (max_data_HN_can_tx)) {
            //                modemh->last_packet() = 1;
            //                modemh->use_burst_data() = 1;
            //            } else {
            //                modemh->last_packet() = 0;
            //                modemh->use_burst_data() = 1;
            //            }

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

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_tx()____________________DATA_pck_tx:_source_SN(" << Q_data_source_SN.front() << ")_source_HN(" << mach->macSA() << ")_"
                << "destination(" << mach->macDA() << ")_id_pck_original:" << old_sn << "_new_id_pck:" << cbrh->sn() << "_size:" << cmh->size() << "[byte]" << std::endl;

            Q_data_source_SN.pop();
            DATA_HN_tx();
        }
    }
} //end state_DATA_HN_tx();

void uwUFetch_NODE::state_DATA_HN_finish_tx() {

    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        uwUFetch_NODE::state_DATA_HN_finish_tx_without();
    } else {
        //Communication between HN & AUV with RTS and CTS packets
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_DATA_HN_finish_tx()_____________End_tx_DATA_pcks_to_the_AUV." << std::endl;
        if (sectionCBeacon) {
            //HN has start the communication with AUV before to start the transmission of CBEACON packet, so after the end of the communication
            //with AUV, HN return in state_CBEACON_tx().
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_DATA_HN_finish_tx() ---->HN has transmitted all his DATA"
                    << " packets to the AUV with MAC address: " << mac_addr_AUV_in_CTS << ", so return in STATE_CBEACON_TX()." << std::endl;

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
                    << " packets to the AUV with MAC address: " << mac_addr_AUV_in_CTS << ", so return in IDLE STATE." << std::endl;
            refreshReason(UWUFETCH_NODE_STATUS_CHANGE_HN_TX_ALL_DATA_TO_AUV);
            n_TRIGGER_pck_rx_by_HN = 0;
            n_DATA_pck_tx_by_HN = 0;

            txBEACONEnabled = true;
            rxTRIGGEREnabled = true;
            txDATAEnabledHN = false;

            stateIdle_HN();
        }
    }
} //end state_DATA_HN_finish_tx();

void uwUFetch_NODE::state_BEACON_tx() {

    while (!Q_probbed_mac_HN.empty()) {
        Q_probbed_mac_HN.pop();
        Q_probbed_n_pcks_NODE_want_tx_HN.pop();
        Q_probbed_backoff_time.pop();
    }

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_BEACON_tx() ---->HN is initializing the BEACON packet"
            << " which will then transmit to the NODEs in broadcast." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________Initializing_BEACON_pck." << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_TRANSMIT_BEACON);
    if (print_transitions) printStateInfo();

    Packet* p = Packet::alloc(); //allocate the memory
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    hdr_BEACON_UFETCH* beaconh = HDR_BEACON_UFETCH(p);

    cmh->ptype() = PT_BEACON_UFETCH; //type of the packet

    mach->set(MF_CONTROL, addr, MAC_BROADCAST); //set the information of the packet (type,source address,destination address)
    mach->macSA() = addr; //set the source address as AUV MAC address
    mach->macDA() = MAC_BROADCAST; //set the destination address as BROADCAST address

    beaconh->t_min_bc() = (int) (T_MIN_BACKOFF_PROBE * 1000); //minimum value for to choose the back-off time for tx PROBE pck
    beaconh->t_max_bc() = (int) (T_MAX_BACKOFF_PROBE * 1000); //maximum value for to choose the back-off time for tx PROBE pck
    beaconh->num_Max_CBEACON_tx_by_HN() = MAX_ALLOWED_CBEACON_TX; //maximum number of CBEACON tx by this HN
    cmh->size() = sizeof (hdr_BEACON_UFETCH); //set the size of BEACON header packet

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________PARAMETERS_of_BEACON_pck." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________Source_address:_" << mach->macSA() << "." << std::endl;
    if (mach->macDA() == -1) {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________Destination_address:_BROADCAST." << std::endl;
    } else {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________Destination_address:_" << mach->macDA() << "." << std::endl;
    }
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________MIN_backoff_time_PROBE_pck:_" << T_MIN_BACKOFF_PROBE << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________MAX_backoff_time_PROBE_pck: " << T_MAX_BACKOFF_PROBE << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________MAX_number_of_CBEACON_that_HN_will_tx:_" << beaconh->num_Max_CBEACON_tx_by_HN() << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_BEACON_tx()_____________________Size_of_BEACON_pck:_" << cmh->size() << "[byte]." << std::endl;

    curr_BEACON_HN_pck_tx = p->copy(); //create a copy of the BEACON packet

    Packet::free(p); //de-allocate the memory

    BEACON_tx(); //recall the function that simulate the transmission of the BEACON

} //end state_BEACON_tx();

void uwUFetch_NODE::BEACON_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::BEACON_tx() ---->HN is transmitting a BEACON packet"
            << " to the NODEs in broadcast." << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_BEACON_TX); //specify the reason that the protocol state change

    Mac2PhyStartTx(curr_BEACON_HN_pck_tx);

} //end BEACON_tx();

void uwUFetch_NODE::state_wait_PROBE() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_PROBE_wait() ---->HN is scheduling PROBE TIMER T= "
            << T_PROBE << "[s]" << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_WAIT_PROBE_PACKET);
    if (print_transitions) printStateInfo();

    if (T_PROBE < 0) {
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_PROBE() ---->The waiting time for PROBE packets is negative"
                << " so set the default value: 15[s]." << std::endl;
        T_PROBE = 15;
    }
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_PROBE()____________________Waiting_PROBE_pck from"
            "_SNs." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_PROBE()____________________Timeout_within_rx_PROBE"
            << "_pcks_" << T_PROBE << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_PROBE()____________________....Waiting_PROBE" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_PROBE()____________________....Waiting_PROBE" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_PROBE()____________________....Waiting_PROBE" << std::endl;

    rxPROBEEnabled = true;

    PROBE_timer.schedule(T_PROBE);

} //end state_wait_PROBE();

void uwUFetch_NODE::ProbeTOExpired() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::ProbeTOExpired() ---->PROBE timeout is expired: other"
            << " PROBE packets can not be received by the HN." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::ProbeTOExpired()______________________PROBE_timeout_expired." << std::endl;

    if ((Q_probbed_mac_HN.empty()) && (getCBeaconPckTx_by_HN() == MAX_ALLOWED_CBEACON_TX)) {

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::ProbeTOExpired() ---->HN has received 0 PROBE packets and"
                << " it has reached the maximum number of attempts to transmit CBEACON packets." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::ProbeTOExpired()______________________0_PROBE_rx_and_MAX_CBEACON_tx." << std::endl;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_CBEACON_MAX_ALLOWED_TX_0_PROBE_RX);

        n_BEACON_pck_tx_by_HN = 0; //reset the number of BEACON packet transmitted by the HN
        n_PROBE_pck_rx_by_HN = 0; //reset the number of PROBE packet received by the HN  
        n_POLL_pck_tx_by_HN = 0; //reset the number of POLL packet transmitted by the HN
        n_DATA_pck_rx_by_HN = 0; //reset the number of DATA packet received by the HN
        n_CBEACON_pck_tx_by_HN = 0; //reset the number of CBEACON packet transmitted by the HN

        rxPROBEEnabled = false;
        txBEACONEnabled = true;
        rxTRIGGEREnabled = true;

        stateIdle_HN();

    } else if ((Q_probbed_mac_HN.empty()) && (getCBeaconPckTx_by_HN() < MAX_ALLOWED_CBEACON_TX)) {

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::ProbeTOExpired() ---->HN has received 0 PROBE packet and it"
                << " has not reached the maximum number of attempts to transmit CBEACON packets"
                << " that is: " << MAX_ALLOWED_CBEACON_TX << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::ProbeTOExpired()______________________0_PROBE_rx_and_another_CBEACON_can_be_tx." << std::endl;

        n_PROBE_pck_rx_by_HN = 0; //reset the number of PROBE packet received by the HN  
        n_POLL_pck_tx_by_HN = 0; //reset the number of POLL packet transmitted by the HN
        n_DATA_pck_rx_by_HN = 0; //reset the number of DATA packet received by the HN

        rxPROBEEnabled = false;
        txCBEACONEnabled = true;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_CBEACON_ALLOWED_TX_0_PROBE_RX);

        state_CBEACON_tx();

    } else {

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::ProbeTOExpired() ---->HN has received "
                << (Q_probbed_mac_HN.size()) << " PROBE packets, so start a new transmission of POLL packet." << std::endl;

        n_DATA_pck_rx_by_HN = 0; //reset the number of DATA packet received by the HN

        rxPROBEEnabled = false;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_TO_EXPIRED_AT_LEAST_1_PROBE_RX);

        state_POLL_tx();
    }
} //end ProbeTOExpire();

void uwUFetch_NODE::PROBE_rx() {
    //HN is enabled to receive a PROBE packet

    hdr_mac* mach = HDR_MAC(curr_PROBE_HN_pck_rx);
    hdr_PROBE_UFETCH* probeh = HDR_PROBE_UFETCH(curr_PROBE_HN_pck_rx);

    refreshState(UWUFETCH_NODE_STATUS_PROBE_RX);
    if (print_transitions) printStateInfo();

    /**
     *  Save the information of the NODE that has sent the PROBE packet correctly 
     *  to the HN
     *
     */
    Q_probbed_mac_HN.push(mach->macSA()); //store the MAC address of the NODE that
    //has sent the PROBE packet to the HN
    Q_probbed_n_pcks_NODE_want_tx_HN.push(probeh->n_DATA_pcks_Node_tx()); //store the number of packets that the NODE want to tx at
    //the HN
    Q_probbed_backoff_time.push((double) probeh->backoff_time_PROBE() / 1000); //store the back-off time choice by the NODE before 
    //to transmit the PROBE packet

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::PROBE_rx()____________________________PROBE_pck_rx_number:_"
            << getProbePckRx_HN() << "." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::PROBE_rx()____________________________PARAMETERS_OF_PROBE_PCK." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::PROBE_rx()____________________________Source_address:_" << mach->macSA() << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::PROBE_rx()____________________________Destination_address:_" << mach->macDA() << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::PROBE_rx()____________________________DATA_pck_would_be_tx_SN:_" << probeh->n_DATA_pcks_Node_tx() << "[pck]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::PROBE_rx()____________________________Backoff_time_choice_before_tx_PROBE:_" << (double) probeh->backoff_time_PROBE() / 1000 << "[s]." << std::endl;

    Packet::free(curr_PROBE_HN_pck_rx);

    if (Q_probbed_mac_HN.size() == MAX_POLLED_NODE) {
        //MAXIMUM number of PROBE packet is received by the HN

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::PROBE_Rx() ---->HN has received the maximum number of PROBE"
                << " packets and the timeout is not expired. Consequence of that, the HN with MAC address: " << addr << " can"
                << " not receive other PROBEs packets." << std::endl;

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::PROBE_Rx() ---->HN reset the PROBE timer"
                << " and start the transmission of POLL packets." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::PROBE_rx()____________________________MAX_number_of_PROBE_pck_rx:_reset_PROBE_timeout." << std::endl;

        PROBE_timer.force_cancel();

        rxPROBEEnabled = false;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_MAX_PROBE_RX_PROBE_TO_NOT_EXPIRED);

        state_POLL_tx();

    } else {
        //Timeout is not expire and thus wait other PROBE packets
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::PROBE_Rx() ---->Timeout is not expired and another PROBE"
                << " packet can be received by the HN with MAC address: " << addr << " from other NODEs." << std::endl;
        state_wait_other_PROBE();
    }
} //end PROBE_rx();

void uwUFetch_NODE::state_wait_other_PROBE() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_other_PROBE() ---->HN is waiting the reception of"
            << " PROBE packet number: " << (Q_probbed_mac_HN.size() + 1) << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_other_PROBE()______________....Waiting_PROBE." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_other_PROBE()______________....Waiting_PROBE." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_other_PROBE()______________....Waiting_PROBE." << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_WAIT_PROBE_PACKET);
    if (print_transitions) printStateInfo();

} //end state_wait_other_PROBE();

void uwUFetch_NODE::state_POLL_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_POLL_tx() ---->HN is creating a POLL packet that"
            << " will then transmit at the first node of the queue that contain the PROBEs received. " << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_POLL_TX);
    if (print_transitions) printStateInfo();

    Packet* p = Packet::alloc();
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    hdr_POLL_UFETCH* pollh = HDR_POLL_UFETCH(p);

    cmh->ptype() = PT_POLL_UFETCH;
    cmh->size() = sizeof (hdr_POLL_UFETCH);

    mach->set(MF_CONTROL, addr, Q_probbed_mac_HN.front());
    mach->macSA() = addr;
    mach->macDA() = Q_probbed_mac_HN.front();

    //Filling the HEADER of the POLL packet
    if (Q_probbed_n_pcks_NODE_want_tx_HN.front() <= MAX_PCK_HN_WANT_RX_FROM_NODE) {
        pollh->num_DATA_pcks_MAX_rx() = Q_probbed_n_pcks_NODE_want_tx_HN.front(); //Maximum number of DATA packets that the HN want to 
        //receive from the NODE that is being to poll
    } else {
        pollh->num_DATA_pcks_MAX_rx() = MAX_PCK_HN_WANT_RX_FROM_NODE; //Maximum numbero of DATA packets that the HN want to 
        //receive from the NODE that is being to poll 
    }
    number_data_pck_HN_rx_exact = pollh->num_DATA_pcks_MAX_rx();
    pollh->mac_addr_Node_polled() = Q_probbed_mac_HN.front(); //Mac address of the NODE that the HN is being to poll
    mac_addr_NODE_polled = Q_probbed_mac_HN.front(); //Store the mac address of the NODE that the HN is being
    //to poll

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_POLL_tx()_______________________PARAMETERS_OF_POLL_PCK:" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_POLL_tx()_______________________Source_address:_" << mach->macSA() << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_POLL_tx()_______________________Destination_address:_" << mach->macDA() << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_POLL_tx()_______________________DATA_pck_want_rx_from_SN:_" << number_data_pck_HN_rx_exact << "[pck]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_POLL_tx()_______________________Size_of_POLL_pck:_" << cmh->size() << "[byte]." << std::endl;

    curr_POLL_HN_pck_tx = p->copy();

    Packet::free(p); //de-allocate the memory

    txPOLLEnabled = true;

    POLL_tx();
} //end state_POLL_tx();

void uwUFetch_NODE::POLL_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::POLL_tx() ---->HN is transmitting a POLL packet to the NODE with MAC"
            << " address: " << mac_addr_NODE_polled << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_POLL_TX); //specify the reason that the protocol state change

    computeTxTime_HN(UWUFETCH_NODE_PACKET_TYPE_POLL);

    Mac2PhyStartTx(curr_POLL_HN_pck_tx);

} //end POLL_tx();

void uwUFetch_NODE::state_wait_first_DATA() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_first_DATA() ---->HN is waiting to receive the FIRST DATA"
            << " packet from the NODE with MAC address: " << mac_addr_NODE_polled << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_WAIT_DATA_NODE);
    if (print_transitions) printStateInfo();

    data_timeout = getDataTimerValue(); //Compute the timeout interval within HN want to receive all DATA packets from NODE 

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_first_DATA() ---->HN want to receive all the DATA packets"
            << " from the NODE with MAC address: " << mac_addr_NODE_polled << " within "
            << " an interval time: " << data_timeout << "[s]." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_first_DATA()_______________Timeout_within_rx_all_DATA"
            << "_pcks: " << data_timeout << "[s]." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_first_DATA()_______________.......Waiting_DATA" << std::endl;

    rxDATAEnabled = true;

    DATA_timer.schedule(data_timeout);
} //end state_wait_first_DATA();

void uwUFetch_NODE::state_wait_other_DATA() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_wait_other_DATA() ---->HN is waiting the DATA packet number: "
            << (getDataPckRx_by_HN() + 1) << " from the NODE with MAC address: " << mac_addr_NODE_polled << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_other_DATA()_______________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_other_DATA()_______________.......Waiting_DATA" << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_wait_other_DATA()_______________.......Waiting_DATA" << std::endl;

    refreshState(UWUFETCH_NODE_STATUS_WAIT_DATA_NODE);
    if (print_transitions) printStateInfo();

    txBEACONEnabled = false;
    rxPROBEEnabled = false;
    txPOLLEnabled = false;
    rxDATAEnabled = true;
    txCBEACONEnabled = false;
    rxTRIGGEREnabled = false;
    txRTSEnabled = false;
    rxCTSEnabled = false;
    txDATAEnabledHN = false;

} //end state_wait_other_DATA();

void uwUFetch_NODE::DataTOExpired() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DataTOExpired() ---->HN has received from the NODE with MAC"
            << " address: " << mac_addr_NODE_polled << " only " << getDataPckRx_by_HN() << " packets." << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DataTOExpired()_______________________DATA_timeout_expired." << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_TO_EXPIRED); //Refresh the reason for the changing of the state

    rxDATAEnabled = false;
    updateListProbbedNode(); //Remove the NODE that has already transmit his data packet

    //Verify if another node can be polled, or the HN will start the transmission of CBEACON
    if ((Q_probbed_mac_HN.empty()) && (getCBeaconPckTx_by_HN() == MAX_ALLOWED_CBEACON_TX)) {
        //There aren't another node to poll, and the maximum number of CBEACONS are transmitted
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DataTOExpired() ---->HN has not other nodes to POLL, and"
                << " a CBEACON packet can not be transmitted: return in IDLE STATE." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DataTOExpired()_______________________0_SN_to_polled_and_MAX_number_CBEACON_tx." << std::endl;

        n_BEACON_pck_tx_by_HN = 0;
        n_PROBE_pck_rx_by_HN = 0;
        n_POLL_pck_tx_by_HN = 0;
        n_DATA_pck_rx_by_HN = 0;
        n_CBEACON_pck_tx_by_HN = 0;

        txBEACONEnabled = true;
        rxPROBEEnabled = false;
        txPOLLEnabled = false;
        rxDATAEnabled = false;
        txCBEACONEnabled = false;
        rxTRIGGEREnabled = true;
        txRTSEnabled = false;
        rxCTSEnabled = false;
        txDATAEnabledHN = false;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_NO_OTHER_CBEACON);

        stateIdle_HN();

    } else if ((Q_probbed_mac_HN.empty()) && (getCBeaconPckTx_by_HN() < MAX_ALLOWED_CBEACON_TX)) {
        //There aren't another node to poll, and the maximum number of CBEACONS are not transmitted
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DataTOExpired() ---->HN has not other nodes to POLL, but"
                << " a CBEACON packet can be transmitted: transmit a CBEACON" << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DataTOExpired()_______________________0_SN_to_polled_and_another_CBEACON_can_be_tx." << std::endl;

        n_PROBE_pck_rx_by_HN = 0;
        n_POLL_pck_tx_by_HN = 0;
        n_DATA_pck_rx_by_HN = 0;

        txBEACONEnabled = false;
        rxPROBEEnabled = false;
        txPOLLEnabled = false;
        rxDATAEnabled = false;
        txCBEACONEnabled = false;
        rxTRIGGEREnabled = false;
        txRTSEnabled = false;
        rxCTSEnabled = false;
        txDATAEnabledHN = false;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_YES_OTHER_CBEACON);

        state_CBEACON_tx();

    } else {
        //There are another nodes to poll
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DataTOExpired() ---->HN has other nodes to polled." << std::endl;

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DataTOExpired()_______________________Another_SN_can_be_polled." << std::endl;

        n_PROBE_pck_rx_by_HN = 0;
        n_DATA_pck_rx_by_HN = 0;

        txBEACONEnabled = false;
        rxPROBEEnabled = false;
        txPOLLEnabled = false;
        rxDATAEnabled = false;
        txCBEACONEnabled = false;
        rxTRIGGEREnabled = false;
        txRTSEnabled = false;
        rxCTSEnabled = false;
        txDATAEnabledHN = false;

        refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_YES_NODE_TO_POLL);

        state_POLL_tx();
    }
} //end DataTOExpired();

void uwUFetch_NODE::DATA_rx() {

    if (rxDATAEnabled) {

        hdr_mac* mach = HDR_MAC(curr_DATA_HN_pck_rx);

        refreshState(UWUFETCH_NODE_STATUS_DATA_RX);
        if (print_transitions) printStateInfo();

        /**
         *  Save the information of the NODE that has sent the DATA packet correctly 
         *  to the HN
         *
         */
        mac_addr_NODE_in_data = mach->macSA(); //Source MAC address of the NODE that has transmitted the DATA packet to the HN

        hdr_uwcbr* cbrh = HDR_UWCBR(curr_DATA_HN_pck_rx);

        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DATA_rx()_____________________________DATA_pck_rx_by_HN(" << addr << ")_from_SN(" << mac_addr_NODE_in_data << ")_id_pck:" << cbrh->sn() << ""
            << "_It_s_CORRECT." << std::endl;

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_Rx() ---->HN has received the DATA packet"
                << " number: " << getDataPckRx_by_HN() << " from the NODE with"
            << " MAC address: " << mac_addr_NODE_in_data << "." << std::endl;

        //save the data packet in the QUEUE of the HN
        Q_data_HN.push(curr_DATA_HN_pck_rx->copy()); //Save the data packets in the QUEUE of the HN
        Q_data_source_SN.push(mac_addr_NODE_in_data); //Save the MAC address from which the data packet it's arrived.

        Packet::free(curr_DATA_HN_pck_rx);

        if (getDataPckRx_by_HN() == number_data_pck_HN_rx_exact) {
            //Another DATA packet can not be received by the HN
            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_rx() ---->HN has received the LAST DATA packet from NODE"
                    << " with MAC address: " << mac_addr_NODE_in_data << std::endl;

            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DATA_rx()_____________________________All_DATA_pcks"
                    << "_were_rx_from_SN(" << mac_addr_NODE_in_data << ")." << std::endl;

            refreshReason(UWUFETCH_NODE_STATUS_CHANGE_LAST_DATA_PCK_RX);

            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_rx() ---->HN reset DATA timer" << std::endl;
            if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DATA_rx()_____________________________Reset_DATA_Timeout." << std::endl;
            DATA_timer.force_cancel(); //reset the timer

            updateListProbbedNode(); //Remove the NODE that has already transmit his data packet

            //Verify if another node can be polled, or the HN will start the transmission of CBEACON
            if ((Q_probbed_mac_HN.empty()) && (getCBeaconPckTx_by_HN() == MAX_ALLOWED_CBEACON_TX)) {
                //There aren't another node to poll, and the maximum number of CBEACONS are transmitted
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_rx() ---->HN has not other nodes to POLL, and"
                        << " a CBEACON packet can not be transmitted: return in IDLE STATE." << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DATA_rx()_____________________________0_SNs_to_polled_and_MAX_CBEACON_is_tx." << std::endl;

                n_BEACON_pck_tx_by_HN = 0;
                n_PROBE_pck_rx_by_HN = 0;
                n_POLL_pck_tx_by_HN = 0;
                n_DATA_pck_rx_by_HN = 0;
                n_CBEACON_pck_tx_by_HN = 0;

                txBEACONEnabled = true;
                rxDATAEnabled = false;
                rxTRIGGEREnabled = true;

                refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_NO_OTHER_CBEACON);

                stateIdle_HN();

            } else if ((Q_probbed_mac_HN.empty()) && (getCBeaconPckTx_by_HN() < MAX_ALLOWED_CBEACON_TX)) {
                //There aren't another node to poll, and the maximum number of CBEACONS are not transmitted
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_rx() ---->HN has not other nodes to POLL, but"
                        << " a CBEACON packet can be transmitted: transmit a CBEACON" << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DATA_rx()_____________________________0_SNs_to_polled_and_another_CBEACON_can_be_tx." << std::endl;

                n_PROBE_pck_rx_by_HN = 0;
                n_POLL_pck_tx_by_HN = 0;
                n_DATA_pck_rx_by_HN = 0;

                rxDATAEnabled = false;
                txCBEACONEnabled = true;

                refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_0_NODE_TO_POLL_YES_OTHER_CBEACON);

                state_CBEACON_tx();

            } else {
                //There are another nodes to poll
                if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_rx() ---->HN has other nodes to polled." << std::endl;

                if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DATA_rx()_____________________________Another_SNs_can_be_polled." << std::endl;

                n_DATA_pck_rx_by_HN = 0;

                rxDATAEnabled = false;
                txPOLLEnabled = true;

                refreshReason(UWUFETCH_NODE_STATUS_CHANGE_DATA_MAX_ALLOWED_PCK_RX_YES_NODE_TO_POLL);

                state_POLL_tx();
            }

        } else {
            //Another DATA packet can be received by the HN

            if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::DATA_rx() ---->HN wait an another DATA packet,"
                    << " the number: " << (getDataPckRx_by_HN() + 1) << ", from NODE with MAC "
                << " address: " << mac_addr_NODE_in_data << std::endl;

            rxDATAEnabled = true;
            state_wait_other_DATA();

        }
    } else {
        //HN is not enabled to receive a DATA packet
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::DATA_rx()_____________________________Not_enabled_to_rx_DATA_pcks." << std::endl;

        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") DATA_rx() ---->HN(" << addr << ") is not enabled to receive the DATA"
                << " packet: DROP IT." << std::endl;

        drop(curr_DATA_HN_pck_rx, 1, UWUFETCH_NODE_DROP_REASON_NOT_ENABLE);
    }
} //end DATA_rx();

void uwUFetch_NODE::state_CBEACON_tx() {

    while (!Q_probbed_mac_HN.empty()) {
        Q_probbed_mac_HN.pop();
        Q_probbed_n_pcks_NODE_want_tx_HN.pop();
        Q_probbed_backoff_time.pop();
    }
    //HN transmit a CBEACON packet to the sensor nodes
    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::state_CBEACON_tx() ---->HN is initializing the CBEACON packet"
            << " which will then transmit to the NODEs in broadcast. " << std::endl;

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________Initializing_CBEACON_pck_number_"
            << getCBeaconPckTx_by_HN() + 1 << std::endl;

    txRTSbeforeCBEACON == false;
    refreshState(UWUFETCH_NODE_STATUS_TRANSMIT_BEACON);
    if (print_transitions) printStateInfo();

    Packet* p = Packet::alloc(); //allocate the memory
    hdr_cmn* cmh = hdr_cmn::access(p);
    hdr_mac* mach = HDR_MAC(p);
    hdr_CBEACON_UFETCH* cbeaconh = HDR_CBEACON_UFETCH(p);

    cmh->ptype() = PT_CBEACON_UFETCH; //type of the packet

    mach->set(MF_CONTROL, addr, MAC_BROADCAST); //set the information of the packet (type,source address,destination address)
    mach->macSA() = addr; //set the source address as AUV MAC address
    mach->macDA() = MAC_BROADCAST; //set the destination address as BROADCAST address

    cbeaconh->t_min_bc() = (int) (T_MIN_BACKOFF_PROBE * 1000); //minimum value for to choose the back-off time for tx PROBE pck
    cbeaconh->t_max_bc() = (int) (T_MAX_BACKOFF_PROBE * 1000); //maximum value for to choose the back-off time for tx PROBE pck

    cmh->size() = sizeof (hdr_CBEACON_UFETCH); //set the size of CBEACON header packet
    curr_CBEACON_HN_pck_tx = p->copy(); //create a copy of the CBEACON packet

    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________PARAMETERS_of_CBEACON_pck." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________Source_address:_" << mach->macSA() << "." << std::endl;
    if (mach->macDA() == -1) {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________Destination_address:_BROADCAST." << std::endl;
    } else {
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________Destination_address:_" << mach->macDA() << "." << std::endl;
    }
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________MIN_backoff_time_PROBE_pck:_" << T_MIN_BACKOFF_PROBE << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________MAX_backoff_time_PROBE_pck:_" << T_MAX_BACKOFF_PROBE << "[s]." << std::endl;
    if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::state_CBEACON_tx()____________________Size_of_CBEACON_pck:_" << cmh->size() << "[byte]." << std::endl;

    Packet::free(p); //de-allocate the memory

    txCBEACONEnabled = true; //Indicates if or not the HN is enabled to transmit a CBEACON packet to the NODE

    CBEACON_tx(); //recall the function that simulate the transmission of the CBEACON

} //end state_CBEACON_tx();

void uwUFetch_NODE::CBEACON_tx() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::CBEACON_tx() ---->HN is transmitting a CBEACON packet"
            << " to the NODEs in broadcast." << std::endl;

    refreshReason(UWUFETCH_NODE_STATUS_CHANGE_CBEACON_TX); //specify the reason that the protocol state change

    Mac2PhyStartTx(curr_CBEACON_HN_pck_tx);

} //end CBEACON_tx();

/*******************************************************************************
 *                          METODI AUSILIARI                                   * 
 ******************************************************************************/
bool uwUFetch_NODE::typeCommunication() {
    if (MODE_COMM_HN_AUV == 1) {
        return true;
    } else {
        return false;
    }
} //end typeCommunication();

bool uwUFetch_NODE::burstDATA() {
    if (MODE_BURST_DATA == 1) {
        return true;
    } else {
        return false;
    }
} //end burstDATA()

double uwUFetch_NODE::choiceBackOffTimer_HN() {
    if (typeCommunication()) {
        //Communication between HN & AUV without RTS and CTS packets
        //RNG::defaultrng()->set_seed(addr*5);
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") choiceBackOffTimer() ---->HN is choosing the back-off timer." << std::endl;
        srand(time(NULL) + addr);
        int random = rand() % (int) T_max_bck_DATA + (int) T_min_bck_DATA;

        if (debug_) std::cout << NOW << " uwUFetch_NODE(" << addr << ") ::choiceBackOffTimer() ---->Back-off time choice by the HN is = "
                << 0.833 * random << "[s]." << std::endl;
        return (0.833 * random);
    } else {
        //Communication between HN & AUV with RTS and CTS packets
        //RNG::defaultrng()->set_seed(addr*5);
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") choiceBackOffTimer() ---->HN is choosing the back-off timer." << std::endl;
        srand(time(NULL) + addr);
        int random = rand() % (int) T_max_bck_RTS + (int) T_min_bck_RTS;

        if (debug_) std::cout << NOW << " uwUFetch_NODE(" << addr << ") ::choiceBackOffTimer() ---->Back-off time choice by the HN is = "
                << 0.833 * random << "[s]." << std::endl;
        return (0.833 * random);
    }
} //end choiceBackoffTimer_HN();

double uwUFetch_NODE::getDataTimerValue() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::getDataTimerValue() ---->HN is computing DATA timeout." << std::endl;

    computeTxTime_HN(UWUFETCH_NODE_PACKET_TYPE_DATA);
    RTT = getRTT();

    int pck_tx_number = 0;
    if (Q_probbed_n_pcks_NODE_want_tx_HN.front() <= MAX_PCK_HN_WANT_RX_FROM_NODE) {
        pck_tx_number = Q_probbed_n_pcks_NODE_want_tx_HN.front();
    } else {
        pck_tx_number = MAX_PCK_HN_WANT_RX_FROM_NODE;
    }

    double data_timeout_val = 0;
    data_timeout_val = number_data_pck_HN_rx_exact * (Tdata_HN + T_GUARD + RTT);

    return data_timeout_val;

} //end getDataTimerValue();

void uwUFetch_NODE::recvFromUpperLayers_HN(Packet* p) {
    hdr_uwcbr* cbrh = HDR_UWCBR(p);

    if ((int) Q_data.size() < MAXIMUM_BUFFER_DATA_PCK_NODE) {
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::recvFromUpperLayers() ---->HN is queuing a DATA packet"
                << " generated by the APPLICATION layer" << std::endl;
        Q_data.push(p);
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::recvFromUpperLayer()__________________DATA_pck_from_upper_layer:id_pck_" << cbrh->sn() << "_STORE_It." << std::endl;
    } else {
        if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::recvFromUpperLayers()--->HN dropped DATA packet because"
                << " its buffer is full" << std::endl;
        if (debugMio_) out_file_logging << NOW << "uwUFetch_HEAD_NODE(" << addr << ")::recvFromUpperLayer()__________________DATA_pck_from_upper_layer:id_pck_" << cbrh->sn() << "_BUFFER_FULL:_drop_It." << std::endl;
        drop(p, 1, UWUFETCH_NODE_DROP_REASON_BUFFER_FULL);
    }
} //end recvFromUpperLayers_HN();

void uwUFetch_NODE::computeTxTime_HN(UWUFETCH_NODE_PACKET_TYPE tp) {
    Packet* simple_pck = Packet::alloc();

    if (tp == UWUFETCH_NODE_PACKET_TYPE_POLL) {
        hdr_cmn* cmh = HDR_CMN(simple_pck);
        cmh->size() = sizeof (hdr_POLL_UFETCH);
        hdr_mac* mach = HDR_MAC(simple_pck);

        mach->macSA() = addr;
        mach->macDA() = mac_addr_NODE_polled;
        Tpoll = Mac2PhyTxDuration(simple_pck);

    } else {
        hdr_cmn* cmh = HDR_CMN(simple_pck);
        hdr_mac* mach = HDR_MAC(simple_pck);
        cmh->size() = MAX_PAYLOAD;

        mach->macSA() = addr;
        mach->macDA() = mac_addr_AUV_in_trigger;
        Tdata_HN = Mac2PhyTxDuration(simple_pck);
    }

    Packet::free(simple_pck);
} //end computeTxTime_HN();

double uwUFetch_NODE::getRTT() {
    RTT = 0;

    if (tx_BEACON_start_HN_time > tx_CBEACON_start_HN_time) {
        RTT = rx_PROBE_finish_HN_time - tx_BEACON_start_HN_time; //- bck_value_probe_tx - Tprobe; 
    } else {
        RTT = rx_PROBE_finish_HN_time - tx_CBEACON_start_HN_time; //  - bck_value_probe_tx - Tprobe;
    }
    return RTT;
} //end getRTT();

void uwUFetch_NODE::updateListProbbedNode() {

    if (debug_) std::cout << NOW << " uwUFetch_NODE (" << addr << ") ::updateListProbbedNode() ---->HN has finished to receive DATA packet from the NODE with"
            << " MAC address: " << mac_addr_NODE_polled << ", so remove the NODE from the list." << std::endl;

    Q_probbed_mac_HN.pop();
    Q_probbed_n_pcks_NODE_want_tx_HN.pop();
    Q_probbed_backoff_time.pop();

} //end updateListProbbedNode();

/*******************************************************************************
 *                              EXPIRE METHODS                                 *
 ******************************************************************************/
void uwUFetch_NODE::uwUFetch_BEACON_timer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << " uwUFetch_NODE (" << module->addr << ") ::uwUFetch_BEACON_timer::expire() ---->BEACON timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->BeaconTxTOExpire();
} //end uwUFetch_BEACON_timer::expire();

void uwUFetch_NODE::uwUFetch_PROBE_timer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << " uwUFetch_NODE (" << module->addr << ")::uwUFetch_PROBE_timer::expire() ---->PROBE timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->ProbeTOExpired();
} //end uwUFetch_PROBE_timer::expire();

void uwUFetch_NODE::uwUFetch_DATA_timer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << " uwUFetch_NODE (" << module->addr << ") ::uwUFetch_DATA_timer::expire() ---->DATA timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->DataTOExpired();
} //end uwUFetch_DATA_timer::expire();

void uwUFetch_NODE::uwUFetch_CTS_timer::expire(Event* e) {
    if ((module->debug_)) std::cout << NOW << " uwUFetch_NODE (" << module->addr << ")::uwUFetch_CTS_timer::expire() ---->CTS timeout expired" << std::endl;
    timer_status = UWUFETCH_TIMER_STATUS_EXPIRED;

    module->CtsTOExpired();
} //end uwUFetch_CTS_timer::expire();


