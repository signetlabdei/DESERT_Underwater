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
 * @file packer-common.cpp
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief  Implementation of the class responsible to map the ns2 common header into a bit stream, and vice-versa.
 * @author Cristiano Tapparello
 * \version 1.5.0 
 */

#include "packer-common.h"

/**
 * Class to create the Otcl shadow object for an object of the class packer.
 */
static class PackerCOMMONClass : public TclClass {
public:

    PackerCOMMONClass() : TclClass("NS2/COMMON/Packer") {
    }

    TclObject* create(int, const char*const*) {
        return (new packerCOMMON());
    }
} class_module_packerCOMMON;

packerCOMMON::packerCOMMON() : packer(false) {

    PTYPE_Bits     = sizeof (packet_t);
    SIZE_Bits      = sizeof (int);
    UID_Bits       = sizeof (int);
    ERROR_Bits     = sizeof (int);
    TIMESTAMP_Bits = sizeof (double);
    DIRECTION_Bits = 0;
    PREV_HOP_Bits  = sizeof (nsaddr_t);
    NEXT_HOP_Bits  = sizeof (nsaddr_t);
    ADDR_TYPE_Bits = sizeof (int);
    LAST_HOP_Bits  = sizeof (nsaddr_t);
    TXTIME_Bits    = sizeof (double);

    errbitcnt_Bits    = 0;
    fecsize_Bits      = 0;
    iface_Bits        = 0;
    src_rt_valid_Bits = 0;
    ts_arr_Bits       = 0;
    aomdv_salvage_count_Bits = 0;
    xmit_failure_Bits        = 0;
    xmit_failure_data_Bits   = 0;
    xmit_reason_Bits         = 0;
    num_forwards_Bits        = 0;
    opt_num_forwards_Bits    = 0;

    bind("PTYPE_Bits", (int*) &PTYPE_Bits);
    bind("SIZE_Bits", (int*) &SIZE_Bits);
    bind("UID_Bits", (int*) &UID_Bits);
    bind("ERROR_Bits", (int*) &ERROR_Bits);
    bind("TIMESTAMP_Bits", (int*) &TIMESTAMP_Bits);
    bind("DIRECTION_Bits", (int*) &DIRECTION_Bits);
    bind("PREV_HOP_Bits", (int*) &PREV_HOP_Bits);
    bind("NEXT_HOP_Bits", (int*) &NEXT_HOP_Bits);
    bind("ADDR_TYPE_Bits", (int*) &ADDR_TYPE_Bits);
    bind("LAST_HOP_Bits", (int*) &LAST_HOP_Bits);
    bind("TXTIME_Bits", (int*) &TXTIME_Bits);

    bind("errbitcnt_Bits", (int*) &errbitcnt_Bits);
    bind("fecsize_Bits", (int*) &fecsize_Bits);
    bind("iface_Bits", (int*) &iface_Bits);
    bind("src_rt_valid_Bits", (int*) &src_rt_valid_Bits);
    bind("ts_arr_Bits", (int*) &ts_arr_Bits);
    bind("aomdv_salvage_count_Bits", (int*) &aomdv_salvage_count_Bits);
    bind("xmit_failure_Bits", (int*) &xmit_failure_Bits);
    bind("xmit_failure_data_Bits", (int*) &xmit_failure_data_Bits);
    bind("xmit_reason_Bits", (int*) &xmit_reason_Bits);
    bind("num_forwards_Bits", (int*) &num_forwards_Bits);
    bind("opt_num_forwards_Bits", (int*) &opt_num_forwards_Bits);

    if (debug_)
        cout << "Initialization (from constructor) of n_bits for the UWCOMMON packer " << endl;

    n_bits.clear();
    n_bits.assign(NUM_FIELDS,0); // NUM_FIELDS zeroes as below

    n_bits[PTYPE]               = PTYPE_Bits;
    n_bits[SIZE]                = SIZE_Bits;
    n_bits[UID]                 = UID_Bits;
    n_bits[ERROR]               = ERROR_Bits;
    n_bits[TIMESTAMP]           = TIMESTAMP_Bits;
    n_bits[DIRECTION]           = DIRECTION_Bits;
    n_bits[PREV_HOP]            = PREV_HOP_Bits;
    n_bits[NEXT_HOP]            = NEXT_HOP_Bits;
    n_bits[ADDR_TYPE]           = ADDR_TYPE_Bits;
    n_bits[LAST_HOP]            = LAST_HOP_Bits;
    n_bits[TXTIME]              = TXTIME_Bits;

    n_bits[errbitcnt_]           = errbitcnt_Bits;
    n_bits[fecsize_]             = fecsize_Bits;
    n_bits[iface_]               = iface_Bits;
    n_bits[src_rt_valid]         = src_rt_valid_Bits;
    n_bits[ts_arr_]              = ts_arr_Bits;
    n_bits[aomdv_salvage_count_] = aomdv_salvage_count_Bits;
    n_bits[xmit_failure_]        = xmit_failure_Bits;
    n_bits[xmit_failure_data_]   = xmit_failure_data_Bits;
    n_bits[xmit_reason_]         = xmit_reason_Bits;
    n_bits[num_forwards_]        = num_forwards_Bits;
    n_bits[opt_num_forwards_]    = opt_num_forwards_Bits;

}

