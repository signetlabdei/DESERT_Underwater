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
#include <module.h>
#include <packet.h>
#include <rng.h>
#include <string.h>
#include <tclcl.h>
#include <uwip-module.h>

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
	 * Add a layer in the layer map
	 *
	 * @param id unique identifier of the module
	 * @param order of the layer
	 */
	virtual void addLayer(int id, int order);

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
	int
			lower_id_active_; /**< Id of the current lower layer active. It is
								 used only in MANUAL MODE.*/
	int IP_rov_; /**< IP address of the ROV. It is used to determine the lower
					layer to use. */
	std::map<int, int> id2order; /**< Maps each layer id into its logic order
									(layer_id, order).*/
	std::map<int, int>
			order2id; /**< Return the layer id given its logic order. */

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
	 * return the logic order of the layer given its layer_id
	 *
	 * @param id to select the layer
	 *
	 * @return the order of the layer
	 */
	int inline getOrder(int layer_id)
	{
		return id2order.find(layer_id) == id2order.end()
				? UwMultiDestination::layer_not_exist
				: id2order.find(layer_id)->second;
	}

	/**
	 * return the layer id given its logic order
	 *
	 * @param order of the layer
	 *
	 * @return the layer id
	 */
	int inline getId(int layer_order)
	{
		return order2id.find(layer_order) == order2id.end()
				? UwMultiDestination::layer_not_exist
				: order2id.find(layer_order)->second;
	}

	/**
	 * return if the specified layer, identified by id, is available
	 *
	 * @param id unique identifier of the module
	 *
	 * @return if the specified layer is available
	 */
	virtual bool isLayerAvailable(int id);

private:
	// Variables
};

#endif /* UWMULTI_DESTINATION_H  */
