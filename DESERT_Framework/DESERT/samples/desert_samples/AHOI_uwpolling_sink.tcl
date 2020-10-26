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
# Author: Emanuele Coccolo, Roberto Francescon
# Version: 1.0.0
#

# if set to 1 the Application listen from the socket port provided in input
set opt(AppSocket)  1;
# Protocol to use for the Application socket, TCP or UDP
set opt(protocol) "TCP" ;
######################
# Simulation Options #
######################
set opt(n_node) 3
if {$opt(n_node) <= 0} {
  puts "Number of nodes equal to zero! Please put a number of nodes >= 2"
  exit(1)
}

if {$opt(n_node) <= 1} {
  puts "WARNING!! number of nodes set to 1. Put a number of nodes >= 2"
}

# Terminal's parameter check
if {$argc != 8} {
  puts "The script needs 8 input to work"
  puts "1 - ID of the node"
  puts "2 - ID of the asv"
  puts "3 - Start time (seconds)"
  puts "4 - Stop time (seconds)"
  puts "5 - Period of scheduler (seconds)"
  puts "6 - Serial Interface Path (e.g. ttyUSB0)"
  puts "7 - Application port"
  puts "8 - ID of the experiment"
  puts "Please try again."
  exit
} else {
  set opt(node)     [lindex $argv 0]
  set opt(auv)     	[lindex $argv 1]
  set opt(sink)		  fffffffff
  set opt(start)    [lindex $argv 2]
  set opt(stop)     [lindex $argv 3]
  set opt(traffic)  [lindex $argv 4]
  set opt(i_path)   [lindex $argv 5]
  set opt(app_port) [lindex $argv 6]
  set opt(n_run)    [lindex $argv 7]


  set opt(rep_num)	[lindex $argv 0]
}

#####################
# Library Loading   #
#####################
# Load here all the NS-Miracle libraries you need
load libMiracle.so
load libmphy.so
load libuwip.so
load libuwmll.so
load libuwstaticrouting.so
load libuwudp.so
load libuwcbr.so
load libuwvbr.so
load libuwcbrtracer.so
load libuwapplication.so
load libpackeruwapplication.so
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so
load libpackeruwcbr.so
load libuwphy_clmsgs.so
load libuwconnector.so
load libuwmodem.so
load libuwahoimodem.so
load libuwpolling.so
load libpackeruwpolling.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

#Declare the use of a Real Time Schedule (necessary for the interfacing with real hardware)
$ns use-scheduler RealTime

##################
# Tcl variables  #
##################

# MAC address of the machine
set adrMAC $opt(node)

# time when actually to stop the simulation
set time_stop [expr "$opt(stop) + 30"]

#Trace file name
set tf_name "S2C_Evologics_Uwpolling.tr"

#Open a file for writing the trace data
set tf [open $tf_name w]
$ns trace-all $tf

set rng [new RNG]
$rng seed         $opt(node)

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng

#########################
# Simulation parameters #
#########################
set opt(pktsize)    32
# set opt(T_backoff)	15
set opt(T_backoff)  15
set opt(max_read)   64
set opt(exp_ID)     $opt(rep_num)

global def_rng
set def_rng [new RNG]
$def_rng default
for {set k 0} {$k < $opt(node)} {incr k} {
     $def_rng next-substream
}


#########################
# Module Configuration  #
#########################
# Put here all the commands to set globally the initialization values of the binded variables (optional)

# variables for the CBR module
Module/UW/CBR set debug_		       0
# Module/UW/CBR set packetSize_          $opt(pktsize)
# Module/UW/CBR set period_              $opt(traffic)
# Module/UW/CBR set PoissonTraffic_      1
# Module/UW/CBR set drop_out_of_order_   0

UW/APP/uwApplication/Packer set SN_FIELD_ 16
UW/APP/uwApplication/Packer set RFFT_FIELD_ 5
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 2
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 8
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 8
UW/APP/uwApplication/Packer set debug_ 0

