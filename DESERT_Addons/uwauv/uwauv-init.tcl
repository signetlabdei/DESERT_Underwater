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
# @file   uwauv-defaults.tcl
# @author Filippo Campagnaro Alessia Ortile
# @version 1.0.0

PacketHeaderManager set tab_(PacketHeader/UWAUV) 1
PacketHeaderManager set tab_(PacketHeader/UWAUVCtr) 1
PacketHeaderManager set tab_(PacketHeader/UWAUVError) 1


Module/UW/AUV set packetSize_         500
Module/UW/AUV set period_             60
Module/UW/AUV set destPort_           0
Module/UW/AUV set destAddr_           0
Module/UW/AUV set debug_              0
Module/UW/AUV set PoissonTraffic_     1
Module/UW/AUV set drop_out_of_order_  1
Module/UW/AUV set ackTimeout_		  10
Module/UW/AUV set ackPriority_ 		  0
Module/UW/AUV set drop_old_waypoints_ 0
Module/UW/AUV set log_flag_ 		  0

Module/UW/AUV instproc init {args} {
    $self next $args
    $self settag "UW/AUV"
}

Module/UW/AUV/CTR set packetSize_         500
Module/UW/AUV/CTR set period_             60
Module/UW/AUV/CTR set destPort_           0
Module/UW/AUV/CTR set destAddr_           0
Module/UW/AUV/CTR set debug_              0
Module/UW/AUV/CTR set PoissonTraffic_     1
Module/UW/AUV/CTR set drop_out_of_order_  0
Module/UW/AUV/CTR set adaptiveRTO_     	  0

Module/UW/AUV/CTR instproc init {args} {
    $self next $args
    $self settag "UW/AUV/CTR"
}

Module/UW/AUV/ERR set packetSize_         500
Module/UW/AUV/ERR set period_             60
Module/UW/AUV/ERR set destPort_           0
Module/UW/AUV/ERR set destAddr_           0
Module/UW/AUV/ERR set debug_              0
Module/UW/AUV/ERR set PoissonTraffic_     1
Module/UW/AUV/ERR set drop_out_of_order_  1
Module/UW/AUV/ERR set adaptiveRTO_     	  0

Module/UW/AUV/ERR instproc init {args} {
    $self next $args
    $self settag "UW/AUV/ERR"
}

Module/UW/AUV/CER set packetSize_         100
Module/UW/AUV/CER set period_             60
Module/UW/AUV/CER set destPort_           0
Module/UW/AUV/CER set destAddr_           0
Module/UW/AUV/CER set debug_              0
Module/UW/AUV/CER set PoissonTraffic_     0
Module/UW/AUV/CER set drop_out_of_order_  1
Module/UW/AUV/CER set adaptiveRTO_        0

Module/UW/AUV/CER instproc init {args} {
    $self next $args
    $self settag "UW/AUV/CER"
}

Module/UW/AUV/CEB set packetSize_         100
Module/UW/AUV/CEB set period_             60
Module/UW/AUV/CEB set destPort_           0
Module/UW/AUV/CEB set destAddr_           0
Module/UW/AUV/CEB set debug_              0
Module/UW/AUV/CEB set PoissonTraffic_     0
Module/UW/AUV/CEB set drop_out_of_order_  1
Module/UW/AUV/CEB set adaptiveRTO_        0

Module/UW/AUV/CEB instproc init {args} {
    $self next $args
    $self settag "UW/AUV/CEB"
}

Module/UW/AUV/ERB set packetSize_         100
Module/UW/AUV/ERB set period_             60
Module/UW/AUV/ERB set destPort_           0
Module/UW/AUV/ERB set destAddr_           0
Module/UW/AUV/ERB set debug_              0
Module/UW/AUV/ERB set PoissonTraffic_     0
Module/UW/AUV/ERB set drop_out_of_order_  1
Module/UW/AUV/ERB set adaptiveRTO_     	  0

Module/UW/AUV/ERB instproc init {args} {
    $self next $args
    $self settag "UW/AUV/ERB"
}



Position/UWSM set debug_ 0
Position/UWSME set debug_ 0

PlugIn/PositionDB set debug_ 0
