//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
//
// This module has only slightly modification as respect to the mll module contained  in Miracle, 
// released under the same BSD copyright and implemented by 
// Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen (Karlstad University)

/**
 * @file   marptable.h
 * @author Saiful Azad
 * @version 1.0.0
 *
 * \brief Provides the definition of the ARP table of MLL module.
 *
 */

#ifndef UW_ARPTABLE_H
#define UW_ARPTABLE_H

#include <packet.h>
#include <map>

#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL 125
#endif /* !EADDRNOTAVAIL */

#ifndef DROP_IFQ_ARP_FULL
#define DROP_IFQ_ARP_FULL               "ARP"
#endif

#define ARP_MAX_REQUEST_COUNT   3

/**
 * ARP table entry
 */
class UWARPEntry {
	public:
		/**
		 * Constructor
		 * @param dst Destination network address
		 */
		UWARPEntry(nsaddr_t dst) : up_(0), macaddr_(0), ipaddr_(dst), hold_(0), count_(0) { }

		/** Is address up? */
		int		up_;
		/** network address */
		nsaddr_t	ipaddr_;
		/** mac address */
		int		macaddr_;
		/** cached packet */
		Packet		*hold_;
		/** Number of tries */
		int		count_;
};

/**
 * ARP table
 */
class UWARPTable {
	public:
		/** Constructor */
		UWARPTable();
		/** Desctructor */
		~UWARPTable();

		/**
		 * Add entry to ARP table
		 * @param entry UWARPEntry to add
		 */
		void addEntry(UWARPEntry *entry);

		/**
		 * Lookup entry in table
		 * @param addr network address to look for
		 * @return If entry found, a pointer to an UWARPEntry, else null
		 */
		UWARPEntry *lookup(nsaddr_t addr);

		/**
		 * Remove all entries in table
		 */
		void clear();

	private:
		/** The ARP table as an associative array */
		map<nsaddr_t, UWARPEntry*> table_;
};

#endif /* ARPTABLE_H */
