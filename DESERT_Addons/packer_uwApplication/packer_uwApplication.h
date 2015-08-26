//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
//

/* 
 * File:   packer_uwApplication.h
 * Author: Loris Brolo
 *
 * Created on 10 gennaio 2014, 11.11
 */

#ifndef PACKER_UWAPPLICATION_H
#define	PACKER_UWAPPLICATION_H

//#include "../../packer.h"
#include "packer.h"
#include "uwApplication_cmn_header.h"
#include <iostream>

/**
 * Class to map a ns2 mac header into a bit stream, and vice-versa.
 */
class packer_uwApplication : public packer {
public:
    /** 
     * Constructor packer_uwApplication class
     * 
     */
    packer_uwApplication();

    /** 
     * Destructor packer_uwApplication class
     * 
     */
    ~packer_uwApplication();


private:

    /**
     * Init the packer for uwApplication prototocol
     */
    void init();

    /**
     *  Method to transform the headers of uwApplication protocol into a stream of bits
     * 
     * @param Pointer to the packet to serialize
     * @param Pointer to the buffer 
     * @param Offset from the begin of the buffer 
     * 
     * @return  size_t New offset after packing the headers of the packets 
     */
    size_t packMyHdr(Packet* p, unsigned char* buffer, size_t offset);

    /**
     *  Method responsible to take the informations from the received buffer and store it into the headers of the packet
     * 
     * @param Pointer to the buffer received
     * @param Offset from the begin of the buffer
     * @param Pointer to the new packet
     * 
     * @return size_t New offset after unpacking the headers
     */
    size_t unpackMyHdr(unsigned char* buffer, size_t offset, Packet* p);

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
        SN_FIELD=0,/**< Serial number of the packet. */     
        RFFT_FIELD,/**< Forward Trip Time of the packet. */
        RFFTVALID_FIELD,/**< Flag used to set the validity of the fft field. */
        PRIORITY_FIELD,/**< Priority flag: 1 means high priority, 0 normal priority. */
        PAYLOAD_SIZE_FIELD/**< Message payload */
        
    };

    size_t sn_field_Bits; /**< Bit length of the sn_ field to be put in the header stream of bits. */
    size_t rfft_field_Bits; /**< Bit length of the rfft_field to be put in the header stream of bits. */
    size_t rfftvalid_field_Bits; /**< Bit length of the rfftvalid_field to be put in the header stream of bits. */
    size_t priority_filed_Bits; /**< Bit length of the priority_field to be put in the header stream of bits. */
    size_t payload_size_field_Bits; /**< Bit length of the payloadmsg_field to be put in the header stream of bits. */
};
#endif	/* PACKER_UWAPPLICATION_H */

