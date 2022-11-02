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
 * @file   uwofdmphy.cpp
 * @author Sara Falleni
 * @version 1.1.0
 *
 * \brief Implementation of UwOFDMPhy class
 *
 */

#include "uwofdmphy.h"
#include <fstream>
#include <sstream>

#include "uwphy-clmsg.h"

static class UwOFDMPhyClass : public TclClass
{
public:
	UwOFDMPhyClass()
		: TclClass("Module/UW/OFDM/PHY")
	{
	}
	TclObject *
	create(int, const char *const *)
	{
		return (new UwOFDMPhy);
	}
} class_module_uwofdmphy;

UwOFDMPhy::UwOFDMPhy()
	: UnderwaterPhysical(), sentUpPkts(0), totTransTime(0), phySentPkt_(0), 
	bufferSize_(50), buffered_pkt_num(0), current_rcvs(0), nodeNum_(-1), centerFreq_(0), 
	subCarrier_(-1), nodeID_(-1), tx_busy_(0), powerScaling(1)

{ // binding to TCL variables
	bind("FRAME_BIT", &FRAME_BIT);
	bind("powerScaling_", (int *)&powerScaling);
	Interference_Model = "MEANPOWER";
}

UwOFDMPhy::~UwOFDMPhy()
{
}

int UwOFDMPhy::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2)
	{
		if (strcasecmp(argv[1], "getTotalDelay") == 0)
		{
			// return Get_Rx_Time();
			tcl.resultf("%f", getTotalDelay());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getNodeNum") == 0)
		{
			tcl.resultf("%d", getNodeNum());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getSubCarNum") == 0)
		{
			tcl.resultf("%d", getSubCarNum());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "showSubCar") == 0)
		{
			showSubCar();
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getNodeID") == 0)
		{
			tcl.resultf("%d", getNodeID());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getSentUpPkts") == 0)
		{
			tcl.resultf("%d", getSentUpPkts());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getLowSnrPktLost") == 0)
		{
			tcl.resultf("%d", getLowSnrPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getNoiseErrPktLost") == 0)
		{
			tcl.resultf("%d", getNoiseErrPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getCollErrPktLost") == 0)
		{
			tcl.resultf("%d", getCollErrPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getTxPenPktLost") == 0)
		{
			tcl.resultf("%d", getTxPenPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getTxPenCtrlLost") == 0)
		{
			tcl.resultf("%d", getTxPenCtrlLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getFreqCollPktLost") == 0)
		{
			tcl.resultf("%d", getFreqCollPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getModErrPktLost") == 0)
		{
			tcl.resultf("%d", getModErrPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getTransmissionTime") == 0)
		{
			tcl.resultf("%f", getTransmissionTime());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getCtrlFCollPktLost") == 0)
		{
			tcl.resultf("%d", getCtrlFCollPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getCtrlCerrPktLost") == 0)
		{
			tcl.resultf("%d", getCtrlCErrPktLost());
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "getPhyPktSent") == 0)
		{
			tcl.resultf("%d", getPhyPktSent());
			return TCL_OK;
		}
	}
	else if (argc == 3)
	{
		if (strcasecmp(argv[1], "setNodeNum") == 0)
		{
			setNodeNum(atoi(argv[2]));
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "setSubCarNum") == 0)
		{
			setSubCarNum(atoi(argv[2]));
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "setNodeID") == 0)
		{
			setNodeID(atoi(argv[2]));
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "setBufferSize") == 0)
		{
			setBufferSize(atoi(argv[2]));
			return TCL_OK;
		}
	}
	else if (argc == 4)
	{
		if (strcasecmp(argv[1], "setBrokenCar") == 0)
		{
			setBrokenCar(atoi(argv[2]), atoi(argv[3]));
			return TCL_OK;
		}
	}
	else if (argc == 6)
	{
		if (strcasecmp(argv[1], "init_ofdm_node") == 0)
		{
			init_ofdm_node(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
			return TCL_OK;
		}
	}

	return UnderwaterPhysical::command(argc, argv);
}

// This function initializes set the buffer size for a node
void UwOFDMPhy::setBufferSize(int s)
{
	bufferSize_ = s;
}

// This function initializes all the parameter inside the node
// All subcarriers are used
void UwOFDMPhy::init_ofdm_node(int nn, int cf, int scn, int ID)
{
	nodeNum_ = nn;
	centerFreq_ = cf;
	subCarrier_ = scn;
	nodeID_ = ID;
	msgDisp.initDisplayer(nodeID_, "UwOFDMPhy", debug_);
	std::cout << NOW << " UwOFDMPhy(" << nodeID_ << ")::init_ofdm_node  Node created" << std::endl;
	msgDisp.printStatus("Node Created ", "init_ofdm_node", NOW, nodeID_);
	return;
}

/*
THe recv method takes care any packet comes to phy layer.
direction for both up and down is permitted.
The assignment of subcarrier is done here just before the packet is
transmitted. ( direction down)
TODO: save only non conflitting packets
*/

void UwOFDMPhy::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);

	double temptxtime;

	if (ch->direction() == hdr_cmn::UP)
	{

		ph->dstSpectralMask = getRxSpectralMask(p);
		ph->dstPosition = getPosition();
		ph->dstAntenna = getRxAntenna(p);

		assert(ph->dstSpectralMask);
		assert(ph->dstPosition);

		ph->Pr = getRxPower(p);

		p->txinfo_.RxPr = 0;
		p->txinfo_.CPThresh = 0;

		if (buffered_pkt_num >= bufferSize_) {
			cerr << "UwOFDMPhy ERROR: buffer overflow" << endl;
			dropPacket(p);

		} else if (ph->Pr > 0) {
			if (ofdmph->nativeOFDM)
				ph->Pn = getOFDMNoisePower(p);
			else
				ph->Pn = getNoisePower(p);
			msgDisp.printStatus("received a native? " + itos(ofdmph->nativeOFDM) + " pkt", "recv", NOW, nodeID_);

			if (!ofdmph->nativeOFDM)
				createOFDMhdr(p);
			msgDisp.printStatus("Adding to Interference", "recv", NOW, nodeID_);

			if (interference_) {
				interference_->addToInterference(p);
			}
			msgDisp.printStatus("START recv new pkt_type " + itos(ch->ptype()) +
									" duration is " + dtos(ph->duration),
									"recv", NOW, nodeID_);

			ph->rxtime = NOW;
			ph->worth_tracing = true;

			if (isOn == true) {
				PacketEvent *pe = new PacketEvent(p);
				Scheduler::instance().schedule(&rxtimer, pe, ph->duration);
				buffered_pkt_num++;

				startRx(p);
			} else {
				Packet::free(p);
			}
		} else {
			Packet::free(p);
		}
	}
	else
	{ // Direction DOWN
		assert(isOn);
		hdr_OFDM *ofdmph = HDR_OFDM(p);

		if (current_rcvs > 0)
		{
			// IDEALLY this should never happen, MAC layer must handle concurrency not phy layer

			std::cerr << NOW << " UwOFDMPhy(" << nodeID_ 
					<< ")::recv ATTENTION: KILLING RECEIVING PACKETS TO TRANSMIT " 
					<< current_rcvs << std::endl;
			interruptReceptions();
		}

		ph->Pr = 0;
		ph->Pn = 0;
		ph->Pi = 0;
		ph->txtime = NOW;
		ph->rxtime = ph->txtime;
		ofdmph->srcID = nodeID_;

		ph->worth_tracing = false;

		ph->srcSpectralMask = getTxSpectralMask(p);
		ph->srcAntenna = getTxAntenna(p);
		ph->srcPosition = getPosition();
		ph->dstSpectralMask = 0;
		ph->dstPosition = 0;
		ph->dstAntenna = 0;
		ph->modulationType = getModulationType(p);
		ph->duration = getTxDuration(p);

		if (powerScaling)
		{
			int usedCarriers = 0;
			for (int i = 0; i < subCarrier_; i++)
			{
				if (ofdmph->carriers[i] == 1)
					usedCarriers++;
			}
			// TxPower is scaled with used carriers
			ph->Pt = getTxPower(p) * usedCarriers / subCarrier_;
		}
		else
			ph->Pt = getTxPower(p);

		assert(ph->srcSpectralMask);
		assert(ph->srcPosition);
		assert(ph->duration > 0);
		assert(ph->Pt > 0);

		Packet *pnew = Packet::alloc();

		PacketEvent *pe = new PacketEvent(p->copy());

		msgDisp.printStatus("Sending pkt_type " + itos(ch->ptype()) +
								" duration is " + dtos(ph->duration),
								"recv", NOW, nodeID_);

		Scheduler::instance().schedule(&txtimer, pe, (ph->duration));

		temptxtime = ph->duration;
		totTransTime += Scheduler::instance().clock() - ch->timestamp();

		msgDisp.printStatus("Ready to start transmission", "recv", 
								Scheduler::instance().clock(), nodeID_);

		phySentPkt_++;
		if (current_rcvs > 0)
			if (debug_)
				std::cerr << NOW << " UwOFDMPhy(" << nodeID_
						  << ")::recv() ERROR sending while receiving " << std::endl;

		startTx(p);
	}
} /* UnderwaterPhysical::recv */

void UwOFDMPhy::endTx(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);

	Tx_Time_ += ph->duration;

	if (powerScaling)
	{
		int bw = 0;
		for (int i = 0; i < subCarrier_; i++)
			if (ofdmph->carriers[i] == 1)
				bw += 1;
		Energy_Tx_ += consumedEnergyTx(ph->duration) * bw / subCarrier_;
	}
	else
		Energy_Tx_ += consumedEnergyTx(ph->duration);

	Transmitted_bytes_ += ch->size();

	return UnderwaterMPhyBpsk::endTx(p);
} /* UwOFDMPhy::endTx */

void UwOFDMPhy::startRx(Packet *p)
{
	hdr_mac *mach = HDR_MAC(p);
	hdr_MPhy *ph = HDR_MPHY(p);

	hdr_OFDM *ofdmph = HDR_OFDM(p);
	hdr_cmn *ch = HDR_CMN(p);

	std::ofstream myfile;
	bool overlapping;

	static int mac_addr = -1;

	ClMsgPhy2MacAddr msg;
	sendSyncClMsg(&msg);
	mac_addr = msg.getAddr();

	msgDisp.printStatus("Reception starting, current_rcvs " + 
						std::to_string(current_rcvs), "startRx", 
						Scheduler::instance().clock(), nodeID_);

	overlapping = freqOverlap(p, ofdmph->nativeOFDM);

	msgDisp.printStatus("freqOverlap() executed. Overlapping is " + 
						std::to_string(overlapping), "startRx", NOW, nodeID_);

	if (txPending == false && tx_busy_ == false && overlapping == false)
	{ // for simultaneous receptions

		// The receiver is is not synchronized on any transmission
		// so we can sync on this packet
		double snr_dB = 10 * log10(ph->Pr / ph->Pn);
		;
		if (snr_dB > getAcquisitionThreshold())
		{
			if (ph->modulationType == modid)
			{
				// This is a BPSK packet so we sync on it
				PktRx = p;

				if (debug_)
					std::cout << NOW << " UwOFDMPhy(" << nodeID_ << ")::StartRx() Adding packet " << p
							  << " seq_num " << ch->uid() << " isnative " << ofdmph->nativeOFDM 
							  << " ph->Pr " << ph->Pn << " ph->Pn " << ph->Pn << std::endl;

				pktqueue_.push_back(*p);

				if (debug_)
					plotPktQueue();

				current_rcvs++; // I am ACTUALLY TRYING TO RECEIVE THE PACKET, same for above

				msgDisp.printStatus("About to notify the MAC", "startRx", NOW, nodeID_);

				// Notify the MAC
				Phy2MacStartRx(p);

				return;
			} else {

				lostPackets[MODERR]++;
				if ((mach->macDA() == mac_addr) && (mach->ftype() != MF_CONTROL)){
					incrTot_pkts_lost();

				} else if ((mach->macDA() == mac_addr) && (mach->ftype() == MF_CONTROL)) {
					incrTotCrtl_pkts_lost();
				}
			}
		} else {

			lostPackets[LOWSNR]++;
			msgDisp.printStatus("dropping pkt, LOW SNR", "startRx", NOW, nodeID_);

			incrErrorPktsNoise();
			if (mach->ftype() != MF_CONTROL) {
				incrTot_pkts_lost();
			
			} else if (mach->ftype() == MF_CONTROL) {
				incrTotCrtl_pkts_lost();
			}
		}
	} else if (txPending == true || tx_busy_ == true) {
		msgDisp.printStatus("dropping pkt, tx pending", "startRx", NOW, nodeID_);

		lostPackets[TXPEN]++;
		if (mach->ftype() == MF_DATA) {

			if (mach->macDA() == nodeID_)
			if(debug_)
				std::cout << NOW << " UwOFDMPhy(" << nodeID_
						  << ")::startRx() [PROBLEM] Dropping DATA that was for me seq_num " 
						  << ch->uid() << " src = " << mach->macSA() << " bc txpending" << std::endl;

			incrTot_pkts_lost();
		}
		if (mach->ftype() == MF_RTS || mach->ftype() == MF_CTS) {
			incrTotCrtl_pkts_lost();
			lostPackets[TXPENCTRL]++;
		}
	} else {

		lostPackets[FREQCOLL]++;
		msgDisp.printStatus("dropping pkt, Frequency Collision", "startRx", NOW, nodeID_);

		if (mach->ftype() == MF_DATA) {
			if (mach->macDA() == nodeID_)
				if(debug_)
					std::cerr << NOW << " UwOFDMPhy(" << nodeID_
						  << ")::startRx() [PROBLEM] Dropping DATA that was for me seq_num " 
						  << ch->uid() << " src = " << mach->macSA() << " bc freqcollisions" << std::endl;
			incrTot_pkts_lost();
		} else if (mach->ftype() == MF_RTS || mach->ftype() == MF_CTS) {
			incrTotCrtl_pkts_lost();
			lostPackets[CTRLFERR]++;
		}
	}
} /* UwOFDMPhy::startRx */

/*
The method is activate at the end of receiving side.
*/

void UwOFDMPhy::endRx(Packet *p)
{
	Packet *current_p;
	current_p = p;
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_mac *mach = HDR_MAC(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);

	counter interferent_pkts;
	static int mac_addr = -1;
	ClMsgPhy2MacAddr msg;
	sendSyncClMsg(&msg);
	total_delay_ = ph->duration;
	mac_addr = msg.getAddr();

	bool pktfound = false;

	msgDisp.printStatus("Reception ended for pkt_type " + itos(ch->ptype()) + " seq_num " + itos(ch->uid()) +
							", current_rcvs " + itos(current_rcvs),
						"EndRx()", Scheduler::instance().clock(), nodeID_);

	for (auto x = pktqueue_.begin(); x != pktqueue_.end();)
	{
		hdr_cmn *chx = HDR_CMN(&*x);
		hdr_mac *machx = HDR_MAC(&*x);

		if ((chx->uid() == ch->uid()) && (ch->ptype() == chx->ptype()) 
		&& (mach->macDA() == machx->macDA()) && (mach->macSA() == machx->macSA()))
		{
			if (debug_)
				std::cout << NOW << " UwOFDMPhy(" << nodeID_ << ")::EndRx() Packet found in pktqueue_. current_p " 
				<< current_p << " isNative " << ofdmph->nativeOFDM << " seq_num " << ch->uid() 
				<< " dest " << mach->macDA() << std::endl;

			pktfound = true;
			current_p = &*x;
			x = pktqueue_.erase(x);
			break;
		} else {
			++x;
		}
	}
	if (debug_)
		plotPktQueue();

	ch = HDR_CMN(current_p);
	ph = HDR_MPHY(current_p);
	mach = HDR_MAC(current_p);
	ofdmph = HDR_OFDM(current_p);

	if (pktfound) {

		if (pktfound) {

			double per_ni; // packet error rate due to noise and/or interference
			double per_n;  // packet error rate due to noise only
			int nbits = ch->size() * 8;
			double interference_power;
			double x = RNG::defaultrng()->uniform_double();
			if (debug_)
				std::cout << NOW << " UwOFDMPhy(" << nodeID_ 
				<< ")::EndRx() FOUND IN QUEUE packet seq_num " << ch->uid() 
				<< " isnative " << ofdmph->nativeOFDM << " ph->Pr " << ph->Pn 
				<< " ph->Pn " << ph->Pn << std::endl;

			current_rcvs--; // I previously started to receive the packet

			if (ofdmph->nativeOFDM) {
				if (debug_)
					std::cout << NOW << " UwOFDMPhy::endRx(" << nodeID_ 
					<< ") getOFDMPER with noise. ph->Pr " << ph->Pr 
					<< " Ph->Pn " << ph->Pn << std::endl;

				per_n = getOFDMPER(ph->Pr / ph->Pn, nbits, current_p);
			} else {
				if (debug_)
					std::cout << NOW << " UwOFDMPhy::endRx(" << nodeID_ 
					<< ") getPER with noise. ph->Pr " << ph->Pr << " Ph->Pn " << ph->Pn << std::endl;
				per_n = getPER(ph->Pr / ph->Pn, nbits, p);
			}

			bool error_n = x <= per_n;
			bool error_ni = 0;

			if (!error_n)
			{
				if (interference_)
				{
					if (Interference_Model == "MEANPOWER")
					{ // only meanpower
					  // is allow in right now
					  // OFDMphy. It's a uwphysical class variable
						msgDisp.printStatus("getting interference power", "EndRx()", NOW, nodeID_);

						// WARNING: this only uses the interference power on the used subcarriers
						// the problem is that it averages it which is not always the case in real life
						interference_power = interference_->getInterferencePower(p);
						// per_ni = interference > 0; // this if model unknown and interf always distructive
						msgDisp.printStatus("getOFDMPER with Interference", "EndRx()", NOW, nodeID_);
						
						if (ofdmph->nativeOFDM)
							per_ni = getOFDMPER(ph->Pr / (ph->Pn + interference_power), nbits, p);
						else
							per_ni = getPER(ph->Pr / (ph->Pn + interference_power), nbits, p);
						error_ni = x <= per_ni;

						if (debug_)
							std::cout << "Interference from x = " << x << " interf_power = " 
							<< interference_power << " per_ni " << per_ni << std::endl;
					} else {
						std::cerr << "Please choose only MEANPOWER as "
									 "Interference_Model"
								  << std::endl;
						exit(1);
					}
					interferent_pkts = interference_->getCounters(p);

				} else {
					if (ofdmph->nativeOFDM)
						per_ni = getOFDMPER(ph->Pr / (ph->Pn + ph->Pi),
											nbits, p); // PER of OFDM acoustic modem
					else
						per_ni = getPER(ph->Pr / (ph->Pn + ph->Pi),
										nbits, p); // PER of OFDM acoustic modem
					error_ni = x <= per_ni;
				}
			}
			if (time_ready_to_end_rx_ > Scheduler::instance().clock())
			{
				Rx_Time_ = Rx_Time_ + ph->duration + getPropagationDelay(p) - time_ready_to_end_rx_ +
						   Scheduler::instance().clock();
			} else {
				Rx_Time_ += (ph->duration + getPropagationDelay(p)); // calculating the total delay.
			}

			time_ready_to_end_rx_ = Scheduler::instance().clock() + ph->duration;
			Energy_Rx_ += consumedEnergyRx(ph->duration);
			total_delay_ = (Rx_Time_ + getPropagationDelay(p));

			ch->error() = error_ni || error_n;
			if (ch->error()) {
				if (error_n) {
					if(debug_)
						std::cout << NOW << "  UwOFDMPhy(" << nodeID_
							<< ")::endRx() error due to noise. PER = " << per_n << std::endl;
					
					if (mach->ftype() == MF_DATA) {
						if (mach->macDA() == nodeID_)
							if(debug_)
							std::cout << NOW << " UwOFDMPhy(" << nodeID_
									  << ")::endRx() [PROBLEM] Dropping DATA for me seq_num " 
									  << ch->uid() << " src = " << mach->macSA() << " bc noise" << std::endl;
					}
				}
				if (error_ni) {
					if(debug_)
						std::cout
							<< NOW << "  UwOFDMPhy(" << nodeID_
							<< ")::endRx() error due to interference. PER = " << per_ni
							<< " Interference Power " << interference_power << std::endl;
					if (mach->ftype() == MF_DATA)
					{
						if (mach->macDA() == nodeID_)
						if (debug_)
							std::cout << NOW << " UwOFDMPhy(" << nodeID_
									  << ")::endRx() [PROBLEM] Dropping DATA for me seq_num " 
									  << ch->uid() << " src = " << mach->macSA() << " bc interference" 
									  << " pwr " << ph->Pr << " noise " << ph->Pn << " interf_pwr " 
									  << interference_power << std::endl;
					}
				}
			}
			if (debug_)
			{
				if (error_ni == 1) {
					std::cout
						<< NOW << "  UwOFDMPhy(" << nodeID_
						<< ")::endRx() packet " << ch->uid()
						<< " contains errors due to noise and interference."
						<< std::endl;
				}
				else if (error_n == 1)
				{
					std::cout << NOW << "  UwOFDMPhy(" << nodeID_
							  << ")::endRx() packet " << ch->uid()
							  << " contains errors due to noise." << std::endl;
				}
			}
			if (error_n)
			{
				incrErrorPktsNoise();
				lostPackets[NOISEERR]++;
				if (mach->ftype() != MF_CONTROL) {
					incrTot_pkts_lost();
				
				} else if (mach->ftype() == MF_CONTROL) {
					incrTotCrtl_pkts_lost();
				}
			} else if (error_ni) {
				lostPackets[COLLERR]++;
				if (mach->ftype() != MF_CONTROL) {

					incrErrorPktsInterf();
					incrTot_pkts_lost();
					
					if (interferent_pkts.second >= 1) {
						incrCollisionDATA();
					} else {
						if (interferent_pkts.first > 0) {
							incrCollisionDATAvsCTRL();
						}
					}
				} else if (mach->ftype() == MF_CONTROL) {

					lostPackets[CTRLCERR]++;
					incrTotCrtl_pkts_lost();
					incrErrorCtrlPktsInterf();

					if (interferent_pkts.first > 0) {
						incrCollisionCTRL();
					}
				}
			}
			buffered_pkt_num--;
			sentUpPkts++;

			sendUp(p);

			if (debug_)
				std::cout << NOW << "  UwOFDMPhy(" << nodeID_
						  << ")::EndRx() Packet UP DONE" << std::endl;

			// PktRx = 0;
		} else {
			buffered_pkt_num--;
			dropPacket(p);
		}
	} else {
		msgDisp.printStatus("Packet NOT found in pktqueue_, DROPPING it", "EndRx()", NOW, nodeID_);

		buffered_pkt_num--;
		dropPacket(p);
	}

	msgDisp.printStatus("Function Ending", "EndRx", NOW, nodeID_);
}

void UwOFDMPhy::interruptReceptions()
{

	for (auto x = pktqueue_.begin(); x != pktqueue_.end();)
	{
		dropPacket(&*x);
		x = pktqueue_.erase(x);
		buffered_pkt_num--;
	}

	current_rcvs = 0;
	std::cerr << NOW << "  UwOFDMPhy(" << nodeID_
			  << ")::interruptReceptions() pktqueue size " << pktqueue_.size() 
			  << " buffered pkts " << buffered_pkt_num << " current_rcvs " << current_rcvs << std::endl;
}

double UwOFDMPhy::getTxDuration(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);

	double new_BitRate_;
	int txdurationtemp;

	//   assert(ph->srcSpectralMask);
	msgDisp.printStatus("ph->carrierNum " + itos(ofdmph->carrierNum), "getTxDuration", NOW, nodeID_);

	double used_bw = 0;
	for (int i = 0; i < ofdmph->carrierNum; i++)
	{
		if (ofdmph->carriers[i] == 1) {
			if (ofdmph->carMod[i] == "BPSK")
				used_bw += 1;

			else if (ofdmph->carMod[i] == "QPSK")
				used_bw += 2;
		}
	}

	if (used_bw == 0)
		std::cerr << "UwOFDMPhy(" << nodeID_ << ")::getTxDuration() ERROR: USED BANDWIDTH = 0" << std::endl;

	// for BPSK the bandwidth is B = 2/T,
	// where T is the symbol duration.
	// Here B is pre-determined by the Spectral Mask,
	// so we can calculate the bitrate R = 1/T = B/2
	if (BitRate_ <= 0)
		BitRate_ = ph->srcSpectralMask->getBandwidth() / 2.0;

	new_BitRate_ = used_bw / ofdmph->carrierNum * BitRate_;

	double txtime = (ch->size() * 8.0 / new_BitRate_);

	if (txtime <= 0) {
		cerr << " ch->size(): " << ch->size() << " bitrate: " << BitRate_ << std::endl;
	}
	assert(txtime > 0);

	if (debug_)
	{
		cerr << showpoint << NOW << " " << __PRETTY_FUNCTION__
			 << " packet size: " << ch->size()
			 << " tx duration: " << txtime
			 << " new_bitrate: " << new_BitRate_
			 << endl;
	}
	msgDisp.printStatus("tx_time " + dtos(txtime) + " old_brate " + dtos(BitRate_) + 
	" new_brate " + dtos(new_BitRate_), "getTxDuration", NOW, nodeID_);

	return (txtime);
}

double
UwOFDMPhy::getOFDMPER(double _snr, int _nbits, Packet *p)
{
	hdr_MPhy *ph = HDR_MPHY(p);
	MSpectralMask *sm = getRxSpectralMask(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);
	assert(sm);

	int ncarriers = sm->getBandwidth() / ofdmph->carrierSize;
	double snr_with_penalty = _snr * pow(10, RxSnrPenalty_dB_ / 10.0);
	double ber_ = 0;
	int usedCarriers = 0;
	double tber_ = 0;
	int brokenProb = 10; // out of 100

	for (int i = 0; i < ofdmph->carrierNum; i++)
	{
		// set the associated modulation with something like ph->modulationType = subCarMod

		if (ofdmph->carMod[i] == "BPSK")
		{
			tber_ = 0.5 * erfc(sqrt(snr_with_penalty));
		}
		else if (ofdmph->carMod[i] == "QPSK")
		{
			tber_ = erfc(sqrt(snr_with_penalty));
		}
		else if (ofdmph->carMod[i] == "BFSK")
		{
			tber_ = 0.5 * exp(-snr_with_penalty / 2);
		}
		else if (ofdmph->carMod[i] == "8PSK")
		{
			double const M = 8;
			tber_ = (1 / this->log2(M)) *
					get_prob_error_symbol_mpsk(snr_with_penalty, M);
		}
		else if (ofdmph->carMod[i] == "16PSK")
		{
			double const M = 16;
			tber_ = (1 / this->log2(M)) *
					get_prob_error_symbol_mpsk(snr_with_penalty, M);
		}
		else if (ofdmph->carMod[i] == "32PSK")
		{
			double const M = 32;
			tber_ = (1 / this->log2(M)) *
					get_prob_error_symbol_mpsk(snr_with_penalty, M);
		}

		ber_ += tber_ * ofdmph->carriers[i];
		usedCarriers += ofdmph->carriers[i];
	}
	ber_ = ber_ / usedCarriers;
	// WARNING: the BER calculated carrier by carrier makes sense if there are weird thinngs in the network,
	// otherwise, since the noise it's already scaled, it's the same as computing on total snr

	// PER calculation
	double per = 1 - pow(1 - ber_, _nbits);
	if (debug_)
		std::cout << NOW << " UwOFDMPhy(" << nodeID_ << ")::getOFDMPER BER = " << ber_ << " PER = " << per << std::endl;
	return per;
}

// Redefinition of getNoisePower function dependent on subcarriers
// The noise has a constant value that is multiplied for the used bandwidth
// A smaller bandwidth corresponds to a smaller noise value and higher SNR

double UwOFDMPhy::getOFDMNoisePower(Packet *p)
{
	double actualBand = 0;
	hdr_MPhy *ph = HDR_MPHY(p);
	hdr_OFDM *ofdmph = HDR_OFDM(p);

	// std::cout << "NodeID " << nodeID_ << " subCarrier_ " << subCarrier_ << std::endl;
	MSpectralMask *sm = getRxSpectralMask(p);
	assert(sm);
	for (int i = 0; i < ofdmph->carrierNum; i++)
	{
		actualBand += ofdmph->carrierSize * ofdmph->carriers[i];
	}

	double noiseOFDM = getNoisePower(p) * actualBand / sm->getBandwidth();
	if (debug_)
		std::cout << NOW << " UwOFDMPhy::getOFDMNoisePower actualBand " << actualBand 
		<< " noisePower " << getNoisePower(p) << " noiseOFDM " << noiseOFDM << std::endl;

	return (noiseOFDM);
}

double
UwOFDMPhy::getDistance(Packet *_p) // calculates the distance from src to the sink.
{
	hdr_MPhy *ph = HDR_MPHY(_p);
	double x_src = (ph->srcPosition)->getX();
	double y_src = (ph->srcPosition)->getY();
	double z_src = (ph->srcPosition)->getZ();
	double x_dst = (ph->dstPosition)->getX();
	double y_dst = (ph->dstPosition)->getY();
	double z_dst = (ph->dstPosition)->getZ();
	return sqrt(pow(x_src - x_dst, 2.0) + pow(y_src - y_dst, 2.0) +
				pow(z_src - z_dst, 2.0));
}

// calculates the propagation delay based on the distance and velocity of sound in sea water.
double
UwOFDMPhy::getPropagationDelay(Packet *_p)
{
	double dist = getDistance(_p);

	return (dist / 1500);
}

// Checks if the arriving packet overlaps in carriers with previous packets that
// are being received in this moment
bool UwOFDMPhy::freqOverlap(Packet *p2, bool isOFDM)
{
	if (!p2)
		return false;

	hdr_OFDM *ofdmph2 = HDR_OFDM(p2);

	if (debug_)
		for (int i = 0; i < ofdmph2->carrierNum; i++)
			std::cout << "carrier[" << i << "]=" << ofdmph2->carriers[i] << std::endl;

	for (const auto &x : pktqueue_)
	{
		hdr_OFDM *ofdmph1 = HDR_OFDM(&x);
		for (int i = 0; i < ofdmph1->carrierNum; i++)
			if (ofdmph1->carriers[i] && ofdmph2->carriers[i])
				return true;
	}

	if (!isOFDM)
	{
		// A non-OFDM packet arrives
		std::cout << "WARNING: OFDM node receiving NON OFDM packet" << std::endl;
	}

	return false;
}
void UwOFDMPhy::createOFDMhdr(Packet *p)
{
	// TODO: are these spectral mask really different,
	//  how does the packet know?
	//  Fill the carriers of the original system

	hdr_OFDM *ofdmph = HDR_OFDM(p);
	hdr_MPhy *ph = HDR_MPHY(p);

	MSpectralMask *txsm = ph->srcSpectralMask;
	MSpectralMask *rxsm = getRxSpectralMask(p);
	double newPktStart = txsm->getFreq() - txsm->getBandwidth() / 2;
	double newPktEnd = txsm->getFreq() + txsm->getBandwidth() / 2;
	double nodeStart = rxsm->getFreq() - rxsm->getBandwidth() / 2;
	double nodeEnd = rxsm->getFreq() + rxsm->getBandwidth() / 2;
	double carsize = rxsm->getBandwidth() / subCarrier_;

	if (debug_)
	{
		std::cout << NOW << " incoming Pkt Start and End: " << newPktStart << " " << newPktEnd << std::endl;
		std::cout << NOW << " node Start and End: " << nodeStart << " " << nodeEnd << std::endl;
	}

	ofdmph->carrierNum = subCarrier_;
	int i = 0;
	for (double s = nodeStart; s < nodeEnd; s = s + carsize)
		if ((newPktStart <= s + carsize) && (newPktEnd > s))
		{
			if (debug_)
				std::cout << NOW << " 1 Added " << std::endl;
			ofdmph->carriers[i] = 1;
		}
		else
			ofdmph->carriers[i] = 0;
	i++;

	if (debug_)
		std::cout << NOW << " End createOFDMhdr function" << std::endl;
	return;
}

// Returns total number of packets lost for low SNR
int UwOFDMPhy::getLowSnrPktLost() const
{
	return lostPackets[LOWSNR];
}

// Returns total number of packets lost for noise error
int UwOFDMPhy::getNoiseErrPktLost() const
{
	return lostPackets[NOISEERR];
}

// Returns total number of packets lost for Collision error
int UwOFDMPhy::getCollErrPktLost() const
{
	return lostPackets[COLLERR];
}

// Returns total number of packets lost for transmission pending
int UwOFDMPhy::getTxPenPktLost() const
{
	return lostPackets[TXPEN];
}

// Returns total number of packets lost for transmission pending
int UwOFDMPhy::getTxPenCtrlLost() const
{
	return lostPackets[TXPENCTRL];
}

// Returns total number of packets (DATA + CTRL) lost for collision in frequency
int UwOFDMPhy::getFreqCollPktLost() const
{
	return lostPackets[FREQCOLL];
}

// Returns total number of SubCarriers available for a given node
int UwOFDMPhy::getModErrPktLost() const
{
	return lostPackets[MODERR];
}

// Returns total number of CTRL Packets lost for collision in frequency
int UwOFDMPhy::getCtrlFCollPktLost() const
{
	return lostPackets[CTRLFERR];
}

// Returns total number of CTRL Packets lost for collision error
int UwOFDMPhy::getCtrlCErrPktLost() const
{
	return lostPackets[CTRLCERR];
}
// Returns total number of SubCarriers available for a given node
double UwOFDMPhy::getTransmissionTime() const
{
	return totTransTime;
}
// Returns total number of packets sent by phy layer
int UwOFDMPhy::getPhyPktSent() const
{
	return phySentPkt_;
}

// return nodeID
int UwOFDMPhy::getNodeID() const
{
	return nodeID_;
}

// return number of nodes in the simulation
int UwOFDMPhy::getNodeNum() const
{
	return nodeNum_;
}

// return the number of subcarriers in the simulation
int UwOFDMPhy::getSubCarNum() const
{
	return subCarrier_;
}

// set number of nodes in the simulation
void UwOFDMPhy::setNodeNum(int n)
{
	nodeNum_ = n;
	return;
}
int UwOFDMPhy::getSentUpPkts()
{
	return sentUpPkts;
}

// set number of nodes in the simulation
void UwOFDMPhy::setNodeID(int n)
{
	nodeID_ = n;
	return;
}

// set number of nodes in the simulation
void UwOFDMPhy::setSubCarNum(int n)
{
	subCarrier_ = n;
	return;
}
void UwOFDMPhy::setBrokenCar(int bottom, int top)
{
	brokenCarriers_.push_back(bottom);
	brokenCarriers_.push_back(top);
	return;
}

// return the number of subcarriers in the simulation
void UwOFDMPhy::showSubCar()
{
	std::cout << "showSubCar() unimplemented\n"
			  << std::endl;
	return;
}

// sets subcarriers from outside the class
void UwOFDMPhy::setSubCar(int index, int value)
{
	std::cerr << "setSubCar() unimplemented\n"
			  << std::endl;
	return;
}

int UwOFDMPhy::recvSyncClMsg(ClMessage *m)
{
	if (m->type() == CLMSG_UWPHY_TX_BUSY)
	{
		if (((ClMsgUwPhyTxBusy *)m)->getGetOp() == 1)
		{
			if (debug_)
				std::cout << NOW << " UwOFDMPhy(" << nodeID_ 
				<< ")::recvSyncClMsg [get] tx_busy_ to send back is " << tx_busy_ << std::endl;
			((ClMsgUwPhyTxBusy *)m)->setTxBusy(tx_busy_);
		}
		else
		{
			tx_busy_ = ((ClMsgUwPhyTxBusy *)m)->getTxBusy();
			if (debug_)
				std::cout << NOW << " UwOFDMPhy(" << nodeID_ 
				<< ")::recvSyncClMsg [set] tx_busy_ received is " << tx_busy_ << std::endl;
		}
		return 0;
	}
	return UnderwaterMPhyBpsk::recvSyncClMsg(m);
}
void UwOFDMPhy::plotPktQueue()
{
	std::cout << NOW << " UwOFDMPhy(" << nodeID_ << ")::plotPktQueue()" << std::endl;
	for (auto x = pktqueue_.begin(); x != pktqueue_.end();)
	{
		hdr_cmn *ch = HDR_CMN(&*x);
		hdr_MPhy *ph = HDR_MPHY(&*x);
		hdr_OFDM *ofdmph = HDR_OFDM(&*x);
		hdr_mac *mach = HDR_MAC(&*x);
		std::cout << "Packet " << &*x << " seq_num " << ch->uid() 
					<< " native " << ofdmph->nativeOFDM << " ph->Pr " << ph->Pn
					<< " ph->Pn " << ph->Pn << " MACDE " << mach->macDA() 
					<< " MACSRC " << mach->macSA() << std::endl;
		++x;
	}
}