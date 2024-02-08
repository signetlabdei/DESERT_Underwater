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
 * @file   stoppable_thread.h
 * @author Torsten Pfuetzenreuter
 * @version 1.0.0
 *
 * \brief Provides a simple C++11 thread the StoppableThread class
 *
 */

#pragma once

#include <atomic>
#include <thread>
using namespace std::chrono_literals;

class StoppableThread
{
public:
    bool Start()
    {
        try
        {
            m_thread = std::thread(&StoppableThread::RunInternal, this);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    void Stop(bool wait = false)
    {
        m_stop.store(true);
        if (wait && m_thread.joinable())
        {
            try
            {
                m_thread.join();
            }
            catch (...)
            {
            }
        }
    }
    virtual void Run()
    {
        while (!StopRequested())
        {
            Sleep(100ms);
        }
    }
    /** Sleep for the given duration, use literals like 1s, 100ms, 10us */
    template <class Rep, class Period>
    void Sleep(const std::chrono::duration<Rep, Period> &d)
    {
        std::this_thread::sleep_for(d);
    }
    bool Running() { return m_running.load(); }
    bool StopRequested() { return m_stop.load(); }

private:
    void RunInternal()
    {
        m_running = true;
        Run();
        m_running = false;
    }
    std::atomic_bool m_running{false};
    std::atomic_bool m_stop{false};
    std::thread m_thread;
};
