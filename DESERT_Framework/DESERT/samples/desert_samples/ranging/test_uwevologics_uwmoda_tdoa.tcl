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

# Author: Vincenzo Cimino
# This script is used to test UW/RANGINGTDOA on real devices (SuM and Evologics S2C modems).

# if set to 1 the Application listen from the socket port provided in input
set opt(AppSocket)  1;
# Protocol to use for the Application socket, TCP or UDP
set opt(protocol) "TCP" ;

set opt(csv_output) 1;

##############################
# Terminal's parameter check #
##############################
if {$opt(AppSocket) == 1} {
  if {$argc < 11} {
    puts "The script needs 10 input to work"
    puts "1 - ID of the node"
    puts "2 - Start time"
    puts "3 - Stop time"
    puts "4 - Packet generation period"
    puts "5 - Delay range start"
    puts "6 - IP of the modem"
    puts "7 - Port of the modem"
    puts "8 - Application socket port"
    puts "9 - Random generator stream"
    puts "10 - Number of nodes"
    puts "11 - ID(s) of the receiver(s)"
    puts "Please try again."
    exit
  } else {
    set opt(node)          [lindex $argv 0]
    set opt(start)         [lindex $argv 1]
    set opt(stop)          [lindex $argv 2]
    set opt(traffic)       [lindex $argv 3]
    set opt(range_delay)   [lindex $argv 4]
    set opt(ip)            [lindex $argv 5]
    set opt(port)          [lindex $argv 6]
    set opt(app_port)      [lindex $argv 7]
    set opt(rngstream)     [lindex $argv 8]
    set opt(n_node)        [lindex $argv 9]
    for {set k 0} {$k < [expr $opt(n_node)-1]} {incr k} {
  	  set opt(dest,$k)  [lindex $argv [expr 10+$k]]
    }
  }
} else {
    puts "Set the AppSocket to 1"
    exit
}

set opt(mac_layer)			"tdma";					# mac protocol to use: tdma or csma_aloha
set opt(modem_hw)			"moda";					# devices to use: moda or evologics
set opt(payload_size)		16;						# application layer payload size
set opt(range_max)          1000.0;					# maximum range (m) for ranging
#set opt(soundspeed)         343.0					# sound speed in air
set opt(soundspeed)         1481.0;					# sound speed in water
set opt(range_period)       $opt(traffic);			# time between ranging updates (s)
set opt(poisson_traffic)    0;						# 1 to enable poisson traffic, 0 to enable CBR
set opt(range_entries)      [expr $opt(n_node)-1];	# 0,-1,-2 adaptive payload size
set opt(guard_time)			1;						# tdma guard time
set opt(frame_duration) $opt(traffic)
set opt(pktsize) 33;								# Actual packet size with 3 nodes (output purposes only)
set opt(real_dist) 0;								# Actual distance between nodes (output purposes only)

# Hardcoded delay depending on the underlying used modem
set opt(modem_delay) "none"
switch $opt(modem_delay) {
	evologics { set opt(mac2phy_delay) [expr 0.01595] }		
	moda { set opt(mac2phy_delay)      [expr 0.05292] }
	default { set opt(mac2phy_delay)   [expr 1e-4] }
}

#####################
# Library Loading   #
#####################
# Load here all the NS-Miracle libraries you need
load libuwip.so
load libuwmll.so
load libuwstaticrouting.so
load libuwudp.so
load libuwapplication.so
load libpackeruwapplication.so
load libuwaloha.so
load libuwcsmaaloha.so
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so
load libuwphy_clmsgs.so
load libuwconnector.so
load libuwmodem.so
load libuwmodamodem.so
load libuwevologicss2c.so
load libuwmmac_clmsgs.so
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libuwcbr.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwstats_utilities.so
load libuwsmposition.so
load libuwphysical.so
load libuwtdma.so
load libuwtap.so
load libuwranging_tdoa.so
load libpackeruwrangingtdoa.so
load libpackeruwcbr.so

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
# address and port of the modem
set address "${opt(ip)}:${opt(port)}"
if {$opt(modem_hw) == "moda"} {
	set opt(s_port) 55006
    set opt(sig_address) "${opt(ip)}:${opt(s_port)}"
}

# time when actually to stop the simulation
set time_stop [expr "$opt(stop)+10"]

