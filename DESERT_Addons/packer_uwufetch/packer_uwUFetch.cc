//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file       packer_uwUFetch.cc
 * @author Loris Brolo
 * \version  1.0.0
 * \brief      Implementation of the class responsible to map the ns2 packet of UFETCH into a bit stream, and vice-versa.
 */

#include "packer_uwUFetch.h"

static class PackerUwUFetchClass : public TclClass {
public:

    PackerUwUFetchClass() : TclClass("NS2/MAC/uwUFetch/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packer_uwUFetch());
    }
} class_module_packerUwUFetch;

packer_uwUFetch::packer_uwUFetch() : packer(false),
t_bck_max_probe_Bits(0),
t_bck_min_probe_Bits(0),
n_cbeacon_tx_Bits(0),
t_bck_choice_sensor_Bits(0),
n_pck_sensor_want_tx_Bits(0),
mac_addr_sensor_polled_Bits(0),
n_pck_hn_want_tx_Bits(0),
max_cbeacon_tx_hn_Bits(0),
t_bck_max_rts_Bits(0),
t_bck_min_rts_Bits(0),
max_pck_want_rx_Bits(0),
num_DATA_pcks_Bits(0),
backoff_time_RTS_Bits(0),
num_DATA_pcks_MAX_rx_Bits(0),
mac_addr_HN_ctsed_Bits(0) {
    //Header of the BEACON packet
    bind("T_BCK_MAX_PROBE_", (int*) &t_bck_max_probe_Bits);
    bind("T_BCK_MIN_PROBE_", (int*) &t_bck_min_probe_Bits);
    bind("N_CBEACON_TX_", (int*) &n_cbeacon_tx_Bits);

    //Header of the PROBE packet
    bind("T_BCK_CHOICE_SENSOR_", (int*) &t_bck_choice_sensor_Bits);
    bind("N_PCK_SENSOR_WANT_TX_", (int*) &n_pck_sensor_want_tx_Bits);

    //Header of the POLL packet
    bind("MAC_ADDR_SENSOR_POLLED_", (int*) &mac_addr_sensor_polled_Bits);
    bind("N_PCK_HN_WANT_RX_", (int*) &n_pck_hn_want_tx_Bits);

    //Header of the CBEACON packet
    bind("MAX_CBEACON_TX_HN_", (int*) &max_cbeacon_tx_hn_Bits);

    //Header of the TRIGGER packet
    bind("T_BCK_MAX_RTS_", (int*) &t_bck_max_rts_Bits);
    bind("T_BCK_MIN_RTS_", (int*) &t_bck_min_rts_Bits);
    bind("N_PCK_AUV_WANT_RX_", (int*) &max_pck_want_rx_Bits);

    //Header of the RTS packet
    bind("NUM_DATA_PCKS_", (int*) &num_DATA_pcks_Bits);
    bind("BACKOFF_TIME_RTS_", (int*) &backoff_time_RTS_Bits);

    //Header of the CTS packet
    bind("NUM_DATA_PCKS_MAX_RX_", (int*) &num_DATA_pcks_MAX_rx_Bits);
    bind("MAC_ADRR_HN_CTSED_", (int*) &mac_addr_HN_ctsed_Bits);

    this->init();
} //end constructor of packer_uwUFetch class

packer_uwUFetch::~packer_uwUFetch() {

} //destructor of packer_uwUFetch class

