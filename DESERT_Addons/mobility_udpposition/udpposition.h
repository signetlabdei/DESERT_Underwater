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
 * @file   udpposition.h
 * @author Torsten Pfuetzenreuter
 * @version 1.0.0
 *
 * \brief Provides the definition of the class <i>UDPPosition</i>.
 *
 * Provides the definition of the class <i>UDPPosition</i>.
 * This class implements a UDP socket server that allows to set the node
 * position from external applications (e.g. a simulator)
 *
 * @see NodeCore, Position
 **/

#ifndef _UDPPOSITION_
#define _UDPPOSITION_

#include <node-core.h>
#include <mutex>
#include "position_listener.h"
#include "logging.h"

class UDPPosition : public Position
{
public:
    /**
    * Constructor
    */
    UDPPosition();
    /**
    * Destructor
    */
    virtual ~UDPPosition() = default;

    /**
    * Method that return the current projection of the node on the x-axis.
    * If it's necessary (updating time ia expired), update the position values
    * before returns it.
    */
    virtual double getX();
    /**
    * Method that return the current projection of the node on the y-axis.
    * If it's necessary (updating time ia expired), update the position values
    * before returns it.
    */
    virtual double getY();

    /**
    * Method that return the current projection of the node on the z-axis.
    * If it's necessary (updating time ia expired), update the position values
    * before returns it.
    */
    virtual double getZ();

    /**
    * TCL command interpreter
    *  <li><b>setdest &lt;<i>integer value</i>&gt;<i>integer
    *value</i>&gt;<i>integer value</i>&gt;</b>:
    *   set the movement pattern: the firts two values define the point to be
    *reached (i.e., the
    *   direction of the movement) and the third value defines the speed to be
    *used
    * </ul>
    *
    * Moreover it inherits all the OTcl method of Position
    *
    *
    * @param argc number of arguments in <i>argv</i>
    * @param argv array of strings which are the comand parameters (Note that
    *argv[0] is the name of the object)
    *
    * @return TCL_OK or TCL_ERROR whether the command has been dispatched
    *succesfully or no
    *
    **/
    virtual int command(int argc, const char *const *argv);
    virtual void setX(double x);
    virtual void setY(double y);
    virtual void setZ(double z);

    int debugLevel() const { return debug_; }

    void setPosition(const PositionData& pos);
private:
    int debug_;
    /** Position receive port number for UDP socket */
    unsigned int m_UdpReceivePort;
    /** Socket timeout for select() call in [us] */
    unsigned int m_SocketReadTimeout{50000};

    std::mutex mutex_;
    PositionListener<UDPPosition> *p_PositionListener{nullptr};
};

#endif // _UDPPOSITION_
