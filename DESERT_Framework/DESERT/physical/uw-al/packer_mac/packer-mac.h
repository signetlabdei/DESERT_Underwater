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
 * @file packer-mac.h
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Header of the class responsible to map the ns2 packet of mac into a bit stream, and vice-versa.
 */

#ifndef PACKER_MAC_H
#define PACKER_MAC_H

#include "packer.h"

#include <mac.h>

#include <iostream>

/**
 * Class to map a ns2 mac header into a bit stream, and vice-versa.
 */
class packerMAC : public packer {
public:

    /** 
     * Class constructor.
     * 
     */
    packerMAC();

    /** 
     * Class destructor.
     * 
     */
    ~packerMAC();


private:

    /**
     * Init the Packer
     */
    void init();
    /**
     * Method to transform the headers of Uwpolling into a stream of bits
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

    size_t Ftype_Bits; /** Bit length of the ftype_ field to be put in the header stream of bits. */
    size_t SRC_Bits; /** Bit length of the macSA_ field to be put in the header stream of bits. */
    size_t DST_Bits; /** Bit length of the macDA_ field to be put in the header stream of bits. */
    size_t Htype_Bits; /** Bit length of the hdr_type_ field to be put in the header stream of bits. */
    size_t TXtime_Bits; /** Bit length of the txtime_ field to be put in the header stream of bits. */
    size_t SStime_Bits; /** Bit length of the sstime_ field to be put in the header stream of bits. */
    size_t Padding_Bits; /** Bit length of the padding_ field to be put in the header stream of bits. */
};

#endif

