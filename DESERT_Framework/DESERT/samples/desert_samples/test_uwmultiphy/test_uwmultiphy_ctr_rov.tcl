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
# This script is used to test MULTI_STACK_CONTROLLER_PHY MASTER and SLAVE module
# There are 2 nodes that can transmit each other packets with a CBR (Constant 
# Bit Rate) Application Module
# The MASTER controls the switch between three UW/PHYSICAL layers with different
# frequency and bandwidth, according with the received power metrics. The slave 
# switches according to the MASTER behavior. UnderwaterChannel is used as channel.
#
# Author: Filippo Campagnaro <campagn1@dei.unipd.it>
# Version: 1.0.0
#
# NOTE: tcl sample tested on Ubuntu 11.10, 64 bits OS
#
# Stack of the nodes
#                   MASTER                                         SLAVE 
#   +------------------------------------------+   +------------------------------------------+
#   |  10. UW/ROV/CTR                          |   |  10. UW/ROV                              |
#   +------------------------------------------+   +------------------------------------------+
#   |  9. UW/UDP                               |   |  9. UW/UDP                               |
#   +------------------------------------------+   +------------------------------------------+
#   |  8. UW/STATICROUTING                     |   |  8. UW/STATICROUTING                     |
#   +------------------------------------------+   +------------------------------------------+
#   |  7. UW/IP                                |   |  7. UW/IP                                |
#   +------------------------------------------+   +------------------------------------------+
#   |  6. UW/MLL                               |   |  6. UW/MLL                               | 
#   +------------------------------------------+   +------------------------------------------+
#   |  5. UW/TDMA                              |   |  5. UW/TDMA                              |
#   +------------------------------------------+   +------------------------------------------+
#   |  4. UW/MULTI_STACK_CONTROLLER_PHY_MASTER |   |  4. UW/MULTI_STACK_CONTROLLER_PHY_SLAVE  |
#   +--------------+-------------+-------------+   +--------------+-------------+-------------+
#   | 3.UW/PHYSICAL|2.UW/PHYSICAL|1.UW/PHYSICAL|   | 3.UW/PHYSICAL|2.UW/PHYSICAL|1.UW/PHYSICAL|
#   +--------------+-------------+-------------+   +--------------+-------------+-------------+ 
#           |             |              |                  |             |            |     
#   +-----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                   |   
#   +-----------------------------------------------------------------------------------------+   

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
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwcsmaaloha.so
load libuwaloha.so
load libuwinterference.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwhermesphy.so
load libuwoptical_propagation.so
load libuwem_channel.so
load libuwoptical_channel.so
load libuwoptical_phy.so
load libuwrov.so
load libuwsmposition.so
load libuwmmac_clmsgs.so
load libuwtdma.so
load libuwmulti_stack_controller.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################

set opt(nn)                 2.0 ;# Number of Nodes
set opt(ROV_pktsize)            1000;#125  ;# Pkt size in byte
set opt(CTR_pktsize)            124;#125  ;# Pkt size in byte
set opt(ROV_period)       0.02

set opt(starttime)          1	
set opt(stoptime)           2800 
set opt(time_interval)      12
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation


set opt(maxinterval_)       20.0
set opt(ack_mode)           "setNoAckMode"
set opt(rngstream)	1
####################
# Evologics' 48/78 #
####################

# set opt(freq)               63000.0 ;#Frequency used in Hz
# set opt(bw)                 30000.0	;#Bandwidth used in Hzz
# set opt(bitrate)            30000.0	;#bitrate in bps
# set opt(txpower)            160.0  ;#Power transmitted in dB re uPa

# #####################
# # Evologics' S2C HS #
# #####################

set opt(freq)               160000.0 ;#Frequency used in Hz
set opt(bw)                 80000.0 ;#Bandwidth used in Hzz
set opt(bitrate)            60000.0 ;#bitrate in bps, up to 62.5
set opt(txpower)            177.0  ;#Power transmitted in dB re uPa (10 W)

######################
# Hermes             #
######################

set opt(hermes_freq)               375000.0 ;#Frequency used in Hz
set opt(hermes_bw)                 76000.0 ;#Bandwidth used in Hz
set opt(hermes_bitrate)            87768.0 ;#150000;#bitrate in bps
set opt(hermes_txpower)            180.0  ;#Power transmitted in dB re uPa (32 W)

######################
# Optical            #
######################

