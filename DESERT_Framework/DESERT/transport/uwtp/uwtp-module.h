//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwtp-module.h
 * @author Saiful Azad
 * @version 1.0.0
 *
 * @brief  This is the base class of a simple transport layer protocol.
 */

#ifndef UWTP_H_
#define UWTP_H_

#include "uwtp-pkt.h"
#include <map>
#include <module.h>
#include <random.h>
#include <timer-handler.h>
#include <utility>
#include <vector>

#define TIME Scheduler::instance().clock()
#define JITTER Random::uniform()
#define RNV Random::uniform()
#define MAX_PORT_NO 65535

#define UNRECOGNIZED_PORT_NUMBER "UPN"
#define LOWER_THAN_EXPECTED_SEQUENCE_NUMBER "LTESN"

typedef int SeqNo;
typedef int PortNo;
typedef int ExpectedPktSeqNo;

typedef pair<PortNo, SeqNo> UWTPPair;

enum ACK_MODE { WITH_ACK = 1, WITHOUT_ACK };
enum ACK_TX_MODE { WITH_CUM_ACK = 1, WITHOUT_CUM_ACK };

class UWTP;

/**
 *UWTP_delayTimer class is used to delay the sending of nack packets, because,
 *if different packets are transmitted through different route; it may happen
 *that a packet with higher sequence number may reach early than the lower one.
 *If we delay the tranmission of nack packet, we may avoid retransmitting the
 *same packet. In this case, whenever a nodes receives a data packet it check
 *its nack packet buffer whether there is any nack packet created for that data
 *packet. If it finds a nack packet, it deletes that nack packet.
 */
class UWTP_delayTimer : public TimerHandler
{

public:
	UWTP_delayTimer(UWTP *m)
	{
		module = m;
	}

protected:
	UWTP *module;
	virtual void expire(Event *e);
};

/**
 *UWTPPktStoreInfo class is used to store the packet in the buffer with some
 *related information.
 */
class UWTPPktStoreInfo
{
public:
	/**
	 *Constructor of UWTPPktStoreInfo class
	 */
	UWTPPktStoreInfo()
		: pkt_store_time(0.0)
		, pkt_tx_info(FALSE)
	{
	}

	/**
	 *Destructor of UWTPPktStoreInfo class
	 */
	~UWTPPktStoreInfo()
	{
	}

	/**
	 *Store the time of the packet when it is saved in the buffer
	 *@param time when the packet is stored
	 */
	void
	setPktStoreTime(double time)
	{
		pkt_store_time = time;
	}

	/**
	 *Set the pointer of the initialized data packet
	 *@param Packet pointer
	 */
	void
	setPktPnt(Packet *pa)
	{
		p = pa;
	}

	/**
	 *Set the packet transmission information.
	 *@param transmission information
	 */
	void
	setPktTxInfo(bool tx_info)
	{
		pkt_tx_info = tx_info;
	}

	/**
	 *Return the time when the packet is stored
	 *@return packet store time
	 */
	double
	getPktStoreTime()
	{
		return pkt_store_time;
	}

	/**
	 *Return the spent time of the packet in the queue
	 *@return spent time in the queue
	 */
	double
	getTimeSpentInQueue()
	{
		return TIME - getPktStoreTime();
	}

	/**
	 *Return the pointer of the stored packet
	 *@return Packet pointer
	 */
	Packet *
	getPktPnt()
	{
		return p;
	}

	/**
	 *Return the transmission information of the packet
	 *@return <i>true</i> if packet is transmitted, <i>false</i> otherwise
	 */
	bool
	getPktTxInfo()
	{
		return pkt_tx_info;
	}

protected:
	double pkt_store_time; /**< When the packet is stored, this variable is used
							* to keep track of the period of storing of this
							* packet in the queue. It will help the node to
							* delete the packets which are stored for the long
							* time in the queue in case of no ACK based
							* scenario. */

	Packet *p; /**< This is the pointer of the packet which is initialized.
				* After receiving a packet from upper layer, a node first
				* initialize this packet and save it in the buffer. In case of
				* ACK based case, the packet is deleted after receiving the ACK.
				* If any NACK packet is received, this packet is retransmitted.
				*/

	bool pkt_tx_info; /**< This is variable is used to find out whether this
					   * packet is already transitted or not. If TRUE, that
					   * means, packet is already transmitted, if FALSE, packet
					   * is not yet transmitted. */
};

/**
 *NackPktStoreInfo is used to store the NACK packet in the buffer with valuable
 *information. As mentioned earlier, we are storing nack packet for some time
 *before transmitting to avoid retransmission of successfully received packet.
 */
class NackPktStoreInfo
{
public:
	/**
	 *Constructor of NackPktStoreInfo class
	 */
	NackPktStoreInfo()
		: nack_tx_time(0.0)
		, nack_tx(FALSE)
	{
	}

	/**
	 *Destructor of NackPktStoreInfo class
	 */
	~NackPktStoreInfo()
	{
	}

