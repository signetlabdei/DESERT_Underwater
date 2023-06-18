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
# @file   uwmc-init.tcl
# @author Filippo Campagnaro, Vincenzo Cimino
# @version 1.0.0

PacketHeaderManager set tab_(PacketHeader/UWMC) 1

Module/UW/MC set packetSize_         500
Module/UW/MC set period_             60
Module/UW/MC set destPort_           0
Module/UW/MC set destAddr_           0
Module/UW/MC set debug_              0
Module/UW/MC set PoissonTraffic_     1
Module/UW/MC set drop_out_of_order_  0
Module/UW/MC set ackTimeout_		  10
Module/UW/MC set ackPriority_ 		  0
Module/UW/MC set drop_old_waypoints_ 0
Module/UW/MC set log_flag_ 		  0
Module/UW/MC set adaptiveRTO_     	  0

Module/UW/MC instproc init {args} {
    $self next $args
    $self settag "UW/MC"
}

Position/UWSM set debug_ 0

PlugIn/PositionDB set debug_ 0