packerCOMMON::~packerCOMMON() {

}

void packerCOMMON::init() {

    if (debug_)
        cout << "Re-initialization of n_bits for the UWCOMMON packer " << endl;

    n_bits.clear();
    n_bits.assign(NUM_FIELDS,0); // NUM_FIELDS zeroes as below

    n_bits[PTYPE]               = PTYPE_Bits;
    n_bits[SIZE]                = SIZE_Bits;
    n_bits[UID]                 = UID_Bits;
    n_bits[ERROR]               = ERROR_Bits;
    n_bits[TIMESTAMP]           = TIMESTAMP_Bits;
    n_bits[DIRECTION]           = DIRECTION_Bits;
    n_bits[PREV_HOP]            = PREV_HOP_Bits;
    n_bits[NEXT_HOP]            = NEXT_HOP_Bits;
    n_bits[ADDR_TYPE]           = ADDR_TYPE_Bits;
    n_bits[LAST_HOP]            = LAST_HOP_Bits;
    n_bits[TXTIME]              = TXTIME_Bits;

    n_bits[errbitcnt_]           = errbitcnt_Bits;
    n_bits[fecsize_]             = fecsize_Bits;
    n_bits[iface_]               = iface_Bits;
    n_bits[src_rt_valid]         = src_rt_valid_Bits;
    n_bits[ts_arr_]              = ts_arr_Bits;
    n_bits[aomdv_salvage_count_] = aomdv_salvage_count_Bits;
    n_bits[xmit_failure_]        = xmit_failure_Bits;
    n_bits[xmit_failure_data_]   = xmit_failure_data_Bits;
    n_bits[xmit_reason_]         = xmit_reason_Bits;
    n_bits[num_forwards_]        = num_forwards_Bits;
    n_bits[opt_num_forwards_]    = opt_num_forwards_Bits;

}

