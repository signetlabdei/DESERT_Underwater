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
 * @file   uwtp-module.cc
 * @author Saiful Azad
 * @version 1.0.0
 *
 * @brief  Implementations of a simple Transport Layer Protocol (UWTP)
 */

#include <cassert>
#include <iomanip>
#include <iostream>

#include <uwip-module.h>

#include "uwtp-module.h"

/**
 * Class that represent the binding with tcl scripting language
 */
static class UWTPClass : public TclClass
{
public:
	/**
	 * Constructor of the class
	 */
	UWTPClass()
		: TclClass("Module/UW/TP")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UWTP);
	}
} class_module_uwtp;

void
UWTP_delayTimer::expire(Event *e)
{

	module->sendNack();
}

UWTP::UWTP()
	: portcounter(0)
	, ack_tx_count(0)
	, nack_tx_count(0)
	, ack_rx_count(0)
	, nack_rx_count(0)
	, delay_timer_(this)
	, ack_mode(WITH_ACK)
	, destPort_(0)
	, seq_no_counter(-1)
	, ack_tx_mode(WITHOUT_CUM_ACK)
{
	if (portcounter != 0)
		portcounter = 0;

	bind("debug_", &debug_);
	bind("send_buffer_size_", (int *) &send_buffer_size);
	bind("receive_buffer_size_", (int *) &receive_buffer_size);
	bind("delay_interval_", (double *) &delay_interval);
	bind("destPort_", (int *) &destPort_);
	bind("nack_retx_time_", (double *) &nack_retx_time);
	bind("pkt_delete_time_from_queue_", (double *) &pkt_delete_time_from_queue);
	bind("expected_ACK_threshold_", (double *) &expected_ACK_threshold);
}

UWTP::~UWTP()
{
}