	/**
	 *Store the time when the NACK packet is saved in the buffer
	 *@param time when the NACK packet is stored
	 */
	void
	setNackTxTime(double time)
	{
		nack_tx_time = time;
	}

	/**
	 *Set the pointer of the initialized NACK packet
	 *@param Packet pointer
	 */
	void
	setNackPnt(Packet *pa)
	{
		p = pa;
	}

	/**
	 *Set the NACk packet transmission information.
	 *@param transmission information
	 */
	void
	setNackTxInfo(bool tx_info)
	{
		nack_tx = tx_info;
	}

	/**
	 *Return the time when the NACK packet is stored
	 *@return NACK packet store time
	 */
	double
	getNackTxTime()
	{
		return nack_tx_time;
	}

	/**
	 *Return the spent time of the NACK packet in the queue/buffer
	 *@return spent time in the queue
	 */
	double
	getTimeSpentInNackBuffer()
	{
		return TIME - getNackTxTime();
	}

	/**
	 *Return the pointer of the stored NACK packet
	 *@return NACK Packet pointer
	 */
	Packet *
	getNackPnt()
	{
		return p;
	}

	/**
	 *Return the transmission information of the NACK packet
	 *@return <i>true</i> if NACK packet is transmitted, <i>false</i> otherwise
	 */
	bool
	getNackTxInfo()
	{
		return nack_tx;
	}

protected:
	double nack_tx_time; /**< When the NACK packet is stored, this variable is
						  *used to keep track of the period storing of this NACK
						  *packet in the queue. If the packet is stored for long
						  *time, which means the data packet against this nack
						  *packet is not received yet (because of data packet
						  *loss or nack packet loss), in such cases, the node
						  *retransmits the NACK packet.
						  */

	Packet *p; /**< This is the pointer of the NACK packet which is initialized.
				*In case of data packet loss or nack packet loss like mentioned
				*earlier, this packet is retransmitted again.
				*/

	bool nack_tx; /**< This boolean variable is used to find out whether this
				   *NACK packet is already transitted or not. If TRUE, that
				   *means, packet is already transmitted, if FALSE, packet is
				   *not yet transmitted.
				   */
};

extern packet_t PT_UWTP;

/**
 *This is the base class of UnderWater TransPort (UWTP) protocol, which is a
 *derived class of Module
 */

class UWTP : public Module
{

	friend class UWTP_delayTimer;
	friend class UWTPPktStoreInfo;
	friend class NackPktStoreInfo;

public:
	/**
	 *Constructor of UWTP class
	 */
	UWTP();

	/**
	 *Destructor of UWTP class
	 */
	virtual ~UWTP();

	/**
	 *This function is used to receive a packet from upper layer. But since,
	 *upper layer did not put any id of the port, the packet is deleted
	 *@param Packet pointer
	 */
	virtual void recv(Packet *p);

	/**
	 *Performs the reception of packets from upper layer and lower layer
	 *@param Packet pointer
	 *@param Id of the src
	 */
	virtual void recv(Packet *p, int idSrc);

	/**
	 *It assigns the port numbers for different applications to identify them
	 *and send packet accordingly to the right application
	 *@param Module
	 *@return an integer port number
	 */
	virtual int assignPort(Module *m);

	/**
	 *Initializing the data packet. Here, a packet is associated with a header.
	 *@param Packet pointer
	 *@param id of the node
	 *@param Sequence_number of the packet
	 */
	virtual void initPkt(Packet *p, int id, int seq_no_counter);

	/**
	 *Initializing the ACK packet. Here, an ACK packet is associated with a
	 *header.
	 *@param Data_Packet pointer
	 *@param ACK_Packet pointer
	 *@param Sequence_number
	 */
	virtual void initAckPkt(Packet *p, Packet *ack_pkt, int seq_no);

	/**
	 *Initializing the NACK packet. Here, a NACK packet is associated with a
	 *header.
	 *@param Data_Packet pointer
	 *@param NACK_Packet pointer
	 *@param Sequence_number
	 */
	virtual void initNackPkt(Packet *p, Packet *nack_pkt, int nack_seq_no);

	/**
	 *Send an ACK packet to the source from where it receives the data packet.
	 *@param ACK_Packet pointer
	 */
	virtual void sendAck(Packet *p);

	/**
	 *Send a NACK packet which is not yet transmitted or whose data packet is
	 *not yet receive after certain time
	 */
	virtual void sendNack();

	/**
	 *Receives the data packet. It sends an ACK packet when acknowledge mode is
	 *enable and then check wherether the received packet's sequence number is
	 *equal to the expected sequence numnber. If equal then it'll send the
	 *packet to the application, otherwise it does two thing: 1) if sequence
	 *number is lower it drops the packet, and 2) if sequence number is higher,
	 *it creates NACK packet(s) for which NACK packet(s) are not already
	 *created.In case of cumulative acknowledgement, instead of sending ACK for
	 *every packet, cumulitive ACK packet is sent.
	 *@param Packet
	 *@param id of the source
	 */
	virtual void recvData(Packet *p, int id);

