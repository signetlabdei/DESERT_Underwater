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
 * @file	uwpolling_cmn_hdr.h
 * @author	Federico Favaro
 * @version     1.0.0
 *
 * \brief	Common structures and variables in the protocol
 *
 *
 */

#ifndef UWPOLLING_CMN_HDR_H
#define UWPOLLING_CMN_HDR_H

#include <mmac.h>
#include <module.h>
#include <packet.h>

#include <list>

#define HDR_PROBE(p) \
	(hdr_PROBE::access(p)) /**< alias defined to access the PROBE HEADER */
#define HDR_TRIGGER(p)                                                       \
	(hdr_TRIGGER::access(p)) /**< alias defined to access the TRIGGER HEADER \
								*/
#define HDR_POLL(p) \
	(hdr_POLL::access(p)) /**< alias defined to access the POLL HEADER */
#define HDR_AUV_MULE(p) \
	(hdr_AUV_MULE::access(p))/**< alias defined to access the AUV MULE HEADER */

#define HDR_ACK_SINK(p) \
	(hdr_ACK_SINK::access(p))/**< alias defined to access the ACK SINK HEADER*/
#define HDR_PROBE_SINK(p) \
	(hdr_PROBE_SINK::access(p))/**< alias defined to access the ACK SINK HEADER*/

extern packet_t PT_TRIGGER;
extern packet_t PT_POLL;
extern packet_t PT_PROBE;
extern packet_t PT_PROBE_SINK;
extern packet_t PT_AUV_MULE;
extern packet_t PT_ACK_SINK;

static const int MAX_POLLED_NODE = 4000; /**< Maximum number of node to POLL */
static const double MIN_T_POLL = 5; /**< Minimum duration of the POLL timer */
static const double MIN_T_DATA = 5; /**< Minimum duration of the DATA timer */
static const int MAX_BUFFER_SIZE =
		100; /**< Maximum size of the queue in number of packets */
static const int prop_speed =
		1500; /**< Typical underwater sound propagation speed */

/** Single location of the POLL vector. Each POLL_ID represent a polled node */
typedef struct POLL_ID {
	int id_; /**< ID of the node */
	double t_wait_; /**< Time that a node has to wait before being polled */
} id_poll;

/**
 * Header of the TRIGGER message
 */
typedef struct hdr_TRIGGER {
	uint16_t
			t_in_; /**< Minimum value in which the node can choose his backoff
					  time */
	uint16_t
			t_fin_; /**< Maximum value in which the node can choose his backoff
					   time */
	uint TRIGGER_uid_; /**< TRIGGER packet unique ID */
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	/**
	 * Reference to the t_in variable
	 */
	uint16_t &
	t_in()
	{
		return (t_in_);
	}

	/**
	 * Reference to the t_fin variable
	 */
	uint16_t &
	t_fin()
	{
		return (t_fin_);
	}
	/**
	 * Reference to the TRIGGER_uid variable
	 */
	uint &
	TRIGGER_uid()
	{
		return (TRIGGER_uid_);
	}

	inline static struct hdr_TRIGGER *
	access(const Packet *p)
	{
		return (struct hdr_TRIGGER *) p->access(offset_);
	}
} hdr_TRIGGER;

/**
 * Header of the POLL message
 */
typedef struct hdr_POLL {
	int id_; /**< ID of the POLLED node */
	uint POLL_uid_; /**< POLL packet unique ID */
	uint16_t POLL_time_; /**< Time needed by the AUV to poll all the nodes */
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to the id_ variable
	 */
	int &
	ID()
	{
		return (id_);
	}

	/**
	 * Reference to the POLL_uid_ variable
	 */
	uint &
	POLL_uid()
	{
		return (POLL_uid_);
	}

	uint16_t &
	POLL_time()
	{
	  return (POLL_time_);
	}

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_POLL *
	access(const Packet *p)
	{
		return (struct hdr_POLL *) p->access(offset_);
	}
} hdr_POLL;

/**
 * Header of the PROBE message
 */
typedef struct hdr_PROBE {
	//uint16_t backoff_time_; /**< Backoff time chosen by the node */
	uint16_t ts_; /**< Timestamp of the most recent data packet */
	int n_pkts_; /**< Number of packets that the node wish to transmit to the
					AUV */
	uint id_node_; /**< ID of the node */
	uint PROBE_uid_; /**< Unique ID of the PROBE packet */
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to timestamp variable
	 */
	uint16_t &
	ts()
	{
		return (ts_);
	}

	/**
	 * Reference to backoff_time variable
	 */
/*	uint16_t &
	backoff_time()
	{
		return (backoff_time_);
	}*/

	/**
	 * Reference to n_pkts variable
	 */
	int &
	n_pkts()
	{
		return (n_pkts_);
	}

	/**
	 * Reference to id_node variable
	 */
	uint &
	id_node()
	{
		return (id_node_);
	}

	/**
	 * Reference to PROBE_uid_ variable
	 */
	uint &
	PROBE_uid()
	{
		return (PROBE_uid_);
	}

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_PROBE *
	access(const Packet *p)
	{
		return (struct hdr_PROBE *) p->access(offset_);
	}
} hdr_PROBE;


/**
 * Header of the PROBE message
 */
typedef struct hdr_PROBE_SINK {
	uint id_sink_; /**< ID of the sink */
	uint PROBE_uid_; /**< Unique ID of the PROBE packet */
	uint16_t id_ack_; /**< ID used for ack purpose */
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to id_sink variable
	 */
	uint &
	id_sink()
	{
		return (id_sink_);
	}

	/**
	 * Reference to PROBE_uid_ variable
	 */
	uint &
	PROBE_uid()
	{
		return (PROBE_uid_);
	}

	/**
	 * Reference to id_ack_ variable
	 */
	uint16_t &
	id_ack()
	{
		return (id_ack_);
	}

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_PROBE_SINK *
	access(const Packet *p)
	{
		return (struct hdr_PROBE_SINK *) p->access(offset_);
	}
} hdr_PROBE_SINK;


/**
 * Header of the data sent from AUV MULE to SINK
 */
typedef struct hdr_AUV_MULE {
	uint16_t pkt_uid_;/**< unique ID of the transmitted packet by the AUV node*/
	uint16_t last_pkt_uid_; /** ID of the last packet transmitted in the round*/
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to the pkt_uid_ variable
	 */
	uint16_t &
	pkt_uid()
	{
		return (pkt_uid_);
	}
	
	/**
	 * Reference to the last_pkt_uid_ variable
	 */
	uint16_t &
	last_pkt_uid()
	{
		return (last_pkt_uid_);
	}

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_AUV_MULE *
	access(const Packet *p)
	{
		return (struct hdr_AUV_MULE *) p->access(offset_);
	}
} hdr_AUV_MULE;

/**
 * Header of the ACK sent by the SINK
 */
typedef struct hdr_ACK_SINK {
	std::vector<uint16_t> id_ack_; /**< ACK is the id of the wrong packets */
	static int offset_; /**< Required by the PacketHeaderManager. */

	/**
	 * Reference to the id_ack_ variable
	 */
	std::vector<uint16_t> &
	id_ack()
	{
		return (id_ack_);
	}

	/**
	 * Reference to the offset variable
	 */
	inline static int &
	offset()
	{
		return offset_;
	}

	inline static struct hdr_ACK_SINK *
	access(const Packet *p)
	{
		return (struct hdr_ACK_SINK *) p->access(offset_);
	}
} hdr_ACK_SINK;

#endif
