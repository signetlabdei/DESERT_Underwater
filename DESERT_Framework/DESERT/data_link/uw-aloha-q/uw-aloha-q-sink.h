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
 * @file   alohaq-sink.h
 * @author Aleksa Albijanic
 * @version 1.0.0
 *
 * @brief Provides the definition of the class <i>UWALOHAQ-SINK</i>.
 *
 */

#ifndef UW_ALOHA_Q_SINK_H
#define UW_ALOHA_Q_SINK_H

#include <mmac.h>
#include <queue>
#include <deque>
#include <iostream>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <sys/time.h>

#include <mphy.h>


extern packet_t PT_MMAC_ACK;

/**
 * Class that represents an ALOHAQ Sink
 */
class UwAloha_Q_SINK : public MMac
{

public:
	/**
	 * Constructor of the TDMA class
	 */
	UwAloha_Q_SINK();

	/**
	 * Destructor of the TDMA class
	 */
	virtual ~UwAloha_Q_SINK();
	
	virtual int recvSyncClMsg(ClMessage* m);

protected:
	enum UWALOHAQ_PKT_TYPE{
	UWALOHAQ_ACK_PKT,
	UWALOHAQ_DATA_PKT
	};
	
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
	
	virtual int getLayerIdFromTag(const std::string& tag);
	
	/**
	 * Method called when the Phy Layer start to receive a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in reception
	 */
	virtual void Phy2MacStartRx(const Packet *p);
	/**
	 * Method called when the Mac Layer start to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */	
	//virtual void Mac2PhyStartTx(Packet *p);
	/**
	 * Method called when the Mac Layer finish to transmit a Packet
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void Phy2MacEndTx(const Packet *p);
	
	/**
	 * Method called to transmit ACK packet
	 */
	virtual void txAck(int dest_addr);
	
	/**
	 * Method called to add the MAC header size
	 * @param const Packet* Pointer to an Packet object that rapresent the
	 * Packet in transmission
	 */
	virtual void initPkt(Packet *p, int dest_adrr);
	
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
	/**
	 * Enumeration class of UWTDMA status.
	 */
	enum UWALOHAQ_SINK_STATUS { IDLE, TRANSMITTING, RECEIVING };

	UWALOHAQ_SINK_STATUS
	sink_status; /**<Variable holding the status enum type*/
	std::ofstream out_file_stats; /**<File stream for the log file*/
	
	int packet_sent_curr_slot_; /**<counter of packet has been sent in the
								current slot */
	int max_queue_size; /**< Maximum dimension of Queue */
	
	
	std::string name_label_; /**<label added in the log file, empty string by default*/
	
	int debug_; /**<Debug variable: 0 for no info,
				>-5 for small info, <-5 for complete info*/
	int sea_trial_; /**<Written log variable*/
	int ACK_size;
	int HDR_size; /**<Size of the HDR if any*/
	std::deque<Packet *> buffer; /**<Buffer of the MAC node*/
	
	int ack_phy_id;
	//bool enable;

	
	
};

#endif
