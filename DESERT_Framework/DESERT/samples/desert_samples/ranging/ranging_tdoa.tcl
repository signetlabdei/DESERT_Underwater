#
# Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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
# This script is used to test UW/TDMA module
# There are 3 nodes placed in line that can transmit each other 
# packets with a CBR (Constant Bit Rate) Application Module
#
#
# Author: Antonio Montanari
# Version: 1.0.0
#
# NOTE: tcl sample tested on Ubuntu 22.04 and WSL2 over Win10, 64 bits OS
#
# Stack of the nodes
#   +-------------------------+
#   |  7. UW/CBR              |
#   +-------------------------+
#   |  6. UW/UDP              |
#   +-------------------------+
#   |  5. UW/STATICROUTING    |
#   +-------------------------+
#   |  4. UW/IP               |
#   +-------------------------+
#   |  3. UW/MLL              |
#   +-------------------------+
#   |  2. UW/ALOHA            |
#   +-------------------------+
#   |  1. UW/PHYSICAL      |
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |    UnderwaterChannel    |
#   +-------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(trace_files)		0
set opt(bash_parameters) 	0
#set tcl_precision 9
#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwmmac_clmsgs.so
load libuwcbr.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwsmposition.so
load libuwphysical.so
load libuwphysical.so
load libuwtdma.so
load libuwcsmaaloha.so
load libuwtokenbus.so
load libuwranging_tokenbus.so
load libuwtap.so
load libuwranging_tdoa.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle
set pi 3.1415926535897931
set phase 0
##################
# Tcl variables  #
##################
set opt(verbose) 		    1
set opt(mac)                "aloha"; # can be aloha,tdma, tb 
set opt(nn)                 10 ;# Number of Nodes
set opt(range_entries)      -1;#[expr $opt(nn)-1]; # 0 for adaptive payload size
set opt(range_period)       1; #period (s) between ranging updates
set opt(poisson_traffic)    0; # 1 to enable poisson traffic, 0 to enable CBR
set opt(range_max)          1500.0; #maximum range (m) for ranging
set opt(guard_time)         0.15; # expressed as fraction of payload tx time so that slot_time = tx_time*(1 + guard_time)
set opt(interf)             "MEANPOWER" ;# MEANPOWER or CHUNK
set opt(node_speed)         1; #in m/s
set opt(maxdrift)           20; #maximum drift in m for each XYZ coordinate
set opt(XYdiameter)         1200.0; #diameter of node wandering field in XY plane
set opt(XYinitdiam)         5.0;# diameter of initial positions
set opt(Zwander)            0.0; #maximum wander of nodes in depth
set opt(step)               5; #period (s) between course changes and ranging updates
set opt(printsteps)         0; # to print statistics for each step 
set opt(starttime)          0; #do not change from 0	
set opt(stoptime)           1500
set opt(soundspeed)         1500.0
set opt(txduration)         [expr {$opt(stoptime) - $opt(starttime)}] ;# Duration of the simulation
set opt(mac2phy_delay)      [expr 1.0e-4]; # Delay between MAC and PHY layers (s) needed to prevent packet collisions when nodes are moving towards each other
set opt(txpower)            170.0  ;#Power transmitted in dB re uPa
set opt(maxinterval_)       20.0
set opt(freq)               63000.0 ;#Frequency used in Hz
set opt(bw)                 30000.0	;#Bandwidth used in Hz
set opt(bitrate)            5120.0	;#bitrate in bps
set opt(ack_mode)           "setNoAckMode"
set opt(cbr_period)         10
set opt(pktsize)            64 ;# CBR Pkt size in byte
set opt(rngstream)	        17
set opt(debug)             -108
set opt(rawfile)            "./cbr_$opt(mac)_$opt(cbr_period)_$opt(range_entries).csv"; #format of each line will be time_of_measure,measuring_node,from_node_i,to_node_j,ranging_error

