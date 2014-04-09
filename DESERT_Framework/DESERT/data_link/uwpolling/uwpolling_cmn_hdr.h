//
// Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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
 * @file	uwpolling_cmn_hdr.h
 * @author	Federico Favaro
 * @version     1.0.0
 *
 * \brief	Common structures and variables in the protocol
 *
 *
 */

#ifndef UWPOLLING_CMN_HDR_H
#define	UWPOLLING_CMN_HDR_H

#include <mmac.h>
#include <module.h>
#include <packet.h>

#define HDR_PROBE(p) (hdr_PROBE::access(p))		/**< alias defined to access the PROBE HEADER */
#define HDR_TRIGGER(p) (hdr_TRIGGER::access(p)) /**< alias defined to access the TRIGGER HEADER */
#define HDR_POLL(p) (hdr_POLL::access(p))		/**< alias defined to access the POLL HEADER */

extern packet_t PT_TRIGGER;
extern packet_t PT_POLL;
extern packet_t PT_PROBE;

static const int MAX_POLLED_NODE = 40; /**< Maximum number of node to POLL */
static const double MIN_T_POLL = 5; /**< Minimum duration of the POLL timer */
static const int MAX_BUFFER_SIZE = 100; /**< Maximum size of the queue in number of packets */
static const int prop_speed = 1500; /**< Typical underwater sound propagation speed */

/** Single location of the POLL vector. Each POLL_ID represent a polled node */
typedef struct POLL_ID {
    int id_; /**< ID of the node */
    double t_wait_; /**< Time that a node has to wait before being polled */
} id_poll;

/**
 * Header of the TRIGGER message 
 */
typedef struct hdr_TRIGGER {
    uint16_t t_in_; /**< Minimum value in which the node can choose his backoff time */
    uint16_t t_fin_; /**< Maximum value in which the node can choose his backoff time */
    uint TRIGGER_uid_; /**< TRIGGER packet unique ID */
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     * Reference to the t_in variable 
     */
    uint16_t& t_in() {
        return (t_in_);
    }

    /**
     * Reference to the t_fin variable 
     */
    uint16_t& t_fin() {
        return (t_fin_);
    }
    /**
     * Reference to the TRIGGER_uid variable 
     */
    uint& TRIGGER_uid() {
        return (TRIGGER_uid_);
    }

    inline static struct hdr_TRIGGER* access(const Packet* p) {
        return (struct hdr_TRIGGER*) p->access(offset_);
    }
} hdr_TRIGGER;

/**
 * Header of the POLL message 
 */
typedef struct hdr_POLL {
    int id_; /**< ID of the POLLED node */
    uint POLL_uid_; /**< POLL packet unique ID */ 
    static int offset_; /**< Required by the PacketHeaderManager. */
    
    /**
     * Reference to the id_ variable 
     */
    int& ID() {
        return (id_);
    }
    
    /**
     * Reference to the POLL_uid_ variable 
     */
    uint& POLL_uid() {
        return (POLL_uid_);
    }

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    inline static struct hdr_POLL* access(const Packet* p) {
        return (struct hdr_POLL*) p->access(offset_);
    }
} hdr_POLL;

/**
 * Header of the PROBE message 
 */
typedef struct hdr_PROBE {			
    uint16_t backoff_time_; /**< Backoff time chosen by the node */
    uint16_t ts_; /**< Timestamp of the most recent data packet */
    int n_pkts_; /**< Number of packets that the node wish to transmit to the AUV */
    uint id_node_; /**< ID of the node */
    uint PROBE_uid_;  /**< Unique ID of the PROBE packet */
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to timestamp variable 
     */
    uint16_t& ts() {
        return (ts_);
    }

    /**
     * Reference to backoff_time variable
     */
    uint16_t& backoff_time() {
        return (backoff_time_);
    }

    /**
     * Reference to n_pkts variable
     */
    int& n_pkts() {
        return (n_pkts_);
    }

    /**
     * Reference to id_node variable
     */
    uint& id_node() {
        return (id_node_);
    }
    
    /**
     * Reference to PROBE_uid_ variable
     */
    uint& PROBE_uid() {
        return (PROBE_uid_);
    }

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    inline static struct hdr_PROBE* access(const Packet* p) {
        return (struct hdr_PROBE*) p->access(offset_);
    }
} hdr_PROBE;
#endif



