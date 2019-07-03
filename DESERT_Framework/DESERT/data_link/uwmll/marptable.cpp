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

// This module has only slightly modification as respect to the mll module
// contained  in Miracle,
// released under the same BSD copyright and implemented by
// Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen (Karlstad University)

/**
 * @file   marptable.cpp
 * @author Saiful Azad
 * @version 1.0.0
 *
 * \brief Provides the implementation of the ARP table useful for MLL module
 *
 */

#include "marptable.h"

UWARPTable::UWARPTable()
{
}

UWARPTable::~UWARPTable()
{
}

void
UWARPTable::addEntry(UWARPEntry *entry)
{
	table_[entry->ipaddr_] = entry;
}

UWARPEntry *
UWARPTable::lookup(nsaddr_t addr)
{
	map<nsaddr_t, UWARPEntry *>::iterator it;
	it = table_.find(addr);
	if (it != table_.end())
		return it->second;
	else
		return 0;
}

void
UWARPTable::clear()
{
	table_.clear();
}