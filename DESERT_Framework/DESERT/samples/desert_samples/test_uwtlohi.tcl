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
# This script is used to test UW-T-LOHI protocol
# There are 25 nodes placed on the seafloor in a square of 5x5 
# with nearest neighbour 1 km apart at 1000 m of depth
# and a fixed sink in the middle of the network at a depth of 600 m retrieving
# data packets from the nodes
#
#
# Author: Federico Favaro <favarofe@dei.unipd.it>
# Version: 1.0.0
#
# NOTE: tcl sample tested on Ubuntu 11.10, 64 bits OS
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
#   |  2. UW/T-LOHI           |
#   +-------------------------+
#   |  1. MPHY/BPSK/Underwater|
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |    UnderwaterChannel    |
#   +-------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 			1
set opt(trace_files)		0
set opt(bash_parameters) 	0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libmphy.so
load libmmac.so
load libMiracleBasicMovement.so
load libUwmStd.so
load libuwinterference.so
load libuwphysical.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwtlohi.so



#############################
# NS-Miracle initialization #
#############################
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################

set opt(start_clock) [clock seconds]

set opt(nn) 				25.0    ;#Number of nodes
set opt(pktsize)	 		125     ;# Size of the packet in Bytes
set opt(stoptime)        	214000  
set opt(dist_nodes) 		1000.0  ;# Distace of the nodes in m
set opt(nn_in_row) 			5       ;# Number of a nodes in m
set opt(ack_mode)	 		"setNoAckMode"

set rng [new RNG]
if {$opt(bash_parameters)} {
	if {$argc != 2} {
		puts "Tcl example need two parameters"
		puts "- The first for seed"
		puts "- The second for CBR period"
		puts "For example, ns test_uw_t_lohi.tcl 4 100"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
	} else { 
		$rng seed 					[lindex $argv 0]
		set opt(rep_num)			[lindex $argv 0]
		set opt(cbr_period)	 		[lindex $argv 1]
	}
} else {
	$rng seed 			1
	set opt(rep_num) 	1
	set opt(cbr_period)	100
}

set opt(cbrpr) [expr int($opt(cbr_period))]
set opt(rnpr)  [expr int($opt(rep_num))]
set opt(apr)   "a"

if {$opt(ack_mode) == "setNoAckMode"} {
   set opt(apr)  "na" 
}
set opt(starttime)       	0.1
set opt(txduration)     	[expr $opt(stoptime) - $opt(starttime)]
set opt(extra_time)			250.0

#PHY PARAMETERS:
set opt(txpower)	 		150 ;# Power in dB re uPa
set opt(per_tgt)	 		0.1
set opt(rx_snr_penalty_db)	0.0
set opt(tx_margin_db)		0.0

set opt(maxinterval_)    	20.0      
set opt(freq) 				25000.0   ;# Frequency in Hz
set opt(bw)              	5000.0    ;# Bandwidth in Hz
set opt(bitrate)	 		4800.0    ;# Bitrate in bps



