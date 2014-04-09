#
# Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
# Author: Federico Favaro <favarofe@dei.unipd.it>
# Version: 1.0.0


# This script is used to test UW-POLLING protocol
# There are 8 nodes in a square of 4 x 4 nodes with nearest neighbour
# 1 km apart and an AUV that patrols the network retreiving data packets
# making a trajectory described in the Waypoints
# 
# N.B.: This example uses the Waypoint mobility model provided by WOSS.
# For more informations please refer to http://telecom.dei.unipd.it/ns/woss/
#
# NOTE: tcl sample tested on Ubuntu 11.10, 64 bits OS
#
#
#
# Stack of the nodes                               Stack of the AUV
#	+-----------------------+                       +-----------------------+
#	|    7.  UW/CBR  (tx)    |                      |      7. UW/CBR(rx)     |
#	+-----------------------+	                      +-----------------------+
#	|       6. UW/UDP        |                      |       6. UW/UDP        |
#	+-----------------------+	                      +-----------------------+
#	|  5. UW/staticROUTING   |                      |  5. UW/staticROUTING   |
#	+-----------------------+	                      +-----------------------+
#	|       4. UW/IP         |                      |       4. UW/IP         |
#	+-----------------------+	                      +-----------------------+
#	|       3. UW/MLL        |                      |       3. UWMLL        |
#	+-----------------------*                       +-----------------------+
#	|2.   UW/POLLING_NODE    |                      | 2.  UW/POLLING_AUV     |
# +.......................+                       +.......................+
#	: 1 Module/UW/PHYSICAL   :                      : 1. Module/UW/PHYSICAL  :
#	+.......................+	                      +.......................+
#           |                                               |
#	      +-------------------------------------------------------------+
#       |                       UnderwaterChannel                    |
#       +-------------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose)          1
set opt(trace_files)      0
set opt(bash_parameters)  0



#####################
# Library Loading   #
#####################
load libMiracle.so
load libmphy.so
load libmmac.so
load libMiracleBasicMovement.so
load libUwmStd.so
load libWOSSPhy.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwpolling.so
load libuwinterference.so
load libuwphysical.so

set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################

set opt(start_clock) [clock seconds]


set opt(start_lat)  	 	  44.51 ;# Starting Latitude
set opt(start_long)    		13.5  ;# Starting Longitude
set opt(nn) 			        8.0    ;# Number of nodes
set opt(pktsize)	 	      125    ;# Packet size in bytes
set opt(stoptime)        	126000  ;# Stoptime
set opt(dist_nodes) 		  1000.0    ;# Distance between nodes in m
set opt(nn_in_row) 		    4          ;# Number of nodes in a row
set opt(knots)        		4      ;# Speed of the AUV in knots
set opt(speed)            [expr $opt(knots) * 0.51444444444] ;#Speed of the AUV in m/s

set rng [new RNG]

if {$opt(bash_parameters)} {
  if {$argc != 2} {
    puts "The aloha.tcl script requires two numbers to be inputed. one for seed and one for cbr period"
    puts "For example, ns test_uwpolling.tcl 1 100"
    puts "If you want to leave the default values, please set to 0"
    puts "the value opt(bash_parameters) in the tcl script"
    puts "Please try again."
  }   else { 
    $rng seed 			[lindex $argv 0]
    set opt(rep_num)	 	[lindex $argv 0]
    set opt(cbr_period)   [lindex $argv 1]
  }
} else {
  set opt(rep_num)    1
  $rng seed [lindex $argv 0]
  set opt(cbr_period) 100
}

set opt(cbrpr) [expr int($opt(cbr_period))]
set opt(rnpr)  [expr int($opt(rep_num))]
set opt(starttime)       	0.1
set opt(txduration)     	[expr $opt(stoptime) - $opt(starttime)]

set opt(txpower)	 	150.0 
set opt(per_tgt)	 	0.1
set opt(rx_snr_penalty_db)	0.0
set opt(tx_margin_db)		0.0

