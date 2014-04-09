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
 * @file packer.cpp
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Header of the class responsible to map an NS-Miracle packet into a bit stream, and vice-versa.
 */

#include "packer.h"

#include <iostream>
#include <limits.h>
#include <iomanip>

typedef unsigned char BARR_ELTYPE;

#define DEFAULT_DUMMY_CONTENT_LENGTH 256

/**
 * The size, in bits, of a BARR_ELTYPE (unsigned char). Used for code portability.
 */
#define BARR_ELBITS (CHAR_BIT * sizeof(BARR_ELTYPE))
/**
 * MACRO to compute the minimum number of elements that a buffer of BARR_ELTYPEs must have in order to store N bits.
 * 
 * @param N the number of bits to store.
 */
#define BARR_ARRAYSIZE(N) (((N) + BARR_ELBITS-1) / BARR_ELBITS)
/**
 * MACRO to compute the number of the element of a buffer of BARR_ELTYPEs where is stored the bit N.
 * 
 * @param N indication of the bit.
 */
#define BARR_ELNUM(N) ((N) / BARR_ELBITS)
/**
 * MACRO to compute the position where is stored the bit N, within its corresponding element of a buffer of BARR_ELTYPEs .
 * 
 * @param N indication of the bit.
 */
#define BARR_BITNUM(N) ((N) % BARR_ELBITS)
/**
 * MACRO to set to \e 1 the bit N of a buffer of BARR_ELTYPEs.
 * 
 * @param barr pointer to the buffer
 * @param N indication of the bit.
 */
#define BARR_SET(barr, N) (((BARR_ELTYPE*)(barr))[BARR_ELNUM(N)] |= (BARR_ELTYPE)1 << BARR_BITNUM(N))
/**
 * MACRO to set to \e 0 the bit N of a buffer of BARR_ELTYPEs.
 * 
 * @param barr pointer to the buffer
 * @param N indication of the bit.
 */
#define BARR_CLEAR(barr, N) (((BARR_ELTYPE*)(barr))[BARR_ELNUM(N)] &= ~((BARR_ELTYPE)1 << BARR_BITNUM(N)))
/**
 * MACRO to test if the bit N of a buffer of BARR_ELTYPEs is set to \e 1.
 * 
 * @param barr pointer to the buffer
 * @param N indication of the bit.
 */
#define BARR_TEST(barr, N) (((BARR_ELTYPE*)(barr))[BARR_ELNUM(N)] & ((BARR_ELTYPE)1 << BARR_BITNUM(N)))

/**
 * Class to create the Otcl shadow object for an object of the class packer.
 */
static class PackerClass : public TclClass {
public:

    PackerClass() : TclClass("UW/AL/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packer(true));
    }
} class_module_packer;

packer::packer(bool init_)
: n_bits(0), activePackers(0), payload_length(0) {
    debug_ = 0;
    bind("debug_", &debug_);
    printAllFields = false;

    if (init_) {
        SRC_ID_Bits = 8 * sizeof (uint8_t);
        PKT_ID_Bits = 8 * sizeof (unsigned int);
        FRAME_OFFSET_Bits = 8 * sizeof (uint16_t);
        M_BIT_Bits = 1;
        DUMMY_CONTENT_Bits = DEFAULT_DUMMY_CONTENT_LENGTH;

        bind("SRC_ID_Bits", (int*) &SRC_ID_Bits);
        bind("PKT_ID_Bits", (int*) &PKT_ID_Bits);
        bind("FRAME_OFFSET_Bits", (int*) &FRAME_OFFSET_Bits);
        bind("M_BIT_Bits", (int*) &M_BIT_Bits);
        bind("DUMMY_CONTENT_Bits", (int*) &DUMMY_CONTENT_Bits);

        if (debug_) {
            std::cout << "Initialization (from constructor) of n_bits for the UWAL packer " << std::endl;
        }

        n_bits.clear();
        n_bits.push_back(SRC_ID_Bits);
        n_bits.push_back(PKT_ID_Bits);
        n_bits.push_back(FRAME_OFFSET_Bits);
        n_bits.push_back(M_BIT_Bits);
        n_bits.push_back(DUMMY_CONTENT_Bits);
    }
    hdr_length = BARR_ARRAYSIZE(getMyHdrBinLength());
}

