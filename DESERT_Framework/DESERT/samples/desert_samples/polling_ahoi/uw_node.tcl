# Copyright (c) 2020 Regents of the SIGNET lab, University of Padova.
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
# Author: Roberto Francescon
# Version: 1.0.0
# File: uw_node.tcl
#

# This script is still at a prototypica stage: please check assignments of NET, PHY and MAC
# addresses when you run it. The setting expects 3 AHOI nodes: 16, 12 nad 13. To these nodes
# we intended to assign, respectively, the net addresses 1, 2 and 3. Check.

# if set to 1 the Application listen from the socket port provided in input
set opt(AppSocket)  1;
# Protocol to use for the Application socket, TCP or UDP
set opt(protocol) "TCP";
# If AppSocket is active send data as fast as possible
if {$opt(AppSocket)} {
	set opt(traffic) 0.01;
}
# Set uwpolling parameters
set opt(rep_num) 2

##############################
# Terminal's parameter check #
##############################
if {$opt(AppSocket) == 1} {
  if {$argc != 9} {
	puts "Underwater Node: The script needs 8 input to work"
    puts "1 - Modem ID of the UW-NODE (current)"
	puts "2 - Modem ID of the AUV"
	puts "3 - Modem ID of the SINK"
    puts "3 - Start time"
    puts "4 - Stop time"
    puts "5 - Path to the serial iface"
    puts "6 - Application socket port"
    puts "7 - Maximum socket read size"
    puts "8 - Experiment ID"
    puts "Please try again."
    exit
  } else {
    set opt(phy_addr)      [lindex $argv 0]
	set opt(auv_phy_addr)  [lindex $argv 1]
	set opt(sink_phy_addr) [lindex $argv 2]
    set opt(start)         [lindex $argv 3]
    set opt(stop)          [lindex $argv 4]
    set opt(i_path)        [lindex $argv 5]
    set opt(app_port)      [lindex $argv 6]
    set opt(max_read)      [lindex $argv 7]
    set opt(exp_ID)        [lindex $argv 8]
  }
} else {
  if {$argc != 9} {
    puts "Multimodal Node: The script needs 8 input to work"
    puts "1 - Modem ID of the UW-NODE (current)"
	puts "2 - Modem ID of the AUV"
	puts "3 - Modem ID of the SINK"
    puts "4 - Start time"
    puts "5 - Stop time"
    puts "6 - Packet generation period (0 if the node doesn't generate data)"
    puts "7 - Path to the serial iface"
    puts "8 - Payload size (bytes)"
    puts "9 - Experiment ID"
    puts "Please try again."
    exit
  } else {
    set opt(phy_addr)      [lindex $argv 0]
	set opt(auv_phy_addr)  [lindex $argv 1]
	set opt(sink_phy_addr) [lindex $argv 2]
    set opt(start)         [lindex $argv 3]
    set opt(stop)          [lindex $argv 4]
    set opt(traffic)       [lindex $argv 5]
    set opt(i_path)        [lindex $argv 6]
    set opt(payload_size)  [lindex $argv 7]
    set opt(exp_ID)        [lindex $argv 8]
  }
}

# Set network addresses
if {$opt(auv_phy_addr) != $opt(sink_phy_addr)} {
    set opt(net_addr)      1
    set opt(auv_net_addr)  2
    set opt(sink_net_addr) 3
} else {
    set opt(net_addr)      1
    set opt(auv_net_addr)  2
    set opt(sink_net_addr) 2
}

proc color {foreground text} {
    # tput is a little Unix utility that lets you use the termcap database
    # *much* more easily...
    return [exec tput setaf $foreground]$text[exec tput sgr0]
}

#####################
# Library Loading   #
#####################
# Load here all the NS-Miracle libraries you need
load libMiracle.so
load libmphy.so
load libmmac.so
load libuwip.so
load libuwmll.so
load libuwstaticrouting.so
load libuwudp.so
load libuwapplication.so
load libpackeruwapplication.so
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libuwcbr.so
load libuwmmac_clmsgs.so
load libuwcsmaaloha.so
load libpackeruwudp.so
load libuwphy_clmsgs.so
load libuwpolling.so
load libpackeruwpolling.so
load libuwconnector.so
load libuwmodem.so
load libuwahoimodem.so
load libuwmulti_destination.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