set opt(min_token_hold)     0.0000000001
#set opt(frame_duration)     [expr {1.1*$opt(nn)*(($opt(nn)-1)*4+2)*(1.0 + $opt(guard_time))*8/$opt(bitrate)}]; #frame_duration set 10% wider than stricly needed
set opt(frame_duration)     [expr {1*$opt(nn)*($opt(guard_time)+(($opt(nn)+1)*2*8/$opt(bitrate)))}]; #frame_duration set 10% wider than stricly needed
set opt(frame_duration)     [expr {1*$opt(nn)*$opt(pktsize)*8/$opt(bitrate)*(1+$opt(guard_time))}]; #packets of 40 bytes
set opt(frame_duration)     [expr {1*$opt(nn)*$opt(pktsize)*8/$opt(bitrate)+($opt(nn)*$opt(guard_time))}]; #packets of 40 bytes
puts "frame duration: $opt(frame_duration)"
if {$opt(step) <= $opt(frame_duration)} {
    puts "BEWARE! Thou might want to set evaluation step opt(step) greather than frame_duration!!!"
}



if {$argc == 1} {set opt(rngstream)  [lindex $argv 0]}


###########################
#Random Number Generators #
###########################

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

#### POSITIONS AND COURSES ####

proc initpos_circle {diameter} {
    global opt position pi 
    for {set id 0} { $id < $opt(nn) } { incr id } {  
        $position($id) setX_ [expr {0.5*$diameter*cos($id*2*$pi/$opt(nn))}]
        $position($id) setY_ [expr {0.5*$diameter*sin($id*2*$pi/$opt(nn))}]
        $position($id) setZ_ -200
    }
}
proc drift {} {
    global opt position defaultRNG savedPositions
    for {set n 0} { $n < $opt(nn) } { incr n } {
        set newX [expr {$savedPositions(X$n)+[$defaultRNG testdouble $opt(maxdrift)]-$opt(maxdrift)/2}]
        set newY [expr {$savedPositions(Y$n)+[$defaultRNG testdouble $opt(maxdrift)]-$opt(maxdrift)/2}]
        set newZ [expr {$savedPositions(Z$n)+[$defaultRNG testdouble $opt(maxdrift)]-$opt(maxdrift)/2}]
        $position($n) setdest $newX $newY $newZ $opt(node_speed)
    }
}

proc setdest_circle {diameter} {
    global opt position pi
    for {set id 0} { $id < $opt(nn) } { incr id } {  
        set newX [expr {0.5*$diameter*cos($id*2*$pi/$opt(nn))}]
        set newY [expr {0.5*$diameter*sin($id*2*$pi/$opt(nn))}]
        set newZ [$position($id) getZ_]
        $position($id) setdest $newX $newY $newZ $opt(node_speed)
    }
}

proc savePositions {} {
    global savedPositions position opt
    for {set n 0} { $n < $opt(nn) } { incr n } {
        set savedPositions(X$n) [$position($n) getX_]
        set savedPositions(Y$n) [$position($n) getY_]
        set savedPositions(Z$n) [$position($n) getZ_]
    }
}

###########################
#Trace files              #
###########################
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_ranging_tdma.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_ranging_tdma.cltr"
	set opt(cltracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}

#########################
# Module Configuration  #
#########################
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      0
Module/UW/CBR set debug_               0
Module/UW/CBR set drop_out_of_order_   1


### RANGING MODULE ###
Module/UW/RANGING_TDOA  set debug_     0
Module/UW/TAP  set debug_     0
Module/UW/RANGING_TDOA  set debug_tdoa     $opt(debug)