packer::~packer() {
}

void packer::init() {
    if (debug_) {
        std::cout << "Re-initialization of n_bits for the UWAL packer " << std::endl;
    }

    n_bits.clear();
    n_bits.push_back(SRC_ID_Bits);
    n_bits.push_back(PKT_ID_Bits);
    n_bits.push_back(FRAME_OFFSET_Bits);
    n_bits.push_back(M_BIT_Bits);
    n_bits.push_back(DUMMY_CONTENT_Bits);
}

int packer::command(int argc, const char*const* argv) {

    if (argc == 2) {
        if (strcmp(argv[1], "packerInit") == 0) {
            init();
            hdr_length = BARR_ARRAYSIZE(getMyHdrBinLength());
            return TCL_OK;
        }
        if (strcmp(argv[1], "printMap") == 0) {
            printMap();
            return TCL_OK;
        }
        if (strcmp(argv[1], "printAllFields") == 0) {
            printAllFields = true;
            return TCL_OK;
        }
    }

    if (argc == 3) {
        if (strcmp(argv[1], "addPacker") == 0) {
            activePackers.push_back((packer*) TclObject::lookup(argv[2]));
            payload_length = BARR_ARRAYSIZE(getPayloadBinLength());
            if (debug_) {
                std::cout << "Packer added, current payload length: " << payload_length << " [Bytes]. Corresponding MAP: " << std::endl;
                activePackers.back()->printMyHdrMap();
            }
            return TCL_OK;
        }
    }
    return TclObject::command(argc, argv);
}

size_t packer::getPayloadBinLength() {
    size_t PayloadBinLength = 0;
    for (std::vector<packer*>::iterator it = activePackers.begin(); it != activePackers.end(); ++it) {
        PayloadBinLength += (*it)->getMyHdrBinLength();
    }
    return PayloadBinLength;
}

size_t packer::getHdrBinLength() {
    return getMyHdrBinLength();
}

std::string packer::packHdr(Packet* p) {
    unsigned char *buf = new unsigned char[hdr_length];
    memset(buf, '\0', hdr_length);

    size_t offset = 0;
    packMyHdr(p, buf, offset);
    hdr_uwal* hal = HDR_UWAL(p);
    memset(hal->binPkt(), '\0', hdr_length);
    hal->binHdrLength() = 0;

    if (!(hdr_length > MAX_BIN_PKT_ARRAY_LENGTH)) {

        memcpy(hal->binPkt(), buf, hdr_length);
        hal->binHdrLength() = hdr_length;
        hal->binPktLength() += hdr_length;

        if (hal->binPktLength() != hdr_length && hal->binPktLength() != hdr_length + payload_length) {
            std::cout << "\033[0;0;31m" << " ERROR" << "\033[0m" << std::endl;;
            cout << "in packer::packHdr -> pkt size set to: " << hal->binPktLength() << ", whilst header length is: " << hdr_length << " and DEFAULT payload length is: " << payload_length << endl;
            exit(-1);
        }

    } else {
        std::cout << "\033[0;0;31m" << " ERROR" << "\033[0m" << std::endl;;
        cout << "in packer::packHdr -> hdr size returned by packer is " << hdr_length << ", higher than MAX_BIN_PKT_ARRAY_LENGTH: " << MAX_BIN_PKT_ARRAY_LENGTH << ". Hdr is not serialized." << endl;
    }

    std::string res;
    res.assign((const char *) buf, hdr_length);
    delete[] buf;

    if (debug_) {
        std::cout << "\033[0;47;30m" << " TX" << "\033[0m" << std::endl;
        cout << "--> Bin data header generated by packer:" << hexdump(hal->binPkt(), hdr_length) << endl;
        cout << "--> Header length (unsigned char):" << hdr_length << endl;
    }

    return res;
}

