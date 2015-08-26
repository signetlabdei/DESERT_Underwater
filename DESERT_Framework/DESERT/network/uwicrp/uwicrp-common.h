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
 * @file   uwicrp-common.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Common structures and definition used by UWICRP.
 * 
 * Common structures and definition used by UWICRP.
 */

#ifndef UWICRP_COMMON_H
#define	UWICRP_COMMON_H

#include "config.h"

static const int MAX_HOP_NUMBER = 6;            /**< Maximum number of hops contained in a <i>SUN Path Establishment</i> packet. */
static const int HOP_TABLE_LENGTH = 10;         /**< Maximum length of the routing table of a node FOR UWICRP. */     

/**
 * routing_table_entry describes an entry in the routing table used by UWICRP.
 */
struct routing_table_entry {
    
    nsaddr_t destination;       /**< Address of the destination. */
    nsaddr_t next_hop;          /**< Address of the next hop. */
    int hopcount;               /**< Hop count. */
    double creationtime;        /**< Creation Time of the packet. */
    bool isValid;               /**< Flag to check the validity of the packet. */
};

#endif // UWICRP_COMMON_H

