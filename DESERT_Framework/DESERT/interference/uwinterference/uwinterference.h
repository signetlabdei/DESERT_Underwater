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
 * @file   uwinterference.h
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Implementation of uwinterference class.
 *
 */

#ifndef UW_INTERFERENCE
#define UW_INTERFERENCE

#include <interference_miv.h>
#include <list>
#include <scheduler.h>
#include <assert.h>
#include <cmath>

enum PKT_TYPE { CTRL, DATA }; /**< Pkt type: CONTROL or DATA packet */
// enum RX_STATE { START_RX, END_RX};

typedef std::pair<int, int> counter; /**< counter of collisions */

class ListNode
{
public:
	double time;
	double sum_power; /** sum of the rx power in the node at the given time*/
	int ctrl_cnt; /** control packet counter */
	int data_cnt; /** data packet counter */

	/**
	 * Constructor of the class ListNode
	 */
	ListNode()
	{
		time = 0;
		sum_power = 0;
		// n_pkts = 0;
		ctrl_cnt = 0;
		data_cnt = 0;
	}

	/**
	 * Constructor of the class ListNode
	 * @param t time
	 * @param sum_pw sum of the rx power in the node at the given time
	 * @param ctrl  control packet counter
	 * @param data data packet counter
	 */
	ListNode(double t, double sum_pw, /*double n,*/ int ctrl, int data)
	{
		time = t;
		sum_power = sum_pw;
		ctrl_cnt = ctrl;
		data_cnt = data;
	}

	/**
	 * Destructor of the class ListNode
	 */
	virtual ~ListNode()
	{
	}
};

class EndInterfEvent : public Event
{
public:
	/**
	 * Constructor of the class EndInterfEvent
	 * @param pw Received power of the current packet
	 * @param type type of the packet (DATA or CTRL)
	 */
	EndInterfEvent(double pw, PKT_TYPE tp)
	{
		power = pw;
		type = tp;
	}

	/**
	 * Destructor of the class EndInterfEvent
	 */
	virtual ~EndInterfEvent()
	{
	}
	double power;
	PKT_TYPE type;
};

class uwinterference;

class EndInterfTimer : public Handler
{
public:
	EndInterfTimer(uwinterference *ptr)
	{
		interference = ptr;
	}
	virtual void handle(Event *e);

protected:
	uwinterference *interference;
};

class uwinterference : public MInterferenceMIV
{
public:
	/**
	 * Constructor of the class uwinterference
	 */
	uwinterference();
	/**
	 * Destructor of the class uwinterference
	 */
	virtual ~uwinterference();
	/**
	 * Add a packet to the interference calculation
	 * @param p Pointer to the interferer packet
	 */
	virtual void addToInterference(Packet *p);
	/**
	 * Add a packet to the interference calculation
	 * @param pw Received power of the current packet
	 * @param type type of the packet (DATA or CTRL)
	 */
	virtual void addToInterference(double pw, PKT_TYPE tp);
	/**
	 * Remove a packet to the interference calculation
	 * @param pw Received power of the current packet
	 * @param type type of the packet (DATA or CTRL)
	 */
	virtual void removeFromInterference(double pw, PKT_TYPE tp);
	/**
	 * Compute the average interference power for the given packet
	 * @param p Pointer to the interferer packet
	 * @return average interference power
	 */
	virtual double getInterferencePower(Packet *p);
	/**
	 * Compute the average interference power for the given packet
	 * @param pw Received power of the current packet
	 * @param starttime timestamp of the start of reception phase
	 * @param duration duration of the reception phase
	 * @return average interference power
	 */
	virtual double getInterferencePower(
			double power, double starttime, double duration);
	virtual double getCurrentTotalPower();
	/**
	 * Returns the percentage of overlap between current packet and interference
	 * packets
	 * @param p Pointer of the packet for which the overlap is needed
	 * @return percantage of overlap
	 */
	virtual double getTimeOverlap(Packet *p);
	/**
	 * Returns the percentage of overlap between current packet and interference
	 * packets
	 * @param starttime timestamp of the start of reception phase
	 * @param duration duration of the reception phase
	 * @return percantage of overlap
	 */
	virtual double getTimeOverlap(double starttime, double duration);
	/**
	 * Returns the counters of collisions
	 * @param p Pointer of the packet for which the counters are needed
	 * @return counter variable that represent the counters of the interference
	 */
	virtual counter getCounters(Packet *p);
	/**
	 * Returns the counters of collisions
	 * @param starttime timestamp of the start of reception phase
	 * @param duration duration of the reception phase
	 * @param type type of the packet (DATA or CTRL)
	 * @return counter variable that represent the counters of the interference
	 */
	virtual counter getCounters(double starttime, double duration, PKT_TYPE tp);
	/**
	 * Get the timestamp of the start of reception phase
	 * @return timestamp of the start of reception phase
	 */
	inline double
	getStartRxTime()
	{
		return start_rx_time;
	}
	/**
	 * Get the timestamp of the end of reception phase
	 * @return timestamp of the start of reception phase
	 */
	inline double
	getEndRxTime()
	{
		return end_rx_time;
	}
	/**
	 * Get the timestamp of the begin of reception of the first interferer
	 * packet
	 * @return timestamp of the begin of reception of the first interferer
	 * packet
	 */
	inline double
	getInitialInterferenceTime()
	{
		return initial_interference_time;
	}

protected:
	std::list<ListNode> power_list; /**<List with power and counters*/
	EndInterfTimer end_timer; /**< Timer for schedules end of interference
									 for a transmission */
	double use_maxinterval_; /**< set to 1 to use maxinterval_. */

	double initial_interference_time; /**< timestamp of the begin of reception
										 of the first interferer packet */
	double start_rx_time; /**< timestamp of the start of reception phase */
	double end_rx_time; /**< timetamp of the end of reception phase */
};

#endif /*UW_INTERFERENCE*/