std::string packer::packPayload(Packet* p) {
    std::string res;
    hdr_uwal* hal = HDR_UWAL(p);
    memset(hal->binPkt() + hdr_length, '\0', MAX_BIN_PKT_ARRAY_LENGTH - hdr_length);
    size_t offset;
    if (!activePackers.empty()) {
        //unsigned char *buf = new unsigned char[payload_length];
        //memset(buf, '\0', payload_length);
        unsigned char *buf = new unsigned char[MAX_BIN_PKT_ARRAY_LENGTH - hdr_length];
        memset(buf, '\0', MAX_BIN_PKT_ARRAY_LENGTH - hdr_length);
        
        offset = 0;
        for (std::vector<packer*>::iterator it = activePackers.begin(); it != activePackers.end(); ++it) {
            offset = (*it)-> packMyHdr(p, buf, offset);
        }

        if (!(BARR_ARRAYSIZE(offset) > MAX_BIN_PKT_ARRAY_LENGTH - hdr_length)) {
            memcpy(hal->binPkt() + hdr_length, buf, offset);
            hal->binPktLength() += BARR_ARRAYSIZE(offset);
            if (hal->binPktLength() != hdr_length + payload_length) {
                if (debug_ > 1){
                    std::cout << "\033[0;0;31m" << " WARNING - REDUCED PACKET" << "\033[0m" << std::endl;;
                    std::cout << "in packer::packPayload -> pkt size set to: " << hal->binPktLength() << ", header length is: " << hdr_length << " and payload length is: " << BARR_ARRAYSIZE(offset) <<" , DEFAULT payload length is " << payload_length << std::endl;
                }
            }
        } else {
            std::cout << "\033[0;0;31m" << " ERROR" << "\033[0m" << std::endl;;
            std::cout << "in packer::packPayload -> payload size returned by packer is: " << offset << ", higher than: " << (MAX_BIN_PKT_ARRAY_LENGTH - hdr_length) << " Which is the max pkt length: " << MAX_BIN_PKT_ARRAY_LENGTH << " minus the header length: " << hdr_length << ". Payload is not serialized." << std::endl;
        }

/*      if (!(payload_length > MAX_BIN_PKT_ARRAY_LENGTH - hdr_length)) {
            memcpy(hal->binPkt() + hdr_length, buf, payload_length);
            hal->binPktLength() += payload_length;
            if (hal->binPktLength() != payload_length && hal->binPktLength() != hdr_length + payload_length) {
                std::cout << "\033[0;0;31m" << " ERROR" << "\033[0m" << std::endl;;
                cout << "in packer::packHdr -> pkt size set to: " << hal->binPktLength() << ", whilst header length is: " << hdr_length << " and payload length is: " << payload_length << endl;
                exit(-1);
            }
        } else {
            std::cout << "\033[0;0;31m" << " ERROR" << "\033[0m" << std::endl;;
            std::cout << "in packer::packPayload -> payload size returned by packer is: " << payload_length << ", higher than: " << (MAX_BIN_PKT_ARRAY_LENGTH - hdr_length) << " Which is the max pkt length: " << MAX_BIN_PKT_ARRAY_LENGTH << " minus the header length: " << hdr_length << ". Payload is not serialized." << std::endl;
        }
        // conversion from const char* to string
        res.assign((const char *) buf, payload_length);
        delete[] buf;
*/
        // conversion from const char* to string
        res.assign((const char *) buf, BARR_ARRAYSIZE(offset));
        delete[] buf;
    } else {
        if (payload_length != 0) {
            std::cout << "\033[0;0;31m" << " WARNING" << "\033[0m" << std::endl;
            std::cout << "in packer::packPayload -> payload activePackers empty but payload_length: " << payload_length << ". Empty string is returned" << std::endl;
        }
        res.assign("");
    }

    if (debug_) {
        std::cout << "\033[0;47;30m" << " TX" << "\033[0m" << std::endl;
        std::cout << "--> Bin data payload generated by packer:" << hexdump(hal->binPkt() + hdr_length, BARR_ARRAYSIZE(offset)) << std::endl;
        std::cout << "--> Payload length (unsigned char):" << BARR_ARRAYSIZE(offset) << std::endl;
    }

/*    if (debug_) {

        std::cout << "\033[0;47;30m" << " TX" << "\033[0m" << std::endl;
        std::cout << "--> Bin data payload generated by packer:" << hexdump(hal->binPkt() + hdr_length, payload_length) << std::endl;
        std::cout << "--> Payload length (unsigned char):" << payload_length << std::endl;
    }
*/
    return res;
}