# Declare the use of a Real Time Schedule (necessary for the interfacing with real hardware)
$ns use-scheduler RealTime

##################
# Tcl variables  #
##################

# Trace file: set name and open a file for writing the trace data
set tf_name "./polling_uwnode.tr"
set tracefile [open $tf_name w]
$ns trace-all $tracefile

# Instantiate a random core and set seed
set rng [new RNG]
$rng seed         $opt(phy_addr)

# Build a Uniform random generator
set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng

# Set nuber of nodes to build ARP and routing tables
set opt(nn)  3

##########################
# Modules Configuration  #
##########################
# Put here all the commands to set globally the initialization values of
# the binded variables (optional)

# Variables for the uwApplication module
Module/UW/APPLICATION set debug_ 0
Module/UW/APPLICATION set period_ $opt(traffic)
Module/UW/APPLICATION set PoissonTraffic_ 0
if {$opt(AppSocket) == 1} {
  Module/UW/APPLICATION set Socket_Port_ $opt(app_port)
  Module/UW/APPLICATION set max_read_length $opt(max_read)
} else {
  Module/UW/APPLICATION set Payload_size_ $opt(payload_size)
}
Module/UW/APPLICATION set drop_out_of_order_ 1    
Module/UW/APPLICATION set pattern_sequence_ 0     
Module/UW/APPLICATION set EXP_ID_ $opt(exp_ID)

# Variables for the uwpolling module
#Module/UW/POLLING/NODE set T_poll_                [expr $opt(T_backoff) + 5] ;#has to be bigger than T_probe(AUV)
Module/UW/POLLING/NODE set T_poll_guard_          7 ;#has to be bigger than T_probe(AUV)
Module/UW/POLLING/NODE set backoff_tuner_         1
Module/UW/POLLING/NODE set max_payload_           $opt(max_read)
Module/UW/POLLING/NODE set buffer_data_pkts_      50
Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_      5
Module/UW/POLLING/NODE set intra_data_guard_time_ 0.001
Module/UW/POLLING/NODE set useAdaptiveTpoll_      1
Module/UW/POLLING/NODE set n_run                  $opt(rep_num);#used for c++ rng
Module/UW/POLLING/NODE set debug_		          1
Module/UW/POLLING/NODE set node_id_		          13
Module/UW/POLLING/NODE set sea_trial_		      1
# Module/UW/POLLING/NODE set ack_enabled_		      0
Module/UW/POLLING/NODE set modem_data_bit_rate_	  200

# Variables for the AL module
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 1400
Module/UW/AL set debug 0
Module/UW/AL set interframe_period 0.e1
Module/UW/AL set frame_set_validity 0

# Variables for the ahoi! modem's management
Module/UW/UwModem/AHOI set debug_		 0
Module/UW/UwModem/AHOI set buffer_size   4096
Module/UW/UwModem/AHOI set max_read_size 4096
Module/UW/UwModem/AHOI set ID_           $opt(phy_addr)
Module/UW/UwModem/AHOI set parity_bit    0
Module/UW/UwModem/AHOI set stop_bit      1
Module/UW/UwModem/AHOI set flow_control  0
Module/UW/UwModem/AHOI set baud_rate     115200
Module/UW/UwModem/AHOI set modem_id      $opt(phy_addr)
Module/UW/UwModem/AHOI set max_n_retx    2
Module/UW/UwModem/AHOI set wait_delivery 3000
Module/UW/UwModem/AHOI set pck_duration  2000

##########################
# Packers Configuration  #
##########################
# Put here all the commands to set globally the initialization values of
# the binded variables (optional)

# Variables for the Application packer
UW/APP/uwApplication/Packer set SN_FIELD_ 16
UW/APP/uwApplication/Packer set RFFT_FIELD_ 0
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 0
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 1
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 8
UW/APP/uwApplication/Packer set debug_ 0