Module/UW/APPLICATION set debug_ 10
Module/UW/APPLICATION set period_ $opt(traffic)
Module/UW/APPLICATION set PoissonTraffic_ 0
if {$opt(AppSocket) == 1} {
  # Module/UW/APPLICATION set Socket_Port_ $opt(app_port)
  Module/UW/APPLICATION set max_read_length $opt(max_read)
} else {
  Module/UW/APPLICATION set Payload_size_ $opt(payload_size)
}
Module/UW/APPLICATION set drop_out_of_order_ 0
Module/UW/APPLICATION set pattern_sequence_ 0     
Module/UW/APPLICATION set EXP_ID_ $opt(exp_ID)


if {$opt(node) ==  $opt(auv)} {
  Module/UW/POLLING/AUV set max_payload_        $opt(pktsize)
  Module/UW/POLLING/AUV set T_min_              0
  Module/UW/POLLING/AUV set T_max_              $opt(T_backoff) ;#MAX backoff time
  # Module/UW/POLLING/AUV set T_probe_guard_            3; #related to T_max + RTT
  Module/UW/POLLING/AUV set T_probe_guard_            3; #related to T_max + RTT
  # Module/UW/POLLING/AUV set T_guard_            2
  Module/UW/POLLING/AUV set T_guard_            2
  Module/UW/POLLING/AUV set max_polled_node_    2000
  Module/UW/POLLING/AUV set max_buffer_size_    500
  Module/UW/POLLING/AUV set max_tx_pkts_      20
  Module/UW/POLLING/AUV set n_run_                  $opt(rep_num);#used for c++ rng
  Module/UW/POLLING/AUV set debug_        1
  Module/UW/POLLING/AUV set full_knowledge_       0
  Module/UW/POLLING/AUV set sea_trial_            1
  # Module/UW/POLLING/AUV set Data_Poll_guard_time_ 2
  Module/UW/POLLING/AUV set Data_Poll_guard_time_ 2
  Module/UW/POLLING/AUV set modem_data_bit_rate_ 200
} else {
  if {$opt(node) ==  $opt(sink)} {
    # Module/UW/POLLING/SINK set T_data_guard_  5 ;#has to be bigger than T_probe(AUV)
    Module/UW/POLLING/SINK set T_data_guard_  5 ;#has to be bigger than T_probe(AUV)
    Module/UW/POLLING/SINK set debug_     1
    Module/UW/POLLING/SINK set sink_id_       253
    Module/UW/POLLING/SINK set n_run_               $opt(rep_num);#used for c++ rng
    Module/UW/POLLING/SINK set useAdaptiveTdata_    1
    Module/UW/POLLING/SINK set addr_       $opt(node)
    Module/UW/POLLING/SINK set sea_trial_            1
    # Module/UW/POLLING/SINK set T_guard_              2
    Module/UW/POLLING/SINK set T_guard_              1
    Module/UW/POLLING/SINK set max_payload_       $opt(pktsize)
    Module/UW/POLLING/SINK set modem_data_bit_rate_ 200

  } else {
    # Module/UW/POLLING/NODE set T_poll_guard_      5;#has to be bigger than T_probe(AUV)
    Module/UW/POLLING/NODE set T_poll_guard_      5;#has to be bigger than T_probe(AUV)
    Module/UW/POLLING/NODE set backoff_tuner_     1
    Module/UW/POLLING/NODE set max_payload_       $opt(pktsize)
    Module/UW/POLLING/NODE set buffer_data_pkts_  150
    Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_  5
    Module/UW/POLLING/NODE set intra_data_guard_time_ 0.01
    Module/UW/POLLING/NODE set useAdaptiveTpoll_   1
    Module/UW/POLLING/NODE set n_run_              $opt(rep_num);#used for c++ rng
    Module/UW/POLLING/NODE set debug_     1
    Module/UW/POLLING/NODE set addr_       $opt(node)
    Module/UW/POLLING/NODE set sea_trial_            1
    Module/UW/POLLING/NODE set modem_data_bit_rate_ 200
  }
}


