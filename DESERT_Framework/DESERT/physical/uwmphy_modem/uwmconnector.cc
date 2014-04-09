//
// Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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
 * @file uwmconnector.cc
 * @author Riccardo Masiero, Matteo Petrani, Ivano Calabrese
 * \version 2.0.0
 * \brief Implementation of the UWMconnector class.
 */

#include "uwmconnector.h"
#include "uwmdriver.h"

#include <cctype>


static void hexdump(std::string name, std::string str)
{
    int len = str.size();
    const char *data = str.c_str();

    std::cout << name << "[" << len << "]: " << std::hex;
    for (int i = 0; i < len; i++)
    {
        std::cout.fill('0');
        std::cout.width(2);
        std::cout << std::right << (int)data[i];

        if (std::isalnum(data[i]) || std::ispunct(data[i]))
            std::cout << "(" << data[i] << ")";
        std::cout << " ";
    }

    std::cout.width(0);
    std::cout << std::dec << std::endl;
}

static std::string hex2bin(std::string hex)
{
    size_t len = (hex.size() + 2) / 3;
    std::stringstream ss(std::stringstream::in | std::stringstream::out);
    std::string result;

    ss << hex;
    for (int i = 0; i < len; i++)
    {
        int code;
        ss >> std::hex >> code;
        result.push_back(code);
    }

    return result;
}


UWMconnector::UWMconnector(UWMdriver* pmDriver_, std::string pathToDevice_){
  
  // Members initialization
  pmDriver = pmDriver_;
  debug_ = pmDriver -> getDebug();
  pathToDevice = pathToDevice_;
  readingBuff = "";
  
  if (debug_ >= 2) {cout << this << ": in constructor of UWMconnector which points to driver: " << pmDriver << "\n";}
}

UWMconnector::~UWMconnector(){
    
    out.close();
    in.close();
}
		
		
void UWMconnector::closeConnection(){
}
		
std::string UWMconnector::readFromModem(){
  
  std::string return_str;
  
  if (!queueMsg.empty()){
      msgModem tmp_ = queueMsg.front();
      return_str = tmp_.msg_rx;
      if (debug_ >= 2)  { 
	  hexdump("UWMCONNECTOR::READ_FROM_MODEM::", return_str);
      }
      queueMsg.pop();
  }else {
      return_str = "";
  }
  
  return return_str;	 
}

//void UWMconnector::create_readingBuff(){
//        
//        if (debug_ >= 2)
//        {
//            cout << "UWMCONNECTOR::CREATE_READINGBUFF" << endl;
//        }
//        std::stringstream str("");
//        str << "readingBuff" << pmDriver -> getID();
//        str >> readingBuff;
//        out.open(readingBuff.c_str());
//	if (!out)
//	{
//		std::cerr << "WARNING: Opening error ( " << strerror(errno) << " ). It was not possible to create " << readingBuff << "\n";
//	}
//	out.close();
//}
