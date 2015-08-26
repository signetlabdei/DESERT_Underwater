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
 * @file   uwgainfromdb.cc
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Implementation of UnderwaterGainFromDb class.
 * 
 */

#include "uwgainfromdb.h"
#include "uwip-module.h"

using namespace std;

static class UnderwaterGainFromDbClass : public TclClass {
public:
  UnderwaterGainFromDbClass() : TclClass("Module/UW/GAINFROMDB") {}
  TclObject* create(int, const char*const*) {
    return (new UnderwaterGainFromDb);
  }
} class_UnderwaterGainFromDb;

UnderwaterGainFromDb::UnderwaterGainFromDb() :
time_roughness_(1),
depth_roughness_(1),
distance_roughness_(1),
total_time_(1),
frequency_correction_factor_(1)
{
    bind("time_roughness_", &time_roughness_);
    bind("depth_roughness_", &depth_roughness_);
    bind("distance_roughness_", &distance_roughness_);
    bind("total_time_", &total_time_);
    bind("frequency_correction_factor_", &frequency_correction_factor_);
    bind_error("token_separator_", &token_separator_);
    token_separator_ = '\t';
    path_ = "";
}

int UnderwaterGainFromDb::command(int argc, const char*const* argv) {
    if (argc == 3) {
        if (strcasecmp(argv[1], "path") == 0) {
            string tmp_ = ((char *) argv[2]);
            path_ = new char[tmp_.length() + 1];
            strcpy(path_, tmp_.c_str());
            if (path_ == NULL) {
                fprintf(stderr, "Empty string for the path_ file name");
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }
    return UnderwaterPhysical::command(argc, argv);
} /* UnderwaterGainFromDb::command */

double UnderwaterGainFromDb::getPER(double _snr, int _nbits, Packet* p) {
    hdr_MPhy* ph = HDR_MPHY(p);
    
    double x_ = (ph->srcPosition)->getX();
    double y_ = (ph->srcPosition)->getY();
    double z_ = (ph->srcPosition)->getZ();
    double x_dst_ = (ph->dstPosition)->getX();
    double y_dst_ = (ph->dstPosition)->getY();
    double z_dst_ = (ph->dstPosition)->getZ();
    double depth_src_ = z_;
    double depth_dst_ = z_dst_;
    double dist_dst_ = sqrt((x_ - x_dst_)*(x_ - x_dst_) + (y_ - y_dst_)*(y_ - y_dst_));

    double gain_ = pow(10, (this->getGain(NOW, depth_src_, depth_dst_, dist_dst_) / 10));
    gain_ = gain_ * pow(frequency_correction_factor_, - dist_dst_);
    
    double snir_;
    if ((ph->Pn + ph->Pi) != 0) {
        snir_ = (ph->Pt * gain_) / (ph->Pn + ph->Pi);
    } else {
        snir_ = - INT_MAX;
        cerr << "ph->Pn + ph->Pi = 0!" << endl;
    }
    return UnderwaterPhysical::getPER(snir_, _nbits, p);
} /* UnderwaterGainFromDb::getPER */

void UnderwaterGainFromDb::setTimeRoughness(const int& _time) {
    assert(_time >= 0);
    time_roughness_ = _time;
    return;
} /* UnderwaterGainFromDb::setTimeRoughness */

void UnderwaterGainFromDb::setDepthRoughness(const int& _depth) {
    assert(_depth >= 0);
    depth_roughness_ = _depth;
    return;
} /* UnderwaterGainFromDb::setDepthRoughness */

void UnderwaterGainFromDb::setDistanceRoughness(const int& _distance) {
    assert(_distance >= 0);
    distance_roughness_ = _distance;
    return;
} /* UnderwaterGainFromDb::setDistanceRoughness */

void UnderwaterGainFromDb::setTotalTime(const int& _total_time) {
    assert(_total_time > 0);
    total_time_ = _total_time;
    return;
} /* UnderwaterGainFromDb::setTotalTime */

void UnderwaterGainFromDb::setFrequencyCorrectionFactor(const double& _frequency_correction_factor) {
    assert(_frequency_correction_factor > 0);
    frequency_correction_factor_ = _frequency_correction_factor;
    return;
} /* UnderwaterGainFromDb::setFrequencyCorrectionFactor */


double UnderwaterGainFromDb::getGain(const double& _time, const double& _source_depth, const double& _destination_depth, const double& _destination_distance) {
    assert(_time >= 0);
    assert(_source_depth <= 0);
    assert(_destination_depth <= 0);
    assert(_destination_distance >= 0);
    
    int time_filename_ = (int) ((int) ceil(_time) / ((int) time_roughness_) * (int) time_roughness_ % (int) total_time_);
    int source_depth_filename_ = (int) ((int) ceil(_source_depth * -1) / (int) depth_roughness_ * (int) depth_roughness_);
    if (source_depth_filename_ == 0)
        source_depth_filename_ = depth_roughness_;
    string file_name = this->createNameFile(time_filename_, source_depth_filename_);

    int line_index_ = (int) ((int) ceil(_destination_depth * -1) / (int) depth_roughness_);
    int column_index_ = (int) ((int) ceil(_destination_distance) / (int) distance_roughness_);
    double gain_ = this->retriveGainFromFile(file_name, line_index_, column_index_);

    return gain_;
} /* UnderwaterGainFromDb::getSnr */

double UnderwaterGainFromDb::retriveGainFromFile(const string& _file_name, const int& _row_index, const int& _column_index) const {
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
        cerr << "Impossible to open file " << _file_name << endl;
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
//    cout << "file:" << _file_name << ":column:" << _column_index << ":row:" << _row_index << ":gain:" << return_value_ << endl;
    
    delete[] tmp_;
    if (this->isZero(return_value_)) {
        return (- INT_MAX);
    } else {
        return return_value_ ;
    }
} /* UnderwaterGainFromDb::retriveSnrFromFile */

string UnderwaterGainFromDb::createNameFile(const int& _time, const int& _source_depth) {
    osstream_.clear();
    osstream_.str("");
    osstream_ << path_ << "/" << _time << "_" << _source_depth << ".txt";
    return osstream_.str();
} /* UnderwaterGainFromDb::createNameFile */
