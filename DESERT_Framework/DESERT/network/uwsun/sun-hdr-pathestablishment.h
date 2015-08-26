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
 * @file   sun-hdr-pathestablishment.h
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Provides the Path Establishment Messages header description.
 *
 */

#ifndef HDR_SUN_PATH_EST_H
#define	HDR_SUN_PATH_EST_H

#include "sun-ipr-common-structures.h"

#include <packet.h>

#define HDR_SUN_PATH_EST(p) (hdr_sun_path_est::access(p))
#define PATH_SEARCH 1
#define PATH_ANSWER 2
#define DATA_ACK    3
#define PATH_ERROR  4

extern packet_t PT_SUN_PATH_EST;

typedef u_int8_t packet_path_est;

/**
 * <i>hdr_sun_path_est</i> describes path establishment packets used by <i>UWSUN</i>
 */
typedef struct hdr_sun_path_est {
    
    uint8_t sink_associated_;               /**< IP of the sink associated with the packet */
    packet_path_est ptype_;                 /**< Idientifier of the packet type: PATH_SEARCH, PATH_ANSWER or PATH_ERROR */
    uint8_t list_of_hops_[MAX_HOP_NUMBER];  /**< List of IPs saved in the header */
    int8_t pointer_to_list_of_hops_;        /**< Pointer used to keep track of the last IPs processed */
    uint8_t list_of_hops_length_;           /**< Current number of IPs stored in the header */
    float quality_;                         /**< Used to save information about the "quality" of the metric used */
    static int offset_;                     /**< Required by the PacketHeaderManager */
    
    /**
     * Reference to the offset_ variable
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     * Reference to the sink_associated_ variable
     */
    inline uint8_t& sinkAssociated() {
        return (sink_associated_);
    }
    
    /**
     * Reference to the ptype_ variable
     */
    inline packet_path_est& ptype() {
        return (ptype_);
    }
    
    /**
     * Pointer to the list_of_hops_ variable
     */
    inline uint8_t* list_of_hops() {
        return list_of_hops_;
    }
    
    /**
     * Reference to the list_of_hops_length_ variable
     */
    inline u_int8_t& list_of_hops_length() {
        return list_of_hops_length_;
    }
    
    /**
     * Reference to the pointer_to_list_of_hops_ variable
     */
    inline int8_t& pointer() {
        return pointer_to_list_of_hops_;
    }
    
    /**
     * Reference to the quality_ variable
     */
    float& quality() {
        return quality_;
    }
    
    inline static struct hdr_sun_path_est * access(const Packet * p) {
        return (struct hdr_sun_path_est*) p->access(offset_);
    }
} hdr_sun_path_est;

#endif // HDR_SUN_PATH_EST_H