#Trace file name
set tf_name "/dev/null/"

#Open a file for writing the trace data
set tf [open $tf_name w]
$ns trace-all $tf

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

#########################
# Module Configuration  #
#########################
# Put here all the commands to set globally the initialization values of
# the binded variables (optional)

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
NS2/COMMON/Packer set NEXT_HOP_Bits 8
NS2/COMMON/Packer set ADRR_TYPE_Bits 0
NS2/COMMON/Packer set LAST_HOP_Bits 0
NS2/COMMON/Packer set TXTIME_Bits 0
NS2/COMMON/Packer set debug_ 0

NS2/MAC/Packer set Ftype_Bits 0
NS2/MAC/Packer set SRC_Bits 8
NS2/MAC/Packer set DST_Bits 8
NS2/MAC/Packer set Htype_Bits 0
NS2/MAC/Packer set TXtime_Bits 0
NS2/MAC/Packer set SStime_Bits 0
NS2/MAC/Packer set Padding_Bits 0
NS2/MAC/Packer set debug_ 0

UW/IP/Packer set SAddr_Bits 8
UW/IP/Packer set DAddr_Bits 8
UW/IP/Packer set debug_ 0

UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 0

UW/APP/uwApplication/Packer set SN_FIELD_ 8
UW/APP/uwApplication/Packer set RFFT_FIELD_ 5
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 2
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 8
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 8
UW/APP/uwApplication/Packer set debug_ 0

UW/RANGING_TDOA/Packer set SOURCE_PKT_ID_FIELD          8
UW/RANGING_TDOA/Packer set SOURCE_NODE_ID_FIELD			3
UW/RANGING_TDOA/Packer set TIMES_SIZE_FIELD				3
UW/RANGING_TDOA/Packer set debug_ 0

Module/UW/APPLICATION set debug_ -1              
Module/UW/APPLICATION set period_ $opt(traffic)
Module/UW/APPLICATION set PoissonTraffic_ 0
if {$opt(AppSocket) == 1} {
  Module/UW/APPLICATION set Socket_Port_ $opt(app_port)
} else {
  Module/UW/APPLICATION set Payload_size_ $opt(payload_size)
}
Module/UW/APPLICATION set drop_out_of_order_ 1    
Module/UW/APPLICATION set pattern_sequence_ 0     
Module/UW/APPLICATION set EXP_ID_ 1

# variables for the S2C modem's interface
Module/UW/UwModem/EvoLogicsS2C set debug_		 0
Module/UW/UwModem/EvoLogicsS2C set buffer_size   4096
Module/UW/UwModem/EvoLogicsS2C set max_read_size 4096
Module/UW/UwModem/EvoLogicsS2C set period_   0.0001 

# variables for the MODA modem interface
Module/UW/UwModem/MODA set debug_	 0
Module/UW/UwModem/MODA set buffer_size   4096
Module/UW/UwModem/MODA set max_read_size 4096
Module/UW/UwModem/MODA set premodulation 1
Module/UW/UwModem/MODA set period_   0.0001 

Module/UW/TAP  set debug_     0

Module/UW/RANGING_TDOA  set debug_			0
Module/UW/RANGING_TDOA  set debug_tdoa		3
Module/UW/RANGING_TDOA  set n_nodes_		$opt(n_node)
Module/UW/RANGING_TDOA  set max_tt			[expr $opt(range_max)/$opt(soundspeed)]
Module/UW/RANGING_TDOA  set range_period	$opt(range_period)
Module/UW/RANGING_TDOA  set delay_start_	$opt(range_delay)
Module/UW/RANGING_TDOA  set poisson_traffic	$opt(poisson_traffic)
Module/UW/RANGING_TDOA  set range_entries	$opt(range_entries)
Module/UW/RANGING_TDOA  set soundspeed		$opt(soundspeed)
Module/UW/RANGING_TDOA  set mac2phy_delay_ 	$opt(mac2phy_delay)  

Module/UW/TDMA set debug_				0
Module/UW/TDMA set sea_trial_			0
Module/UW/TDMA set frame_duration		$opt(frame_duration)
Module/UW/TDMA set guard_time			$opt(guard_time)
Module/UW/TDMA set tot_slots			$opt(n_node)
Module/UW/TDMA set fair_mode            1
Module/UW/TDMA set max_packet_per_slot  1
Module/UW/TDMA set queue_size_          1
Module/UW/TDMA set drop_old_            1
Module/UW/TDMA set mac2phy_delay_       [expr 1.0e-9]