size_t packerCOMMON::packMyHdr(Packet* p, unsigned char* buf, size_t offset) {

    // Pointer to the COMMON packet header
    hdr_cmn* ch = HDR_CMN(p);

    // Common header fields useful for DESERT
    offset += put(buf, offset, &(ch->ptype_), n_bits[PTYPE]);

    offset += put(buf, offset, &(ch->size_), n_bits[SIZE]);

    offset += put(buf, offset, &(ch->uid_), n_bits[UID]);

    offset += put(buf, offset, &(ch->error_), n_bits[ERROR]);

    offset += put(buf, offset, &(ch->ts_), n_bits[TIMESTAMP]);

    offset += put(buf, offset, &(ch->direction_), n_bits[DIRECTION]);

    offset += put(buf, offset, &(ch->prev_hop_), n_bits[PREV_HOP]);

    offset += put(buf, offset, &(ch->next_hop_), n_bits[NEXT_HOP]);

    offset += put(buf, offset, &(ch->addr_type_), n_bits[ADDR_TYPE]);

    offset += put(buf, offset, &(ch->last_hop_), n_bits[LAST_HOP]);

    offset += put(buf, offset, &(ch->txtime_), n_bits[TXTIME]);

    // Other common header fields (maybe useful for future usage)	
    offset += put(buf, offset, &(ch->errbitcnt_), n_bits[errbitcnt_]);

    offset += put(buf, offset, &(ch->fecsize_), n_bits[fecsize_]);

    offset += put(buf, offset, &(ch->iface_), n_bits[iface_]);

    offset += put(buf, offset, &(ch->src_rt_valid), n_bits[src_rt_valid]);

    offset += put(buf, offset, &(ch->ts_arr_), n_bits[ts_arr_]);

    offset += put(buf, offset, &(ch->aomdv_salvage_count_), n_bits[aomdv_salvage_count_]);

    offset += put(buf, offset, &(ch->xmit_failure_), n_bits[xmit_failure_]);

    offset += put(buf, offset, &(ch->xmit_failure_data_), n_bits[xmit_failure_data_]);

    offset += put(buf, offset, &(ch->xmit_reason_), n_bits[xmit_reason_]);

    offset += put(buf, offset, &(ch->num_forwards_), n_bits[num_forwards_]);

    offset += put(buf, offset, &(ch->opt_num_forwards_), n_bits[opt_num_forwards_]);

    if (debug_) {
        printf("\033[0;43;30m TX COMMON packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

size_t packerCOMMON::unpackMyHdr(unsigned char* buf, size_t offset, Packet* p) {
    // Pointer to the COMMON packet header
    hdr_cmn* ch = HDR_CMN(p);

    // Common header fields useful for DESERT
    if (n_bits[PTYPE]){
        memset(&(ch->ptype_), 0, sizeof (ch->ptype_));
        offset += get(buf, offset, &(ch->ptype_), n_bits[PTYPE]);
    }

    if (n_bits[SIZE]){
	   memset(&(ch->size_), 0, sizeof (ch->size_));
	   offset += get(buf, offset, &(ch->size_), n_bits[SIZE]);
	   ch->size_ = restoreSignedValue(ch->size_, n_bits[SIZE]);
    }
	
    if(n_bits[UID]){
        memset(&(ch->uid_), 0, sizeof (ch->uid_));
        offset += get(buf, offset, &(ch->uid_), n_bits[UID]);
        ch->uid_ = restoreSignedValue(ch->uid_, n_bits[UID]);
    }
    
    if(n_bits[ERROR]){
        memset(&(ch->error_), 0, sizeof (ch->error_));
        offset += get(buf, offset, &(ch->error_), n_bits[ERROR]);
        ch->error_ = restoreSignedValue(ch->error_, n_bits[ERROR]);
    }

    if(n_bits[TIMESTAMP]){
        memset(&(ch->ts_), 0, sizeof (ch->ts_));
        offset += get(buf, offset, &(ch->ts_), n_bits[TIMESTAMP]);
    }

    if(n_bits[DIRECTION]){
        memset(&(ch->direction_), 0, sizeof (ch->direction_));
        offset += get(buf, offset, &(ch->direction_), n_bits[DIRECTION]);
        ch->direction_ = restoreSignedValue(ch->direction_, n_bits[DIRECTION]);   
    }
    
    if(n_bits[PREV_HOP]){
        memset(&(ch->prev_hop_), 0, sizeof (ch->prev_hop_));
        offset += get(buf, offset, &(ch->prev_hop_), n_bits[PREV_HOP]);
        ch->prev_hop_ = restoreSignedValue(ch->prev_hop_, n_bits[PREV_HOP]);
    }

    if(n_bits[NEXT_HOP]){
        memset(&(ch->next_hop_), 0, sizeof (ch->next_hop_));
        offset += get(buf, offset, &(ch->next_hop_), n_bits[NEXT_HOP]);
        ch->next_hop_ = restoreSignedValue(ch->next_hop_, n_bits[NEXT_HOP]);
    }

    if(n_bits[ADDR_TYPE]){
        memset(&(ch->addr_type_), 0, sizeof (ch->addr_type_));
        offset += get(buf, offset, &(ch->addr_type_), n_bits[ADDR_TYPE]);
        ch->addr_type_ = restoreSignedValue(ch->addr_type_, n_bits[ADDR_TYPE]);
    }
    
    if(n_bits[LAST_HOP]){
        memset(&(ch->last_hop_), 0, sizeof (ch->last_hop_));
        offset += get(buf, offset, &(ch->last_hop_), n_bits[LAST_HOP]);
        ch->last_hop_ = restoreSignedValue(ch->last_hop_, n_bits[LAST_HOP]);
    }
    
    if(n_bits[TXTIME]){
        memset(&(ch->txtime_), 0, sizeof (ch->txtime_));
        offset += get(buf, offset, &(ch->txtime_), n_bits[TXTIME]);
    }

    // Other common header fields (maybe useful for future usage)	
    memset(&(ch->errbitcnt_), 0, sizeof (ch->errbitcnt_));
    offset += get(buf, offset, &(ch->errbitcnt_), n_bits[errbitcnt_]);
    ch->errbitcnt_ = restoreSignedValue(ch->errbitcnt_, n_bits[errbitcnt_]);

    memset(&(ch->fecsize_), 0, sizeof (ch->fecsize_));
    offset += get(buf, offset, &(ch->fecsize_), n_bits[fecsize_]);
    ch->fecsize_ = restoreSignedValue(ch->fecsize_, n_bits[fecsize_]);

    memset(&(ch->iface_), 0, sizeof (ch->iface_));
    offset += get(buf, offset, &(ch->iface_), n_bits[iface_]);
    ch->iface_ = restoreSignedValue(ch->iface_, n_bits[iface_]);

    memset(&(ch->src_rt_valid), 0, sizeof (ch->src_rt_valid));
    offset += get(buf, offset, &(ch->src_rt_valid), n_bits[src_rt_valid]);

    memset(&(ch->ts_arr_), 0, sizeof (ch->ts_arr_));
    offset += get(buf, offset, &(ch->ts_arr_), n_bits[ts_arr_]);

    memset(&(ch->aomdv_salvage_count_), 0, sizeof (ch->aomdv_salvage_count_));
    offset += get(buf, offset, &(ch->aomdv_salvage_count_), n_bits[aomdv_salvage_count_]);
    ch->aomdv_salvage_count_ = restoreSignedValue(ch->aomdv_salvage_count_, n_bits[aomdv_salvage_count_]);

    memset(&(ch->xmit_failure_), 0, sizeof (ch->xmit_failure_));
    offset += get(buf, offset, &(ch->xmit_failure_), n_bits[xmit_failure_]);

    memset(&(ch->xmit_failure_data_), 0, sizeof (ch->xmit_failure_data_));
    offset += get(buf, offset, &(ch->xmit_failure_data_), n_bits[xmit_failure_data_]);

    memset(&(ch->xmit_reason_), 0, sizeof (ch->xmit_reason_));
    offset += get(buf, offset, &(ch->xmit_reason_), n_bits[xmit_reason_]);
    ch->xmit_reason_ = restoreSignedValue(ch->xmit_reason_, n_bits[xmit_reason_]);

    memset(&(ch->num_forwards_), 0, sizeof (ch->num_forwards_));
    offset += get(buf, offset, &(ch->num_forwards_), n_bits[num_forwards_]);

    memset(&(ch->opt_num_forwards_), 0, sizeof (ch->opt_num_forwards_));
    offset += get(buf, offset, &(ch->opt_num_forwards_), n_bits[opt_num_forwards_]);

    if (debug_) {
        printf("\033[0;43;30m RX COMMON packer hdr \033[0m \n");
        printMyHdrFields(p);
    }

    return offset;
}

void packerCOMMON::printMyHdrMap() {
    std::cout << "\033[0;43;30m" << " Packer Name " << "\033[0m" << " UWCOMMON" << std::endl;
    std::cout << "\033[0;43;30m 1st field " << "\033[0m" << " ptype_: " << PTYPE_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 2st field " << "\033[0m" << " size_: " << SIZE_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 3st field " << "\033[0m" << " uid_: " << UID_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 4st field " << "\033[0m" << " error_: " << ERROR_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 5st field " << "\033[0m" << " ts_: " << TIMESTAMP_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 6st field " << "\033[0m" << " direction_: " << DIRECTION_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 7st field " << "\033[0m" << " prev_hop_: " << PREV_HOP_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 8st field " << "\033[0m" << " next_hop_: " << NEXT_HOP_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 9st field " << "\033[0m" << " addr_type_: " << ADDR_TYPE_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 10st field " << "\033[0m" << " last_hop_: " << LAST_HOP_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 11st field " << "\033[0m" << " txtime_: " << TXTIME_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 12st field " << "\033[0m" << " errbitcnt_: " << errbitcnt_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 13st field " << "\033[0m" << " fecsize_: " << fecsize_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 14st field " << "\033[0m" << " iface_: " << iface_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 15st field " << "\033[0m" << " src_rt_valid: " << src_rt_valid_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 16st field " << "\033[0m" << " ts_arr_: " << ts_arr_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 17st field " << "\033[0m" << " aomdv_salvage_count_: " << aomdv_salvage_count_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 18st field " << "\033[0m" << " xmit_failure_: " << xmit_failure_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 19st field " << "\033[0m" << " xmit_failure_data: " << xmit_failure_data_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 20st field " << "\033[0m" << " xmit_reason: " << xmit_reason_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 21st field " << "\033[0m" << " num_forwards: " << num_forwards_Bits << " bits" << std::endl;
    std::cout << "\033[0;43;30m 22st field " << "\033[0m" << " opt_num_forwards: " << opt_num_forwards_Bits << " bits" << std::endl;
}

void packerCOMMON::printMyHdrFields(Packet* p) {

    hdr_cmn* ch = HDR_CMN(p);

    if (n_bits[PTYPE]) cout << "\033[0;43;30m ptype :\033[0m " << ch->ptype_ << " " << hex_bytes((size_t) ch->ptype_, n_bits[PTYPE]) << std::endl;
    if (n_bits[SIZE])  cout << "\033[0;43;30m size  :\033[0m " << ch->size_ << " " << hex_bytes(ch->size_, n_bits[SIZE]) << std::endl;
    if (n_bits[UID])   cout << "\033[0;43;30m uid   :\033[0m " << ch->uid_ << " " << hex_bytes(ch->uid_, n_bits[UID]) << std::endl;
    if (n_bits[ERROR]) cout << "\033[0;43;30m error :\033[0m " << ch->error_ << " " << hex_bytes(ch->error_, n_bits[3]) << std::endl;
    if (n_bits[4] != 0) cout << "\033[0;43;30m ts   :\033[0m " << ch->ts_ << " " << hex_bytes(ch->ts_) << std::endl;
    if (n_bits[5] != 0) cout << "\033[0;43;30m direction:\033[0m " << ch->direction_ << " " << hex_bytes(ch->direction_, n_bits[5]) << std::endl;
    if (n_bits[6] != 0) cout << "\033[0;43;30m prev_hop:\033[0m " << ch->prev_hop_ << " " << hex_bytes(ch->prev_hop_, n_bits[6]) << std::endl;
    if (n_bits[7] != 0) cout << "\033[0;43;30m next_hop:\033[0m " << ch->next_hop_ << " " << hex_bytes(ch->next_hop_, n_bits[7]) << std::endl;
    if (n_bits[8] != 0) cout << "\033[0;43;30m addr_type:\033[0m " << ch->addr_type_ << " " << hex_bytes(ch->addr_type_, n_bits[8]) << std::endl;
    if (n_bits[9] != 0) cout << "\033[0;43;30m last_hop:\033[0m " << ch->last_hop_ << " " << hex_bytes(ch->last_hop_, n_bits[9]) << std::endl;
    if (n_bits[10] != 0) cout << "\033[0;43;30m txtime:\033[0m " << ch->txtime_ << " " << hex_bytes(ch->txtime_) << std::endl;
    if (n_bits[11] != 0) cout << "\033[0;43;30m errbitcnt:\033[0m " << ch->errbitcnt_ << " " << hex_bytes(ch->errbitcnt_, n_bits[11]) << std::endl;
    if (n_bits[12] != 0) cout << "\033[0;43;30m fecsize:\033[0m " << ch->fecsize_ << " " << hex_bytes(ch->fecsize_, n_bits[12]) << std::endl;
    if (n_bits[13] != 0) cout << "\033[0;43;30m iface:\033[0m " << ch->iface_ << " " << hex_bytes(ch->iface_, n_bits[13]) << std::endl;
    //if (n_bits[14] != 0) cout << "\033[0;43;30m src_rt_valid:\033[0m " << ch->src_rt_valid_ << " " << hex_bytes(ch->src_rt_valid_, n_bits[14]) << std::endl;
    if (n_bits[15] != 0) cout << "\033[0;43;30m ts_arr:\033[0m " << ch->ts_arr_ << " " << hex_bytes(ch->ts_arr_) << std::endl;
    if (n_bits[16] != 0) cout << "\033[0;43;30m aomdv_salvage_count:\033[0m " << ch->aomdv_salvage_count_ << " " << hex_bytes(ch->aomdv_salvage_count_, n_bits[16]) << std::endl;
    //if (n_bits[17] != 0) cout << "\033[0;43;30m xmit_failure_:\033[0m " << ch->xmit_failure_ << " " << hex_bytes(ch->xmit_failure_, n_bits[17]) << std::endl;
    //if (n_bits[18] != 0) cout << "\033[0;43;30m xmit_failure_data_:\033[0m " << ch->xmit_failure_data_ << " " << hex_bytes(ch->xmit_failure_data_, n_bits[18]) << std::endl;
    if (n_bits[19] != 0) cout << "\033[0;43;30m xmit_reason_:\033[0m " << ch->xmit_reason_ << " " << hex_bytes(ch->xmit_reason_, n_bits[19]) << std::endl;
    if (n_bits[20] != 0) cout << "\033[0;43;30m num_forwards_:\033[0m " << ch->num_forwards_ << " " << hex_bytes(ch->num_forwards_, n_bits[20]) << std::endl;
    if (n_bits[21] != 0) cout << "\033[0;43;30m opt_num_forwards_:\033[0m " << ch->opt_num_forwards_ << " " << hex_bytes(ch->opt_num_forwards_, n_bits[21]) << std::endl;
    //cout << "\033[0;41;30m packerMAC::printMyHdrField WARNING \033[0m, Field index " << field << " does not exist or its printing is not implemented." << std::endl;
}
