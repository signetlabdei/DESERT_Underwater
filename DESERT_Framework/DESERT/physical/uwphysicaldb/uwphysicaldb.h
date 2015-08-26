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
 * @file   uwphysicaldb.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Definition of UnderwaterPhysicaldb class.
 * 
 */

#ifndef UWPHYSICALDB_H 
#define UWPHYSICALDB_H 

#include <uwphysical.h>
#include <uwip-module.h>
#include "uwinterference.h"

#include <packet.h>
#include <module.h>
#include <tclcl.h>

#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <set>

namespace uwphysicaldb {
    static const double FROMKMTOMILES = 0.621371192237;
}


class UnderwaterPhysicaldb : public UnderwaterPhysical {

public:
    /**
     * Constructor of UnderwaterMPhyBpskDb class.
     */
    UnderwaterPhysicaldb();
    
    /**
     * Destructor of UnderwaterMPhyBpskDb class.
     */
    virtual ~UnderwaterPhysicaldb() { }
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int, const char*const*);
    
    virtual void recv(Packet*);
    
protected:
    
    virtual void endRx(Packet* p);
    
    /**
     * Packet error rate based on SNR.
     * 
     * @param SNR.
     * @param nbits length in bit of the packet.
     * @param p Packet.
     * @return PER of the packet retrieved from file.
     */
    virtual double getPERfromSNR(const double&, const int&, const Packet*);
    
    /**
     * Packet error rate based on Overlap and SIR.
     * 
     * @param Overlap.
     * @param SIR.
     * @return PER of the packet retrieved from file.
     */
    virtual double getPERfromSIR(const double&, const double&);
    
    /**
     * Nearest neighbor of a value contained in a set.
     * 
     * @param Set that contains the values in which to search.
     * @param value to search for.
     * @return Nearest neighbor.
     */
    virtual double getNearestNeighbor(const std::set<double>&, const double&);
    
    virtual const double retrievePerFromFile(const std::string&, const double&) const;
    
    /**
     * Evaluates is the number passed as input is equal to zero. When C++ works with
     * double and float number you can't compare them with 0. If the absolute
     * value of the number is smaller than eplison that means that the number is
     * equal to zero.
     * 
     * @param double& Number to evaluate.
     * @return <i>true</i> if the number passed in input is equal to zero, <i>false</i> otherwise.
     * @see std::numeric_limits<double>::epsilon()
     */
    inline const bool isZero(const double& _value) const { return std::fabs(_value) < std::numeric_limits<double>::epsilon(); }
    
    /**
     * Converts from kilometers to miles.
     * 
     * @param _km The value to be converted in miles.
     * @return The value converted in miles.
     */
    inline const double fromKmToMiles(const double& _km) const { return _km * uwphysicaldb::FROMKMTOMILES; }
    
    // Variables
    char* path_;                                   /**< Name of the trace file writter for the current node. */
    uwinterference* interference_;         /**< Interference Model. */
    std::set<double> snr;                          /**< Set of the available SNRs. */
    std::set<double> overlap;                      /**< Set of the available Overlaps. */
    std::set<double> sir;                          /**< Set of the available SIRs. */
    std::map<uint8_t, std::set<double> > range;    /**< Set of the available Ranges. */
    std::map<uint8_t, string> type_of_node;        /**< Set of the available type of nodes. */
	std::map<string, uint8_t> range_nums;		   /**< Set of the number of ranges for a given pathtype, e.g "AA". */
    string country;                                /**< Name of the Country. */
    string modulation;                             /**< Name of the Modulation. */
    uint8_t ipAddr_;                               /**< IP of the node. */
    
    std::pair<double, double> interf_val;          /**< (SIR, Overlap) */
    
    char token_separator;      /**< Token used to parse the elements in a line of the database. */
    
private:
    ostringstream osstream;
};

#endif /* UWPHYSICALDB_H  */
