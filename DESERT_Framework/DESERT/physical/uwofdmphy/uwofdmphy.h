//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
//  names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
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
 * @file   uwofdmphy.h
 * @author Sara Falleni
 * @version 1.0.0
 *
 * \brief Definition of UwOFDMPhy class.
 * Your can find the brief description of this physical layer in the paper, named
 * "Development and Testing of an OFDM Physical Layer for the DESERT Simulator"
 * IEEE/OCEANS, San Diego-Porto, 2021.
**/

#ifndef UWOFDMPHY_H
#define UWOFDMPHY_H

#include "uwphysical.h"
#include "string.h"
#include "uwofdmphy_hdr.h"
#include <math.h>
#include <iostream>
#include <map>
#include <ctime>
#include <phymac-clmsg.h>


#include "msg-display.h"

extern packet_t PT_MMAC_CTS;
extern packet_t PT_MMAC_DATA;
extern packet_t PT_MMAC_RTS; 
extern packet_t PT_MMAC_ACK;

typedef ::std::map<double, double> PdrLut;
typedef PdrLut::iterator PdrLutIt;

enum errordistribution {LOWSNR=0, NOISEERR=1, COLLERR=2, TXPEN=3, FREQCOLL=4, MODERR=5, CTRLCERR=6, CTRLFERR=7, TXPENCTRL = 8};

class UwOFDMPhy : public UnderwaterPhysical
{

public:
	/**
	 * Constructor of UwOFDMPhy class.
	 */
	UwOFDMPhy();

	/**
	 * Destructor of UwOFDMPhy class.
	 */
	virtual ~UwOFDMPhy();

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

	//Initialize parameters inside a node, default all carriers are used
	void init_ofdm_node(int nn, int cf, int scn, int ID);

	//set the size of a node buffer  
	void setBufferSize(int);

	//Returns number of nodes in the simulation for a given node 
	int getNodeNum() const;

	//Returns Packets sent to Mac Layer
	int getSentUpPkts();

	//Set number of nodes in the simulation for a given node
	void setNodeNum(int);

	//Returns total number of SubCarriers available for a given node
	int getSubCarNum() const;

	//Returns total number of packets lost for low SNR
	int getLowSnrPktLost() const;

	//Returns total number of packets lost for noise error
	int getNoiseErrPktLost() const;

	//Returns total number of packets lost for Collision error
	int getCollErrPktLost() const;

	//Returns total number of packets lost for transmission pending
	int getTxPenPktLost() const;

	//Returns number of ctrl packets lost for transmission pending
	int getTxPenCtrlLost() const;

	//Returns total number of packets lost for collision in frequency
	int getFreqCollPktLost() const;

	//Returns total number of CTRL Packets lost for collision error
	int getCtrlCErrPktLost() const;

	//Returns total number of CTRL Packets lost for collision in frequency
	int getCtrlFCollPktLost() const;

	//Returns total number of SubCarriers available for a given node
	int getModErrPktLost() const;

	//Returns total number of packets sent by phy layer
	int getPhyPktSent() const;

	//Set number of nodes in the simulation for a given node
	void setSubCarNum(int);

	//Set node ID
	void setNodeID(int);

	//Shows chosen subcarriers for a given node 
	void showSubCar();

	//sets subcarriers from outside the class 
	void setSubCar(int, int);

	//sets broken subcarriers from outside simulation 
	void setBrokenCar(int, int);
	
	int getNodeID() const;

	//Returns the sum of all transmission times
	double getTransmissionTime() const;

	//True if packets are overlappin in frequency, false otherwise
	bool freqOverlap(Packet*, bool);

	//True if a transmission is happening
	bool txongoing_;

protected:
	/**
	 * recv method. It is called when a packet is received from the channel
	 *
	 * @param Packet* Pointer to the packet that are going to be received
	 *
	 */
	virtual void recv(Packet *);

	/**
	 * Handles the end of a packet reception
	 *
	 * @param Packet* p Pointer to the packet received
	 *
	 */
	virtual void endRx(Packet *p);

	/**
	 * Handles the start of a packet reception
	 *
	 * @param Packet* p Pointer to the packet received
	 *
	 */
	virtual void startRx(Packet *p);