# Variables for UDP packer settings
UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 0

# Variables for IP packer settings
UW/IP/Packer set SAddr_Bits 8
UW/IP/Packer set DAddr_Bits 8
UW/IP/Packer set debug_ 0

# Variables for the AL packer
UW/AL/Packer set SRC_ID_Bits 8
UW/AL/Packer set PKT_ID_Bits 8
UW/AL/Packer set FRAME_OFFSET_Bits 1 ;#0
UW/AL/Packer set M_BIT_Bits 1
UW/AL/Packer set DUMMY_CONTENT_Bits 0
UW/AL/Packer set debug_ 0

# Variables for the MAC packer settings
NS2/MAC/Packer set Ftype_Bits 0
NS2/MAC/Packer set SRC_Bits 8
NS2/MAC/Packer set DST_Bits 8
NS2/MAC/Packer set Htype_Bits 0
NS2/MAC/Packer set TXtime_Bits 0
NS2/MAC/Packer set SStime_Bits 0
NS2/MAC/Packer set Padding_Bits 0
NS2/MAC/Packer set debug_ 0

# Variables for the polling packer settings
NS2/MAC/Uwpolling/Packer set t_in_Bits            16
NS2/MAC/Uwpolling/Packer set t_fin_Bits           16
NS2/MAC/Uwpolling/Packer set uid_TRIGGER_Bits     16
NS2/MAC/Uwpolling/Packer set id_polled_Bits       8
NS2/MAC/Uwpolling/Packer set backoff_time_Bits    16
NS2/MAC/Uwpolling/Packer set ts_Bits              16
NS2/MAC/Uwpolling/Packer set n_pkts_Bits          16
NS2/MAC/Uwpolling/Packer set uid_PROBE_Bits       16
NS2/MAC/Uwpolling/Packer set id_node_Bits         8
NS2/MAC/Uwpolling/Packer set uid_POLL_Bits        8
NS2/MAC/Uwpolling/Packer set poll_time_Bits       8
NS2/MAC/Uwpolling/Packer set uid_sink_Bits        8
NS2/MAC/Uwpolling/Packer set uid_probe_sink_Bits  8
NS2/MAC/Uwpolling/Packer set uid_PROBE_Bits       8
NS2/MAC/Uwpolling/Packer set sink_mac_			  13
NS2/MAC/Uwpolling/Packer set debug_				  1

