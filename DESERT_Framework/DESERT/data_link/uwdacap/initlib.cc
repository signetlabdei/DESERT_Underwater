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
//

/**
 * @file   data_link/uwdacap/initlib.cc
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the initializazions of the uwdacap libraries
 *
 */

#include <tclcl.h>
#include <sap.h>

#include "uw-mac-DACAP-alter.h"
// #include "uw-mac-DACAP-new.h"


extern EmbeddedTcl DacapInitTclCode;

packet_t PT_DACAP;

int hdr_dacap::offset_;

// packet_t PT_DACAP_NEW;

// int hdr_dacap_new::offset_;
/**
 * Class that describe the header of DACAP Header
 */
static class DACAPPktClass : public PacketHeaderClass {
public:
  /**
   * Constructor of the class
   */
  DACAPPktClass() : PacketHeaderClass("PacketHeader/DACAP", sizeof(hdr_dacap)) {
//     this->bind();
    bind_offset(&hdr_dacap::offset_);
  }
} class_dacap_pkt; 

// static class DACAPPktClassNew : public PacketHeaderClass {
// public:
//   DACAPPktClassNew() : PacketHeaderClass("PacketHeader/DACAP/New", sizeof(hdr_dacap_new)) {
// //     this->bind();
//     bind_offset(&hdr_dacap_new::offset_);
//   }
// } class_dacap_pkt_new; 

extern "C" int Uwdacap_Init()
{
  /*
   * Put here all the commands which must be execute when the library
   * is loaded (i.e. TCL script execution)  
   * Remember to return 0 if all is OK, otherwise return 1
  */
  DACAPPktClass* dacap_header_class = new DACAPPktClass;  // <---
  dacap_header_class->bind();	
  PT_DACAP = p_info::addPacket("DACAP");
  
//   DACAPPktClassNew* dacap_header_class_new = new DACAPPktClassNew;  // <---
//   dacap_header_class_new->bind(); 
//   PT_DACAP_NEW = p_info::addPacket("DACAP_NEW");
  
  DacapInitTclCode.load();
 
  return 0;
}

extern "C" int  Cygmiracledacap_Init()
{
  Uwdacap_Init();
}



