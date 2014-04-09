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
 * @file packer-mac.cpp
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the ns2 packet of mac into a bit stream, and vice-versa.
 */

#include "packer-mac.h"

/**
 * Class to create the Otcl shadow object for an object of the class packer.
 */
static class PackerMACClass : public TclClass {
public:

    PackerMACClass() : TclClass("NS2/MAC/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packerMAC());
    }
} class_module_packerMAC;

packerMAC::packerMAC() : packer(false) {

    Ftype_Bits = 0;
    SRC_Bits = sizeof (int);
    DST_Bits = sizeof (int);
    Htype_Bits = 0;
    TXtime_Bits = 0;
    SStime_Bits = 0;
    Padding_Bits = 0;

    bind("Ftype_Bits", (int*) &Ftype_Bits);
    bind("SRC_Bits", (int*) &SRC_Bits);
    bind("DST_Bits", (int*) &DST_Bits);
    bind("Htype_Bits", (int*) &Htype_Bits);
    bind("TXtime_Bits", (int*) &TXtime_Bits);
    bind("SStime_Bits", (int*) &SStime_Bits);
    bind("Padding_Bits", (int*) &Padding_Bits);

    if (debug_)
        cout << "Initialization (from constructor) of n_bits for the UWMAC packer " << endl;

    n_bits.clear();

    n_bits.push_back(Ftype_Bits);
    n_bits.push_back(SRC_Bits);
    n_bits.push_back(DST_Bits);
    n_bits.push_back(Htype_Bits);
    n_bits.push_back(TXtime_Bits);
    n_bits.push_back(SStime_Bits);
    n_bits.push_back(Padding_Bits);

}

packerMAC::~packerMAC() {

}

void packerMAC::init() {

    if (debug_)
        cout << "Re-initialization of n_bits for the UWMAC packer " << endl;

    n_bits.clear();

    n_bits.push_back(Ftype_Bits);
    n_bits.push_back(SRC_Bits);
    n_bits.push_back(DST_Bits);
    n_bits.push_back(Htype_Bits);
    n_bits.push_back(TXtime_Bits);
    n_bits.push_back(SStime_Bits);
    n_bits.push_back(Padding_Bits);
}

size_t packerMAC::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {

    // Pointer to the MAC packet header
    hdr_mac* hmac = HDR_MAC(p);

    int field_idx = 0;

    offset += put(buf, offset, &(hmac->ftype_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hmac->macSA_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hmac->macDA_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hmac->hdr_type_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hmac->txtime_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hmac->sstime_), n_bits[field_idx++]);
    offset += put(buf, offset, &(hmac->padding_), n_bits[field_idx++]);

    if (debug_) {
        printf("\033[0;45;30m TX MAC packer hdr \033[0m \n");
        printMyHdrFields(p);
    }
    return offset;
}

