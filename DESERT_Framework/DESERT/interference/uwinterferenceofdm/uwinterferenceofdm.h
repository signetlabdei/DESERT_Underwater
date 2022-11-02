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
 * @file   uwinterferenceofdm.h
 * @author Sara Falleni
 * @version 1.0.0
 *
 * @brief Implementation of uwinterference class.
 * Your can find the brief description of this module in the paper, named
 * "Development and Testing of an OFDM Physical Layer for the DESERT Simulator"
 * IEEE/OCEANS, San Diego-Porto, 2021.
 */

#ifndef UW_INTERFERENCEOFDM
#define UW_INTERFERENCEOFDM

#include "uwinterference.h"
#include "uwofdmphy_hdr.h"


typedef std::pair<int, int> counter; /**< counter of collisions */


class uwinterferenceofdm;

class ListNodeOFDM : public ListNode
{
public:
	std::vector<double> carrier_power;


	/**
	 * Constructor of the class ListNode when used with multicarrier
	 * @param t time
	 * @param sum_pw sum of the rx power in the node at the given time
	 * @param ctrl  control packet counter
	 * @param data data packet counter
	 */
	ListNodeOFDM(double t, double sum_pw, int ctrl, int data, const std::vector<double>& carPwr): 
	carrier_power(carPwr)
	{
		time = t;
		sum_power = sum_pw;
		ctrl_cnt = ctrl;
		data_cnt = data;		
	}

	/**
	 * Destructor of the class ListNode
	 */
	virtual ~ListNodeOFDM()
	{
	}
};

class EndInterfTimerOFDM : public Handler
{
public:
	EndInterfTimerOFDM(uwinterferenceofdm *ptr)
	{
		interference = ptr;
	}
	virtual void handle(Event *e);

protected:
	uwinterferenceofdm *interference;
};


class uwinterferenceofdm : public uwinterference
{
public:
	int interfSubCarriers_;

	/**
   * TCL command interpreter. It implements the following OTcl methods:
   *
   * @param argc Number of arguments in <i>argv</i>.
   * @param argv Array of strings which are the command parameters (Note that
   * <i>argv[0]</i> is the name of the object).
   * @return TCL_OK or TCL_ERROR whether the command has been dispatched
   * successfully or not.
   *
   */
	virtual int command(int, const char *const *);

	/**
	 * Constructor of the class uwinterference
	 */
	uwinterferenceofdm();
	/**
	 * Destructor of the class uwinterference
	 */
	virtual ~uwinterferenceofdm();
	/**
	 * Add a packet to the interference calculation
	 * @param p Pointer to the interferer packet
	 */
	virtual void addToInterference(Packet *p);
	/**
	 * Add a packet to the interference calculation
	 * @param pw Received power of the current packet
	 * @param type type of the packet (DATA or CTRL)
	 * @param carriers vector of carriers used in that packet
	 * @param carNum explicit number of carriers
	 */
	virtual void addToInterference(double pw, PKT_TYPE tp, int* carriers, int carNum);
	/**
	 * Remove a packet to the interference calculation
	 * @param pw Received power of the current packet
	 * @param type type of the packet (DATA or CTRL)
	 */
	virtual void removeFromInterference(double pw, PKT_TYPE tp, const std::vector<double>& carPwr);
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
			double power, double starttime, double duration, int* carriers, int ncar);
	/**/
	virtual double getCurrentTotalPower();
	/**
	 * Compute the total power on a carrier
	 * @param carrier Carrier targeted
	 * @return the total power on a carrier
	 */
	virtual double getCurrentTotalPowerOnCarrier(int carrier);
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
	 * @return number of carriers used by the node
	 */
	inline int getInterfCarriers(){
		return interfSubCarriers_;
	}
	/**
	 * Sets number of carriers used by the node
	 * @param sc number of carriers
	 */
	inline void setInterfCarriers(int sc){
		interfSubCarriers_ = sc;		
		return;
	}

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

	std::list<ListNodeOFDM> power_list; /**<List with power and counters*/
	EndInterfTimerOFDM end_timerOFDM; /**< Timer for schedules end of interference for a transmission */
	int inodeID; /* ID of the node */

};

#endif /*UW_INTERFERENCE*/
