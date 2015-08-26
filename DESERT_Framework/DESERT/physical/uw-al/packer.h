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
 * @file packer.h
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Header of the class responsible to map an NS-Miracle packet into a bit stream, and vice-versa.
 */

#ifndef PACKER_H
#define PACKER_H

#include "hdr-uwal.h"

#include <packet.h>
#include <math.h>

#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <bitset>

/**
 * Class exploited by the Uwal module to map an NS-Miracle packet into a bit stream, and vice-versa.
 */
class packer : public TclObject {
public:

    /** 
     * Class constructor.
     * 
     */
    packer(bool);

    /** 
     * Class destructor.
     * 
     */
    ~packer();

    /** 
     * Method to map tcl commands into c++ methods.
     * 
     * @param argc number of arguments in <em> argv </em> 
     * @param argv array of arguments where <em> argv[3] </em> is the tcl command name and <em> argv[4, 5, ...] </em> are the parameters for the corresponding c++ method.
     */
    int command(int, const char*const*);

    inline size_t getPayloadBytesLength() {
        return payload_length;
    }

    inline size_t getHdrBytesLength() {
        return hdr_length;
    }

    size_t getPayloadBinLength();

    size_t getHdrBinLength();

    /**
     * Method to map an NS-Miracle packet into a legal modem payload (i.e., a string of binary characters) when one among the following protocol stack is in use (see UWMPhy_modem::stack): 
     *  - STACK1, that includes:
     *            - UWCBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA/MFSK_WHOI_MM,or subsets of it;
     *            - UWCBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA-CSMA/MFSK_WHOI_MM,or subsets of it;
     *            - UWVBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA/MFSK_WHOI_MM,or subsets of it;
     *            - UWVBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA-CSMA/MFSK_WHOI_MM,or subsets of it.
     *  - STACK2, that includes:
     *            - UWCBR/UWUDP/UWSUN/UWIP/UWMLL/ALOHA/MFSK_WHOI_MM,or subsets of it;
     *            - UWVBR/UWUDP/UWSUN/UWIP/UWMLL/ALOHA-CSMA/MFSK_WHOI_MM,or subsets of it. 
     *
     * NOTE: With the currently implemented map:
     *  - we do not guarentee to support ARP message exchanges at the MLL layer (i.e., we suggest to manually set the ARP tables 
     *    in the tcl files)
     * 
     * @see packer_IM::pack1
     * @see packer_IM::pack2  
     * @param p pointer to the NS-Miracle packet that must be mapped into a modem payload.
     * @return res, a binary string to be sent acoustically as payload of the next modem packet to transmit.
     */
    std::string packPayload(Packet*);

    std::string packHdr(Packet*);

    /**
     * Method to de-map a legal modem payload (i.e., a string of binary characters) into an NS-Miracle packet, when one among the following protocol stack is in use (see UWMPhy_modem::stack): 
     *  - STACK1, that includes: 
     *            UWCBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA/S2C_EvoLogics,or subsets of it
     *            UWCBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA-CSMA/S2C_EvoLogics,or subsets of it
     *            UWVBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA/S2C_EvoLogics,or subsets of it
     *            UWVBR/UWUDP/UWStaticRouting/UWIP/UWMLL/ALOHA-CSMA/S2C_EvoLogics,or subsets of it
     *  - STACK2, that includes: 
     *            UWCBR/UWUDP/UWSUN/UWIP/UWMLL/ALOHA/S2C_EvoLogics,or subsets of it
     *            UWCBR/UWUDP/UWSUN/UWIP/UWMLL/ALOHA-CSMA/S2C_EvoLogics,or subsets of it
     *            UWVBR/UWUDP/UWSUN/UWIP/UWMLL/ALOHA/S2C_EvoLogics,or subsets of it
     *            UWVBR/UWUDP/UWSUN/UWIP/UWMLL/ALOHA-CSMA/S2C_EvoLogics,or subsets of it
     * 
     * @see packer_IM::pack
     * @see packer_IM::unpack1
     * @see packer_IM::unpack2
     * @param s the node ID from which the last received packet arrives. 
     * @param d the node ID to which the last received packet is sent.
     * @param src the payload of the last packet received from the modem.
     * @return p the NS-Miracle packet recovered from the information contained in \e s, \e d and \e src.
     */
    Packet* unpackPayload(Packet*);

    Packet* unpackHdr(Packet*);

