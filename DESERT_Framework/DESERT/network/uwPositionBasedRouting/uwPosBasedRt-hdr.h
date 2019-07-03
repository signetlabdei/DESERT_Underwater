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
 * @file   uwPosBasedRt-hdr.h
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Provides the header description for UwPosBasedRt packets.
 *
 */


#ifndef HDR_UWPOS_BASED_RT_H
#define HDR_UWPOS_BASED_RT_H

#include <packet.h>

#define HDR_UWPOS_BASED_RT(p) (hdr_uwpos_based_rt::access(p))

extern packet_t PT_UWPOSBASEDRT;

/**
 * <i>hdr_uwpos_based_rt</i> describes packets used by <i>UWPOSBASEDRT</i>.
 */
typedef struct hdr_uwpos_based_rt {

	double x_ROV_; /**<ROV x coordinate at the moment when packet is generated*/
	double y_ROV_; /**<ROV y coordinate at the moment when packet is generated*/
	double z_ROV_; /**<ROV z coordinate at the moment when packet is generated*/
	double timestamp_; /**<To check validity of packet inormations */
	double x_wp_; /**<x coordinate of the last received waypoint */
	double y_wp_; /**<y coordinate of the last received waypoint */
	double z_wp_; /**<z coordinate of the last received waypoint */
	double ROV_speed_;
	uint8_t ipROV_; /***<IP of the ROV related to the informations inserted in
						the header. */
	
	static int offset_; /**< Required by the PacketHeaderManager. */


	inline double& x_ROV()
	{
		return x_ROV_;
	}

	inline double& y_ROV()
	{
		return y_ROV_;
	}

	inline double& z_ROV()
	{
		return z_ROV_;
	}

	inline double& timestamp()
	{
		return timestamp_;
	}

	inline double& x_waypoint()
	{
		return x_wp_;
	}

	inline double& y_waypoint()
	{
		return y_wp_;
	}

	inline double& z_waypoint()
	{
		return z_wp_;
	}

	inline double& ROV_speed()
	{
		return ROV_speed_;
	}

	inline uint8_t& IP_ROV()
	{
		return ipROV_;
	}


	/**
	 * Reference to the offset_ variable.
	 */
	inline static int &
	offset()
	{
		return offset_;
	}



	inline static struct hdr_uwpos_based_rt *
	access(const Packet *p)
	{
		return (struct hdr_uwpos_based_rt *) p->access(offset_);
	}
} hdr_uwpos_based_rt;

#endif // HDR_UWPOS_BASED_RT_H
