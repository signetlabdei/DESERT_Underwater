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
 * @file   uwphysicaldb.cc
 * @author Giovanni Toso
 * @version 1.0.0
 * 
 * \brief Implementation of UnderwaterPhysicaldb class.
 * 
 */

#include "uwphysicaldb.h"

static class UnderwaterPhysicaldbClass : public TclClass {
public:
  UnderwaterPhysicaldbClass() : TclClass("Module/UW/PHYSICALDB") {}
  TclObject* create(int, const char*const*) {
    return (new UnderwaterPhysicaldb);
  }
} class_UnderwaterPhysicaldb;

UnderwaterPhysicaldb::UnderwaterPhysicaldb() {
    bind_error("token_separator_", &token_separator);
    path_ = "";
    token_separator = '\t';
}

int UnderwaterPhysicaldb::command(int argc, const char*const* argv) {
    if (argc == 3) {
        if (strcasecmp(argv[1], "addr") == 0) {
            ipAddr_ = static_cast<uint8_t>(atoi(argv[2]));
            if (ipAddr_ == 0) {
                fprintf(stderr, "0 is not a valid IP address");
                return TCL_ERROR;
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setCountry") == 0) {
            country = ((char *) argv[2]);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setModulation") == 0) {
            modulation = ((char *) argv[2]);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "addSnr") == 0) {
            snr.insert(snr.end(), strtod(argv[2], NULL));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "addSir") == 0) {
            sir.insert(sir.end(), strtod(argv[2], NULL));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "addOverlap") == 0) {
            overlap.insert(overlap.end(), strtod(argv[2], NULL));
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setPath") == 0) {
            path_ = (char*) (argv[2]);
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setInterference") == 0) {
            interference_ = dynamic_cast<uwinterference*> (TclObject::lookup(argv[2]));
            if(!interference_)
                return TCL_ERROR;
            return TCL_OK;
	}
    } else if (argc == 4) {
        if (strcasecmp(argv[1], "addRange") == 0) {
            uint8_t node_id_ = atoi(argv[2]);
            double range_ = atof(argv[3]);
            std::map<uint8_t, std::set<double> >::iterator it = range.find(node_id_);
            if (it == range.end()) {
                std::set<double> tmp_set_;
                tmp_set_.insert(range_);
                range.insert(pair<uint8_t, std::set<double> >(node_id_, tmp_set_));
            } else {
                it->second.insert(range_);
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "addTypeOfNode") == 0) {
            uint8_t node_id_ = atoi(argv[2]);
            string node_type_ = (char*) (argv[3]);
            std::map<uint8_t, string>::iterator it = type_of_node.find(node_id_);
            if (it == type_of_node.end()) {
                type_of_node.insert(pair<double, string>(node_id_, node_type_));
                return TCL_OK;
            } else if (it != type_of_node.end() && it->second != node_type_) {
                type_of_node.erase(it);
                type_of_node.insert(pair<double, string>(node_id_, node_type_));
                return TCL_OK;
            } else {
                std::fprintf(stderr, "You added the same node with the same type twice.");
                return TCL_ERROR;
            }
        } else if (strcasecmp(argv[1], "addRangeNum") == 0) {
        	string pathtype_ = (char*) (argv[2]);
        	uint8_t rangenum_ = atoi(argv[3]);
        	std::map<string, uint8_t >::iterator it = range_nums.find(pathtype_);
        	if (it == range_nums.end()) {
        		range_nums.insert(pair<string,uint8_t>(pathtype_,rangenum_));
        		return TCL_OK;
        	} else if (it != range_nums.end() && it->second != rangenum_) {
        		range_nums.erase(it);
        		range_nums.insert(pair<string,uint8_t>(pathtype_,rangenum_));
        		return TCL_OK;
        	} else {
                std::fprintf(stderr, "You added the same range_num already");
                return TCL_ERROR;
            }
        }
    }
    return UnderwaterPhysical::command(argc, argv);
} /* UnderwaterPhysicaldb::command */