# Common Packer settings
NS2/COMMON/Packer set PTYPE_Bits 16
NS2/COMMON/Packer set SIZE_Bits 8 ;#
NS2/COMMON/Packer set UID_Bits 8 ; #0
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 8
NS2/COMMON/Packer set PREV_HOP_Bits 8
NS2/COMMON/Packer set NEXT_HOP_Bits 8
NS2/COMMON/Packer set ADRR_TYPE_Bits 0
NS2/COMMON/Packer set LAST_HOP_Bits 0
NS2/COMMON/Packer set TXTIME_Bits 0
NS2/COMMON/Packer set debug_ 0

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
	global ns opt socket_port node_ address tracefile
	global app_ transport_ port_ routing_ ipif_ mac_ modem_ ipif_ mll_ uwal_ app_sink

	    # build the NS-Miracle node
    set node_ [$ns create-M_Node $tracefile]

    # define the module(s) you want to put in the node
    # APPLICATION LAYER
    set app_ [new Module/UW/APPLICATION]
    
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
	set mac_  [new Module/UW/POLLING/NODE]
    # set mac_ [new Module/UW/CSMA_ALOHA]; #[new Module/UW/TDMA]
	# $mac_ setNoAckMode
    # $mac_ initialize

    set uwal_ [new Module/UW/AL]

    # PHY LAYER
    set modem_ [new Module/UW/UwModem/AHOI]

    puts "Creating UW node (NET:$opt(net_addr), PHY:$opt(phy_addr))..."

    # insert the module(s) into the node
    $node_ addModule 8 $app_ 1 "UWA"
    $node_ addModule 7 $transport_ 1 "UDP"
    $node_ addModule 6 $routing_ 1 "IPR"
    $node_ addModule 5 $ipif_ 1 "IPIF"
    $node_ addModule 4 $mll_ 1 "ARP"  
    $node_ addModule 3 $mac_ 1 "POLL"
    $node_ addModule 2 $uwal_ 1 "UWAL"
    $node_ addModule 1 $modem_ 1 "AHOI" 

    $node_ setConnection $app_ $transport_ trace
    $node_ setConnection $transport_ $routing_ trace
    $node_ setConnection $routing_ $ipif_ trace
    $node_ setConnection $ipif_ $mll_ trace
    $node_ setConnection $mll_ $mac_ trace
    $node_ setConnection $mac_ $uwal_ trace
    $node_ setConnection $uwal_ $modem_ trace

    # assign a socket port for the APP
    set port_ [$transport_ assignPort $app_]
    # assign network address
    $ipif_ addr $opt(net_addr)
    # assign MAC address (many times, apparently)
    set mac_addr $opt(phy_addr)
    $mac_ setMacAddr $mac_addr
    $mac_ set node_id_ $opt(phy_addr)
    $mac_ initialize
    # configure modem
    $modem_ set ID_ $opt(phy_addr)
    $modem_ setLogLevel DBG
    $modem_ setModemAddress $opt(i_path)
    $modem_ setLogSuffix "-$opt(exp_ID).log"

    # set packer for Adaptation Layer
    set packer_ [new UW/AL/Packer]

    set packer_payload0 [new NS2/COMMON/Packer]  
    #$packer_payload0 printAllFields  
    set packer_payload1 [new UW/IP/Packer]
    set packer_payload2 [new NS2/MAC/Packer]
    set packer_payload3 [new NS2/MAC/Uwpolling/Packer]
    set packer_payload4 [new UW/UDP/Packer]
    set packer_payload5 [new UW/APP/uwApplication/Packer]
    # $packer_payload3 printMap
    # $packer_payload0 printMap
	#$packer_payload4 printAllFields

    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4
	$packer_ addPacker $packer_payload5
    if {$opt(AppSocket) == 1} {
        $app_ setSocketProtocol $opt(protocol)
        $app_ set Socket_Port_ $opt(app_port)
    } else {
      $app_ setSocketProtocol "NONE"
    }
    $app_ set node_ID_  $opt(net_addr); #assign net address to app address
    $app_ print_log

    $uwal_ linkPacker $packer_
    
    $uwal_ set nodeID $opt(phy_addr); #assign modem ID to AL ID

	$mac_ initialize
	
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
$app_ set destAddr_ [expr $opt(sink_net_addr)]
$app_ set destPort_ 1

# Set routes among nodes
$routing_ addRoute $opt(auv_net_addr) $opt(net_addr)
$routing_ addRoute $opt(auv_net_addr) $opt(auv_net_addr)
$routing_ addRoute $opt(sink_net_addr) $opt(auv_net_addr)
# Fill the ARP table
$mll_ addentry  $opt(auv_net_addr) $opt(auv_phy_addr)
$mll_ addentry  $opt(net_addr) $opt(phy_addr)

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

$ns at 0 "$modem_ start"

if {$opt(traffic) != 0} {
  $ns at $opt(start) "$app_ start"
  $ns at $opt(stop) "$app_ stop"
}

set time_stop  [expr "$opt(stop)+5"]
$ns at $time_stop "$modem_ stop"

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
   
   global ns tracefile tf_name	
   # computation of the statics

   # display messages
   puts "[color 1 Finished!]"
   #puts "tracefile: $tf_name"

   # save traces
   $ns flush-trace
   
   # close files
   close $tracefile
}

##################
# Run simulation #
##################
# Specify the time at which to call the finish procedure and halt ns

puts -nonewline "\n[color 2 Started!]\n"

$ns at [expr $time_stop] "finish; $ns halt"

# You always need the following line to run the NS-Miracle simulator
$ns run

