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
# This script is used to test UW/RANGING_TOKENBUS layer
# There are 3 nodes placed in line that can transmit each other 
# packets with a CBR (Constant Bit Rate) Application Module
#
#
# Author: Antonio Montanari
# Version: 1.0.0
#
# NOTE: tcl sample tested on WSL2 over Win10, 64 bits OS
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
#   |  2. UW/RANGING_TOKENBUS         |
#   +-------------------------+
#   |  1. UW/HMMPHYSICAL      |
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |    UnderwaterChannel    |
#   +-------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 			0
set opt(trace_files)		0
set opt(bash_parameters) 	0

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
load libuwphysical.so
load libuwhmmphysical.so
load libuwtokenbus.so
load libuwranging_tokenbus.so
load libuwsmposition.so
#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(verbose)            1
set opt(nn)                 10 ;# Number of Nodes
set opt(node_speed)         1.0
set opt(XYdiameter)         1200.0; #maximum field diameter in XY plane
set opt(XYinitdiam)         5.0;# diameter of initial positions
set opt(Zwander)            0.0; #maximum wander of nodes in depth
set opt(step)               5; #period (s) between course changes and ranging updates
set opt(printsteps)         0; # to print statistics for each step 
set opt(min_token_hold)     0.0001
set opt(rawfile)            "./tb$opt(nn).csv"; #format of each line will be time_of_measure,measuring_node,from_node_i,to_node_j,ranging_error
set opt(starttime)          0	
set opt(stoptime)           600
set opt(soundspeed)         1500.0
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation
set opt(txpower)            180.0  ;#Power transmitted in dB re uPa
set opt(maxinterval_)       20.0
set opt(freq)               25000.0 ;#Frequency used in Hz
set opt(bw)                 5000.0	;#Bandwidth used in Hz
set opt(bitrate)            4800.0	;#bitrate in bps
set opt(ack_mode)           "setNoAckMode"
set opt(cbr_period)         9999999
set opt(pktsize)            8 ;# Pkt size in byte
set opt(rngstream)	            13

if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires three inputs:"
        puts "- the first one is the cbr packet size (byte);"
        puts "- the second one is the cbr poisson period (seconds);"
        puts "- the third one is the random generator substream;"
        puts "example: ns uwcbr.tcl 125 60 13"
        puts "Please try again."
        return
    } else {
        set opt(pktsize)       [lindex $argv 0]
        set opt(cbr_period)    [lindex $argv 1]
        set opt(rngstream)       [lindex $argv 2]
    }
}
###########################
#Random Number Generators #
###########################

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

proc updateCourses {} {
    global opt position rng
    for {set n 0} { $n < $opt(nn) } { incr n } {
        $position($n) setdest [$rng testdouble $opt(XYdiameter)] [$rng testdouble $opt(XYdiameter)] -200 $opt(node_speed)
    }
}

###########################
#Trace files              #
###########################
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_hmmphy.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_hmmphy.cltr"
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

#RANGING_TOKENBUS
#max_tt_ should be the maximum travel times between any node
#set max_tt_ [expr hypot(hypot($opt(XYdiameter),$opt(XYdiameter)),$opt(Zwander))/$opt(soundspeed)]
set max_tt_ [expr {$opt(XYdiameter)/$opt(soundspeed)}]
puts [expr {$opt(XYdiameter)/$opt(soundspeed)}]
puts "slot time: $max_tt_ token_pass_timeout: [expr {2.0*$max_tt_ + $opt(min_token_hold)}] bus_idle_timeout_: [expr {$max_tt_ + $opt(min_token_hold)}]"
Module/UW/RANGING_TOKENBUS set n_nodes_ 		        $opt(nn)
Module/UW/RANGING_TOKENBUS set slot_time_ 		    $max_tt_ 
Module/UW/RANGING_TOKENBUS set min_token_hold_time_ 	$opt(min_token_hold)
Module/UW/RANGING_TOKENBUS set token_pass_timeout_ 	[expr {2.0*$max_tt_ + 2.0*$opt(min_token_hold)}]
Module/UW/RANGING_TOKENBUS set bus_idle_timeout_ 	[expr {$max_tt_ + 2.0*$opt(min_token_hold)}]
Module/UW/RANGING_TOKENBUS set queue_size_ 		    1000
Module/UW/RANGING_TOKENBUS set debug_tb 		        0
Module/UW/RANGING_TOKENBUS set debug_ 		        0
Module/UW/RANGING_TOKENBUS set max_token_hold_time_ 	10
Module/UW/RANGING_TOKENBUS set drop_old_ 		    0
Module/UW/RANGING_TOKENBUS set checkPriority_ 		0
Module/UW/RANGING_TOKENBUS set mac2phy_delay_ 		[expr 1.0e-9]
Module/UW/RANGING_TOKENBUS set epsilon 		        [expr 1e-6]
Module/UW/RANGING_TOKENBUS set max_tt 		        [expr $max_tt_*1.1]

