#
# Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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
# @file   uwrov-defaults.tcl
# @author Filippo Campagnaro
# @version 1.1.0

PacketHeaderManager set tab_(PacketHeader/UWROV) 1

Module/UW/ROV set packetSize_         500
Module/UW/ROV set period_             60
Module/UW/ROV set destPort_           0
Module/UW/ROV set destAddr_           0
Module/UW/ROV set debug_              0
Module/UW/ROV set PoissonTraffic_     1
Module/UW/ROV set drop_out_of_order_  0
Module/UW/ROV set send_ack_immediately 0

Module/UW/ROV instproc init {args} {
    $self next $args
    $self settag "UW/ROV"
}

Module/UW/ROV/CTR set packetSize_         500
Module/UW/ROV/CTR set period_             60
Module/UW/ROV/CTR set destPort_           0
Module/UW/ROV/CTR set destAddr_           0
Module/UW/ROV/CTR set debug_              0
Module/UW/ROV/CTR set PoissonTraffic_     1
Module/UW/ROV/CTR set drop_out_of_order_  0

Module/UW/ROV/CTR instproc init {args} {
    $self next $args
    $self settag "UW/ROV/CTR"
}

#Module/UW/TDMA set slot_status  0 
#Module/UW/TDMA set num_hosts  2 
#Module/UW/TDMA set host_id  2
#Module/UW/TDMA set frame_time  10
#Module/UW/TDMA set guard_time 1
#Module/UW/TDMA set slot_duration  4

#Module/UW/TDMA instproc init {args} {
#    $self next $args
#    $self settag "UW/TDMA"
#}

Position/UWSM set debug_ 0

PlugIn/PositionDB set debug_ 0