Module/UW/RANGING_TDOA  set n_nodes_ $opt(nn)
Module/UW/RANGING_TDOA  set epsilon 1e-6
Module/UW/RANGING_TDOA  set max_tt [expr $opt(range_max)/$opt(soundspeed)]
Module/UW/RANGING_TDOA  set range_period $opt(range_period)
Module/UW/RANGING_TDOA  set poisson_traffic $opt(poisson_traffic)
Module/UW/RANGING_TDOA  set range_entries $opt(range_entries)
Module/UW/RANGING_TDOA  set soundspeed $opt(soundspeed)
Module/UW/RANGING_TDOA  set mac2phy_delay_ 		$opt(mac2phy_delay)  

### TDMA MAC ###
Module/UW/TDMA set frame_duration   $opt(frame_duration)
Module/UW/TDMA set debug_           0
Module/UW/TDMA set sea_trial_       0
Module/UW/TDMA set fair_mode        1; #FAIR Mode MUST BE ON
Module/UW/TDMA set guard_time       $opt(guard_time);#[expr {($opt(frame_duration)/$opt(nn))*($opt(guard_time)/(1.0+$opt(guard_time)))}];
Module/UW/TDMA set tot_slots        $opt(nn)
puts "guardtime (s): $opt(guard_time)";#[expr {($opt(frame_duration)/$opt(nn))*($opt(guard_time)/(1.0+$opt(guard_time)))}]"
puts "guardtime (m): [expr {1500.0*$opt(guard_time)}]"; #($opt(frame_duration)/$opt(nn))*($opt(guard_time)/(1.0+$opt(guard_time)))}]"
puts "evaluation timestep: $opt(step)"

Module/UW/TDMA set HDR_size_ 		0
Module/UW/TDMA set max_packet_per_slot  3
Module/UW/TDMA set queue_size_          1
Module/UW/TDMA set drop_old_            1
Module/UW/TDMA set checkPriority_		0
Module/UW/TDMA  set mac2phy_delay_ 		$opt(mac2phy_delay) 

## ALOHA MAC ###
Module/UW/CSMA_ALOHA set wait_costant_		0.25
Module/UW/CSMA_ALOHA set debug_			0
Module/UW/CSMA_ALOHA set backoff_tuner_   		1
Module/UW/CSMA_ALOHA set max_payload_			70;#ns full$opt(pktsize)
Module/UW/CSMA_ALOHA set ACK_timeout_			0.1
Module/UW/CSMA_ALOHA set alpha_			0.8
Module/UW/CSMA_ALOHA set buffer_pkts_			-1
Module/UW/CSMA_ALOHA set max_backoff_counter_   	4
Module/UW/CSMA_ALOHA set listen_time_ 		0.5
Module/UW/CSMA_ALOHA  set mac2phy_delay_ 		$opt(mac2phy_delay) 

#TOKENBUS
#max_tt_ should be the maximum travel times between any node
#set max_tt_ [expr hypot(hypot($opt(XYdiameter),$opt(XYdiameter)),$opt(Zwander))/$opt(soundspeed)]
set max_tt_ [expr {$opt(XYdiameter)/$opt(soundspeed)}]
puts "slot time: $max_tt_ token_pass_timeout: [expr {2.0*$max_tt_ + $opt(min_token_hold)}] bus_idle_timeout_: [expr {$max_tt_ + $opt(min_token_hold)}]"
Module/UW/TOKENBUS set n_nodes_ 		        $opt(nn)
Module/UW/TOKENBUS set slot_time_ 		    $max_tt_ 
Module/UW/TOKENBUS set min_token_hold_time_ 	$opt(min_token_hold)
Module/UW/TOKENBUS set token_pass_timeout_ 	[expr {2.0*$max_tt_ + 2.0*$opt(min_token_hold)}]
Module/UW/TOKENBUS set bus_idle_timeout_ 	[expr {$max_tt_ + 2.0*$opt(min_token_hold)}]
Module/UW/TOKENBUS set queue_size_ 		    1
Module/UW/TOKENBUS set drop_old_ 		    1
Module/UW/TOKENBUS set debug_tb 		        0
Module/UW/TOKENBUS set debug_ 		        0
Module/UW/TOKENBUS set max_token_hold_time_ 	10
Module/UW/TOKENBUS set max_packet_per_slot  100
Module/UW/TOKENBUS set drop_old_ 		1
Module/UW/TOKENBUS set checkPriority_ 		0
Module/UW/TOKENBUS set mac2phy_delay_ 		$opt(mac2phy_delay) 
Module/UW/TOKENBUS set max_tt 		        [expr $max_tt_*1.1]


