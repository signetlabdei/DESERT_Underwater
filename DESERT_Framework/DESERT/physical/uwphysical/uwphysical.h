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
 * @file   uwphysical.h
 * @author Giovanni Toso and Federico Favaro
 * @version 1.0.0
 *
 * \brief Definition of UwPhysical class.
 *
 */

#ifndef UWPHYSICAL_H
#define UWPHYSICAL_H

#include "clmsg-stats.h"
#include "underwater-bpsk.h"
#include "uwinterference.h"

#include <cmath>
#include <limits>

class UwPhysicalStats : public Stats
{
public:
	/**
	 * Constructor of UwPhysicalStats class
	 */
	UwPhysicalStats();

	/**
	 *	Constructor of UwPhysicalStats class
	 */
	UwPhysicalStats(int mod_id, int stck_id);

	/**
	 * Destructor of UwPhysicalStats class
	 */
	virtual ~UwPhysicalStats() = default;

	/**
	 * Virtual method used by the Module class in order to copy its stats an a
	 * generic fashion, without the need to know the derived stats
	 * implementation.
	 * @return the copy of a module the stats.
	 **/
	virtual Stats *clone() const;

	/**
	 * Method to update stats with the param of last received packet
	 * @param rx_pwr, received power of the packet
	 * @param noise_pwr, noise power
	 * @param interf_pwr, noise power
	 * @param sinr, signal to noise ratio
	 * @param ber, bit error rate
	 * @param per, packet error rate
	 * @param error, true if packet has error
	 */
	virtual void updateStats(int mod_id, int stck_id, double rx_pwr,
			double noise_pwr, double interf_pwr, double sinr, double ber,
			double per, bool error);

	bool has_error; /** True if last packet wasn't correctly received. */
	double last_rx_power; /** Power of the last received packet.*/
	double last_noise_power; /** Noise power of the last received packet. */
	double instant_noise_power; /** Noise power measured on query. */
	double last_interf_power; /** Interference power of the last rx packet.*/
	double last_sinr; /** SINR of the last received packet. */
	double last_ber; /** BER of the last received packet. */
	double last_per; /** PER of the last received packet. */
};

class UnderwaterPhysical : public UnderwaterMPhyBpsk
{

public:
	/**
	 * Constructor of UnderwaterPhysical class.
	 */
	UnderwaterPhysical();

	/**
	 * Destructor of UnderwaterPhysical class.
	 */
	virtual ~UnderwaterPhysical();

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
	virtual int command(int, const char *const *) override;
	/**
	 * recv method. It is called when a packet is received from the channel
	 *
	 * @param Packet* Pointer to the packet that are going to be received
	 *
	 */
	virtual void recv(Packet *) override;

	/**
	 * recv syncronous cross layer messages to require an operation from another
	 * module
	 *
	 * @param m Pointer cross layer message
	 *
	 */
	int recvSyncClMsg(ClMessage *m) override;

protected:
	/**
	 * Update the stats before sending them through crosslayer message
	 */
	virtual void updateInstantaneousStats();

	/**
	 * Handles the end of a packet transmission
	 *
	 * @param Packet* p Pointer to the packet transmitted
	 *
	 */
	virtual void endTx(Packet *p) override;

	/**
	 * Handles the end of a packet reception
	 *
	 * @param Packet* p Pointer to the packet received
	 *
	 */
	virtual void endRx(Packet *p) override;

	/**
	 * Handles the start of a reception. This method is called from the recv
	 * method
	 *
	 * @param Packet* p Pointer to the packet that the PHY are receiving.
	 *
	 */
	virtual void startRx(Packet *p) override;

	/**
	 * Compute the energy (in Joule) spent by the modem in transmission
	 *
	 * @param const double& time spent by the modem in transmission phase
	 * @return double& the value of energy in Joule
	 */
	virtual double
	consumedEnergyTx(const double &_duration) const
	{
		return (tx_power_ * _duration);
	}

	/**
	 * Compute the energy (in Joule) spent by the modem in reception
	 *
	 * @param const double& time spent by the modem in reception phase
	 * @return double& the value of energy in Joule
	 *
	 */
	virtual double
	consumedEnergyRx(const double &_duration) const
	{
		return (rx_power_ * _duration);
	}

	/**
	 * Returns the packet error rate by using the length of a packet and the
	 * information contained in the packet (position
	 * of the source and the destiantion.
	 *
	 * @param snr Calculated by nsmiracle with the Urick model (unused).
	 * @param nbits length in bit of the packet.
	 * @param p Packet by witch the module gets information about source and
	 * destination.
	 * @return PER of the packet passed as parameter.
	 */
	virtual double getPER(double snr, int nbits, Packet *_p);

	/**
	 * Evaluates is the number passed as input is equal to zero. When C++ works
	 * with
	 * double and float number you can't compare them with 0. If the absolute
	 * value of the number is smaller than eplison that means that the number is
	 * equal to zero.
	 *
	 * @param double& Number to evaluate.
	 * @return <i>true</i> if the number passed in input is equal to zero,
	 * <i>false</i> otherwise.
	 * @see std::numeric_limits<double>::epsilon()
	 */
	bool
	isZero(const double &value) const
	{
		return std::fabs(value) < std::numeric_limits<double>::epsilon();
	}

