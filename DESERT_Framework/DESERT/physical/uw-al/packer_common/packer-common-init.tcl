#
# Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University of Padova (SIGNET lab) nor the 
#    names of its contributors may be used to endorse or promote products 
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#


NS2/COMMON/Packer set PTYPE_Bits 32
NS2/COMMON/Packer set SIZE_Bits 32
NS2/COMMON/Packer set UID_Bits 32
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 64
NS2/COMMON/Packer set DIRECTION_Bits 0
NS2/COMMON/Packer set PREV_HOP_Bits 32
NS2/COMMON/Packer set NEXT_HOP_Bits 32
NS2/COMMON/Packer set ADRR_TYPE_Bits 32
NS2/COMMON/Packer set LAST_HOP_Bits 32
NS2/COMMON/Packer set TXTIME_Bits 64
     
NS2/COMMON/Packer set errbitcnt_Bits 0
NS2/COMMON/Packer set fecsize_Bits 0
NS2/COMMON/Packer set iface_Bits 0
NS2/COMMON/Packer set src_rt_valid_Bits 0
NS2/COMMON/Packer set ts_arr_Bits 0
NS2/COMMON/Packer set aomdv_salvage_count_Bits 0
NS2/COMMON/Packer set xmit_failure_Bits 0
NS2/COMMON/Packer set xmit_failure_data_Bits 0
NS2/COMMON/Packer set xmit_reason_Bits 0
NS2/COMMON/Packer set num_forwards_Bits 0
NS2/COMMON/Packer set opt_num_forwards_Bits 0

NS2/COMMON/Packer set debug_ 0