set opt(optical_freq)              10000000
# set opt(optical_bw)                100000
# set opt(optical_bitrate)           1000000
set opt(optical_bw)                200000
set opt(optical_bitrate)           2000000
set opt(optical_txpower)           100
set opt(opt_acq_db)        10
set opt(temperatura)       293.15 ; # in Kelvin
set opt(txArea)            0.000010
set opt(rxArea)            0.0000011 ; # receveing area, it has to be the same for optical physical and propagation
set opt(c)                 0.15 ; # seawater attenation coefficient
set opt(theta)             1
set opt(id)                [expr 1.0e-9]
set opt(il)                [expr 1.0e-6]
set opt(shuntRes)          [expr 1.49e9]
set opt(sensitivity)       0.26
set opt(LUTpath)           "../dbs/optical_noise/LUT.txt"

###################################################
# Multi stack controller signaling configuaration #
###################################################

set opt(master_signaling_active) 1
set opt(signaling_size)          5


##################################
# Switching thresholds           #
##################################

#set opt(evo2hermes_thresh) 3.846e12;# 119.5 m 48/78
set opt(evo2hermes_thresh) 9.893e13;# 119.5m HS
set opt(hermes2evo_thresh) 6.379e13; #120.5m
set opt(hermes2opt_thresh) 1.58e16
set opt(opt2hermes_thresh) 1e-8

#################
# Waypoint file #
#################
# set opt(waypoint_file)  "../dbs/wp_path/rov_path.csv"
set opt(waypoint_file)  "../dbs/wp_path/rov_path.csv"

if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the first for the seed"
		puts "- the second one is for the Poisson ROV period"
		puts "- the third one is the ROV packet size (byte);"
		puts "example: ns test_uw_csma_aloha_fully_connected.tcl 1 60 125"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
		return
	} else {
		set opt(rngstream)    [lindex $argv 0]
		set opt(ROV_period) [lindex $argv 1]
		set opt(ROV_pktsize)    [lindex $argv 2]
	}
} 

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwcsmaaloha.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwcsmaaloha.cltr"
	set opt(cltracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}

MPropagation/Underwater set practicalSpreading_ 1.5
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

set hermes_data_mask [new MSpectralMask/Rect]
$hermes_data_mask setFreq       $opt(hermes_freq)
$hermes_data_mask setBandwidth  $opt(hermes_bw)

set optical_data_mask [new MSpectralMask/Rect]
$optical_data_mask setFreq       $opt(optical_freq)
$optical_data_mask setBandwidth  $opt(optical_bw)

#########################
# Module Configuration  #
#########################

#TDMA

Module/UW/TDMA set frame_duration 6.0
Module/UW/TDMA set debug_               0
# Module/UW/TDMA set guard_time 0.5 
#Module/UW/TDMA set guard_time 0.2 
Module/UW/TDMA set ACK_size_           0
Module/UW/TDMA set max_tx_tries_               1
Module/UW/TDMA set max_payload_                10000
Module/UW/TDMA set ACK_timeout_                10000.0
Module/UW/TDMA set listen_time_          [expr 1.0e-8]
Module/UW/TDMA set wait_costant_         [expr 1.0e-12]
Module/UW/TDMA set fair_mode        0

Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                0
Module/UW/PHYSICAL  set MaxTxRange_                 50000
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

Module/UW/HERMES/PHY  set BitRate_                    $opt(hermes_bitrate)
Module/UW/HERMES/PHY  set AcquisitionThreshold_dB_    15.0 
Module/UW/HERMES/PHY  set RxSnrPenalty_dB_            0
Module/UW/HERMES/PHY  set TxSPLMargin_dB_             0
Module/UW/HERMES/PHY  set MaxTxSPL_dB_                $opt(hermes_txpower)
Module/UW/HERMES/PHY  set MinTxSPL_dB_                0
Module/UW/HERMES/PHY  set MaxTxRange_                 200
Module/UW/HERMES/PHY  set PER_target_                 0    
Module/UW/HERMES/PHY  set CentralFreqOptimization_    0
Module/UW/HERMES/PHY  set BandwidthOptimization_      0
Module/UW/HERMES/PHY  set SPLOptimization_            0
Module/UW/HERMES/PHY  set debug_                      0

