//
// Copyright (c) 2023 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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
* @file uwauvctr-module.h
* @author Alessia Ortile
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWAUV</i>.
*
*/

#ifndef UWAUV_CTR_MODULE_H
#define UWAUV_CTR_MODULE_H
#include <uwcbr-module.h>
#include "uwauv-packet.h"
#include <uwsmwpposition.h>
#include "node-core.h"
#include <queue>
#define UWAUV_DROP_REASON_UNKNOWN_TYPE "UKT" /**< Reason for a drop in a <i>UWAUV</i> module. */
#define UWAUV_DROP_REASON_OUT_OF_SEQUENCE "OOS" /**< Reason for a drop in a <i>UWAUV</i> module. */
#define UWAUV_DROP_REASON_DUPLICATED_PACKET "DPK" /**< Reason for a drop in a <i>UWAUV</i> module. */

class UwAUVCtrModule;

/**
* UwSendTimer class is used to handle the scheduling period of <i>UWAUV</i> packets.
*/
class UwAUVCtrSendTimer : public UwSendTimer {
	public:

	/**
	 * Conscructor of UwSendTimer class 
	 * @param UwAUVCtrModule *m pointer to an object of type UwAUVCtrModule
	*/
	UwAUVCtrSendTimer(UwAUVCtrModule *m) : UwSendTimer((UwCbrModule*)(m)){
	};
};

/**
* UwAUVCtrModule class is used to manage <i>UWAUVCtr</i> packets and to collect statistics about them.
*/
class UwAUVCtrModule : public UwCbrModule {
public:

	/**
	* Constructor of UwAUVCtrModule class.
	*/
	UwAUVCtrModule();

	/**
	* Constructor of UwAUVCtrModule class with position setting.
	*/
	UwAUVCtrModule(UWSMWPPosition* p);

	/**
	* Destructor of UwAUVCtrModule class.
	*/
	virtual ~UwAUVCtrModule();

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
	* Initializes a control data packet passed as argument with the default values.
	* 
	* @param Packet* Pointer to a packet already allocated to fill with the right values.
	*/
	virtual void initPkt(Packet* p) ;

	/**
	* Reset retransmissions
	*/
	inline void reset_retx() {p=NULL; sendTmr_.force_cancel();}

	/**
	* Set the position of the AUVCtr
	*
	* @param Position * p Pointer to the AUVCtr position
	*/
	virtual void setPosition(UWSMWPPosition* p);

	/**
	* Returns the position of the AUVCtr
	*
	* @return the current AUVCtr position
	*/
	inline UWSMWPPosition* getPosition() const { return posit;}
	
	/**
	* Returns the last AUV position monitored
	*
	* @return the last AUV position monitored
	*/
	inline Position getMonitoredAUVPosition() { 
		Position monitored_p_auv;
		monitored_p_auv.setX(x_auv);
		monitored_p_auv.setY(y_auv);
		monitored_p_auv.setZ(z_auv);
		return monitored_p_auv;
	}

	/**
	* Performs the reception of packets from upper and lower layers.
	*
	* @param Packet* Pointer to the packet will be received.
	*/
	virtual void recv(Packet* p);

	/**
	* Creates and transmits a packet.
	*
	* @see UwCbrModule::sendPkt()
	*/
	virtual void transmit();

	/**
	* Start the controller.
	*/
	virtual void start();

	/**
	* Returns the size in byte of a <i>hdr_uwAUV_monitoring</i> packet header.
	*
	* @return The size of a <i>hdr_uwAUV_monitoring</i> packet header.
	*/
	static inline int getAUVMonHeaderSize() { return sizeof(hdr_uwAUV_monitoring); }

	/**
	* Returns the size in byte of a <i>hdr_uwAUV_ctr</i> packet header.
	*
	* @return The size of a <i>hdr_uwAUV_monitoring</i> packet header.
	*/
	static inline int getAUVCTRHeaderSize() { return sizeof(hdr_uwAUV_ctr); }



protected:

	UWSMWPPosition* posit; /**< Controller position.*/
	float x_auv; /**< X of the last AUV position monitored.*/
	float y_auv; /**< Y of the last AUV position monitored.*/
	float z_auv; /**< Z of the last AUV position monitored.*/
	float newX; /**< X of the new position sent to the AUV.*/
	float newY; /**< Y of the new position sent to the AUV.*/
	float newZ; /**< Z of the new position sent to the AUV.*/
	float speed; /**< Moving speed sent to the AUV.*/
	int sn; /**< Sequence number of the last control packet sent.*/
	Packet* p;
	int adaptiveRTO; /**< 1 if an adaptive RTO is used, 0 if a
						constant RTO is used.*/
	double adaptiveRTO_parameter; /**< Parameter for the adaptive RTO.*/		

};

#endif // UWAUVCtr_MODULE_H