### TRACE FILE
if {$opt(trace_files)} {	
	set opt(tracefilename) "./test_uwtlohi.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwtlohi.cltr"
	set opt(cltracefile) [open $opt(cltracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null/"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}
###########################
#Random Number Generators #
###########################

global def_rng
set def_rng [new RNG]
$def_rng default

for {set k 0} {$k < $opt(rep_num)} {incr k} {
     $def_rng next-substream
}

#########################
# Module Configuration  #
#########################

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]

set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

Module/UW/CBR set debug_		       0
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1


#PHY BPSK
Module/UW/PHYSICAL  set debug_                     0
Module/UW/PHYSICAL  set BitRate_                   $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_   5.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_           $opt(rx_snr_penalty_db)
Module/UW/PHYSICAL  set TxSPLMargin_dB_            $opt(tx_margin_db)
Module/UW/PHYSICAL  set MaxTxSPL_dB_               $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_               10
Module/UW/PHYSICAL  set MaxTxRange_                50000
Module/UW/PHYSICAL  set PER_target_                $opt(per_tgt)
Module/UW/PHYSICAL  set CentralFreqOptimization_   0
Module/UW/PHYSICAL  set BandwidthOptimization_     0
Module/UW/PHYSICAL  set SPLOptimization_           0

Module/UW/TLOHI set debug_               0
Module/UW/TLOHI set max_prop_delay       0.6579
Module/UW/TLOHI set HDR_size             10
Module/UW/TLOHI set ACK_size             24
Module/UW/TLOHI set seed                 102
Module/UW/TLOHI set max_tx_rounds        1
Module/UW/TLOHI set wait_costant         0.1
Module/UW/TLOHI set max_payload          $opt(pktsize)
Module/UW/TLOHI set recontend_time       [expr 3.0 * 5e-3]

#Phy level for Wake up tone
Module/MPhy/Underwater/WKUP set AcquisitionThreshold_dB_   1.0
Module/MPhy/Underwater/WKUP set ToneDuration               5e-3
Module/MPhy/Underwater/WKUP set TxPower_                   74645.5e7
Module/MPhy/Underwater/WKUP set MaxTxRange_                5000.0
Module/MPhy/Underwater/WKUP set SPLOptimization_	       0
Module/MPhy/Underwater/WKUP set debug_                     0

###################
# TONE CHANNEL    #
###################
set propagation_tone [new MPropagation/Underwater]

set tone_channel [new Module/UnderwaterChannel]

set tone_mask [new MSpectralMask/Rect]
$tone_mask setFreq      $opt(freq)
$tone_mask setBandwidth $opt(bw)


################################
# Procedure(s) to create nodes #
################################


proc createNode { id } {
    
    global channel propagation data_mask ns cbr position node port portnum ipr ipif channel_estimator
    global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global row propagation_tone tone_channel tone_mask
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr($id)  [new Module/UW/CBR] 
    set port($id) [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/TLOHI]
    set phy_data($id)  [new Module/UW/PHYSICAL]
    set phy_tone($id)  [new Module/MPhy/Underwater/WKUP]


    $node($id)  addModule 7 $cbr($id)   1  "CBR"
    $node($id)  addModule 6 $port($id)  1  "PRT"
    $node($id)  addModule 5 $ipr($id)   1  "IPR"
    $node($id)  addModule 4 $ipif($id)  1  "IPF"   
    $node($id) addModule  3 $mll($id)   1  "MLL"
    $node($id)  addModule 2 $mac($id)   1  "MAC"
    $node($id)  addModule 1 $phy_data($id)   1  "PHY"
    $node($id)  addModule 1 $phy_tone($id)   0  "TPHY"

    $node($id) setConnection $cbr($id)   $port($id)  0
    $node($id) setConnection $port($id)  $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy_data($id)   1
    $node($id) setConnection $mac($id)   $phy_tone($id)   1
    $node($id) addToChannel  $channel    $phy_data($id)   1
    $node($id) addToChannel $tone_channel	 $phy_tone($id)	  1


    set portnum($id) [$port($id) assignPort $cbr($id) ]

    #$ipif($id) addr "1.0.0.${id}"
    $ipif($id) addr [expr $id + 1]

    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)
     ##########################
     #  NODES PLACEMENT	      #
     ##########################
    set curr_x 0.0
    set curr_y 0.0
    if { $id < 5 } {
    	set curr_x [expr $opt(dist_nodes) * $id ]
    	set row 0
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 5)  &&  ($id < 10) } {
    	set curr_x  [expr ($id -($opt(nn)/5 )) * $opt(dist_nodes) ]
    	set row 1
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 10) && ($id < 15) } {
	set curr_x  [expr ($id -($opt(nn)/5 )*2) * $opt(dist_nodes) ]
    	set row 2
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 15) && ($id < 20) } {
	set curr_x  [expr ($id -($opt(nn)/5 )*3) * $opt(dist_nodes) ]
    	set row 3
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 20) && ($id < 25) } {
	set curr_x  [expr ($id -($opt(nn)/5 )*4) * $opt(dist_nodes) ]
    	set row 4
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    }

    $position($id) setX_ $curr_x

    $position($id) setY_ $curr_y

    $position($id) setZ_ -100

    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0
    

    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
    $phy_data($id) set debug_ 0
    
    set interf_tone($id) [new "Module/UW/INTERFERENCE"]
    $interf_tone($id) set maxinterval_ $opt(maxinterval_)
    $interf_tone($id) set debug_       0

    $phy_tone($id) setSpectralMask $tone_mask
    $phy_tone($id) setPropagation $propagation_tone
    $phy_tone($id) setInterference $interf_tone($id)
    $phy_tone($id) set debug_ 0


    $mac($id) set seed [expr $id * 3]
    $mac($id) setDataName "Module/UW/PHYSICAL"
    $mac($id) initialize
    $mac($id) setAggressiveUnsyncMode
    $mac($id) $opt(ack_mode)

}

