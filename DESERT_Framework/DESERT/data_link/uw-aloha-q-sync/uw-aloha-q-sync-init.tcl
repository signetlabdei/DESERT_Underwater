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

# Author: Aleksa Albijanic
# version: 1.0.0

Module/UW/ALOHAQ_SYNC_NODE set debug_ 	            0
Module/UW/ALOHAQ_SYNC_NODE set HDR_size_ 	    5
Module/UW/ALOHAQ_SYNC_NODE set wait_constant_       0.1
Module/UW/ALOHAQ_SYNC_NODE set MAC_addr_ 	    0
Module/UW/ALOHAQ_SYNC_NODE set sea_trial_ 	    0
Module/UW/ALOHAQ_SYNC_NODE set guard_time           0
Module/UW/ALOHAQ_SYNC_NODE set nn                   5
Module/UW/ALOHAQ_SYNC_NODE set max_queue_size_      10
Module/UW/ALOHAQ_SYNC_NODE set drop_old_            0
Module/UW/ALOHAQ_SYNC_NODE set checkPriority_       0
Module/UW/ALOHAQ_SYNC_NODE set mac2phy_delay_       [expr 1.0e-9]
Module/UW/ALOHAQ_SYNC_NODE set curr_slot            0
Module/UW/ALOHAQ_SYNC_NODE set slot_duration_factor 1.5
Module/UW/ALOHAQ_SYNC_NODE set subslot_num	    5

Module/UW/ALOHAQ_SYNC_SINK set debug_ 	            0
Module/UW/ALOHAQ_SYNC_SINK set HDR_size_ 	    5
Module/UW/ALOHAQ_SYNC_SINK set ACK_size_ 	    2
Module/UW/ALOHAQ_SYNC_SINK set wait_constant_       0.1
Module/UW/ALOHAQ_SYNC_SINK set MAC_addr_ 	    0
Module/UW/ALOHAQ_SYNC_SINK set sea_trial_ 	    0  
Module/UW/ALOHAQ_SYNC_SINK set mac2phy_delay_       [expr 1.0e-9]
Module/UW/ALOHAQ_SYNC_SINK set nn                   5
Module/UW/ALOHAQ_SYNC_SINK set slot_duration_factor 1.5





