//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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
* @file uswc-clmsg.cc
* @author Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the <i>UWSC-CLMSG</i> class implementation.
*
* Provides the <i>UWSC-CLMSG</i> class implementation.
*/

#include "uwsc-clmsg.h"


////////////////////////////////////////////////////////

ClMsgCtr2McPosition::ClMsgCtr2McPosition(int source_id, int dest_id)
	: ClMessage(CLMSG_CTR2MC_VERBOSITY,
		CLMSG_CTR2MC_GETPOS, UNICAST, dest_id)
	, rov_position(nullptr)
	, rov_id(source_id)
{
}

ClMsgCtr2McPosition::~ClMsgCtr2McPosition()
{
}

ClMsgCtr2McPosition* 
ClMsgCtr2McPosition::copy()
{
  return new ClMsgCtr2McPosition(*this);
}

void
ClMsgCtr2McPosition::setRovPosition(UWSMPosition* position)
{
	rov_position = position;
}

UWSMPosition*
ClMsgCtr2McPosition::getRovPosition() const
{
	return rov_position;
}

int
ClMsgCtr2McPosition::getRovId() const
{
	return rov_id;
}

////////////////////////////////////////////////////////

ClMsgMc2CtrPosition::ClMsgMc2CtrPosition(int dest_id)
	: ClMessage(CLMSG_MC2CTR_VERBOSITY,
		CLMSG_MC2CTR_SETPOS, UNICAST, dest_id)
	, rov_destination(nullptr)
{
}

ClMsgMc2CtrPosition::~ClMsgMc2CtrPosition()
{
}

ClMsgMc2CtrPosition* 
ClMsgMc2CtrPosition::copy()
{
  return new ClMsgMc2CtrPosition(*this);
}

void
ClMsgMc2CtrPosition::setRovDestination(UWSMPosition* destination)
{
	rov_destination = destination;
}

UWSMPosition*
ClMsgMc2CtrPosition::getRovDestination() const
{
	return rov_destination;
}
////////////////////////////////////////////////////////

ClMsgTrack2McPosition::ClMsgTrack2McPosition(int dest_id)
	: ClMessage(CLMSG_TRACK2MC_VERBOSITY,
		CLMSG_TRACK2MC_TRACKPOS, UNICAST, dest_id)
	, tracking_position(nullptr)
{
}

ClMsgTrack2McPosition::~ClMsgTrack2McPosition()
{
}

ClMsgTrack2McPosition* 
ClMsgTrack2McPosition::copy()
{
  return new ClMsgTrack2McPosition(*this);
}

void
ClMsgTrack2McPosition::setTrackPosition(UWSMPosition* position)
{
	tracking_position = position;
}

UWSMPosition*
ClMsgTrack2McPosition::getTrackPosition() const
{
	return tracking_position;
}
