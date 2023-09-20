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
* Cross layer messages for uwswarm_control modules communication.
*/

#ifndef UWSC_CLMSG_H
#define UWSC_CLMSG_H

#include <clmessage.h>
#include <node-core.h>

#define CLMSG_MC2CTR_VERBOSITY 3 /**< Verbosity level. */
#define CLMSG_CTR2MC_VERBOSITY 3 /**< Verbosity level. */
#define CLMSG_TRACK2MC_VERBOSITY 3 /**< Verbosity level. */

extern ClMessage_t CLMSG_MC2CTR_SETPOS;
extern ClMessage_t CLMSG_MC2CTR_SETSTATUS;
extern ClMessage_t CLMSG_CTR2MC_GETPOS;
extern ClMessage_t CLMSG_TRACK2MC_TRACKPOS;
extern ClMessage_t CLMSG_TRACK2MC_GETSTATUS;

class ClSAP;

/**
 * Class that manages cross layer messages that require the position of
 * the ROV follower.
 */
class ClMsgCtr2McPosition : public ClMessage
{
  public:

	/**
	 * Class constructor.
	 * @param dest_id id of the destination module
	 */
    ClMsgCtr2McPosition(int dest_id);
  
	/**
	 * Destructor of ClMsgCtr2McPosition class.
	 */
    virtual ~ClMsgCtr2McPosition();

	/**
	 * Creates a copy of the object.
	 * @return Pointer to a copy of the object
	 */
    ClMsgCtr2McPosition* copy();

    /**
	* Sets the ROV follower position.
	* @param position rov follower current position
	*/
    void setRovPosition(Position* position);

	/**
	 * Get the ROV follower current position.
	 * @return rov_position rov follower current position
	 */
    Position* getRovPosition() const;

  private:
    Position* rov_position; /**< Rov follower current position. */
};

/**
 * Class that manages cross layer messages that require the new destination of
 * the ROV follower.
 */
class ClMsgMc2CtrPosition : public ClMessage
{
  public:

	/**
	 * Class constructor
     * @param dest_id id of the destination module
	 */
    ClMsgMc2CtrPosition(int dest_id);
       
	/**
	 * Destructor of ClMsgMc2CtrPosition class.
	 */
    virtual ~ClMsgMc2CtrPosition();

	/**
	 * Creates a copy of the object
	 * @return Pointer to a copy of the object
	*/
    ClMsgMc2CtrPosition* copy();

    /**
	 * Sets the ROV follower destination.
	 * @param destination rov follower destination
	 */
    void setRovDestination(Position* destination);

	/**
	 * Get the ROV follower destination. @return rov_destination rov follower destination
	 */
    Position* getRovDestination() const;
   
   
  private:
    Position* rov_destination; /**< Rov follower new destination. */
};

/**
 * Class that manages cross layer messages that require the status of
 * the ROV follower.
 */
class ClMsgMc2CtrStatus : public ClMessage
{
  public:

	/**
	 * Class constructor.
	 * @param dest_id id of the destination module
	 */
    ClMsgMc2CtrStatus(int dest_id);
       
	/**
	 * Destructor of ClMsgMc2CtrStatus class.
	 */
    virtual ~ClMsgMc2CtrStatus();

	/**
	 * Creates a copy of the object.
	 * @return Pointer to a copy of the object
	 */
    ClMsgMc2CtrStatus* copy();

	/**
	 * Sets the rov follower status.
	 * @param detect rov follower status
	 */
    void setRovStatus(bool detect);

	/**
	 * Get the rov follower status.
	 * @return rov_detect rov follower status
	 */
    bool getRovStatus() const;

   
  private:
    bool rov_status; /**< Status of the rov follower, true if detected a mine. */
};

/**
 * Class that manages cross layer messages that require the track position of
 * the ROV follower.
 */
class ClMsgTrack2McPosition : public ClMessage
{
  public:

	/**
	 * Class constructor.
	 * @param dest_id id of the destination module
	 */
    ClMsgTrack2McPosition(int dest_id);
       
	/**
	 * Destructor of ClMsgTrack2McPosition class.
	 */
    virtual ~ClMsgTrack2McPosition();

	/**
	 * Creates a copy of the object.
	 * @return Pointer to a copy of the object
	 */
    ClMsgTrack2McPosition* copy();

	/**
	 * Sets the track position
	 * @param track_position Tracked position from UwTracker module
	 */
    void setTrackPosition(Position* position);

	/**
	 * Get the track position.
	 * @return track_position Tracked position from UwTracker module
     */
    Position* getTrackPosition() const;
 
  private:
    Position* track_position; /**< Track position received from UwTracker module */
};

/**
 * Class that manages cross layer messages that require the status of
 * a mine tracked from a rov follower.
 */
class ClMsgTrack2McStatus : public ClMessage
{
  public:

	/**
	 * Class constructor.
	 * @param dest_id id of the destination module
	 */
    ClMsgTrack2McStatus(int dest_id);
       
	/**
	 * Destructor of ClMsgTrack2McStatus class.
	 */
    virtual ~ClMsgTrack2McStatus();

	/**
	 * Creates a copy of the object.
	 * @return Pointer to a copy of the object
	 */
    ClMsgTrack2McStatus* copy();

	/**
	 * Sets the current mine status.
	 * @param remove status of the current mine
	 */
    void setMineStatus(bool remove);

	/**
	 * Get the current mine status.
	 * @return mine_status status of the current mine
	 */
    bool getMineStatus() const;

   
  private:
    bool mine_status; /**< Status of the current mine; true if removed, false otherwise. */
};

#endif /* UWSC_CLMSG_H */