Packet* packer::unpackHdr(Packet* p) {
    hdr_uwal* hal = HDR_UWAL(p);
    hal->binHdrLength() = hdr_length;

    if (debug_) {
        std::cout << "\033[0;47;30m" << " RX" << "\033[0m" << std::endl;
        std::cout << "<-- Bin data header received by packer:" << hexdump(hal->binPkt(), hdr_length) << std::endl;
    }

    size_t offset = 0;

    unpackMyHdr((unsigned char *) hal->binPkt(), offset, p);

    return p;
}

Packet* packer::unpackPayload(Packet* p) {
    hdr_uwal* hal = HDR_UWAL(p);

    if (debug_) {
        std::cout << "\033[0;47;30m" << " RX" << "\033[0m" << std::endl;
        //std::cout << "<-- Bin data payload received by packer:" << hexdump(hal->binPkt() + hal->binHdrLength(), payload_length) << std::endl;
        hdr_cmn *ch = HDR_CMN(p);
        std::cout << "<-- Bin data payload received by packer:" << hexdump(hal->binPkt() + hal->binHdrLength(), ch->size()) << std::endl;
    }

    if (activePackers.empty()) {
        if (hal->binPktLength() - hal->binHdrLength() != 0) {
            std::cout << "\033[0;0;31m" << " WARNING" << "\033[0m" << std::endl;
            std::cout << "in packer::unpackPayload -> payload activePackers empty but binary payload: " << hexdump(hal->binPkt() + hal->binHdrLength(), hal->binPktLength() - hal->binHdrLength()) << ". Packet in ERROR is returned" << std::endl;
            hdr_cmn *ch = HDR_CMN(p);
            ch->error() = 1;
            return p;
        }
    }

    if (hal->binPktLength() - hal->binHdrLength() < payload_length) {
        if (debug_>1){
            std::cout << "\033[0;0;31m" << " WARNING" << "\033[0m" << std::endl;
            std::cout << "in packer::unpackPayload -> the payload size computed from the HDR_UWAL corresponding fields is: " << hal->binPktLength() - hal->binHdrLength() << " which is not the DEFAULT expected size: " << payload_length << ". Is it a REDUCED packet?" << std::endl;
        }
        
//        hdr_cmn *ch = HDR_CMN(p);
//        ch->error() = 1;
//        return p;
    }

    size_t offset = 0;
    for (std::vector<packer*>::iterator it = activePackers.begin(); it != activePackers.end(); ++it) {
        offset = (*it)->unpackMyHdr((unsigned char *) (hal->binPkt() + hal->binHdrLength()), offset, p);
    }
  
    //Now we have unpacked the whole packet, set the size according to the total received bits
    HDR_CMN(p)->size() = offset / (sizeof(char)*8);
    if (debug_){
        std::cout << "ch->size() after unpack is: " << HDR_CMN(p)->size() << std::endl;   
    }
    
    return p;
}