# var binded by UW/PHYSICAL
Module/UW/HMMPHYSICAL  set TxPower_                    $opt(txpower)
Module/UW/HMMPHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/HMMPHYSICAL  set AcquisitionThreshold_dB_    4.0 
Module/UW/HMMPHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/HMMPHYSICAL  set TxSPLMargin_dB_             0
Module/UW/HMMPHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/HMMPHYSICAL  set MinTxSPL_dB_                10
Module/UW/HMMPHYSICAL  set MaxTxRange_                 10000
Module/UW/HMMPHYSICAL  set PER_target_                 0    
Module/UW/HMMPHYSICAL  set CentralFreqOptimization_    0
Module/UW/HMMPHYSICAL  set BandwidthOptimization_      0
Module/UW/HMMPHYSICAL  set SPLOptimization_            0
Module/UW/HMMPHYSICAL  set ConsumedEnergy_             0
Module/UW/HMMPHYSICAL  set NoiseSPD_                   0
Module/UW/HMMPHYSICAL  set debug_                      0
####################################
Position/UWSM   set debug_  0

#var binded by UW/HMMPHYSICAL
Module/UW/HMMPHYSICAL  set step_duration               1  ; # sampling period for channel state transitions
####################################

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

    global channel propagation data_mask ns cbr position node udp portnum ipr ipif channel_estimator
    global phy posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global node_coordinates
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		set cbr($id,$cnt)  [new Module/UW/CBR] 
		
	}
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL]
    set mac($id)  [new Module/UW/RANGING_TOKENBUS]
    set phy($id)  [new Module/UW/HMMPHYSICAL]  
	
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		$node($id) addModule 7 $cbr($id,$cnt)   1  "CBR"
	}
    $node($id) addModule 6 $udp($id)   1  "UDP"
    $node($id) addModule 5 $ipr($id)   1  "IPR"
    $node($id) addModule 4 $ipif($id)  1  "IPF"   
    $node($id) addModule 3 $mll($id)   1  "MLL"
    $node($id) addModule 2 $mac($id)   1  "MAC"
    $node($id) addModule 1 $phy($id)   1  "PHY"

	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		$node($id) setConnection $cbr($id,$cnt)   $udp($id)   1
		
		set portnum($id,$cnt) [$udp($id) assignPort $cbr($id,$cnt) ]
	}
    $node($id) setConnection $udp($id)   $ipr($id)   1
    $node($id) setConnection $ipr($id)   $ipif($id)  1
    $node($id) setConnection $ipif($id)  $mll($id)   1
    $node($id) setConnection $mll($id)   $mac($id)   1
    $node($id) setConnection $mac($id)   $phy($id)   1
    $node($id) addToChannel  $channel    $phy($id)   1

    if {$id > 254} {
		puts "hostnum > 254!!! exiting"
		exit
    }

    #Set the IP address of the node
    set ip_value [expr $id + 1]
    $ipif($id) addr $ip_value

    #$mac($id) setMacAddr    [expr $id + 1]
    # $mac($id) setNodeId [expr $id]
    
    set position($id) [new "Position/UWSM"]
    $node($id) addPosition $position($id)
    
    proc initpos_circle {id} {
        global opt position ns
        set pi 3.1415926535897931
        $position($id) setX_ [expr {0.5*$opt(XYinitdiam)*cos($id*2*$pi/$opt(nn))}]
        $position($id) setY_ [expr {0.5*$opt(XYinitdiam)*sin($id*2*$pi/$opt(nn))}]
        $position($id) setZ_ -200
        $ns at 0.001 "$position($id) setdest [expr {0.5*$opt(XYdiameter)*cos($id*2*$pi/$opt(nn))}] [expr {0.5*$opt(XYdiameter)*sin($id*2*$pi/$opt(nn))}] -200 $opt(node_speed)"
        $ns at 300 "$position($id) setdest [expr {0.5*$opt(XYinitdiam)*cos($id*2*$pi/$opt(nn))}] [expr {0.5*$opt(XYinitdiam)*sin($id*2*$pi/$opt(nn))}] -200 $opt(node_speed)"
    }    
    #Setup positions
    # $position($id) setX_ 0
    # $position($id) setY_ [expr $id*15.0]
    # $position($id) setZ_ -200
    initpos_circle $id
    
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

