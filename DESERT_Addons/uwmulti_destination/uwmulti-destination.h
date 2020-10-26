//
// Copyright (c) 2018 Regents of the SIGNET lab, University of Padova.
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

/**
* @file   uwmulti-destination.h
* @author William Rizzo
* @version 1.0.0h
*
* \brief Definition of UwMultiDestination class.
*
*/

#ifndef UWMULTI_DESTINATION_H
#define UWMULTI_DESTINATION_H

#include <climits>
#include <cmath>
#include <iostream>
#include <map>
#include <list>
#include <module.h>
#include <packet.h>
#include <rng.h>
#include <string.h>
#include <tclcl.h>
#include <uwip-module.h>

typedef struct IP_range
{
	int min_IP; /**< Minimum IP address of the range */
	int max_IP; /**< Maximum IP address of the range */

	IP_range(int min, int max)
	:
	min_IP(min),
	max_IP(max)
	{
	}

	/**
	 * Return true if the 2 ranges are not overlapped
	 * @param range IP range to check
	 * @return true if the ranges are not overlapped
	 */
	bool overlappingRange(IP_range range)
	{
		return overlappingRange(range.min_IP, range.max_IP);
	}

	/**
	 * Return true if the 2 ranges are overlapped
	 * @param min minimum IP address of the range
	 * @param max masimum IP address of the range
	 * @return true if the ranges are overlapped
	 */
	bool overlappingRange(int min, int max)
	{
		return !(min_IP > max || max_IP < min);
	}

	/**
	 * Check if the given IP addr is in the range
	 * @param addr IP address
	 * @return true if the IP address is in the range, false otherwise
	 */
	bool isInRange(int addr)
	{
		return (addr >= min_IP && addr <= max_IP);
	}
} range_IP;

typedef std::pair<int, IP_range> layer_IPrange;


/**
 * Class used to represents the UwMultiDestination layer of a node.
 */
class UwMultiDestination : public Module
{

public:
	// constant definitions
	static int const layer_not_exist; /**< This constant is returned when a
										 searched layer does not exist>*/

	/**
	 * Constructor of UwMultiDestination class.
	 */
	UwMultiDestination();

	/**
	 * Destructor of UwMultiDestination class.
	 */
	virtual ~UwMultiDestination()
	{
	}

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 * <i>argv[0]</i>
	 *             is the name of the object).
	 *
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 * successfully or not.
	 */
	virtual int command(int, const char *const *);

	/**
	 * Add a layer in the layer map if the IP range is valide, i.e., 
	 * if the range doesn't overlap with an existing one
	 *
	 * @param range IP range
	 * @param id identifier of the lower layer
	 * return false if the range overlaps with an existing one, true otherwise
	 */
	virtual bool addLayer(IP_range range, int id);

	/**
	 * recv method. It is called when a packet is received from the other layers
	 *
	 * @param Packet* Pointer to the packet that are going to be received
	 */
	virtual void recv(Packet *p);

protected:
	// Variables
	/**< Switch modes >*/
	enum Mode {
		UW_MANUAL_SWITCH = 0, /**< State to switch-mode manually.*/
		UW_AUTOMATIC_SWITCH /**< State to switch-mode automatically.*/
	};

	int debug_; /**< Flag to activate debug verbosity.*/
	double min_delay_;
	Mode switch_mode_; /**< Current switch mode (either AUTOMATIC or MANUAL).*/
	int lower_id_active_; /**< Id of the current lower layer active. It is
								 used only in MANUAL MODE.*/

	std::list<layer_IPrange> layer_list;/**Maps a layer id into an IP_range. */

	int default_lower_id; /**Default lower id to use if dest adress is not found
							in the considered IP ranges. */

	/**
	 * Handle a packet coming from upper layers
	 *
	 * @param p pointer to the packet
	*/
	virtual void recvFromUpperLayers(Packet *p);

	/**
	 * Return the best layer to forward the packet when the system works in
	 * AUTOMATIC_MODE.
	 *
	 * @param p pointer to the packet
	 *
	 * @return id of the module representing the best layer.
	*/

	virtual int getDestinationLayer(Packet *p);
	
	/**
	 * return true if there is not overlap between the new range and the 
	 * previous rnage in the list
	 *
	 * @param range that has to be inserted in the list
	 * @return true if there is not overlap, false otherwise 
	 */
	virtual bool checkNotOverlap(IP_range range);

private:
	// Variables
};

#endif /* UWMULTI_DESTINATION_H  */