Module/UW/CSMA_ALOHA set debug_			0
Module/UW/CSMA_ALOHA set wait_costant_	0.1
Module/UW/CSMA_ALOHA set listen_time_ 	0.5

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
	global ns opt socket_port node_ address
	global app_ transport_ port_ routing_ ipif_ mac_ modem_ ipif_ mll_ uwal_ app_sink range_ tap_

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
	
    # DATA LINK LAYER - MEDIA LINK LAYER
    set mll_ [new Module/UW/MLL]
    
	# Ranging
	set range_ [new Module/UW/RANGING_TDOA]

    # DATA LINK LAYER - MAC LAYER
	switch $opt(mac_layer) {
		csma_aloha { set mac_ [new Module/UW/CSMA_ALOHA] }
		tdma { set mac_ [new Module/UW/TDMA] }
		default { puts "no mac specified using csma aloha";  set mac_ [new Module/UW/CSMA_ALOHA] }
	}

	# TAP LAYER
	set tap_ [new Module/UW/TAP]

	# ADAPTION LAYER
    set uwal_ [new Module/UW/AL]

    # PHY LAYER
	switch $opt(modem_hw) {
		evologics { set modem_ [new Module/UW/UwModem/EvoLogicsS2C] }
		moda { set modem_ [new Module/UW/UwModem/MODA] }
		default { puts "no physical layer specified (moda or evologics)" ; exit }
	}

	puts "Creating node..."

	# insert the module(s) into the node
	$node_ addModule 10 $app_ 1 "UWA"
	$node_ addModule 9 $transport_ 1 "UDP"
	$node_ addModule 8 $routing_ 1 "IPR"
	$node_ addModule 7 $ipif_ 1 "IPIF"
	$node_ addModule 6 $mll_ 1 "ARP"  
    $node_ addModule 5 $range_		1 "RANGE"
	$node_ addModule 4 $mac_ 1 "MAC"
    $node_ addModule 3 $tap_		1 "TAP"
	$node_ addModule 2 $uwal_ 1 "UWAL"
	$node_ addModule 1 $modem_ 1 "PHY" 

	$node_ setConnection $app_ $transport_ trace
    $node_ setConnection $transport_ $routing_ trace
    $node_ setConnection $routing_ $ipif_ trace
    $node_ setConnection $ipif_ $mll_ trace
    $node_ setConnection $mll_ $range_ trace
    $node_ setConnection $range_ $mac_ trace
    $node_ setConnection $mac_ $tap_ trace
    $node_ setConnection $tap_ $uwal_ trace
    $node_ setConnection $uwal_ $modem_ trace

    if {$opt(AppSocket) == 1} {
    	$app_ setSocketProtocol $opt(protocol)
    	$app_ set Socket_Port_ $opt(app_port)
    } else {
    	$app_ setSocketProtocol "NONE"
    }
    $app_ set node_ID_  $opt(node)
    $app_ print_log

	# assign a port number to the application considered (CBR or VBR)
    set port_ [$transport_ assignPort $app_]

    $ipif_ addr $opt(node)

    # ids in the ranging module start from 0
    $range_ setId [expr $opt(node)-1]
    $range_ setPHYId [$modem_ Id_]

    $mac_ setMacAddr $opt(node)
	switch $opt(mac_layer) {
		csma_aloha { 
			$mac_ setNoAckMode
			$mac_ initialize
		}
		tdma { 
			$mac_ setSlotNumber $opt(node)
			$ns at $opt(start)    "$mac_ start"
    		$ns at $opt(stop)    "$mac_ stop"
		}
		default { 
			puts "no mac specified using csma aloha";  
			$mac_ setNoAckMode
			$mac_ initialize
		}
	}

    $modem_ set ID_ $opt(node)
	$modem_ setModemAddress $address
    $modem_ setLogLevel DBG
	if { $opt(modem_hw) == "evologics" } {
        $modem_ setTXDurationFileName "../dbs/evologics_LUT/txduration_s2c_LUT.csv"
        $modem_ initLUT
	} elseif { $opt(modem_hw) == "moda" } {
        $modem_ setSignalAddress $opt(sig_address)
    }

    # set packer for Adaptation Layer
    set packer_ [new UW/AL/Packer]

    set packer_payload0 [new NS2/COMMON/Packer]  
    set packer_payload1 [new NS2/MAC/Packer]
    set packer_payload2 [new UW/RANGING_TDOA/Packer]
    set packer_payload3 [new UW/IP/Packer]
    set packer_payload4 [new UW/UDP/Packer]
    set packer_payload5 [new UW/APP/uwApplication/Packer]

    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4
    $packer_ addPacker $packer_payload5
	$packer_ printMap

    $uwal_ linkPacker $packer_
    $uwal_ set nodeID $opt(node)
	
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
createNode

