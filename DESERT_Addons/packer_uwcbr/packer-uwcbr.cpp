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
 * @file packer-uwcbr.cpp
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the NS-Miracle packet of uw-cbr into a bit stream, and vice-versa.
 */

#include "packer-uwcbr.h"

/**
 * Class to create the Otcl shadow object for an object of the class packer.
 */
static class PackerUWCBRClass : public TclClass {
public:

    PackerUWCBRClass() : TclClass("UW/CBR/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packerUWCBR());
    }
} class_module_packerUWCBR;

packerUWCBR::packerUWCBR() : packer(false) {
    SN_Bits = 8 * sizeof (u_int32_t);
    RFTT_Bits = 0;
    RFTT_VALID_Bits = 0;

    bind("SN_Bits", (int*) &SN_Bits);
    bind("RFTT_Bits", (int*) &RFTT_Bits);
    bind("RFTT_VALID_Bits", (int*) &RFTT_VALID_Bits);

    if (debug_)
        cout << "Initialization (from constructor) of n_bits for the UWCBR packer " << endl;

    n_bits.clear();

    n_bits.push_back(SN_Bits);
    n_bits.push_back(RFTT_Bits);
    n_bits.push_back(RFTT_VALID_Bits);

}

packerUWCBR::~packerUWCBR() {

}

void packerUWCBR::init() {

    if (debug_)
        cout << "Re-initialization of n_bits for the UWCBR packer " << endl;

    n_bits.clear();

    n_bits.push_back(SN_Bits);
    n_bits.push_back(RFTT_Bits);
    n_bits.push_back(RFTT_VALID_Bits);
}

size_t packerUWCBR::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {

    // Pointer to the UWCBR packet header
    hdr_cmn* ch = HDR_CMN(p);
    hdr_uwcbr* uch = HDR_UWCBR(p);

    if ( ch->ptype() == PT_UWCBR ) {
        int field_idx = 0;

        offset += put(buf, offset, &(uch->sn_), n_bits[field_idx++]);

        offset += put(buf, offset, &(uch->rftt_), n_bits[field_idx++]);

        offset += put(buf, offset, &(uch->rftt_valid_), n_bits[field_idx++]);

        if (debug_) {
            printf("\033[0;46;30m TX CBR packer hdr \033[0m \n");
            printMyHdrFields(p);
        }
    }

    return offset;
}

size_t packerUWCBR::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {

    // Pointer to the UWCBR packet header
    hdr_cmn* ch = HDR_CMN(p);
    hdr_uwcbr* uch = HDR_UWCBR(p);

    if ( ch->ptype() == PT_UWCBR ) {
        int field_idx = 0;

        memset(&(uch->sn_), 0, sizeof (uch->sn_));
        offset += get(buf, offset, &(uch->sn_), n_bits[field_idx++]);

        memset(&(uch->rftt_), 0, sizeof (uch->rftt_));
        offset += get(buf, offset, &(uch->rftt_), n_bits[field_idx++]);

        memset(&(uch->rftt_valid_), 0, sizeof (uch->rftt_valid_));
        offset += get(buf, offset, &(uch->rftt_valid_), n_bits[field_idx++]);

        if (debug_) {
            printf("\033[0;46;30m RX CBR packer hdr \033[0m \n");
            printMyHdrFields(p);
        }
    }

    return offset;
}

void packerUWCBR::printMyHdrMap() {
    std::cout << "\033[0;46;30m Packer Name \033[0m: UWCBR \n";
    std::cout << "\033[0;46;30m sn: \033[0m" << SN_Bits << " bits \n";
    std::cout << "\033[0;46;30m rftt: \033[0m" << RFTT_Bits << " bits \n";
    std::cout << "\033[0;46;30m rftt valid: \033[0m" << RFTT_VALID_Bits << " bits" << std::endl;
}


void packerUWCBR::printMyHdrFields(Packet* p) {
    hdr_uwcbr* uch = HDR_UWCBR(p);
    
    if (n_bits[0] != 0) std::cout << "\033[0;46;30m sn: \033[0m" << uch->sn() << std::endl;
    if (n_bits[1] != 0) std::cout << "\033[0;46;30m rftt: \033[0m" << uch->rftt() << std::endl;
    if (n_bits[2] != 0) std::cout << "\033[0;46;30m rftt_valid: \033[0m" << uch->rftt_valid() << std::endl;
}