size_t packerMAC::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {

    // Pointer to the MAC packet header
    hdr_mac* hmac = HDR_MAC(p);

    int field_idx = 0;

    memset(&(hmac->ftype_), 0, sizeof (hmac->ftype_));
    offset += get(buf, offset, &(hmac->ftype_), n_bits[field_idx++]);

    memset(&(hmac->macSA_), 0, sizeof (hmac->macSA_));
    offset += get(buf, offset, &(hmac->macSA_), n_bits[field_idx++]);
    hmac->macSA_ = restoreSignedValue(hmac->macSA_, n_bits[field_idx - 1]);

    memset(&(hmac->macDA_), 0, sizeof (hmac->macDA_));
    offset += get(buf, offset, &(hmac->macDA_), n_bits[field_idx++]);
    hmac->macDA_ = restoreSignedValue(hmac->macDA_, n_bits[field_idx - 1]);

    /* Old function to uncompresso signed value
    if (n_bits[field_idx - 1] < sizeof(hmac->macDA()) * 8) { // Compression!
        bitset<sizeof(hmac->macDA()) * 8 > my_bitset(hmac->macDA());
        if (my_bitset[n_bits[field_idx - 1] - 1] == 1) {
            for (int i = n_bits[field_idx - 1]; i < sizeof(hmac->macDA()) * 8; ++i) {
                my_bitset.set(i,1);
            }
        }
        hmac->macDA() = static_cast<int>(my_bitset.to_ulong());
    }
     */

    memset(&(hmac->hdr_type_), 0, sizeof (hmac->hdr_type_));
    offset += get(buf, offset, &(hmac->hdr_type_), n_bits[field_idx++]);

    memset(&(hmac->txtime_), 0, sizeof (hmac->txtime_));
    offset += get(buf, offset, &(hmac->txtime_), n_bits[field_idx++]);

    memset(&(hmac->sstime_), 0, sizeof (hmac->sstime_));
    offset += get(buf, offset, &(hmac->sstime_), n_bits[field_idx++]);

    memset(&(hmac->padding_), 0, sizeof (hmac->padding_));
    offset += get(buf, offset, &(hmac->padding_), n_bits[field_idx++]);
    hmac->padding_ = restoreSignedValue(hmac->padding_, n_bits[field_idx - 1]);

    if (debug_) {
        printf("\033[0;45;30m RX MAC packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

void packerMAC::printMyHdrMap() {
    std::cout << "\033[0;45;30m" << " Packer Name " << "\033[0m" << " UWMAC" << std::endl;
    std::cout << "\033[0;45;30m 1st field " << "\033[0m" << " ftype: " << Ftype_Bits << " bits" << std::endl;
    std::cout << "\033[0;45;30m 2st field " << "\033[0m" << " macDA: " << SRC_Bits << " bits" << std::endl;
    std::cout << "\033[0;45;30m 3st field " << "\033[0m" << " macDA: " << DST_Bits << " bits" << std::endl;
    std::cout << "\033[0;45;30m 4st field " << "\033[0m" << " hdr_type: " << Htype_Bits << " bits" << std::endl;
    std::cout << "\033[0;45;30m 5st field " << "\033[0m" << " txtime: " << TXtime_Bits << " bits" << std::endl;
    std::cout << "\033[0;45;30m 6st field " << "\033[0m" << " sstime: " << SStime_Bits << " bits" << std::endl;
    std::cout << "\033[0;45;30m 7st field " << "\033[0m" << " padding: " << Padding_Bits << " bits" << std::endl;
    return;
}

void packerMAC::printMyHdrFields(Packet* p) {

    hdr_mac* hmac = HDR_MAC(p);

    if (n_bits[0] != 0) cout << "\033[0;45;30m ftype:\033[0m " << hmac->ftype_ << " " << hex_bytes(hmac->ftype_, n_bits[0]) << std::endl;
    if (n_bits[1] != 0) cout << "\033[0;45;30m macSA:\033[0m " << hmac->macSA_ << " " << hex_bytes(hmac->macSA_, n_bits[1]) << std::endl;
    if (n_bits[2] != 0) cout << "\033[0;45;30m macDA:\033[0m " << hmac->macDA_ << " " << hex_bytes(hmac->macDA_, n_bits[2]) << std::endl;
    if (n_bits[3] != 0) cout << "\033[0;45;30m hdr_type:\033[0m " << hmac->hdr_type_ << " " << hex_bytes(hmac->hdr_type_, n_bits[3]) << std::endl;
    if (n_bits[4] != 0) cout << "\033[0;45;30m txtime:\033[0m " << hmac->txtime_ << " " << hex_bytes(hmac->txtime_) << std::endl;
    if (n_bits[5] != 0) cout << "\033[0;45;30m sstime:\033[0m " << hmac->sstime_ << " " << hex_bytes(hmac->sstime_) << std::endl;
    if (n_bits[6] != 0) cout << "\033[0;45;30m padding:\033[0m " << hmac->padding_ << " " << hex_bytes(hmac->padding_, n_bits[6]) << std::endl;
    //cout << "\033[0;41;30m packerMAC::printMyHdrField WARNING \033[0m, Field index " << field << " does not exist or its printing is not implemented." << std::endl;
}

