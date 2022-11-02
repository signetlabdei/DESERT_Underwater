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
 * @file   uwphy-clmsg.h
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Definition of ClMsgUwPhy class.
 *
 */

#ifndef UWPHY_CLMSG_H
#define UWPHY_CLMSG_H

#include <clmessage.h>
#include <packet.h>

#define CLMSG_UWPHY_VERBOSITY (3)

#define CLMSG_UWPHY_NOT_VALID (-1)

#define CLMSG_UWPHY_TIME_NOT_VALID (-1)

#define CLMSG_UWPHY_STACK_ID_NOT_VALID (-1)

extern ClMessage_t CLMSG_UWPHY_TX_POWER;
extern ClMessage_t CLMSG_UWPHY_B_RATE;
extern ClMessage_t CLMSG_UWPHY_THRESH;
extern ClMessage_t CLMSG_UWPHY_LOSTPKT;
extern ClMessage_t CLMSG_UWPHY_TX_BUSY;

/**
* ClMsgUwPhy should be extended and used to ask to set or get a parameter of a specific phy.
* In addition, ClMsgUwPhy can be used from the phy to reply such a request.
**/
class ClMsgUwPhy : public ClMessage
{
public:

  enum ReqType
  {
    NOT_VALID = -1,
    SET_REQ,
    GET_REQ,
    SET_REPLY,
    GET_REPLY
  };

  /**
  * Broadcast constructor of the ClMsgUwPhy class
  **/
  ClMsgUwPhy(ClMessage_t type);

  /**
  * Unicast constructor of the ClMsgUwPhy class
  * @param int stack_id: id of the stack
  * @param dest_mod_id: id of the destination module
  **/
  ClMsgUwPhy(int stack_id, int dest_module_id, ClMessage_t type);

  /**
  * Copy constructor
  * @param const ClMsgUwPhy& msg: ClMsgUwPhy that has to be copied
  */
  ClMsgUwPhy(const ClMsgUwPhy& msg);

  /**
    * Destructor of the ClMsgUwPhy class
  **/
  virtual ~ClMsgUwPhy();
  
  /**
    * Copy method of the ClMsgUwPhy class, the specialization of the return value is 
    * intentional and it is allowed by c++ standard
    *
    * @return pointer to a copy of the current ClMsgUwPhy object
  **/
  virtual ClMsgUwPhy* copy();

  /**
  * method to set the request type
  * @param ReqType type: request type
  */
  void setReqType(ReqType type);

  /**
  * method to return the request type
  * @return req_type
  */
  ReqType getReqType();

    
protected:

  // receiver stack id (valid only for broadcast request)
  int stack_id; /* <id of the stack */
  ReqType req_type; /*< request type: either get, set, request or reply */

};


/**
* ClMsgUwPhyTxPwr should be and used to ask either to set or get the
* transmitting power of a specific phy.
* In addition, ClMsgUwPhyTxPwr can be used from the phy to reply such a request.
**/

class 	ClMsgUwPhyTxPwr : public ClMsgUwPhy
{
public:

  /**
  * Broadcast constructor of the ClMsgUwPhyTxPwr class
  **/
  ClMsgUwPhyTxPwr();

  /**
  * Unicast constructor of the ClMsgUwPhyTxPwr class
  * @param int stack_id: id of the stack
  * @param dest_mod_id: id of the destination module
  **/
  ClMsgUwPhyTxPwr(int stack_id, int dest_module_id);

  /**
  * Copy constructor
  * @param const ClMsgUwPhy& msg: ClMsgUwPhyTxPwr that has to be copied
  */
  ClMsgUwPhyTxPwr(const ClMsgUwPhyTxPwr& msg);

  /**
    * Destructor of the ClMsgUwPhyTxPwr class
  **/
  virtual ~ClMsgUwPhyTxPwr();

  /**
  * method to return the transmitting power
  * @return tx_power
  */
  double getPower();

  /**
  * method to set the transmitting power
  * @param double powr: power to set
  */
  void setPower(double powr);

private:

  double tx_power; /* < Transmission power. Its definition (W, dB, dB re uPa, ...) depends on the specific phy.*/
    
};


/**
* ClMsgUwPhyBRate should be and used to ask either to set or get the
* communication rate (can be bitrate or baudrate, depending on the phy) 
* of a specific phy.
* In addition, ClMsgUwPhyBRate can be used from the phy to reply such a request.
**/

class ClMsgUwPhyBRate : public ClMsgUwPhy
{
public:

  /**
  * Broadcast constructor of the ClMsgUwPhyBRate class
  **/
  ClMsgUwPhyBRate();

  /**
  * Unicast constructor of the ClMsgUwPhyBRate class
  * @param int stack_id: id of the stack
  * @param dest_mod_id: id of the destination module
  **/
  ClMsgUwPhyBRate(int stack_id, int dest_module_id);

  /**
  * Copy constructor
  * @param const ClMsgUwPhyBRate& msg: ClMsgUwPhyBRate that has to be copied
  */
  ClMsgUwPhyBRate(const ClMsgUwPhyBRate& msg);


  /**
    * Destructor of the ClMsgUwPhyTxPwr class
  **/
  virtual ~ClMsgUwPhyBRate();

