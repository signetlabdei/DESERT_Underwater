//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file   sun-ipr-common-structures.h
 * @author Giovanni Toso
 * @version 1.1.1
 * 
 * \brief Common structures and definition used by SUN.
 * 
 * Common structures and definition used by SUN
 */

#ifndef SUN_COMMON_STRUCTURES_H
#define	SUN_COMMON_STRUCTURES_H

#define STACK_TRACE 0                                   /**< Used to keep track of methods call. */
#define MIN_SNR -9999.9                                 /**< Reference variable for min values for the SNR. */
// From ip-routing.h
#define DROP_DEST_UNREACHABLE_REASON "DUR"              /**< Reason for a drop in a <i>UWSUN</i> module. */

#define DROP_PATH_ALREADY_PROCESSED "PAP"               /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_MAX_NUMBER_OF_HOP_IN_LIST_REACHED "MHR"    /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_WRONG_NODE_PATH_EST "WNP"                  /**< Reason for a drop in a <i>UWSUN</i> module. */

// Drop reasons for Data packets.
#define DROP_DATA_HOPS_LENGTH_EQUALS_ZERO "DHZ"         /**< Reason for a drop in a <i>UWSUN</i> module. */

// Drops reasons for Path Establishment Search packets
#define DROP_PATH_ESTABLISHMENT_SEARCH_PACKET_HOP_LIST_FULL "PSF"               /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_PATH_ESTABLISHMENT_SEARCH_PACKET_ALREADY_PROCESSED "PSA"           /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_PATH_ESTABLISHMENT_SEARCH_PACKET_WRONG_CHECKSUM "PWC"              /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_PATH_ESTABLISHMENT_SEARCH_PACKET_NO_ROUTE_TO_DESTINATION "PNR"     /**< Reason for a drop in a <i>UWSUN</i> module. */

// Drops reasons for Path Establishment answer packets
#define DROP_PATH_ESTABLISHMENT_ANSWER_PACKET_GARBAGE "PAG"             /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_PATH_ESTABLISHMENT_ANSWER_NODE_ROUTING_TABLE_FULL "PAF"    /**< Reason for a drop in a <i>UWSUN</i> module. */

// Drop reasons for Sink Node
#define DROP_SINK_PACKET_TYPE_UNSOPPORTED "SPU"         /**< Reason for a drop in a <i>UWSUN</i> module. */

// Others
#define DROP_PACKET_NOT_FOR_ME "PNM"                    /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_PACKET_WITHOUT_DIRECTION "PWD"             /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_BROKEN_LINK_DROP "BLD"                     /**< Reason for a drop in a <i>UWSUN</i> module. */
#define DROP_BUFFER_IS_FULL   "BIF"                     /**< Reason for a drop in a <i>UWSUN</i> module. */

#include "packet.h"

#include <iostream>
#include <sstream>

static const int MAX_HOP_NUMBER = 5;    /**< Maximum number of hops contained in a <i>SUN Path Establishment</i> packet.*/
static int sunuid_ = 0;                 /**< Unique identifier for <i>UWSUN</i> packets. */

/**
 * buffer_element describes an entry in the buffer used by <i>SUN</i>.
 */
typedef struct buffer_element {
    
    Packet* p_;                 /**< Pointer to the packet buffered. */   
    int id_pkt_;                /**< ID of the packet buffered. */
    double t_reception_;        /**< Time instant in which the packet was buffered. */
    double t_last_tx_;          /**< Time instant of the last transmission attempt. */
    int num_retx_;              /**< Number of retransmission tentatives. */
    buffer_element() {}         /**< Constructor for buffer_element. */
    buffer_element(Packet* _p, int _id_pkt_ = 0, double _t_reception_ = Scheduler::instance().clock(), double _t_last_tx_ = 0, int _num_retx_ = 0) : p_(_p), id_pkt_(_id_pkt_), t_reception_(_t_reception_), t_last_tx_(_t_last_tx_), num_retx_(_num_retx_) {}  /**< Constructor for buffer_element. */
    
} buffer_element;

#endif // SUN_COMMON_STRUCTURES_H
