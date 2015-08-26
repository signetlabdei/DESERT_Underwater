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
# @file   sun-defaults.tcl
# @author Giovanni Toso
# @version 1.1.1

PacketHeaderManager set tab_(PacketHeader/SUN_ACK) 1
PacketHeaderManager set tab_(PacketHeader/SUN_DATA) 1
PacketHeaderManager set tab_(PacketHeader/SUN_PEST) 1
PacketHeaderManager set tab_(PacketHeader/SUN_PROBE) 1

Module/UW/SUNNode set metrics_ 1
Module/UW/SUNNode set ipAddr_ 0
Module/UW/SUNNode set PoissonTraffic_ 1
Module/UW/SUNNode set period_status_ 0.1
Module/UW/SUNNode set period_data_ 30
Module/UW/SUNNode set timer_route_validity_ 600
Module/UW/SUNNode set timer_sink_probe_validity_ 70
Module/UW/SUNNode set timer_buffer_ 30
Module/UW/SUNNode set timer_search_path_ 60
Module/UW/SUNNode set printDebug_ 0
Module/UW/SUNNode set max_ack_error_ 0
Module/UW/SUNNode set alpha_ 1.0/3.0
Module/UW/SUNNode set buffer_max_size_ 5
Module/UW/SUNNode set probe_min_snr_ 15
Module/UW/SUNNode set safe_timer_buffer_ 0
Module/UW/SUNNode set debug_ 0
Module/UW/SUNNode set disable_path_error_ 0
Module/UW/SUNNode set reset_buffer_if_error_ 0

Module/UW/SUNSink set ipAddr_ 0
Module/UW/SUNSink set PoissonTraffic_ 1
Module/UW/SUNSink set periodPoissonTraffic_ 0.1
Module/UW/SUNSink set t_probe 30
Module/UW/SUNSink set printDebug_ 0
Module/UW/SUNSink set debug_ 0