# var binded by UW/PHYSICAL
Module/UW/PHYSICAL  set TxPower_                    $opt(txpower)
Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    -90 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            -7.5
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 10000
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set ConsumedEnergy_             0
Module/UW/PHYSICAL  set NoiseSPD_                   0
Module/UW/PHYSICAL  set debug_                      0
####################################
Position/UWSM   set debug_  0

####################################
### Channel ###
MPropagation/Underwater set practicalSpreading_ 1.75
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          15
MPropagation/Underwater set shipping_           1

Module/UnderwaterChannel   set propSpeed_      $opt(soundspeed)
set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)
$data_mask setPropagationSpeed  $opt(soundspeed)

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    puts "now creating node $id"
    global channel ns cbr position node udp portnum ipr ipif defaultRNG
    global opt mll mac tap phy range propagation data_mask interf_data
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		set cbr($id,$cnt)  [new Module/UW/CBR] 
	}
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL]
    set range($id)  [new Module/UW/RANGING_TDOA]
    set mac($id)  [new Module/UW/CSMA_ALOHA]
    # Set the MAC address
    $mac($id) setMacAddr [expr $id]
    $mac($id) setNoAckMode
    $mac($id) initialize
    switch $opt(mac) {
        aloha {puts "using CSMA_ALOHA MAC"}
        tb {puts "using TokenBus mac"; set mac($id)  [new Module/UW/TOKENBUS]}
        tdma {
            puts "using TDMA mac"
            set mac($id)  [new Module/UW/TDMA]
            $mac($id) setMacAddr [expr $id]
            $mac($id) setSlotNumber $id
            $ns at $opt(starttime)    "$mac($id) start"
            $ns at $opt(stoptime)    "$mac($id) stop"
        }
        default {puts "WRONG opt(mac) value, using CSMA_ALOHA!!!!"}
    } 
    set tap($id)  [new Module/UW/TAP]
    set phy($id)  [new Module/UW/PHYSICAL]  
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node($id) addModule 9 $cbr($id,$cnt)   1  "CBR"
    }

    $node($id) addModule 8 $udp($id)   1  "UDP"
    $node($id) addModule 7 $ipr($id)   1  "IPR"
    $node($id) addModule 6 $ipif($id)  1  "IPF"
    $node($id) addModule 5 $mll($id)   0  "MLL"
    $node($id) addModule 4 $range($id)  0  "RANGE"
    $node($id) addModule 3 $mac($id)   0  "MAC"
    $node($id) addModule 2 $tap($id)   0  "TAP"
    $node($id) addModule 1 $phy($id)   0  "PHY"

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node($id) setConnection $cbr($id,$cnt)   $udp($id)   0
        set portnum($id,$cnt) [$udp($id) assignPort $cbr($id,$cnt) ]
    }

    $node($id) setConnection $udp($id)   $ipr($id)   1
    $node($id) setConnection $ipr($id)   $ipif($id)  1
    $node($id) setConnection $ipif($id)   $mll($id)  1
    $node($id) setConnection $mll($id)   $range($id)   0
    $node($id) setConnection $range($id)  $mac($id)   0
    $node($id) setConnection $mac($id)   $tap($id)   0
    $node($id) setConnection $tap($id)   $phy($id)   0
    $node($id) addToChannel  $channel    $phy($id)   0

    
    #Set the IP address of the node
    $ipif($id) addr "1.0.0.${id}"
    $ipif($id) addr [expr $id + 1]

    #set RANGING parameters
    $range($id) setId $id


    set position($id) [new "Position/UWSM"]
    $node($id) addPosition $position($id)
    
    #Setup positions
    $position($id) setX_ [$defaultRNG testdouble 100]; #[expr $id*15]
    $position($id) setY_ [$defaultRNG testdouble 100];#[expr $id*0*15]
    $position($id) setZ_ -200

    $position($id) setX_ [expr $id*15]
    $position($id) setY_ [expr $id*0*15]
    $position($id) setZ_ -200

    

    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    #Propagation model
    $phy($id) setPropagation $propagation
    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
    $phy($id) setInterferenceModel "MEANPOWER"
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}