# # variables for the ALOHA-CSMA module

# variables for the AL module
Module/UW/AL set nodeID $opt(node)
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 1400
Module/UW/AL set debug_ 0
Module/UW/AL set interframe_period 0.e1
Module/UW/AL set frame_set_validity 5

# variables for the packer(s)
UW/AL/Packer set SRC_ID_Bits 6
UW/AL/Packer set PKT_ID_Bits 8 ;#32
UW/AL/Packer set FRAME_OFFSET_Bits 0 ;#16
UW/AL/Packer set M_BIT_Bits 1
UW/AL/Packer set DUMMY_CONTENT_Bits 0
UW/AL/Packer set debug_ 0

NS2/COMMON/Packer set PTYPE_Bits 8
NS2/COMMON/Packer set SIZE_Bits 6
NS2/COMMON/Packer set UID_Bits 0 ;#8 Not useful since ns runs on different PCs
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 0 ;#8 Not useful since timestamp is double
NS2/COMMON/Packer set PREV_HOP_Bits 8
NS2/COMMON/Packer set NEXT_HOP_Bits 8
NS2/COMMON/Packer set ADRR_TYPE_Bits 0
NS2/COMMON/Packer set LAST_HOP_Bits 0
NS2/COMMON/Packer set TXTIME_Bits 0
NS2/COMMON/Packer set debug_ 0

UW/IP/Packer set SAddr_Bits 8
UW/IP/Packer set DAddr_Bits 8
UW/IP/Packer set debug_ 0

NS2/MAC/Packer set Ftype_Bits 0
NS2/MAC/Packer set SRC_Bits 8
NS2/MAC/Packer set DST_Bits 8
NS2/MAC/Packer set Htype_Bits 0
NS2/MAC/Packer set TXtime_Bits 0
NS2/MAC/Packer set SStime_Bits 0
NS2/MAC/Packer set Padding_Bits 0
NS2/MAC/Packer set debug_ 0

NS2/MAC/Uwpolling/Packer set t_in_Bits          4
NS2/MAC/Uwpolling/Packer set t_fin_Bits         16
NS2/MAC/Uwpolling/Packer set uid_trigger_Bits   5
NS2/MAC/Uwpolling/Packer set id_polled_Bits     6  ;#ID polled nodes
NS2/MAC/Uwpolling/Packer set backoff_time_Bits  16
NS2/MAC/Uwpolling/Packer set ts_Bits            16
NS2/MAC/Uwpolling/Packer set n_pkts_Bits        4
NS2/MAC/Uwpolling/Packer set uid_probe_Bits     16
NS2/MAC/Uwpolling/Packer set id_node_Bits       6
NS2/MAC/Uwpolling/Packer set uid_poll_Bits      16
NS2/MAC/Uwpolling/Packer set poll_time_Bits		16
NS2/MAC/Uwpolling/Packer set uid_sink_Bits      8
NS2/MAC/Uwpolling/Packer set uid_probe_sink_Bits      16
NS2/MAC/Uwpolling/Packer set uid_ack_Bits	    16
NS2/MAC/Uwpolling/Packer set uid_packet_Bits    16
NS2/MAC/Uwpolling/Packer set uid_last_packet_Bits      16
NS2/MAC/Uwpolling/Packer set ack_array_size_Bits      6
NS2/MAC/Uwpolling/Packer set ack_array_el_Bits      16
NS2/MAC/Uwpolling/Packer set ack_array_size      64
NS2/MAC/Uwpolling/Packer set sink_mac_      $opt(sink)
NS2/MAC/Uwpolling/Packer set debug_      0


UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 0

# UW/CBR/Packer set SN_bits 16
# UW/CBR/Packer set RFFT_bits 0
# UW/CBR/Packer set RFFT_VALID_bits 0
# UW/CBR/Packer set debug_ 0

