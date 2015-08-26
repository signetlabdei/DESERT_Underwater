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
 * @file   clmsg-wakeUp-tone.h
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the implementation of the CrossLayerMessage that represent the WakeUP tone for data PHY layer
 *
 */

#ifndef TLOHI_WAKEUPTONE_CLMSG_H
#define TLOHI_WAKEUPTONE_CLMSG_H

#include <packet.h>
#include <clmessage.h>

#define TLOHI_WAKEUPTONE_VERBOSITY 3

extern ClMessage_t CLMSG_MAC2PHY_TXTONE;
/**
 * Class that represent the Crosslayer Service Access Point
 */
class ClSAP;

/**
 * Class that describe the Mac2PhyTxTone CrossLayer message
 */
class ClMsgMac2PhyTxTone : public ClMessage
{

public:
  /**
   * Constructor of the class
   */
  ClMsgMac2PhyTxTone();

  /**
   * Copy the message
   * @return pointer to the Message
   */
  ClMessage* copy();

  /**
   * Return the status of the message
   * @return status
   */
  bool getStatus();
  /**
   * Set the status of the message
   * @param bool status of the message
   */
  void setStatus(bool status);

private:

  bool done; /**< Flag that indicates whether the message has been sent */
};



#endif /* TLOHI_WAKEUPTONE_CLMSG_H */


