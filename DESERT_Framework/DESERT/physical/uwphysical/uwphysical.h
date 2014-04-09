//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwphysical.h
 * @author Giovanni Toso and Federico Favaro
 * @version 1.0.0
 * 
 * \brief Definition of UwPhysical class.
 * 
 */

#ifndef UWPHYSICAL_H 
#define UWPHYSICAL_H 

#include "underwater-bpsk.h"
#include "uwinterference.h"
#include "mac.h"

#include <phymac-clmsg.h>
#include <string.h>

#include <rng.h>
#include <packet.h>
#include <module.h>
#include <tclcl.h>

#include <iostream>
#include <string.h>
#include <cmath>
#include <limits>
#include <climits>

class UnderwaterPhysical : public UnderwaterMPhyBpsk {

public:
    /**
     * Constructor of UnderwaterMPhyBpskDb class.
     */
    UnderwaterPhysical();
    
    /**
     * Destructor of UnderwaterMPhyBpskDb class.
     */
    virtual ~UnderwaterPhysical() { }
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int, const char*const*);
    /**
     * recv method. It is called when a packet is received from the channel
     * 
     * @param Packet* Pointer to the packet that are going to be received
     * 
     */    
    virtual void recv(Packet*);

protected:
    /**
     * Handles the end of a packet transmission
     * 
     * @param Packet* p Pointer to the packet transmitted
     * 
     */
    virtual void endTx(Packet* p);
    /**
     * Handles the end of a packet reception
     * 
     * @param Packet* p Pointer to the packet received
     * 
     */
    virtual void endRx(Packet* p);
    /**
     * Handles the start of a reception. This method is called from the recv method
     * 
     * @param Packet* p Pointer to the packet that the PHY are receiving. 
     * 
     */ 
    virtual void startRx(Packet* p);
    /**
     * Compute the energy (in Joule) spent by the modem in transmission
     * 
     * @param const double& time spent by the modem in transmission phase
     * @return double& the value of energy in Joule 
     */
    inline virtual double consumedEnergyTx(const double& _duration) { return (tx_power_ * _duration); }
    /**
     * Compute the energy (in Joule) spent by the modem in reception
     * 
     * @param const double& time spent by the modem in reception phase
     * @return double& the value of energy in Joule 
     * 
     */
    inline virtual double consumedEnergyRx(const double& _duration) { return (rx_power_ * _duration); }
    
    /**
     * Returns the packet error rate by using the length of a packet and the information contained in the packet (position
     * of the source and the destiantion.
     * 
     * @param snr Calculated by nsmiracle with the Urick model (unused).
     * @param nbits length in bit of the packet.
     * @param p Packet by witch the module gets information about source and destination.
     * @return PER of the packet passed as parameter.
     */
    virtual double getPER(double snr, int nbits, Packet*);
    
    
    /**
     * Evaluates is the number passed as input is equal to zero. When C++ works with
     * double and float number you can't compare them with 0. If the absolute
     * value of the number is smaller than eplison that means that the number is
     * equal to zero.
     * 
     * @param double& Number to evaluate.
     * @return <i>true</i> if the number passed in input is equal to zero, <i>false</i> otherwise.
     * @see std::numeric_limits<double>::epsilon()
     */
    inline const bool isZero(const double& value) const { return std::fabs(value) < std::numeric_limits<double>::epsilon(); }
    
    /**
     * Returns the time (in seconds) spent by the node in transmission.
     * 
     * @return time (in seconds) spent by the node in transmission.
     */
    inline double Get_Tx_Time() { return Tx_Time_; }
    
    /**
     * Returns the time (in seconds) spent by the node in reception.
     * 
     * @return time (in seconds) spent by the node in reception.
     */
    inline double Get_Rx_Time() { return Rx_Time_; }
    
    /**
     * Returns the energy (in Joule) spent by the node in transmission.
     * 
     * @return energy (in Joule) spent by the node in transmission.
     */
    inline double Get_Energy_Tx() { return Energy_Tx_; }
    
    /**
     * Returns the energy (in Joule) spent by the node in reception.
     * 
     * @return energy (in Joule) spent by the node in reception.
     */
    inline double Get_Energy_Rx() { return Energy_Rx_; }
    /**
     * Increment the number of packets discarded
     */
    inline void incrTot_pkts_lost() { tot_pkts_lost++; }
    /**
     * Increment the number of CTRL packets discarded
     */
    inline void incrTotCrtl_pkts_lost() {tot_ctrl_pkts_lost++;}
    /**
     * Increment the number of CTRL packets discarded due to interference
     */
    inline void incrErrorCtrlPktsInterf() {errorCtrlPktsInterf++;}
    /**
     * Increment the number of collisions DATA/CTRL
     */
    inline void incrCollisionDATAvsCTRL() {collisionDataCTRL++;}
    /**
     * Increment the number of CTRL pkts discarded due to a collision
     */
    inline void incrCollisionCTRL() {collisionCTRL++;}
    /**
     * Increment the number of DATA pkts discarded due to a collision
     */
    inline void incrCollisionDATA() {collisionDATA++;}
    /**
     * 
     * @return the total number of packets lost
     */
    inline int getTot_pkts_lost() {return tot_pkts_lost; }
    /**
     * 
     * @return the total number of CTRL packets lost
     */
    inline int getTot_CtrlPkts_lost() {return tot_ctrl_pkts_lost;}
    /**
     *
     * @return the total number of CTRL pkts lost due to interference
     */
    inline int getError_CtrlPktsInterf() {return errorCtrlPktsInterf;}
    /**
     * 
     * @return the total number of collision between DATA and CTRL
     */
    inline int getCollisionsDATAvsCTRL() {return collisionDataCTRL;}
    /**
     * 
     * @return the number of collisions with a CTRL packet
     */
    inline int getCollisionsCTRL() {return collisionCTRL;}
    /**
     *
     * @return the number of collisions with a DATA packet
     */
    inline int getCollisionsDATA() {return collisionDATA;}
    
    // Variables
    string modulation_name_;                    /**< Modulation scheme name. */
    double time_ready_to_end_rx_;               /**< Used to keep track of the arrival time. */
        
    double Tx_Time_;                            /**< Time (in seconds) spent by the node in transmission. */
    double Rx_Time_;                            /**< Time (in seconds) spent by the node in reception. */

    double Energy_Tx_;                          /**< Energy (in Joule) spent by the node in transmission. */
    double Energy_Rx_;                          /**< Energy (in Joule) spent by the node in transmission. */
    
    double tx_power_;                           /**< Power required in transmission. */
    double rx_power_;                           /**< Power required in reception. */
    
    int tot_pkts_lost;                          /**< Total number of packets lost */
    
    int tot_ctrl_pkts_lost;                     /**< Total number of CTRL pkts lost */
    
    int errorCtrlPktsInterf;                    /**< Total number of CTRL pkts lost due to interference */
    
    int collisionDataCTRL;                      /**< Total number of collisions between DATA pkts and CTRL pkts */
    
    int collisionCTRL;                          /**< Total number of CTRL pkts lost due to collision */
    
    int collisionDATA;                          /**< Total number of DATA pkts lost due to collision */
    
    string Interference_Model;                  /**< Interference calcuation mode chosen: CHUNK model or MEANPOWER model */
    
    uwinterference* interference_;              /**< Pointer to the interference model module */
private:
    //Variables
};

#endif /* UWPHYSICAL_H  */
