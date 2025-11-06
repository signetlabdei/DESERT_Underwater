//
// Copyright (c) 2024 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwjammer.h
 * @author Riccardo Casagrande
 * @version 1.0.0
 *
 * \brief Provides the description of Uwjammer Class.
 *
 */

#ifndef UWJAMMER_H
#define UWJAMMER_H

#include "mmac.h"
#include <map>
#include <string>

#define UWJAMMER_DROP_REASON_BUFFER_FULL "DBF" /**< Buffer is full. */
#define UWJAMMER_DROP_REASON_JAMMER_PROTOCOL "DJP" /**< Protocol rules. */

/**
 * Class that describes a Uwjammer module
 */
class Uwjammer : public MMac
{
public:
	/**
	 * Constructor of the class
	 */
	Uwjammer();

	/**
	 * Destructor of the class
	 */
	virtual ~Uwjammer() = default;

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters (Note that
	 *<i>argv[0]</i> is the name of the object).
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 *successfully or not.
	 *
	 **/
	virtual int command(int argc, const char *const *argv) override;

	/**
	 * Cross-Layer messages interpreter
	 *
	 * @param ClMessage* an instance of ClMessage that represent the message
	 * received
	 * @return <i>0</i> if successful.
	 */
	virtual int crLayCommand(ClMessage *m) override;

protected:
	/**
	 * Enum that rapresents the status of the protocol machine state.
	 */
	enum class JammerStatus { IDLE = 1, BUSY };

	/**
	 * Receives the packet from the upper layer (e.g. IP)
	 * @param Packet* pointer to the packet received
	 *
	 */
	virtual void recvFromUpperLayers(Packet *p) override;

	/**
	 * Transmits the jam packet (calling Mac2PhyStartTx) and increment the
	 * counter of transmitted jam packets.
	 */
	virtual void txJam();

	/**
	 * Pass the packet to the PHY layer
	 * @param Packet* Pointer to an object of type Packet that represent the
	 * Packet to transmit
	 */
	virtual void Mac2PhyStartTx(Packet *p);

	/**
	 * Method called when the PHY layer finish to transmit the packet.
	 * @param Packet* Pointer to an object of type Packet that represent the
	 * Packet transmitted
	 */
	virtual void Phy2MacEndTx(const Packet *p) override;

	/**
	 * IDLE state, check if there is at least one more packet to transmit.
	 */
	virtual void stateIdle();

	/**
	 * Method called when the Phy Layer finish to receive a Packet.
	 * @param Packet* Pointer to an object of type Packet that represent the
	 * Packet received
	 */
	virtual void Phy2MacEndRx(Packet *p) override;

	/**
	 * Returns the number of packets sent during the simulation.
	 * @return int n_jam_sent the number of packets sent
	 */
	inline int
	getJamSent() const
	{
		return n_jam_sent;
	}

	/**
	 * Returns the number of packets discarded during the simulation because the
	 * buffer is full.
	 * @return int n_jam_discarded the number of packets discarded
	 */
	inline int
	getJamDiscarded() const
	{
		return n_jam_discarded;
	}

	/**
	 * Returns the number of packets received and discarded during the
	 * simulation.
	 * @return int n_data_discarded the number of packets sent
	 */
	inline int
	getDataDiscarded() const
	{
		return n_data_discarded;
	}

	/**
	 * Refresh the state of the protocol.
	 * @param JammerStatus current state of the protcol
	 */
	inline virtual void
	refreshState(JammerStatus state)
	{
		curr_state = state;
	}

	int buffer_data_pkts; /**< Size of the buffer in number of packets. */
	uint node_id; /**< Unique Node ID. */
	uint JAMMER_uid; /**< JAMMER Unique ID. */
	size_t n_jam_sent; /**< Number of packets sent. */
	size_t n_jam_discarded; /**< Number of packets discarded because the buffer
							   is full. */
	size_t n_data_discarded; /**< Number of packets received and discarded. */

	Packet *curr_data_pkt; /**< Pointer to the current DATA packet. */
	std::queue<Packet *> Q_data; /**< Queue of DATA in number of packets. */

	JammerStatus curr_state; /**< Current state of the protocol. */
	static const std::map<JammerStatus, std::string>
			status_info; /**< Textual info of the state. */

	constexpr static const int MAX_BUFFER_SIZE =
			100; /**< Maximum size of the queue in number of packets. */
};

#endif /* UWJAMMER_H */