set opt(node_min_angle)		-90.0
set opt(node_max_angle)		90.0
set opt(sink_min_angle)		-90.0
set opt(sink_max_angle) 	90.0
set opt(node_bathy_offset)	-2.0

set opt(maxinterval_)    	10.0
set opt(freq) 			      25000.0
set opt(bw)              	5000.0
set opt(bitrate)	 	      4800.0



### TRACE FILE
if {$opt(trace_files)} {
  set opt(tracefilename) "./test_uwpolling.tr"
  set opt(tracefile) [open $opt(tracefilename) w]
  set opt(cltracefilename) "./test_uwpolling.cltr"
  set opt(cltracefile) [open $opt(cltracefilename) w]
} else {
  set opt(tracefilename) "/dev/null/"
  set opt(tracefile) [open $opt(tracefilename) w]
  set opt(cltracefilename) "/dev/null/"
  set opt(cltracefile) [open $opt(cltracefilename) w]
}
###



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


WOSS/Utilities set debug 0
set woss_utilities [new WOSS/Utilities]

set woss_utilities [new "WOSS/Utilities"]
WOSS/Manager/Simple set debug 0
WOSS/Manager/Simple set space_sampling 0.0
set woss_manager [new "WOSS/Manager/Simple"]


set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]

set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

Module/UW/CBR set debug_		    0
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1


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

WOSS/Position/WayPoint set time_threshold_            [expr 1.0 / $opt(speed)]
WOSS/Position/WayPoint set compDistance_              0.0
WOSS/Position/WayPoint set verticalOrientation_       0.0
WOSS/Position/WayPoint set minVerticalOrientation_    -40.0
WOSS/Position/WayPoint set maxVerticalOrientation_    40.0



Module/UW/POLLING/NODE set T_poll_            40
Module/UW/POLLING/NODE set backoff_tuner_     1
Module/UW/POLLING/NODE set max_payload_       125
Module/UW/POLLING/NODE set buffer_data_pkts_  50
Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_  50

Module/UW/POLLING/AUV set max_payload_        125
Module/UW/POLLING/AUV set T_probe_            10
Module/UW/POLLING/AUV set T_min_              0.5
Module/UW/POLLING/AUV set T_max_              5
Module/UW/POLLING/AUV set T_guard_            5
Module/UW/POLLING/AUV set max_polled_node_    20


################################
# Procedure(s) to create nodes #
################################

proc createNode { id } {
    
    global channel propagation data_mask ns cbr position node port portnum ipr ipif channel_estimator
    global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global row
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    
    
    set cbr($id)  [new Module/UW/CBR] 
    set port($id) [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/POLLING/NODE]
    set phy_data($id)  [new Module/UW/PHYSICAL]

    $node($id)  addModule 7 $cbr($id)   1  "CBR"
    $node($id)  addModule 6 $port($id)  1  "PRT"
    $node($id)  addModule 5 $ipr($id)   1  "IPR"
    $node($id)  addModule 4 $ipif($id)  1  "IPF"   
    $node($id) addModule  3 $mll($id)   1  "MLL"
    $node($id)  addModule 2 $mac($id)   1  "MAC"
    $node($id)  addModule 1 $phy_data($id)   1  "PHY"

    $node($id) setConnection $cbr($id)   $port($id)  0
    $node($id) setConnection $port($id)  $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy_data($id)   1
    $node($id) addToChannel  $channel    $phy_data($id)   1


    set portnum($id) [$port($id) assignPort $cbr($id) ]
    if {$id > 254} {
	     puts "hostnum > 254!!! exiting"
	     exit
    }

    #$ipif($id) addr "1.0.0.${id}"
    $ipif($id) addr [expr $id + 1]
    

    set position($id) [new "WOSS/Position/WayPoint"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)
     ##########################
     #  POSIZIONAMENTO NODI   #
     ##########################
    if { $id < 4 } {
    	set curr_x [expr $opt(dist_nodes) * $id ]
    	set row 0
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } else {
    	set curr_x  [expr ($id -($opt(nn)/2 )) * $opt(dist_nodes) ]
    	set row 1
    	set curr_y  [expr $row * $opt(dist_nodes) ]
        }

    set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
    set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $curr_x ]
    set curr_depth 100
    puts "$curr_x $curr_y $curr_depth"

    
    $position($id) setLatitude_  $curr_lat
    $position($id) setLongitude_ $curr_lon
    $position($id) setAltitude_  [expr -1.0 * $curr_depth]


    puts "node $id at ([$position($id) getLatitude_], [$position($id) getLongitude_], [$position($id) getAltitude_]) , ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"


    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0


    
    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
    $phy_data($id) set debug_ 0

    $mac($id) set node_id_ $id
    $mac($id) initialize

}