	/**
	 *When an ACK packet is received, it deletes the packet from the buffer. In
	 *case of cumulative acknowledgement, packets less than or equal to the
	 *sequence number are deleted from the buffer.
	 *@param ACK_Packet
	 */
	virtual void recvAck(Packet *p);

	/**
	 *When a NACK packet is received, the source retransmits the packet again
	 *@param NACK_Packet
	 */
	virtual void recvNack(Packet *p);

	/**
	 *It checks whether any NACK packet is saved in the buffer for receive data
	 *packet.
	 *@param Port_No
	 *@param Sequence_number
	 */
	virtual void checkNack(int port_no, int seq_no);

	/**
	 *This function checks the receive buffer to find out any packet possible to
	 *send to the upper layer.
	 *@param Port_No
	 *@param Id of the src
	 */
	virtual void checkReceiveQueue(int port, int id);

	/**
	 *This function tells the Buffer size of the sender.
	 *@return sender buffer size
	 */
	virtual int
	getSendBufferSize()
	{
		return send_buffer_size;
	}

	/**
	 *This function tells the Receiver Buffer size.
	 *@return receiver buffer size
	 */
	virtual int
	getReceiveBufferSize()
	{
		return receive_buffer_size;
	}

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the command parameters (Note that
	 *argv[0] is the name of the object)
	 *
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 *succesfully or not
	 *
	 **/
	virtual int command(int argc, const char *const *argv);

	inline void
	setId(int id_)
	{
		node_id_ = id_;
	}
	inline int
	getId()
	{
		return node_id_;
	}

protected:
	int debug_; /**< Set debug flag of UWTP class. */

	int send_buffer_size; /**< Size of the sender buffer. */

	int receive_buffer_size; /**< Size of the receive buffer. */

	double delay_interval; /**< Set the maximum delay time to be considered
							  before transmitting a NACK packet. */

	int ack_tx_count; /**< Count the number of ACK packets are sent. */

	int nack_tx_count; /**< Count the number of NACK packets are transmitted. */

	int ack_rx_count; /**< Count the number of ACK packets received. */

	int nack_rx_count; /**< Count the number of NACK packets received. */

	int destPort_; /**< Destination port number. */

	int seq_no_counter; /**< This variable is used to generate the sequence
						   number of the data packet. */

	double nack_retx_time; /**< When NACK will be retransmitted again in case of
							*NACK packet loss, which results in no
							*retransmission of expected packet. It will force
							*the receiver the send the NACK packet again.
							*/

	double pkt_delete_time_from_queue; /**< This variable is used for two
										*purposes, they are: 1) in case of UWTP
										*without ACK enable, this timer helps
										*deleting the packet from the queue, and
										*2) in case of ACK packet loss, it
										*deletes that packet after this time
										*expire
										*/

	double expected_ACK_threshold; /**< It is used to limit the number of ACK
									*packets transmission in cumulative ACK mode
									*of UWTP. If generated random value is
									*grater than this value then an ACK will be
									*initialized, otherwise no ACK will be sent
									*for the receive packet.
									*/

	UWTP_delayTimer delay_timer_; /**< This is the object of UWTP_delayTimer. */

	ACK_MODE ack_mode; /**< Sets the acknowledgement mode of the UWTP protocol.
						*/

	ACK_TX_MODE ack_tx_mode; /**< Sets the cumulative acknowledgement mode of
								the UWTP protocol. */

	UWTPPktStoreInfo *pkt_store_info; /**< Object pointer of the class
										 UWTPPktStoreInfo. */

	NackPktStoreInfo *nack_store_info; /**< Object pointer of the class
										  NACKPktStoreInfo. */

	map<UWTPPair, UWTPPktStoreInfo *>
			sendBuffer; /**< Send buffer is created with a map container, where
						 * pair of port_no of the destination and sequence
						 * number of the packet is used as key value.
						 */

	map<UWTPPair, Packet *>
			receiveBuffer; /**< Receive buffer is created with a map container,
							* where pair of port_no of the destination and
							* sequence number of the packet is used as key
							* value.
							*/

	map<UWTPPair, NackPktStoreInfo *>
			nackBuffer; /**< NACK buffer is created with a map container, where
						 * pair of port_no of the destination and sequence
						 * number of the packet is used as key value.
						 */

	map<PortNo, ExpectedPktSeqNo>
			expPktInfo; /**< Expected packet information container is created
						 * with a map, where port_no of the destination is used
						 * as key value.
						 */

	int portcounter; /**< counter to generate new port numbers. */

	map<int, int> port_map; /**< Container to store the packet. value = port;
							   key = id. */

	map<int, int>
			id_map; /**< Container to keep the id. value = id;    key = port */

	virtual int getUWTPDataHSize(); /**< This function is used to get the header
									 * size of the data packet which can be used
									 * to calculate overhead in data packet.
									 */

	virtual int
	getUWTPAckHSize(); /**< It tells the header size of the ACK packet.
						*/

	virtual int
	getUWTPNackHSize(); /**< It tells the header size of the NACK packet. */

	int node_id_; /**< Identification number of the current node. */
};

#endif /*  _UWTP_H_ */
