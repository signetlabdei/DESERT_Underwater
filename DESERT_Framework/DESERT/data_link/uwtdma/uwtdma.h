//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwtdma.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 * 
 * \brief Provides the definition of the class <i>UWTDMA</i>.
 * 
 */

#ifndef UWTDMA_H
#define UWTDMA_H

#include <mmac.h>
#include <queue>

#define UW_TDMA_STATUS_MY_SLOT 1 /**< Status slot active, whether TDMA modality is on >**/
#define UW_TDMA_STATUS_NOT_MY_SLOT 2 /**< Status slot not active, whether TDMA modality is on >**/

#define UW_CHANNEL_IDLE 1 // status channel idle
#define UW_CHANNEL_BUSY 2 // status channel busy


using namespace std;

class UwTDMA;

/**
 * UwTDMATimer class is used to handle the scheduling period of <i>UWTDMA</i> slots.
 */
class UwTDMATimer : public TimerHandler {
public:

    /**
     * Constructor of the UwTDMATimer class
     * @param UwTDMA *m a pointer to an object of type Uwpolling_AUV
     */
    UwTDMATimer(UwTDMA *m) : TimerHandler() {
        module = m;
    }
protected:
    virtual void expire(Event *e);
    UwTDMA* module;
};

class UwTDMA: public MMac {
public:
	UwTDMA();

	virtual ~UwTDMA();

	/**
     * Change TDMA status from 
     */
	virtual void change_tdma_status();
 	
 	/**
     * Schedule the beginning of TDMA slots
     */
    virtual void start();

    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     **/
    virtual int command(int argc, const char*const* argv);

	/**
     * Transmit a data packet whether is my slot
     */
    virtual void txData();

    /**
     * State status my slot and and start to txData
     */
    virtual void stateTxData();


protected:

	int slot_status; /**< Flag about the slot status: it can be either active or not */
	int channel_status; /**< Flag about the channel status: it can be either busy or idle */
	int debug_; /**< Flag anabling debug print out */
	double frame_time; /**< Frame duration */
	double guard_time; /**< Guard time between slots */
	double slot_duration; /**< Slot duration */
	UwTDMATimer tdma_timer; /**< Tdma timer handler */
	std::queue<Packet*> buffer; /**< Packet buffer*/

    /**
     * Receives the packet from the upper layer (e.g. IP)
     * @param Packet* pointer to the packet received
     *
     */
	virtual void recvFromUpperLayers(Packet* p);

    /**
     * Pass the packet to the PHY layer
     * @param Packet* Pointer to an object of type Packet that rapresent the Packet to transmit
     */
    virtual void Mac2PhyStartTx(Packet* p);

    /**
     * Method called when the PHY layer finish to transmit the packet.
     * @param Packet* Pointer to an object of type Packet that rapresent the Packet transmitted
     */  
    virtual void Phy2MacEndTx(const Packet* p);

    /**
     * Method called when the Phy Layer start to receive a Packet 
     * @param const Packet* Pointer to an object of type Packet that rapresent the Packet that is in reception
     */
    virtual void Phy2MacStartRx(const Packet* p);

    /**
     * Method called when the Phy Layer finish to receive a Packet 
     * @param const Packet* Pointer to an object of type Packet that rapresent the packet received
     */
	virtual void Phy2MacEndRx(Packet* p);

};

#endif 