# ################################
# # Inter-node module connection #
# ################################
proc connectNodes {id1 des1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink ipr_sink opt 

    $cbr($id1,$des1) set destAddr_ [$ipif($des1) addr]
    $cbr($id1,$des1) set destPort_ $portnum($des1,$id1)
}

# ##################
# # Setup flows    #
# ##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
		connectNodes $id1 $id2
	}
}

# ##################
# # ARP tables     #
# ##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
	}
}



# ##################
# # Routing tables #
# ##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
			$ipr($id1) addRoute [$ipif($id2) addr] [$ipif($id2) addr]
	}
}


######################
# ranging procedures #
######################
array set true_dists {}
array set calc_dists {}
array set nodes_RMSE {}
array set nodes_max_err {}
array set dist_RMSE {}
array set dist_max_err {}
array set cerr {}
array set movavg_nodes_RMSE {}
array set mov_nodes_max_err {}
array set movavg_dist_RMSE {}
array set mov_dist_max_err {}
set dnum [expr {$opt(nn)*($opt(nn)-1)/2}]


for {set n 0} { $n < $opt(nn) } { incr n } {
    set movavg_nodes_RMSE($n) 0.0
    set mov_nodes_max_err($n) 0.0
}
for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
    for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
            set movavg_dist_RMSE($i,$j) 0.0
            set mov_dist_max_err($i,$j) 0.0      
    }
}

#returns range measured by node n between nodes i and j
proc calcDist {n i j} {
    global range opt
    set dist [expr {$opt(soundspeed)*[$range($n) get_distance $i $j]}]
    if {$dist>0} {
        return $dist
    } else {return 0}
}

#returns true distance between nodes i and j
proc trueDist {i j} {
    global position
    return [expr {hypot(hypot([$position($i) getX_]-[$position($j) getX_],[$position($i) getY_]-[$position($j) getY_]),[$position($i) getZ_]-[$position($j) getZ_])}]
}

# updates true distances between nodes
proc updateTrueDist {} {
    global opt true_dists
    for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
        for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
                set true_dists($i,$j) [trueDist $i $j]      
        }
    }
}


# Updates (and prints) distances for EVERY node
proc updateDist {print} {
    updateTrueDist
    global opt true_dists calc_dists nodes_RMSE
    for {set n 0} {$n < $opt(nn)} {incr n}  {
        for {set i 0} {$i < $opt(nn) } {incr i}  {
            for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
                set calc_dists($n,$i,$j) [calcDist $n $i $j]
                if {$print} {puts "Node $n : range between $i and $j : $calc_dists($n,$i,$j) true distance: $true_dists($i,$j)"}   
            }
        }
        if {$print} {puts ""}
    }
    if {$print} {puts ""}
}