  /**
  * method to return the transmitting rate
  * @return b_rate
  */
  double getBRate();

  /**
  * method to set the transmitting rate
  * @param double rate: rate to set
  */
  void setBRate(double rate);


private:

  double b_rate; /* < Communication threshold. Can be either the baud or the bit rate.*/
    
};


/**
* ClMsgUwPhyThresh should be and used to ask either to set or get the
* receiving threshold (can be SNR or receiving power, depending on the phy) 
* of a specific phy.
* In addition, ClMsgUwPhyThresh can be used from the phy to reply such a request.
**/
class ClMsgUwPhyThresh : public ClMsgUwPhy
{
public:

  /**
  * Broadcast constructor of the ClMsgUwPhyBRate class
  **/
  ClMsgUwPhyThresh();

  /**
  * Unicast constructor of the ClMsgUwPhyThresh class
  * @param int stack_id: id of the stack
  * @param dest_mod_id: id of the destination module
  **/
  ClMsgUwPhyThresh(int stack_id, int dest_module_id);

  /**
  * Copy constructor
  * @param const ClMsgUwPhyThresh& msg: ClMsgUwPhyThresh that has to be copied
  */
  ClMsgUwPhyThresh(const ClMsgUwPhyThresh& msg);


  /**
    * Destructor of the ClMsgUwPhyThresh class
  **/
  virtual ~ClMsgUwPhyThresh();

  /**
  * method to return the rx threshold
  * @return threshold
  */
  double getThresh();

  /**
  * method to set the rx threshold
  * @param double thresh: threshold to set
  */
  void setThresh(double thresh);


private:

  double threshold; /* < Receiving threshold. Can be either the rx_power or the SNR.*/
    
};

/**
* ClMsgUwPhyGetLostPkts should be used by a layer to ask the phy how many packets it discarded 
* from the beginning of the simulation.
* In addition, ClMsgUwPhyThresh is used from the phy to reply such a request.
**/
class ClMsgUwPhyGetLostPkts : public ClMsgUwPhy
{
public:

  /**
   * Broadcast constructor of the ClMsgUwPhyBRate class
   * @param control set to true if it refers to the control packets, 
   * false (default) for data packets
   * 
  */
  ClMsgUwPhyGetLostPkts(bool control = false);

  /**
   * Unicast constructor of the ClMsgUwPhyGetLostPkts class
   * @param int stack_id: id of the stack
   * @param dest_mod_id: id of the destination module
   * @param control set to true if it refers to the control packets, 
   * false (default) for data packets
   */
  ClMsgUwPhyGetLostPkts(int stack_id, int dest_module_id, bool control = false);

  /**
  * Copy constructor
  * @param const ClMsgUwPhyGetLostPkts& msg: ClMsgUwPhyGetLostPkts that has to be copied
  */
  ClMsgUwPhyGetLostPkts(const ClMsgUwPhyGetLostPkts& msg);


  /**
    * Destructor of the ClMsgUwPhyGetLostPkts class
  **/
  ~ClMsgUwPhyGetLostPkts();

  /**
  * method to return the number of packets lost by the phy.
  * @return the number of packets lost by the phy.
  */
  uint getLostPkts();

  /**
  * method to set the number of packets lost by the phy.
  * @param lost_pkt: number of packets lost by the phy.
  */
  void setLostPkts(uint lost_pkt);

  inline bool isControl() {return is_control;}


private:

  uint lost_packets; /* < Number of packets lost by the phy.*/
  bool is_control;
   
};

/**
* ClMsgUwPhyTxBusy should be and used to ask either to set or get the
* transmitting busy variable of a specific phy.
* In addition, ClMsgUwPhyTxBusy can be used from the phy to reply such a request.
**/

class 	ClMsgUwPhyTxBusy : public ClMsgUwPhy
{
public:

  /**
  * Broadcast constructor of the ClMsgUwPhyTxBusy class
  **/
  ClMsgUwPhyTxBusy();

  /**
  * Unicast constructor of the ClMsgUwPhyTxBusy class
  * @param int stack_id: id of the stack
  * @param dest_mod_id: id of the destination module
  **/
  ClMsgUwPhyTxBusy(int stack_id, int dest_module_id);

  /**
  * Copy constructor
  * @param const ClMsgUwPhy& msg: ClMsgUwPhyTxBusy that has to be copied
  */
  ClMsgUwPhyTxBusy(const ClMsgUwPhyTxBusy& msg);

  /**
    * Destructor of the  class
  **/
  ~ClMsgUwPhyTxBusy();

  /**
  * method to return the transmitting power
  * @return tx_busy
  */
  int getTxBusy();

  /**
  * method to set the transmitting busy variable
  * @param int busy: busy value to set
  */
  void setTxBusy(int powr);

  /**
  * method to return the transmitting power
  * @return tx_busy
  */
  int getGetOp();

  /**
  * method to return the transmitting power
  * @return tx_busy
  */
  void setGetOp(int);

private:

int tx_busy; /* < Transmission power. Its definition (W, dB, dB re uPa, ...) depends on the specific phy.*/
int getop; // To signal if mac just wants to read the value
    
};

#endif /* UWPHY_CLMSG_H  */
