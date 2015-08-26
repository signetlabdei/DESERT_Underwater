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
 * @file   uwphysicalfromdb.cc
 * @author Giovanni Toso
 * @version 1.0.1
 *
 * \brief Implementation of UnderwaterPhysicalfromdb class.
 *
 */

#include "uwphysicalfromdb.h"

static class UnderwaterPhysicalfromdbClass : public TclClass {
public:
    UnderwaterPhysicalfromdbClass() : TclClass("Module/UW/PHYSICALFROMDB") {}
    TclObject* create(int, const char*const*) {
        return (new UnderwaterPhysicalfromdb);
    }
} class_UnderwaterPhysicalfromdb;

UnderwaterPhysicalfromdb::UnderwaterPhysicalfromdb() :
tau_index(1)
{
    bind("tau_index_", &tau_index);
    path_gainmaps = "";
    path_selfinterference = "";
}

int UnderwaterPhysicalfromdb::command(int argc, const char*const* argv) {
    if (argc == 3) {
        if (strcasecmp(argv[1], "setPathGainmaps") == 0) {
            string tmp_ = ((char *) argv[2]);
            path_gainmaps = new char[tmp_.length() + 1];
            strcpy(path_gainmaps, tmp_.c_str());
            if (path_ == NULL) {
                fprintf(stderr, "Empty string for the path_gainmaps_ file name");
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "setPathSelfInterference") == 0) {
            string tmp_ = ((char *) argv[2]);
            path_selfinterference = new char[tmp_.length() + 1];
            strcpy(path_selfinterference, tmp_.c_str());
            if (path_ == NULL) {
                fprintf(stderr, "Empty string for the path_selfinterference_ file name");
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }
    return UnderwaterGainFromDb::command(argc, argv);
} /* UnderwaterPhysicalfromdb::command */


double UnderwaterPhysicalfromdb::getPER(double _snr, int _nbits, Packet* p) {
    hdr_MPhy* ph = HDR_MPHY(p);

    double x_ = (ph->srcPosition)->getX();
    double y_ = (ph->srcPosition)->getY();
    double z_ = (ph->srcPosition)->getZ();
    double x_dst_ = (ph->dstPosition)->getX();
    double y_dst_ = (ph->dstPosition)->getY();
    double z_dst_ = (ph->dstPosition)->getZ();
    double depth_src_ = z_;
    double depth_dst_ = z_dst_;
    double dist_dst_ = sqrt ((x_ - x_dst_)*(x_ - x_dst_) + (y_ - y_dst_)*(y_ - y_dst_));

    double gain_ = pow(10, (getGain(NOW, depth_src_, depth_dst_, dist_dst_) / 10));
    double self_interference_ = pow(10, (getSelfInterference(NOW, depth_src_, depth_dst_, dist_dst_) / 10));
    //gain_ = gain_ * pow(frequency_correction_factor_, - dist_dst_);

    double snir_;
    if ((ph->Pn + ph->Pi + self_interference_) != 0) {
        ph->Pi += self_interference_; // Add the self interference to the interference.
        snir_ = (ph->Pt * gain_) / (ph->Pn + ph->Pi);
    } else {
        snir_ = - INT_MAX;
    }
    return UnderwaterPhysical::getPER(snir_, _nbits, p);
}

double UnderwaterPhysicalfromdb::getGain(const double& _time, const double& _source_depth, const double& _destination_depth, const double& _destination_distance) {
    assert(_time >= 0);
    assert(_source_depth <= 0);
    assert(_destination_depth <= 0);
    assert(_destination_distance >= 0);

    // Quantize the input values
    int time_filename_ = static_cast<int> (static_cast<int> (floor (_time) / getTimeRoughness()) * getTimeRoughness() % getTotalTime());
#if __cplusplus > 199711L
    // C++11
    // The column index corresponds to the depth of the receiver
    int source_depth_filename_ = static_cast<int> (round ((_source_depth * -1) / getDepthRoughness()) * getDepthRoughness());
    // The line index is relative to the distance of the receiver
    int line_index_ = static_cast<int> (round (_destination_distance / getDistanceRoughness()));
    // The column index corresponds to the depth of the receiver
    int column_index_= static_cast<int> (round ((_destination_depth * -1) / getDepthRoughness()));
#else
    // Adding 0.5 and cast is equivalent to use round only because we are dealing with positive integers
    int source_depth_filename_ = static_cast<int> ((_source_depth * -1) / getDepthRoughness() + 0.5) * getDepthRoughness();
    int line_index_ = static_cast<int> (_destination_distance / getDistanceRoughness() + 0.5);
    int column_index_= static_cast<int> ((_destination_depth * -1) / getDepthRoughness() + 0.5);
#endif

    if (source_depth_filename_ == 0) {
        source_depth_filename_ = getDepthRoughness();
    }

    string file_name_ = createNameFile(path_gainmaps, time_filename_, source_depth_filename_, getTauIndex());

    return retrieveFromFile(file_name_, column_index_, line_index_);
} /* UnderwaterPhysicalfromdb::getGain */

double UnderwaterPhysicalfromdb::getSelfInterference(const double& _time, const double& _source_depth, const double& _destination_depth, const double& _destination_distance) {
    assert(_time >= 0);
    assert(_source_depth <= 0);
    assert(_destination_depth <= 0);
    assert(_destination_distance >= 0);

    // Quantize the input values.
    int time_filename_ = static_cast<int> (static_cast<int> (ceil (_time) / getTimeRoughness()) * getTimeRoughness() % getTotalTime());
    int source_depth_filename_ = static_cast<int> ((_source_depth * -1) / getDepthRoughness()) * getDepthRoughness();
    // The line index is relative to the distance of the receiver
    int column_index_= static_cast<int> ((_destination_depth * -1) / getDepthRoughness());
    // The column index corresponds to the depth of the receiver
    int line_index_ = static_cast<int> (_destination_distance / getDistanceRoughness());


    if (time_filename_ == 0) {
        time_filename_ = getTimeRoughness();
    }

    if (source_depth_filename_ == 0) {
        source_depth_filename_ = getDepthRoughness();
    }

    string file_name_ = createNameFile(path_selfinterference, time_filename_, source_depth_filename_, getTauIndex());

    return retrieveFromFile(file_name_, column_index_, line_index_);
} /* UnderwaterPhysicalfromdb::getSelfInterference */

double UnderwaterPhysicalfromdb::retrieveFromFile(const string& _file_name, const int& _row_index, const int& _column_index) const {
    int row_iterator_ = 0;
    int column_iterator_ = 0;
    ifstream input_file_;
    istringstream stm;
    string line_;
    string token_;
    string value_;
    double return_value_;

    char* tmp_ = new char[_file_name.length() + 1];
    strcpy(tmp_, _file_name.c_str());
    if (tmp_ == NULL) {
        std::cerr << "Impossible to open file " << _file_name << std::endl;
        fprintf(stderr, "Empty string for the file name");
    }

    input_file_.open(tmp_);
    if (input_file_.is_open()) {
        while (std::getline(input_file_, line_)) {
            row_iterator_++;
            if (row_iterator_ == _row_index) {
                break;
            }
        }
    } else {
        std::cerr << "Impossible to open file " << _file_name << std::endl;
    }

    istringstream iss(line_);
    while (getline(iss, token_, token_separator_)) {
        column_iterator_++;
        if (column_iterator_ == _column_index) {
            value_ = token_;
        }
    }

    stm.str(value_);
    stm >> return_value_;
    //std::cout << "file:" << _file_name << ":column:" << _column_index << ":row:" << _row_index << ":gain:" << return_value_ << std::endl;

    delete[] tmp_;
    if (this->isZero(return_value_)) {
        return (- INT_MAX);
    } else {
        return return_value_ ;
    }
} /* UnderwaterPhysicalfromdb::retriveFromFile */

string UnderwaterPhysicalfromdb::createNameFile(const char* _path, const int& _time, const int& _source_depth, const int& _tau_index) {
    osstream_.clear();
    osstream_.str("");
    osstream_ << _path << "/" << _time << "_" << _source_depth << "_" << _tau_index;
    return osstream_.str();
} /* UnderwaterPhysicalfromdb::createNameFile */