void UnderwaterPhysicaldb::recv(Packet* p) {
    hdr_cmn *ch = HDR_CMN(p);
    hdr_MPhy *ph = HDR_MPHY(p);

    if (ch->direction() == hdr_cmn::UP) {
        ph->dstSpectralMask = getRxSpectralMask(p);
        ph->dstPosition = getPosition();
        ph->dstAntenna = getRxAntenna(p);

        assert(ph->dstSpectralMask);
        assert(ph->dstPosition);

        ph->Pr = getRxPower(p);

        p->txinfo_.RxPr = 0;
        p->txinfo_.CPThresh = 0;

        if (ph->Pr > 0) {
            ph->Pn = getNoisePower(p);

            if (interference_) {
                interference_->addToInterference(p);
            }

            ph->rxtime = NOW;
            ph->worth_tracing = true;

            if (isOn == true) {
                PacketEvent* pe = new PacketEvent(p);
                Scheduler::instance().schedule(&rxtimer, pe, ph->duration);

                startRx(p);
            } else {
                Packet::free(p);
            }
        } else {
            Packet::free(p);
        }
    } else { // Direction DOWN
        assert(isOn);

        ph->Pr = 0;
        ph->Pn = 0;
        ph->Pi = 0;
        ph->txtime = NOW;
        ph->rxtime = ph->txtime;

        ph->worth_tracing = false;

        ph->srcSpectralMask = getTxSpectralMask(p);
        ph->srcAntenna = getTxAntenna(p);
        ph->srcPosition = getPosition();
        ph->dstSpectralMask = 0;
        ph->dstPosition = 0;
        ph->dstAntenna = 0;
        ph->modulationType = getModulationType(p);
        ph->duration = getTxDuration(p);

        ph->Pt = getTxPower(p);

        assert(ph->srcSpectralMask);
        assert(ph->srcPosition);
        assert(ph->duration > 0);
        assert(ph->Pt > 0);

	ch->prev_hop_ = ipAddr_;  // Must be added to ensure compatibility with uw-al

        PacketEvent* pe = new PacketEvent(p->copy());
        Scheduler::instance().schedule(&txtimer, pe, ph->duration);

        startTx(p);
    }
}

void UnderwaterPhysicaldb::endRx(Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    hdr_MPhy* ph = HDR_MPHY(p);

    if (PktRx != 0) {
        if (PktRx == p) {
            double per_ni; // packet error rate due to noise and/or interference
            double per_n; // packet error rate due to noise only 

            int nbits = ch->size() * 8;
            double x = RNG::defaultrng()->uniform_double();
            per_n = getPERfromSNR(ph->Pr / ph->Pn, nbits, p);
            bool error_n = x <= per_n;
            bool error_ni = 0;
            
            if (!error_n) {
                double recv_power = ph->Pr;
                double duration = ph->duration;
                double endrx_time = ph->rxtime + duration;
                if (interference_) {
                    double interference_power_ = interference_->getInterferencePower(p);
                    double interference_start_time_ = interference_->getInitialInterferenceTime();
                    if (interference_power_ > 0) {
                        interf_val.first = recv_power / interference_power_;
                    } else {
                        interf_val.first = recv_power;
                    }
                    assert(duration > 0);
                    interf_val.second = ((endrx_time - interference_start_time_) / (duration));
                    per_ni = this->getPERfromSIR(interf_val.first, interf_val.second);
//                    std::cout << "\33[31;40m" << per_ni << "\33[0m" << std::endl;
                    x = RNG::defaultrng()->uniform_double();
                    error_ni = x <= per_ni;
                } else {
                    std::cerr << "No interference model provided!" << std::endl;
                    interf_val.first = 0;
                    interf_val.second = 0;
                    exit(1);
                }
            }
            
            if (time_ready_to_end_rx_ > Scheduler::instance().clock()) {
		Rx_Time_ = Rx_Time_ + ph->duration - time_ready_to_end_rx_ + Scheduler::instance().clock();
            } else {
                Rx_Time_ += ph->duration;
            }
            time_ready_to_end_rx_ = Scheduler::instance().clock() + ph->duration;
            Energy_Rx_ += consumedEnergyRx(ph->duration);

            ch->error() = error_ni || error_n;

            if (error_n) {
                incrErrorPktsNoise();
            } else if (error_ni) {
                incrErrorPktsInterf();
            }
            sendUp(p);
            PktRx = 0; // We can now sync onto another packet
        } else {
            dropPacket(p);
        }
    } else {
        dropPacket(p);
    }
} /* UnderwaterPhysicaldb::endRx */