Module/UW/OPTICAL/PHY   set TxPower_                    $opt(optical_txpower)
Module/UW/OPTICAL/PHY   set BitRate_                    $opt(optical_bitrate)
Module/UW/OPTICAL/PHY   set AcquisitionThreshold_dB_    $opt(opt_acq_db)
Module/UW/OPTICAL/PHY   set Id_                         $opt(id)
Module/UW/OPTICAL/PHY   set Il_                         $opt(il)
Module/UW/OPTICAL/PHY   set R_                          $opt(shuntRes)
Module/UW/OPTICAL/PHY   set S_                          $opt(sensitivity)
Module/UW/OPTICAL/PHY   set T_                          $opt(temperatura)
Module/UW/OPTICAL/PHY   set Ar_                         $opt(rxArea)
Module/UW/OPTICAL/PHY   set debug_                      0

Module/UW/OPTICAL/Propagation set Ar_       $opt(rxArea)
Module/UW/OPTICAL/Propagation set At_       $opt(txArea)
Module/UW/OPTICAL/Propagation set c_        $opt(c)
Module/UW/OPTICAL/Propagation set theta_    $opt(theta)
Module/UW/OPTICAL/Propagation set debug_    0

set optical_propagation [new Module/UW/OPTICAL/Propagation]
$optical_propagation setOmnidirectional

set optical_channel [new Module/UW/Optical/Channel]


Module/UW/ROV set packetSize_          $opt(ROV_pktsize)
Module/UW/ROV set period_              $opt(ROV_period)
Module/UW/ROV set debug_               0

