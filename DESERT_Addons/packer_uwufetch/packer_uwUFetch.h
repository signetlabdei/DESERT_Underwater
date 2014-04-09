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
/* 
 * @file        packer_uwUFetch.h
 * @author  Loris Brolo
 * \version   1.0.0 
 * 
 * \brief  Header of the class responsible to map the ns2 packet of uwUFecth into a bit stream, and vice-versa.
 */

#ifndef PACKER_UWUFETCH_H
#define	PACKER_UWUFETCH_H

#include "packer.h"
#include "uwUFetch_cmn_hdr.h"
#include <mac.h>
#include <iostream>

/**
 * Class to map a ns2 mac header into a bit stream, and vice-versa.
 */
class packer_uwUFetch : public packer {
public:
    /** 
     * Constructor packer_uwUFetch class
     * 
     */
    packer_uwUFetch();

    /** 
     * Destructor packer_uwUFetch class
     * 
     */
    ~packer_uwUFetch();


private:

    /**
     * Init the packer for uwUFetch prototocol
     */
    void init();

    /**
     *  Method to transform the headers of UFetch protocol into a stream of bits
     * 
     * @param Pointer to the packet to serialize
     * @param Pointer to the buffer 
     * @param Offset from the begin of the buffer 
     * 
     * @return  size_t New offset after packing the headers of the packets 
     */
    size_t packMyHdr(Packet*, unsigned char*, size_t);

    /**
     *  Method responsible to take the informations from the received buffer and store it into the headers of the packet
     * 
     * @param Pointer to the buffer received
     * @param Offset from the begin of the buffer
     * @param Pointer to the new packet
     * 
     * @return size_t New offset after unpacking the headers
     */
    size_t unpackMyHdr(unsigned char*, size_t, Packet*);

    /**
     * Method used for debug purposes. It prints the number of bits for each header serialized
     */
    void printMyHdrMap();

    /**
     *  Method used for debug purposes. It prints the value of the headers of a packet
     * 
     * @param Pointer of the packet  
     */
    void printMyHdrFields(Packet*);

    /**< Index bits */
    enum nbits_index {
        T_BCK_MAX_PROBE = 0, /**< Upper bound of the time interval from which the sensor node choice the backoff time before to transmit probe */
        T_BCK_MIN_PROBE, /**< Lower bound of the time interval from which the sensor node choice the backoff time before to transmit probe */
        N_CBEACON_TX, /**< Maximum number of CBEACON that the head node can transmit */
        T_BCK_CHOICE_SENSOR, /**< Backoff time choose by the sensor node before to transmit the probe packet */
        N_PCK_SENSOR_WANT_TX, /**< Number of the packets that the sensor node would like to transmit to the head node */
        MAC_ADDR_SENSOR_POLLED, /**< Mac address of the sensor node from which the head node want to receive the data packet */
        N_PCK_HN_WANT_RX, /**< Maximum number of data packet that the head node want to receive from the sensor node polled */
        MAX_CBEACON_TX_HN, /**< Maximum number of CBEACON that the head node can transmit */
        T_BCK_MAX_RTS, /**< Upper bound of the time interval from which the sensor node choice the backoff time before to transmit rts */
        T_BCK_MIN_RTS, /**< Lower bound of the time interval from which the sensor node choice the backoff time before to transmit rts  */
        N_PCK_AUV_WANT_RX, /**< Maximum number of packets that AUV want receive from the HN */
        NUM_DATA_PCKS, /**< Number of DATA packets that HN would like to transmit to the AUV */
        BACKOFF_TIME_RTS, /**< Backoff time choice by the HN before to transmit a RTS packet */
        NUM_DATA_PCKS_MAX_RX, /**< Exact number of DATA packets that AUV want to receive from the HN */
        MAC_ADDR_HN_CTSED /**< MAC address of the HN from which AUV want to receive DATA packet */
    };

    size_t t_bck_max_probe_Bits; /**< number of bits used for t_bck_max_probe_ field on BEACON header */
    size_t t_bck_min_probe_Bits; /**< number of bits used for t_bck_min_probe_  field on BEACON header */
    size_t n_cbeacon_tx_Bits; /**< number of bits used for n_cbeacon_tx_ field on BEACON header */
    size_t t_bck_choice_sensor_Bits; /**< number of bits used for t_bck_choice_sensor_ field on PROBE header */
    size_t n_pck_sensor_want_tx_Bits; /**< number of bits used for n_pck_sensor_want_tx_ field on PROBE header */
    size_t mac_addr_sensor_polled_Bits; /**< number of bits used for mac_addr_sensor_polled_ field on POLL header */
    size_t n_pck_hn_want_tx_Bits; /**< number of bits used for n_pck_hn_want_tx_ field on POLL header */
    size_t max_cbeacon_tx_hn_Bits; /**< number of bits used for max_cbeacon_tx_hn_ field on CBEACON header */
    size_t t_bck_max_rts_Bits; /**< number of bits used for t_bck_max_rts_ field on CBEACON header */
    size_t t_bck_min_rts_Bits; /**< number of bits used for t_bck_min_rts_ field on CBEACON header */
    size_t max_pck_want_rx_Bits; /**< number of bits used for max_pck_want_rx_ field on TRIGGER header */
    size_t num_DATA_pcks_Bits; /**< number of bits used for num_DATA_pcks_ field on TRIGGER header */
    size_t backoff_time_RTS_Bits; /**< number of bits used for backoff_time_RTS_ field on TRIGGER header */
    size_t num_DATA_pcks_MAX_rx_Bits; /**< number of bits used for num_DATA_pcks_MAX_rx_ field on RTSheader */
    size_t mac_addr_HN_ctsed_Bits; /**< number of bits used for mac_addr_HN_ctsed_ field on CTS header */

};
#endif

