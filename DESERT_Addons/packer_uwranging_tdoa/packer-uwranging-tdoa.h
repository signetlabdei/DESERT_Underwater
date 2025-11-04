//
// Copyright (c) 2024 Regents of the SIGNET lab, University of Padova.
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
 * @file packer-uwraning-tdoa.h
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 * \brief  Header of the class responsible to map the NS-Miracle packet of
 * uwrangingTDOA into a bit stream, and vice-versa.
 */

#ifndef PACKER_UWRANGINGTDOA_H
#define PACKER_UWRANGINGTDOA_H

#include "packer.h"

/**
 * Class to map a ns2 uwRangingTDOA header into a bit stream, and vice-versa. */
class packer_uwRangingTDOA : public packer
{
public:
	/**
	 * Constructor packer_uwRangingTDOA class
	 *
	 */
	packer_uwRangingTDOA();

	/**
	 * Destructor packer_uwRangingTDOA class
	 *
	 */
	virtual ~packer_uwRangingTDOA() = default;

private:
	/**
	 * Init the packer for uwRangingTDOA prototocol
	 */
	virtual void init() override;

	/**
	 *  Method to transform the headers of uwRangingTDOA protocol into a stream
	 * of bits
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	virtual size_t packMyHdr(
			Packet *p, unsigned char *buffer, size_t offset) override;

	/**
	 *  Method responsible to take the informations from the received buffer and
	 * store it into the headers of the packet
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return size_t New offset after unpacking the headers
	 */
	virtual size_t unpackMyHdr(
			unsigned char *buffer, size_t offset, Packet *p) override;

	/**
	 * Method used for debug purposes. It prints the number of bits for each
	 * header serialized
	 */
	virtual void printMyHdrMap() override;

	/**
	 *  Method used for debug purposes. It prints the value of the headers of a
	 * packet
	 *
	 * @param Pointer of the packet
	 */
	virtual void printMyHdrFields(Packet *) override;

	/** Index bits */
	enum nbits_index {
		SOURCE_PKT_ID_FIELD = 0, /**< ID of the ranging packet. */
		SOURCE_NODE_ID_FIELD, /**< ID of the node. */
		TIMES_SIZE_FIELD, /**< Size of the payload. */
		LAST_ELEM
	};

	size_t source_pkt_id_Bits; /**< Bit length of the source_pkt_id field to be
								  put in the header stream of bits. */
	size_t source_node_id_Bits; /**< Bit length of the source_node_id to be put
								   in the header stream of bits. */
	size_t times_size_Bits; /**< Bit length of the times_size_field to be put in
							   the header stream of bits. */
};
#endif /* PACKER_UWRANGINGTDOA_H */
