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
 * @file       packer_uwApplication.cc
 * @author     Loris Brolo
 * \version    1.0.0
 *  Created on 10 gennaio 2014, 11.30
 *  * \brief      Implementation of the class responsible to map the ns2 packet of wAPPLICATION into a bit stream, and vice-versa.
 */

#include "packer_uwApplication.h"

static class PackerUwApplicationClass : public TclClass {
    public:

    PackerUwApplicationClass() : TclClass("UW/APP/uwApplication/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packer_uwApplication());
    }
} class_module_packerUwApplication;

packer_uwApplication::packer_uwApplication() 
:
packer(false),
sn_field_Bits(0),
rfft_field_Bits(0),
rfftvalid_field_Bits(0),
priority_filed_Bits(0),
payload_size_field_Bits(0)
{
    bind("SN_FIELD_", (int*) &sn_field_Bits);
    bind("RFFT_FIELD_", (int*) &rfft_field_Bits);
    bind("RFFTVALID_FIELD_", (int*) &rfftvalid_field_Bits);
    bind("PRIORITY_FIELD_", (int*) &priority_filed_Bits);
    bind("PAYLOADMSG_FIELD_SIZE_", (int*) &payload_size_field_Bits);
    
    this->init();
} //end packer_uwApplication() constructor

packer_uwApplication::~packer_uwApplication(){

} //end packer_uwApplication() destructor

void packer_uwApplication::init() {
    if (debug_)
        std::cout << "Re-initialization of n_bits for the uwApplication packer." << std::endl;

    n_bits.clear();
    n_bits.assign(5, 0);
    
    n_bits[SN_FIELD] = sn_field_Bits;
    n_bits[RFFT_FIELD] = rfft_field_Bits;
    n_bits[RFFTVALID_FIELD] = rfftvalid_field_Bits;
    n_bits[PRIORITY_FIELD] = priority_filed_Bits;
    n_bits[PAYLOAD_SIZE_FIELD] = payload_size_field_Bits;
    
} //end init()

size_t packer_uwApplication::packMyHdr(Packet* p, unsigned char* buffer, size_t offset) {
    hdr_cmn* hcmn = HDR_CMN(p);

    if (hcmn->ptype() == PT_DATA_APPLICATION) {
        //Packet to be serialized is DATA packet
        hdr_DATA_APPLICATION* applh = HDR_DATA_APPLICATION(p);

        offset += put(buffer, offset, &(applh->sn_), n_bits[SN_FIELD]);
        offset += put(buffer, offset, &(applh->rftt_), n_bits[RFFT_FIELD]);
        offset += put(buffer, offset, &(applh->rftt_valid_), n_bits[RFFTVALID_FIELD]);
        offset += put(buffer, offset, &(applh->priority_), n_bits[PRIORITY_FIELD]);
        offset += put(buffer, offset, &(applh->payload_size_), n_bits[PAYLOAD_SIZE_FIELD]);
        int payload_size_bits = applh->payload_size()*8;
        offset += put(buffer, offset, &(applh->payload_msg), payload_size_bits);

        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWAPPLICATION::DATA packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }
    return offset;
} //end packMyHdr() method

size_t packer_uwApplication::unpackMyHdr(unsigned char* buffer, size_t offset, Packet* p) {
    
    hdr_cmn* hcmn = HDR_CMN(p);

    if (hcmn->ptype() == PT_DATA_APPLICATION) {
        //Packet to be serialized is a DATA packet
        hdr_DATA_APPLICATION* applh = HDR_DATA_APPLICATION(p);

        memset(&(applh->sn_), 0, sizeof (applh->sn_));
        offset += get(buffer, offset, &(applh->sn_), n_bits[SN_FIELD]);
        memset(&(applh->rftt_), 0, sizeof (applh->rftt_));
        offset += get(buffer, offset, &(applh->rftt_), n_bits[RFFT_FIELD]);
        memset(&(applh->rftt_valid_), 0, sizeof (applh->rftt_valid_));
        offset += get(buffer, offset, &(applh->rftt_valid_), n_bits[RFFTVALID_FIELD]);
        memset(&(applh->priority_), 0, sizeof (applh->priority_));
        offset += get(buffer, offset, &(applh->priority_), n_bits[PRIORITY_FIELD]);
        memset(&(applh->payload_msg), 0, sizeof (applh->payload_size_));
        offset += get(buffer, offset, &(applh->payload_size_), n_bits[PAYLOAD_SIZE_FIELD]);
        memset(&(applh->payload_msg), 0, sizeof (applh->payload_msg));
        int payload_size_bit = applh->payload_size()*8;
        //offset += get(buffer, offset, &(applh->payload_msg), n_bits[PAYLOADMSG_FIELD]);
        offset += get(buffer, offset, &(applh->payload_msg), payload_size_bit);
                
        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWAPPLICATION::DATA packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }
    return offset;
} //end unpackMyHdr() method

void packer_uwApplication::printMyHdrMap() {
    std::cout << "\033[1;37;45m Packer Name \033[0m: UWAPPLICATION \n";
    std::cout << "** DATA fields:\n";
    std::cout << "\033[1;37;45m Field: SN_FIELD: \033[0m:" << n_bits[SN_FIELD] << " bits\n";
    std::cout << "\033[1;37;45m Field: RFFT_FIELD: \033[0m:" << n_bits[RFFT_FIELD] << " bits\n";
    std::cout << "\033[1;37;45m Field: RFFTVALID_FIELD: \033[0m:" << n_bits[RFFTVALID_FIELD] << " bits\n";
    std::cout << "\033[1;37;45m Field: PRIORITY_FIELD: \033[0m:" <<  n_bits[PRIORITY_FIELD] << " bits\n";
    std::cout << "\033[1;37;45m Field: PAYLOADMSG_SIZE_FIELD: \033[0m:" << n_bits[PAYLOAD_SIZE_FIELD] << " bits\n";
    
    std::cout << std::endl;
} //end printMyHdrMap() method

void packer_uwApplication::printMyHdrFields(Packet* p) {
    //Print information packet serialized by the method packMyHdr
    hdr_cmn* hcmn = HDR_CMN(p);

    if (hcmn->ptype() == PT_DATA_APPLICATION) {
        //Packet to be serialized is a DATA packet
        hdr_DATA_APPLICATION* applh = HDR_DATA_APPLICATION(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, SN_FIELD: " << applh->sn_ << std::endl;
        std::cout << "\033[1;37;45m 2nd field \033[0m, RFFT_FIELD: " << applh->rftt_ << std::endl;
        std::cout << "\033[1;37;45m 3rd field \033[0m, RFFTVALID_FIELD: " << applh->rftt_valid_ << std::endl;
        std::cout << "\033[1;37;45m 4th field \033[0m, PRIORITY_FIELD: " << (int)applh->priority_ << std::endl;
        std::cout << "\033[1;37;45m 5th field \033[0m, PAYLOADMSG_SIZE_FIELD: " << applh->payload_size_ << std::endl;
        std::cout << "\033[1;37;45m 5th field \033[0m, PAYLOADMSG_FIELD: ";
        for(int i=0;i<applh->payload_size();i++)
        {
            cout << applh->payload_msg[i];
        }
        std::cout << endl;
    }
} //end printMyHdrFields() method