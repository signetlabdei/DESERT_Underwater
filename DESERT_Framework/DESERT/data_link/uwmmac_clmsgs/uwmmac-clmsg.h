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
 * \brief Definition of ClMsgUwMmac class.
 *
 */

#ifndef UWMMAC_CLMSG_H
#define UWMMAC_CLMSG_H

#include <clmessage.h>
#include <packet.h>

#define CLMSG_UWMMAC_VERBOSITY (3)

#define CLMSG_UWMMAC_NOT_VALID (-1)

#define CLMSG_UWMMAC_TIME_NOT_VALID (-1)

#define CLMSG_UWMMAC_STACK_ID_NOT_VALID (-1)

extern ClMessage_t CLMSG_UWMMAC_ENABLE;

/**
* ClMsgUwMmac should be extended and used to ask to set or get a parameter of a specific phy.
* In addition, ClMsgUwMmac can be used from the phy to reply such a request.
**/
class ClMsgUwMmac : public ClMessage
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
  * Broadcast constructor of the ClMsgUwMmac class
  **/
  ClMsgUwMmac(ClMessage_t type);

  /**
  * Unicast constructor of the ClMsgUwMmac class
  * @param int stack_id: id of the stack
  * @param dest_mod_id: id of the destination module
  **/
  ClMsgUwMmac(int stack_id, int dest_module_id, ClMessage_t type);

  /**
  * Copy constructor
  * @param const ClMsgUwMmac& msg: ClMsgUwMmac that has to be copied
  */
  ClMsgUwMmac(const ClMsgUwMmac& msg);

  /**
    * Destructor of the ClMsgUwMmac class
  **/
  virtual ~ClMsgUwMmac();
  
  /**
    * Copy method of the ClMsgUwMmac class, the specialization of the return value is 
    * intentional and it is allowed by c++ standard
    *
    * @return pointer to a copy of the current ClMsgUwMmac object
  **/
  virtual ClMsgUwMmac* copy();

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
* ClMsgUwMmacEnable should be and used to ask either to set or get the
* transmitting power of a specific phy.
* In addition, ClMsgUwMmacEnable can be used from the phy to reply such a request.
**/

class ClMsgUwMmacEnable : public ClMsgUwMmac
{
public:

  /**
  * Broadcast constructor of the ClMsgUwMmacEnable class
  **/
  ClMsgUwMmacEnable();

  /**
  * Unicast constructor of the ClMsgUwMmacEnable class
  * @param int stack_id: id of the stack
  * @param dest_mod_id: id of the destination module
  **/
  ClMsgUwMmacEnable(int stack_id, int dest_module_id);

  /**
  * Copy constructor
  * @param const ClMsgUwMmac& msg: ClMsgUwMmacEnable that has to be copied
  */
  ClMsgUwMmacEnable(const ClMsgUwMmacEnable& msg);

  /**
    * Destructor of the ClMsgUwMmacEnable class
  **/
  ~ClMsgUwMmacEnable();

  /**
  * method to return the transmitting power
  * @return tx_power
  */
  double getEnable();

  /**
  * method to set the transmitting power
  * @param double powr: power to set
  */
  void disable();  

  /**
  * method to set the transmitting power
  * @param double powr: power to set
  */
  void enable();

private:

  bool enable_; /* < Transmission power. Its definition (W, dB, dB re uPa, ...) depends on the specific phy.*/
    
};


#endif /* UWMMAC_CLMSG_H  */
