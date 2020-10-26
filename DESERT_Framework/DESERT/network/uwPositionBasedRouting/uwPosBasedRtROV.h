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
 * @file   uwPosBasedRtROV.h
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Routing protocol for vehicles based on ROV position
 *
 */

#ifndef UW_POS_BASED_RT_ROV_H
#define UW_POS_BASED_RT_ROV_H

#define DROP_NEXT_HOP_NOT_FOUND \
	"NNF"
#define DROP_IP_NOT_SET \
	"INS"

#define pi (4 * atan(1.0))

#include <module.h>
#include <utility>
#include "uwip-module.h"
#include "uwPosBasedRt-hdr.h"
#include "node-core.h"
#include "uwsmposition.h"
#include <map>
#include <list>
#include <tclcl.h>

class UwPosBasedRtROV : public Module
{
public:
	/**
	 * Constructor of UwPosBasedRtROV class
	 */
	UwPosBasedRtROV();

	/**
	 *  Destructor of UwPosBasedRtROV class
	 */
	virtual ~UwPosBasedRtROV();

protected:
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not.
	 *
	 */
	virtual int command(int, const char *const *);

	/**
	* Performs the reception of packets from upper and lower layers.
	*
	* @param Packet* Pointer to the packet will be received.
	*/
	virtual void recv(Packet* p);

	/**
	* Initialize field of <i>hdr_uwpos_based_rt</i>
	*
	* @param Packet* Pointer to the packet will be received.
	*/
	virtual void initPkt(Packet* p);


	/**
	* Find next hop of a packet passed as input
	*
	* @param Packet* Pointer to the packet will be received.
	*/
	virtual uint8_t findNextHop(const Packet* p);

	/**
	* Set maximum transmission range 
	*
	* @param float New value for transmission range.
	*/
	virtual void setMaxTxRange(double newRange);

private:

	/**
	* Compute absoulute distance between 2 nodes
	*
	* @param Position position first node.
	* @param Position position second node.
	*/
	virtual double nodesDistance(Position& p1, Position& p2); 

	uint8_t ipAddr;

	double maxTxRange; /**<Maximum transmission range, 
						in meters, for this node. */

	UWSMPosition* ROV_pos; /**<Pointer to ROV position. 
							Give ROV position via TCL command. */

	typedef std::pair<Position,uint8_t> pair_posIP;
	std::list<pair_posIP> list_posIP; /**<List with position of all 
										the other nodes with its IP. */

	int debug_; /**< Flag to enable or disable dirrefent levels of debug. */

};

#endif //UW_POS_BASED_RT_H