proc createSink { } {
    global channel propagation smask data_mask ns cbr_sink position_sink node_sink port_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator
    global woss_utilities woss_creator position
    global auv_curr_depth propagation_tone tone_channel tone_mask

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
	  set cbr_sink($cnt)  [new Module/UW/CBR] 
      }
    
    set port_sink      [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/TLOHI]
    set phy_data_sink  [new Module/UW/PHYSICAL]
    set phy_tone_sink  [new Module/MPhy/Underwater/WKUP]
 

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
	$node_sink addModule 7 $cbr_sink($cnt) 0 "CBR"
     }
     $node_sink addModule 6 $port_sink      0 "PRT"
     $node_sink addModule 5 $ipr_sink       0 "IPR"
     $node_sink addModule 4 $ipif_sink      0 "IPF"   
     $node_sink addModule 3 $mll_sink       0 "MLL"
     $node_sink addModule 2 $mac_sink       1 "MAC"
     $node_sink addModule 1 $phy_data_sink  1 "PHY"
     $node_sink addModule 1 $phy_tone_sink  0 "TPHY"

     for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
     $node_sink setConnection $cbr_sink($cnt)  $port_sink     	1   
      }
     $node_sink setConnection $port_sink $ipr_sink      	0
     $node_sink setConnection $ipr_sink  $ipif_sink       	0
     $node_sink setConnection $ipif_sink $mll_sink        	0 
     $node_sink setConnection $mll_sink  $mac_sink        	0
     $node_sink setConnection $mac_sink  $phy_data_sink   	1
     $node_sink setConnection $mac_sink  $phy_tone_sink		1
     $node_sink addToChannel $channel    $phy_data_sink   	1
     $node_sink addToChannel $tone_channel $phy_tone_sink	1

     for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_sink($cnt) [$port_sink assignPort $cbr_sink($cnt)]
     }
	  
	 set position_sink [new "Position/BM"]    
     set auv_curr_depth 600
     
    
     # $position_sink setLatitude_ [$position(12) getLatitude_]
     # $position_sink setLongitude_ [$position(12) getLongitude_]
     # $position_sink setAltitude_  [expr -1.0 * $auv_curr_depth]
     
     $node_sink addPosition $position_sink

     $position_sink setX_ [$position(12) getX_]
     $position_sink setY_ [$position(12) getY_]
     $position_sink setZ_ [expr -1.0*$auv_curr_depth]

    
     $ipif_sink addr 253

     set interf_data_sink [new "Module/UW/INTERFERENCE"]
     $interf_data_sink set maxinterval_ $opt(maxinterval_)
     $interf_data_sink set debug_       0

     $phy_data_sink setSpectralMask     $data_mask
     $phy_data_sink setPropagation      $propagation
     $phy_data_sink setInterference     $interf_data_sink
     
      set interf_tone_sink [new "Module/UW/INTERFERENCE"]
      $interf_tone_sink set maxinterval_ $opt(maxinterval_)
      $interf_tone_sink set debug_       0

      $phy_tone_sink setSpectralMask $tone_mask
      $phy_tone_sink setPropagation $propagation_tone
      $phy_tone_sink setInterference $interf_tone_sink
      $phy_tone_sink set debug_ 0


    $mac_sink set seed [expr 3]
    $mac_sink setDataName "Module/UW/PHYSICAL"
    $mac_sink initialize
    $mac_sink setAggressiveUnsyncMode
    $mac_sink $opt(ack_mode)

}


