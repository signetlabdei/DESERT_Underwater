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
###########################################################################
# This script is used to test uwswarm_controller. 
# There is a swarm of three AUV, composed of one leader and two followers.
# Each of them has a precomputed route to follow.
# The two followers has a tracking system in order to find mines.
# Everytime a follower find a mine the auv notifies it to the leader
# which in turns sends the auv to the mines position.
# The way-point list is imported by an external file. 
###########################################################################
# Author: Vincenzo Cimino
# Version: 1.0.0
#
#
# Stack of the AUVs leader and followers
#					 AUV Leader							  AUV follower 1			      AUV follower 2
#   +--------------+-----------+---------------+   +------------+------------+   +------------+------------+
#   |  7. UW/SC/MC | UW/SC/CTR | UW/SC/TRACKER |   |  7. UW/ROV | UW/TRACKER |   | 7. UW/ROV  | UW/TRACKER |
#   +--------------+-----------+---------------+   +------------+------------+   +-------------+-----------+
#   |  6. UW/UDP							   |   |  6. UW/UDP              |   |  6. UW/UDP              |
#   +------------------------------------------+   +-------------------------+   +-------------------------+
#   |  5. UW/STATICROUTING					   |   |  5. UW/STATICROUTING    |   |  5. UW/STATICROUTING    |
#   +------------------------------------------+   +-------------------------+   +-------------------------+
#   |  4. UW/IP								   |   |  4. UW/IP               |   |  4. UW/IP               |
#   +------------------------------------------+   +-------------------------+   +-------------------------+
#   |  3. UW/MLL							   |   |  3. UW/MLL              |   |  3. UW/MLL              |
#   +------------------------------------------+   +-------------------------+   +-------------------------+
#   |  2. UW/CSMA_ALOHA						   |   |  2. UW/CSMA_ALOHA       |   |  2. UW/CSMA_ALOHA       |
#   +------------------------------------------+   +-------------------------+   +-------------------------+
#   |  1. UW/PHYSICAL   					   |   |  1. UW/PHYSICAL         |   |  1. UW/PHYSICAL         |
#   +------------------------------------------+   +-------------------------+   +-------------------------+
#            |						  |                    |          |                   |			|       
#   +------------------------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel												   |
#   +------------------------------------------------------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 		1
set opt(trace_files)		0
set opt(bash_parameters) 	1
set opt(ACK_Active)         0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libuwip.so
load libuwstaticrouting.so
load libmphy.so
load libmmac.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwrov.so
load libuwsmposition.so
load libuwtracker.so
load libuwswarm_control.so
load libuwinterference.so
load libUwmStd.so
load libuwstats_utilities.so
load libuwmmac_clmsgs.so
load libuwcsmaaloha.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwphysical.so
load libuwahoi_phy.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(nn)				3 ;# Number of Nodes
set opt(ROV_pktsize)    32;#125  ;# Pkt size in byte
set opt(CTR_pktsize)    32;#125  ;# Pkt size in byte

set opt(ROV_period) 	60
set opt(CTR_period) 	60

