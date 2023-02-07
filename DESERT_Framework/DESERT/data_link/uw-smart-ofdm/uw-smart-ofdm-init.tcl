#
# Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
# Author: Sara Falleni

# Network is 70x70x90 -> max diagonal is 133. 
# Max propagation time 0.08s. Max round trip time 2*0.08 = 0.16
# CTS_backoff = 2s 

PacketHeaderManager set tab_(PacketHeader/OFDMMAC) 1

Module/UW/SMART_OFDM set debug_ 		    0
Module/UW/SMART_OFDM set HDR_size_ 		    0
Module/UW/SMART_OFDM set ACK_size_  		10; 
Module/UW/SMART_OFDM set RTS_size_  		14; #2 Bytes BytesToSend, 2byte int mask for carriers w binary masks
Module/UW/SMART_OFDM set CTS_size_  		20; #1 double TimeReserved, 2byte int mask for carriers w binary masks
Module/UW/SMART_OFDM set DATA_size_  		1536
Module/UW/SMART_OFDM set bitrateCar_        6400
Module/UW/SMART_OFDM set max_tx_tries_		2
Module/UW/SMART_OFDM set max_rts_tries_     4
Module/UW/SMART_OFDM set wait_constant_		0.1
Module/UW/SMART_OFDM set uwsmartofdm_debug_	0
Module/UW/SMART_OFDM set max_payload_		125
Module/UW/SMART_OFDM set ACK_timeout_		1.5
Module/UW/SMART_OFDM set alpha_			    0.8
Module/UW/SMART_OFDM set buffer_pkts_		-1
Module/UW/SMART_OFDM set backoff_tuner_   	1
Module/UW/SMART_OFDM set max_backoff_counter_   4
Module/UW/SMART_OFDM set MAC_addr_ 		    0
Module/UW/SMART_OFDM set timeslots_         10
Module/UW/SMART_OFDM set timeslot_length_   3.0
Module/UW/SMART_OFDM set req_tslots_        3
Module/UW/SMART_OFDM set max_car_reserved_  4 
Module/UW/SMART_OFDM set print_transitions_ 0
Module/UW/SMART_OFDM set max_burst_size_    1
Module/UW/SMART_OFDM set fullBand_          0