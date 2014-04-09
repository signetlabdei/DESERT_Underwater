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
 * @file packer-uwip.cpp
 * @author Paolo Casari
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the NS-Miracle packet of uw-ip into a bit stream, and vice-versa.
 */

#include "packer-uwip.h"

/**
 * Class to create the Otcl shadow object for an object of the class packer.
 */
static class PackerUWIPClass : public TclClass {
public:

    PackerUWIPClass() : TclClass("UW/IP/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packerUWIP());
    }
} class_module_packerUWIP;

//packerUWIP::packerUWIP() : packer(false), isRacunBroadcast(0) {
packerUWIP::packerUWIP() : packer(false) {
    SAddr_Bits = 8 * sizeof (uint8_t);
    DAddr_Bits = 8 * sizeof (uint8_t);

    bind("SAddr_Bits", (int*) &SAddr_Bits);
    bind("DAddr_Bits", (int*) &DAddr_Bits);
    //bind("isRacunBroadcast_", &isRacunBroadcast);

    if (debug_)
        cout << "Initialization (from constructor) of n_bits for the UWIP packer " << endl;

    n_bits.clear();

    n_bits.push_back(SAddr_Bits);
    n_bits.push_back(DAddr_Bits);
}

packerUWIP::~packerUWIP() {

}

void packerUWIP::init() {

    if (debug_)
        cout << "Re-initialization of n_bits for the UWIP packer " << endl;

    n_bits.clear();

    n_bits.push_back(SAddr_Bits);
    n_bits.push_back(DAddr_Bits);
}

size_t packerUWIP::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {

    // Pointer to the UWIP packet header
    hdr_uwip* hip = HDR_UWIP(p);

    int field_idx = 0;

    offset += put(buf, offset, &(hip->saddr_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hip->daddr_), n_bits[field_idx++]);

    if (debug_) {
        printf("\033[0;42;30m TX IP packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

size_t packerUWIP::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {

    // Pointer to the UWIP packet header
    hdr_uwip* hip = HDR_UWIP(p);

    int field_idx = 0;

    memset(&(hip->saddr()), 0, sizeof ( hip->saddr_));
    offset += get(buf, offset, &(hip->saddr_), n_bits[field_idx++]);

    memset(&(hip->daddr()), 0, sizeof ( hip->daddr_));
    offset += get(buf, offset, &(hip->daddr_), n_bits[field_idx++]);

//    if (isRacunBroadcast && (hip->daddr_ == RACUN_BROADCAST)){
//        hip->daddr_ = UWIP_BROADCAST;
//    }

    if (debug_) {
        printf("\033[0;42;30m RX IP packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

void packerUWIP::printMyHdrMap() {
    std::cout << "\033[0;42;30m" << " Packer Name " << "\033[0m" << " UWIP" << std::endl;
    std::cout << "\033[0;42;30m 1st field " << "\033[0m" << " saddr: " << SAddr_Bits << " bits" << std::endl;
    std::cout << "\033[0;42;30m 2nd field " << "\033[0m" << " daddr: " << DAddr_Bits << " bits" << std::endl;
    return;
}

void packerUWIP::printMyHdrFields(Packet* p) {
    hdr_uwip* hip = HDR_UWIP(p);
    
    if (n_bits[0] != 0) {
        std::cout << "\033[0;42;30m saddr:\033[0m " << static_cast<uint32_t> (hip->saddr()) << " " << hex_bytes(hip->saddr(), n_bits[0]) << std::endl;
    }
    if (n_bits[1] != 0) {
        std::cout << "\033[0;42;30m daddr:\033[0m " << static_cast<uint32_t> (hip->daddr()) << " " << hex_bytes(hip->daddr(), n_bits[1]) << std::endl;
    }
}