void packer_uwUFetch::init() {
    if (debug_)
        std::cout << "Re-initialization of n_bits for the uwUFETCH packer " << std::endl;

    n_bits.clear();
    n_bits.assign(15, 0);
    //Header of the BEACON packet
    n_bits[T_BCK_MAX_PROBE] = t_bck_max_probe_Bits;
    n_bits[T_BCK_MIN_PROBE] = t_bck_min_probe_Bits;
    n_bits[N_CBEACON_TX] = n_cbeacon_tx_Bits;
    //Header of the PROBE packet
    n_bits[T_BCK_CHOICE_SENSOR] = t_bck_choice_sensor_Bits;
    n_bits[N_PCK_SENSOR_WANT_TX] = n_pck_sensor_want_tx_Bits;
    //Header of the POLL packet
    n_bits[MAC_ADDR_SENSOR_POLLED] = mac_addr_sensor_polled_Bits;
    n_bits[N_PCK_HN_WANT_RX] = n_pck_hn_want_tx_Bits;
    //Header of the CBEACON packet
    n_bits[MAX_CBEACON_TX_HN] = max_cbeacon_tx_hn_Bits;
    //Header of the TRIGGER packet
    n_bits[T_BCK_MAX_RTS] = t_bck_max_rts_Bits;
    n_bits[T_BCK_MIN_RTS] = t_bck_min_rts_Bits;
    n_bits[N_PCK_AUV_WANT_RX] = max_pck_want_rx_Bits;
    //Header of RTS packet
    n_bits[NUM_DATA_PCKS] = num_DATA_pcks_Bits;
    n_bits[BACKOFF_TIME_RTS] = backoff_time_RTS_Bits;
    //Header of the CTS packet
    n_bits[NUM_DATA_PCKS_MAX_RX] = num_DATA_pcks_MAX_rx_Bits;
    n_bits[MAC_ADDR_HN_CTSED] = mac_addr_HN_ctsed_Bits;

} //end init()

