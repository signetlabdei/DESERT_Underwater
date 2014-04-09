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
 * @file packer-uwudp.cpp
 * @author Paolo Casari
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the NS-Miracle packet of uw-udp into a bit stream, and vice-versa.
 */

#include "packer-uwudp.h"

/**
 * Class to create the Otcl shadow object for an object of the class packer.
 */
static class PackerUWUDPClass : public TclClass {
public:

    PackerUWUDPClass() : TclClass("UW/UDP/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packerUWUDP());
    }
} class_module_packerUWUDP;

packerUWUDP::packerUWUDP() : packer(false) {
    SPort_Bits = 8 * sizeof (u_int16_t);
    DPort_Bits = 8 * sizeof (u_int16_t);

    bind("SPort_Bits", (int*) &SPort_Bits);
    bind("DPort_Bits", (int*) &DPort_Bits);

    if (debug_)
        cout << "Initialization (from constructor) of n_bits for the UWUDP packer " << endl;

    n_bits.clear();

    n_bits.push_back(SPort_Bits);
    n_bits.push_back(DPort_Bits);
}

packerUWUDP::~packerUWUDP() {

}

void packerUWUDP::init() {

    if (debug_)
        cout << "Re-initialization of n_bits for the UWUDP packer " << endl;

    n_bits.clear();

    n_bits.push_back(SPort_Bits);
    n_bits.push_back(DPort_Bits);
}

size_t packerUWUDP::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {

    // Pointer to the UWUDP packet header
    hdr_uwudp* hudp = HDR_UWUDP(p);

    int field_idx = 0;

    offset += put(buf, offset, &(hudp->sport_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hudp->dport_), n_bits[field_idx++]);

    if (debug_) {
        printf("\033[1;37;44m TX UDP packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

size_t packerUWUDP::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {

    // Pointer to the UWUDP packet header
    hdr_uwudp* hudp = HDR_UWUDP(p);

    int field_idx = 0;

    memset(&(hudp->sport()), 0, sizeof ( hudp->sport_));
    offset += get(buf, offset, &(hudp->sport_), n_bits[field_idx++]);

    memset(&(hudp->dport()), 0, sizeof ( hudp->dport_));
    offset += get(buf, offset, &(hudp->dport_), n_bits[field_idx++]);

    if (debug_) {
        printf("\033[1;37;44m RX UDP packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

void packerUWUDP::printMyHdrMap() {
    std::cout << "\033[1;37;44m" << " Packer Name " << "\033[0m" << " UWUDP" << std::endl;
    std::cout << "\033[1;37;44m 1st field " << "\033[0m" << " sport: " << SPort_Bits << " bits" << std::endl;
    std::cout << "\033[1;37;44m 2nd field " << "\033[0m" << " dport: " << DPort_Bits << " bits" << std::endl;
}

void packerUWUDP::printMyHdrFields(Packet* p) {
    hdr_uwudp* hudp = HDR_UWUDP(p);
    
    if (n_bits[0] != 0) {
        std::cout << "\033[1;37;44m sport:\033[0m " << static_cast<uint32_t> (hudp->sport()) << " " << hex_bytes(hudp->sport(), n_bits[0]) << std::endl;
    }
    if (n_bits[1] != 0) {
        std::cout << "\033[1;37;44m dport:\033[0m " << static_cast<uint32_t> (hudp->dport()) << " " << hex_bytes(hudp->dport(), n_bits[1]) << std::endl;
    }
}

