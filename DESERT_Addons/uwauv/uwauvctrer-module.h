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
* @file uwauvctrer-module.h
* @author Alessia Ortile
* @version 1.0.0
*
* \brief Provides the definition of the class <i>UWAUVCtrEr</i>.
*
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

class UwAUVCtrErModule;

/**
* UwSendTimer class is used to handle the scheduling period of <i>UWAUV</i> packets.
*/
class UwAUVErrorSendTimer : public UwSendTimer {
	public:

	/**
   * Conscructor of UwSendTimer class 
   * @param UwAUVCtrModule *m pointer to an object of type UwAUVCtrModule
   */
	UwAUVErrorSendTimer(UwAUVCtrErModule *m) : UwSendTimer((UwCbrModule*)(m)){
	};
};


/**
* UwAUVCtrModule class is used to manage <i>UWAUVCtr</i> packets and to collect statistics about them.
*/
class UwAUVCtrErModule : public UwCbrModule {
public:

	/**
	* Constructor of UwAUVCtrModule class.
	*/
	UwAUVCtrErModule();

	/**
	* Constructor of UwAUVCtrModule class with position setting.
	*/
	UwAUVCtrErModule(UWSMWPPosition* p);

	/**
	* Destructor of UwAUVCtrModule class.
	*/
	virtual ~UwAUVCtrErModule();

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
	virtual void recv(Packet* p);

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

	/**
	* Returns the distance (in a 2D space) between a starting point s and a destination point d.
	* @param x_s x coordinate of point s
	* @param y_s y coordinate of point s
	* @param x_d x coordinate of point d
	* @param y_d y coordinate of point d
	*
	* @return distance between s and d.
	*/
	float getDistance(float x_s,float y_s, float x_d, float y_d);
	/**
	* Returns the distance (in a 3D space) between a starting point s and a destination point d.
	* @param x_s x coordinate of point s
	* @param y_s y coordinate of point s
	* @param z_s z coordinate of point s
	* @param x_d x coordinate of point d
	* @param y_d y coordinate of point d
	* @param z_d z coordinate of point d
	*
	* @return distance between s and d.
	*/
	float getDistance(float x_s,float y_s, float z_s, float x_d, float y_d, float z_d);  

protected:

	UWSMWPPosition* posit; /**< Controller position.*/
	int last_sn_confirmed; /**< Sequence number of the last command Packete received.*/
	int sn; /**< Sequence number of the last control packet sent.*/
	int drop_old_waypoints; /** < Flag set to 1 to drop waypoints with sequence number 
								lower or equal than last_sn_confirmed.*/
	int period;

	float x_err; /**< X of the last AUV position with an error.*/
	float y_err; /**< Y of the last AUV position with an error.*/

	float x_s; /**< X of the last AUV position with an error that has been solved.*/
	float y_s; /**< Y of the last AUV position with an error that has been solved.*/
	
	Packet* p; /**< Pointer to the packet that will be received*/
	int log_on_file;
	float x_sorg; /**< X of the starting AUV position.*/
	float y_sorg; /**< Y of the starting AUV position.*/
	double speed; /**< speed of the AUV.*/

	static int alarm_mode; /**< status of the error resolution mission
								* 0 no error
								* 2 SUV is mooving towards the error location
								* 3 AUV is delving close to the depth of the error location.*/
	bool active_alarm;
	bool error_released;   /**<
								* 0 no error
								* 1 maybe an error --> wait more info
								* 2 for sure an error -> go there
							*/
	static vector<vector<float>> alarm_queue; /**< list of the active errors*/
	static vector<vector<float>> gray_queue; /**< list of the errors which 
												  the status is not determined yet*/

private:

	enum dev_status {no_error = 0, undetermined = 1, error=2}; /**< type of errors*/
	/**
	 * Return the status of an error based on the probability and the number of pkts rcv
	 * @param m probability of being an error
	 * @param n_pkt number of pkts received regarding the status of this error
	 * @param x x coordinate of the location of the error
	 * @param y y coordinate of the location of the error
	*/
	int checkError(double m, int n_pkt, float x, float y); 
	std::ofstream pos_log;
	std::ofstream err_log;
	std::ofstream t_err_log;
	double sigma; /**< standard deviation */
	double th_ne; /**< if x < th_e NO error */
	double accuracy; /**< level of accuracy to achieve before defining the status of an error*/
	vector<vector<float>> rcv_queue; /**< list of error received*/

};

int UwAUVCtrErModule::alarm_mode = 0;
vector<vector<float>>  UwAUVCtrErModule::alarm_queue = {};
vector<vector<float>>  UwAUVCtrErModule::gray_queue = {};


#endif // UWAUVCtr_MODULE_H
