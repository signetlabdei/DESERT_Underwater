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
 * @file   uwphysical.cc
 * @author Giovanni Toso and Federico Favaro
 * @version 1.1.0
 *
 * \brief Implementation of UnderwaterPhysical class
 *
 */

#include "uwphysical.h"

static class UwPhysicalClass : public TclClass {
public:
    UwPhysicalClass() : TclClass("Module/UW/PHYSICAL") {}
    TclObject* create(int, const char*const*) {
        return (new UnderwaterPhysical);
    }
} class_module_uwphysical;

UnderwaterPhysical::UnderwaterPhysical() :
modulation_name_("BPSK"),
time_ready_to_end_rx_(0),
Tx_Time_(0),
Rx_Time_(0),
Energy_Tx_(0),
Energy_Rx_(0),
tx_power_(3.3),
rx_power_(0.620),
Interference_Model("CHUNK")
{
    bind("rx_power_consumption_", &rx_power_);
    bind("tx_power_consumption_", &tx_power_);
}

int UnderwaterPhysical::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();

    if(argc == 2) {
        if(strcasecmp(argv[1],"getTxTime") == 0) {
            //return Get_Tx_Time();
            tcl.resultf("%f",Get_Tx_Time());
            return TCL_OK;
        }
        else if(strcasecmp(argv[1],"getRxTime") == 0) {
            //return Get_Rx_Time();
            tcl.resultf("%f",Get_Rx_Time());
            return TCL_OK;
        }
        else if(strcasecmp(argv[1],"getConsumedEnergyTx") == 0) {
            //return Get_Energy_Tx();
            tcl.resultf("%f",Get_Energy_Tx());
            return TCL_OK;
        }
        else if(strcasecmp(argv[1],"getConsumedEnergyRx") == 0) {
            //return Get_Energy_Rx();
            tcl.resultf("%f",Get_Energy_Rx());;
            return TCL_OK;
        }
        else if(strcasecmp(argv[1],"getTransmittedBytes") == 0) {
            //return Get_Energy_Rx();
            tcl.resultf("%f",Get_Transmitted_bytes());
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "getTotPktsLost") == 0) {
            tcl.resultf("%d", getTot_pkts_lost());
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "getCollisionsDATAvsCTRL") == 0) {
            tcl.resultf("%d", getCollisionsDATAvsCTRL());
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "getCollisionsCTRL") == 0) {
            tcl.resultf("%d", getCollisionsCTRL());
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "getCollisionsDATA") == 0) {
            tcl.resultf("%d", getCollisionsDATA());
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "getTotCtrlPktsLost") == 0) {
            tcl.resultf("%d", getTot_CtrlPkts_lost());
            return TCL_OK;
        }
        else if (strcasecmp(argv[1], "getErrorCtrlPktsInterf") == 0) {
            tcl.resultf("%d", getError_CtrlPktsInterf());
            return TCL_OK;
        }
    } else if (argc == 3) {
        if (strcasecmp(argv[1], "modulation") == 0) {
            modulation_name_ = ((char *) argv[2]);
            if (modulation_name_ == "" || (modulation_name_ != "BPSK" && modulation_name_ != "BFSK" && modulation_name_ != "8PSK" && modulation_name_ != "16PSK" && modulation_name_ != "32PSK")) {
                std::cerr << "Empty or wrong name for the modulation scheme" << std::endl;
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        else if(strcasecmp(argv[1],"setInterferenceModel") == 0) {
            Interference_Model = (string) argv[2];
            if (Interference_Model != "CHUNK" && Interference_Model != "MEANPOWER") {
                std::cerr <<  "Empty or wrong name of the Interference Model: CHUNK or MEANPOWER are valid interference models" << std::endl;
                return TCL_ERROR;
            }
            return TCL_OK;
        } else if (strcasecmp(argv[1], "setInterference") == 0) {
            interference_ = dynamic_cast<uwinterference*> (TclObject::lookup(argv[2]));
            if(!interference_) {
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }
    return UnderwaterMPhyBpsk::command(argc, argv);
} /* UnderwaterPhysical::command */

void UnderwaterPhysical::recv(Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    hdr_MPhy* ph = HDR_MPHY(p);

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

        PacketEvent* pe = new PacketEvent(p->copy());
        Scheduler::instance().schedule(&txtimer, pe, ph->duration);

        startTx(p);
    }
} /* UnderwaterPhysical::recv */

void UnderwaterPhysical::endTx(Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    hdr_MPhy* ph = HDR_MPHY(p);

    Tx_Time_   += ph->duration;
    Energy_Tx_ += consumedEnergyTx(ph->duration);

    Transmitted_bytes_ += ch->size();

    return UnderwaterMPhyBpsk::endTx(p);
} /* UnderwaterPhysical::endTx */

void UnderwaterPhysical::startRx(Packet* p) {
    hdr_mac* mach = HDR_MAC(p);
    hdr_MPhy* ph  = HDR_MPHY(p);

    static int mac_addr = -1;

    ClMsgPhy2MacAddr msg;
    sendSyncClMsg(&msg);
    mac_addr = msg.getAddr();

    if ((PktRx == 0) && (txPending == false)) {
        // The receiver is is not synchronized on any transmission
        // so we can sync on this packet
        double snr_dB = 10 * log10(ph->Pr / ph->Pn);
        if (debug_)
            std::cout << NOW << "  UnderwaterPhysical(" << mac_addr << ")::startRx() " << "snr_dB = " << snr_dB
                << "; AcquisitionThreshold_dB_ = " << getAcquisitionThreshold() << " pr " << 10 * log10(ph->Pr)
                << " pn " << 10 * log10(ph->Pn) << " end " << NOW + HDR_CMN(p)->txtime() << " src " << HDR_CMN(p)->prev_hop_
                << " dest " << HDR_CMN(p)->next_hop()
                << " size " << HDR_CMN(p)->size() << std::endl;
        if (snr_dB > getAcquisitionThreshold()) {
            if (ph->modulationType == modid) {
                // This is a BPSK packet so we sync on it
                PktRx = p;
                // Notify the MAC
                Phy2MacStartRx(p);
                if (debug_)
                    std::cout << NOW << "  UnderwaterPhysical(" << mac_addr << ")::startRx() " << " sync on PktRx = " << PktRx
                        << " end " << NOW + HDR_CMN(p)->txtime() << " src " << HDR_CMN(p)->prev_hop_
                        << " dest " << HDR_CMN(p)->next_hop()
                        << " size " << HDR_CMN(p)->size() << std::endl;
                return;
            }
            else {
                if (debug_)
                    std::cout << NOW << "  UnderwaterPhysical(" << mac_addr << ")::startRx() dropping pkt, wrong mod id" << std::endl;
                if ((mach->macDA() == mac_addr) && (mach->ftype() != MF_CONTROL)) {
                    incrTot_pkts_lost();
                } else if ((mach->macDA() == mac_addr) && (mach->ftype() == MF_CONTROL)) {
                    incrTotCrtl_pkts_lost();
                }
            }
        } else {
            if (debug_)
                std::cout << NOW << "  UnderwaterPhysical(" << mac_addr << ")::startRx() dropping pkt, below threshold" << std::endl;
            incrErrorPktsNoise();
            if (mach->ftype() != MF_CONTROL) {
                incrTot_pkts_lost();
            }
            else if (mach->ftype() == MF_CONTROL) {
                incrTotCrtl_pkts_lost();
            }
        }
    } else if (txPending == true) {
        if (debug_)
            std::cout << NOW << "  UnderwaterPhysical(" << mac_addr << ")::startRx() dropping pkt, tx pending" << std::endl;
        if (mach->ftype() != MF_CONTROL) {
            incrTot_pkts_lost();
        }
        if (mach->ftype() == MF_CONTROL) {
            incrTotCrtl_pkts_lost();
        }
    } else {
        if (mach->ftype() != MF_CONTROL) {
            incrTot_pkts_lost();
        }
        else if (mach->ftype() == MF_CONTROL) {
            incrTotCrtl_pkts_lost();
        }
    }
} /* UnderwaterPhysical::startRx */

void UnderwaterPhysical::endRx(Packet* p) {
    hdr_cmn* ch = HDR_CMN(p);
    hdr_MPhy* ph = HDR_MPHY(p);
    hdr_mac* mach = HDR_MAC(p);
    counter interferent_pkts;

    static int mac_addr = -1;

    ClMsgPhy2MacAddr msg;
    sendSyncClMsg(&msg);
    mac_addr = msg.getAddr();

    if (PktRx != 0) {
        if (PktRx == p) {
            double per_ni;

            int nbits = ch->size()*8;
            double x = RNG::defaultrng()->uniform_double();
            bool error_n = 0; //x <= per_n;
            bool error_ni = 0;
            double interference_power = 0;
            if (interference_) {
                if (Interference_Model == "CHUNK") {
                    const PowerChunkList& power_chunk_list = interference_->getInterferencePowerChunkList(p);
                    for (PowerChunkList::const_iterator itInterf = power_chunk_list.begin(); itInterf != power_chunk_list.end(); itInterf++) {
                        int nbits2 = itInterf->second * BitRate_;
				        interference_power = itInterf->first;
                        per_ni = getPER(ph->Pr / (ph->Pn + itInterf->first), nbits2, p);
                        x = RNG::defaultrng()->uniform_double();
                        error_ni = x <= per_ni;
                        if (error_ni) {
                            break;
                        }
                    }
                } else if (Interference_Model == "MEANPOWER") {
                    interference_power = interference_->getInterferencePower(p);
                    per_ni = getPER(ph->Pr / (ph->Pn + interference_power), nbits, p);
                    error_ni = x <= per_ni;
                } else {
                    std::cerr << "Please choose the right interference model to use: CHUNK or MEANPOWER" << std::endl;
                    exit(1);
                }
                interferent_pkts = interference_->getCounters(p);

            } else {
                interference_power = ph->Pi;
                per_ni = getPER(ph->Pr / (ph->Pn + ph->Pi), nbits, p);
                error_ni = x <= per_ni;
            }
	    
	        if (interference_power < (ph->Pn / 10))
            {
                error_n = error_ni;
                error_ni = 0; //error transfered on noise
            }
            if (time_ready_to_end_rx_ > Scheduler::instance().clock()) {
                Rx_Time_ = Rx_Time_ + ph->duration - time_ready_to_end_rx_ + Scheduler::instance().clock();
            } else {
                Rx_Time_ += ph->duration;
            }
            time_ready_to_end_rx_ = Scheduler::instance().clock() + ph->duration;
            Energy_Rx_ += consumedEnergyRx(ph->duration);
            ch->error() = error_n || error_ni;
            if (debug_) 
            {
                if (error_ni == 1) {
                    std::cout << NOW << "  UnderwaterPhysical(" << mac_addr << ")::endRx() packet " << ch->uid() << " contains errors due to noise and interference." << std::endl;
                } else if (error_n == 1) {
                    std::cout << NOW << "  UnderwaterPhysical(" << mac_addr << ")::endRx() packet " << ch->uid() << " contains errors due to noise." << std::endl;
                }
            }
            if (error_n) {
                incrErrorPktsNoise();
                if (mach->ftype() != MF_CONTROL) {
                    incrTot_pkts_lost();
                }
                else if (mach->ftype() == MF_CONTROL) {
                    incrTotCrtl_pkts_lost();
                }
            } else if (error_ni) {
                if (mach->ftype() != MF_CONTROL) {
                    incrErrorPktsInterf();
                    incrTot_pkts_lost();
                    if (interferent_pkts.second >= 1) {
                        incrCollisionDATA();
                    } else {
                        if (interferent_pkts.first > 0) {
                            incrCollisionDATAvsCTRL();
                        }
                    }
                }
                else if (mach->ftype() == MF_CONTROL) {
                    incrTotCrtl_pkts_lost();
                    incrErrorCtrlPktsInterf();
                    if (interferent_pkts.first > 0) {
                        incrCollisionCTRL();
                    }
                }

            }
            sendUp(p);
            PktRx = 0;
        } else {
            dropPacket(p);
        }
    } else {
        dropPacket(p);
    }
} /* UnderwaterPhysical::endRx */

double UnderwaterPhysical::getPER(double _snr, int _nbits, Packet* _p) {
    double snr_with_penalty = _snr * pow(10, RxSnrPenalty_dB_ / 10.0);

    double ber_ = 0;
    if (modulation_name_ == "BPSK") {
        ber_ = 0.5 * erfc(sqrt(snr_with_penalty));
    } else if (modulation_name_ == "BFSK") {
        ber_ = 0.5 * exp(-snr_with_penalty / 2);
    } else if (modulation_name_ == "8PSK") {
        double const M = 8;
        ber_ = (1 / this->log2(M)) * get_prob_error_symbol_mpsk(snr_with_penalty, M);
    } else if (modulation_name_ == "16PSK") {
        double const M = 16;
        ber_ = (1 / this->log2(M)) * get_prob_error_symbol_mpsk(snr_with_penalty, M);
    } else if (modulation_name_ == "32PSK") {
        double const M = 32;
        ber_ = (1 / this->log2(M)) * get_prob_error_symbol_mpsk(snr_with_penalty, M);
    }

    // PER calculation
    return 1 - pow(1 - ber_, _nbits);
} /* UnderwaterPhysical::getPER */

