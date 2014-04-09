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
 * @file   uwgainfromdb.h
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Definition of UnderwaterGainFromDb class.
 * 
 */

#ifndef UWGAINFROMDB_H 
#define UWGAINFROMDB_H 

#include <uwphysical.h>

#include <packet.h>
#include <module.h>
#include <tclcl.h>

#include <fstream>
#include <sstream>

class UnderwaterGainFromDb : public UnderwaterPhysical {

public:
    /**
     * Constructor of UnderwaterMPhyBpskDb class.
     */
    UnderwaterGainFromDb();
    
    /**
     * Destructor of UnderwaterMPhyBpskDb class.
     */
    virtual ~UnderwaterGainFromDb() { }
    
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     */
    virtual int command(int, const char*const*);
    
protected:
    /**
     * Returns the packet error rate by using the length of a packet and the information contained in the packet (position
     * of the source and the destiantion.
     * 
     * @param snr Calculated by nsmiracle with the Urick model (unused).
     * @param nbits length in bit of the packet.
     * @param p Packet by witch the module gets information about source and destination.
     * @return PER of the packet passed as parameter.
     */
    virtual double getPER(double snr, int nbits, Packet* p);
    
    /**
     * Sets the time_roughness_ parameter.
     * 
     * @param Roughness of the temporal samples.
     */
    virtual void setTimeRoughness(const int&);
    
    /**
     * Sets the depth_roughness_ parameter.
     * 
     * @param Roughness of the depth samples.
     */
    virtual void setDepthRoughness(const int&);
    
    /**
     * Sets the distance_roughness_ parameter.
     * 
     * @param Roughness of the distance samples.
     */
    virtual void setDistanceRoughness(const int&);
    
    /**
     * Sets the total_time_ parameter.
     * 
     * @param Maximum value of the temporal samples.
     */
    virtual void setTotalTime(const int&);

    /**
     * Returns the time_roughness_ parameter.
     * 
     * @return time_roughness_
     */
    inline const int& getTimeRoughness() const { return time_roughness_; }
    
    /**
     * Returns the depth_roughness_ parameter.
     * 
     * @return depth_roughness_
     */
    inline const int& getDepthRoughness() const { return depth_roughness_; }
    
    /**
     * Returns the distance_roughness_ parameter.
     * 
     * @return distance_roughness_
     */
    inline const int& getDistanceRoughness() const { return distance_roughness_; }
    
    /**
     * Returns the total_time_ parameter.
     * 
     * @return total_time_
     */
    inline const int& getTotalTime() const { return total_time_; }

    /**
     * 
     * @param 
     * @param 
     * @param 
     * @param 
     * @return 
     */
    virtual double getGain(const double&, const double&, const double&, const double&);
    
    /**
     * 
     * @param 
     * @param 
     * @param 
     * @return 
     */
    virtual double retriveGainFromFile(const string&, const int&, const int&) const;
    
    /**
     * Creates the name of the file to load.
     * 
     * @param time
     * @param depth
     * @return The name of the file to load.
     */
    virtual string createNameFile(const int&, const int&);
    
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
    inline const bool isZero(const double& value) const { return std::fabs(value) < std::numeric_limits<double>::epsilon(); }
    
    char* path_;                /**< Name of the trace file writter for the current node. */
    
private:
    // Variables
    int time_roughness_;        /**< Roughness of the temporal samples. */
    int depth_roughness_;       /**< Roughness of the depth samples. */
    int distance_roughness_;    /**< Roughness of the distance samples. */
    int total_time_;            /**< Maximum value of the temporal samples, after this limit the smilulation time will be reset to zero. */
    char token_separator_;      /**< Token used to parse the elements in a line of the database. */
    ostringstream osstream_;    /**< Used to create strings. */
    double frequency_correction_factor_; /**< used to shift from a frequency value to another one. */
};

#endif /* UWGAINFROMDB_H  */