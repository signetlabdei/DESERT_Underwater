//
// Copyright (c) 2019 Regents of the SIGNET lab, University of Padova.
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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>

#include "ahoitypes.h"
#include "uwinterpreterahoi.h"

const uint8_t UwInterpreterAhoi::dle = 0x10;
const uint8_t UwInterpreterAhoi::stx = 0x02;
const uint8_t UwInterpreterAhoi::etx = 0x03;

uint UwInterpreterAhoi::header_size = 6;
uint UwInterpreterAhoi::footer_size = 6;

std::map<ahoi::Command, uint8_t> ahoi::commands_id{
		std::make_pair(ahoi::Command::send, 0x00), // can be anything in [0,126]
		std::make_pair(ahoi::Command::id, 0x84),
		std::make_pair(ahoi::Command::batvol, 0x85),
		std::make_pair(ahoi::Command::reset, 0x87),
		std::make_pair(ahoi::Command::agc, 0x98),
		std::make_pair(ahoi::Command::rxgain, 0x99),
		std::make_pair(ahoi::Command::txgain, 0x9A),
		std::make_pair(ahoi::Command::range_delay, 0xA8),
		std::make_pair(ahoi::Command::distance, 0xA9),
		std::make_pair(ahoi::Command::packetstat, 0xC0),
		std::make_pair(ahoi::Command::packetstatreset, 0xC1),
		std::make_pair(ahoi::Command::syncstat, 0xC2),
		std::make_pair(ahoi::Command::syncstatreset, 0xC3),
		std::make_pair(ahoi::Command::sfdstat, 0xC4),
		std::make_pair(ahoi::Command::sfdstatreset, 0xC5),
		std::make_pair(ahoi::Command::allstat, 0xC6),
		std::make_pair(ahoi::Command::allstatreset, 0xC7),
		std::make_pair(ahoi::Command::confirm, 0xFF)};

UwInterpreterAhoi::UwInterpreterAhoi(int id)
	: id(id)
	, sn(0)
	, beg_del()
	, end_del()
{
	beg_del[0] = dle;
	beg_del[1] = stx;

	end_del[0] = dle;
	end_del[1] = etx;
}

UwInterpreterAhoi::~UwInterpreterAhoi()
{
}

std::string
UwInterpreterAhoi::serializePacket(ahoi::packet_t *packet)
{
	std::string serialized{""};

	uint8_t *raw_pck = (uint8_t *) packet;

	serialized += dle;
	serialized += stx;

	for (uint i = 0; i < header_size + packet->header.len; i++) {
		if (*raw_pck == dle) {
			serialized += dle;
		}
		serialized += *raw_pck++;
	}

	serialized += dle;
	serialized += etx;

	return serialized;
}

std::string
UwInterpreterAhoi::buildSend(ahoi::packet_t pck)
{

	return serializePacket(&pck);
}

std::string
UwInterpreterAhoi::buildID(int id)
{

	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::id];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildBatVol()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::batvol];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildReset()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::reset];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildAgc()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::agc];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildRangeDelay()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::range_delay];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildDistance()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::distance];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildRxGain()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::rxgain];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildTxGain()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::txgain];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildPacketStat()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::packetstat];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildPacketStatReset()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::packetstatreset];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildAllStat()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::allstat];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildAllStatReset()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::allstatreset];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildSfdStat()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::sfdstat];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildSfdStatReset()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::sfdstatreset];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildSyncStat()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::syncstat];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::buildSyncStatReset()
{
	ahoi::header_t header;
	header.src = id;
	header.dst = 0xFF;
	header.type = ahoi::commands_id[ahoi::Command::syncstatreset];
	header.status = 0x00;
	header.dsn = sn++;
	header.len = 0;

	ahoi::packet_t packet;
	packet.header = header;

	return serializePacket(&packet);
}