Module/UW/ROV/CTR set packetSize_          $opt(CTR_pktsize)
Module/UW/ROV/CTR set debug_               0

Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE set debug_      0
Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER set debug_     0
# Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER set alpha_     0.5
Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER set signaling_active_ $opt(master_signaling_active)
Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER set signaling_period_ 100
Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE set min_delay_  [expr 1.79e-4]
################################
# Procedure(s) to create nodes #
################################
proc createNode {id} {

  global channel propagation optical_propagation data_mask hermes_data_mask optical_data_mask ns application position node udp portnum
  global phy posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager channel_estimator ipr ipif 
  global node_coordinates optical_channel ctr
  
  set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
	if {$id == 1} {
    set application($id) [new Module/UW/ROV]
    set ctr($id)  [new Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE]
  } else {
    set application($id) [new Module/UW/ROV/CTR]
    set ctr($id)  [new Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER]
  }  
	set udp($id)  [new Module/UW/UDP]
  set ipr($id)  [new Module/UW/StaticRouting]
  set ipif($id) [new Module/UW/IP]
  set mll($id)  [new Module/UW/MLL] 
  set mac($id)  [new Module/UW/TDMA]
  $mac($id) setMacAddr [expr $id + 1]
  set phy($id)  [new Module/UW/PHYSICAL] 
  set hermes_phy($id)  [new Module/UW/HERMES/PHY]  
  set optical_phy($id)  [new Module/UW/OPTICAL/PHY]  
	
	
	$node($id) addModule 10 $application($id)   1  "CBR"
	$node($id) addModule 9 $udp($id)   1  "UDP"
  $node($id) addModule 8 $ipr($id)   1  "IPR"
  $node($id) addModule 7 $ipif($id)  1  "IPF"   
  $node($id) addModule 6 $mll($id)   1  "MLL"
  $node($id) addModule 5 $mac($id)   1  "MAC"
  $node($id) addModule 4 $ctr($id)   1  "CTR"
  $node($id) addModule 3 $phy($id)   1  "PHY1"
  $node($id) addModule 2 $hermes_phy($id)   1  "PHY2"
  $node($id) addModule 1 $optical_phy($id)   1  "PHY3"

	$node($id) setConnection $application($id)   $udp($id)   0
  set portnum($id) [$udp($id) assignPort $application($id) ]
	$node($id) setConnection $udp($id)   $ipr($id)   0
  $node($id) setConnection $ipr($id)   $ipif($id)  1
  $node($id) setConnection $ipif($id)  $mll($id)   1
  $node($id) setConnection $mll($id)   $mac($id)   1
  $node($id) setConnection $mac($id)   $ctr($id)   1
  $node($id) setConnection $ctr($id)   $phy($id)   1
  $node($id) setConnection $ctr($id)   $hermes_phy($id)  1
  $node($id) setConnection $ctr($id)   $optical_phy($id)  1
  $node($id) addToChannel  $channel    $phy($id)   1
  $node($id) addToChannel  $channel    $hermes_phy($id)   1
  $node($id) addToChannel  $optical_channel    $optical_phy($id)   1

  if {$id > 254} {
  	puts "hostnum > 254!!! exiting"
  	exit
  }
  #Set the IP address of the node
  set ip_value [expr $id + 1]
  $ipif($id) addr $ip_value
  
  # set position($id) [new "Position/UWSM"]
  set position($id) [new "Position/UWSM"]
  $node($id) addPosition $position($id)
  
  #Setup positions
  $position($id) setX_ [expr $id*100]
  $position($id) setY_ [expr $id*0]
  $position($id) setZ_ [expr -13.5 -$id*1.5] 
  
  $application($id) setPosition $position($id)
  
  
  $mac($id) setGuardTime 0.2
  if {$id == 1} {
    $mac($id) setStartTime    2.0
    $mac($id) setSlotDuration 5.0
  } else {
	$mac($id) setStartTime    1.0
    $mac($id) setSlotDuration 1.0      
  }
  
  #Interference model
  set interf_data($id) [new "Module/UW/INTERFERENCE"]
  $interf_data($id) set maxinterval_ $opt(maxinterval_)
  $interf_data($id) set debug_       0
  
  set interf_data2($id) [new "Module/UW/INTERFERENCE"]
  $interf_data2($id) set maxinterval_ $opt(maxinterval_)
  $interf_data2($id) set debug_       0

  set optical_interf_data($id) [new "MInterference/MIV"]
  $optical_interf_data($id) set maxinterval_ $opt(maxinterval_)
  $optical_interf_data($id) set debug_       0
	#Propagation modelpr
  $phy($id) setPropagation $propagation
  $hermes_phy($id) setPropagation $propagation
  $optical_phy($id) setPropagation $optical_propagation

  $phy($id) setSpectralMask $data_mask
  $phy($id) setInterference $interf_data($id)
  # $phy($id) setInterferenceModel "MEANPOWER"

  $hermes_phy($id) setSpectralMask $hermes_data_mask
  $hermes_phy($id) setInterference $interf_data2($id)
  $hermes_phy($id) setInterferenceModel "MEANPOWER"
  $hermes_phy($id) setLUTFileName "../dbs/hermes/default.csv"
  $hermes_phy($id) initLUT

  $optical_phy($id) setSpectralMask $optical_data_mask
  $optical_phy($id) setInterference $optical_interf_data($id)
  $optical_phy($id) setLUTFileName "$opt(LUTpath)"
  $optical_phy($id) setLUTSeparator " "
  $optical_phy($id) useLUT

  $ctr($id) setManualLowerlId [$hermes_phy($id) Id_]
  $ctr($id) setAutomaticSwitch
  if {$id == 0} {
    #$ctr($id) setManualLowerlId [$phy($id) Id_]
    $ctr($id) addLayer [$phy($id) Id_] 1
    $ctr($id) addLayer [$hermes_phy($id) Id_] 2
    $ctr($id) addLayer [$optical_phy($id) Id_] 3

    $ctr($id) addThreshold [$phy($id) Id_] [$hermes_phy($id) Id_] $opt(evo2hermes_thresh)
    $ctr($id) addThreshold [$hermes_phy($id) Id_] [$phy($id) Id_] $opt(hermes2evo_thresh)
    $ctr($id) addThreshold [$hermes_phy($id) Id_] [$optical_phy($id) Id_] $opt(hermes2opt_thresh)
    $ctr($id) addThreshold [$optical_phy($id) Id_] [$hermes_phy($id) Id_] $opt(opt2hermes_thresh)
    #$ctr($id) setManualSwitch
  }
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}


################################
# Inter-node module connection #
################################
$application(0) set destAddr_ [$ipif(1) addr]
$application(0) set destPort_ $portnum(1)
$application(1) set destAddr_ [$ipif(0) addr]
$application(1) set destPort_ $portnum(0)


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

#Print the routing tables of the nodes
#if {$opt(verbose)} {
#	for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
#		$ipr($id1) printroutes
#	}
#}

set applicationCTR $application(0);
set applicationROV $application(1);
set macROV $mac(1);
set macCBR $mac(0);

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
set outfile [open "test_uwrov_results.csv" "w"]
close $outfile
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
foreach line $data {
  if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {
    $ns at $t "update_and_check"
    $ns at $t "$applicationCTR sendPosition $x $y $z"
    }
}
$ns at $opt(starttime)    "$applicationROV start"
$ns at $opt(stoptime)     "$applicationROV stop"
$ns at $opt(starttime)    "$mac(0) start"
$ns at $opt(starttime)    "$mac(1) start"