void packer::printMap() {
    std::cout << "\n\033[0;47;30m" << " MAP in USE" << "\033[0m" << std::endl;
    std::cout << "Header length: " << hdr_length << " [Bytes]"<< std::endl;
    printMyHdrMap();
    std::cout << "Payload length: " << payload_length << " [Bytes]"<< std::endl;
    for (std::vector<packer*>::iterator it = activePackers.begin(); it != activePackers.end(); ++it) {
        (*it)->printMyHdrMap();
    }
}

size_t packer::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {
    // Pointer to the AL packer header
    hdr_uwal* alh = HDR_UWAL(p);

    int field_idx = 0;

    offset += put(buf, offset, &(alh->srcID_), n_bits[field_idx++]);
    offset += put(buf, offset, &(alh->pktID_), n_bits[field_idx++]);
    offset += put(buf, offset, &(alh->framePayloadOffset_), n_bits[field_idx++]);
    offset += put(buf, offset, &(alh->Mbit_), n_bits[field_idx++]);
    offset += put(buf, offset, &(alh->dummyStr_), n_bits[field_idx++]);

    if (debug_) {
        std::cout << "\033[0;47;30m" << " TX AL packer hdr" << "\033[0m" << std::endl;
        printMyHdrFields(p);
    }

    return offset;
}

size_t packer::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {
    // Pointer to the AL packer header
    hdr_uwal* alh = HDR_UWAL(p);

    int field_idx = 0;

    memset(&(alh->srcID_), 0, sizeof (alh->srcID_));
    offset += get(buf, offset, &(alh->srcID_), n_bits[field_idx++]);

    memset(&(alh->pktID_), 0, sizeof (alh->pktID_));
    offset += get(buf, offset, &(alh->pktID_), n_bits[field_idx++]);

    memset(&(alh->framePayloadOffset_), 0, sizeof (alh->framePayloadOffset_));
    offset += get(buf, offset, &(alh->framePayloadOffset_), n_bits[field_idx++]);

    memset(&(alh->Mbit_), 0, sizeof (alh->Mbit_));
    offset += get(buf, offset, &(alh->Mbit_), n_bits[field_idx++]);

    memset(&(alh->dummyStr_), '\0', MAX_DUMMY_STRING_LENGTH);
    offset += get(buf, offset, &(alh->dummyStr_), n_bits[field_idx++]);

    if (debug_) {
        std::cout << "\033[0;47;30m" << " RX AL packer hdr" << "\033[0m" << std::endl;
        printMyHdrFields(p);
    }

    return offset;
}

void packer::printMyHdrMap() {
    std::cout << "\033[0;47;30m" << " Packer Name " << "\033[0m" << " UWAL" << std::endl;
    std::cout << "\033[0;47;30m 1st field " << "\033[0m" << " src ID: " << SRC_ID_Bits << " bits" << std::endl;
    std::cout << "\033[0;47;30m 2st field " << "\033[0m" << " pkt IkD: " << PKT_ID_Bits << " bits" << std::endl;
    std::cout << "\033[0;47;30m 3st field " << "\033[0m" << " frame offset: " << FRAME_OFFSET_Bits << " bits" << std::endl;
    std::cout << "\033[0;47;30m 4st field " << "\033[0m" << " M bit: " << M_BIT_Bits << " bits" << std::endl;
    std::cout << "\033[0;47;30m 5st field " << "\033[0m" << " dummy content: " << DUMMY_CONTENT_Bits << " bits" << std::endl;
    return;
}

void packer::printMyHdrFields(Packet* p) {
    for (int i = 0, l = n_bits.size(); i < l; i++) {
        if (printAllFields || n_bits[i] != 0)
            printMyHdrField(p, i);
    }
}