set opt(starttime)      1	
set opt(stoptime)       10000
set opt(txduration)     [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation

set opt(txpower)		156;#Power transmitted in dB re uPa

set opt(maxinterval_)     200.0
set opt(freq)             65200.0 ;#Frequency used in Hz
set opt(bw)               25000.0 ;#Bandwidth used in Hz
set opt(bitrate)		  200 ;#bitrate in bps
set opt(ack_mode)         "setNoAckMode"

set opt(pktsize)  32
set opt(cbr_period)   60
set opt(poisson_traffic) 0

set opt(rngstream)	155

if {$opt(ACK_Active)} {
    set opt(ack_mode)           "setAckMode"    
} else {
    set opt(ack_mode)           "setNoAckMode"
}

set opt(waypoint_file)  "./sin_path.csv"

if {$opt(bash_parameters)} {
    if {$argc != 3} {
        puts "The script requires two inputs:"
		puts "- the first for the number of nodes"
        puts "- the second for the rngstream"
        puts "- the third for the path"
        puts "example: ns test_uwswarm.tcl 3 13 rov_path.csv"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        return
    } else {
        set opt(nn)			[lindex $argv 0]
        set opt(rngstream)	[lindex $argv 1];
		set opt(waypoint_file)  [lindex $argv 2]
    }   
} 

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwrovmovement.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwrovmovement.cltr"
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
### APP ###
Module/UW/CBR set packetSize_			$opt(pktsize)
Module/UW/CBR set PoissonTraffic_		$opt(poisson_traffic)
Module/UW/CBR set period_				$opt(cbr_period)
Module/UW/CBR set debug_				0
Module/UW/CBR set tracefile_enabler_	1

Module/UW/TRACKER set debug_					0

Module/UW/SC/TRACKERF set max_tracking_distance_	50
Module/UW/SC/TRACKERF set packetSize_				$opt(pktsize)
Module/UW/SC/TRACKERF set PoissonTraffic_			$opt(poisson_traffic)
Module/UW/SC/TRACKERF set period_					$opt(cbr_period)
Module/UW/SC/TRACKERF set debug_					0
Module/UW/SC/TRACKERF set tracefile_enabler_		1
Module/UW/SC/TRACKERF set send_only_active_trace_	1

Module/UW/SC/TRACKER set debug_					0
Module/UW/SC/TRACKER set tracefile_enabler_		1

Module/UW/ROV set packetSize_          $opt(ROV_pktsize)
Module/UW/ROV set period_              $opt(ROV_period)
Module/UW/ROV set debug_               0

Module/UW/SC/CTR set packetSize_      $opt(CTR_pktsize)
Module/UW/SC/CTR set period_          $opt(CTR_period)
Module/UW/SC/CTR set debug_			  0

Plugin/UW/SC/MC set debug_ 0

### Channel ###
MPropagation/Underwater set practicalSpreading_	2
MPropagation/Underwater set debug_				0
MPropagation/Underwater set windspeed_			1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq				$opt(freq)
$data_mask setBandwidth			$opt(bw)

### MAC ###
Module/UW/CSMA_ALOHA set listen_time_	1

### PHY ###
#Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
#Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0
#Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
#Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
#Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
#Module/UW/PHYSICAL  set MinTxSPL_dB_                10
#Module/UW/PHYSICAL  set MaxTxRange_                 200
#Module/UW/PHYSICAL  set PER_target_                 0
#Module/UW/PHYSICAL  set CentralFreqOptimization_    0
#Module/UW/PHYSICAL  set BandwidthOptimization_      0
#Module/UW/PHYSICAL  set SPLOptimization_            0
#Module/UW/PHYSICAL  set debug_                      0

Module/UW/AHOI/PHY  set BitRate_                    $opt(bitrate)
Module/UW/AHOI/PHY  set AcquisitionThreshold_dB_    5.0 
Module/UW/AHOI/PHY  set RxSnrPenalty_dB_            0
Module/UW/AHOI/PHY  set TxSPLMargin_dB_             0
Module/UW/AHOI/PHY  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/AHOI/PHY  set MinTxSPL_dB_                10
Module/UW/AHOI/PHY  set MaxTxRange_                 200
Module/UW/AHOI/PHY  set PER_target_                 0    
Module/UW/AHOI/PHY  set CentralFreqOptimization_    0
Module/UW/AHOI/PHY  set BandwidthOptimization_      0
Module/UW/AHOI/PHY  set SPLOptimization_            0
Module/UW/AHOI/PHY  set debug_                      0

Position/UWSM set debug_ 0

#################
# Node Creation #
#################
source "node_leader.tcl"
source "node_follower.tcl"

set leader_id 0

createNodeL $leader_id
for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
	createNodeF $id1
}

