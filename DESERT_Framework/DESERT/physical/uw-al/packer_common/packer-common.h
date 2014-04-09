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
 * @file packer-common.h
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Header of the class responsible to map the ns2 common header into a bit stream, and vice-versa.
 */

#ifndef PACKER_COMMON_H
#define PACKER_COMMON_H

#include "packer.h"

#include <packet.h>

#include <iostream>

#define NUM_FIELDS 22

/**
 * Class to map a ns2 mac header into a bit stream, and vice-versa.
 */
class packerCOMMON : public packer {
public:

    /** 
     * Class constructor.
     * 
     */
    packerCOMMON();

    /** 
     * Class destructor.
     * 
     */
    ~packerCOMMON();


private:

    enum nbits_index {
        PTYPE = 0,
        SIZE,
        UID,
        ERROR,
        TIMESTAMP,
        DIRECTION,
        PREV_HOP,
        NEXT_HOP,
        ADDR_TYPE,
        LAST_HOP,
        TXTIME,
        errbitcnt_,
        fecsize_,
        iface_,
        src_rt_valid,
        ts_arr_,
        aomdv_salvage_count_,
        xmit_failure_,
        xmit_failure_data_,
        xmit_reason_,
        num_forwards_,
        opt_num_forwards_
    };
    /**
     * Init the Packer
     */
    void init();
    /**
     * Method to transform the headers into a stream of bits
     * @param Pointer to the packet to serialize
     * @param Pointer to the buffer
     * @param Offset from the begin of the buffer
     * @return New offset after packing the headers of the packets
     */
    size_t packMyHdr(Packet*, unsigned char*, size_t);
    /**
     * Method responsible to take the informations from the received buffer and store it into the headers of the packet
     * @param Pointer to the buffer received
     * @param Offset from the begin of the buffer
     * @param Pointer to the new packet
     * @return New offset after unpacking the headers
     */
    size_t unpackMyHdr(unsigned char*, size_t, Packet*);
    /**
     * Method used for debug purposes. It prints the number of bits for each header serialized
     */
    void printMyHdrMap();
    /**
     * Method used for debug purposes. It prints the value of the headers of a packet
     * @param Pointer of the packet 
     */
    void printMyHdrFields(Packet*);

    // Common header fields useful for DESERT
    size_t PTYPE_Bits; /** Bit length of the ptype_ field to be put in the header stream of bits. */
    size_t SIZE_Bits; /** Bit length of the size_ field to be put in the header stream of bits. */
    size_t UID_Bits; /** Bit length of the uid_ field to be put in the header stream of bits. */
    size_t ERROR_Bits; /** Bit length of the error_ field to be put in the header stream of bits. */
    size_t TIMESTAMP_Bits; /** Bit length of the ts_ field to be put in the header stream of bits. */
    size_t DIRECTION_Bits; /** Bit length of the direction_ field to be put in the header stream of bits. */
    size_t PREV_HOP_Bits; /** Bit length of the prev_hop_ field to be put in the header stream of bits. */
    size_t NEXT_HOP_Bits; /** Bit length of the next_hop_ field to be put in the header stream of bits. */
    size_t ADDR_TYPE_Bits; /** Bit length of the addr_type_ field to be put in the header stream of bits. */
    size_t LAST_HOP_Bits; /** Bit length of the last_hop_ field to be put in the header stream of bits. */
    size_t TXTIME_Bits; /** Bit length of the txtime_ field to be put in the header stream of bits. */

    // Other common header fields (maybe useful for future usage)    
    size_t errbitcnt_Bits; /** Bit length of the errbitcnt_ field to be put in the header stream of bits. */
    size_t fecsize_Bits; /** Bit length of the fecsize_ field to be put in the header stream of bits. */
    size_t iface_Bits; /** Bit length of the iface_ field to be put in the header stream of bits. */
    size_t src_rt_valid_Bits; /** Bit length of the src_rt_valid_ field to be put in the header stream of bits. */
    size_t ts_arr_Bits; /** Bit length of the ts_arr_ field to be put in the header stream of bits. */
    size_t aomdv_salvage_count_Bits; /** Bit length of the aomdv_salvage_count_ field to be put in the header stream of bits. */
    size_t xmit_failure_Bits; /** Bit length of the xmit_failure_ field to be put in the header stream of bits. */
    size_t xmit_failure_data_Bits; /** Bit length of the xmit_failure_data_ field to be put in the header stream of bits. */
    size_t xmit_reason_Bits; /** Bit length of the xmit_reason_ field to be put in the header stream of bits. */
    size_t num_forwards_Bits; /** Bit length of the num_forwards_ field to be put in the header stream of bits. */
    size_t opt_num_forwards_Bits; /** Bit length of the opt_num_forwards_ field to be put in the header stream of bits. */

};

#endif
