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
 * @file        uwUFetch_cmn_hdr.h
 * @author  Loris Brolo
 * @version 1.0.0
 * 
 * \brief	Common structures and variables in the protocol
 */

#ifndef UWUFETCH_CMN_HDR_H      
#define UWUFETCH_CMN_HDR_H

#include <mmac.h>
#include <module.h>
#include <packet.h>

#define HDR_TRIGGER_UFETCH(p) (hdr_TRIGGER_UFETCH::access(p))     /**< alias defined to access the TRIGGER HEADER */
#define HDR_RTS_UFETCH(p) (hdr_RTS_UFETCH::access(p))                     /**< alias defined to access the RTS HEADER */
#define HDR_CTS_UFETCH(p) (hdr_CTS_UFETCH::access(p))                     /**< alias defined to access the CTS HEADER */
#define HDR_BEACON_UFETCH(p) (hdr_BEACON_UFETCH::access(p))      /**< alias defined to access the BEACON HEADER*/
#define HDR_PROBE_UFETCH(p) (hdr_PROBE_UFETCH::access(p))           /**< alias defined to access the PROBE HEADER*/
#define HDR_POLL_UFETCH(p) (hdr_POLL_UFETCH::access(p))                /**< alias defined to access the POLL HEADER*/
#define HDR_CBEACON_UFETCH(p) (hdr_CBEACON_UFETCH::access(p)) /**< alias defined to access the CBEACON HEADER*/

extern packet_t PT_BEACON_UFETCH; /**< BEACON packet type */
extern packet_t PT_POLL_UFETCH; /**< POLL packet type */
extern packet_t PT_CBEACON_UFETCH; /**< CBEACON packet type */
extern packet_t PT_RTS_UFETCH; /**< RTS packet type */
extern packet_t PT_PROBE_UFETCH; /**< PROBE packet type */
extern packet_t PT_TRIGGER_UFETCH; /**< TRIGGER packet type */
extern packet_t PT_CTS_UFETCH; /**< CTS packet type */


static const int v_speed_sound = 1500; /**< Underwater sound propagation speed */
static const int BIT_RATE_SERVICE = 1000;
static const int BIT_RATE_INSTANT = 10000;

/**
 * Content header of TRIGGER packet
 */
typedef struct hdr_TRIGGER_UFETCH {
    int t_min_; /**< minimum value in which the HN can choose his backoff time  */
    int t_max_; /**< maximum value in which the HN can choose his backoff time  */
    int max_pck_want_rx_; /**< Maximum number of pck that AUV want receive from sink */

    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     *  Reference to the t_min variable 
     * 
     * @return int& t_min_  
     */
    int& t_min() {
        return (t_min_);
    }

    /**
     *  Reference to the t_max variable
     *  
     * @return int& t_max_ 
     */
    int& t_max() {
        return (t_max_);
    }

    /**
     * Reference to the max_pck_want_rx variable
     * 
     * @return int & max_pck_want_rx_
     */
    int& max_pck_want_rx() {
        return (max_pck_want_rx_);
    }

    inline static hdr_TRIGGER_UFETCH * access(const Packet * p) {
        return (hdr_TRIGGER_UFETCH*) p->access(offset_);
    }
} hdr_TRIGGER_UFETCH;

/**
 * Content header of RTS packet
 */
typedef struct hdr_RTS_UFETCH {
    int num_DATA_pcks_; /**< Number of packets that the HN want to transmit to the AUV */
    int backoff_time_RTS_; /**< Backoff time chosen by the HN for collision free */

    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     *  Reference to the num_Data_pcks_ variable
     * 
     * @return int& num_DATA_pcks_
     */
    int& num_DATA_pcks() {
        return num_DATA_pcks_;
    }

    /**
     *  Reference to the backoff_Time_RTS_ variable
     * 
     * @return int& backoff_time_RTS_
     */
    int& backoff_time_RTS() {
        return backoff_time_RTS_;
    }

    inline static hdr_RTS_UFETCH * access(const Packet * p) {
        return (hdr_RTS_UFETCH*) p->access(offset_);
    }
} hdr_RTS_UFETCH;

/**
 * Content header of CTS packet
 */
typedef struct hdr_CTS_UFETCH {
    int num_DATA_pcks_MAX_rx_; /**< maximum number of packets that AUV want receive from HN */
    int mac_addr_HN_ctsed_; /**< Mac address of the HN from which the AUV want to receive the DATA packets  */
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     *  Reference to the num_DATA_pcks_MAX_rx_ variable
     * 
     * @return int& num_DATA_pcks_MAX_rx_
     */
    int& num_DATA_pcks_MAX_rx() {
        return num_DATA_pcks_MAX_rx_;
    }

    /**
     *  Reference to the mac_addr_HN_ctsed_ variable 
     * 
     * @return int& mac_addr_HN_ctsed_
     */
    int& mac_addr_HN_ctsed() {
        return mac_addr_HN_ctsed_;
    }

    inline static hdr_CTS_UFETCH * access(const Packet * p) {
        return (hdr_CTS_UFETCH*) p->access(offset_);
    }
} hdr_CTS_UFETCH;