proc createSink { } {
    global channel propagation smask data_mask ns cbr_sink position_sink node_sink port_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator
    global woss_utilities woss_creator

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
	       set cbr_sink($cnt)  [new Module/UW/CBR] 
    }
    
    set port_sink      [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/POLLING/AUV]
    set phy_data_sink  [new Module/UW/PHYSICAL]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
	     $node_sink addModule 7 $cbr_sink($cnt) 0 "CBR"
      }
     $node_sink addModule 6 $port_sink      0 "PRT"
     $node_sink addModule 5 $ipr_sink       0 "IPR"
     $node_sink addModule 4 $ipif_sink      0 "IPF"   
     $node_sink addModule 3 $mll_sink       0 "MLL"
     $node_sink addModule 2 $mac_sink       1 "MAC"
     $node_sink addModule 1 $phy_data_sink  1 "PHY"

     for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
     $node_sink setConnection $cbr_sink($cnt)  $port_sink     	1   
      }
     $node_sink setConnection $port_sink $ipr_sink      	0
     $node_sink setConnection $ipr_sink  $ipif_sink       	0
     $node_sink setConnection $ipif_sink $mll_sink        	0 
     $node_sink setConnection $mll_sink  $mac_sink        	0
     $node_sink setConnection $mac_sink  $phy_data_sink   	1
     $node_sink addToChannel $channel    $phy_data_sink   	1

     for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_sink($cnt) [$port_sink assignPort $cbr_sink($cnt)]
       if {$cnt > 252} {
        puts "hostnum > 252!!! exiting"
        exit
       }
     }

     set curr_depth 10
     
     set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 0 ]
     set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  0 ]


     set position_sink [new "WOSS/Position/WayPoint"]
     $position_sink addStartWayPoint $curr_lat $curr_lon [expr -1.0 * $curr_depth] $opt(speed) 0.0
     $node_sink addPosition $position_sink

    
     $ipif_sink addr 253
    

    
      
     puts "node sink at ([$position_sink getLatitude_], [$position_sink getLongitude_], [$position_sink getAltitude_]) , ([$position_sink getX_], [$position_sink getY_], [$position_sink getZ_])"

     set interf_data_sink [new Module/UW/INTERFERENCE]
     $interf_data_sink set maxinterval_ $opt(maxinterval_)
     $interf_data_sink set debug_       0

     $phy_data_sink setSpectralMask     $data_mask
     $phy_data_sink setPropagation      $propagation
     $phy_data_sink setInterference     $interf_data_sink

     $mac_sink initialize

}

##################
# Sink WayPoints #
##################

