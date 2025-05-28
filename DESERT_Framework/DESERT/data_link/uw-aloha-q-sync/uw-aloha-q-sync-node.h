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

/**
 * @file   uw-aloha-q-sync-node.h
 * @author Aleksa Albijanic
 * @version 1.0.0 
 *
 * \brief Class that represents a node of UWALOHAQ_SYNC
 *
 *
 */

#ifndef UW_ALOHA_Q_SYNC_NODE_H
#define UW_ALOHA_Q_SYNC_NODE_H

#include <mmac.h>
#include <queue>
#include <deque>
#include <iostream>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <sys/time.h>

#include <mphy.h>

#include <uw-aloha-q-sync-hdrs.h>

#include <vector>

class UwAloha_Q_Sync_NODE;

/**
 * UwAlohaQSyncTimer class is used to handle the scheduling period of <i>UwAloha_Q_Sync_NODE</i>
 * slots.
 */

class UwAlohaQSyncTimer : public TimerHandler
{

public:
	/**
	 * Costructor of the class UwAlohaQSyncTimer
	 * @param Pointer of a UwAloha_Q_Sync_NODE object
	 */
	UwAlohaQSyncTimer(UwAloha_Q_Sync_NODE *m)
		: TimerHandler()
	{
		assert(m != NULL);
		module = m;
	}

protected:
	/**
	 * Method call when the timer expire
	 * @param Event*  pointer to an object of type Event
	 */
	virtual void expire(Event *e);
	UwAloha_Q_Sync_NODE *module;
};

/**
 * Class that represents a UwAlohaQSync Node
 */
class UwAloha_Q_Sync_NODE : public MMac
{

	friend class UwAlohaQSyncTimer;

public:
	/**
	 * Constructor of the UwAloha_Q_Sync_NODE class
	 */
	UwAloha_Q_Sync_NODE();

	/**
	 * Destructor of the UwAloha_Q_Sync_NODE class
	 */
	virtual ~UwAloha_Q_Sync_NODE();


  /**
   * Cross-Layer messages synchronous interpreter.
   * 
   * @param ClMessage* an instance of ClMessage that represent the message received
   * @return <i>0</i> if successful.
   */
	virtual int recvSyncClMsg(ClMessage* m);

protected:
	
	/**
     * Confirm ACK is received in desired time slot
	 * @param Packet* pointer to the ACK packet received
	 * @param addr MAC adress of the receiving node 
     **/
	virtual void stateRxAck(Packet *p, int addr); 	
	/**
     * Search Q-table and find slot for transmission in the following frame
     **/
	virtual void findMySlot();	
	/**
     * Update Q-table based on reward value
	  * @param ack_received reward value - 1 if ACK received, 0 if not 
     **/
	virtual void updateQ_table(int ack_received); 
	
	virtual void handleTimerExpiration();
	/**
	 * Transmit a data packet if in my slot
	 */
	virtual void txData();
	/**
	 * Schedule the beginning of each TDMA cycle, each one after \p delay
	 * @param delay to await before starting the protocol
	 */
	virtual void start(double delay);
	/**
	 * Cancel the timer
	 */
	virtual void stop();
	/**
	 * Receive the packet from the upper layer (e.g. IP)
	 * @param Packet* pointer to the packet received
	 *
	 */
	virtual void recvFromUpperLayers(Packet *p);
	/**
	 * Method called when the Phy Layer finish to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacEndRx(Packet *p);
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(Packet *p);
	/**
	 * Method called when the Mac Layer start to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Mac2PhyStartTx(Packet *p);
	/**
	 * Method called when the Mac Layer finish to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Phy2MacEndTx(const Packet *p);
	/**
	 * Method called when the Packet received is determined to be not for me
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void rxPacketNotForMe(Packet *p);
	/**
	 * Method called to add the MAC header size
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void initPkt(Packet *p);
	/**
	 * Calculate the epoch of the event. Used in sea-trial mode
	 * @return the epoch of the system
	 */
	inline unsigned long int
	getEpoch()
	{
		return time(NULL);
	}

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
				(Note that <i>argv[0]</i> is the name of the
	 object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
				successfully or
	 not.
	 */
	virtual int command(int argc, const char *const *argv);
	
	enum UWALOHAQ_STATUS { IDLE, TRANSMITTING, RECEIVING }; /**<Enumeration class of UWALOHAQ status*/
	
	enum SLOT_STATUS {TRANSMIT, RECEIVE}; /**<Enumeration class of UWALOHAQ slot status*/

	UwAlohaQSyncTimer alohaq_sync_timer; /**<UwAlohaQSync timer handler*/
	std::vector <std::vector<double>> Q_table{}; /**<2D Q-table*/
	double start_time; /**<Time to wait before starting the protocol*/
	UWALOHAQ_STATUS 
	transceiver_status;
	
	SLOT_STATUS
	slot_status;
	
	std::ofstream out_file_stats; /**<File stream for the log file*/
	double guard_time; 
	
	int HDR_size; /**<Size of the HDR if any*/
	int packet_sent_curr_frame; /**<counter of packet has been sent in the
								current frame */
	int max_queue_size; /**< Maximum dimension of Queue */
	int drop_old_; /**<flag to set the drop packet policy in case of buffer overflow: 
					if 0 (default) drops the new packet, if 1 the oldest*/
	bool enable;
	std::string name_label_;
	int checkPriority; /**<flag to set to 1 if UWCBR module uses packets with priority,
						set to 0 otherwise. Priority can be used only with UWCBR module */
	
	int debug_; /**<Debug variable: 0 for no info,
				>1 for info*/
				
	int sea_trial_; /**<Written log variable*/
	std::deque<Packet *> buffer; /**<Buffer of the MAC node*/

	int curr_slot; /**<Current slot*/
	int curr_subslot; /**<Current subslot*/
	int my_curr_slot; /**<Node's slot chosen for transmission*/
	int my_curr_subslot; /**<Node's subslot chosen for transmission*/
	double t_prop_max; /**<Maximal propagation delay*/
	double t_dp; /**<Transmission delay*/
	double t_guard; /**<Guard interval*/
	std::vector<int> ack_data; /**<Sink's table to collect successful transmittors*/
	double slot_duration_factor; /**<slot_duration = t_dp * slot_duration_factor */
	int nn; /**<number of nodes */
	std::vector<int> max_arr_slot; 
	std::vector<int> max_arr_subslot;
	int subslot_num; /**<Number of subslots */

};

#endif
