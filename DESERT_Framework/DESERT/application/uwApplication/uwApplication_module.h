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
#include <module.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <atomic>
#include <chrono>
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
	virtual ~uwApplicationModule();

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
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 */
	virtual void recv(Packet *) override;

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
	 * Return the number of DATA packets sorted in the server queue.
	 *
	 * @return pkts_push_queue
	 */
	virtual int
	getPktsPushQueue() const
	{
		return pkts_push_queue;
	}

	/**
	 * Return period generation time.
	 *
	 * @return period
	 */
	virtual double
	getPeriod() const
	{
		return period;
	}

	/**
	 * Calculate the epoch of the event. Used in sea-trial mode.
	 * @return the epoch of the system.
	 */
	std::string
	getEpoch() const
	{
		unsigned long int timestamp =
				(unsigned long int) (std::chrono::duration_cast<
						std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch())
								.count());

		return to_string(timestamp);
	}

	/**
	 * Method to send the log message to the logger.
	 * If sea_trial enabled add epoch of the event to the message and
	 * use node_id passed via tcl.
	 *
	 * @param log_level LogLevel representing the amout of logs.
	 * @param module String name of the plugin/module.
	 * @param message String log message.
	 *
	 */
	virtual void
	printOnLog(Logger::LogLevel log_level, const std::string &module,
			const std::string &message) const override
	{
		if (enable_log) {
			if (sea_trial)
				logger.printOnLog(log_level,
						"[" + getEpoch() + "]::" + module + "(" +
								to_string(node_id) + ")::" + message);
			else
				PlugIn::printOnLog(log_level, module, message);
		}
	}

protected:
	/**
	 * Set all the field of the DATA packet that must be send down after the
	 * creation
	 * to the below level. In this case the payload of DATA packet are generated
	 * in
	 * a random way.
	 */
	virtual void transmit();

	/** Method that binds the listening TCP socket.
	 *
	 * @return true if the listening socket is bind and open.
	 */
	virtual bool listenTCP();

	/**
	 * Method that puts in place a listening TCP socket.
	 *
	 */
	virtual void acceptTCP();

	/**
	 * Method that reads a TCP byte stream from external application and
	 * converts it to a Packet.
	 *
	 * @param clnSock int client file descriptor.
	 */
	virtual void readFromTCP(int clnSock);

	/**
	 * When socket communication is used, this method establish a connection
	 * between client and server. This is required because a UDP protocol is
	 * used.
	 *
	 * @return true if the  socket is bind.
	 */
	virtual bool openConnectionUDP();

	/**
	 * Method that waits for UDP packets from external application and converts
	 * it to a Packet.
	 *
	 */
	virtual void readFromUDP();

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
	 * Compute the DATA generation rate, that can be constant and equal to the
	 * period established by the user, or can occur with a Poisson process (only
	 * without socket).
	 *
	 * @return generation period for DATA packets
	 */
	virtual double getTimeBeforeNextPkt();

	/**
	 * Update the RTT after the reception of a new packet
	 *
	 * @param RTT of the current packet received
	 */
	virtual void updateRTT(double rtt);

	/**
	 * Returns the average Forward Trip Time
	 *
	 * @return the average Forward Trip Time
	 *
	 */

	/**
	 * Update the FTT after the reception of a new packet
	 *
	 * @param FTT of the current packet received
	 */
	virtual void updateFTT(double ftt);

	/**
	 * Update the Throughput after the reception of a new packet
	 *
	 * @param Throughput of the current packet received
	 */
	virtual void updateThroughput(int bytes, double dt);

	bool socket_active; /** Flag set to true if packets are received from
						   external application. */
	bool socket_tcp; /** Flag set to true if the external application is
						connected via a TCP socket, false if UDP. */
	bool *sn_check; /**< Used to keep track of the packets already received. */
	uint8_t dst_addr; /**< Destination IP address. */
	int poisson_traffic; /**< Poisson process for generation of data packets
							<i>1</i> enabled <i>0</i> not enabled. */
	int payloadsize; /**< Size of each data packet payaload generated. */
	int port_num; /**< Destination port number. */
	int drop_out_of_order; /**< Ordering of data packet received <i>1</i>
							  enabled <i>0</i> not enabled. */
	int uidcnt; /**< Identifier counter that identify uniquely the DATA packet
				   generated. */
	int hrsn; /**< Highest received sequence number. */
	int txsn; /**< Transmission sequence number of DATA packet. */
	int rftt; /**< Forward round trip time. */
	int pkts_lost; /**< Counter of the packet lost during the transmission. */
	int pkts_recv; /**< Counter of the packets correctly received by the server.
					*/
	int pkts_ooseq; /**< Counter of the packets received out of order by the
					   server. */
	int pkts_invalid; /**< Counter of the packets received with errors by the
						 server. */
	int pkts_push_queue; /**< Counter of DATA packets received by server and not
							yet passed to the below levels of ISO/OSI stack
							protocol. */
	int pkts_last_reset; /**< Used for error checking after stats are reset. Set
							to pkts_lost+pkts_recv each time resetStats is
							called. */
	int sea_trial; /**< Set to 1 to enable epoch time in log output. */
	int node_id; /**< Node id to be print in log output. */
	int servSockDescr; /**< Socket descriptor for server. */
	int clnSockDescr; /**< Socket descriptor for client. */
	int servPort; /**< Socket server port. */
	uint32_t esn; /**< Expected serial number. */
	int rttsamples; /**< Number of RTT samples. */
	int fttsamples; /**< Number of FTT samples. */
	double period; /**< Time between successive generation data packets. */
	double lrtime; /**< Time of last packet reception. */
	double sumrtt; /**< Sum of RTT samples. */
	double sumrtt2; /**< Sum of (RTT^2). */
	double sumftt; /**< Sum of FTT samples. */
	double sumftt2; /**< Sum of (FTT^2). */
	double sumbytes; /**< Sum of bytes received. */
	double sumdt; /**< Sum of the delays. */

	struct sockaddr_in servAddr; /**< Server address. */
	struct sockaddr_in clnAddr; /**< Client address. */
	std::thread socket_thread; /**< Object with the socket rx thread */
	std::mutex socket_mutex; /**< Mutex associated with the socket rx thread */
	std::atomic<bool> receiving; /** Atomic boolean variable that controls the
									socket rx looping thread */
	std::queue<Packet *> queuePckReadTCP; /**< Queue that store the DATA packets
											 recevied from the client by the
											 server using a TCP protocol. */
	std::queue<Packet *> queuePckReadUDP; /**< Queue that store the DATA packets
											 recevied from the client by the
											 server using a UDP protocol. */
	uwSendTimerAppl *chkTimerPeriod; /**< Timer that schedule the period between
								successive generation of DATA packets. */

	static uint MAX_READ_LEN; /**< Maximum size (bytes) of a single read of the
								 socket */
};
#endif /* UWAPPLICATION_MODULE_H */
