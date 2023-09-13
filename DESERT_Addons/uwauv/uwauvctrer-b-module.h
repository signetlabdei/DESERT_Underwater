//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
* @file uwauvctrer-b-module.h
* @author Alessia Ortile
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWAUV</i>.
*
* Provides the definition of the class <i>UWAUVCTR</i>, based on <i>UwCbr</i>.
* <i>UWAUVCTR</i> can manage no more than 2^16 packets. If a module generates more
* than 2^16 packets, they will be dropped, according with <i>UwCbr</i>.
* <i>UWAUVCTR</i> sends control packets containing the next waypoint that has to be
* reach by a AUV. In addition it receives monitoring packets containing the current
* AUV position and acks of the sent packets. Whether the ack is not received, the
* control packet is resent, according to the priority. In particular, last waypoint
* transmitted has the highest priority, whereas the others are forgotten.:
*/

#ifndef UWAUVError_MODULE_H
#define UWAUVError_MODULE_H
#include <uwcbr-module.h>
#include "uwauv-packet.h"
#include "uwsmwpposition.h"
#include "node-core.h"
#include <queue>
#include <fstream>
#define UWAUV_DROP_REASON_UNKNOWN_TYPE "UKT" /**< Reason for a drop in a <i>UWAUV</i> module. */
#define UWAUV_DROP_REASON_OUT_OF_SEQUENCE "OOS" /**< Reason for a drop in a <i>UWAUV</i> module. */
#define UWAUV_DROP_REASON_DUPLICATED_PACKET "DPK" /**< Reason for a drop in a <i>UWAUV</i> module. */
#define HDR_UWAUV_MONITORING(p) (hdr_uwAUV_monitoring::access(p))
#define HDR_UWAUV_CTR(p) (hdr_uwAUV_ctr::access(p))
#define HDR_UWAUV_ERROR(p) (hdr_uwAUV_error::access(p))
using namespace std;
class UwAUVCtrErBModule;

/**
* UwSendTimer class is used to handle the scheduling period of <i>UWAUV</i> packets.
*/
class UwAUVErrorSendTimer : public UwSendTimer {
	public:

	/**
   * Conscructor of UwSendTimer class 
   * @param UwAUVCtrModule *m pointer to an object of type UwAUVCtrModule
   */
	UwAUVErrorSendTimer(UwAUVCtrErBModule *m) : UwSendTimer((UwCbrModule*)(m)){
	};
};


/**
* UwAUVCtrModule class is used to manage <i>UWAUVCtr</i> packets and to collect statistics about them.
*/
class UwAUVCtrErBModule : public UwCbrModule {
public:

	/**
	* Constructor of UwAUVCtrModule class.
	*/
	UwAUVCtrErBModule();

	/**
	* Constructor of UwAUVCtrModule class with position setting.
	*/
	UwAUVCtrErBModule(UWSMWPPosition* p);

	/**
	* Destructor of UwAUVCtrModule class.
	*/
	virtual ~UwAUVCtrErBModule();

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
	* Reset retransmissions
	*/
	inline void reset_retx() {p=NULL; sendTmr_.force_cancel();}


	/**
	* Creates and transmits a packet.
	*
	* @see UwCbrModule::sendPkt()
	*/
	virtual void transmit();
	

	/**
	* Performs the reception of packets from upper and lower layers.
	*
	* @param Packet* Pointer to the packet will be received.
	*/
	virtual void recv(Packet*);

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

	/**
	* Returns the size in byte of a <i>hdr_uwAUV_error</i> packet header.
	*
	* @return The size of a <i>hdr_uwAUV_monitoring</i> packet header.
	*/
	static inline int getAUVErrorHeaderSize() { return sizeof(hdr_uwAUV_error); }

	float getDistance(float x_s,float y_s, float x_d, float y_d); 



protected:

	UWSMWPPosition* posit; /**< Controller position.*/
	int last_sn_confirmed; /**< Sequence number of the last command Packete received.*/
	int sn; /**Sequence number of the last control packet sent.*/
	int drop_old_waypoints;
	int period;

	float x_err; /**< X of the last AUV position with an error.*/
	float y_err; /**< Y of the last AUV position with an error.*/

	float x_s; /**< X of the last AUV position with an error that has been solved.*/
	float y_s; /**< Y of the last AUV position with an error that has been solved.*/
	
	Packet* p;
	int log_flag;
	float x_sorg;
	float y_sorg;
	double speed;

	static int alarm_mode;
	static vector<vector<float>> alarm_queue;

private:
 
	std::ofstream pos_log;
	std::ofstream err_log;
	std::ofstream t_err_log;

};

int UwAUVCtrErBModule::alarm_mode = 0;
vector<vector<float>>  UwAUVCtrErBModule::alarm_queue = {};

#endif // UWAUVCtr_MODULE_H