double UnderwaterPhysicaldb::getPERfromSNR(const double& _snr, const int& _nbits, const Packet* p) {
    hdr_cmn* ch  = HDR_CMN(p);
    hdr_MPhy* ph = HDR_MPHY(p);
//    hdr_uwip* iph = HDR_UWIP(p);
    
    // Type of node.
    std::map<uint8_t, string>::const_iterator it = type_of_node.find(ch->prev_hop_);
    assert(it != type_of_node.end());
    const string type_prev_ = it->second;
    it = type_of_node.find(ipAddr_);
    assert(it != type_of_node.end());
    const string type_me_ = it->second;
    string type_ = type_prev_ + type_me_;
    string type_reversed_ = type_me_ + type_prev_;

	// Modification to allow LUT use when type-pairs are missing
	std::map<string, uint8_t>::const_iterator it_r = range_nums.find(type_);
	std::map<string, uint8_t>::const_iterator it_rr = range_nums.find(type_reversed_);
	if ((it_r != range_nums.end() && it_r->second == 0) || (it_rr != range_nums.end() && it_rr->second == 0)) {
		if (country == "NO") {
			if (type_ == "AA") {
				type_ = "AG";
			} else {
				type_ = "BG";
			}
		}
		else if (country == "SE") {
			if ( (type_ == "BG") || (type_ == "GB") || (type_ == "GG") ) {
				type_ = "BB";
			} else {
				type_ = "AB";
			}
		}
		else if (country == "NL") {
			if ( (type_ == "AB") || (type_ == "BA") ) {
				type_ = "AG";
			} else {
				type_ = "GG";
			}
		}
		else if (country == "IT") {
			type_ = "BB";
		}
	}

    // Nearest neighbor range.    
    const double x_ = (ph->srcPosition)->getX();
    const double y_ = (ph->srcPosition)->getY();
    const double z_ = (ph->srcPosition)->getZ();
    const double x_dst_ = (ph->dstPosition)->getX();
    const double y_dst_ = (ph->dstPosition)->getY();
    const double z_dst_ = (ph->dstPosition)->getZ();
    const double distance_ = sqrt((x_ - x_dst_)*(x_ - x_dst_) + (y_ - y_dst_)*(y_ - y_dst_) + (z_ - z_dst_)*(z_ - z_dst_));
    const double distance_miles_ = this->fromKmToMiles(distance_ / 1000);
    
    std::map<uint8_t, std::set<double> >::const_iterator it2 = range.find(ch->prev_hop_);
    double nn_range_;
    if (it2 != range.end()) {
        nn_range_ = this->getNearestNeighbor(it2->second, distance_miles_);
    } else {
//        if (ch->direction() == 1) {
//            std::cout << "\33[31;40m" << "DIR: UP" << "\33[0m" << std::endl;
//        } else if (ch->direction() == -1) {
//            std::cout << "\33[31;40m" << "DIR: DOWN" << "\33[0m" << std::endl;
//        }
//        std::cout << "\33[32;40m" << "ME:" << static_cast<int>(ipAddr_);
//        std::cout << "\33[33;40m" << " NXT:" << static_cast<int>(ch->next_hop_);
//        std::cout << "\33[34;40m" << " PRV:" << static_cast<int>(ch->prev_hop_);
//        std::cout << "\33[35;40m" << " DST:" << static_cast<int>(iph->daddr());
//        std::cout << "\33[36;40m" << " SRC:" << static_cast<int>(iph->saddr());
//        std::cout << "\33[0m" << std::endl;
//        std::cout << "\33[31;40m" << "TPE:" << type_ << "\33[0m" << std::endl;
        return 1;
//        std::cerr << "Empty range table" << std::endl;
//        exit(1);
    }
    
    // File's name.
    osstream.clear();
    osstream.str("");
    osstream << path_ << country << "_" << modulation << "_" << type_ << "_" << nn_range_;
    const string file_name_ = osstream.str();
    
    // Nearest neighbor snr.
    const double nn_snr_ = this->getNearestNeighbor(snr, 10 * log10(_snr));
    
    // Open the file and get the per that corresponds to the nn_snr_ value.
    const double per_ = this->retrievePerFromFile(file_name_, nn_snr_);

    return per_;
} /* UnderwaterPhysicaldb::getPERfromSNR */

