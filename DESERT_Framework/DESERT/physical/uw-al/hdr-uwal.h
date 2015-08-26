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
 * @file   hdr-uwal.h
 * @author Riccardo Masiero
 * @version 1.0.0
 * 
 * \brief Header of the class providing the Uwal header's description.
 */

#ifndef HDR_UWAL_H
#define HDR_UWAL_H

#include <packet.h>

#define HDR_UWAL(p) (hdr_uwal::access(p))
#define MAX_BIN_PKT_ARRAY_LENGTH 2240
#define MAX_DUMMY_STRING_LENGTH 2240

extern packet_t PT_UWAL;

/**
 * <i>hdr_uwal</i> describes the packet header used by <i>Uwal</i> objects.
 */
typedef struct hdr_uwal {
    
    // This members are always needed by NS 
    static int offset_;
    inline static int& offset() { return offset_; }
    inline static struct hdr_uwal* access(const Packet* p) {
	    return (struct hdr_uwal*)p->access(offset_);
    }

    // Custom header fields
    // Fields that can be encoded and sent over the channel
    uint8_t srcID_;   /**< ID of the packet's source node. */    
    unsigned int pktID_;   /**< ID of the packet. */
    uint16_t framePayloadOffset_; /**< Offset of the frame payload. */
    uint8_t Mbit_; /**< M bit: if set to 0 the current frame is the last or the only one; if set to 1 the current frame is not the last. */
    char dummyStr_[MAX_DUMMY_STRING_LENGTH];   /**< array containing a dummy string. */
   
    // Fields to handle only locally (actually, this is the information to be sent over and retrieved from the channel ad modem payload...)
    char binPkt_[MAX_BIN_PKT_ARRAY_LENGTH];  /**< array to store binary data as encoded from or to be decoded to this NS-Miracle packet header (NOTE: this CANNOT be fragmented) and the active packers linked to packer. @see classes packer and uwmphy_modem */
    uint32_t binPktLength_; /**< number of chars in binPkt_ to consider. */
    uint32_t binHdrLength_; /**< number of chars in binPkt_ to consider as header. */
    
    /**
     * Reference to the srcID_ variable.
     */
    inline uint8_t& srcID() {
        return srcID_;
    }
    
    /**
     * Reference to the pktID_ variable.
     */
    inline unsigned int& pktID() {
        return pktID_;
    }
    
    /**
     * Reference to the frameOffset_ variable.
     */
    inline uint16_t& framePayloadOffset() {
        return framePayloadOffset_;
    }
    
    /**
     * Reference to the Mbit_ variable.
     */
    inline uint8_t& Mbit() {
         return Mbit_;
    }
    
    /**
     * Return the pointer to the dummyStr_ array.
     */
    inline char* dummyStr() {
        return dummyStr_;
    }
    
    /**
     * Return to the binPkt_ array pointer.
     */
    inline char* binPkt() {
        return binPkt_;
    }
    
    /**
     * Reference to the binPktLength_ variable.
     */
    inline uint32_t& binPktLength() {
        return binPktLength_;
    }
    
    /**
     * Reference to the binHdrLength_ variable.
     */
    inline uint32_t& binHdrLength() {
        return binHdrLength_;
    }
    
} hdr_uwal;

#endif // HDR_UWAL_H