void packer::printMyHdrField(Packet* p, int field) {
    hdr_uwal* alh = HDR_UWAL(p);

    //size_t printedFieldLength = 2048;
    //char printedField[printedFieldLength];
    //memset(printedField, '\0', printedFieldLength);
    
    switch (field) {
        case 0:
            std::cout << "\033[0;47;30m src ID:\033[0m " << static_cast<uint32_t> (alh->srcID()) << " " << hex_bytes(alh->srcID(), n_bits[0]) << std::endl;
            break;
        case 1:
            std::cout << "\033[0;47;30m pkt ID:\033[0m " << static_cast<uint32_t> (alh->pktID()) << " " << hex_bytes(alh->pktID(), n_bits[1]) << std::endl;
            break;
        case 2:
            std::cout << "\033[0;47;30m frame offset:\033[0m " << static_cast<uint32_t> (alh->framePayloadOffset()) << " " << hex_bytes(alh->framePayloadOffset(), n_bits[2]) << std::endl;
            break;
        case 3:
            std::cout << "\033[0;47;30m M bit:\033[0m " << static_cast<uint32_t> (alh->Mbit()) << " " << hex_bytes(alh->Mbit(), n_bits[3]) << std::endl;
            break;
        case 4:
            std::cout << "\033[0;47;30m dummy content:\033[0m " << hexdump(alh->dummyStr()) << std::endl;
            break;
        default:
            std::cout << "\033[0;41;30m WARNING \033[0m, Field number " << (field + 1) << " does not exist or its printing is not implemented" << std::endl;
    }
    //return printedField;
}

size_t packer::getMyHdrBinLength() {

    size_t total_bits = 0;

    for (int i = 0, l = n_bits.size(); i < l; i++)
        total_bits += n_bits[i];

    return total_bits;
}

size_t packer::get(unsigned char *buffer, size_t offset, void *val, size_t h) {
    for (size_t j = 0; j < h; j++)
        if (BARR_TEST(buffer, (offset + j)))
            BARR_SET(val, j);
        else
            BARR_CLEAR(val, j);

    return h;
}

size_t packer::put(unsigned char *buffer, size_t offset, void *val, size_t h) {
    for (size_t j = 0; j < h; j++)
        if (BARR_TEST(val, j))
            BARR_SET(buffer, (offset + j));
        else
            BARR_CLEAR(buffer, (offset + j));

    return h;
}

std::string packer::hexdump_nice(std::string str) {
    const char *data = str.c_str();
    size_t len = str.size();

    return hexdump_nice(data, len);
}

std::string packer::hexdump_nice(const char *data, size_t len) {
    std::string str_out = "";

    for (size_t i = 0; i < len; i++) {
        if (std::isalnum(data[i]) || std::ispunct(data[i]))
            str_out += data[i];
        else {
            std::stringstream sstr("");
            sstr << "[";
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (unsigned int) (unsigned char) data[i] << std::dec << "]";
            str_out += sstr.str();
        }
    }
    return str_out;
}

std::string packer::hexdump(std::string str) {
    const char *data = str.c_str();
    size_t len = str.size();
    return hexdump(data, len);
}

std::string packer::hexdump(const char *data, size_t len) {
    std::string str_out = "";

    for (size_t i = 0; i < len; i++) {
        std::stringstream sstr("");
        sstr << "[";
        sstr.fill('0');
        sstr.width(2);
        sstr << std::hex << (unsigned int) (unsigned char) data[i] << std::dec << "]";
        str_out += sstr.str();
    }
    return str_out;
}

std::string packer::hex_bytes(float value) {
    unsigned char *p = (unsigned char *) &value;
    std::stringstream sstr("");
    //cout << "packer::hex_bytes:Value to print:" << value << std::endl;
    sstr << "[";
    for (int i = 0; i < sizeof (value); i++) {
        sstr.fill('0');
        sstr.width(2);
        sstr << std::hex << static_cast<uint32_t> (p[i]);
    }
    sstr << std::dec << "]";

    return sstr.str();
}

