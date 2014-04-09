//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file uwal.h
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief Header of the main class that implements the adaptation layer between ns2/NS-Miracle and binary data packets. 
 */

#ifndef UWAL_H
#define UWAL_H

#include "hdr-uwal.h"
#include "packer.h"
#include "frame-set.h"

#include <mphy.h>
#include <mac.h>

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <map>
#include <queue>
#include <time.h>
#include <rng.h>

typedef struct PERListElement {
    int node_ID;
    double per;
} pl_element;

using namespace std;

class Uwal;

/** 
 *  The class used by Uwal to handle simulator's event expirations; it is exploited to schedule the sendDown() of successive frames.
 */
class TxFrameTimer : public TimerHandler
{
public:
	
        /** 
	 * Class constructor.
	 * 
	 * @param pUwal pointer to the Uwal object to be linked with this TxFrameTimer object.
	 */ 	 
	 TxFrameTimer(Uwal* pUwal) : TimerHandler()
	 {
		  pUwal_ = pUwal;
		  
	 }

protected:
         
         /** 
	 * Method to handle the expiration of a given event.
	 * 
	 * @param e event to be handled.
	 */
	 virtual void expire(Event* e);

	 Uwal* pUwal_; /**< Pointer to an Uwal object. It is used to call Uwal::ALqueueManager() when the countdown expires. @see Uwal::interframe_period*/
};


/**
 * The main class implementing the module used to implement the adaptation layer between ns2/NS-Miracle and binary data packets.
 */
class Uwal : public MPhy
{         
	 /**
          * Friend class used to implement the timer handler.
          *   
          * @see TxFrameTimer
          */
	  friend class TxFrameTimer;

  public: 
        
	  /** 
	  * Class constructor.
	  */ 
	  Uwal();
                
	  /** 
	  * Class destructor.
	  */
	  ~Uwal();
                
	  /** 
	  * Method to handle the reception of packets arriving from the upper layers of the network simulator.
	  * 
	  * @param p pointer to the packet that has been received from the simulator's upper layers.
	  */
	  virtual void recv(Packet*);
		
	  /** 
	  * Method to map tcl commands into c++ methods.
	  * 
	  * @param argc number of arguments in <em> argv </em> 
	  * @param argv array of arguments where <em> argv[3] </em> is the tcl command name and <em> argv[4, 5, ...] </em> are the parameters for the corresponding c++ method.
	  */
	  virtual int command(int, const char*const*);
	
	  size_t getPSDU() {return PSDU;}
		
  protected:
	  
	  int nodeID; /**< Node ID */
	  unsigned int pkt_counter; /**< Counter for the pktID to set in TX. */ 
	  packer* pPacker; /**< Pointer to the packer of the protocol headers */
	  size_t PSDU; /**< size of the PSDU */
	  string dummyStr; /** String containing dummy characters to be used as padding chars if necessary. */
	  int debug_;  /**< Flag to enable debug mode (i.e., printing of debug messages) if set to 1. */	
	  std::queue<Packet*> sendDownPkts; /**< queue of the packet to send down to the modem */
	  std::queue<Packet*> sendDownFrames; /**< queue of the frames to send down */
	  std::queue<Packet*> sendUpFrames; /**< queue of the frames to send up to the upper protocols */
	  std::queue<Packet*> sendUpPkts; /**< queue of the packets to send up to the upper protocols */
          list<PERListElement> PERList; /**< PER list (couple of ID of the node and Packet Error Rate associated ) */
	  std::map<RxFrameSetKey,RxFrameSet> sendUpFrameSet; /**< map of the frames to send up */
	  /**
           * Method responsible to manage the queueing system of Adaptation Layer
           */
	  void ALqueueManager();
	  /**
           * Method responsible to initialize the headers of the packet
           * @param Pointer to the packet
           * @param ID of the packet
           */
	  void initializeHdr(Packet*,unsigned int);
	  /**
           * Method responsible to fragment the packet
           * @param Pointer to the packet that are going to be fragmented
           */
	  void fragmentPkt(Packet*);
	  /**
           * Method responsible to reassemble the various fragments in a unique packets
           * @param pointer to the original Packet
           */
	  void reassembleFrames(Packet*);
          /**
           * Method responsible to check for errors the received frames
           */
	  void checkRxFrameSet();
	  
	  /**
	   *  Method to start the packet transmission. 
	   * 
	   *  @param p pointer to the packet to be transmitted.
	  */
	  virtual void startTx(Packet*);

	  /**
	   *  Method to end a packet transmission. This method is also in charge to send a cross layer message Phy2MacEndTx(p) to notify the above layers of the simulator about the end of a transmission, see http://telecom.dei.unipd.it/ns/miracle/doxygen/classMPhy.html#c8c2f52d3833fed8ac736aca0ee52c98. 
	   * 
	   *  @param p pointer to the last transmitted packet.
	   */
	  virtual void endTx(Packet*);

	  /**
	   *  Method to start a packet reception. This method is also in charge to send a cross layer message Phy2MacStartRx(p) to notify the above layers of the simulator about the start of a reception, see http://telecom.dei.unipd.it/ns/miracle/doxygen/classMPhy.html#a15cc91b98013e1c631ad85072867ab6. 
	   * 
	   *  @param p pointer to the last received packet.
	   */
	  virtual void startRx(Packet*);

	  /**
	   *  Method to end a packet reception. 
	   * 
	   *  @param p pointer to the last received packet.
	   */
	  virtual void endRx(Packet*);
	 
	  /** Unused method at the moment. To be implemented because defined as virtual in MPhy. */
	  virtual double getTxDuration(Packet*) { return -1; }
                
	  /** Unused method at the moment. To be implemented because defined as virtual in MPhy. */
	  virtual int getModulationType(Packet*) { return -1; }
          /**
           * Method to search a MAC address in the PER List
           * @param mac_addr Address to search
           * @return true if mac_addr in in the PERList, false otherwise
           */
          virtual bool isInPERList(int mac_addr);
          /**
           * Method to search for the PER associated with a particular MAC address
           * @param mac_addr Addr of the node for which search the related Packet Error Rate
           * @return the Packet Error Rate associated
           */
          virtual double getPERfromID(int mac_addr);
	  
  private:
        
	  TxFrameTimer InterframeTmr; /**< Object of the class TxFrameTimer */
	  double interframe_period; /**< Time period [s] between two successive frame to be sent down. */
	  double frame_set_validity; /**< Time of validity of a frame set */
	  int frame_padding; /**< Flag to determine if perfoming bit padding up to PSDU size. */
	  
}; /* class Uwal */

#endif /* UWAL_H */