# variables for the ahoi! modem's management
Module/UW/UwModem/AHOI set debug_        1
Module/UW/UwModem/AHOI set buffer_size   4096
Module/UW/UwModem/AHOI set max_read_size 4096
Module/UW/UwModem/AHOI set ID_           $opt(node)
Module/UW/UwModem/AHOI set parity_bit    0
Module/UW/UwModem/AHOI set stop_bit      1
Module/UW/UwModem/AHOI set flow_control  0
Module/UW/UwModem/AHOI set baud_rate     115200
Module/UW/UwModem/AHOI set modem_id      $opt(node)
Module/UW/UwModem/AHOI set max_n_retx    2
Module/UW/UwModem/AHOI set wait_delivery 3000
Module/UW/UwModem/AHOI set pck_duration  2000


#######

puts "AUV id = $opt(auv)"
puts "SINK id = $opt(sink)"
puts "(self) id = $opt(node)"

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
    global ns opt node_ app_ transport_ port_ routing_ ipif_ mac_ modem_ ipif_ mll_
    global uwal_ app_sink app_auv
    # build the NS-Miracle node
    set node_ [$ns create-M_Node]

    # define the module(s) you want to put in the node
    # APPLICATION LAYER
    if {$opt(node) == $opt(sink)} {
        set app_sink [new Module/UW/APPLICATION]
    } else {
  			set app_ [new Module/UW/APPLICATION]
    }
    
    # TRANSPORT LAYER
    set transport_ [new Module/UW/UDP]

    # NETWORK LAYER
    # Static Routing
    set routing_ [new Module/UW/StaticRouting]
	
    # IP interface
    set ipif_ [new Module/UW/IP]
	
    # DATA LINK LAYER - MEDIA LINK LAYER
    set mll_ [new Module/UW/MLL]
    
    # DATA LINK LAYER - MAC LAYER
	if {$opt(node) == $opt(sink)} {
    set mac_ [new Module/UW/POLLING/SINK]
  } else {
   	if {$opt(node) == $opt(auv)} {
      set mac_ [new Module/UW/POLLING/AUV]
    } else {
			set mac_ [new Module/UW/POLLING/NODE]
		}
  }
    set uwal_             [new Module/UW/AL]

	set modem_ [new Module/UW/UwModem/AHOI]

	$modem_ setLogLevel DBG

  # insert the module(s) into the node
	if {$opt(node) == $opt(sink)} {
		$node_ addModule 8 $app_sink 1 "UWA"
  } else {
		$node_ addModule 8 $app_ 1 "UWA"
  }
	$node_ addModule 7 $transport_ 1 "UDP"
	$node_ addModule 6 $routing_ 1 "IPR"
	$node_ addModule 5 $ipif_ 1 "IPIF"
	$node_ addModule 4 $mll_ 1 "ARP"  
	$node_ addModule 3 $mac_ 1 "POLL"
	$node_ addModule 2 $uwal_ 1 "UWAL"
	$node_ addModule 1 $modem_ 1 "AHOI" 

	if {$opt(node) == $opt(sink)} {
  	$node_ setConnection $app_sink $transport_ 1
  } else {
  		$node_ setConnection $app_ $transport_ 1
  }
	$node_ setConnection $transport_ $routing_ trace
  $node_ setConnection $routing_ $ipif_ trace
  $node_ setConnection $ipif_ $mll_ trace
  $node_ setConnection $mll_ $mac_ trace
  $node_ setConnection $mac_ $uwal_ trace
  $node_ setConnection $uwal_ $modem_ trace

	if {$opt(node) == $opt(sink)} {
    set port_ [$transport_ assignPort $app_sink]
  } else {
  		set port_ [$transport_ assignPort $app_]
  }

  $mac_ setMacAddr $opt(node)
	puts "MAC ADDR [$mac_ addr]"
  $ipif_ addr $opt(node)
  $modem_ set ID_ $opt(node)
  $modem_ setLogLevel DBG
  $modem_ setModemAddress $opt(i_path)
  $modem_ setLogSuffix ".log"

  # set packer for Adaptation Layer
  set packer_ [new UW/AL/Packer]

  set packer_payload0 [new NS2/COMMON/Packer]  
  #$packer_payload0 printAllFields  

  set packer_payload1 [new UW/IP/Packer]

  set packer_payload2 [new NS2/MAC/Packer]
  set packer_payload3 [new NS2/MAC/Uwpolling/Packer]
  set packer_payload4 [new UW/UDP/Packer]
  set packer_payload5 [new UW/APP/uwApplication/Packer]
  # $packer_payload3 printAllFields

  $packer_ addPacker $packer_payload0
  $packer_ addPacker $packer_payload1
  $packer_ addPacker $packer_payload2
  $packer_ addPacker $packer_payload3
  $packer_ addPacker $packer_payload4
  $packer_ addPacker $packer_payload5

  if {$opt(node) == $opt(sink)} {
    if {$opt(AppSocket) == 1} {
      $app_sink setSocketProtocol $opt(protocol)
      $app_sink set Socket_Port_ $opt(app_port)
    } else {
      $app_sink setSocketProtocol "NONE"
    }
    $app_sink set node_ID_  $opt(node)
  } else {
    if {$opt(AppSocket) == 1} {
      $app_ setSocketProtocol $opt(protocol)
      $app_ set Socket_Port_ $opt(app_port)
    } else {
      $app_ setSocketProtocol "NONE"
    }
    $app_ set node_ID_  $opt(node)
  }

  #$packer_ printMap
  #$packer_ printAllFields

  $uwal_ linkPacker $packer_
  
  $uwal_ set nodeID $opt(node)

  if {($opt(node) != $opt(sink)) && ($opt(node) != $opt(auv))} {
  	$mac_ set node_id_ $opt(node)
  }

  $mac_ initialize
  # $modem_ setLogSuffix ""
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
createNode

