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
 * @file   uwApplication_module.cpp
 * @author Federico Favaro
 * @version 1.1.0
 *
 * \brief Provides the definition of uwApplicationModule class.
 * This class allows to connect external application with DESERT.
 * Alternatively it can be used as a constant bitrate application.
 *
 */

#ifndef UWAPPLICATION_MODULE_H
#define UWAPPLICATION_MODULE_H

#include <uwApplication_cmn_header.h>
#include <uwip-module.h>
#include <uwudp-module.h>

#include <assert.h>
#include <climits>
#include <math.h>
#include <module.h>
#include <stddef.h>

#include <fcntl.h>
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <chrono>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>

#define UWAPPLICATION_DROP_REASON_UNKNOWN_TYPE \
	"DPUT" /**< Drop the packet. Packet received is an unknown type*/
#define UWAPPLICATION_DROP_REASON_DUPLICATED_PACKET \
	"DPD" /**< Drop the packet. Packet received is already analyzed*/
#define UWAPPLICATION_DROP_REASON_OUT_OF_SEQUENCE \
	"DOOS" /**< Drop the packet. Packet received is out of sequence. */

class uwApplicationModule;

/**
 * uwSendTimerAppl is used to handle the scheduling period of
 * uwApplicationModule packets.
 */
class uwSendTimerAppl : public TimerHandler
{
public:
	uwSendTimerAppl(uwApplicationModule *m)
		: TimerHandler()
	{
		module = m;
	}

	virtual ~uwSendTimerAppl()
	{
	}

protected:
	virtual void expire(Event *e);
	uwApplicationModule *module;
};

class uwApplicationModule : public Module
{
	friend class uwSendTimerAppl;

public:
	/**
	 * Constructor of uwApplicationModule class
	 */
	uwApplicationModule();

	/**
	 * Destructor of uwApplicationModule class
	 */
	virtual ~uwApplicationModule() = default;

	/**
	 * TCL command interpreter. It implements the following OTCL methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 *<i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 *successfully or not.
	 *
	 **/
	virtual int command(int argc, const char *const *argv) override;

	/**
	 * Handle the communication between server and client
	 *
	 * @param clnSock socket obtained after the accept function and use for the
	 *                communication between server and client
	 */

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 */
	virtual void recv(Packet *) override;

	virtual bool listenTCP();
	virtual void acceptTCP();
	virtual void readFromTCP(int clnSock);
	virtual void readFromUDP();
	/**
	 * Calculate the epoch of the event. Used in sea-trial mode
	 * @return the epoch of the system
	 */
	unsigned long int
	getEpoch() const
	{
		unsigned long int timestamp =
				(unsigned long int) (std::chrono::duration_cast<
						std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch())
								.count());
		return timestamp;
	}

	int servSockDescr; /**< socket descriptor for server */
	int clnSockDescr; /**< *socket descriptor for client */
	struct sockaddr_in servAddr; /**< Server address */
	struct sockaddr_in clnAddr; /**< Client address */
	int servPort; /**< Server port*/
	std::queue<Packet *> queuePckReadTCP; /**< Queue that store the DATA packets
											 recevied from the client by the
											 server using a TCP protocol*/
	std::queue<Packet *> queuePckReadUDP; /**< Queue that store the DATA packets
											 recevied from the client by the
											 server using a UDP protocol*/
	std::ofstream out_log; /**< Variable that handle the file in which the
							  protocol write the statistics */
	bool logging;
	int node_id;
	int exp_id;

	/** Maximum size (bytes) of a single read of the socket */
	static uint MAX_READ_LEN;