###############################
# routing of nodes
###############################

proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    $cbr_sink($id1) set destPort_ $portnum($id1)  
    $ipr($id1) addRoute [$ipif_sink addr] [$ipif_sink addr]
    $ipr_sink  addRoute [$ipif($id1) addr] [$ipif($id1) addr]
}

###############################
# create nodes
###############################

for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}

###############################
# create sink
###############################

createSink
################################
#Setup flows
################################

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    connectNodes $id1
}

################################
#fill ARP tables
################################

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
	$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }   
    $mll($id1) addentry [$ipif_sink addr] [ $mac_sink addr]
    $mll_sink addentry [$ipif($id1) addr] [ $mac($id1) addr]
}


################################
#Start cbr(s)
################################


set force_stop $opt(stoptime)

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
      $ns at $opt(starttime)	     "$cbr($id1) start"
      $ns at $opt(stoptime)          "$cbr($id1) stop"
}




proc finish { } {
    global ns opt cbr mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global woss_manager outfile outfile_sink
    
	if {$opt(verbose)} {
		puts "\n"
		puts "CBR_PERIOD : $opt(cbr_period)"
		puts "SEED: $opt(rep_num)"
		puts "NUMBER OF NODES: $opt(nn)"
	} else {
		puts "Simulation done!"
	}
    set sum_cbr_throughput 	0
    set sum_per		0
    set sum_cbr_sent_pkts	0.0
    set sum_cbr_rcv_pkts	0.0
    set consumed_energy_tx_node	0.0
    set consumed_energy_rx_node	0.0
    set stdby_time		0.0
    set total_stdby_time	0.0

	
    for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
	set cbr_throughput	   [$cbr_sink($id3) getthr]
	set cbr_per	           [$cbr_sink($id3) getper]
	set cbr_pkts         [$cbr($id3) getsentpkts]
	set cbr_rcv_pkts       [$cbr_sink($id3) getrecvpkts]
	################################################
	set ftt					[$cbr_sink($id3) getftt]
	set ftt_std				[$cbr_sink($id3) getfttstd]
	#################################################
	if {$opt(verbose)} {
		puts "cbr_sink($id3) throughput                : $cbr_throughput"
		puts "cbr_sink($id3) packet error rate         : $cbr_per"
		puts "cbr($id3) sent packets 	       	       : $cbr_pkts"
		puts "cbr_sink($id3) received packets          : $cbr_rcv_pkts"
	}
	

	set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
	set sum_per [expr $sum_per + $cbr_per]
	set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_pkts]
	set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
	###############################################
	###############################################
    }
    if {$opt(verbose)} {
		puts "Mean throughput:  [expr ($sum_cbr_throughput/($opt(nn)))]"
		puts "Total transmitted packets: [expr ($sum_cbr_sent_pkts)]"
		puts "Total received packets: [expr ($sum_cbr_rcv_pkts)]"
		puts "Mean PER: [expr (1 - ($sum_cbr_rcv_pkts/($sum_cbr_sent_pkts)))]"
	}
    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################

puts -nonewline "\nSimulating...\n"

$ns at [expr $opt(stoptime) + $opt(extra_time)]  "finish; $ns halt" 

$ns run
    