################################
# Inter-node module connection #
################################
proc connectNodes { id1 } {
  global ipif ipr opt leader_id app_ctr app_trl app_rov app_trf
  global portnum_ctr portnum_trl portnum_rov portnum_trf

  $app_ctr($leader_id,$id1) set destAddr_ [$ipif($id1) addr]
  $app_ctr($leader_id,$id1) set destPort_ $portnum_rov($id1)
  
  $app_rov($id1) set destAddr_ [$ipif($leader_id) addr]
  $app_rov($id1) set destPort_ $portnum_ctr($leader_id,$id1)

  $app_trl($leader_id,$id1) set destAddr_ [$ipif($id1) addr]
  $app_trl($leader_id,$id1) set destPort_ $portnum_trf($id1)

  $app_trf($id1,$leader_id) set destAddr_ [$ipif($leader_id) addr]
  $app_trf($id1,$leader_id) set destPort_ $portnum_trl($leader_id,$id1)
}

##################
# Setup flows  #
##################
for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
  connectNodes $id1
}

###################
# Fill ARP tables #
###################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
  for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
	   $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
  }
}

########################
# Setup routing tables #
########################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
  for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
    if {$id1 != $id2} {
      $ipr($id1) addRoute [$ipif($id2) addr] [$ipif($id2) addr]
    }
  }
}

#####################
# Start/Stop Timers #
#####################
set outfile_auv [open "test_auv_results.csv" "w"]
set outfile_mine [open "test_mine_results.csv" "w"]
close $outfile_auv
close $outfile_mine
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
foreach line $data {
	if {[regexp {^(.*),(.*),(.*),(.*),(.*)$} $line -> t x y z s]} {
		$ns at $t "update_and_check $t $leader_id"
		$ns at $t "$position($leader_id) setdest $x $y $z $s"
		for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
			$ns at $t "update_and_check $t $id1"
			if { [expr $id1 % 2] == 0 } {
  			    set x1 [expr $x - 12.5*$id1/2]
  			    set y1 [expr $y + 12.5*$id1/2]
				$ns at $t "$app_ctr($leader_id,$id1) sendPosition $x1 $y1 $z $s"
  			} else {
  			    set x1 [expr $x - 12.5*($id1+1)/2]
  			    set y1 [expr $y - 12.5*($id1+1)/2]
				$ns at $t "$app_ctr($leader_id,$id1) sendPosition $x1 $y1 $z $s"
  			}
		}
	}
}

