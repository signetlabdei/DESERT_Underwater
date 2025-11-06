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
 * @file   logging.h
 * @author Torsten Pfuetzenreuter
 * @version 1.0.0
 *
 * \brief Provides the definition of PositionData struct
 *
 */

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <iostream>
#include <string_view>

namespace ConsoleColours
{

#ifdef _WIN32
#error Not implemented.
#else
inline constexpr std::string_view Red = "\x1b[1;31m";
inline constexpr std::string_view Green = "\x1b[1;32m";
inline constexpr std::string_view Yellow = "\x1b[1;33m";
inline constexpr std::string_view Blue = "\x1b[1;34m";
inline constexpr std::string_view Magenta = "\x1b[1;35m";
inline constexpr std::string_view Cyan = "\x1b[1;36m";
inline constexpr std::string_view Reset = "\x1b[0m";
#endif

}; // namespace ConsoleColours

#define LOG_MSG(color, msg) \
	std::cout << color << msg << ConsoleColours::Reset << std::endl;

#define LOG_MSG_ONCE(color, msg)       \
	{                                  \
		static bool msg_shown = false; \
		if (!msg_shown) {              \
			msg_shown = true;          \
			LOG_MSG(color, msg)        \
		}                              \
	}

#define LOG_MSG_INFO(msg) LOG_MSG(ConsoleColours::Green, msg)
#define LOG_MSG_WARN(msg) LOG_MSG(ConsoleColours::Yellow, msg)
#define LOG_MSG_ERROR(msg) LOG_MSG(ConsoleColours::Red, msg)

#define LOG_MSG_INFO_ONCE(msg) LOG_MSG_ONCE(ConsoleColours::Green, msg)
#define LOG_MSG_WARN_ONCE(msg) LOG_MSG_ONCE(ConsoleColours::Yellow, msg)
#define LOG_MSG_ERROR_ONCE(msg) LOG_MSG_ONCE(ConsoleColours::Red, msg)

#endif /* _LOGGING_H_ */
