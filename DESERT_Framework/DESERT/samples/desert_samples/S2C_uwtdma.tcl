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
# Author: Federico Favaro, Filippo Campagnaro
# Version: 1.0.0
#
######################
# Simulation Options #
######################
set opt(n_node) 5
if {$opt(n_node) <= 0} {
  puts "Number of nodes equal to zero! Please put a number of nodes >= 2"
  exit(1)
}

if {$opt(n_node) <= 1} {
  puts "WARNING!! number of nodes set to 1. Put a number of nodes >= 2"
}

# Terminal's parameter check
if {$argc != 7} {
  puts "The script needs 7 input to work"
  puts "1 - ID of the node"
  puts "2 - Start time"
  puts "3 - Stop time"
  puts "4 - CBR period (time in seconds between two consecutive packets)"
  puts "5 - ID of the experiment"
  puts "6 - IP of the modem"
  puts "7 - Port of the modem"
  puts "Please try again."
  exit(1)
} else {
  set opt(node)     [lindex $argv 0]
  set opt(start)    [lindex $argv 1]
  set opt(stop)     [lindex $argv 2]
  set opt(traffic)  [lindex $argv 3]
  set opt(n_run)    [lindex $argv 4]
  set opt(ip)       [lindex $argv 5]
  set opt(port)     [lindex $argv 6]
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
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so
load libpackeruwcbr.so
load libuwmphy_modem.so
load libmstwoc_evologics.so
load libuwtdma.so

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
# #Socket Ports to be used
set socket_port "${opt(ip)}:${opt(port)}"

set adrMAC $opt(node)

# time when actually to stop the simulation
set time_stop [expr "$opt(stop)+15"]

#Trace file name
set tf_name "S2C_Evologics_Uwtdma.tr"

#Open a file for writing the trace data
set tf [open $tf_name w]
$ns trace-all $tf

set rng [new RNG]
$rng seed         $opt(node)

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng

#########################
# Module Configuration  #
#########################
# Put here all the commands to set globally the initialization values of the binded variables (optional)

# variables for the CBR module
Module/UW/CBR set packetSize_          125;#13
Module/UW/CBR set period_              $opt(traffic)
Module/UW/CBR set PoissonTraffic_      0
Module/UW/CBR set debug_               0

Module/UW/TDMA set frame_duration   12
Module/UW/TDMA set debug_           10
Module/UW/TDMA set sea_trial_       1
Module/UW/TDMA set fair_mode        1 
Module/UW/TDMA set guard_time       0.3
Module/UW/TDMA set tot_slots        4


# # variables for the ALOHA-CSMA module

# variables for the AL module
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 1400
Module/UW/AL set debug_ 0
Module/UW/AL set interframe_period 0.e1
Module/UW/AL set frame_set_validity 0

# variables for the packer(s)
UW/AL/Packer set SRC_ID_Bits 8
UW/AL/Packer set PKT_ID_Bits 8
UW/AL/Packer set FRAME_OFFSET_Bits 15
UW/AL/Packer set M_BIT_Bits 1
UW/AL/Packer set DUMMY_CONTENT_Bits 0
UW/AL/Packer set debug_ 0

NS2/COMMON/Packer set PTYPE_Bits 8
NS2/COMMON/Packer set SIZE_Bits 8
NS2/COMMON/Packer set UID_Bits 8
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 8
NS2/COMMON/Packer set PREV_HOP_Bits 8
NS2/COMMON/Packer set NEXT_HOP_Bits 38
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

UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 0

UW/CBR/Packer set SN_bits 32
UW/CBR/Packer set RFFT_bits 0
UW/CBR/Packer set RFFT_VALID_bits 0
UW/CBR/Packer set debug_ 0



# variables for the S2C modem's interface
#####
Module/UW/MPhy_modem/S2C set period_ 			      1
Module/UW/MPhy_modem/S2C set debug_ 			      1
Module/UW/MPhy_modem/S2C set log_                             1
Module/UW/MPhy_modem/S2C set SetModemID_	 	      0
#######

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
    global ns opt socket_port node_ app_ transport_ port_ routing_ ipif_ mac_ modem_ ipif_ mll_
    global uwal_ 
    # build the NS-Miracle node
    set node_ [$ns create-M_Node]

    # define the module(s) you want to put in the node
    # APPLICATION LAYER
    set app_ [new Module/UW/CBR]
    
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
    set mac_ [new Module/UW/TDMA]
    $mac_ setMacAddr     $opt(node)
    $mac_ setSlotNumber [expr $opt(node) -1]
    set uwal_             [new Module/UW/AL]

    # PHY LAYER
    set modem_ [new "Module/UW/MPhy_modem/S2C" $socket_port]    
    puts "creo nodo"
    # insert the module(s) into the node
   
    $node_ addModule 8 $app_ 1 "CBR"
    $node_ addModule 7 $transport_ 1 "UDP"
    $node_ addModule 6 $routing_ 1 "IPR"
    $node_ addModule 5 $ipif_ 1 "IPIF"
    $node_ addModule 4 $mll_ 1 "ARP"  
    $node_ addModule 3 $mac_ 1 "TDMA"
    $node_ addModule 2 $uwal_ 1 "UWAL"
    $node_ addModule 1 $modem_ 1 "S2C" 

    $node_ setConnection $app_ $transport_ trace
  	$node_ setConnection $transport_ $routing_ trace
  	$node_ setConnection $routing_ $ipif_ trace
  	$node_ setConnection $ipif_ $mll_ trace
  	$node_ setConnection $mll_ $mac_ trace
  	$node_ setConnection $mac_ $uwal_ trace
  	$node_ setConnection $uwal_ $modem_ trace
    
    # intra-node module connections (if needed)
    
    # set module and node parameters (optional)
    
    # assign a port number to the application considered (CBR or VBR)
    set port_ [$transport_ assignPort $app_]
    $ipif_ addr $opt(node)
    $mac_ setMacAddr $opt(node)
    $modem_ set ID_ $opt(node)

    # set packer for Adaptation Layer
    set packer_ [new UW/AL/Packer]

    set packer_payload0 [new NS2/COMMON/Packer]  
    #$packer_payload0 printAllFields  

    set packer_payload1 [new UW/IP/Packer]

    set packer_payload2 [new NS2/MAC/Packer]
    set packer_payload3 [new UW/UDP/Packer]
    set packer_payload4 [new UW/CBR/Packer]
    # $packer_payload3 printAllFields

    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4

    #$packer_ printMap
    # $packer_ printAllFields

    $uwal_ linkPacker $packer_
    
    $uwal_ set nodeID $opt(node)

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
if {$opt(node) == 3} {
  $app_ set destAddr_ 1
} else {
  $app_ set destAddr_ 3
}
$app_ set destPort_ 1

for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
  $routing_ addRoute [expr $cnt + 1] [expr $cnt + 1]
  $mll_ addentry  [expr $cnt + 1] [expr $cnt + 1]
}


#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

$ns at 0 "$modem_ start"
      
$ns at $opt(start) "$app_ start"
$ns at $opt(start) "$mac_ start"

$ns at $opt(stop) "$app_ stop"
$ns at $opt(stop) "$mac_ stop"

$ns at $time_stop "$modem_ stop"

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
   
   global ns tf tf_name	
   # computation of the statics

   # display messages
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

$ns at $time_stop "finish; $ns halt"

# You always need the following line to run the NS-Miracle simulator
$ns run
