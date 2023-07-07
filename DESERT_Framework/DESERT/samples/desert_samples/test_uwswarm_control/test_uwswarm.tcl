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
# Author: Filippo Campagnaro, Vincenzo Cimino
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
set opt(bash_parameters) 	0
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
set opt(ROV_pktsize)    1000;#125  ;# Pkt size in byte
set opt(CTR_pktsize)    1024;#125  ;# Pkt size in byte

set opt(ROV_period) 	10

set opt(starttime)      1	
set opt(stoptime)       10000
set opt(txduration)     [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation

set opt(txpower)		185.08;#Power transmitted in dB re uPa

set opt(propagation_speed) 1500;# m/s

set opt(maxinterval_)     20.0
set opt(freq)             25000.0 ;#Frequency used in Hz
set opt(bw)               5000.0 ;#Bandwidth used in Hz
set opt(bitrate)		  4800.3 ;#150000;#bitrate in bps
set opt(ack_mode)         "setNoAckMode"

set opt(pktsize)  32
set opt(cbr_period)   10
set opt(poisson_traffic) 0

set opt(rngstream)	155

if {$opt(ACK_Active)} {
    set opt(ack_mode)           "setAckMode"    
} else {
    set opt(ack_mode)           "setNoAckMode"
}

set opt(waypoint_file)  "./rov_path.csv"

if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires two inputs:"
		puts "- the first for the number of nodes"
        puts "- the second for the rngstream"
        puts "example: ns test_uw_rov.tcl 3 13"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        return
    } else {
        set opt(nn)			[lindex $argv 0]
        set opt(rngstream)	[lindex $argv 1];
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

Module/UW/ROV/CTR set packetSize_			$opt(CTR_pktsize)
Module/UW/ROV/CTR set debug_				0

Module/UW/SC/CTR set debug_					0

Plugin/UW/SC/MC set debug_ 0

### Channel ###
MPropagation/Underwater set practicalSpreading_	2
MPropagation/Underwater set debug_				0
MPropagation/Underwater set windspeed_			10
MPropagation/Underwater set shipping_			1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq				$opt(freq)
$data_mask setBandwidth			$opt(bw)
$data_mask setPropagationSpeed  $opt(propagation_speed)

### MAC ###
Module/UW/CSMA_ALOHA set listen_time_	1

### PHY ###
Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    4.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 3000
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

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
  			    set x1 [expr $x + 25*$id1/2]
  			    set y1 [expr $y - 25*$id1/2]
				$ns at $t "$app_ctr($leader_id,$id1) sendPosition $x1 $y1 $z $s"
  			} else {
  			    set x1 [expr $x - 25*($id1+1)/2]
  			    set y1 [expr $y - 25*($id1+1)/2]
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
set mine_count 0
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
    global ns opt position mine_count
	global leader_id app_rov app_ctr app_mc mine_position 

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
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
  global ns opt
  global app_ctr app_trl app_rov app_trf leader_id
  global mac propagation channel propagation phy

  if ($opt(verbose)) {
    puts "-----------------------------------------------------------------"
    puts "Simulation summary"
    puts "-----------------------------------------------------------------"
    puts "Simulation summary"
    puts "number of nodes  : $opt(nn)"
    puts "packet size      : $opt(pktsize) byte"
    puts "cbr period       : $opt(cbr_period) s"
    puts "number of nodes  : $opt(nn)"
    puts "simulation length: $opt(txduration) s"
    puts "tx frequency     : $opt(freq) Hz"
    puts "tx bandwidth     : $opt(bw) Hz"
    puts "bitrate          : $opt(bitrate) bps"
    puts "-----------------------------------------------------------------"
  }

  set sum_throughput     0
  set sum_sent_pkts      0.0
  set sum_rcv_pkts       0.0    

  for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
    set ctr_throughput($id1)              [$app_ctr($leader_id,$id1) getthr]
    set ctr_per($id1)                     [$app_ctr($leader_id,$id1) getper]
    set ctr_sent_pkts($id1)               [$app_ctr($leader_id,$id1) getsentpkts]
    set ctr_rcv_pkts($id1)                [$app_ctr($leader_id,$id1) getrecvpkts]
    
    set trl_throughput($id1)              [$app_trl($leader_id,$id1) getthr]
    set trl_per($id1)                     [$app_trl($leader_id,$id1) getper]
    set trl_sent_pkts($id1)               [$app_trl($leader_id,$id1) getsentpkts]
    set trl_rcv_pkts($id1)                [$app_trl($leader_id,$id1) getrecvpkts]

    set rov_throughput($id1)              [$app_rov($id1) getthr]
    set rov_per($id1)                     [$app_rov($id1) getper]
    set rov_sent_pkts($id1)               [$app_rov($id1) getsentpkts]
    set rov_rcv_pkts($id1)                [$app_rov($id1) getrecvpkts]

    set trf_throughput($id1)              [$app_trf($id1,$leader_id) getthr]
    set trf_per($id1)                     [$app_trf($id1,$leader_id) getper]
    set trf_sent_pkts($id1)               [$app_trf($id1,$leader_id) getsentpkts]
    set trf_rcv_pkts($id1)                [$app_trf($id1,$leader_id) getrecvpkts]

	set sum_throughput [expr $sum_throughput + $ctr_throughput($id1) + $trl_throughput($id1) + $rov_throughput($id1) + $trf_throughput($id1)]
	set sum_sent_pkts [expr $sum_sent_pkts + $ctr_sent_pkts($id1) + $trl_sent_pkts($id1) + $rov_sent_pkts($id1) + $trf_sent_pkts($id1)]
	set sum_rcv_pkts [expr $sum_rcv_pkts + $ctr_rcv_pkts($id1) + $trl_rcv_pkts($id1) + $rov_rcv_pkts($id1) + $trf_rcv_pkts($id1)]

	if ($opt(verbose)) {
  	  puts "CTR ($id1) Throughput     : $ctr_throughput($id1)"
  	  puts "TRL ($id1) Throughput     : $trl_throughput($id1)"
  	  puts "ROV ($id1) Throughput     : $rov_throughput($id1)"
  	  puts "TRF ($id1) Throughput     : $trf_throughput($id1)"
  	  puts "-------------------------------------------"
  	}
  }

  if ($opt(verbose)) {
    puts "Mean Throughput           : [expr ($sum_throughput/(($opt(nn))))]"
    puts "---------------------------------------------------------------------"
    puts "Sent Packets	: $sum_sent_pkts"
    puts "Received	: $sum_rcv_pkts"
    puts "Packet Delivery Ratio	: [expr $sum_rcv_pkts / $sum_sent_pkts * 100]"
      puts "---------------------------------------------------------------------"

    set NL_packet_lost             [$phy($leader_id) getTotPktsLost]
    set packet_lost                $NL_packet_lost
    for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
		set NF_packet_lost($id1)       [$phy($id1) getTotPktsLost]
		set packet_lost                [expr $packet_lost + $NF_packet_lost($id1)]
    }

    for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
      puts "- PHY layer statistics for the node Follower ($id1) -"
      puts "Tot. pkts lost            : $NF_packet_lost($id1)"
      puts "Tot. CTRL pkts lost due to Interference   : [$phy($id1) getErrorCtrlPktsInterf]"
      puts "---------------------------------------------------------------------"
	}

    puts "- PHY layer statistics for the node Leader -"
    puts "Tot. pkts lost            : $NL_packet_lost"
    puts "Tot. CTRL pkts lost due to Interference   : [$phy($leader_id) getErrorCtrlPktsInterf]"
    puts "---------------------------------------------------------------------"
    puts "- Global situation -"
    puts "Tot. pkts lost            : $packet_lost"
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