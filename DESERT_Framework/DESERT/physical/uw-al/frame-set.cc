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
 * @file frame-set.cc
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Implementation of the classes defining the frame elements (key and set) exploited by Uwal objects to reasseble received fragments.
 */

#include "frame-set.h"
#include "packer.h"

#include <limits.h>

typedef unsigned char BARR_ELTYPE;

/**
 * @see packer for an explaination of the following MACROs
 */

#define BARR_ELBITS (CHAR_BIT * sizeof(BARR_ELTYPE))

#define BARR_ELNUM(N) ((N) / BARR_ELBITS)

#define BARR_BITNUM(N) ((N) % BARR_ELBITS)

#define BARR_SET(barr, N) (((BARR_ELTYPE*)(barr))[BARR_ELNUM(N)] |= (BARR_ELTYPE)1 << BARR_BITNUM(N))

#define BARR_TEST(barr, N) (((BARR_ELTYPE*)(barr))[BARR_ELNUM(N)] & ((BARR_ELTYPE)1 << BARR_BITNUM(N)))

RxFrameSetKey::RxFrameSetKey(uint8_t srcID, unsigned int pktID) {
    srcID_ = srcID;
    pktID_ = pktID;
}

RxFrameSetKey::~RxFrameSetKey() {
}

bool RxFrameSetKey::operator<(RxFrameSetKey key2) const {

    if (srcID_ == key2.srcID_)
        return pktID_ < key2.pktID_;
    else
        return srcID_ < key2.srcID_;

}

std::string RxFrameSetKey::displayKey() const {

    std::string str;
    std::stringstream sstr("");
    sstr << "srcID_:" << (int) srcID_ << ",pktID_:" << pktID_;
    sstr >> str;

    return str;
}

RxFrameSet::RxFrameSet() {

    memset(binPayload_, '\0', MAX_BIN_PAYLOAD_ARRAY_LENGTH);
    memset(binPayloadCheck_, '\0', MAX_BIN_PAYLOAD_CHECK_ARRAY_LENGTH);
    tot_length_ = -1;
    curr_length_ = 0;
    t_last_rx_frame_ = 0;
    error_ = false;
}

RxFrameSet::~RxFrameSet() {
}

std::string RxFrameSet::displaySet() {

    std::string str("");
    std::stringstream sstr;

    sstr << "binPayload_:" << packer::hexdump_nice(binPayload(false)) << "binPayloadCheck_:" << packer::bindump(binPayloadCheck_, MAX_BIN_PAYLOAD_CHECK_ARRAY_LENGTH) << ",tot_length_:" << tot_length() << ",curr_length_:" << curr_length() << ",t_last_rx_frame_:" << t_last_rx_frame();

    sstr >> str;

    return str;
}

std::string RxFrameSet::binPayload(bool extract) const {

    std::string res;

    if (extract) {

        if (tot_length_ > -1)
            res.assign(binPayload_, tot_length_);
        else
            res = "";
    } else {

        res.assign(binPayload_, MAX_BIN_PAYLOAD_ARRAY_LENGTH);
    }

    return res;
}

void RxFrameSet::UpdateRxFrameSet(char* frame, size_t offset, size_t length, int tot_length, double time) {

    if (offset + length > MAX_BIN_PAYLOAD_ARRAY_LENGTH) {
        cout << "ERROR in RxFrameSet::UpdateRxFrameSet, offset: " << offset << ", length:" << length << ", but MAX_BIN_PAYLOAD_ARRAY_LENGTH:" << MAX_BIN_PAYLOAD_ARRAY_LENGTH << endl;
        return;
    }

    if (tot_length > MAX_BIN_PAYLOAD_ARRAY_LENGTH) {
        cout << "ERROR in RxFrameSet::UpdateRxFrameSet, tot_length: " << tot_length << ", but MAX_BIN_PAYLOAD_ARRAY_LENGTH:" << MAX_BIN_PAYLOAD_ARRAY_LENGTH << endl;
        return;
    }

    if (BARR_TEST(binPayloadCheck_, offset)) {
        cout << "WARNING in RxFrameSet::UpdateRxFrameSet, duplicated or overlapping frame has been received." << endl;
        return;
    }

    for (int i = offset; i < offset + length; i++) {
        BARR_SET(binPayloadCheck_, i);
    }

    memcpy(binPayload_ + offset, frame, length);

    curr_length_ += length;

    t_last_rx_frame_ = time;

    if (tot_length > -1) {

        tot_length_ = tot_length;

    }
}