##################################
# Setup MCLinks with dummy data  #
##################################

# new MCLink p_succ_good p_succ_bad p_gb p_bg [GOOD/BAD [cur_step]]
set ber_good [expr 0.0051]
set ber_bad [expr 0.0193]
set p_gb [expr 0.053]
set p_bg [expr 0.192]
set pktbits [expr {($opt(nn))*16+8}]; #number of bits in the packet
set p_succ_good [expr {pow((pow(1-$ber_good,7) + 7*$ber_good*pow(1-$ber_good,6)),($pktbits/4))}]
set p_succ_bad [expr {pow((pow(1-$ber_bad,7) + 7*$ber_bad*pow(1-$ber_bad,6)),($pktbits/4))}]
set mclink_near [new Module/UW/HMMPHYSICAL/MCLINK $p_succ_good $p_succ_bad $p_gb $p_bg GOOD]
set mclink_ideal [new Module/UW/HMMPHYSICAL/MCLINK 1 1 $p_gb $p_bg GOOD]
set mclink_off [new Module/UW/HMMPHYSICAL/MCLINK 0 0 $p_gb $p_bg GOOD]
puts "p_succ_good: $p_succ_good"
puts "p_succ_bad: $p_succ_bad"

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
            $phy($id1) setMCLink [$mac($id2) addr] $mclink_ideal
            # $ns at 200 "$phy($id1) setMCLink $id2 [new Module/UW/HMMPHYSICAL/MCLINK $p_succ_good $p_succ_bad $p_gb $p_bg GOOD]"
            # $ns at 400 "$phy($id1) setMCLink $id2 $mclink_near"
        } 
    }
}
# # turn off all the links from and to node "nodeoff" 
# set nodeoff 2
# for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
#         $ns at 100 "$phy($nodeoff) setMCLink [$mac($id1) addr] $mclink_off"
#         $ns at 200 "$phy($nodeoff) setMCLink [$mac($id1) addr] $mclink_ideal"
#         $ns at 400 "$phy($nodeoff) setMCLink [$mac($id1) addr] $mclink_off"
#         $ns at 500 "$phy($nodeoff) setMCLink [$mac($id1) addr] $mclink_ideal"

#         $ns at 100 "$phy($id1) setMCLink [$mac($nodeoff) addr] $mclink_off"
#         $ns at 200 "$phy($id1) setMCLink [$mac($nodeoff) addr] $mclink_ideal"
#         $ns at 400 "$phy($id1) setMCLink [$mac($nodeoff) addr] $mclink_off"
#         $ns at 500 "$phy($id1) setMCLink [$mac($nodeoff) addr] $mclink_ideal" 
# }

################################
# Inter-node module connection #
################################
proc connectNodes {id1 des1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink ipr_sink opt 

    $cbr($id1,$des1) set destAddr_ [$ipif($des1) addr]
    $cbr($id1,$des1) set destPort_ $portnum($des1,$id1)
}

##################
# Setup flows    #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
		connectNodes $id1 $id2
	}
}

##################
# ARP tables     #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
	}
}



##################
# Routing tables #
##################
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
set dnum [expr {$opt(nn)*[expr $opt(nn)-1]/2}]


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
    global mac opt
    set dist [expr {$opt(soundspeed)*[$mac($n) get_distance $i $j]}]
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
                set true_dists($i,$j) [expr [trueDist $i $j]]      
        }
    }
}


