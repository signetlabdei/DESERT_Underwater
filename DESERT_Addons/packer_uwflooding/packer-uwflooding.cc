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
 * @file packer-uwflooding.cc
 * @author Giovanni Toso
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the NS-Miracle packet of uwflooding into a bit stream, and vice-versa.
 */

#include "packer-uwflooding.h"

/**
 * Class to create the Otcl shadow object for an object of the class packer.
 */
static class PackerUwfloodingClass : public TclClass {
public:

    PackerUwfloodingClass() : TclClass("UW/FLOODING/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new PackerUwFlooding());
    }
} class_module_PackerUwflooding;

PackerUwFlooding::PackerUwFlooding() : packer(false) {
    ttl_Bits = 8 * sizeof (uint8_t);

    bind("ttl_Bits", (int*) &ttl_Bits);

    if (debug_) {
        std::cout << "Initialization (from constructor) of n_bits for the UWFLOODING packer " << std::endl;
    }

    n_bits.clear();

    n_bits.push_back(ttl_Bits);
}

PackerUwFlooding::~PackerUwFlooding() {

}

void PackerUwFlooding::init() {

    if (debug_)
        std::cout << "Re-initialization of n_bits for the UWFLOODING packer " << std::endl;

    n_bits.clear();
    n_bits.push_back(ttl_Bits);
}

size_t PackerUwFlooding::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {

    // Pointer to the UWFLOODING packet header
    hdr_uwflooding* hflooding = HDR_UWFLOODING(p);

    int field_idx = 0;

    offset += put(buf, offset, &(hflooding->ttl()), n_bits[field_idx++]);

    if (debug_) {
        printf("\033[1;37;46m TX UWFLOODING packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

size_t PackerUwFlooding::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {

    hdr_uwflooding* hflooding = HDR_UWFLOODING(p);

    int field_idx = 0;

    memset(&(hflooding->ttl()), 0, sizeof (hflooding->ttl()));
    offset += get(buf, offset, &(hflooding->ttl()), n_bits[field_idx++]);

    if (debug_) {
        printf("\033[1;37;46m RX UWFLOODING packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

void PackerUwFlooding::printMyHdrMap() {
    std::cout << "\033[1;37;46m" << " Packer Name " << "\033[0m" << " UWFLOODING" << std::endl;
    std::cout << "\033[1;37;46m 1st field " << "\033[0m" << " ttl: " << ttl_Bits << " bits" << std::endl;
    return;
}

void PackerUwFlooding::printMyHdrFields(Packet* p) {
    hdr_uwflooding* hflooding = HDR_UWFLOODING(p);
    
    if (n_bits[0] != 0) {
        std::cout << "\033[1;37;46m ttl:\033[0m " << static_cast<uint32_t>(hflooding->ttl()) << " " << hex_bytes(hflooding->ttl(), n_bits[0]) << std::endl;
    }
}