size_t packer_uwUFetch::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {

    hdr_cmn* hcmn = HDR_CMN(p);

    if (hcmn->ptype() == PT_BEACON_UFETCH) {
        //Packet to be serialized is a BEACON
        hdr_BEACON_UFETCH* beaconh = HDR_BEACON_UFETCH(p);

        offset += put(buf, offset, &(beaconh->t_max_bc_), n_bits[T_BCK_MAX_PROBE]);
        offset += put(buf, offset, &(beaconh->t_min_bc_), n_bits[T_BCK_MIN_PROBE]);
        offset += put(buf, offset, &(beaconh->num_Max_CBEACON_tx_by_HN_), n_bits[N_CBEACON_TX]);

        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWUFETCH::BEACON packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    } else if (hcmn->ptype() == PT_PROBE_UFETCH) {
        //Packet to be serialized is a PROBE
        hdr_PROBE_UFETCH* probeh = HDR_PROBE_UFETCH(p);

        offset += put(buf, offset, &(probeh->backoff_time_PROBE_), n_bits[T_BCK_CHOICE_SENSOR]);
        offset += put(buf, offset, &(probeh->n_DATA_pcks_Node_tx_), n_bits[N_PCK_SENSOR_WANT_TX]);

        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWUFETCH::PROBE packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    } else if (hcmn->ptype() == PT_POLL_UFETCH) {
        //Packet to be serialized is a POLL
        hdr_POLL_UFETCH* pollh = HDR_POLL_UFETCH(p);

        offset += put(buf, offset, &(pollh->mac_addr_Node_polled_), n_bits[MAC_ADDR_SENSOR_POLLED]);
        offset += put(buf, offset, &(pollh->num_DATA_pcks_MAX_rx_), n_bits[N_PCK_HN_WANT_RX]);

        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWUFETCH::POLL packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    } else if (hcmn->ptype() == PT_CBEACON_UFETCH) {
        //Packet to be serialized is a CBEACON
        hdr_CBEACON_UFETCH* cbeaconh = HDR_CBEACON_UFETCH(p);

        offset += put(buf, offset, &(cbeaconh->num_Max_CBEACON_tx_by_HN_), n_bits[MAX_CBEACON_TX_HN]);
        offset += put(buf, offset, &(cbeaconh->t_max_bc_), n_bits[T_BCK_MAX_PROBE]);
        offset += put(buf, offset, &(cbeaconh->t_min_bc_), n_bits[T_BCK_MIN_PROBE]);

        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWUFETCH::CBEACON packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    } else if (hcmn->ptype() == PT_TRIGGER_UFETCH) {
        //Packet to be serialized is a TRIGGER
        hdr_TRIGGER_UFETCH* triggerh = HDR_TRIGGER_UFETCH(p);

        offset += put(buf, offset, &(triggerh->t_max_), n_bits[T_BCK_MAX_RTS]);
        offset += put(buf, offset, &(triggerh->t_min_), n_bits[T_BCK_MIN_RTS]);
        offset += put(buf, offset, &(triggerh->max_pck_want_rx_), n_bits[N_PCK_AUV_WANT_RX]);
        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWUFETCH::TRIGGER packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    } else if (hcmn->ptype() == PT_RTS_UFETCH) {
        //Packet to be serialized is a RTS
        hdr_RTS_UFETCH* rtsh = HDR_RTS_UFETCH(p);

        offset += put(buf, offset, &(rtsh->backoff_time_RTS_), n_bits[BACKOFF_TIME_RTS]);
        offset += put(buf, offset, &(rtsh->num_DATA_pcks_), n_bits[NUM_DATA_PCKS]);

        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWUFETCH::RTS packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    } else if (hcmn->ptype() == PT_CTS_UFETCH) {
        //Packet to be serialized is a CTS
        hdr_CTS_UFETCH* ctsh = HDR_CTS_UFETCH(p);

        offset += put(buf, offset, &(ctsh->mac_addr_HN_ctsed_), n_bits[MAC_ADDR_HN_CTSED]);
        offset += put(buf, offset, &(ctsh->num_DATA_pcks_MAX_rx_), n_bits[NUM_DATA_PCKS_MAX_RX]);

        if (debug_) {
            std::cout << "\033[1;37;45m (TX) UWUFETCH::CTS packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }

    return offset;
} //end packMyHeader()

size_t packer_uwUFetch::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {

    hdr_cmn* hcmn = HDR_CMN(p);

    if (hcmn->ptype() == PT_BEACON_UFETCH) {
        //Packet to be serialized is a BEACON
        hdr_BEACON_UFETCH* beaconh = HDR_BEACON_UFETCH(p);

        memset(&(beaconh->t_max_bc_), 0, sizeof (beaconh->t_max_bc_));
        offset += get(buf, offset, &(beaconh->t_max_bc_), n_bits[T_BCK_MAX_PROBE]);

        memset(&(beaconh->t_min_bc_), 0, sizeof (beaconh->t_min_bc_));
        offset += get(buf, offset, &(beaconh->t_min_bc_), n_bits[T_BCK_MIN_PROBE]);

        memset(&(beaconh->num_Max_CBEACON_tx_by_HN_), 0, sizeof (beaconh->num_Max_CBEACON_tx_by_HN_));
        offset += get(buf, offset, &(beaconh->num_Max_CBEACON_tx_by_HN_), n_bits[N_CBEACON_TX]);

        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWUFETCH::BEACON packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    } else if (hcmn->ptype() == PT_PROBE_UFETCH) {
        //Packet to be serialized is a PROBE
        hdr_PROBE_UFETCH* probeh = HDR_PROBE_UFETCH(p);

        memset(&(probeh->backoff_time_PROBE_), 0, sizeof (probeh->backoff_time_PROBE_));
        offset += get(buf, offset, &(probeh->backoff_time_PROBE_), n_bits[T_BCK_CHOICE_SENSOR]);

        memset(&(probeh->n_DATA_pcks_Node_tx_), 0, sizeof (probeh->n_DATA_pcks_Node_tx_));
        offset += get(buf, offset, &(probeh->n_DATA_pcks_Node_tx_), n_bits[N_PCK_SENSOR_WANT_TX]);

        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWUFETCH::PROBE packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    } else if (hcmn->ptype() == PT_POLL_UFETCH) {
        //Packet to be serialized is a POLL
        hdr_POLL_UFETCH* pollh = HDR_POLL_UFETCH(p);

        memset(&(pollh->mac_addr_Node_polled_), 0, sizeof (pollh->mac_addr_Node_polled_));
        offset += get(buf, offset, &(pollh->mac_addr_Node_polled_), n_bits[MAC_ADDR_SENSOR_POLLED]);

        memset(&(pollh->num_DATA_pcks_MAX_rx_), 0, sizeof (pollh->num_DATA_pcks_MAX_rx_));
        offset += get(buf, offset, &(pollh->num_DATA_pcks_MAX_rx_), n_bits[N_PCK_HN_WANT_RX]);

        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWUFETCH::POLL packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }

    } else if (hcmn->ptype() == PT_CBEACON_UFETCH) {
        //Packet to be serialized is a CBEACON
        hdr_CBEACON_UFETCH* cbeaconh = HDR_CBEACON_UFETCH(p);

        memset(&(cbeaconh->num_Max_CBEACON_tx_by_HN_), 0, sizeof (cbeaconh->num_Max_CBEACON_tx_by_HN_));
        offset += get(buf, offset, &(cbeaconh->num_Max_CBEACON_tx_by_HN_), n_bits[MAX_CBEACON_TX_HN]);

        memset(&(cbeaconh->t_max_bc_), 0, sizeof (cbeaconh->t_max_bc_));
        offset += get(buf, offset, &(cbeaconh->t_max_bc_), n_bits[T_BCK_MAX_PROBE]);

        memset(&(cbeaconh->t_min_bc_), 0, sizeof (cbeaconh->t_min_bc_));
        offset += get(buf, offset, &(cbeaconh->t_min_bc_), n_bits[T_BCK_MIN_PROBE]);

        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWUFETCH::CBEACON packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    } else if (hcmn->ptype() == PT_TRIGGER_UFETCH) {
        //Packet to be serialized is a TRIGGER
        hdr_TRIGGER_UFETCH* triggerh = HDR_TRIGGER_UFETCH(p);

        memset(&(triggerh->t_max_), 0, sizeof (triggerh->t_max_));
        offset += get(buf, offset, &(triggerh->t_max_), n_bits[T_BCK_MAX_RTS]);

        memset(&(triggerh->t_min_), 0, sizeof (triggerh->t_min_));
        offset += get(buf, offset, &(triggerh->t_min_), n_bits[T_BCK_MIN_RTS]);

        memset(&(triggerh->max_pck_want_rx_), 0, sizeof (triggerh->max_pck_want_rx_));
        offset += get(buf, offset, &(triggerh->max_pck_want_rx_), n_bits[N_PCK_AUV_WANT_RX]);

        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWUFETCH::TRIGGER packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    } else if (hcmn->ptype() == PT_RTS_UFETCH) {
        //Packet to be serialized is a RTS
        hdr_RTS_UFETCH* rtsh = HDR_RTS_UFETCH(p);

        memset(&(rtsh->backoff_time_RTS_), 0, sizeof (rtsh->backoff_time_RTS_));
        offset += get(buf, offset, &(rtsh->backoff_time_RTS_), n_bits[BACKOFF_TIME_RTS]);

        memset(&(rtsh->num_DATA_pcks_), 0, sizeof (rtsh->num_DATA_pcks_));
        offset += get(buf, offset, &(rtsh->num_DATA_pcks_), n_bits[NUM_DATA_PCKS]);

        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWUFETCH::RTS packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    } else if (hcmn->ptype() == PT_CTS_UFETCH) {
        //Packet to be serialized is a CTS
        hdr_CTS_UFETCH* ctsh = HDR_CTS_UFETCH(p);

        memset(&(ctsh->mac_addr_HN_ctsed_), 0, sizeof (ctsh->mac_addr_HN_ctsed_));
        offset += get(buf, offset, &(ctsh->mac_addr_HN_ctsed_), n_bits[MAC_ADDR_HN_CTSED]);

        memset(&(ctsh->num_DATA_pcks_MAX_rx_), 0, sizeof (ctsh->num_DATA_pcks_MAX_rx_));
        offset += get(buf, offset, &(ctsh->num_DATA_pcks_MAX_rx_), n_bits[NUM_DATA_PCKS_MAX_RX]);

        if (debug_) {
            std::cout << "\033[1;32;40m (RX) UWUFETCH::CTS packer hdr \033[0m" << std::endl;
            printMyHdrFields(p);
        }
    }
    return offset;
} //end unpackMyHdr()

void packer_uwUFetch::printMyHdrMap() {
    std::cout << "\033[1;37;45m Packer Name \033[0m: UWUFETCH \n";
    std::cout << "** BEACON fields:\n";
    std::cout << "\033[1;37;45m Field: T_BCK_MAX_PROBE: \033[0m:" << n_bits[T_BCK_MAX_PROBE] << " bits\n";
    std::cout << "\033[1;37;45m Field: T_BCK_MIN_PROBE: \033[0m:" << n_bits[T_BCK_MIN_PROBE] << " bits\n";
    std::cout << "\033[1;37;45m Field: N_CBEACON_TX: \033[0m:" << n_bits[N_CBEACON_TX] << " bits\n";

    std::cout << "** PROBE fields:\n";
    std::cout << "\033[1;37;45m Field: T_BCK_CHOICE_SENSOR: \033[0m:" << n_bits[T_BCK_CHOICE_SENSOR] << " bits\n";
    std::cout << "\033[1;37;45m Field: N_PCK_SENSOR_WANT_TX: \033[0m:" << n_bits[N_PCK_SENSOR_WANT_TX] << " bits\n";

    std::cout << "** POLL fields:\n";
    std::cout << "\033[1;37;45m Field: MAC_ADDR_SENSOR_POLLED: \033[0m:" << n_bits[MAC_ADDR_SENSOR_POLLED] << " bits\n";
    std::cout << "\033[1;37;45m Field: N_PCK_HN_WANT_RX: \033[0m:" << n_bits[N_PCK_HN_WANT_RX] << " bits\n";

    std::cout << "** CBEACON fields:\n";
    std::cout << "\033[1;37;45m Field: MAX_CBEACON_TX_HN: \033[0m:" << n_bits[MAX_CBEACON_TX_HN] << " bits\n";
    std::cout << "\033[1;37;45m Field: T_BCK_MAX_PROBE: \033[0m:" << n_bits[T_BCK_MAX_PROBE] << " bits\n";
    std::cout << "\033[1;37;45m Field: T_BCK_MIN_PROBE: \033[0m:" << n_bits[T_BCK_MIN_PROBE] << " bits\n";

    std::cout << "** TRIGGER fields:\n";
    std::cout << "\033[1;37;45m Field: T_BCK_MAX_RTS: \033[0m:" << n_bits[T_BCK_MAX_RTS] << " bits\n";
    std::cout << "\033[1;37;45m Field: T_BCK_MIN_RTS: \033[0m:" << n_bits[T_BCK_MIN_RTS] << " bits\n";
    std::cout << "\033[1;37;45m Field: N_PCK_AUV_WANT_RX: \033[0m:" << n_bits[N_PCK_AUV_WANT_RX] << " bits\n";

    std::cout << "** RTS fields:\n";
    std::cout << "\033[1;37;45m Field: BACKOFF_TIME_RTS: \033[0m:" << n_bits[BACKOFF_TIME_RTS] << " bits\n";
    std::cout << "\033[1;37;45m Field: NUM_DATA_PCKS: \033[0m:" << n_bits[NUM_DATA_PCKS] << " bits\n";

    std::cout << "** CTS fields:\n";
    std::cout << "\033[1;37;45m Field: MAC_ADDR_HN_CTSED: \033[0m:" << n_bits[MAC_ADDR_HN_CTSED] << " bits\n";
    std::cout << "\033[1;37;45m Field: NUM_DATA_PCKS_MAX_RX: \033[0m:" << n_bits[NUM_DATA_PCKS_MAX_RX] << " bits\n";

    cout << std::endl; // Only at the end do we actually flush the buffer and print
} //end printMyHdrMap()

void packer_uwUFetch::printMyHdrFields(Packet* p) {
    //Print information packet serialized by the method packMyHdr
    hdr_cmn* hcmn = HDR_CMN(p);

    if (hcmn->ptype() == PT_BEACON_UFETCH) {
        //Packet to be serialized is a BEACON
        hdr_BEACON_UFETCH* beaconh = HDR_BEACON_UFETCH(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, T_BCK_MAX_PROBE: " << beaconh->t_max_bc_ << std::endl;
        std::cout << "\033[1;37;45m 2nd field \033[0m, T_BCK_MIN_PROBE: " << beaconh->t_min_bc_ << std::endl;
        std::cout << "\033[1;37;45m 3nd field \033[0m, N_CBEACON_TX: " << beaconh->num_Max_CBEACON_tx_by_HN_ << std::endl;

    } else if (hcmn->ptype() == PT_PROBE_UFETCH) {
        //Packet to be serialized is a PROBE
        hdr_PROBE_UFETCH* probeh = HDR_PROBE_UFETCH(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, T_BCK_CHOICE_SENSOR: " << probeh->backoff_time_PROBE_ << std::endl;
        std::cout << "\033[1;37;45m 2nd field \033[0m, N_PCK_SENSOR_WANT_TX: " << probeh->n_DATA_pcks_Node_tx_ << std::endl;

    } else if (hcmn->ptype() == PT_POLL_UFETCH) {
        //Packet to be serialized is a POLL
        hdr_POLL_UFETCH* pollh = HDR_POLL_UFETCH(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, MAC_ADDR_SENSOR_POLLED: " << pollh->mac_addr_Node_polled_ << std::endl;
        std::cout << "\033[1;37;45m 2nd field \033[0m, N_PCK_HN_WANT_RX: " << pollh->num_DATA_pcks_MAX_rx_ << std::endl;

    } else if (hcmn->ptype() == PT_CBEACON_UFETCH) {
        //Packet to be serialized is a CBEACON
        hdr_CBEACON_UFETCH* cbeaconh = HDR_CBEACON_UFETCH(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, MAX_CBEACON_TX_HN: " << cbeaconh->num_Max_CBEACON_tx_by_HN_ << std::endl;
        std::cout << "\033[1;37;45m 2st field \033[0m, T_BCK_MAX_PROBE: " << cbeaconh->t_max_bc_ << std::endl;
        std::cout << "\033[1;37;45m 3st field \033[0m, T_BCK_MIN_PROBE: " << cbeaconh->t_min_bc_ << std::endl;

    } else if (hcmn->ptype() == PT_TRIGGER_UFETCH) {
        //Packet to be serialized is a TRIGGER
        hdr_TRIGGER_UFETCH* triggerh = HDR_TRIGGER_UFETCH(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, T_BCK_MAX_RTS: " << triggerh->t_max_ << std::endl;
        std::cout << "\033[1;37;45m 2nd field \033[0m, T_BCK_MIN_RTS: " << triggerh->t_min_ << std::endl;
        std::cout << "\033[1;37;45m 3nd field \033[0m, N_PCK_AUV_WANT_RX: " << triggerh->max_pck_want_rx_ << std::endl;

    } else if (hcmn->ptype() == PT_RTS_UFETCH) {
        //Packet to be serialized is a RTS
        hdr_RTS_UFETCH* rtsh = HDR_RTS_UFETCH(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, BACKOFF_TIME_RTS: " << rtsh->backoff_time_RTS_ << std::endl;
        std::cout << "\033[1;37;45m 2nd field \033[0m, NUM_DATA_PCKS: " << rtsh->num_DATA_pcks_ << std::endl;

    } else if (hcmn->ptype() == PT_CTS_UFETCH) {
        //Packet to be serialized is a CTS
        hdr_CTS_UFETCH* ctsh = HDR_CTS_UFETCH(p);
        std::cout << "\033[1;37;45m 1st field \033[0m, MAC_ADDR_HN_CTSED: " << ctsh->mac_addr_HN_ctsed_ << std::endl;
        std::cout << "\033[1;37;45m 2nd field \033[0m, NUM_DATA_PCKS_MAX_RX: " << ctsh->num_DATA_pcks_MAX_rx_ << std::endl;
    }
} // end printMyHdrFields
