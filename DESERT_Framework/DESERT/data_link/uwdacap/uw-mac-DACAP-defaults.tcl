#
# Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
# Author: Federico Guerra
# Version: 1.0.0

PacketHeaderManager set tab_(PacketHeader/DACAP) 1

Module/UW/DACAP set t_min               2.0
Module/UW/DACAP set T_W_min             0.0
Module/UW/DACAP set delta_D             1500.0
Module/UW/DACAP set delta_data          0
Module/UW/DACAP set debug_              10
Module/UW/DACAP set max_prop_delay      2.0
Module/UW/DACAP set CTS_size            4
Module/UW/DACAP set RTS_size            4
Module/UW/DACAP set HDR_size            2
Module/UW/DACAP set WRN_size            4
Module/UW/DACAP set ACK_size            4
# Module/UW/DACAP set consecutive_packets 1
Module/UW/DACAP set backoff_tuner       1.0
Module/UW/DACAP set wait_costant        0.2
Module/UW/DACAP set max_payload         10000
Module/UW/DACAP set max_backoff_counter 5
Module/UW/DACAP set max_tx_tries  	    5
Module/UW/DACAP set buffer_pkts	        -1
Module/UW/DACAP set alpha_     	        0.8


#Module/UW/DACAP instproc init {args} {
#    $self next $args
#    $self settag "FcSMAC"
#}

