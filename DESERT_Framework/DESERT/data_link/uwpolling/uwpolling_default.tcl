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
# Author: Favaro Federico
# version: 1.0.0

PacketHeaderManager set tab_(PacketHeader/POLL) 1
PacketHeaderManager set tab_(PacketHeader/TRIGGER) 1
PacketHeaderManager set tab_(PacketHeader/PROBE) 1


Module/UW/POLLING/NODE set T_poll_              50
Module/UW/POLLING/NODE set backoff_tuner_       1
Module/UW/POLLING/NODE set max_payload_         125
Module/UW/POLLING/NODE set buffer_data_pkts_    50
Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_    1
Module/UW/POLLING/NODE set sea_trial_            0
Module/UW/POLLING/NODE set print_stats_          0
Module/UW/POLLING/NODE set modem_data_bit_rate_ 1000
Module/UW/POLLING/NODE set n_run                0
Module/UW/POLLING/NODE set intra_data_guard_time_ 7

Module/UW/POLLING/AUV set max_payload_          125
Module/UW/POLLING/AUV set T_probe_              10
Module/UW/POLLING/AUV set T_min_                0.5
Module/UW/POLLING/AUV set T_max_                5
Module/UW/POLLING/AUV set T_guard_              20
Module/UW/POLLING/AUV set max_polled_node_      10
Module/UW/POLLING/AUV set sea_trial_            0
Module/UW/POLLING/AUV set print_stats_          0
Module/UW/POLLING/AUV set modem_data_bit_rate_  1000
Module/UW/POLLING/AUV set n_run                 0
Module/UW/POLLING/AUV set Data_Poll_guard_time_ 3
