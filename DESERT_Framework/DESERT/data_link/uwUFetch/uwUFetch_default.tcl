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

PacketHeaderManager set tab_(PacketHeader/TRIGGER_UFETCH) 1
PacketHeaderManager set tab_(PacketHeader/RTS_UFETCH) 1
PacketHeaderManager set tab_(PacketHeader/CTS_UFETCH) 1
PacketHeaderManager set tab_(PacketHeader/BEACON_UFETCH) 1
PacketHeaderManager set tab_(PacketHeader/PROBE_UFETCH) 1
PacketHeaderManager set tab_(PacketHeader/POLL_UFETCH) 1
PacketHeaderManager set tab_(PacketHeader/CBEACON_UFETCH) 1

    Module/UW/UFETCH/NODE set  TIME_BEFORE_START_COMU_HN_NODE_       60  ;# Interval time in which HN is enabled to receive a TRIGGER packet from AUV. After that time HN start the communication with SNs transmitting a BEACON packet                    
    Module/UW/UFETCH/NODE set  MAXIMUM_VALUE_BACKOFF_PROBE_          6.0 ;# Upper Bound of time interval in which sensor node choice the backoff time before to transmit a PROBE packet         
    Module/UW/UFETCH/NODE set  MINIMUM_VALUE_BACKOFF_PROBE_          1.0 ;# Lower Bound of time interval in which sensor node choice the backoff time before to transmit a PROBE packet
    Module/UW/UFETCH/NODE set  MAXIMUM_NODE_POLLED_                  8   ;# Maximum number of PROBE packets that HN can received after the transmission of BEACON or CBEACON packets to the SNs         
    Module/UW/UFETCH/NODE set  MAXIMUM_PAYLOAD_SIZE_                 80  ;# size of data packet payload
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_PROBES_PCK_              15.0 ;# Duration of interval time in which HN is enabled to receives PROBE packets from SNs       
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_POLL_PCK_                120  ;# Duration of interval time in which SN is enabled to receive POLL packet from HN     
    Module/UW/UFETCH/NODE set  SEE_THE_TRANSITIONS_STATE_            1
    Module/UW/UFETCH/NODE set  GUARD_INTERVAL_                       6.0  ;#Guard interval
    Module/UW/UFETCH/NODE set  MAXIMUM_BUFFER_SIZE_                  100  ;#Maximum number of data packets that each single SN can be store    
    Module/UW/UFETCH/NODE set  MAXIMUM_CBEACON_TRANSMISSIONS_        2    ;#Number of CBEACON that each HN transmit after the transmission of BEACON packet      	
    Module/UW/UFETCH/NODE set  MAXIMUM_PCK_WANT_RX_HN_FROM_NODE_     100  ;#Maximum number of DATA packets that the HN want to receive in a single cycle BEACON-PROBE-POLL-DATA-CBEACON of simulation        	
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_CTS_                     20.0 ;# Duration of interval time in which HN is enabled to receives CTS packets from AUV        
    Module/UW/UFETCH/NODE set  NUMBER_OF_RUN_                        1	  ;#Number of the run in execution
    Module/UW/UFETCH/NODE set  TIME_BETWEEN_2_DATA_TX_HN_            6	  ;# Interval time that must elapse between two successive DATA packets transmitted by HN to the AUV	
    Module/UW/UFETCH/NODE set  TIME_BETWEEN_2_DATA_TX_NODE_          6    ;# Interval time that must elapse between two successive DATA packets transmitted by SN to the HN
    Module/UW/UFETCH/NODE set  debug_                                1 ;#1=debug ON
																	   ;#0=debug OFF
    Module/UW/UFETCH/NODE set  MY_DEBUG_                             1 ;#1=debug loggin ON
																	   ;#0=debug loggin OFF	 
	Module/UW/UFETCH/NODE set  MODE_COMM_                            0 ;#0=with RTS & CTS
															           ;#1=without RTS & CTS
	Module/UW/UFETCH/NODE set  BURST_DATA_                           0 ;#0=without burst data
															           ;#1=with burst data

	Module/UW/UFETCH/AUV    set T_min_RTS_                              1.0  ;# Lower Bound of time interval in which head node choice the backoff before to transmit a RTS packet
    Module/UW/UFETCH/AUV    set T_max_RTS_                              5.0  ;# Upper Bound of time interval in which head node choice the backoff before to transmit a RTS packet
    Module/UW/UFETCH/AUV    set T_guard_                                10.0 ;# guard interval
    Module/UW/UFETCH/AUV    set t_RTS_                                  10.0 ;# interval time in which AUV is enabled to receive RTS packets from HN           
    Module/UW/UFETCH/AUV    set MAX_PAYLOAD                             80 	 ;# size of data packet payload  	
    Module/UW/UFETCH/AUV    set num_max_DATA_AUV_want_receive_          30.0 ;# maximum number of DATA packets that AUV want to receive from HN in a single cycle TRIGGER-RTS-CTS-DATA of the simulation       
    Module/UW/UFETCH/AUV    set TIME_BEFORE_TX_TRIGGER_PCK_             32  ;# duration of interval time in which AUV wait before to start the transmission of TRIGGER packet 
    Module/UW/UFETCH/AUV    set NUMBER_OF_RUN_                          1   ;# id of simulation
    Module/UW/UFETCH/AUV    set HEAD_NODE_1_ 							249 ;# id head node 1
	Module/UW/UFETCH/AUV    set HEAD_NODE_2_ 							250 ;# id head node 2
	Module/UW/UFETCH/AUV    set HEAD_NODE_3_ 							251 ;# id head node 3
	Module/UW/UFETCH/AUV    set HEAD_NODE_4_ 							252 ;# id head node 4 
    Module/UW/UFETCH/AUV    set debug_                                  1 ;#1=debug ON
																	      ;#0=debug OFF
    Module/UW/UFETCH/AUV    set MY_DEBUG_                               1 ;#1=debug loggin ON
																	      ;#0=debug loggin OFF
	Module/UW/UFETCH/AUV    set NUM_HN_NETWORK_ 					    4	
    Module/UW/UFETCH/AUV    set MODE_COMM_ 							    0 ;#0=with RTS & CTS
															              ;#1=without RTS & CTS

