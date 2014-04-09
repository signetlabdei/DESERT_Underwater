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
 * @file frame-set.h
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Header of the classes defining the frame elements (key and set) exploited by Uwal objects to reasseble received fragments.
 */

#ifndef FRAMESET_H
#define FRAMESET_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <cstring>
#include <sstream>

#include "stdint.h"

#define MAX_BIN_PAYLOAD_ARRAY_LENGTH 2240

#define MAX_BIN_PAYLOAD_CHECK_ARRAY_LENGTH ( (MAX_BIN_PAYLOAD_ARRAY_LENGTH + sizeof(char) - 1) / sizeof(char) )

using namespace std;

class RxFrameSetKey {
    
    uint8_t srcID_;   /**< ID of the packet's source node. */    
    unsigned int pktID_;   /**< ID of the packet. */
   
public:
  
    RxFrameSetKey(uint8_t, unsigned int);
    
    ~RxFrameSetKey();
    
    /**
     * Return the srcID_ variable.
     */
    inline uint8_t srcID() const { return srcID_; }
    
    /**
     * Return the pktID_ variable.
     */
    inline unsigned int pktID() const { return pktID_; }
    
    bool operator<(RxFrameSetKey) const;
        
    std::string displayKey() const;
    
} ;

class RxFrameSet {
    
    char binPayload_[MAX_BIN_PAYLOAD_ARRAY_LENGTH];  /**< array to store binary data received as frames. */
    char binPayloadCheck_[MAX_BIN_PAYLOAD_CHECK_ARRAY_LENGTH];  /**< array to count already received frames. */
    int tot_length_; 
    size_t curr_length_;
    double t_last_rx_frame_;
    bool error_;
    
public:
    
    RxFrameSet();
    
    ~RxFrameSet();
    
    /**
     * Reference to the tot_length_ variable.
     */
    inline int tot_length() const { return tot_length_; }
    
    /**
     * Reference to the curr_length_ variable.
     */
    inline size_t curr_length() const { return curr_length_; }
    
    /**
     * Reference to the pktID_ variable.
     */
    inline double t_last_rx_frame() const { return t_last_rx_frame_; }
    
    /**
     * Return the content of the binPayload_ from 0 until tot_length_ array.
     */
    inline std::string binPayload() const { return binPayload(true); }  

    inline void setError(){ error_ = true; }

    inline bool getError() const { return error_; }
    
    /**
     * Return the content of the binPaylaod_
     * @param extract, if true return binPaylaod_ from 0 up to tot_length_ array, otherwise up to MAX_BIN_PAYLOAD_ARRAY_LENGTH.
     */
    std::string binPayload(bool) const;
    
    void UpdateRxFrameSet(char*,size_t,size_t,int,double);
    
    std::string displaySet();
    
} ;

#endif