std::string
UwInterpreterAhoi::findResponse(std::vector<char>::iterator beg,
		std::vector<char>::iterator end, std::vector<char>::iterator &rsp_beg,
		std::vector<char>::iterator &rsp_end)
{
	std::string cmd = "";
	rsp_beg = beg;
	rsp_end = beg;

	if (beg == end)
		return "";

	// New search for DLE+STX and DLE+ETX
	auto it = std::find(beg, end, dle);
	while (it != end) {

		if (*(it + 1) == dle)
			it = it + 2;
		else if (*(it + 1) == stx)
			rsp_beg = it;
		else if (*(it + 1) == etx)
			rsp_end = it + end_del.size();
		;

		it = std::find(it + 1, end, dle);
	}

	if (std::distance(rsp_beg, rsp_end) < ahoi::HEADER_LEN || rsp_end < rsp_beg)
		return std::string("");
	else
		return std::string(&(*rsp_beg), std::distance(rsp_beg, rsp_end));
}

void
UwInterpreterAhoi::fixEscapes(std::vector<char> &buffer,
		std::vector<char>::iterator &c_beg, std::vector<char>::iterator &c_end)
{

	auto it_1 = c_beg;
	auto it_2 = c_beg;
	auto it = c_beg;

	while (it < buffer.end() - 1) {

		it_1 = std::find(it, c_end, dle);
		if ((it_2 = std::find(std::next(it_1, 1), c_end, dle)) ==
				std::next(it_1, 1)) {

			it = buffer.erase(it_2);
		} else
			it++;
	}
}

std::shared_ptr<ahoi::packet_t>
UwInterpreterAhoi::parseResponse(
		std::vector<char>::iterator c_beg, std::vector<char>::iterator c_end)
{
	std::shared_ptr<ahoi::packet_t> pck = std::make_shared<ahoi::packet_t>();
	std::shared_ptr<ahoi::header_t> head = std::make_shared<ahoi::header_t>();
	std::shared_ptr<ahoi::footer_t> foot = std::make_shared<ahoi::footer_t>();

	auto it = std::next(c_beg, beg_del.size());

	// everything is single character
	// source address
	auto it_n = std::next(it, 1);
	if (it_n > c_end)
		return nullptr;
	std::copy(it, it_n, &(head->src));
	// destination address
	it = it_n;
	it_n = std::next(it, 1);
	if (it_n > c_end)
		return nullptr;
	std::copy(it, it_n, &(head->dst));
	// pck_type
	it = it_n;
	it_n = std::next(it, 1);
	if (it_n > c_end)
		return nullptr;
	std::copy(it, it_n, &(head->type));
	// status
	it = it_n;
	it_n = std::next(it, 1);
	if (it_n > c_end)
		return nullptr;
	std::copy(it, it_n, &(head->status));
	// sequence number
	it = it_n;
	it_n = std::next(it, 1);
	if (it_n > c_end)
		return nullptr;
	std::copy(it, it_n, &(head->dsn));
	// payload length
	it = it_n;
	it_n = std::next(it, 1);
	if (it_n > c_end)
		return nullptr;
	std::copy(it, it_n, &(head->len));

	pck->header = *head;

	// payload
	uint len = head->len;
	it = std::next(it, 1);
	if (len > 0) {
		std::memcpy(&(pck->payload), &*it, (size_t) len);
	} else {
		return pck;
	}

	// footer
	it = it + len;
	if (it < c_end + end_del.size()) {
		// power level
		it_n = std::next(it, 1);
		if (it_n > c_end)
			return nullptr;
		std::copy(it, it_n, &(foot->power));
		// RSSI
		it = it_n;
		it_n = std::next(it, 1);
		if (it_n > c_end)
			return nullptr;
		std::copy(it, it_n, &(foot->rssi));
		// Number of bit errors
		it = it_n;
		it_n = std::next(it, 1);
		if (it_n > c_end)
			return nullptr;
		std::copy(it, it_n, &(foot->biterrors));
		// mean gain
		it = it_n;
		it_n = std::next(it, 1);
		if (it_n > c_end)
			return nullptr;
		std::copy(it, it_n, &(foot->agcMean));
		// Minimum gain
		it = it_n;
		it_n = std::next(it, 1);
		if (it_n > c_end)
			return nullptr;
		std::copy(it, it_n, &(foot->agcMin));
		// Maximum gain
		it = it_n;
		it_n = std::next(it, 1);
		if (it_n > c_end)
			return nullptr;
		std::copy(it, it_n, &(foot->agcMax));

		pck->footer = *foot;
	} else {
	}

	return pck;
}