	/**
	 * Returns the time (in seconds) spent by the node in transmission.
	 *
	 * @return time (in seconds) spent by the node in transmission.
	 */
	double
	Get_Tx_Time() const
	{
		return Tx_Time_;
	}

	/**
	 * Returns the time (in seconds) spent by the node in reception.
	 *
	 * @return time (in seconds) spent by the node in reception.
	 */
	double
	Get_Rx_Time() const
	{
		return Rx_Time_;
	}

	/**
	 * Returns the energy (in Joule) spent by the node in transmission.
	 *
	 * @return energy (in Joule) spent by the node in transmission.
	 */
	double
	Get_Energy_Tx() const
	{
		return Energy_Tx_;
	}

	/**
	 * Returns the energy (in Joule) spent by the node in reception.
	 *
	 * @return energy (in Joule) spent by the node in reception.
	 */
	double
	Get_Energy_Rx() const
	{
		return Energy_Rx_;
	}

	/**
	 * Returns the number of bytes transmitted.
	 *
	 * @return the number of bytes transmitted.
	 */
	double
	Get_Transmitted_bytes() const
	{
		return Transmitted_bytes_;
	}

	/**
	 * Increment the number of packets discarded
	 */
	void
	incrTot_pkts_lost()
	{
		tot_pkts_lost++;
	}

	/**
	 * Increment the number of CTRL packets discarded
	 */
	void
	incrTotCrtl_pkts_lost()
	{
		tot_ctrl_pkts_lost++;
	}

	/**
	 * Increment the number of CTRL packets discarded due to interference
	 */
	void
	incrErrorCtrlPktsInterf()
	{
		errorCtrlPktsInterf++;
	}

	/**
	 * Increment the number of collisions DATA/CTRL
	 */
	void
	incrCollisionDATAvsCTRL()
	{
		collisionDataCTRL++;
	}

	/**
	 * Increment the number of CTRL pkts discarded due to a collision
	 */
	void
	incrCollisionCTRL()
	{
		collisionCTRL++;
	}

	/**
	 * Increment the number of DATA pkts discarded due to a collision
	 */
	void
	incrCollisionDATA()
	{
		collisionDATA++;
	}

	/**
	 * @return the total number of packets lost
	 */
	int
	getTot_pkts_lost() const
	{
		return tot_pkts_lost;
	}

	/**
	 *
	 * @return the total number of CTRL packets lost
	 */
	int
	getTot_CtrlPkts_lost() const
	{
		return tot_ctrl_pkts_lost;
	}

	/**
	 *
	 * @return the total number of CTRL pkts lost due to interference
	 */
	int
	getError_CtrlPktsInterf() const
	{
		return errorCtrlPktsInterf;
	}

	/**
	 *
	 * @return the total number of collision between DATA and CTRL
	 */
	int
	getCollisionsDATAvsCTRL() const
	{
		return collisionDataCTRL;
	}

	/**
	 *
	 * @return the number of collisions with a CTRL packet
	 */
	int
	getCollisionsCTRL() const
	{
		return collisionCTRL;
	}

	/**
	 *
	 * @return the number of collisions with a DATA packet
	 */
	int
	getCollisionsDATA() const
	{
		return collisionDATA;
	}

	/**
	 *
	 * @param _input Input value.
	 * @return the log base 2 of the input.
	 */
	double
	log2(const double &_input) const
	{
		return (log10(_input) / log10(2));
	}

	/**
	 *
	 * @param _snr Signal to Noise Ratio
	 * @param _M Number of simbol of the the MPSK
	 * @return Probability error for the symbol
	 */
	double
	get_prob_error_symbol_mpsk(const double &_snr, const uint32_t _M) const
	{
		return erfc(sqrt(_snr * log2(_M)) * sin(M_PI / _M));
	}

	std::string modulation_name_; /**< Modulation scheme name. */
	double time_ready_to_end_rx_; /**< Used to keep track of the rx time.*/
	double Tx_Time_; /**< Time (in seconds) spent by the node in transmission.
					  */
	double Rx_Time_; /**< Time (in seconds) spent by the node in reception. */
	double Energy_Tx_; /**< Energy (in Joule) spent by the node in
						  transmission. */
	double Energy_Rx_; /**< Energy (in Joule) spent by the node in
						  transmission. */
	double Transmitted_bytes_; /**< Number of bytes transmitted. */
	double tx_power_; /**< Power required in transmission. */
	double rx_power_; /**< Power required in reception. */
	int tot_pkts_lost; /**< Total number of packets lost */
	int tot_ctrl_pkts_lost; /**< Total number of CTRL pkts lost */
	int errorCtrlPktsInterf; /**< Total number of CTRL pkts lost due to
								interference */
	int collisionDataCTRL; /**< Total number of collisions between DATA pkts and
							  CTRL pkts */
	int collisionCTRL; /**< Total number of CTRL pkts lost due to collision */

	int collisionDATA; /**< Total number of DATA pkts lost due to collision */

	std::string Interference_Model; /**< Interference calcuation mode chosen:
										CHUNK model or MEANPOWER model */

	uwinterference *interference_; /**< Pointer to the interference module. */
};

#endif /* UWPHYSICAL_H  */
