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

/**
 * @file packer-uwraning-tdoa.cc
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 * \brief  Header of the class responsible to map the NS-Miracle packet of
 * uwrangingTDOA into a bit stream, and vice-versa.
 *
 */

#include "packer-uwranging-tdoa.h"
#include "uwranging_tdoa_hdr.h"
#include <iostream>

static class PackerUwRangingTDOAClass : public TclClass
{
public:
	PackerUwRangingTDOAClass()
		: TclClass("UW/RANGING_TDOA/Packer")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new packer_uwRangingTDOA());
	}
} class_module_packerUwRangingTDOA;

packer_uwRangingTDOA::packer_uwRangingTDOA()
	: packer(false)
	, source_pkt_id_Bits(0)
	, source_node_id_Bits(0)
	, times_size_Bits(0)
{
	bind("SOURCE_PKT_ID_FIELD", (int *) &source_pkt_id_Bits);
	bind("SOURCE_NODE_ID_FIELD", (int *) &source_node_id_Bits);
	bind("TIMES_SIZE_FIELD", (int *) &times_size_Bits);

	this->init();
}

void
packer_uwRangingTDOA::init()
{
	if (debug_)
		std::cout << "Re-initialization of n_bits for the uwRangingTDOA packer."
				  << std::endl;

	n_bits.clear();
	n_bits.assign(LAST_ELEM, 0);

	n_bits[SOURCE_PKT_ID_FIELD] = source_pkt_id_Bits;
	n_bits[SOURCE_NODE_ID_FIELD] = source_node_id_Bits;
	n_bits[TIMES_SIZE_FIELD] = times_size_Bits;
}

size_t
packer_uwRangingTDOA::packMyHdr(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_cmn *hcmn = HDR_CMN(p);

	if (hcmn->ptype() == PT_UWRANGING_TDOA) {
		hdr_ranging_tdoa *tdoah = HDR_RANGING_TDOA(p);

		offset += put(buffer,
				offset,
				&(tdoah->source_pkt_id),
				n_bits[SOURCE_PKT_ID_FIELD]);
		offset += put(buffer,
				offset,
				&(tdoah->source_node_id),
				n_bits[SOURCE_NODE_ID_FIELD]);
		offset += put(buffer,
				offset,
				&(tdoah->times_size_),
				n_bits[TIMES_SIZE_FIELD]);

		int times_size_bits = tdoah->times_size() * sizeof(tdoa_entry) * 8;
		offset += put(buffer, offset, &(tdoah->times_), times_size_bits);

		if (debug_) {
			std::cout << "\033[1;37;45m (TX) UwRangingTDOA::DATA packer hdr "
						 "\033[0m"
					  << std::endl;
			printMyHdrFields(p);
		}
	}
	return offset;
}

size_t
packer_uwRangingTDOA::unpackMyHdr(
		unsigned char *buffer, size_t offset, Packet *p)
{
	hdr_cmn *hcmn = HDR_CMN(p);

	if (hcmn->ptype() == PT_UWRANGING_TDOA) {
		hdr_ranging_tdoa *tdoah = HDR_RANGING_TDOA(p);

		memset(&(tdoah->source_pkt_id), 0, sizeof(tdoah->source_pkt_id));
		offset += get(buffer,
				offset,
				&(tdoah->source_pkt_id),
				n_bits[SOURCE_PKT_ID_FIELD]);
		memset(&(tdoah->source_node_id), 0, sizeof(tdoah->source_node_id));
		offset += get(buffer,
				offset,
				&(tdoah->source_node_id),
				n_bits[SOURCE_NODE_ID_FIELD]);
		memset(&(tdoah->times_size_), 0, sizeof(tdoah->times_size_));
		offset += get(buffer,
				offset,
				&(tdoah->times_size_),
				n_bits[TIMES_SIZE_FIELD]);

		int times_size_bits = tdoah->times_size() * sizeof(tdoa_entry) * 8;
		offset += get(buffer, offset, &(tdoah->times_), times_size_bits);

		if (debug_) {
			std::cout << "\033[1;32;40m (RX) UwRangingTDOA::DATA packer hdr "
						 "\033[0m"
					  << std::endl;
			printMyHdrFields(p);
		}
	}
	return offset;
}

void
packer_uwRangingTDOA::printMyHdrMap()
{
	std::cout << "\033[1;37;45m Packer Name \033[0m: UWRANGINGTDOA \n";
	std::cout << "** DATA fields:\n";
	std::cout << "\033[1;37;45m Field: SOURCE_PKT_ID_FIELD: \033[0m:"
			  << n_bits[SOURCE_PKT_ID_FIELD] << " bits\n";
	std::cout << "\033[1;37;45m Field: SOURCE_NODE_ID_FIELD: \033[0m:"
			  << n_bits[SOURCE_NODE_ID_FIELD] << " bits\n";
	std::cout << "\033[1;37;45m Field: TIMES_SIZE_FIELD: \033[0m:"
			  << n_bits[TIMES_SIZE_FIELD] << " bits\n";
	std::cout << std::endl;
}

void
packer_uwRangingTDOA::printMyHdrFields(Packet *p)
{
	hdr_cmn *hcmn = HDR_CMN(p);

	if (hcmn->ptype() == PT_UWRANGING_TDOA) {
		// Packet to be serialized is a DATA packet
		hdr_ranging_tdoa *tdoah = HDR_RANGING_TDOA(p);
		std::cout << "\033[1;37;45m 1st field \033[0m, SOURCE_PKT_ID_FIELD: "
				  << (int) tdoah->source_pkt_id << std::endl;
		std::cout << "\033[1;37;45m 2nd field \033[0m, SOURCE_NODE_ID_FIELD: "
				  << (int) tdoah->source_node_id << std::endl;
		std::cout << "\033[1;37;45m 3rd field \033[0m, TIMES_SIZE_FIELD: "
				  << (int) tdoah->times_size_ << std::endl;
		std::cout << "\033[1;37;45m 4th field \033[0m, TIMES: ";
		for (int i = 0; i < tdoah->times_size(); i++) {
			std::cout << "(id: " << (int) tdoah->times_[i].id
					  << " node: " << (int) tdoah->times_[i].node
					  << " time: " << tdoah->times_[i].time << ") , ";
		}
		std::cout << endl;
	}
}
