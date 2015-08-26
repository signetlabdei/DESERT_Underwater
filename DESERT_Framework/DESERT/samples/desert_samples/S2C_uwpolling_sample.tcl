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
# Author: Federico Favaro
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
if {$argc != 8} {
  puts "The script needs 7 input to work"
  puts "1 - ID of the node"
  puts "2 - ID of the sink"
  puts "3 - Start time"
  puts "4 - Stop time"
  puts "5 - CBR period (time in seconds between two consecutive packets)"
  puts "6 - ID of the experiment"
  puts "7 - IP of the modem"
  puts "8 - Port of the modem"
  puts "Please try again."
  exit(1)
} else {
  set opt(node)     [lindex $argv 0]
  set opt(sink)     [lindex $argv 1]
  set opt(start)    [lindex $argv 2]
  set opt(stop)     [lindex $argv 3]
  set opt(traffic)  [lindex $argv 4]
  set opt(n_run)    [lindex $argv 5]
  set opt(ip)       [lindex $argv 6]
  set opt(port)     [lindex $argv 7]
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
# #Socket Ports to be used
set socket_port "${opt(ip)}:${opt(port)}"

set adrMAC $opt(node)

# time when actually to stop the simulation
set time_stop [expr "$opt(stop)+15"]

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
# Module Configuration  #
#########################
# Put here all the commands to set globally the initialization values of the binded variables (optional)

# variables for the CBR module
Module/UW/CBR set packetSize_          13
Module/UW/CBR set period_              $opt(traffic)
Module/UW/CBR set PoissonTraffic_      0
Module/UW/CBR set debug_               0

if {$opt(node) != $opt(sink)} {
	Module/UW/POLLING/NODE set T_poll_            	    20
	Module/UW/POLLING/NODE set backoff_tuner_     	    1
	Module/UW/POLLING/NODE set max_payload_       	    30
	Module/UW/POLLING/NODE set buffer_data_pkts_  	    50
	Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_  	    5
	Module/UW/POLLING/NODE set sea_trial_         	    1
	Module/UW/POLLING/NODE set print_stats_       	    1
	Module/UW/POLLING/NODE set n_run_		                $opt(n_run)
  Module/UW/POLLING/NODE set intra_data_guard_time_ 	2
	Module/UW/POLLING/NODE set debug_		                0
} else {
	Module/UW/POLLING/AUV set max_payload_        	  30
	Module/UW/POLLING/AUV set T_probe_            	  20
	Module/UW/POLLING/AUV set T_min_              	  1
	Module/UW/POLLING/AUV set T_max_              	  5
	Module/UW/POLLING/AUV set T_guard_            	  5
	Module/UW/POLLING/AUV set max_polled_node_    	  4
	Module/UW/POLLING/AUV set sea_trial_         	    1
	Module/UW/POLLING/AUV set print_stats_       	    1
  Module/UW/POLLING/AUV set Data_Poll_guard_time_ 	2
	Module/UW/POLLING/AUV set n_run_		              $opt(n_run)
	Module/UW/POLLING/AUV set debug_		              0
}


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

NS2/MAC/Uwpolling/Packer set t_in_Bits          16
NS2/MAC/Uwpolling/Packer set t_fin_Bits         16
NS2/MAC/Uwpolling/Packer set uid_TRIGGER_Bits   16
NS2/MAC/Uwpolling/Packer set id_polled_Bits     8
NS2/MAC/Uwpolling/Packer set backoff_time_Bits  16
NS2/MAC/Uwpolling/Packer set ts_Bits            16
NS2/MAC/Uwpolling/Packer set n_pkts_Bits        16
NS2/MAC/Uwpolling/Packer set uid_PROBE_Bits     16
NS2/MAC/Uwpolling/Packer set id_node_Bits       8
NS2/MAC/Uwpolling/Packer set uid_POLL_Bits      16
NS2/MAC/Uwpolling/Packer set debug_				      1

UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 0

UW/CBR/Packer set SN_bits 32
UW/CBR/Packer set RFFT_bits 0
UW/CBR/Packer set RFFT_VALID_bits 0
UW/CBR/Packer set debug_ 0






# variables for the S2C modem's interface
#####
Module/UW/MPhy_modem/S2C set period_ 			        1
Module/UW/MPhy_modem/S2C set debug_ 			        1
Module/UW/MPhy_modem/S2C set log_                 1
Module/UW/MPhy_modem/S2C set SetModemID_	 	      0
Module/UW/MPhy_modem/S2C set RemoteControl_       1
#######

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
    global ns opt socket_port node_ app_ transport_ port_ routing_ ipif_ mac_ modem_ ipif_ mll_
    global uwal_ app_sink
    # build the NS-Miracle node
    set node_ [$ns create-M_Node]

    # define the module(s) you want to put in the node
    # APPLICATION LAYER
    if {$opt(node) != $opt(sink)} {
    	set app_ [new Module/UW/CBR]
    } else {
      for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
        set app_sink($cnt) [new Module/UW/CBR]
      }
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
    if {$opt(node) != $opt(sink)} {
    	set mac_ [new Module/UW/POLLING/NODE]
	} else {
		set mac_ [new Module/UW/POLLING/AUV]
	}

    set uwal_             [new Module/UW/AL]

    # PHY LAYER
    set modem_ [new "Module/UW/MPhy_modem/S2C" $socket_port]    
    puts "creo nodo"
    # insert the module(s) into the node
    if {$opt(node) != $opt(sink)} {
	    $node_ addModule 8 $app_ 1 "CBR"
	    $node_ addModule 7 $transport_ 1 "UDP"
	    $node_ addModule 6 $routing_ 1 "IPR"
	    $node_ addModule 5 $ipif_ 1 "IPIF"
	    $node_ addModule 4 $mll_ 1 "ARP"  
	    $node_ addModule 3 $mac_ 1 "ALOHA"
	    $node_ addModule 2 $uwal_ 1 "UWAL"
	    $node_ addModule 1 $modem_ 1 "S2C" 

	    $node_ setConnection $app_ $transport_ trace
    	$node_ setConnection $transport_ $routing_ trace
    	$node_ setConnection $routing_ $ipif_ trace
    	$node_ setConnection $ipif_ $mll_ trace
    	$node_ setConnection $mll_ $mac_ trace
    	$node_ setConnection $mac_ $uwal_ trace
    	$node_ setConnection $uwal_ $modem_ trace
	} else {
      for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
        $node_ addModule 8 $app_sink($cnt) "CBR"
      }
	    $node_ addModule 7 $transport_ 1 "UDP"
	    $node_ addModule 6 $routing_ 1 "IPR"
	    $node_ addModule 5 $ipif_ 1 "IPIF"
	    $node_ addModule 4 $mll_ 1 "ARP"  
	    $node_ addModule 3 $mac_ 1 "ALOHA"
	    $node_ addModule 2 $uwal_ 1 "UWAL"
	    $node_ addModule 1 $modem_ 1 "S2C" 

      for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
        $node_ setConnection $app_sink($cnt) $transport_
      }
    	$node_ setConnection $transport_ $routing_ trace
    	$node_ setConnection $routing_ $ipif_ trace
    	$node_ setConnection $ipif_ $mll_ trace
    	$node_ setConnection $mll_ $mac_ trace
    	$node_ setConnection $mac_ $uwal_ trace
    	$node_ setConnection $uwal_ $modem_ trace
  }
    
    # intra-node module connections (if needed)
    
    # set module and node parameters (optional)
    
    # assign a port number to the application considered (CBR or VBR)
    if {$opt(node) != $opt(sink)} {
    	set port_ [$transport_ assignPort $app_]
    } else {
		  for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
        set port_($cnt) [$transport_ assignPort $app_sink($cnt)]
      }
    }
    $ipif_ addr $opt(node)
    $mac_ setMacAddr $opt(node)
    $modem_ set ID_ $opt(node)

    # set packer for Adaptation Layer
    set packer_ [new UW/AL/Packer]

    set packer_payload0 [new NS2/COMMON/Packer]  
    #$packer_payload0 printAllFields  

    set packer_payload1 [new UW/IP/Packer]

    set packer_payload2 [new NS2/MAC/Packer]
    set packer_payload3 [new NS2/MAC/Uwpolling/Packer]
    set packer_payload4 [new UW/UDP/Packer]
    set packer_payload5 [new UW/CBR/Packer]
    # $packer_payload3 printAllFields

    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4
    $packer_ addPacker $packer_payload5

    #$packer_ printMap
    # $packer_ printAllFields

    $uwal_ linkPacker $packer_
    
    $uwal_ set nodeID $opt(node)

    if { $opt(node) != $opt(sink)} {
    	$mac_ set node_id_ $opt(node)
    }

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
if {$opt(node) != $opt(sink)} {
  $app_ set destAddr_ $opt(sink)
  $app_ set destPort_ 1
} else {
  for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
    if {$cnt != $opt(sink)} {
      $app_sink($cnt) set destAddr_ $cnt
      $app_sink($cnt) set destPort_ 1
    }
  } 
}

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
      
if {$opt(node) != $opt(sink)} {
  $ns at $opt(start) "$app_ start"
  $ns at $opt(stop) "$app_ stop"
} else {
  $ns at $opt(start) "$mac_ run"
}

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
