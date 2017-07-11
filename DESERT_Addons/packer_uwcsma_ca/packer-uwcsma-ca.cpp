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

/**
 * @file packer-uwcsma-ca.cpp
 * @author Fderico Favaro
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the ns2 packet of
 * csma-ca into a bit stream, and vice-versa.
 */

#include "packer-uwcsma-ca.h"
#include "uw-csma-ca-hdrs.h"

static class packerUwCsmaCaTcl : public TclClass
{
public:
	packerUwCsmaCaTcl()
		: TclClass("NS2/MAC/UW-CSMA-CA/Packer")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new packerUwCsmaCa());
	}
} class_module_packerUWCsmaCa;

packerUwCsmaCa::packerUwCsmaCa()
	: packer(false)
	, txtime_Bits(0)
{
	bind("txtime_Bits", (int *) &txtime_Bits);
	this->init();
}

void
packerUwCsmaCa::init()
{
	n_bits.clear();
	n_bits.assign(1, 0);
	n_bits[0] = txtime_Bits;
}

size_t
packerUwCsmaCa::packMyHdr(Packet *p, unsigned char *buf, size_t offset)
{
	hdr_cmn *ch;
	ch = HDR_CMN(p);

	if (ch->ptype() == PT_CA_RTS) {

		hdr_ca_RTS *rts;
		rts = CA_RTS_HDR_ACCESS(p);

		offset += put(buf, offset, &(rts->get_tx_time()), txtime_Bits);

		if (debug_) {
			cout << "\033[1;37;45m (TX) UWCSMA-CA::RTS packer hdr \033[0m"
				 << endl;
			printMyHdrFields(p);
		}

	} else if (ch->ptype() == PT_CA_CTS) {
		hdr_ca_CTS *cts;
		cts = CA_CTS_HDR_ACCESS(p);

		offset += put(buf, offset, &(cts->get_tx_time()), txtime_Bits);

		if (debug_) {
			cout << "\033[1;37;45m (TX) UWCSMA-CA::CTS packer hdr \033[0m"
				 << endl;
			printMyHdrFields(p);
		}
	}
	return offset;
}

size_t
packerUwCsmaCa::unpackMyHdr(unsigned char *buf, size_t offset, Packet *p)
{
	hdr_cmn *ch;
	ch = HDR_CMN(p);

	if (ch->ptype() == PT_CA_RTS) {
		hdr_ca_RTS *rts;
		rts = CA_RTS_HDR_ACCESS(p);

		memset(&(rts->get_tx_time()), 0, sizeof(uint8_t));
		offset += get(buf, offset, &(rts->get_tx_time()), txtime_Bits);

		if (debug_) {
			cout << "\033[1;37;45m (RX) UWCSMA-CA::CTS packer hdr \033[0m"
				 << endl;
			printMyHdrFields(p);
		}
	} else if (ch->ptype() == PT_CA_CTS) {
		hdr_ca_CTS *cts;
		cts = CA_CTS_HDR_ACCESS(p);

		memset(&(cts->get_tx_time()), 0, sizeof(uint8_t));
		offset += get(buf, offset, &(cts->get_tx_time()), txtime_Bits);

		if (debug_) {
			cout << "\033[1;37;45m (RX) UWCSMA-CA::CTS packer hdr \033[0m"
				 << endl;
			printMyHdrFields(p);
		}
	}
	return offset;
}

void
packerUwCsmaCa::printMyHdrFields(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);

	if (ch->ptype() == PT_CA_RTS) {
		hdr_ca_RTS *rts;
		rts = CA_RTS_HDR_ACCESS(p);
		cout << "\033[1;37;41m 1st field \033[0m, txTime_: "
			 << rts->get_tx_time() << endl;
	} else if (ch->ptype() == PT_CA_CTS) {
		hdr_ca_CTS *cts;
		cts = CA_CTS_HDR_ACCESS(p);
		cout << "\033[1;37;41m 1st field \033[0m, txTime_: "
			 << cts->get_tx_time() << endl;
	}
}

void
packerUwCsmaCa::printMyHdrMap()
{
	cout << "\033[1;37;45m Packer Name \033[0m: UW-CSMA-CA \n";
	cout << "\033[1;37;45m Field: txTime \033[0m:" << txtime_Bits << " bits\n";
}
