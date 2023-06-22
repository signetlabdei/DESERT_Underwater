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
* @file uswc-clmsg.h
* @author Vincenzo Cimino
* @version 1.0.0
*
* \brief Provides the <i>UWSC-CLMSG</i> class implementation.
*
* Provides the <i>UWSC-CLMSG</i> class implementation.
*/


#ifndef UWSC_CLMSG_H
#define UWSC_CLMSG_H

#include <clmessage.h>
#include <uwsmposition.h>

#define CLMSG_MC2CTR_VERBOSITY 3
#define CLMSG_CTR2MC_VERBOSITY 3
#define CLMSG_TRACK2MC_VERBOSITY 3

extern ClMessage_t CLMSG_MC2CTR_SETPOS;
extern ClMessage_t CLMSG_CTR2MC_GETPOS;
extern ClMessage_t CLMSG_TRACK2MC_TRACKPOS;

class ClSAP;

/**
 * Message to get the rov follower position via uwrovctr module
 */
class ClMsgCtr2McPosition : public ClMessage
{
  public:

	/**
	 * Class constructor
     * @param dest_id: id of the destination module
	 */
    ClMsgCtr2McPosition(int source_id, int dest_id);
        
    virtual ~ClMsgCtr2McPosition();

    /**
     * Creates a copy of the object
     * @return Pointer to a copy of the object
     */
    ClMsgCtr2McPosition* copy();

    /**
     * Sets the rov follower destination
     * @param position rov follower current position
     */
    void setRovPosition(UWSMPosition* position);

    /**
     * Get the rov follower position
     * @return rov_position rov follower current position
     */
    UWSMPosition* getRovPosition() const;

    /**
     * Get the rov follower id
     * @return rov_id rov follower id
     */
	int getRovId() const;
    
  private:
    UWSMPosition* rov_position; /**< Rov follower current position */
	int rov_id; /**< Rov follower id */
};

/**
 * Message to set the rov follower destination via uwrovctr module
 */
class ClMsgMc2CtrPosition : public ClMessage
{
  public:

	/**
	 * Class constructor
     * @param dest_id: id of the destination module
	 */
    ClMsgMc2CtrPosition(int dest_id);
        
    virtual ~ClMsgMc2CtrPosition();

    /**
     * Creates a copy of the object
     * @return Pointer to a copy of the object
     */
    ClMsgMc2CtrPosition* copy();

    /**
     * Sets the rov follower destination
     * @param destination rov follower destination
     */
    void setRovDestination(UWSMPosition* destination);

    /**
     * Get the rov follower destination
     * @return rov_destination rov follower destination
     */
    UWSMPosition* getRovDestination() const;
    
    
  private:
    UWSMPosition* rov_destination; /**< Rov follower new destination */
};

/**
 * Message to get the tracking position from uwtracker module
 */
class ClMsgTrack2McPosition : public ClMessage
{
  public:

	/**
	 * Class constructor
     * @param dest_id: id of the destination module
	 */
    ClMsgTrack2McPosition(int dest_id);
        
    virtual ~ClMsgTrack2McPosition();

    /**
     * Creates a copy of the object
     * @return Pointer to a copy of the object
     */
    ClMsgTrack2McPosition* copy();

    /**
     * Sets the tracking position received from uwtracker
     * @param track_position Tracked position from uwtracker
     */
    void setTrackPosition(UWSMPosition* position);

    /**
     * Get the tracking position received from uwtracker
     * @return track_position Tracked position from uwtracker
     */
    UWSMPosition* getTrackPosition() const;
    
    
  private:
    UWSMPosition* tracking_position; /**< Tracking position sent by the uwtracker addon */
};

#endif /* UWSC_CLMSG_H */