protected:
	/**
	 * Comupte some statistics as the number of packets sent and receive between
	 * two layer, or control if the packet received is out of sequence.
	 */
	virtual void statistics(Packet *p);

	/**
	 * Set all the field of the DATA packet that must be send down after the
	 * creation
	 * to the below level. In this case the payload of DATA packet are generated
	 * in
	 * a random way.
	 */
	virtual void transmit();

	/**
	 * When socket communication is used, this method establish a connection
	 * between client and server. This is required because a UDP protocol is
	 * used.
	 */
	virtual bool openConnectionUDP();

	/**
	 * Close the socket connection in the case the communication take place with
	 * socket, otherwise stop the execution of the process, so force the
	 * cancellation
	 * of period time generation.
	 */
	virtual void stop();

	/**
	 * Check if the communication take place without socket.
	 *
	 * @return <i>true</i> if communication without socket <i>false</i>
	 * otherwise.
	 */
	virtual bool
	withoutSocket()
	{
		return !socket_active;
	}

	/**
	 * Check if the socket protocol is TCP or UDP.
	 *
	 * @return <i>true</i> if socket uses TCP protocol <i>false</i> if uses UDP.
	 */
	virtual bool
	useTCP()
	{
		return socket_tcp;
	}

	/**
	 * If the communication take place without sockets verify if the data
	 * generation period is constant or a poisson random process
	 *
	 * @return <i>true</i> if use a Poisson process <i>false</i> if use a
	 * constant period data generation
	 */
	virtual bool
	usePoissonTraffic()
	{
		// return poisson_traffic;
		return (poisson_traffic == 1) ? true : false;
	}

	/**
	 * If the communication take place without sockets verify if the data
	 * packets received by the server is out of order or not.
	 * In the first case discard the data packet.
	 *
	 * @return <i>true</i> if enable drop out of order <i>false</i> otherwise.
	 */
	virtual bool
	useDropOutOfOrder()
	{
		return (drop_out_of_order == 1) ? true : false;
	}

	/**
	 * Increase the sequence number and so the number of packets sent by the
	 * server
	 */
	virtual void
	incrPktSent()
	{
		txsn++;
	}

	/**
	 * Increase the number of DATA packets lost by the server
	 */
	virtual void
	incrPktLost(int npkts)
	{
		pkts_lost += npkts;
	}

	/**
	 * Increase the number of DATA packet correctly received by the server
	 */
	virtual void
	incrPktRecv()
	{
		pkts_recv++;
	}
	/**
	 * Increase the number of DATA packets received out of order by the server
	 */
	virtual void
	incrPktOoseq()
	{
		pkts_ooseq++;
	}
	/**
	 * Increse the number of DATA packets received with error by the server
	 */
	virtual void
	incrPktInvalid()
	{
		pkts_invalid++;
	}

	/**
	 * Increase the number of DATA packets stored in the Server queue. This DATA
	 * packets will be sent to the below levels of ISO/OSI stack protocol.
	 */
	virtual void
	incrPktsPushQueue()
	{
		pkts_push_queue++;
	}

	/**
	 * return the number of packets sent by the server
	 *
	 * @return txsn
	 */
	virtual int
	getPktSent() const
	{
		return txsn - 1;
	}
	/**
	 * return the number of DATA packets lost by the server
	 *
	 * @return pkts_lost
	 */
	virtual int
	getPktLost() const
	{
		return pkts_lost;
	}
	/**
	 * return the number of DATA packet correctly received by the server
	 *
	 * @return pkts_recv
	 */
	virtual int
	getPktRecv() const
	{
		return pkts_recv;
	}
	/**
	 * return the number of DATA packets received out of order by the server
	 *
	 * @return pkts_ooseq
	 */
	virtual int
	getPktsOOSequence() const
	{
		return pkts_ooseq;
	}
	/**
	 * return the number of DATA packets received with error by the server
	 *
	 * @return pkts_invalid
	 */
	virtual int
	getPktsInvalidRx() const
	{
		return pkts_invalid;
	}
	/**
	 * return the number of DATA packets sotred in the server queue
	 *
	 * @return pkts_push_queue
	 */
	virtual int
	getPktsPushQueue() const
	{
		return pkts_push_queue;
	}
	/**
	 * return period generation time
	 *
	 * @return period
	 */
	virtual double
	getPeriod() const
	{
		return period;
	}

	/**
	 * Compute the DATA generation rate, that can be constant and equal to the
	 * period established by the user, or can occur with a Poisson process.
	 *
	 * @return generation period for DATA packets
	 */
	virtual double getTimeBeforeNextPkt();

	/**
	 * Returns the average Round Trip Time
	 *
	 * @return the average Round Trip Time
	 */
	virtual double GetRTT() const;

	/**
	 * Return the standard deviation of the Round Trip Time calculated
	 *
	 * @return the standard deviation of the Round Trip Time calculated
	 */
	virtual double GetRTTstd() const;

	/**
	 * Update the RTT after the reception of a new packet
	 *
	 * @param RTT of the current packet received
	 */
	virtual void updateRTT(const double &rtt);

	/**
	 * Returns the average Forward Trip Time
	 *
	 * @return the average Forward Trip Time
	 *
	 */

	virtual double GetFTT() const;
	/**
	 * Return the standard deviation of the Forward Trip Time calculated
	 *
	 * @return the standard deviation of the Forward Trip Time calculated
	 */

	virtual double GetFTTstd() const;
	/**
	 * Rerturn the Packet Error Rate calculated
	 *
	 * @return the Packet Error Rate calculated
	 */

	virtual double GetPER() const;

	/**
	 * Return the Throughput calculated [bps]
	 *
	 * @return Throughput [bps]
	 */
	virtual double GetTHR() const;

	/**
	 * Update the FTT after the reception of a new packet
	 *
	 * @param FTT of the current packet received
	 */
	virtual void updateFTT(const double &ftt);

	/**
	 * Update the Throughput after the reception of a new packet
	 *
	 * @param Throughput of the current packet received
	 */
	virtual void updateThroughput(const int &bytes, const double &dt);

	int debug_; /**< Used for debug purposes <i>1</i> debug activated <i>0</i>
				   debug not activated*/
	double period; /**< Interval time between two successive generation data
				   packets */
	int poisson_traffic; /**< Enable or not the Poisson process for generation
							of data packets <i>1</i> enabled <i>0</i> not
							enabled*/
	int payloadsize; /**< Size of each data packet payaload generated */
	int port_num; /**< Number of the port in which the server provide the
					 service */
	int drop_out_of_order; /**< Enable or not the ordering of data packet
							  received <i>1</i> enabled <i>0</i> not enabled*/
	// int TCP_CMN; /**< Enable or not the use of TCP protocol when is used the
	// socket communication <i>1</i> use TCP <i>0</i> use UDP*/
	uint8_t dst_addr; /**< IP destination address. */

	// TIMER VARIABLES
	uwSendTimerAppl
			chkTimerPeriod; /**< Timer that schedule the period between two
							   successive generation of DATA packets*/

	// STATISTICAL VARIABLES
	bool socket_active;
	bool socket_tcp; // true tcp, udp otherwise
	bool *sn_check; /**< Used to keep track of the packets already received. */
	int uidcnt; /**< Identifier counter that identify uniquely the DATA packet
				   generated*/
	int txsn; /**< Transmission sequence number of DATA packet */
	int rftt; /**< Forward trip time*/
	int pkts_lost; /**< Counter of the packet lost during the transmission */
	int pkts_recv; /**< Counter of the packets correctly received by the
					  server */
	int pkts_ooseq; /**< Counter of the packets received out of order by the
					   server */
	int pkts_invalid; /**< Counter of the packets received with errors by the
						 server */
	int pkts_push_queue; /**< Counter of DATA packets received by server and not
							yet passed to the below levels of ISO/OSI stack
							protocol*/
	int pkts_last_reset; /**< Used for error checking after stats are reset. Set
							to pkts_lost+pkts_recv each time resetStats is
							called. */
	double lrtime; /**< Time of last packet reception. */
	double sumrtt; /**< Sum of RTT samples. */
	double sumrtt2; /**< Sum of (RTT^2). */
	int rttsamples; /**< Number of RTT samples. */
	double sumftt; /**< Sum of FTT samples. */
	double sumftt2; /**< Sum of (FTT^2). */
	int fttsamples; /**< Number of FTT samples. */
	uint32_t esn; /**< Expected serial number. */
	double sumbytes; /**< Sum of bytes received. */
	double sumdt; /**< Sum of the delays. */
	int hrsn; /**< Highest received sequence number. */

	/**Object with the rx thread */
	// union {
	// 	std::thread rx_thread;
	// 	std::thread udp_thread;
	// };
	std::thread rx_thread;

	/** Mutex associated with the transmission queue */
	std::mutex rx_mutex;

}; // end uwApplication_module class
#endif /* UWAPPLICATION_MODULE_H */
