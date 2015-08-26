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
//

/**
 * @file   uwApplication_cmn_header.h
 * @author Federico Favaro
 * @version 1.1.0
 *
 * \brief Provides the headers of the data packet
 *
 */

#ifndef UWAPPLICATION_CMH_HEADER_H
#define	UWAPPLICATION_CMH_HEADER_H

#include <module.h>
#include <packet.h>
#include <pthread.h>

#define MAX_LENGTH_PAYLOAD 4096
#define HDR_DATA_APPLICATION(p) (hdr_DATA_APPLICATION::access(p))     /**< alias defined to access the TRIGGER HEADER */


//pthread_mutex_t mutex_udp = PTHREAD_MUTEX_INITIALIZER;

extern packet_t PT_DATA_APPLICATION; /**< DATA packet type */

/**
 * Content header of TRIGGER packet
 */
typedef struct hdr_DATA_APPLICATION {
    uint16_t sn_;       /**< Serial number of the packet. */
    int rftt_;        /**< Forward Trip Time of the packet. */
    bool rftt_valid_;   /**< Flag used to set the validity of the fft field. */
    uint8_t priority_;     /**< Priority flag: 1 means high priority, 0 normal priority. */
    uint16_t payload_size_; /**< Size (bytes) of the payload */
    char payload_msg[MAX_LENGTH_PAYLOAD]; /**< Message payload*/
    
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the sn_ variable.
     * 
     * @return uint16_t sn_ 
     */
    inline uint16_t& sn() {
        return sn_;
    }
    
   /**
     *  Reference to the rftt_ variable.
     *
     * @return float rftt_    
     */
    inline int& rftt() {
        return (rftt_);
    }
       
    /**
     *  Reference to the rftt_valid_ variable.
     * 
     * @return bool rftt_valid_
     */
    inline bool& rftt_valid() {
        return rftt_valid_;
    }
    
   /**
     *  Reference to the priority variable.
     * 
     * @return char priority_
     */
    inline uint8_t& priority() {
        return priority_;
    }

    inline uint16_t& payload_size() {
        return payload_size_;
    }
    
    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    inline static hdr_DATA_APPLICATION * access(const Packet * p) {
        return (hdr_DATA_APPLICATION*) p->access(offset_);
    }
} hdr_DATA_APPLICATION;


#endif	/* UWAPPLICATION_CMH_HEADER_H */