    void printMap();

    template <typename T>
    static T restoreSignedValue(T _header_field, const uint32_t& _num_compressed_bits) {
        if (_num_compressed_bits < sizeof (_header_field) * 8) {
            bitset<sizeof (_header_field) * 8 > my_bitset(_header_field);
            if (my_bitset[_num_compressed_bits - 1] == 1) {
                for (size_t i = _num_compressed_bits; i < sizeof (_header_field) * 8; ++i) {
                    my_bitset.set(i, 1);
                }
            }
            return static_cast<T> (my_bitset.to_ulong());
        } else {
            return _header_field;
        }
    }

    /** 
     * Function used for log and debug purposes (in order to print binary strings, even if they contain special characters)
     * @param str  the binary string to print
     * @return str_out the string to print (special characters are reported in hex form)
     */
    static std::string hexdump_nice(std::string);

    static std::string hexdump_nice(const char*, size_t);

    static std::string hexdump(std::string);

    static std::string hex_bytes(float);
    static std::string hex_bytes(double);
    static std::string hex_bytes(const char&, const uint32_t&);
    static std::string hex_bytes(const int8_t&, const uint32_t&);
    static std::string hex_bytes(const int16_t&, const uint32_t&);
    static std::string hex_bytes(const int32_t&, const uint32_t&);
    static std::string hex_bytes(const int64_t&, const uint32_t&);
    static std::string hex_bytes(const uint8_t&, const uint32_t&);
    static std::string hex_bytes(const uint16_t&, const uint32_t&);
    static std::string hex_bytes(const uint32_t&, const uint32_t&);
    static std::string hex_bytes(const uint64_t&, const uint32_t&);

    static std::string hexdump(const char*, size_t);

    static std::string bindump(std::string);

    static std::string bindump(const char*, size_t);

protected:

    std::vector<size_t> n_bits; /**< Vector of elements containing the indication of the number of bits to consider for each header field. */

    int debug_; /**< Flag to enable debug messages. */

    virtual void init();

    virtual size_t packMyHdr(Packet*, unsigned char*, size_t);

    virtual size_t unpackMyHdr(unsigned char*, size_t, Packet*);

    virtual void printMyHdrMap();

    virtual void printMyHdrFields(Packet*);

    virtual void printMyHdrField(Packet*, int);

    size_t getMyHdrBinLength();

    /**
     * Method used to retrieve a given variable from a certain number of bits contained in a buffer of chars.
     * 
     * @param[in] buffer pointer to a buffer of characters.
     * @param[in] offset position from which to start reading the buffer of characters.
     * @param[in,out] val pointer to variable to retrieve (passed as reference).
     * @param h the number of bits to read.
     * @return \e h, namely the number of read bits.
     */
    size_t get(unsigned char *buffer, size_t offset, void *val, size_t h);

    /**
     * Method used to map in a certain number of bits, contained in a buffer of chars, a given variable.
     * 
     * @param[in] buffer pointer to a buffer of characters.
     * @param[in] offset position from which to start writing to the buffer of characters.
     * @param[in,out] val pointer to the variable to map (passed as reference).
     * @param h the number of bits to use for the mapping.
     * @return \e h, namely the number of written bits.
     */
    size_t put(unsigned char *buffer, size_t offset, void *val, size_t h);

private:

    std::vector<packer*> activePackers; /**< Vector of elements containing the pointers to the active packers (i.e., the derived classed of packer actually in charge of pack and unpack packets. These are set by the tcl user through the command "addPacker". See packer::command). */

    size_t payload_length; /**< The minimum number of elements that a buffer of char must have in order to store the fields to be coded by the activePackers into a string of bytes. */

    size_t hdr_length; /**< The minimum number of elements that a buffer of char must have in order to store the header fields of the AL. @see: hdr-uwal.h */

    bool printAllFields;

    size_t SRC_ID_Bits; /** Bit length of the srcID_ field to be put in the header stream of bits. */
    size_t PKT_ID_Bits; /** Bit length of the pktID_ field to be put in the header stream of bits. */
    size_t FRAME_OFFSET_Bits; /** Bit length of the frameID_ field to be put in the header stream of bits. */
    size_t M_BIT_Bits; /** Bit length of the Mbit_ field to be put in the header stream of bits. */
    size_t DUMMY_CONTENT_Bits; /** Bit length of the dummy content to be put in the header stream of bits. */
};

#endif
