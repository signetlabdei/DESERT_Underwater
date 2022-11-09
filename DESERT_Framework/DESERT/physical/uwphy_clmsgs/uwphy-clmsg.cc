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

/**
 * @file   uwphy-clmsg.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of ClMsgUwPhy class.
 *
 */

#include "uwphy-clmsg.h"

ClMsgUwPhy::ClMsgUwPhy(ClMessage_t type)
: ClMessage(CLMSG_UWPHY_VERBOSITY, type),
  stack_id(CLMSG_UWPHY_STACK_ID_NOT_VALID),
  req_type(NOT_VALID)
{
}


ClMsgUwPhy::ClMsgUwPhy(int sid, int dest_module_id, ClMessage_t type)
: ClMessage(CLMSG_UWPHY_VERBOSITY, type, UNICAST, dest_module_id),
  stack_id(sid),
  req_type(NOT_VALID)
{
}

ClMsgUwPhy::ClMsgUwPhy(const ClMsgUwPhy& msg)
: ClMessage(msg),
  stack_id(CLMSG_UWPHY_STACK_ID_NOT_VALID)
{
  req_type = msg.req_type;
}

ClMsgUwPhy::~ClMsgUwPhy()
{
}

ClMsgUwPhy* ClMsgUwPhy::copy()
{
  return new ClMsgUwPhy(*this);
}

void ClMsgUwPhy::setReqType(ReqType type)
{
  req_type = type;
}

ClMsgUwPhy::ReqType ClMsgUwPhy::getReqType()
{
  return req_type;
}


ClMsgUwPhyTxPwr::ClMsgUwPhyTxPwr()
: ClMsgUwPhy(CLMSG_UWPHY_TX_POWER),
  tx_power(0)
{
}

ClMsgUwPhyTxPwr::ClMsgUwPhyTxPwr(int sid, int dest_module_id)
: ClMsgUwPhy(sid, dest_module_id,CLMSG_UWPHY_TX_POWER),
  tx_power(0)
{
}

ClMsgUwPhyTxPwr::ClMsgUwPhyTxPwr(const ClMsgUwPhyTxPwr& msg)
: ClMsgUwPhy(msg)
{
  tx_power = msg.tx_power;
}

ClMsgUwPhyTxPwr::~ClMsgUwPhyTxPwr()
{
}

double ClMsgUwPhyTxPwr::getPower()
{
	return tx_power;
}

void ClMsgUwPhyTxPwr::setPower(double power)
{
	tx_power = power;
}


ClMsgUwPhyBRate::ClMsgUwPhyBRate()
: ClMsgUwPhy(CLMSG_UWPHY_B_RATE),
  b_rate(0)
{
}

ClMsgUwPhyBRate::ClMsgUwPhyBRate(int sid, int dest_module_id)
: ClMsgUwPhy(sid, dest_module_id, CLMSG_UWPHY_B_RATE),
  b_rate(0)
{
}

ClMsgUwPhyBRate::ClMsgUwPhyBRate(const ClMsgUwPhyBRate& msg)
: ClMsgUwPhy(msg)
{
  b_rate = msg.b_rate;
}

ClMsgUwPhyBRate::~ClMsgUwPhyBRate()
{
}

double ClMsgUwPhyBRate::getBRate()
{
	return b_rate;
}

void ClMsgUwPhyBRate::setBRate(double rate)
{
	b_rate = rate;
}


ClMsgUwPhyThresh::ClMsgUwPhyThresh()
: ClMsgUwPhy(CLMSG_UWPHY_THRESH),
  threshold(0)
{
}

ClMsgUwPhyThresh::ClMsgUwPhyThresh(int sid, int dest_module_id)
: ClMsgUwPhy(sid, dest_module_id, CLMSG_UWPHY_THRESH),
  threshold(0)
{
}

ClMsgUwPhyThresh::ClMsgUwPhyThresh(const ClMsgUwPhyThresh& msg)
: ClMsgUwPhy(msg)
{
  threshold = msg.threshold;
}

ClMsgUwPhyThresh::~ClMsgUwPhyThresh()
{
}

double ClMsgUwPhyThresh::getThresh()
{
	return threshold;
}

void ClMsgUwPhyThresh::setThresh(double thresh)
{
	threshold = thresh;
}


ClMsgUwPhyTxBusy::ClMsgUwPhyTxBusy()
: ClMsgUwPhy(CLMSG_UWPHY_TX_BUSY),
  tx_busy(0),
  getop(0)
{
}

ClMsgUwPhyTxBusy::ClMsgUwPhyTxBusy(int sid, int dest_module_id)
: ClMsgUwPhy(sid, dest_module_id,CLMSG_UWPHY_TX_BUSY),
  tx_busy(0),
  getop(0)
{
}

ClMsgUwPhyTxBusy::ClMsgUwPhyTxBusy(const ClMsgUwPhyTxBusy& msg)
: ClMsgUwPhy(msg)
{
  tx_busy = msg.tx_busy;
}

ClMsgUwPhyTxBusy::~ClMsgUwPhyTxBusy()
{
}

int ClMsgUwPhyTxBusy::getTxBusy()
{
	return tx_busy;
}

void ClMsgUwPhyTxBusy::setTxBusy(int busy)
{
	tx_busy = busy;
}

int ClMsgUwPhyTxBusy::getGetOp()
{
	return getop;
}

void ClMsgUwPhyTxBusy::setGetOp(int go)
{
	getop = go;
}

ClMsgUwPhyGetLostPkts::ClMsgUwPhyGetLostPkts(bool control)
: ClMsgUwPhy(CLMSG_UWPHY_LOSTPKT)
, lost_packets(0)
, is_control(control)
{

}
ClMsgUwPhyGetLostPkts::ClMsgUwPhyGetLostPkts(int stack_id, int dest_module_id, bool control)
: ClMsgUwPhy(stack_id, dest_module_id, CLMSG_UWPHY_LOSTPKT)
, lost_packets(0)
{

}

ClMsgUwPhyGetLostPkts::ClMsgUwPhyGetLostPkts(const ClMsgUwPhyGetLostPkts& msg)
: ClMsgUwPhy(msg),
  lost_packets(0)
{

}

ClMsgUwPhyGetLostPkts::~ClMsgUwPhyGetLostPkts()
{

}

uint ClMsgUwPhyGetLostPkts::getLostPkts() 
{
  return lost_packets;
}

void ClMsgUwPhyGetLostPkts::setLostPkts(uint lost_pkt)
{
  lost_packets = lost_pkt;
}