# Updates (and prints) distances for EVERY node
proc updateDist {print} {
    updateTrueDist
    global opt true_dists calc_dists nodes_RMSE mac
    for {set n 0} {$n < $opt(nn)} {incr n}  {
        $mac($n) calc_distances
        for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
            for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
                set calc_dists($n,$i,$j) [expr [calcDist $n $i $j]]
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
    updateDist 0
    #updateCourses
    global opt dnum calc_dists true_dists nodes_RMSE nodes_max_err dist_RMSE dist_max_err 
    global mov_dist_max_err mov_nodes_max_err movavg_dist_RMSE movavg_nodes_RMSE cerr csvraw mac
    #calculate node-wise statistics
    for {set n 0} {$n < $opt(nn)} {incr n}  {
        set nodes_max_err($t,$n) 0.0
        set nodes_RMSE($t,$n) 0.0
        for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
            for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
                set cerr($t,$n,$i,$j) [expr {$calc_dists($n,$i,$j) - $true_dists($i,$j)}]
                lappend csvraw "$t,$n,$i,$j,[expr $calc_dists($n,$i,$j)],[expr $true_dists($i,$j)],[$mac($i) get_token_pass_exp],[$mac($i) get_bus_idle_exp]"
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
#for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
#    $ns at $opt(starttime)    "$mac($id1) start"
#    $ns at $opt(stoptime)    "$mac($id1) stop"
# 	for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
# 		if {$id1 != $id2} {
# 			$ns at $opt(starttime)    "$cbr($id1,$id2) start"
# 			$ns at $opt(stoptime)     "$cbr($id1,$id2) stop"
# 		}
# 	}
# }


# Schedule events on ns
for {set t $opt(step)} { $t < $opt(stoptime) } { incr t $opt(step)} {
    $ns at $t "updateAll $t $opt(printsteps)"
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt position csvraw
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global node_coordinates
    global ipr_sink ipr ipif udp cbr phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats
    global cerr movavg_nodes_RMSE mov_nodes_max_err movavg_dist_RMSE mov_dist_max_err

    puts ""
    for {set n 0} { $n < $opt(nn) } { incr n } {
        puts "Node $n mean RMSE : [format "%.2f" $movavg_nodes_RMSE($n)] max_abs_error: [format "%.2f" $mov_nodes_max_err($n)]"
        puts "node $n end position XYZ: [$position($n) getX_] [$position($n) getY_] [$position($n) getZ_]"
    }
    puts ""
    for {set i 0} {$i < [expr $opt(nn) - 1]} {incr i}  {
        for {set j [expr $i + 1]} {$j < $opt(nn)} {incr j} { 
                puts "Distance ($i,$j) mean RMSE: [format "%.2f" $movavg_dist_RMSE($i,$j)] max_abs_error: [format "%.2f" $mov_dist_max_err($i,$j)]"   
        }
    }
    puts ""

    if ($opt(verbose)) {
    
        # for {set i 0} {$i < $opt(nn)} {incr i}  {
        #     for {set j 0} {$j < $opt(nn)} {incr j} {		
        #         if {$i != $j} {
        #             puts "cbr link $j -> $i     pkts sent: [$cbr($j,$i) getsentpkts]    pkts recv: [$cbr($i,$j) getrecvpkts]   PER: [$cbr($i,$j) getPER]  THR: [$cbr($i,$j) getthr]"          
        #         }			
        #     }
        # }
        # puts ""

        for {set i 0} {$i < $opt(nn)} {incr i}  {
            puts "---------------------------------------------------------------------"
            puts "- MAC layer statistics for node $i"
            puts ""
            puts "TX pkts left in buffer: [$mac($i) get_buffer_size]"
            puts "TX queue discarded p  : [$mac($i) getDiscardedPktsTx]"
            puts "TX CTRL packets       : [$mac($i) getCtrlPktsTx]" 
            puts "RX CTRL packets       : [$mac($i) getCtrlPktsRx]"
            puts "RX CTRL_ERR packets   : [$mac($i) getXCtrlPktsRx]"      
            puts "RX DATA packets       : [$mac($i) getDataPktsRx]"
            puts "RX error packets      : [$mac($i) getErrorPktsRx]"
            puts "TX DATA packets       : [$mac($i) getDataPktsTx]"    
            puts "---------------------------------------------------------------------"
        }  
        puts ""
    }

    #Write results in output file
    set rawfile [open $opt(rawfile) w]
    #foreach {key val} [array get cerr] {puts $rawfile "$key,$val"}
    puts $rawfile "time,node,d1,d2,range,true,resend,busidle"
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


$ns at [expr $opt(stoptime) + 1.0]  "finish; $ns halt" 

$ns run