set csvraw {}
#Prints MSE and max error for each node
proc updateAll {t print} {
    updateDist 0; #set to 1 to print each updated distance at each timestep
    #drift
    global phy opt dnum calc_dists true_dists nodes_RMSE nodes_max_err dist_RMSE dist_max_err 
    global mov_dist_max_err mov_nodes_max_err movavg_dist_RMSE movavg_nodes_RMSE cerr csvraw range phase
    #calculate node-wise statistics
    for {set n 0} {$n < $opt(nn)} {incr n}  {
        set nodes_max_err($t,$n) 0.0
        set nodes_RMSE($t,$n) 0.0
        for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
            for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
                set cerr($t,$n,$i,$j) [expr {$calc_dists($n,$i,$j) - $true_dists($i,$j)}]
                #lappend csvraw "$opt(rngstream),$t,$phase,$n,$i,$j,$calc_dists($n,$i,$j),$true_dists($i,$j),[$phy($n) getTotPktsLost],[expr [$range($n) getFullPktTime]*[$range($n) getRangePktsTx]],[expr [$phy($n) getTxTime]]"
                lappend csvraw "$opt(rngstream),$t,$phase,$n,$i,$j,$calc_dists($n,$i,$j),$true_dists($i,$j),[$phy($i) getTotPktsLost],[$phy($i) getErrorPktsInterf],[$range($n) getRangeBytesTx],[expr [$range($n) getSavedEntries]*3],[$range($n) calcOptEntries]"
                set nodes_max_err($t,$n) [expr ($nodes_max_err($t,$n) > abs($cerr($t,$n,$i,$j)))? $nodes_max_err($t,$n) : abs($cerr($t,$n,$i,$j))]     
                set nodes_RMSE($t,$n) [expr {$nodes_RMSE($t,$n) + $cerr($t,$n,$i,$j)*$cerr($t,$n,$i,$j)}]
            }
        }
        set nodes_RMSE($t,$n) [expr sqrt($nodes_RMSE($t,$n)/$dnum)]
        set movavg_nodes_RMSE($n) [expr {[expr $movavg_nodes_RMSE($n)*$t + $nodes_RMSE($t,$n)]/[expr $t + 1.0]}]
        set mov_nodes_max_err($n) [expr ($nodes_max_err($t,$n) > $mov_nodes_max_err($n))? $nodes_max_err($t,$n) : $mov_nodes_max_err($n)] 
        if {$print} {puts "Time $t Node $n : RMSE : $nodes_RMSE($t,$n) max error: $nodes_max_err($t,$n)"}
    }
    if {$print} {puts ""}

    #calculate distance-wise statistics
    for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
        for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} {
            set dist_max_err($t,$i,$j) 0.0
            set dist_RMSE($t,$i,$j) 0.0
            for {set n 0} {$n < $opt(nn)} {incr n}  { 
                set dist_max_err($t,$i,$j) [expr ($dist_max_err($t,$i,$j) > abs($cerr($t,$n,$i,$j)))? $dist_max_err($t,$i,$j) : abs($cerr($t,$n,$i,$j))]     
                set dist_RMSE($t,$i,$j) [expr {$dist_RMSE($t,$i,$j) + $cerr($t,$n,$i,$j)*$cerr($t,$n,$i,$j)}]
            }
            set dist_RMSE($t,$i,$j) [expr sqrt($dist_RMSE($t,$i,$j)/$opt(nn))]
            set movavg_dist_RMSE($i,$j) [expr {[expr $movavg_dist_RMSE($i,$j)*$t + $dist_RMSE($t,$i,$j)]/[expr $t + 1.0]}]
            set mov_dist_max_err($i,$j) [expr ($dist_max_err($t,$i,$j) > $mov_dist_max_err($i,$j))? $dist_max_err($t,$i,$j) : $mov_dist_max_err($i,$j)] 
            if {$print} {puts "Time $t dist ($i,$j) : RMSE : $dist_RMSE($t,$i,$j) max error: $dist_max_err($t,$i,$j)"}
        }
    }
    if {$print} {puts ""}
}

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
   $ns at $opt(starttime)    "$range($id1) start"
   $ns at $opt(stoptime)    "$range($id1) stop"
    #$ns at [expr $opt(starttime)+$id1+1.0]     "$range($id1) sendRange"
	for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
		if {$id1 != $id2} {
			$ns at $opt(starttime)    "$cbr($id1,$id2) start"
			$ns at $opt(stoptime)     "$cbr($id1,$id2) stop"
		}
	}
}

