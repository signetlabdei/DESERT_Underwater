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
 * @file   udpposition.cpp
 * @author Torsten Pfuetzenreuter
 * @version 1.0.0
 *
 * \brief Provides the <i>UDPPosition</i> class implementation.
 *
 * Provides the <i>UDPPosition</i> class implementation.
 */

#include <iostream>
#include "udpposition.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class UDPPositionClass : public TclClass
{
public:
    UDPPositionClass()
        : TclClass("Position/UDP")
    {
    }
    TclObject *
    create(int, const char *const *)
    {
        return (new UDPPosition());
    }
} class_uwsmposition;

UDPPosition::UDPPosition()
    : Position()
{
    bind("debug_", &debug_);
    bind("udp_receive_port_", &m_UdpReceivePort);
}

UDPPosition::~UDPPosition()
{
}

int UDPPosition::command(int argc, const char *const *argv)
{
    // Tcl &tcl = Tcl::instance();
    if (strcasecmp(argv[1], "start") == 0)
    {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = m_SocketReadTimeout;
        p_PositionListener = new PositionListener(this, m_UdpReceivePort, tv);
        if (p_PositionListener)
        {
            if (debug_ >= 1)
                LOG_MSG_INFO(NOW << "::UDPPosition: starting position listener on port " << m_UdpReceivePort);
            p_PositionListener->Start();
        }

        return TCL_OK;
    }
    else if (strcasecmp(argv[1], "stop") == 0)
    {
        if (p_PositionListener)
        {
            if (p_PositionListener->Running())
            {
                if (debug_ >= 1)
                    LOG_MSG_INFO(NOW << "::UDPPosition: stopping position listener");
                p_PositionListener->Stop(true);
            }
            delete p_PositionListener;
        }
        return TCL_OK;
    }
    return Position::command(argc, argv);
}

void UDPPosition::setPosition(const PositionData &pos)
{
    if (debug_ >= 1)
        LOG_MSG_INFO(NOW << "::UDPPosition: setting position to [" << pos.x << "," << pos.y << "," << pos.z << "]");
    std::unique_lock<std::mutex> lock(mutex_);
    x_ = pos.x;
    y_ = pos.y;
    z_ = pos.z;
}

double
UDPPosition::getX()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return x_;
}
double
UDPPosition::getY()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return y_;
}
double
UDPPosition::getZ()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return z_;
}

void UDPPosition::setX(double x)
{
    std::unique_lock<std::mutex> lock(mutex_);
    x_ = x;
}
void UDPPosition::setY(double y)
{
    std::unique_lock<std::mutex> lock(mutex_);
    y_ = y;
}
void UDPPosition::setZ(double z)
{
    std::unique_lock<std::mutex> lock(mutex_);
    z_ = z;
}