proc createSinkWaypoints { } {
  global position_sink opt position woss_utilities

  set toa 0.0
  set curr_lat [$position(1) getLatitude_]
  set curr_lon [$position(1) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
  puts "waypoint 1  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"


  set curr_lat [$position(2) getLatitude_]
  set curr_lon [$position(2) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
  puts "waypoint 2  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"


  set curr_lat [$position(3) getLatitude_]
  set curr_lon [$position(3) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
  puts "waypoint 3  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"


  set curr_lat [$position(4) getLatitude_]
  set curr_lon [$position(4) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
  puts "waypoint 4  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"


  set curr_lat [$position(5) getLatitude_]
  set curr_lon [$position(5) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
  puts "waypoint 5  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"

  set curr_lat [$position(6) getLatitude_]
  set curr_lon [$position(6) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
  puts "waypoint 5  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"


  set curr_lat [$position(7) getLatitude_]
  set curr_lon [$position(7) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
  puts "waypoint 6  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"

  set curr_lat [$position(0) getLatitude_]
  set curr_lon [$position(0) getLongitude_]
  set curr_depth [expr -1.0 * 5]
  set toa      [$position_sink addLoopPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0 1 20]
  puts "waypoint 0  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"
  #1 TOTAL LOOPS
  #0.0 loop_id


}

###############################
# routing of nodes
###############################

proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    #$cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    #$cbr_sink($id1) set destPort_ $portnum($id1)  
    #$ipr($id1) addRoute "1.0.0.253"    "255.255.255.255" "1.0.0.253"
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

createSinkWaypoints
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
$ns at 20 "$mac_sink run"

proc finish { } {
      global ns opt cbr outfile
      global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
      if {$opt(verbose)} {
        puts "CBR_PERIOD : $opt(cbr_period)"
        puts "SEED: $opt(rep_num)"
        puts "NUMBER OF NODES: $opt(nn)"
      } else {
        puts "End of simulation"
      }


      set sum_cbr_throughput 	0
      set sum_per		0
      set sum_cbr_sent_pkts	0.0
      set sum_cbr_rcv_pkts	0.0
      set sum_mac_sent_pkts     0.0
      set cbr_rcv_pkts		0.0
      set mac_pkts		0.0
	
      for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		set cbr_throughput	   [$cbr_sink($id3) getthr]
		set cbr_per	           [$cbr_sink($id3) getper]
		set cbr_pkts         	   [$cbr($id3) getsentpkts]
		set mac_pkts         	   [$mac($id3) getDataPktsTx]
		set cbr_rcv_pkts       [$cbr_sink($id3) getrecvpkts]
    if {$opt(verbose)} {
		  puts "cbr_sink($id3) throughput                : $cbr_throughput"
		  puts "cbr_sink($id3) received packets          : $cbr_rcv_pkts"
		  puts "mac($id3) sent pkts                      : $mac_pkts"
		  puts "MAC level PDR of node		       : [expr {double($cbr_rcv_pkts)/double($mac_pkts)}]"
		  puts "-------------------------------------------------------------------------------------------"
    }

		set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
		set sum_per [expr $sum_per + $cbr_per]
		set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_pkts]
		set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
		set sum_mac_sent_pkts [expr $sum_mac_sent_pkts + $mac_pkts]
      }	
    set mac_auv_rcv_pkts   [$mac_sink getDataPktsRx] 
    set tot_time  	     [$mac_sink GetTotalReceivingTime]
    
    if {$opt(verbose)} {  
      puts "---------------------------------------------------------------------"
      puts "Number of packets transmitted by CBR: [expr ($sum_cbr_sent_pkts)]"
      puts "Number of packets received by CBR: [expr ($sum_cbr_rcv_pkts)]"
      puts "------------------------------------------------------------------"
      puts "Number of packets received by MAC:	$mac_auv_rcv_pkts"
      puts "Number of packets transmitted by MAC:    $sum_mac_sent_pkts"
      puts "------------------------------------------------------------------"
      puts "Mean PER at CBR layer: [expr (1 - ($sum_cbr_rcv_pkts/($sum_cbr_sent_pkts)))]"
      puts "Mean PER at MAC layer: [expr (1- ($sum_cbr_rcv_pkts/($sum_mac_sent_pkts)))]"

      puts "Total receiving time: $tot_time"
      puts "Mean throughput at MAC layer: [expr (($mac_auv_rcv_pkts*125*8)/($tot_time))]"
      puts "Mean throughput at CBR layer: $sum_cbr_throughput"
      puts "done!"
    }
    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################

puts -nonewline "\nSimulating...\n"

$ns at [expr $opt(stoptime) + 345.0]  "$mac_sink stop_count_time"
$ns at [expr $opt(stoptime) + 350.0]  "finish; $ns halt" 

$ns run
    