proc update_and_check {} {
  global position applicationROV
  $position(1) update
  #puts "positions ROV: x = [$applicationROV getX], y = [$applicationROV getY], z =  [$applicationROV getZ]"
  set outfile [open "test_uwrov_results.csv" "a"]
  puts $outfile "positions ROV: x = [$applicationROV getX], y = [$applicationROV getY], z =  [$applicationROV getZ]" 
  close $outfile
}


set partial_tot_rx 0.0
set outTHGfile [open "istTHG.csv" "w"]
close $outTHGfile

proc printInstantThgp { } {
  global macCBR partial_tot_rx opt
  set mac_rov_rcv_pkts   [$macCBR getDataPktsRx]
  set thgp [expr ($mac_rov_rcv_pkts-$partial_tot_rx)*$opt(ROV_pktsize)*8/$opt(time_interval)];#bps
  set partial_tot_rx     $mac_rov_rcv_pkts
  set outTHGfile [open "istTHG.csv" "a"]
  puts " Throughput = $thgp"
}

proc printPos {} {
  global applicationROV
  puts -nonewline " Position: x = [$applicationROV getX], y = [$applicationROV getY], z =  [$applicationROV getZ]"
}

for {set t_thg $opt(time_interval)} {$t_thg <= $opt(stoptime)} {set t_thg [expr $t_thg + $opt(time_interval)]} {
  # $ns at $t "printPos"
  set t_pos [expr $t_thg-$opt(time_interval)/2]
  $ns at $t_pos "puts -nonewline $t_pos; printPos"
  $ns at $t_thg "puts -nonewline ,; printInstantThgp"
}
###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
  global ns opt outfile
  global mac propagation ROV_sink mac_sink phy phy_data_sink channel db_manager propagation
  global node_coordinates
  global ipr_sink ipr ipif udp position applicationROV applicationCTR phy phy_data_sink
  global node_stats tmp_node_stats sink_stats tmp_sink_stats ctr
  if ($opt(verbose)) {
    update_and_check 
    puts "---------------------------------------------------------------------"
    puts "Simulation summary"
    puts "number of nodes  : $opt(nn)"
    puts "ROV packet size      : $opt(ROV_pktsize) byte"
    puts "ROV period       : $opt(ROV_period) s"
    puts "CTR packet size      : $opt(CTR_pktsize) byte"
    puts "number of nodes  : $opt(nn)"
    puts "simulation length: $opt(txduration) s"
    puts "tx power         : $opt(txpower) dB"
    puts "tx frequency     : $opt(freq) Hz"
    puts "tx bandwidth     : $opt(bw) Hz"
    puts "bitrate          : $opt(bitrate) bps"
    if {$opt(ack_mode) == "setNoAckMode"} {
        puts "ACKNOWLEDGEMENT   : disabled"
    } else {
        puts "ACKNOWLEDGEMENT   : active"
    }
    puts "---------------------------------------------------------------------"
  } 
  set ROV_throughput              [$applicationROV getthr]
  set ROV_per                     [$applicationROV getper]
  set ROV_sent_pkts               [$applicationROV getsentpkts]
  set ROV_rcv_pkts                [$applicationROV getrecvpkts]

  set CTR_throughput              [$applicationCTR getthr]
  set CTR_per                     [$applicationCTR getper]
  set CTR_sent_pkts               [$applicationCTR getsentpkts]
  set CTR_rcv_pkts                [$applicationCTR getrecvpkts]
  set mac_ctr_rcv_pkts            [$mac(0) getDataPktsRx] 
  set mac_rov_rcv_pkts            [$mac(1) getDataPktsRx]
  set mac_ctr_tx_pkts            [$mac(0) getDataPktsTx]
  set mac_rov_tx_pkts            [$mac(1) getDataPktsTx]
  if ($opt(verbose)) {
    puts "applicationROV Throughput     : $ROV_throughput"
    puts "applicationROV PER            : $ROV_per       "
    puts "-------------------------------------------"
    puts "applicationCTR Throughput     : $CTR_throughput"
    puts "applicationCTR PER            : $CTR_per       "
    puts "-------------------------------------------"

    if {$opt(ack_mode) == "setAckMode"} {
      set DataPktsTx_CTR                  [$mac(0) getDataPktsTx]
      set UpDataPktsRx_CTR                [$mac(0) getUpLayersDataRx]
      set DataPktsTx_ROV                  [$mac(1) getDataPktsTx]
      set UpDataPktsRx_ROV                [$mac(1) getUpLayersDataRx]
      set rtx_CTR                         [expr (($DataPktsTx_CTR/$ROV_rcv_pkts) - 1)]
      set rtx_ROV                         [expr (($DataPktsTx_ROV/$CTR_rcv_pkts) - 1)]
    }

    set sum_throughput [expr $ROV_throughput + $CTR_throughput]
    set sum_sent_pkts  [expr $ROV_sent_pkts + $CTR_sent_pkts]
    set sum_rcv_pkts   [expr $ROV_rcv_pkts + $CTR_rcv_pkts]
    set sum_mac_rcv_pkts  [expr $mac_ctr_rcv_pkts + $mac_rov_rcv_pkts]
    set sum_mac_tx_pkts  [expr $mac_ctr_tx_pkts + $mac_rov_tx_pkts]
    if {$opt(ack_mode) == "setAckMode"} {
      set sum_rtx      [expr $rtx_ROV + $rtx_CTR]
    }
  }
  set sum_rcv_mac [expr $mac_rov_rcv_pkts + $mac_ctr_rcv_pkts]   
  set sum_sent_mac [expr $mac_rov_tx_pkts + $mac_ctr_tx_pkts]
  set ipheadersize        [$ipif(1) getipheadersize]
  set udpheadersize       [$udp(1) getudpheadersize]
  set ROVheadersize       [$applicationROV getROVMonheadersize]
  set CTRheadersize       [$applicationCTR getROVctrheadersize]
  
  if ($opt(master_signaling_active)) {
    set signalSent        [$ctr(0) getSignalsSent]
    set signalRecv        [$ctr(1) getSignalsRecv]
    set percSignalRecv 0
    if ($signalSent) {
      set percSignalRecv [expr (0.0 + $signalRecv)/$signalSent ]
    }
    set signaling_overhead [expr (0.0 + $signalRecv * $opt(signaling_size)) / ($mac_ctr_rcv_pkts * $opt(ROV_pktsize) + $mac_rov_rcv_pkts * $opt(CTR_pktsize))]
    set signaling_overhead_pkts [expr (0.0 + $signalSent) / ($mac_ctr_rcv_pkts + $mac_rov_rcv_pkts)]
  }

  if ($opt(verbose)) {
    puts "Mean Throughput           : [expr (($sum_throughput+0.0)/(($opt(nn))*($opt(nn)-1)))]"
    puts "Sent Packets CTR --> ROV     : $CTR_sent_pkts"
    puts "Received Packets CTR --> ROV     : $ROV_rcv_pkts"
    puts "Sent Packets  ROV --> CTR   : $ROV_sent_pkts"
    puts "Received Packets ROV --> CTR   : $CTR_rcv_pkts"
    puts "Sent Packets ROV --> CTR mac : $mac_rov_tx_pkts"
    puts "Sent Packets CTR --> ROV mac : $mac_ctr_tx_pkts"
    puts "Received Packets ROV --> CTR mac : $mac_ctr_rcv_pkts"
    puts "Received Packets CTR --> ROV mac : $mac_rov_rcv_pkts"
    puts "---------------------------------------------------------------------"
    puts "Sent Packets        : $sum_sent_pkts"
    puts "Sent Packets  mac   : [expr $mac_rov_tx_pkts + $mac_ctr_tx_pkts]"
    puts "Received            : $sum_rcv_pkts"
    puts "Received mac        : $sum_rcv_mac"
    puts "Packet Delivery Ratio  MAC   : [expr ($sum_rcv_mac+0.0) / $sum_sent_mac * 100]"
    puts "IP Pkt Header Size        : $ipheadersize"
    puts "UDP Header Size           : $udpheadersize"
    puts "ROV Header Size           : $ROVheadersize"
    puts "CTR Header Size           : $CTRheadersize"
    if {$opt(ack_mode) == "setAckMode"} {
      puts "MAC-level average retransmissions per node : [expr ($sum_rtx+0.0)/($opt(nn))]"
    }
    puts "---------------------------------------------------------------------"
    set ROV_packet_lost             [$phy(1) getTotPktsLost]
    set CTR_packet_lost             [$phy(0) getTotPktsLost]
    set packet_lost                 [expr $CTR_packet_lost + $ROV_packet_lost]
    puts "---------------------------------------------------------------------"
    puts "- Overhead -"
    if ($opt(master_signaling_active)) {
      puts "Signaling overhead        : $signaling_overhead"
      puts "Signaling pkts            : $signalSent"
      puts "Signaling overhead (pkts) : $signaling_overhead_pkts"
      puts "Perc. sign. recv.         : $percSignalRecv"
    }
    puts "done!"
  }
    
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


$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 

$ns run