int
UWTP::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();
	if (argc == 2) {
		if (strcasecmp(argv[1], "getUWTPDataHSize") == 0) {
			tcl.resultf("%d", getUWTPDataHSize());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getUWTPAckHSize") == 0) {
			tcl.resultf("%d", getUWTPAckHSize());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getUWTPNackHSize") == 0) {
			tcl.resultf("%d", getUWTPNackHSize());
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getAckTxCount") == 0) {
			tcl.resultf("%d", ack_tx_count);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getNackTxCount") == 0) {
			tcl.resultf("%d", nack_tx_count);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getAckRxCount") == 0) {
			tcl.resultf("%d", ack_rx_count);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "getNackRxCount") == 0) {
			tcl.resultf("%d", nack_rx_count);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setAckMode") == 0) {
			ack_mode = WITH_ACK;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setNoAckMode") == 0) {
			ack_mode = WITHOUT_ACK;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setCumAckMode") == 0) {
			ack_tx_mode = WITH_CUM_ACK;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setNoCumAckMode") == 0) {
			ack_tx_mode = WITHOUT_CUM_ACK;
			return TCL_OK;
		}
	}

	if (argc == 3) {
		if (strcasecmp(argv[1], "assignPort") == 0) {
			Module *m = dynamic_cast<Module *>(tcl.lookup(argv[2]));
			if (!m)
				return TCL_ERROR;
			int port = assignPort(m);
			tcl.resultf("%d", port);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "node_id") == 0) {
			node_id_ = atoi(argv[2]);
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

void
UWTP::recv(Packet *p)
{
	cout << TIME << " UWTP::recv(" << node_id
		 << "), a Packet is sent without source module!!" << endl;
	Packet::free(p);
}

void
UWTP::initPkt(Packet *p, int id, int seq_no_counter)
{

	struct hdr_cmn *cmh = HDR_CMN(p);
	struct hdr_uwtp_data *uwtpdh = HDR_UWTP_DATA(p);

	if (debug_)
		cout << TIME << " UWTP::initPkt(" << node_id
			 << "), initializing data packet " << endl;

	map<int, int>::const_iterator iter = port_map.find(id);

	if (iter == port_map.end()) {
		fprintf(stderr,
				"UWTP::recvData() no port assigned to id %d, dropping "
				"packet!!!\n",
				id);
		Packet::free(p);
	}

	int sport = iter->second;
	assert(sport > 0 && sport <= portcounter);

	cmh->direction() = hdr_cmn::DOWN;
	uwtpdh->type_ = UWTP_DATA;
	cmh->uid() = seq_no_counter;
	uwtpdh->sport_ = sport;
	uwtpdh->dport_ = destPort_;
	cmh->size() += uwtpdh->size();
	if (debug_) {
		cout << "Packet type " << uwtpdh->type_ << endl;
		cout << "Seq no " << cmh->uid() << endl;
		cout << "Source port " << uwtpdh->sport_ << endl;
		cout << "Destination port " << uwtpdh->dport_ << endl;
	}
}

void
UWTP::initAckPkt(Packet *p, Packet *ack_pkt, int seq_no)
{

	if (debug_)
		cout << TIME << " UWTP::initACKPkt(" << node_id
			 << "), initializing ACK packet" << endl;
	struct hdr_cmn *cmh = HDR_CMN(p);
	struct hdr_uwtp_data *uwtpdh = HDR_UWTP_DATA(p);
	struct hdr_uwip *iph = HDR_UWIP(p);
	struct hdr_cmn *cmha = HDR_CMN(ack_pkt);
	struct hdr_uwtp_ack *uwtpah = HDR_UWTP_ACK(ack_pkt);
	struct hdr_uwip *ipah = HDR_UWIP(ack_pkt);

	ipah->daddr() = iph->saddr();
	uwtpah->type_ = UWTP_ACK;
	uwtpah->sport_ = uwtpdh->getDport();
	uwtpah->dport_ = uwtpdh->getSport();
	uwtpah->seq_no_ = seq_no;
	cmha->direction() = hdr_cmn::DOWN;
	cmha->size() += uwtpah->size();
}

void
UWTP::initNackPkt(Packet *p, Packet *nack_pkt, int nack_seq_no)
{

	if (debug_)
		cout << TIME << " UWTP::initNackPkt(" << node_id
			 << "), initializing NACK packet " << endl;
	struct hdr_cmn *cmh = HDR_CMN(p);
	struct hdr_uwtp_data *uwtpdh = HDR_UWTP_DATA(p);
	struct hdr_uwip *iph = HDR_UWIP(p);
	struct hdr_cmn *cmhna = HDR_CMN(nack_pkt);
	struct hdr_uwtp_nack *uwtpnah = HDR_UWTP_NACK(nack_pkt);
	struct hdr_uwip *ipnah = HDR_UWIP(nack_pkt);

	ipnah->daddr() = iph->saddr();
	uwtpnah->type_ = UWTP_NACK;
	uwtpnah->sport_ = uwtpdh->getDport();
	uwtpnah->dport_ = uwtpdh->getSport();
	uwtpnah->seq_no_ = nack_seq_no;
	cmhna->direction() = hdr_cmn::DOWN;
	cmhna->size() += uwtpnah->size();
}

void
UWTP::sendAck(Packet *p)
{

	if (debug_)
		cout << TIME << " UWTP::sendAck(" << node_id << "), sending Ack "
			 << endl;

	ack_tx_count++;

	hdr_uwtp_ack *uah = HDR_UWTP_ACK(p);

	if (p != NULL) {
		sendDown(p);
	} else
		cout << TIME << "Program is pointing to a null pointer " << endl;
}

void
UWTP::sendNack()
{

	if (debug_)
		cout << TIME << " UWTP::sendNack(" << node_id << "), seding Nack "
			 << endl;

	if (debug_) {
		for (map<UWTPPair, NackPktStoreInfo *>::iterator it_nb =
						nackBuffer.begin();
				it_nb != nackBuffer.end();
				it_nb++) {
			cout << TIME << " Destination port : " << (it_nb->first).first
				 << " Seq no " << (it_nb->first).second << " nack tx info "
				 << it_nb->second->getNackTxInfo() << endl;
		}
	}

	nack_tx_count++;

	if (nackBuffer.size() != 0) {
		for (map<UWTPPair, NackPktStoreInfo *>::iterator it_nb =
						nackBuffer.begin();
				it_nb != nackBuffer.end();
				it_nb++) {
			if (it_nb->second->getNackTxInfo() == FALSE) {
				it_nb->second->setNackTxInfo(TRUE);
				it_nb->second->setNackTxTime(TIME);
				Packet *nack_p = (it_nb->second->getNackPnt())->copy();
				sendDown(nack_p);
			} else if (it_nb->second->getNackTxInfo() == TRUE &&
					it_nb->second->getTimeSpentInNackBuffer() >
							nack_retx_time) {
				it_nb->second->setNackTxTime(TIME);
				Packet *nack_p = (it_nb->second->getNackPnt())->copy();
				sendDown(nack_p);
			} else {
				// do nothing
			}
		}
	}

	if (nackBuffer.size() != 0) {
		int count;
		for (map<UWTPPair, NackPktStoreInfo *>::iterator it_nb =
						nackBuffer.begin();
				it_nb != nackBuffer.end();
				it_nb++) {
			if (it_nb->second->getNackTxInfo() == FALSE ||
					it_nb->second->getTimeSpentInNackBuffer() > nack_retx_time)
				count++;
		}

		if (count > 0) {
			delay_timer_.resched(JITTER * delay_interval);
		}
	}
}

void
UWTP::checkReceiveQueue(int port, int id)
{

	if (debug_)
		cout << TIME << " UWTP::checkReceiveQueue(" << node_id
			 << "), any other packet which can be possible to transfer to the "
				"upper layer"
			 << endl;

	map<PortNo, ExpectedPktSeqNo>::iterator it_e = expPktInfo.find(port);

	for (map<UWTPPair, Packet *>::iterator it_r = receiveBuffer.begin();
			it_r != receiveBuffer.end();
			it_r++) {
		if ((it_r->first).first == it_e->first &&
				(it_r->first).second == it_e->second) {
			it_e->second = it_e->second + 1;
			Packet *pp = it_r->second;
			struct hdr_cmn *ch = HDR_CMN(pp);
			ch->size() -= sizeof(hdr_uwtp_data);
			sendUp(id, pp);
		}
	}

	for (map<UWTPPair, Packet *>::iterator it_r = receiveBuffer.begin();
			it_r != receiveBuffer.end();
			it_r++) {
		if ((it_r->first).first == it_e->first &&
				(it_r->first).second < it_e->second) {
			receiveBuffer.erase(make_pair(it_e->first, it_e->second));
		}
	}
}

void
UWTP::checkNack(int port_no, int seq_no)
{

	if (debug_)
		cout << TIME << " UWTP::checkNack(" << node_id
			 << "), any packet which is received has nack packet saved in the "
				"nack buffer"
			 << endl;

	map<UWTPPair, NackPktStoreInfo *>::iterator it_nb =
			nackBuffer.find(make_pair(port_no, seq_no));

	if (it_nb != nackBuffer.end()) {
		nackBuffer.erase(make_pair(port_no, seq_no));
	}

	if (debug_) {
		cout << TIME << " UWTP::checkNack(" << node_id
			 << "), Checking nack buffer " << endl;
		for (map<UWTPPair, NackPktStoreInfo *>::iterator it_nb =
						nackBuffer.begin();
				it_nb != nackBuffer.end();
				it_nb++) {
			cout << TIME << " Destination port : " << (it_nb->first).first
				 << " Seq no " << (it_nb->first).second << " nack tx info "
				 << it_nb->second->getNackTxInfo() << endl;
		}
	}
}

void
UWTP::recvData(Packet *p, int id)
{

	if (debug_)
		cout << TIME << " UWTP::recvData(" << node_id
			 << "), a data packet is received" << endl;

	hdr_uwtp_data *uwtpdh = HDR_UWTP_DATA(p);
	hdr_cmn *cmh = HDR_CMN(p);
	int sport_no = uwtpdh->getSport();
	int dport_no = uwtpdh->getDport();
	int seq_no = cmh->uid();

	if (debug_) {
		cout << "Packet type " << uwtpdh->type_ << endl;
		cout << "Seq No " << seq_no << endl;
		cout << "Source port " << sport_no << endl;
		cout << "Destination port " << dport_no << endl;
	}

	checkNack(dport_no, seq_no);

	map<UWTPPair, UWTPPktStoreInfo *>::iterator it_dp;

	if (debug_) {
		for (map<PortNo, ExpectedPktSeqNo>::iterator it_e = expPktInfo.begin();
				it_e != expPktInfo.end();
				it_e++) {
			cout << TIME << " Port no " << it_e->first << " expected seq no "
				 << it_e->second << endl;
		}
	}

	map<PortNo, ExpectedPktSeqNo>::iterator it_e = expPktInfo.find(dport_no);

	if (ack_mode == WITH_ACK && ack_tx_mode == WITHOUT_CUM_ACK) {
		Packet *ack_pkt = Packet::alloc();
		initAckPkt(p, ack_pkt, seq_no);
		sendAck(ack_pkt);
	}

	if (it_e == expPktInfo.end()) {
		sendUp(id, p);
		expPktInfo.insert(make_pair(dport_no, seq_no + 1));
		map<PortNo, ExpectedPktSeqNo>::iterator it_e =
				expPktInfo.find(dport_no);
	} else {
		if (it_e->second > seq_no) {
			if (debug_)
				cout << TIME
					 << "A packet is received with lower sequence number than "
						"expected one "
					 << endl;
			drop(p, 1, "LTESN"); // Less than expected sequence number
			return;
		} else if (it_e->second == seq_no) {
			cmh->size() -= uwtpdh->size();
			Packet *rcvPkt = p->copy();
			sendUp(id, p);
			map<PortNo, ExpectedPktSeqNo>::iterator it_e =
					expPktInfo.find(dport_no);
			it_e->second = seq_no + 1;
			checkReceiveQueue(dport_no, id);
			if (ack_mode == WITH_ACK && ack_tx_mode == WITH_CUM_ACK) {
				if (expected_ACK_threshold < RNV) {
					Packet *ack_pkt = Packet::alloc();
					map<PortNo, ExpectedPktSeqNo>::iterator it_e =
							expPktInfo.find(dport_no);
					hdr_uwtp_data *udh = HDR_UWTP_DATA(rcvPkt);
					initAckPkt(rcvPkt, ack_pkt, (it_e->second) - 1);
					sendAck(ack_pkt);
				}
			}
			return;
		} else {

			if (nackBuffer.size() == 0) {
				delay_timer_.resched(JITTER * delay_interval);
			}

			for (int i = it_e->second; i < seq_no; i++) {
				map<UWTPPair, Packet *>::iterator it_rb =
						receiveBuffer.find(make_pair(dport_no, i));
				map<UWTPPair, NackPktStoreInfo *>::iterator it_nb =
						nackBuffer.find(make_pair(dport_no, it_e->second));
				if (it_rb == receiveBuffer.end() && it_nb == nackBuffer.end()) {
					Packet *nack_pkt = Packet::alloc();
					initNackPkt(p, nack_pkt, i);
					nack_store_info = new NackPktStoreInfo;
					nack_store_info->setNackPnt(nack_pkt);
					nack_store_info->setNackTxInfo(FALSE);
					nackBuffer.insert(
							make_pair(make_pair(dport_no, i), nack_store_info));
				}
			}

			map<UWTPPair, Packet *>::iterator it_r =
					receiveBuffer.find(make_pair(dport_no, seq_no));
			if (it_r == receiveBuffer.end()) {
				if (receiveBuffer.size() >= receive_buffer_size) {
					int lowest_seq_no = MAX_PORT_NO;
					for (map<UWTPPair, Packet *>::iterator it_rc =
									receiveBuffer.begin();
							it_r != receiveBuffer.end();
							it_r++) {
						if ((it_rc->first).first == dport_no &&
								(it_rc->first).second < lowest_seq_no) {
							lowest_seq_no = (it_rc->first).second;
						}
					}
					map<UWTPPair, Packet *>::iterator it_r1 =
							receiveBuffer.find(
									make_pair(dport_no, lowest_seq_no));
					if (it_r1 != receiveBuffer.end()) {
						map<PortNo, ExpectedPktSeqNo>::iterator it_e =
								expPktInfo.find(dport_no);
						it_e->second = lowest_seq_no;
						checkReceiveQueue(dport_no, id);
					}
					receiveBuffer.insert(
							make_pair(make_pair(dport_no, seq_no), p));
				} else {
					receiveBuffer.insert(
							make_pair(make_pair(dport_no, seq_no), p));
				}
			}
		}
	}

	if (sendBuffer.size() > 0) {
		for (map<UWTPPair, UWTPPktStoreInfo *>::iterator it_p =
						sendBuffer.begin();
				it_p != sendBuffer.end();
				it_p++) {
			if (it_p->second->getTimeSpentInQueue() >
					pkt_delete_time_from_queue) {
				sendBuffer.erase(it_p->first);
			}
		}
	}
}

void
UWTP::recvAck(Packet *p)
{

	if (debug_)
		cout << TIME << " UWTP::rcvAck(" << node_id
			 << "), an ACK packet is received" << endl;

	hdr_uwtp_ack *uwtpah = HDR_UWTP_ACK(p);

	ack_rx_count++;

	if (ack_tx_mode == WITHOUT_CUM_ACK) {

		map<UWTPPair, UWTPPktStoreInfo *>::iterator it_p = sendBuffer.find(
				make_pair(uwtpah->getSport(), uwtpah->getSeqNo()));

		if (it_p != sendBuffer.end()) {
			sendBuffer.erase(it_p->first);
		} else {
			cout << TIME << " A wrong packet is received" << endl;
		}
	} else if (ack_tx_mode == WITH_CUM_ACK) {
		int count = 0;
		for (map<UWTPPair, UWTPPktStoreInfo *>::iterator it_p =
						sendBuffer.begin();
				it_p != sendBuffer.end();
				it_p++) {
			if ((it_p->first).first == uwtpah->getSport() &&
					(it_p->first).second <= uwtpah->getSeqNo()) {
				count++;
				sendBuffer.erase(it_p->first);
			}
		}
		if (debug_)
			cout << TIME << " No of pkt deleted from buffer " << count << endl;
	} else {
		cout << TIME << " Wrong ack tx mode is selected " << endl;
	}
}

void
UWTP::recvNack(Packet *p)
{

	if (debug_)
		cout << TIME << " UWTP::recvNack(" << node_id
			 << "), a Nack packet is received" << endl;

	nack_rx_count++;

	hdr_uwtp_nack *uwtpnah = HDR_UWTP_NACK(p);

	map<UWTPPair, UWTPPktStoreInfo *>::iterator it_p = sendBuffer.find(
			make_pair(uwtpnah->getSport(), uwtpnah->getSeqNo()));

	Packet *curr_data_pkt = (it_p->second->getPktPnt())->copy();

	if (it_p != sendBuffer.end()) {
		sendDown(curr_data_pkt);
	} else {
		cout << TIME << " A wrong nack packet is received" << endl;
	}
}

void
UWTP::recv(Packet *p, int idSrc)
{

	if (debug_)
		cout << TIME << " UWTP::recv(" << node_id
			 << "), a data/ack/nack packet is received from upper layer or "
				"from lower layer"
			 << endl;

	hdr_cmn *ch = HDR_CMN(p);

	if (!ch->error()) {
		if (ch->direction() == hdr_cmn::UP) {

			struct hdr_uwtp *uwtph = HDR_UWTP(p);
			struct hdr_uwtp_data *uwtpdh = HDR_UWTP_DATA(p);

			map<int, int>::const_iterator iter =
					id_map.find(uwtpdh->getDport());

			if (iter == id_map.end()) {
				// Unknown Port Number
				cout << TIME
					 << "UWtrans::recv() (dir:UP), receive a packet with "
						"unrecognized port number"
					 << endl;
				drop(p, 1, "UPN");
				return;
			}

			int id = iter->second;
			if (debug_)
				cout << "dest port " << uwtpdh->getDport() << "id of src " << id
					 << endl;

			switch (uwtph->type_) {

				case (UWTP_DATA):
					recvData(p, id);
					break;

				case (UWTP_ACK):
					recvAck(p);
					break;

				case (UWTP_NACK):
					recvNack(p);
					break;
			}

		} else {

			struct hdr_uwtp_data *uwtpdh = HDR_UWTP_DATA(p);

			map<UWTPPair, UWTPPktStoreInfo *>::iterator it_p =
					sendBuffer.find(make_pair(uwtpdh->getDport(), ch->uid()));

			if (it_p != sendBuffer.end()) {
				if (debug_)
					cout << TIME
						 << " This packet is already stored in the buffer "
						 << endl;
			} else {
				// its a new packet for the destination
				++seq_no_counter;
				initPkt(p, idSrc, seq_no_counter);
				if (sendBuffer.size() < send_buffer_size) {
					pkt_store_info = new UWTPPktStoreInfo;
					pkt_store_info->setPktStoreTime(TIME);
					pkt_store_info->setPktPnt(p);
					pkt_store_info->setPktTxInfo(FALSE);

					sendBuffer.insert(
							make_pair(make_pair(uwtpdh->getDport(), ch->uid()),
									pkt_store_info));
					if (debug_) {
						for (map<UWTPPair, UWTPPktStoreInfo *>::iterator it =
										sendBuffer.begin();
								it != sendBuffer.end();
								it++) {
							cout << TIME << "port no " << (it->first).first
								 << "; seq no " << (it->first).second
								 << "; pkt store time "
								 << (it->second)->getPktStoreTime()
								 << "; pkt pointer "
								 << (it->second)->getPktPnt() << endl;
						}
					}

				} else {
					double highest_waiting_time = 0;
					int port_, sno_;
					for (map<UWTPPair, UWTPPktStoreInfo *>::iterator it_p =
									sendBuffer.begin();
							it_p != sendBuffer.end();
							it_p++) {
						if (it_p->second->getTimeSpentInQueue() >
								highest_waiting_time) {
							highest_waiting_time =
									it_p->second->getTimeSpentInQueue();
							port_ = uwtpdh->getDport();
							sno_ = ch->uid();
						}
					}
					assert(port_ >= 0 && sno_ >= 0);
					sendBuffer.erase(make_pair(port_, sno_));
					pkt_store_info = new UWTPPktStoreInfo;
					pkt_store_info->setPktStoreTime(TIME);
					pkt_store_info->setPktPnt(p);
					pkt_store_info->setPktTxInfo(FALSE);
					sendBuffer.insert(
							make_pair(make_pair(uwtpdh->getDport(), ch->uid()),
									pkt_store_info));
				}

				for (map<UWTPPair, UWTPPktStoreInfo *>::iterator it_p =
								sendBuffer.begin();
						it_p != sendBuffer.end();
						it_p++) {
					if (it_p->second->getPktTxInfo() == FALSE) {
						it_p->second->setPktTxInfo(TRUE);
						Packet *curr_data_pkt =
								(it_p->second->getPktPnt())->copy();
						sendDown(curr_data_pkt);
					}
				}
			}
		}
	}
}

int
UWTP::assignPort(Module *m)
{
	int id = m->getId();

	// Check that the provided module has not been given a port before
	if (port_map.find(id) != port_map.end())
		return TCL_ERROR;

	int newport = ++portcounter;

	port_map[id] = newport;
	assert(id_map.find(newport) == id_map.end());
	id_map[newport] = id;
	assert(id_map.find(newport) != id_map.end());

	if (debug_)
		std::cout << "PortMap::assignPort() "
				  << " id=" << id << " port=" << newport
				  << " portcounter=" << portcounter << std::endl;
	return newport;
}

int
UWTP::getUWTPDataHSize()
{
	return sizeof(hdr_uwtp_data);
}

int
UWTP::getUWTPAckHSize()
{
	return sizeof(hdr_uwtp_ack);
}

int
UWTP::getUWTPNackHSize()
{
	return sizeof(hdr_uwtp_nack);
}
