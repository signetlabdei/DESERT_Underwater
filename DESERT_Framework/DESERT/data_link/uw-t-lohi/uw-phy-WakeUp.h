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
 * @file   uw-phy-WakeUp.h
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the declaration of WakeUp PHY 
 *
 */


#ifndef UW_WAKEUP_H 
#define UW_WAKEUP_H

#include <mphy.h>
#include <map>
#include "wake-up-pkt-hdr.h"

#define UW_WAKEUP_MODNAME "WKUP"
#define DROP_REASON_BELOW_THRESHOLD "DBT"

/**
 * Class that describes the WakeUp PHY layer for T-LOHI MAC protocol
 */
class MPhy_WakeUp : public MPhy 
{

 public:
  /**
   * Class contructor
   */
  MPhy_WakeUp();
  /**
   * Class destructor
   */
  virtual ~MPhy_WakeUp();
  /**
   * Returns the Modulation type
   * @param Packet* pointer to the packet where get the modulation
   * @return type of the modulation
   */
  virtual int getModulationType(Packet*);
	/**
	 * Returns the duration of the transmission for the specified packet
	 * @param Packet* pointer to the packet in which compute the Tx Duration
	 * @return duration of the Tone
	 */
  virtual double getTxDuration(Packet* p) { return (ToneDuration_);}
	/**
	 * Returns the PER for a certain SNR and a dimension of packet
	 * @param double SNR
	 * @param int nbits, dimension of the packet in bit
	 * @return PER
	 */
  virtual double getPER(double snr, int nbits);
	/**
	 * Gets the number of Tx Pending dropped packets
	 * @return Number of Tx Pending dropped packets 
	 */
  virtual int getDroppedPktsTxPending() { return droppedPktsTxPending; }
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * 
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
	 * 
	 **/
  virtual int command(int argc, const char*const* argv);  

 protected:
  /**
   * Starts to transmit a Packet
   * @param Packet* pointer to the packet to transmit
   */
  virtual void startTx(Packet* p);
	/**
	 * Notify to the MAC protocol that the transmission has been finished
	 * @param Packet* pointer to the packet just transmitted
	 */
  virtual void endTx(Packet* p);
	/**
	 * Notify to the MAC protocol the start of a reception
	 * @param Packet* pointer to the packet in reception
	 */
  virtual void startRx(Packet* p);
	/**
	 * Notify to the MAC protocol that the reception is finished and send the packet to it
	 * @param Packet* pointer ot the packet received
	 */
  virtual void endRx(Packet* p);
	/**
	 * Returns the Power of the noise for a packet
	 * @param Packet* pointer to the packet where compute the noise
	 * @return Noise power in dB
	 */
  virtual double getNoisePower(Packet* p);
	/**
	 * Gets the Transmission Spectral Mask for the Packet p
	 * @param Packet* pointer to the packet where get the Spectral Mask
	 * @return the pointer of an Object of MSpectralMask that indicates the type of Spectral Mask adopted
	 */
  virtual MSpectralMask* getTxSpectralMask(Packet* p);
	/**
	 * Drops a packet
	 * @param Packet* pointer to the packet to drop
	 */
  virtual void dropPacket(Packet* p); 
	/**
	 * Increases the number of packet dropped
	 */
  void incrDroppedPktsTxPending() { droppedPktsTxPending++; }


	/**
	 * Used for debug purposes. Permit a step-by-step behaviour of the PHY layer
	 */
  virtual void waitForUser();

  Packet* PktRx;  /**< Pointer to the packeti in reception */

  static bool initialized; /**< used to register the modulation type only once */
  static int modid;        /**< modulation type id */
  bool txActive;

  /* input */
  double AcquisitionThreshold_dB_; /**< How many dB over noise are required for a signal to trigger
									 * acquisition (i.e., a RX attempt) */

  double ToneDuration_; /**< predefined tone duration */

  double MaxTxRange_; /**< Maximum Transmission Range */



  int droppedPktsTxPending; /**< Total number of dropped pkts due to tx pending */



};


#endif /* UW_WAKEUP_H */