/**
 * Content header of BEACON packet
 */
typedef struct hdr_BEACON_UFETCH {
    int t_min_bc_; /**< minimum value in which the Node can choose his backoff time */
    int t_max_bc_; /**< maximum value in which the Node can choose his backoff time */
    int num_Max_CBEACON_tx_by_HN_; /**< Maximum number of CBEACON that the HN can transmit after the BEACON packet */
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     *  Reference to t_min_bc_ variable
     * 
     * @return int& t_min_bc_
     */
    int& t_min_bc() {
        return t_min_bc_;
    }

    /**
     *  Reference to t_max_bc_ variable 
     * 
     * @return int& t_max_bc_
     */
    int& t_max_bc() {
        return t_max_bc_;
    }

    /**
     *  Reference to num_Max_CBEACON_tx_by_HN_ variable 
     * 
     * @return int& num_Max_CBEACON_tx_by_HN_
     */
    int& num_Max_CBEACON_tx_by_HN() {
        return num_Max_CBEACON_tx_by_HN_;
    }

    inline static hdr_BEACON_UFETCH * access(const Packet * p) {
        return (hdr_BEACON_UFETCH*) p->access(offset_);
    }
} hdr_BEACON_UFETCH;

/**
 * Content header of PROBE packet
 */
typedef struct hdr_PROBE_UFETCH {
    int n_DATA_pcks_Node_tx_; /**< number of packets that Node have to transmit at HN */
    int backoff_time_PROBE_; /**< Backoff time chosen by the NODE for collision free */
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     *  Reference to n_DATA_pcks_Node_tx_ variable
     * 
     * @return int& n_DATA_pcks_Node_tx_
     */
    int& n_DATA_pcks_Node_tx() {
        return n_DATA_pcks_Node_tx_;
    }

    /**
     *  Reference to backoff_time_PROBE_ variable
     * 
     * @return int& backoff_time_PROBE_
     */
    int& backoff_time_PROBE() {
        return backoff_time_PROBE_;
    }

    inline static hdr_PROBE_UFETCH * access(const Packet * p) {
        return (hdr_PROBE_UFETCH*) p->access(offset_);
    }
} hdr_PROBE_UFETCH;

/**
 * Content header of POLL packet
 */
typedef struct hdr_POLL_UFETCH {
    int num_DATA_pcks_MAX_rx_; /**< Maximum number of DATA packet that HN want to receive from the specific node that is being to poll */
    int mac_addr_Node_polled_; /**< Mac address of the NODE that the HN has polled and from which it want to receive a data packet */
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     *  Reference to num_DATA_pcks_MAX_rx variable
     * 
     * @return int& num_DATA_pcks_MAX_rx_
     */
    int& num_DATA_pcks_MAX_rx() {
        return num_DATA_pcks_MAX_rx_;
    }

    /**
     *  Reference to mac_addr_Node_polled_ variable
     * 
     * @return int& mac_addr_Node_polled_
     */
    int& mac_addr_Node_polled() {
        return mac_addr_Node_polled_;
    };

    inline static struct hdr_POLL_UFETCH * access(const Packet * p) {
        return (struct hdr_POLL_UFETCH*) p->access(offset_);
    }
} hdr_POLL_UFETCH;

/**
 * Content header of CBEACON packet
 */
typedef struct hdr_CBEACON_UFETCH {
    int t_min_bc_; /**< minimum value in which the Node can choose his backoff time */
    int t_max_bc_; /**< maximum value in which the Node can choose his backoff time */
    int num_Max_CBEACON_tx_by_HN_; /**< Maximum number of CBEACON that the HN can transmit after the BEACON packet */
    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset variable 
     */
    inline static int& offset() {
        return offset_;
    }

    /**
     *  Reference to t_min_bc_ variable
     *  
     * @return int& t_min_bc_
     */
    int& t_min_bc() {
        return t_min_bc_;
    }

    /**
     *  Reference to t_max_bc_ variable 
     * 
     * @return int& t_max_bc_
     */
    int& t_max_bc() {
        return t_max_bc_;
    }

    /**
     *  Reference to num_Max_CBEACON_tx_by_HN_ variable 
     * 
     * @return int& num_Max_CBEACON_tx_by_HN_
     */
    int& num_Max_CBEACON_tx_by_HN() {
        return num_Max_CBEACON_tx_by_HN_;
    }

    inline static hdr_CBEACON_UFETCH * access(const Packet * p) {
        return (hdr_CBEACON_UFETCH*) p->access(offset_);
    }
} hdr_CBEACON_UFETCH;
#endif
