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
 * @file   uwphysicalfromdb.h
 * @author Giovanni Toso
 * @version 1.0.0
 *
 * \brief Definition of UnderwaterPhysicalfromdb class.
 *
 */

#ifndef UWPHYSICALFROMDB_H
#define UWPHYSICALFROMDB_H

#include <uwgainfromdb.h>

class UnderwaterPhysicalfromdb : public UnderwaterGainFromDb {

public:
    /**
     * Constructor of UnderwaterMPhyBpskDb class.
     */
    UnderwaterPhysicalfromdb();

    /**
     * Destructor of UnderwaterMPhyBpskDb class.
     */
    virtual ~UnderwaterPhysicalfromdb() {}

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
     * Return the Packet Error Rate of the packet p passed as input.
     *
     * @param _snr Not used
     * @param _nbits Length of the packet
     * @param p Packet
     *
     * @return Packet Error Rate of the packet p
     */
    virtual double getPER(double _snr, int _nbits, Packet* p);

    /**
     * Read from the gain.
     *
     * @param _time Timestamp (in s)
     * @param _source_depth Source depth (in m)
     * @param _destination_depth Destination depth (in m)
     * @param _destination_distance Destination distance (in m)
     *
     * @return Gain in dB.
     */
    virtual double getGain(const double& _time, const double& _source_depth, const double& _destination_depth, const double& _destination_distance);

    /**
     * Read from the self interference.
     *
     * @param _time Timestamp (in s)
     * @param _source_depth Source depth (in m)
     * @param _destination_depth Destination depth (in m)
     * @param _destination_distance Destination distance (in m)
     *
     * @return Self Interference in dB.
     */
    virtual double getSelfInterference(const double& _time, const double& _source_depth, const double& _destination_depth, const double& _destination_distance);

    /**
     * Create the name of the file to read.
     *
     * @param _time Timestamp (in s)
     * @param _source_depth Source depth (in m)
     * @param _destination_depth Destination depth (in m)
     * @param _destination_distance Destination distance (in m)
     */
    virtual string createNameFile(const char* _path, const int& _time, const int& _source_depth, const int& _tau_index);

    /**
     * Read from a file the value in a specific row - column.
     *
     * @param _file_name name of the file
     * @param _row_index index of the row
     * @param _column_index index of the column
     *
     * @return the value read
     */
    virtual double retrieveFromFile(const string& _file_name, const int& _row_index, const int& _column_index) const;

    /**
     * Set the line_index parameter.
     *
     * @param Line index to load in the file
     */
    inline void setTauIndex(const int& _tau_index) { tau_index = _tau_index; }

    /**
     * Return the value of the tau_index parameter.
     *
     * @return tau_index
     */
    inline const int& getTauIndex() const { return tau_index; }

private:
    char* path_gainmaps;          /**< Name of the trace file writter for the current node. */
    char* path_selfinterference;  /**< Name of the trace file writter for the current node. */
    int tau_index;                /**< Tau index to load in the file. */
};

#endif /* UWPHYSICALFROMDB_H  */
