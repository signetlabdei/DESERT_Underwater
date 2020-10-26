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

#ifndef AHOIPACKETTYPES_H
#define AHOIPACKETTYPES_H

#include <map>
#include <stdint.h>
#include <sys/types.h>

namespace ahoi
{

enum class Command {
	send,
	confirm,
	id,
	batvol,
	reset,
	agc,
	rxgain,
	txgain,
	range_delay,
	distance,
	packetstat,
	packetstatreset,
	syncstat,
	syncstatreset,
	sfdstat,
	sfdstatreset,
	allstat,
	allstatreset
};

extern std::map<ahoi::Command, uint8_t> commands_id;

// TYPES AND VARIABLE
constexpr uint AHOI_TYPE_ACK = ((1 << (8 * sizeof(uint8_t) - 1)) - 1);
constexpr uint AHOI_SERIAL_ACK = ((1 << (8 * sizeof(uint8_t))) - 1);
constexpr uint AHOI_ADDR_BCAST = ((1 << (8 * sizeof(uint8_t))) - 1);

// maximum payload length of a packet
constexpr uint PAYLOAD_MAXLEN = 128;

// PACKET STRUCTS
// packet header
struct __attribute__((__packed__)) header_t {
	uint8_t src; // source address
	uint8_t dst; // destination address
	uint8_t type; // packet type
	uint8_t status; // status flags
	uint8_t dsn; // sequence number
	uint8_t len; // payload length in bytes
};

struct __attribute__((__packed__)) footer_t {
	uint8_t power;
	uint8_t rssi;
	uint8_t biterrors;
	uint8_t agcMean;
	uint8_t agcMin;
	uint8_t agcMax;
};

struct __attribute__((__packed__)) packet_t {
	ahoi::header_t header;
	uint8_t payload[PAYLOAD_MAXLEN];
	ahoi::footer_t footer;
};

constexpr uint MAX_PKT_LEN = (sizeof(packet_t));
constexpr uint HEADER_LEN = (sizeof(header_t));
constexpr uint FOOTER_LEN = (sizeof(footer_t));

constexpr uint MM_PAYLOAD_CRC_LEN = 2; /* bytes */
constexpr uint MM_HEADER_CRC_LEN = 1; /* bytes */

// ack setup
constexpr uint ACK_NONE = 0x00; // do not request ack
constexpr uint ACK_PLAIN = 0x01; // request ack
constexpr uint ACK_RANGING = 0x02; // request ack with ranging info

// STRUCT FOR COMMAND PACKETS
struct __attribute__((__packed__)) agc_cmd {
	bool en; // agc enable/disable (true/false)
};

struct __attribute__((__packed__)) batvoltage_rsp {
	uint16_t voltage;
};

struct __attribute__((__packed__)) config_rsp {
	char descr[1];
};

struct __attribute__((__packed__)) filterraw_cmd {
	uint8_t stage; // filter stage
	uint8_t value; // raw value
};

struct __attribute__((__packed__)) filterraw_rsp {
	struct __attribute__((__packed__)) {
		uint8_t stage; // filter stage
		uint8_t value; // raw value
	} stages[1];
};

struct __attribute__((__packed__)) freqsetup_numbands_cmd {
	uint8_t numBands;
};

struct __attribute__((__packed__)) freqsetup_numcarriers_cmd {
	uint8_t numCarriers;
};

struct __attribute__((__packed__)) id_cmd {
	uint8_t id; // modem id (optional)
};

struct __attribute__((__packed__)) packetstat_rsp {
	uint16_t numTx; // sent packets
	uint16_t numSync; // successful syncs
	uint16_t numSfd; // successful sfds
	uint16_t numRxComplete; // received (intact) packets
	uint16_t numRxHeaderBadCrc; // crc
	uint16_t numRxHeaderBitErr; // repairable bit errors
	uint16_t numRxHeaderBitErrFatal; // non-repairable bit errors
	uint16_t numRxPayloadBadCrc; // crc
	uint16_t numRxPayloadBitErr; // repairable bit errors
	uint16_t numRxPayloadBitErrFatal; // non-repairable bit errors
	uint16_t numRxCritError; // critical errors (unexpected behavior)
};

struct __attribute__((__packed__)) peakwinlen_cmd {
	uint16_t winlen; // length in micro seconds
};

struct __attribute__((__packed__)) powerlevel_rsp {
	uint8_t power; // power value (only for reply)
};

struct __attribute__((__packed__)) rxgain_cmd {
	uint8_t stage; // filter stage
	uint8_t level; // gain level
};

struct __attribute__((__packed__)) rxgain_rsp {
	struct __attribute__((__packed__)) {
		uint8_t stage; // filter stage
		uint8_t level; // gain level
		uint8_t num; // number of levels
	} stages[1];
};

struct __attribute__((__packed__)) rxthresh_cmd {
	uint8_t rxThresh; // in % of max amplitude
};

struct __attribute__((__packed__)) sample_cmd {
	uint8_t trigger; // the required trigger (ACI_SAMPLE_TRIGGER_*)
	uint16_t numSamples; // total number of samples (including delay)
	uint16_t numDelay; // additional samples after the trigger event
};

struct __attribute__((__packed__)) sniff_cmd {
	bool en;
};

struct __attribute__((__packed__)) spreadcode_cmd {
	uint8_t codelen; // code length (1, ..., max code len)
};

struct __attribute__((__packed__)) synclen_cmd {
	uint8_t txlen;
	uint8_t rxlen;
};

struct __attribute__((__packed__)) test_dirac_cmd {
	uint8_t rep; // number of repetitions
};

struct __attribute__((__packed__)) test_freq_cmd {
	uint8_t freqNum; // frequency number "n * df" (0 < n <= FREQ_LIST_NUM)
	uint8_t lvl; // loudness level (% of max. (<=100), 0 == gain compensated)
};

struct __attribute__((__packed__)) test_noise_cmd {
	bool gc; // gain compensation?
	uint8_t step; // stepping (only every nth frequency)
	uint8_t dur; // duration in multiple of symbol (>= 1)
};

struct __attribute__((__packed__)) test_sweep_cmd {
	bool gc; // gain compensation?
	uint8_t gap; // gap between frequencies in multiple of symbol
};

struct __attribute__((__packed__)) transducer_cmd {
	uint8_t type; // transducer to use
};

struct __attribute__((__packed__)) transducer_rsp {
	uint8_t type; // currently used transducer
	char descr[1]; // textual description of tansducer type
};

struct __attribute__((__packed__)) txgain_cmd {
	uint8_t lvl;
};
} // namespace ahoi

#endif