################################
# Inter-node module connection #
################################
# Put here all the commands required to connect nodes in the network (optional), namely, specify end to end connections, fill ARP tables, define routing settings

# connections at the application level
if {($opt(node) != $opt(sink)) && ($opt(node) != $opt(auv))} {
	# $app_ set destAddr_ $opt(sink)
  $app_ set destAddr_ $opt(auv)
    $app_ set destPort_ 1
    # # $app_ set destPort_ [expr $opt(node) - 2]
	# $routing_ addRoute $opt(sink) $opt(auv)
  $routing_ addRoute $opt(auv) $opt(auv)	
}

# if {$opt(node) == $opt(auv)} {
# 	$routing_ addRoute $opt(sink) $opt(sink)
# }

for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
  $mll_ addentry  [expr $cnt + 1] [expr $cnt + 1]
}
$mll_ addentry $opt(sink) $opt(sink)
$mll_ addentry $opt(auv) $opt(auv)



#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

$ns at 0 "$modem_ start"
$ns at $time_stop "$modem_ stop"

# if {($opt(node) != $opt(sink)) && ($opt(node) != $opt(auv))} {
if {($opt(node) != $opt(sink))} {
  $ns at $opt(start) "$app_ start"
  $ns at $opt(stop) "$app_ stop"
}

# if {$opt(node) == $opt(sink)} {
#   $ns at $opt(start) "$app_sink start"
#   $ns at $time_stop "$app_sink stop"
# }

if {$opt(node) == $opt(auv)} {
	$ns at $opt(start) "$mac_ run"
}


# proc printQueueAUV {} {
#   global mac_ opt

#   set tx_buffer_size        [$mac_ getTxBufferSize]
#   set temp_buffer_size      [$mac_ getTempBufferSize]
#   puts "TX BUFFER NODE($opt(node)): $tx_buffer_size"
#   puts "TEMP BUFFER NODE($opt(node)): $temp_buffer_size"
# }

