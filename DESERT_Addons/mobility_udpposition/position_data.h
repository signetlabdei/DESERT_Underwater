//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
 * @file   position_data.h
 * @author Torsten Pfuetzenreuter
 * @version 1.0.0
 *
 * \brief Provides the definition of PositionData struct
 *
 */

#ifndef POSITION_DATA_H
#define POSITION_DATA_H

#include <endian.h>
#if BYTE_ORDER == BIG_ENDIAN
#error Big endian detected! Serialization expects little endian system!
#endif
#include <cstring>  // for std::memcpy()

// Serialization in Python:
// import struct
// data = struct.pack("<ddd", x, y, z)

// < means little endian

/** Position data structure for submitting node positions to DESERT in ENU coordinates */
struct PositionData
{
    /** East in [m] */
    double x;
    /** North in [m] */
    double y;
    /** Up in [m] */
    double z;

    /** Compute required buffer size for (de-)serialization 
     * @return required buffer size
     */
    constexpr size_t size() const { return sizeof(x) + sizeof(y) + sizeof(z); }

    /** Serialize the position data to buffer
     *  @param buffer buffer where the data are written
     *  @param buffer_size size of buffer
     *  @return bytes used in buffer (if > 0) or negative of required buffer size 
     */
    int serialize(char* buffer, size_t buffer_size) const
    {
        int idx = 0;
        if (buffer_size < size()) {
            return -((int)size());
        }
        std::memcpy(buffer + idx, &x, sizeof(x)); idx += sizeof(x);
        std::memcpy(buffer + idx, &y, sizeof(x)); idx += sizeof(y);
        std::memcpy(buffer + idx, &z, sizeof(x)); idx += sizeof(z);
        return idx;
    }
    /** Deserialize the position data from the buffer
     *  @param buffer buffer with serialized data
     *  @param buffer_size size of buffer
     *  @return false if buffer is too small, true otherwise
     */
    bool deserialize(char* buffer, size_t buffer_size)
    {
        int idx = 0;
        if (buffer_size < size())
            return false;
        std::memcpy(&x, buffer + idx, sizeof(x)); idx += sizeof(x);
        std::memcpy(&y, buffer + idx, sizeof(y)); idx += sizeof(y);
        std::memcpy(&z, buffer + idx, sizeof(z)); idx += sizeof(z);
        return true;
    }
};

#endif /* POSITION_DATA_H */