double UnderwaterPhysicaldb::getPERfromSIR(const double& _sir, const double& _overlap) {

    // Nearest neighbor Overlap.
    const double nn_overlap_ = this->getNearestNeighbor(overlap, _overlap * 100); // From [0; 1]  to [0; 100] scale.
    
    // File's name.
    osstream.clear();
    osstream.str("");
    osstream << path_ << "SIR" << "_" << modulation << "_" << nn_overlap_;
    const string file_name_ = osstream.str();
    
    // Nearest neighbor SIR.
    const double nn_sir_ = this->getNearestNeighbor(sir, _sir);
    
    // Open the file and get the per that corresponds to the nn_snr_ value.
    const double per_ = this->retrievePerFromFile(file_name_, nn_sir_);

    return per_;
} /* UnderwaterPhysicaldb::getPERfromSNR */


double UnderwaterPhysicaldb::getNearestNeighbor(const std::set<double>& _set, const double& _value) {
    // Check if the value to search for is smaller than the min or greater than the max.
    std::set<double>::iterator it_ = _set.begin();
    if (_value <= *it_) {
        return *it_;
    }
    it_ = --_set.end();
    if (_value >= *it_) {
        return *it_;
    } else {
        for (std::set<double>::const_iterator it_ = _set.begin(); it_ != _set.end(); it_++) {
            std::set<double>::const_iterator it_next_ = it_;
            it_next_++;
            if (_value >= *it_next_) {
                continue;
            } else {
                if (abs(_value - *it_) <= abs(_value - *it_next_)) {
                    return *it_;
                } else {
                    return *it_next_;
                }
            }
        }
        return 0;
    }
} /* UnderwaterPhysicaldb::findNearestNeightbor */

const double UnderwaterPhysicaldb::retrievePerFromFile(const std::string& _file_name, const double& _snr) const {
    std::ifstream input_file_;
    std::string line_;
    std::string token_;

    // Convert the file name to a char*.
    char* tmp_ = new char[_file_name.length() + 1];
    strcpy(tmp_, _file_name.c_str());
    if (tmp_ == NULL) {
        fprintf(stderr, "Empty string for the file name");
    }

    // Open the file, scan for the line and get the per.
    input_file_.open(tmp_);
    if (input_file_.is_open()) {
        while (std::getline(input_file_, line_)) {
            std::istringstream iss_(line_);
            getline(iss_, token_, token_separator);
            std::stringstream ss_(token_);
            double token_double_;
            ss_ >> token_double_;
            if (token_double_ == _snr) {
                getline(iss_, token_, token_separator);
                delete[] tmp_;
                std::stringstream ss_(token_);
                ss_ >> token_double_;
                return token_double_;
            }
        }
    } else {
        cerr << "Impossible to open file " << _file_name << endl;
        exit(1);
    }
    delete[] tmp_;
    return (- INT_MAX);
}