std::string packer::hex_bytes(double value) {
    unsigned char *p = (unsigned char *) &value;
    std::stringstream sstr("");
    //cout << "packer::hex_bytes:Value to print:" << value << std::endl;
    sstr << "[";
    for (int i = 0; i < sizeof (value); i++) {
        sstr.fill('0');
        sstr.width(2);
        sstr << std::hex << static_cast<uint32_t> (p[i]);
    }
    sstr << std::dec << "]";

    return sstr.str();
}

std::string packer::hex_bytes(const char& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const int BINARY_MASK = 0xff;
    const int num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << value << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    ;
    switch (num_bytes) {
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const int8_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const int8_t BINARY_MASK = 0xff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << static_cast<int32_t>(value) << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const int16_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const int16_t BINARY_MASK = 0x00ff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << static_cast<int32_t>(value) << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 2:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 8) & BINARY_MASK);
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const int32_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const int32_t BINARY_MASK = 0x000000ff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << value << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 4:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 24) & BINARY_MASK);
        case 3:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 16) & BINARY_MASK);
        case 2:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 8) & BINARY_MASK);
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const int64_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const int64_t BINARY_MASK = 0x00000000000000ff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << value << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 8:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 56) & BINARY_MASK);
        case 7:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 48) & BINARY_MASK);
        case 6:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 40) & BINARY_MASK);
        case 5:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 32) & BINARY_MASK);
        case 4:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 24) & BINARY_MASK);
        case 3:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 16) & BINARY_MASK);
        case 2:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 8) & BINARY_MASK);
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const uint8_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const uint8_t BINARY_MASK = 0xff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << static_cast<uint32_t>(value) << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const uint16_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const uint16_t BINARY_MASK = 0x00ff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << static_cast<uint32_t>(value) << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 2:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 8) & BINARY_MASK);
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const uint32_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const uint32_t BINARY_MASK = 0x000000ff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << value << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 4:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 24) & BINARY_MASK);
        case 3:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 16) & BINARY_MASK);
        case 2:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 8) & BINARY_MASK);
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::hex_bytes(const uint64_t& value, const uint32_t& len_bits) {
    assert(len_bits <= sizeof (value) * 8);
    const uint64_t BINARY_MASK = 0x00000000000000ff;
    const uint32_t num_bytes = ceil(len_bits / 8.0);
    std::stringstream sstr("");
    //    cout << "packer::hex_bytes:Value to print:" << value << ", num bits: " << len_bits << ", num bytes: " << num_bytes << std::endl;
    sstr << "[";
    switch (num_bytes) {
        case 8:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 56) & BINARY_MASK);
        case 7:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 48) & BINARY_MASK);
        case 6:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 40) & BINARY_MASK);
        case 5:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 32) & BINARY_MASK);
        case 4:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 24) & BINARY_MASK);
        case 3:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 16) & BINARY_MASK);
        case 2:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << ((value >> 8) & BINARY_MASK);
        case 1:
            sstr.fill('0');
            sstr.width(2);
            sstr << std::hex << (value & BINARY_MASK);
            break;
        default:
            sstr << std::dec << std::endl;
//            sstr << std::dec << "Number of bytes to print higher than " << sizeof (value) << ", not supported." << std::endl;
    }
    sstr << "]";
    return sstr.str();
}

std::string packer::bindump(std::string str) {
    size_t len = str.size();
    const char *data = str.c_str();

    return bindump(data, len);
}

std::string packer::bindump(const char *data, size_t len) {
    std::string str_out = "";
    std::string str_tmp;

    for (size_t i = 0; i < len; i++) {
        str_tmp = "";
        for (int j = 0; j < CHAR_BIT; j++) {
            if BARR_TEST(data, i * CHAR_BIT + j)
                str_tmp += '1';
            else
                str_tmp += '0';
        }
        str_out += ("[" + str_tmp + "]");
    }

    return str_out;
}