# Place and remove mines
set opt(mine_file)   "mine_position.csv"
set fp [open $opt(mine_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set removed_mine 0
set mine_count 0
set time_demine 0
foreach line $data {
	if {[regexp {^(.*),(.*),(.*)$} $line -> x y z]} {
		set mine_position($mine_count) [new "Position/UWSM"]
		$mine_position($mine_count) setX_ $x"
		$mine_position($mine_count) setY_ $y
		$mine_position($mine_count) setZ_ $z

		incr mine_count
    }
}

for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
	for {set cnt 0} {$cnt < $mine_count} {incr cnt}  {
#		$app_trl($leader_id,$id1) setLogSuffix "[expr $leader_id],[expr $id1]"
		$app_trf($id1,$leader_id) setTrack $mine_position($cnt)
	}

  $ns at $opt(starttime)  "$app_rov($id1) start"
  $ns at $opt(stoptime)   "$app_rov($id1) stop"

  $ns at $opt(starttime)  "$app_trf($id1,$leader_id) start"
  $ns at $opt(stoptime)   "$app_trf($id1,$leader_id) stop"
}

proc update_and_check { t id } {
    global ns opt position mine_count time_demine removed_mine
	global leader_id app_rov app_ctr mine_position app_mc

	$position($id) update
	# Auvs path output
    set outfile_auv [open "test_auv_results.csv" "a"]
	puts $outfile_auv "$t,$id,[$position($id) getX_],[$position($id) getY_],[$position($id) getZ_]"
    close $outfile_auv

	# Mines detected output
	if {$id > 0} {
		for {set cnt 0} {$cnt < $mine_count} {incr cnt}  {
		if {[expr abs([$position($id) getX_] - [$mine_position($cnt) getX_])] < 0.1 &&
				[expr abs([$position($id) getY_] - [$mine_position($cnt) getY_])] < 0.1 &&
				[expr abs([$position($id) getZ_] - [$mine_position($cnt) getZ_])] < 0.1} {
				set outfile_mine [open "test_mine_results.csv" "a"]
				puts $outfile_mine "$t,$id,[$mine_position($cnt) getX_],[$mine_position($cnt) getY_],[$mine_position($cnt) getZ_]"
				close $outfile_mine
			}
		}
	}

	set temp_mine [$app_mc($leader_id) getremovedmines]
	if {$temp_mine != $removed_mine} {
		set removed_mine [$app_mc($leader_id) getremovedmines]
		set time_demine $t
	}
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
  global ns opt mine_count time_demine removed_mine
  global app_ctr app_trl app_rov app_trf app_mc leader_id
  global phy

  if ($opt(verbose)) {
    puts "-----------------------------------------------------------------"
    puts "Simulation summary"
    puts "-----------------------------------------------------------------"
    puts "number of nodes  : $opt(nn)"
    puts "packet size      : $opt(pktsize) byte"
    puts "cbr period       : $opt(cbr_period) s"
    puts "simulation length: $opt(txduration) s"
    puts "tx frequency     : $opt(freq) Hz"
    puts "tx bandwidth     : $opt(bw) Hz"
    puts "bitrate          : $opt(bitrate) bps"
    puts "-----------------------------------------------------------------"
  }
  if ($opt(verbose)) {
	# 1kWh for 24 h at 1m/s (Remus 100):
	# P = 1 kWh / 22 h = 0.04167 kW = 41.67 W
	# AHOI: P_tx = 5 W; P_rx = 0.3 W
	# Ping 360 sonar: P = 5 W
    puts "no. mines		: $mine_count"
	puts "no. mines removed	: $removed_mine"
	puts "Removed mine ratio	: [expr double($removed_mine) / $mine_count]"
	puts "demine time		: [expr double($time_demine)/60]"
    puts "-----------------------------------------------------------------"

	set packet_duration [expr double($opt(pktsize)*8)/double($opt(bitrate))]
    for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
	  set rov_sent [expr 0.0 + [$app_rov($id1) getsentpkts]]
	  set rov_recv [expr 0.0 + [$app_rov($id1) getrecvpkts]]

	  set trf_sent [expr 0.0 + [$app_trf($id1,$leader_id) getsentpkts]]
	  set trf_recv [expr 0.0 + [$app_trf($id1,$leader_id) getrecvpkts]]

	  set trl_sent [expr 0.0 + [$app_trl($leader_id,$id1) getsentpkts]]
	  set trl_recv [expr 0.0 + [$app_trl($leader_id,$id1) getrecvpkts]]

	  set ctr_sent [expr 0.0 + [$app_ctr($leader_id,$id1) getsentpkts]]
	  set ctr_recv [expr 0.0 + [$app_ctr($leader_id,$id1) getrecvpkts]]

	  set sum_sent [expr $rov_sent + $ctr_sent + $trf_sent + $trl_sent]
	  set sum_recv [expr $rov_recv + $ctr_recv + $trf_recv + $trl_recv]

	  set energy_tx($id1) [expr $sum_sent*5*$packet_duration]
	  set energy_rx($id1) [expr $sum_recv*0.3*$packet_duration]

	  set per($id1) [expr (1 - $sum_recv/$sum_sent) * 100]

	  puts "sent packets $id1		: $sum_sent"
	  puts "received packets $id1 	: $sum_recv"
	  puts "node $id1 PER		: $per($id1)"
	  puts "---------------------------------------------------------------------"
	}
	puts "energy consumption	: [expr (41.67 + 5)*$time_demine + $energy_tx(1) + $energy_rx(1)]"
    puts "---------------------------------------------------------------------"
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
}


$ns at [expr $opt(stoptime) + 50.0]  "finish; $ns halt" 

$ns run