# Schedule events on ns
#"$position($id) setdest [expr {0.5*$opt(XYdiameter)*cos($id*2*$pi/$opt(nn))}] [expr {0.5*$opt(XYdiameter)*sin($id*2*$pi/$opt(nn))}] -200 $opt(node_speed)"
$ns at 0 "initpos_circle 10; savePositions"
for {set t 0} { $t < 150 } { incr t $opt(step) } {
    $ns at $t "drift"
}


$ns at 150 "incr phase; setdest_circle 100"
$ns at 195 "savePositions; incr phase"
#$ns at 145.001 "drift"
for {set t 195} { $t < 345 } { incr t $opt(step) } {
    $ns at $t "drift"
}

$ns at 345 "incr phase; setdest_circle 500"
$ns at 545 "savePositions; incr phase"
#$ns at 445.001 "drift"
for {set t 545} { $t < 695 } { incr t $opt(step) } {
    $ns at $t "drift"
}

$ns at 695 "incr phase; setdest_circle 1000" ;#was1000
$ns at 945 "savePositions; incr phase"
#$ns at 1100 "savePositions; incr phase"
for {set t 945} { $t < $opt(stoptime) } { incr t $opt(step) } {
    $ns at $t "drift"
}

# for {set id 0} {$id < $opt(nn)} {incr id}  {
#     $ns at 400 "$position($id) setdest [expr {0.5*$opt(XYinitdiam)*cos($id*2*$pi/$opt(nn))}] [expr {0.5*$opt(XYinitdiam)*sin($id*2*$pi/$opt(nn))}] -200 $opt(node_speed)"
# }
for {set t $opt(step)} { $t < $opt(stoptime) } { incr t $opt(step)} {
    $ns at $t "updateAll $t $opt(printsteps)"
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt position csvraw
    global range mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global node_coordinates
    global ipr_sink ipr ipif udp cbr phy phy_data_sink range
    global node_stats tmp_node_stats sink_stats tmp_sink_stats
    global cerr movavg_nodes_RMSE mov_nodes_max_err movavg_dist_RMSE mov_dist_max_err

    puts ""
    puts "tx duration full packet: [expr [$range(0) getFullPktTime]]"
    set tot_rmse 0.0
    for {set n 0} { $n < $opt(nn) } { incr n } {
        set tot_rmse [expr $tot_rmse + $movavg_nodes_RMSE($n)]
        puts "Node $n mean RMSE : [format "%.2f" $movavg_nodes_RMSE($n)] max_abs_error: [format "%.2f" $mov_nodes_max_err($n)]"
        puts "node $n end position XYZ: [$position($n) getX_] [$position($n) getY_] [$position($n) getZ_]"
    }

    puts ""
    # for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
    #     for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
    #             puts "Distance ($i,$j) mean RMSE: [format "%.2f" $movavg_dist_RMSE($i,$j)] max_abs_error: [format "%.2f" $mov_dist_max_err($i,$j)]"   
    #     }
    # }
    # puts ""


    if ($opt(verbose)) {
    
        for {set i 0} {$i < $opt(nn)} {incr i}  {
            for {set j 0} {$j < $opt(nn)} {incr j} {		
                if {$i != $j} {
                    puts "cbr link $j -> $i     pkts sent: [$cbr($j,$i) getsentpkts]    pkts recv: [$cbr($i,$j) getrecvpkts]   PER: [$cbr($i,$j) getPER]  THR: [$cbr($i,$j) getthr]"          
                }			
            }
        }
        puts ""
        puts "TX time FULL packet: [$range(0) getFullPktTime]"
        puts ""
        set tot_bytes_saved 0
        set tot_range_bytes_sent 0
        set tot_interf 0
        for {set i 0} {$i < $opt(nn)} {incr i}  {
            puts "---------------------------------------------------------------------"
            puts "- MAC layer statistics for node $i"
            puts ""
            #puts "TX pkts left in buffer: [$mac($i) get_buffer_size]"
           # puts "TX queue discarded p  : [$mac($i) getDiscardedPktsTx]"
            puts "TX DATA packets             : [$mac($i) getDataPktsTx]"
            puts "TX CTRL packets             : [$mac($i) getCtrlPktsTx]"
            puts "TX range_pkts_sent          : [$range($i) getRangePktsTx]"
            puts "RX range_pkts_recv          : [$range($i) getRangePktsRx]"
            puts "RX range_pkts_err           : [$range($i) getRangePktsErr]"  
            puts "RX CTRL packets             : [$mac($i) getCtrlPktsRx]"
            puts "RX CTRL_ERR packets         : [$mac($i) getXCtrlPktsRx]"      
            puts "RX DATA packets             : [$mac($i) getDataPktsRx]"
            puts "RX error packets            : [$mac($i) getErrorPktsRx]"
            #puts "TX DATA packets            : [$mac($i) getDataPktsTx]"
            puts "PHY TX time                 : [$phy($i) getTxTime]"   
            #puts "PHY duty cycle        : [expr [$phy($i) getTxTime]/[$range($i) getFullPktTime]/[$range($i) getRangePktsTx]]"
            puts "TX range_bytes_sent          : [$range($i) getRangeBytesTx]"
            set  tot_range_bytes_sent [expr $tot_range_bytes_sent + [$range($i) getRangeBytesTx]]     
            puts "RANGE bytes saved       : [expr [$range($i) getSavedEntries]*3]"
            set tot_bytes_saved [expr $tot_bytes_saved + [$range($i) getSavedEntries]*3]      
            puts "PHY getTotPktsLost          : [$phy($i) getTotPktsLost]"
            puts "PHY getErrorPktsInterf      : [$phy($i) getErrorPktsInterf]"
            set  tot_interf [expr $tot_interf + [$phy($i) getErrorPktsInterf]]
            puts "PHY getErrorPktsNoise       : [$phy($i) getErrorPktsNoise]"
            # puts "PHY getCollisionsDATA       : [$phy(1) getCollisionsDATA]"
            # puts "PHY getCollisionsCTRL       : [$phy($i) getCollisionsCTRL]"
            # puts "PHY getTotCtrlPktsLost      : [$phy($i) getTotCtrlPktsLost]"
            # puts "PHY getErrorCtrlPktsInterf  : [$phy($i) getErrorCtrlPktsInterf]"
            puts "---------------------------------------------------------------------"
        }
        
        puts "TOT bytes saved: $tot_bytes_saved"
        puts "TOT bytes sent: $tot_range_bytes_sent"
        puts "TOT interf: $tot_interf"
    }
    puts ""
    puts "$opt(mac)_$opt(cbr_period)_$opt(pktsize)"
    puts "TOT mean RMSE: [format "%.2f" [expr $tot_rmse/$opt(nn)]]"

    #Write results in output file
    set rawfile [open $opt(rawfile) w+]
    #foreach {key val} [array get cerr] {puts $rawfile "$key,$val"}
    #puts $rawfile "rep,time,phase,node,d1,d2,range,true,lost_pkts,tx_full,tx_time"
    puts $rawfile "rep,time,phase,node,d1,d2,range,true,lost_pkts,interf_pkts,range_tx,saved_bytes,opt_entries"
    foreach val $csvraw {puts $rawfile $val}
    close $rawfile

    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################
if ($opt(verbose)) {
    puts "\nStarting Simulation\n"
    puts "----------------------------------------------"
}


$ns at [expr $opt(stoptime) + 5.0]  "finish; $ns halt" 

$ns run