	/**
	 * Handles the end of a transmission, redefined to scale the power
	 * into the carriers
	 *
	 * @param Packet* p Pointer to the packet transmitted
	 *
	 */
	virtual void endTx(Packet *p);

	/**
	 * Returns the packet error rate by using the length of a packet and the
	 * information contained in the packet (position
	 * of the source and the destination). The error rate is computed 
	 * only on the carriers used by the packet.
	 *
	 * @param snr Calculated by nsmiracle with the Urick model (unused).
	 * @param nbits length in bit of the packet.
	 * @param p Packet by witch the module gets information about source and
	 * destination.
	 * @return PER of the packet passed as parameter.
	 */
	virtual double getOFDMPER(double _snr, int _nbits, Packet *);

	/**
	 * Handles the end of a transmission, redefined to scale the power
	 * into the carriers
	 *
	 * @param Packet* p Pointer to the packet transmitted
	 * @return Noise power on used carriers 
	 *
	 */
	virtual double getOFDMNoisePower(Packet* p); 

	/**
	 * Computes the transmission time duration depending 
	 * on the used carriers 
	 *
	 * @param Packet* p Pointer to the packet transmitted
	 * @return Transmission duration 
	 *
	 */
	double getTxDuration(Packet* p);

	/**
	 * Creates an OFDM header for non-OFDM packets 
	 *
	 * @param Packet* p Pointer to the packet transmitted
	 *
	 */
	void createOFDMhdr(Packet* p);

	/**
	 * Interrupts reception if MAC has other priorities 
	 * Theoretically should never be called 
	 */
	void interruptReceptions();

	/**
	 * Handles receiving messages  from the MAC layer
	 */
	int recvSyncClMsg(ClMessage* m); 

	/**
	 * Plots pktqueue elements
	 */
	void plotPktQueue();


private:
	/**
	* Return the distance between source and destination.
	*
	* @param p Packet by witch the module gets information about source and
	*destination.
	**/
	virtual double getDistance(Packet *); 

	/**
	* Return the propagation delay for the packet. The delay is calculated based on
	* the distance between the source and the sink. 
	*
	* @param p Packet by witch the module gets information about source and
	*destination.
	**/
	virtual double getPropagationDelay(Packet *);

	/**
	* returns total delay
	*/
	inline virtual int
	getTotalDelay()
	{
		return total_delay_;
	}

	/**
	* @param i an integer
	* @return i converted to string
	*/
	inline string
	itos(int i)
	{
		return std::to_string(i);
	}

	/**
	* @param d a double
	* @return d converted to string
	*/
	inline string
	dtos(double d)
	{
		return std::to_string(d);
	}

	int buffered_pkt_num; 	// keeps track of number of incoming packet
	int current_rcvs;		// number of packets currently being received
	int nodeNum_;			// indicated the total number of nodes
	int centerFreq_;		// Center frequency of the system
	int subCarrier_;		// indicates maximum number of subcarrier
	int nodeID_;			// corresponding node ID
	double total_delay_; 	// total delay. not used eventually. the RX time is modified to reflect the delay
	int powerScaling; 		// 1 if power scales with percentage of used carriers

	int FRAME_BIT; 			// by default = (9120 info bit +32 CRC bit)

	int bufferSize_; 		//default

	std::vector<Packet> pktqueue_;
	std::vector<Packet *> txqueue_;
	std::vector<double> timesqueue_;
	std::vector<double> brokenCarriers_; // Keeps top and bottom index of broken carriers 

	int sentUpPkts;			// number of packets sent to upper layer 
	int lostPackets[8]={0,0,0,0,0,0,0,0}; //lost packets divided by reason
	double totTransTime;	// total transmission time of all transmitted packets  
	int phySentPkt_;		// number of packets sent into the channel 

	//double NoiseSPD_;
	//double ConsumedEnergy_;
	//double TxPower_ = 130.0;    /// Tx Power in W. Simple adaptations of the
		      /// transmission power might be carried out just
		      /// by re-setting this value. More complex
		      /// strategies (e.g., packet-specific
		      /// transmission power) can be implemented by
		      /// overriding the getTxPower() method.

	int tx_busy_; // 1 if a transmission in happening, 0 otherwise

	MsgDisplayer msgDisp; //object of MsgDisplayer type

};

#endif /* UWOFDMPHY_H  */