# proc printQueueNODE {} {
#   global mac_ opt

#   set data_queue_size     [$mac_ getDataQueueSize]
#   puts "BUFFER NODE($opt(node)): $data_queue_size"
# }


set minute_base 60
if {$opt(node) == $opt(auv)} {
  for {set cnt 1} {$cnt < [expr $time_stop / $minute_base]} {incr cnt} {
    
    set minute [expr $minute_base * $cnt]
    # $ns at $minute "printQueueAUV"
  }
} else {
  if {$opt(node) != $opt(sink)} {
    for {set cnt 1} {$cnt < [expr $time_stop / 60]} {incr cnt} {
      
      set minute [expr $minute_base * $cnt]
      $ns at $minute "$mac_ getDataQueueLog"
    }
  }
}


###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
   
   global ns tf tf_name	mac_ opt app_ app_sink
   # computation of the statics

   # display messages
   if {$opt(node) == $opt(auv)} {
	   puts "==============================================="
	   puts "AUV"
	   puts "==============================================="
	   set mac_auv_trigger_sent	[$mac_ getTriggerSent]
	   set mac_auv_probe_rx	[$mac_ getProbeReceived]
	   set mac_auv_dropped_probe	[$mac_ getDroppedProbePkts]
	   set mac_auv_poll_sent [$mac_ getPollSent]
	   set mac_auv_data_received [$mac_ getDataPktsRx]
	   set mac_auv_sent_pkts   [$mac_ getDataPktsTx] 
	   set mac_auv_pkts_buff_overflow [$mac_ getDiscardedPktsTx]
	   puts "Trigget sent: $mac_auv_trigger_sent"
	   puts "Probe received: $mac_auv_probe_rx"
	   puts "Probe dropped: $mac_auv_dropped_probe"
	   puts "Poll sent: $mac_auv_poll_sent"	
	   puts "Data received: $mac_auv_data_received"	   
	   puts "Data sent: $mac_auv_sent_pkts"
	   puts "Data packet overflow: $mac_auv_pkts_buff_overflow"
   } else  {
	   if {$opt(node) == $opt(sink)} {
		   puts "==============================================="
	   	   puts "SINK"
	       puts "==============================================="
           set mac_sink_rcv_pkts  [$mac_ getDataPktsRx]
	       set mac_sink_duplicated	[$mac_ getDuplicatedPkts]
	       set mac_sink_probe_sent [$mac_ getProbeSent]
	       set mac_sink_trigger_rx		[$mac_ getTriggerReceived]
         set data_received [$app_sink getrecvpkts]
			   puts "Data received app: $data_received"

		   puts "Trigger received: $mac_sink_trigger_rx"
           puts "Probe sent: $mac_sink_probe_sent"
		   puts "Data received: $mac_sink_rcv_pkts"
		   puts "Data duplicated: $mac_sink_duplicated"

	   } else {
		   puts "==============================================="
	       puts "NODE"
	       puts "==============================================="
		   set data_sent         	[$app_ getsentpkts]
		   set mac_tx_pkts         	[$mac_ getDataPktsTx]
		   set mac_probe_sent		[$mac_ getProbeSent]
		   set mac_trigger_rx		[$mac_ getTriggerReceived]
		   set mac_poll_rx			[$mac_ getTimesPolled]
		   puts "Trigger received: $mac_trigger_rx"
       puts "Probe sent: $mac_probe_sent"
		   puts "Poll received: $mac_poll_rx"
		   puts "Data sent: $mac_tx_pkts"
		   puts "Data sent app: $data_sent"
	   }
   }
   puts "done!"
   puts "tracefile: $tf_name"

   # save traces
   $ns flush-trace
   
   # close files
   close $tf
}

##################
# Run simulation #
##################
# Specify the time at which to call the finish procedure and halt ns

$ns at $time_stop  "finish; $ns halt"

# You always need the following line to run the NS-Miracle simulator
$ns run
