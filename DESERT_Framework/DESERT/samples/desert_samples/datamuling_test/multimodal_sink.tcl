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
# File: multimodal_sink.tcl

# This node is responsible for relaying the data received from the acoustic
# modem to the CSA modem

# The opt() list contains many different options for controlling script execution

# if set to 1 the Application listen from the socket port provided in input
set opt(AppSocket)  1;
# Protocol to use for the Application socket, TCP or UDP
set opt(protocol) "TCP" ;
# Select if unidirectional transmission: from UW node to shore node
set opt(direction) "unidirectional";
# Set network addresses
set opt(net_src) 2
set opt(net_dst) 3

##############################
# Terminal's parameter check #
##############################
if {$opt(AppSocket) == 1} {
  if {$argc != 10} {
	puts "Underwater Sink: The script needs 10 input to work"
    puts "1 - Modem ID of current node"
    puts "2 - Modem ID of the destination node"
    puts "3 - Start time"
    puts "4 - Stop time"
    puts "5 - Packet generation period (0 if the node doesn't generate data)"
    puts "6 - Path to the serial iface"
	puts "7 - listening port for CSA/TCP"
    puts "8 - Application socket port"
    puts "9 - Maximum socket read size"
    puts "10 - Experiment ID"
    puts "Please try again."
    exit
  } else {
    set opt(phy_node) [lindex $argv 0]
    set opt(phy_dest) [lindex $argv 1]
    set opt(start)    [lindex $argv 2]
    set opt(stop)     [lindex $argv 3]
    set opt(traffic)  [lindex $argv 4]
	set opt(i_path)   [lindex $argv 5]
	set opt(address)  [lindex $argv 6]
    set opt(app_port) [lindex $argv 7]
    set opt(max_read) [lindex $argv 8]
    set opt(exp_ID)   [lindex $argv 9]
  }
} else {
  if {$argc != 8} {
    puts "Multimodal Sink: The script needs 8 input to work"
    puts "1 - Modem ID of current node"
    puts "2 - Modem ID of the destination node"
    puts "3 - Start time"
    puts "4 - Stop time"
    puts "5 - Packet generation period (0 if the node doesn't generate data)"
    puts "6 - Path to the serial iface"
    puts "7 - Payload size (bytes)"
    puts "8 - Experiment ID"
    puts "Please try again."
    exit
  } else {
    set opt(phy_node)     [lindex $argv 0]
    set opt(phy_dest)     [lindex $argv 1]
    set opt(start)        [lindex $argv 2]
    set opt(stop)         [lindex $argv 3]
    set opt(traffic)      [lindex $argv 4]
    set opt(i_path)       [lindex $argv 5]
    set opt(payload_size) [lindex $argv 6]
    set opt(exp_ID)       [lindex $argv 7]
  }
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
# load libuwtdma.so
load libuwcsmaaloha.so
load libpackeruwudp.so
load libuwphy_clmsgs.so
load libuwconnector.so
load libuwmodem.so
load libuwahoimodem.so
load libuwmodemcsa.so
load libuwmulti_destination.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

# Declare the use of a Real Time Schedule
# (necessary for the interfacing with real hardware)
$ns use-scheduler RealTime

##################
# Tcl variables  #
##################

# time when actually to stop the simulation
set time_stop [expr "$opt(stop)+5"]

#Trace file name
set tf_name "./multimodal_test_sink.tr"

#Open a file for writing the trace data
set tf [open $tf_name w]
$ns trace-all $tf

# Set a random seed generator
set rng [new RNG]
$rng seed $opt(phy_node)

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng

#########################
# Module Configuration  #
#########################
# Put here all the commands to set globally the initialization values of
# the binded variables (optional)

# variables for the packer(s)
# AL packer
UW/AL/Packer set SRC_ID_Bits 8
UW/AL/Packer set PKT_ID_Bits 8
UW/AL/Packer set FRAME_OFFSET_Bits 1 ;#0
UW/AL/Packer set M_BIT_Bits 1
UW/AL/Packer set DUMMY_CONTENT_Bits 0
UW/AL/Packer set debug_ 1
# Common packer
NS2/COMMON/Packer set PTYPE_Bits 16
NS2/COMMON/Packer set SIZE_Bits 8 ;# CHECK THISS!!!!
NS2/COMMON/Packer set UID_Bits 8 ; #0
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 8
NS2/COMMON/Packer set PREV_HOP_Bits 8
NS2/COMMON/Packer set NEXT_HOP_Bits 8
NS2/COMMON/Packer set ADRR_TYPE_Bits 0
NS2/COMMON/Packer set LAST_HOP_Bits 0
NS2/COMMON/Packer set TXTIME_Bits 0
NS2/COMMON/Packer set debug_ 10
# IP packer
UW/IP/Packer set SAddr_Bits 8
UW/IP/Packer set DAddr_Bits 8
UW/IP/Packer set debug_ 1
# MAC packer
NS2/MAC/Packer set Ftype_Bits 0
NS2/MAC/Packer set SRC_Bits 8
NS2/MAC/Packer set DST_Bits 8
NS2/MAC/Packer set Htype_Bits 0
NS2/MAC/Packer set TXtime_Bits 0
NS2/MAC/Packer set SStime_Bits 0
NS2/MAC/Packer set Padding_Bits 0
NS2/MAC/Packer set debug_ 1
# UDP packer
UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 1
# uwApplication packer
UW/APP/uwApplication/Packer set SN_FIELD_ 16
UW/APP/uwApplication/Packer set RFFT_FIELD_ 0
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 0
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 1
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 8
UW/APP/uwApplication/Packer set debug_ 1

# variables for the APPLICATION module 
Module/UW/APPLICATION set debug_ -1
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

# variables for the MULTI_DESTINATION module
Module/UW/MULTI_DESTINATION set debug_ 10

# variables controlling the CSMA/Aloha
# Module/UW/TDMA set fair_mode            0
# Module/UW/TDMA set tot_slots            1
# Module/UW/TDMA set max_packet_per_slot  0
# Module/UW/TDMA set queue_size_          4
# Module/UW/TDMA set drop_old_            0
# Module/UW/TDMA set sea_trial_           1
# Module/UW/TDMA set debug_               1
Module/UW/CSMA_ALOHA set max_tx_tries_      0
Module/UW/CSMA_ALOHA set wait_costant_      0.0001
Module/UW/CSMA_ALOHA set debug_             0
Module/UW/CSMA_ALOHA set backoff_tuner_     0
Module/UW/CSMA_ALOHA set max_payload_       1250
Module/UW/CSMA_ALOHA set ACK_timeout_       1.0
Module/UW/CSMA_ALOHA set alpha_             0.8
Module/UW/CSMA_ALOHA set buffer_pkts_       -1
Module/UW/CSMA_ALOHA set max_backoff_counter_ 0
Module/UW/CSMA_ALOHA set listen_time_       0.0001
Module/UW/CSMA_ALOHA set MAC_addr_          0

# variables for the AL module
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 1400
Module/UW/AL set debug_ 0
Module/UW/AL set interframe_period 0.e1
Module/UW/AL set frame_set_validity 0
Module/UW/AL set frame_padding 0
Module/UW/AL set force_endTx 0

# variables for the ahoi! modem's management
Module/UW/UwModem/AHOI set debug_		 1
Module/UW/UwModem/AHOI set buffer_size   4096
Module/UW/UwModem/AHOI set max_read_size 4096
Module/UW/UwModem/AHOI set ID_           $opt(phy_node)
Module/UW/UwModem/AHOI set parity_bit    0
Module/UW/UwModem/AHOI set stop_bit      1
Module/UW/UwModem/AHOI set flow_control  0
Module/UW/UwModem/AHOI set baud_rate     115200
Module/UW/UwModem/AHOI set modem_id      $opt(phy_node)
Module/UW/UwModem/AHOI set max_n_retx    2
Module/UW/UwModem/AHOI set wait_delivery 3000
Module/UW/UwModem/AHOI set pck_duration  2000

# variables for the CSA/TCP modem's management
Module/UW/UwModem/ModemCSA set debug_ 1
Module/UW/UwModem/ModemCSA set max_read_size 64
Module/UW/UwModem/ModemCSA set buffer_size 4096


Module/UW/IP set debug_ 11
Module/UW/StaticRouting set debug_ 11
################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
	global ns opt socket_port node_ app_sink modem_a modem_t multidest_ mac_addr
	global app_ transport_ port_ routing_ ipif_ mac1_ mac2_ ipif_ mll1_ mll2_ uwal1_ uwal2_

	    # build the NS-Miracle node
    set node_ [$ns create-M_Node]

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

	# MULTIDESTINATION module
	set multidest_ [new Module/UW/MULTI_DESTINATION]
	
    # DATA LINK LAYER - MEDIA LINK LAYERS
    set mll1_ [new Module/UW/MLL]
	set mll2_ [new Module/UW/MLL]
    
    # DATA LINK LAYER - MAC LAYER
	# Module/UW/TDMA set frame_duration 10
    # Module/UW/TDMA set guard_time     1
    set mac1_ [new Module/UW/CSMA_ALOHA]
	$mac1_ setNoAckMode
    $mac1_ initialize
	# Module/UW/TDMA set frame_duration 10
    # Module/UW/TDMA set guard_time     1
	# # variables controlling the TDMA
	# Module/UW/TDMA set fair_mode            0
	# Module/UW/TDMA set tot_slots            1
	# Module/UW/TDMA set max_packet_per_slot  10
	# Module/UW/TDMA set queue_size_          4
	# Module/UW/TDMA set drop_old_            0
	# Module/UW/TDMA set sea_trial_           1
	# Module/UW/TDMA set debug_               1
	set mac2_ [new Module/UW/CSMA_ALOHA]
	$mac2_ setNoAckMode
    $mac2_ initialize

	# ADAPTATION LAYER
    set uwal1_ [new Module/UW/AL]
	set uwal2_ [new Module/UW/AL]

    # PHY LAYER: ACOUSTIC
    set modem_a [new Module/UW/UwModem/AHOI]

	# PHY LAYER: CSA/TCP
    set modem_t [new Module/UW/UwModem/ModemCSA]

	puts "Creating Sink node (2)..."

	# insert the module(s) into the node
	$node_ addModule 13 $app_ 1 "UWA"
	$node_ addModule 12 $transport_ 1 "UDP"
	$node_ addModule 11 $routing_ 1 "IPR"
	$node_ addModule 10 $ipif_ 1 "IPIF"
    $node_ addModule 9 $multidest_ 1 "CTR"
	$node_ addModule 8 $mll1_ 1 "MLL"
	$node_ addModule 7 $mll2_ 1 "MLL"
	$node_ addModule 6 $mac1_ 1 "ALOHA1"
	$node_ addModule 5 $mac2_ 1 "ALOHA2"
	$node_ addModule 4 $uwal1_ 1 "UWAL"
	$node_ addModule 3 $uwal2_ 1 "UWAL"
	$node_ addModule 2 $modem_a 0 "AHOI"
	$node_ addModule 1 $modem_t 0 "CSA" 

	$node_ setConnection $app_ $transport_ trace
    $node_ setConnection $transport_ $routing_ trace
    $node_ setConnection $routing_ $ipif_ trace
	$node_ setConnection $ipif_ $multidest_ trace
	$node_ setConnection $multidest_ $mll1_ trace
    $node_ setConnection $multidest_ $mll2_ trace
    $node_ setConnection $mll1_ $mac1_ trace
	$node_ setConnection $mll2_ $mac2_ trace
    $node_ setConnection $mac1_ $uwal1_ trace
	$node_ setConnection $mac2_ $uwal2_ trace
    $node_ setConnection $uwal1_ $modem_a trace
	$node_ setConnection $uwal2_ $modem_t trace

	# assign a port number to the application considered (CBR or VBR)
    set port_ [$transport_ assignPort $app_]
    $ipif_ addr $opt(net_src)

	# set the MAC addresses
    set mac_addr1 $opt(phy_node)
    set mac_addr2 66

	# set MAC address fo the 2 MAC layers
    $mac1_ setMacAddr $mac_addr1
	# $mac1_ setSlotDuration 1
	# $mac1_ setStartTime 9
	# $mac1_ setSlotNumber 2
	$mac2_ setMacAddr $mac_addr2
	# $mac2_ setSlotDuration 10
	# $mac2_ setStartTime 0

	# configure acoustic ahoi moedm
    $modem_a set ID_ $opt(phy_node)
    $modem_a setLogLevel DBG
    $modem_a setModemAddress $opt(i_path)
    $modem_a setLogSuffix "-ahoi-$opt(exp_ID).log"

	# configure the CSA/TCP modem
	$modem_t set ID_ 66
	puts "DIOCANN::::address::$opt(address)"
	$modem_t setModemAddress $opt(address)
    $modem_t setLogLevel "DBG"
	$modem_t setServer
    $modem_t setTCP

    # set packer for Adaptation Layer
    set packer1_ [new UW/AL/Packer]
	set packer2_ [new UW/AL/Packer]

    set packer_payload10 [new NS2/COMMON/Packer]
    set packer_payload11 [new UW/IP/Packer]
    set packer_payload12 [new NS2/MAC/Packer]
    set packer_payload13 [new UW/UDP/Packer]
    set packer_payload14 [new UW/APP/uwApplication/Packer]
    $packer_payload14 printMap

	set packer_payload20 [new NS2/COMMON/Packer]
    set packer_payload21 [new UW/IP/Packer]
    set packer_payload22 [new NS2/MAC/Packer]
    set packer_payload23 [new UW/UDP/Packer]
    set packer_payload24 [new UW/APP/uwApplication/Packer]
    $packer_payload24 printMap

    $packer1_ addPacker $packer_payload10
    $packer1_ addPacker $packer_payload11
    $packer1_ addPacker $packer_payload12
    $packer1_ addPacker $packer_payload13
    $packer1_ addPacker $packer_payload14

	$packer2_ addPacker $packer_payload20
    $packer2_ addPacker $packer_payload21
    $packer2_ addPacker $packer_payload22
    $packer2_ addPacker $packer_payload23
    $packer2_ addPacker $packer_payload24
	
    if {$opt(AppSocket) == 1} {
        $app_ setSocketProtocol $opt(protocol)
        $app_ set Socket_Port_ $opt(app_port)
    } else {
      $app_ setSocketProtocol "NONE"
    }
    $app_ set node_ID_  2
    $app_ print_log

    $uwal1_ linkPacker $packer1_
	$uwal2_ linkPacker $packer2_
    
    $uwal1_ set nodeID 12
	$uwal2_ set nodeID 66

	# set multi-destination manual switch
	$multidest_ setManualLowerlId [$mll2_ Id_]
	$multidest_ setManualSwitch
	#$multidest_ addLayer [$mll1_ Id_] 1
	#$multidest_ addLayer [$mll2_ Id_] 2
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
createNode

################################
# Inter-node module connection #
################################
# Put here all the commands required to connect nodes in the network (optional), namely,
# specify end to end connections, fill ARP tables, define routing settings

# connections at the application level
$app_ set destAddr_ 4
$app_ set destPort_ 1

# flll the ARP tables, connecting nodes
$routing_ addRoute $opt(net_dst) $opt(net_dst)
$mll1_ addentry  $opt(net_dst) $opt(phy_dest)
$mll1_ addentry  $opt(net_src) 12
$mll2_ addentry  $opt(net_dst) 67
$mll2_ addentry  $opt(net_src) 66

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

# $ns at 0 "$mac1_ start"
# $ns at 0 "$mac2_ start"
$ns at 0 "$modem_a start"
$ns at 0 "$modem_t start"

if {$opt(traffic) != 0} {
  $ns at $opt(start) "$app_ start"
  $ns at $opt(stop)  "$app_ stop"
}

$ns at $time_stop "$modem_a stop"
$ns at $time_stop "$modem_t stop"
# $ns at $time_stop "$mac1_ stop"
# $ns at $time_stop "$mac2_ stop"

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
   
   global ns tf tf_name	
   # computation of the statics

   # display messages
   puts "done!"
   #puts "tracefile: $tf_name"

   # save traces
   $ns flush-trace
   
   # close files
   close $tf
}

##################
# Run simulation #
##################
# Specify the time at which to call the finish procedure and halt ns

$ns at [expr $time_stop] "finish; $ns halt"

# You always need the following line to run the NS-Miracle simulator
$ns run


