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
//

/**
 * @file   uwrov-packet.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 * 
 * \brief Provides both <i>UWROV</i> monitoring and control packets header description.
 * 
 * Provides both <i>UWROV</i> monitoring and control packets header description, in 
 * particular the header structure.
 */

/**
 * <i>hdr_uwROV_ctr</i> describes <i>UWROV_ctr</i> packets for controlling the ROV.
 */
typedef struct hdr_uwROV_ctr {
    float x_;
    float y_;
    float z_;
    float speed_;
    double sn_; // sequence number

    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset_ variable.
     */
    inline static int& offset() {
        return offset_;
    }

    inline static struct hdr_uwROV_ctr * access(const Packet * p) {
        return (struct hdr_uwROV_ctr*) p->access(offset_);
    }

    inline float& x() {
        return x_;
    }

    inline float& y() {
        return y_;
    }

    inline float& z() {
        return z_;
    }

    inline float& speed() {
        return speed_;
    }
    inline double& sn() {
        return sn_;
    }
} hdr_uwROV_ctr;

/**
 * <i>hdr_uwROV_monitoring</i> describes <i>UWROV_monitoring</i> packets sent by the ROV to the base station for monitoring the ROV state.
 */
typedef struct hdr_uwROV_monitoring {
    float x_;
    float y_;
    float z_;
    double ack_; // ack piggybacked of a ctr message. If =0 is not ack, if =b>0 is cumulative ack untill b, if c<0 is cumulative ack untill c-1 and NACK c.

    static int offset_; /**< Required by the PacketHeaderManager. */

    /**
     * Reference to the offset_ variable.
     */
    inline static int& offset() {
        return offset_;
    }

    inline static struct hdr_uwROV_monitoring * access(const Packet * p) {
        return (struct hdr_uwROV_monitoring*) p->access(offset_);
    }

    inline float& x() {
        return x_;
    }

    inline float& y() {
        return y_;
    }

    inline float& z() {
        return z_;
    }

    inline double& ack() {
        return ack_;
    }
    
} hdr_uwROV_monitoring;
