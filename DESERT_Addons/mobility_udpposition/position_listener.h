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
 * @file   position_listener.h
 * @author Torsten Pfuetzenreuter
 * @version 1.0.0
 *
 * \brief Provides the PositionListener class
 *
 */

#ifndef _POSITION_LISTENER_H_
#define _POSITION_LISTENER_H_

#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define SOCKET_TYPE int
#define SET_SOCKOPT_TYPE void *
#define SENDTO_TYPE const char *
#define SOCKET_ERROR -1

#include <cstring>
#include <stdexcept>
#include <vector>

#include "stoppable_thread.h"
#include "position_data.h"
#include "logging.h"

/** Position listener thread with UDP socket
 *
 */
template<typename Owner>
class PositionListener : public StoppableThread
{
public:
    PositionListener(Owner *owner, uint16_t port, timeval read_timeout)
    {
        p_Owner = owner;
        m_Port = port;
        m_ReadTimeout = read_timeout;
    }
    virtual ~PositionListener()
    {
        if (m_SocketFD)
            ::close(m_SocketFD);
    }
    /** Thread function, runs until thread is stopped */
    virtual void Run()
    {
        try
        {
            if (p_Owner->debugLevel() > 0) LOG_MSG_INFO("Starting position data listener on port " << m_Port);
            // set up socket....
            m_SocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (m_SocketFD < 0)
                throw std::runtime_error("PositionListener::ListenLoop()::socket()");

            // allow to reuse the address
            int reuse = 1;
            if (setsockopt(m_SocketFD, SOL_SOCKET, SO_REUSEADDR, (SET_SOCKOPT_TYPE)&reuse, sizeof(reuse)) == -1)
                throw std::runtime_error("PositionListener::ListenLoop::setsockopt::reuse");

            // setup receive buffer to be large enough
            int rx_buffer_size = 1024;
            if (setsockopt(m_SocketFD, SOL_SOCKET, SO_RCVBUF, (SET_SOCKOPT_TYPE)&rx_buffer_size, sizeof(rx_buffer_size)) == -1)
                throw std::runtime_error("PositionListener::ListenLoop()::setsockopt::rcvbuf");

            // construct a datagram address structure
            struct sockaddr_in dg_addr;
            memset(&dg_addr, 0, sizeof(dg_addr));
            dg_addr.sin_family = AF_INET;
            // listen on any address
            dg_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            dg_addr.sin_port = htons(m_Port);

            if (bind(m_SocketFD, (struct sockaddr *)&dg_addr, sizeof(dg_addr)) == -1)
                throw std::runtime_error("PositionListener::ListenLoop()::bind");

            // reserve receive buffer, make it large enough (2 * serialized size for now)
            PositionData pd;
            std::vector<char> incoming_buffer(pd.size() * 2);                  
            while (!StopRequested())
            {
                if (ReadyToRead())
                {
                    if (p_Owner->debugLevel() >= 3)
                        LOG_MSG_INFO("Trying to read data from peer");
                    int num_bytes_read = recvfrom(m_SocketFD, (char *)incoming_buffer.data(), (int)incoming_buffer.size(), 0, 0, 0);
                    if (num_bytes_read < 0)
                    {
                        LOG_MSG_ERROR("Error reading from UDP port: " << num_bytes_read);
                        continue;
                    }
                    if (p_Owner->debugLevel() >= 3)
                        LOG_MSG_INFO("Received " << num_bytes_read << " bytes from peer");
                    try
                    {
                        pd.deserialize(incoming_buffer.data(), incoming_buffer.size());
                        p_Owner->setPosition(pd);
                    }
                    catch (const std::exception &e)
                    {
                        LOG_MSG_ERROR("Caught exception while reading position data: " << e.what() << " - " << std::strerror(errno));
                    }
                }
            }
            if (p_Owner->debugLevel() > 0) LOG_MSG_INFO("Stopping position data listener on port " << m_Port);
        }
        catch (const std::exception &e)
        {
            LOG_MSG_ERROR("Caught exception in listening thread: " << e.what() << " - " << std::strerror(errno));
        }
    }
protected:
    /** Uses select() to do a timed wait for new data
     *  @return true if data ara available, false if not
     */
    bool ReadyToRead()
    {
        int nfds;
        fd_set fdset;

        FD_ZERO(&fdset);
        FD_SET(m_SocketFD, &fdset);

        nfds = (int)m_SocketFD;
        int ret = select(nfds + 1, &fdset, NULL, NULL, &m_ReadTimeout);
        if (ret == SOCKET_ERROR)
        {
            LOG_MSG_ERROR("Error on select: " << ret);
        }
        return ret == 1;
    }
    /** Socket descriptor */
    SOCKET_TYPE m_SocketFD{0};
    /** Timeout for the select call in ReadyToRead()*/
    timeval m_ReadTimeout;
    /** UDP port number to read position from */
    uint16_t m_Port;
    /** Owner instance */
    Owner *p_Owner;
};

#endif // _POSITION_LISTENER_H_