# connections at the application level
$app_ set destAddr_ [expr $opt(dest,0)]
$app_ set destPort_ 1


# ARP tables
for {set k 0} {$k < [expr $opt(n_node)-1]} {incr k} {
    $routing_ addRoute $opt(dest,$k) $opt(dest,$k)
    $mll_ addentry  $opt(dest,$k) $opt(dest,$k)
}

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

$ns at $opt(start)   "$range_ start"
$ns at $opt(stop)    "$range_ stop"

$ns at 0 "$modem_ start"

if {$opt(traffic) != 0} {
  $ns at $opt(start) "$app_ start"
  $ns at $opt(stop) "$app_ stop"
}

$ns at $time_stop "$modem_ stop"


# Return a list with cooridnates of the node position (Works only with vpn nodes)
proc get_pos {node} {
    global opt

	regexp "([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})" $opt(ip) _ first second third _

	set str [exec echo position | nc -W 1 $first.$second.$third.$node 4242]
	if {$str eq ""} {
		return "0 0 0"
	}
	scan $str {%[a-z] = %f %f %f} _ x y z
	set pos "$x $y $z"
	return $pos
}

# returns true distance between nodes i and j (Works only with vpn nodes)
proc trueDist {t i j} {
    global opt

	foreach {xi yi zi} [get_pos $i] break
	foreach {xj yj zj} [get_pos $j] break

	puts "---------------------------------------"
	puts "Time $t"
	puts "Position node $i: $xi,$yi,$zi"
	puts "Position node $j: $xj,$yj,$zj"

    set dist [expr { hypot( hypot($xi - $xj , $yi - $yj) , $zi - $zj ) }]
	puts "True distance between $i and $j: $dist"
	puts "---------------------------------------"
}

# Update the position of node (Works only with vpn nodes)
proc update_pos {x y z} {
	global opt

	exec echo $x $y $z | nc -w 0 $opt(ip) 11000
}

# returns range measured by node between nodes i and j
proc calcDist {t i j} {
    global range_ opt

	# range_ID == node_id-1
	set v [expr $i-1]
	set w [expr $j-1]

	set twtt_dist [$range_ get_distance $v $w]
	set dist [expr $opt(soundspeed)*[$range_ get_distance $v $w]]

	# CSV output
	if {$opt(csv_output)} {
		puts "$opt(node);$opt(range_period);$opt(range_delay);$opt(n_node);$opt(pktsize);$t;$i;$j;$twtt_dist;$opt(soundspeed);$dist;$opt(real_dist)"
	} else {
		puts "---------------------------------------"
		puts "Time $t"
		puts "Range distance between $i and $j: $dist"
		puts "---------------------------------------"
	}
}

 for { set t $opt(range_period) } { $t < $time_stop } { incr t $opt(range_period) } {
    for {set k 0} {$k < [expr $opt(n_node)-1]} {incr k} {
		# uncomment only if you are using evologics vpn nodes
		# $ns at $t "trueDist $t $opt(node) $opt(dest)"
	    $ns at $t "calcDist $t $opt(node) $opt(dest,$k)"

		if {$opt(n_node) > 2} {
			for {set j 0} {$j < [expr $opt(n_node)-1]} {incr j} {
				if { $k != $j } {
					# uncomment only if you are using evologics vpn nodes
					# $ns at $t "trueDist $t $opt(dest,$k) $opt(dest,$j)"
					$ns at $t "calcDist $t $opt(dest,$k) $opt(dest,$j)"
				}
			}
		}
	}
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
   
   global ns tf tf_name	
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
