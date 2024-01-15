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
# Author: Federico Favaro <favarofe@dei.unipd.it>
# Version: 1.0.0


# This script is used to test UW-POLLING protocol
# There are 8 nodes in a square of 4 x 4 nodes with nearest neighbour
# 1 km apart and an AUV that patrols the network retreiving data packets
# making a trajectory described in the Waypoints
# 
# N.B.: This example uses the Waypoint mobility model provided by WOSS.
# For more informations please refer to https://woss.dei.unipd.it
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
load libuwphy_clmsgs.so
load libuwpolling.so
load libuwinterference.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwsmposition.so

set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(start_clock) [clock seconds]
set opt(start_lat)  	 	  44.51 ;# Starting Latitude
set opt(start_long)    		13.5  ;# Starting Longitude
set opt(nn) 			        1.0    ;# Number of nodes
set opt(total_nn)    [expr 2 + $opt(nn)]
set opt(pktsize)	 	      125    ;# Packet size in bytes
set opt(dist_nodes) 		  1000.0    ;# Distance between nodes in m
set opt(nn_in_row) 		    4          ;# Number of nodes in a row
set opt(speed)           0.5;#[expr $opt(knots) * 0.51444444444] ;#Speed of the AUV in m/s
set opt(cbr_period) 1
set rng [new RNG]


set opt(rngstream)    2
set opt(starttime)       	0.1
set opt(stoptime)        	4690  ;# Stoptime
set opt(txduration)     	[expr $opt(stoptime) - $opt(starttime)]

set opt(txpower)	 	150.0 
set opt(per_tgt)	 	0.1
set opt(rx_snr_penalty_db)	0.0
set opt(tx_margin_db)		0.0
set opt(T_backoff) 1

set opt(node_min_angle)		-90.0
set opt(node_max_angle)		90.0
set opt(sink_min_angle)		-90.0
set opt(sink_max_angle) 	90.0
set opt(node_bathy_offset)	-2.0

set opt(maxinterval_)    	100.0
set opt(freq) 			      150000.0
set opt(bw)              	60000.0
set opt(bitrate)	 	      7000.0

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
##

#########################
# Node and WP location  #
#########################

set opt(pos_file)	"../dbs/node_position/circ_position.csv"

set fp [open $opt(pos_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set opt(n_wp) 0
set id_node 0
foreach line $data {
    if {[regexp {^(.*),(.*)$} $line -> x y]} {
		set pos_x($id_node) $x
		set pos_y($id_node) $y
		set id_node [expr $id_node + 1]
    }
}
set opt(nn) [expr $id_node + 0.0];# Number of nodes
#puts "n node $opt(nn)"

if {$opt(bash_parameters)} {
  if {$argc != 2} {
    puts "The script requires two numbers to be inputed. one for rngseed and one for T_backoff period"
    puts "For example, ns test_uwpolling.tcl 1 100"
    puts "If you want to leave the default values, please set to 0"
    puts "the value opt(bash_parameters) in the tcl script"
    puts "Please try again."
  }   else { 
    set opt(rngstream)		[lindex $argv 0]
    set opt(T_backoff)	 	[lindex $argv 1]
  }
} 
###########################
#Random Number Generators #
###########################
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
     $defaultRNG next-substream
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

MPropagation/Underwater set practicalSpreading_ 1.5
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          0
MPropagation/Underwater set shipping_           0
set propagation [new MPropagation/Underwater]


set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

Module/UW/CBR set debug_		    0
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set drop_out_of_order_   0


Module/UW/PHYSICAL  set BitRate_                   $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_   10.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_           $opt(rx_snr_penalty_db)
Module/UW/PHYSICAL  set TxSPLMargin_dB_            $opt(tx_margin_db)
Module/UW/PHYSICAL  set MaxTxSPL_dB_               156
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


Module/UW/POLLING/AUV set max_payload_        $opt(pktsize)
Module/UW/POLLING/AUV set T_min_              0
Module/UW/POLLING/AUV set T_max_              $opt(T_backoff) ;#MAX backoff time
Module/UW/POLLING/AUV set T_probe_guard_      2 ;#related to T_max + RTT
Module/UW/POLLING/AUV set T_guard_            1
Module/UW/POLLING/AUV set max_polled_node_    2000
Module/UW/POLLING/AUV set max_buffer_size_	 	50
Module/UW/POLLING/AUV set max_tx_pkts_ 			20
Module/UW/POLLING/AUV set n_run                $opt(rngstream);#used for c++ rng
Module/UW/POLLING/AUV set debug_			0

Module/UW/POLLING/NODE set T_poll_guard_	  5 ;#has to be bigger than T_probe(AUV)
Module/UW/POLLING/NODE set backoff_tuner_     1
Module/UW/POLLING/NODE set max_payload_       $opt(pktsize)
Module/UW/POLLING/NODE set buffer_data_pkts_  50
Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_  5
Module/UW/POLLING/NODE set intra_data_guard_time_ 0.001
Module/UW/POLLING/NODE set useAdaptiveTpoll_   1
Module/UW/POLLING/NODE set n_run                $opt(rngstream);#used for c++ rng
Module/UW/POLLING/NODE set debug_			0

Module/UW/POLLING/SINK set T_data_guard_	5 ;#has to be bigger than T_probe(AUV)
Module/UW/POLLING/SINK set debug_			0
Module/UW/POLLING/SINK set sink_id_				253
Module/UW/POLLING/SINK set n_run                $opt(rngstream);#used for c++ rng
Module/UW/POLLING/SINK set useAdaptiveTdata_    1

################################
# Procedure(s) to create nodes #
################################

proc createNode { id } {
    
    global channel propagation data_mask ns cbr position node udp portnum ipr ipif channel_estimator
    global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global row pos_x pos_y
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	Module/UW/PHYSICAL  set debug_ 0
    
    set cbr($id)  [new Module/UW/CBR] 
    set udp($id) [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/POLLING/NODE]
    set phy_data($id)  [new Module/UW/PHYSICAL]

    $node($id)  addModule 7 $cbr($id)   1  "CBR"
    $node($id)  addModule 6 $udp($id)  1  "PRT"
    $node($id)  addModule 5 $ipr($id)   1  "IPR"
    $node($id)  addModule 4 $ipif($id)  1  "IPF"   
    $node($id) addModule  3 $mll($id)   1  "MLL"
    $node($id)  addModule 2 $mac($id)   1  "MAC"
    $node($id)  addModule 1 $phy_data($id)   1  "PHY"

    $node($id) setConnection $cbr($id)   $udp($id)  0
    $node($id) setConnection $udp($id)  $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy_data($id)   1
    $node($id) addToChannel  $channel    $phy_data($id)   1


    set portnum($id) [$udp($id) assignPort $cbr($id) ]
    

    #$ipif($id) addr "1.0.0.${id}"
    $ipif($id) addr 1
    

    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    
    #Setup positions
    $position($id) setX_ $pos_x($id)
    $position($id) setY_ $pos_y($id)
    $position($id) setZ_ -100

    
    
    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0


    
    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
    $phy_data($id) set debug_ 0
	$phy_data($id) setInterferenceModel "MEANPOWER"

    $mac($id) set node_id_ [expr $id + 1]
    $mac($id) initialize

}

proc createAUV { } {
    global channel propagation smask data_mask ns cbr_auv position_auv node_auv udp_auv portnum_auv 
    global phy_data_auv posdb_auv opt mll_auv mac_auv ipr_auv ipif_auv bpsk interf_data_auv channel_estimator
    global woss_utilities woss_creator start_lat_auv start_lon_auv propagation_auv

    set node_auv [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
	Module/UW/PHYSICAL  set AcquisitionThreshold_dB_   10.0 
	Module/UW/PHYSICAL  set debug_ 0

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set cbr_auv($cnt)  [new Module/UW/CBR] 
    }
    set udp_auv      [new Module/UW/UDP]
    set ipr_auv       [new Module/UW/StaticRouting]
    set ipif_auv      [new Module/UW/IP]
    set mll_auv       [new Module/UW/MLL] 
    set mac_auv       [new Module/UW/POLLING/AUV]
    set phy_data_auv  [new Module/UW/PHYSICAL]

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
       $node_auv addModule 7 $cbr_auv($cnt) 0 "CBR"
    }
    $node_auv addModule 6 $udp_auv      0 "PRT"
    $node_auv addModule 5 $ipr_auv       0 "IPR"
    $node_auv addModule 4 $ipif_auv      0 "IPF"   
    $node_auv addModule 3 $mll_auv       0 "MLL"
    $node_auv addModule 2 $mac_auv       1 "MAC"
    $node_auv addModule 1 $phy_data_auv  1 "PHY"

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       $node_auv setConnection $cbr_auv($cnt)  $udp_auv     1
    }
    $node_auv setConnection $udp_auv $ipr_auv      	0
    $node_auv setConnection $ipr_auv  $ipif_auv       	0
    $node_auv setConnection $ipif_auv $mll_auv        	0 
    $node_auv setConnection $mll_auv  $mac_auv        	0
    $node_auv setConnection $mac_auv  $phy_data_auv   	1
    $node_auv addToChannel $channel    $phy_data_auv   	1

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_auv($cnt) [$udp_auv assignPort $cbr_auv($cnt)]
    }

    set position_auv [new "Position/UWSM"]
	$position_auv set debug_ 0
    $node_auv addPosition $position_auv
    
    #Setup positions
    $position_auv setX_ 1350
    $position_auv setY_ 850
    $position_auv setZ_ -100
    $ipif_auv addr 253      

    set interf_data_auv [new Module/UW/INTERFERENCE]
    $interf_data_auv set maxinterval_ $opt(maxinterval_)
    $interf_data_auv set debug_       0

    $phy_data_auv setSpectralMask     $data_mask
    $phy_data_auv setPropagation      $propagation
    $phy_data_auv setInterference     $interf_data_auv
	$phy_data_auv setInterferenceModel "MEANPOWER"

    $mac_auv initialize

}


#proc createSINK { } {
#    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
#    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator
#    global woss_utilities woss_creator propagation_sink lat_sink lon_sink
#
#    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
#
#	Module/UW/PHYSICAL  set debug_ 0
#
#    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
#       set cbr_sink($cnt)  [new Module/UW/CBR] 
#    }
#    set udp_sink      [new Module/UW/UDP]
#    set ipr_sink       [new Module/UW/StaticRouting]
#    set ipif_sink      [new Module/UW/IP]
#    set mll_sink       [new Module/UW/MLL] 
#    set mac_sink       [new Module/UW/POLLING/SINK]
#    set phy_data_sink  [new Module/UW/PHYSICAL]
#
#	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
#       $node_sink addModule 7 $cbr_sin170k($cnt) 0 "CBR"
#    }170
#	$node_sink addModule 6 $udp_sink  170    0 "PRT"
#	$node_sink addModule 5 $ipr_sink  170     0 "IPR"
#	$node_sink addModule 4 $ipif_sink 170     0 "IPF"   
#	$node_sink addModule 3 $mll_sink       0 "MLL"
#	$node_sink addModule 2 $mac_sink       1 "MAC"
#	$node_sink addModule 1 $phy_data_sink  1 "PHY"
#
#	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
#       $node_sink setConnection $cbr_sink($cnt)  $udp_sink     1
#    }
#    $node_sink setConnection $udp_sink $ipr_sink      	0
#    $node_sink setConnection $ipr_sink  $ipif_sink       	0
#    $node_sink setConnection $ipif_sink $mll_sink        	0 
#    $node_sink setConnection $mll_sink  $mac_sink        	0
#    $node_sink setConnection $mac_sink  $phy_data_sink   	1
#    $node_sink addToChannel $channel    $phy_data_sink   	1
#
#	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
#       set portnum_sink($cnt) [$udp_sink assignPort $cbr_sink($cnt)]
#    }
#	 
#	set lat_sink 53.541850
#    set lon_sink 9.940937
#    set curr_depth 10
#
#    set position_sink [new "WOSS/Position"]
#    $node_sink addPosition $position_sink
#    set posdb_sink [new "PlugIn/PositionDB"]
#    $node_sink addPlugin $posdb_sink 20 "PDB"
#    $posdb_sink addpos [$ipif_sink addr] $position_sink
#
#    $position_sink setLatitude_  $lat_sink
#    $position_sink setLongitude_ $lon_sink
#    $position_sink setAltitude_  [expr -1.0 * $curr_depth]  
#
#    puts "node sink at ([$position_sink getLatitude_], [$position_sink getLongitude_], [$position_sink getAltitude_]) , ([$position_sink getX_], [$position_sink getY_], [$position_sink getZ_])"
#
#    $ipif_sink addr 252      
#
#    set interf_data_sink [new Module/UW/INTERFERENCE]
#    $interf_data_sink set maxinterval_ $opt(maxinterval_)
#    $interf_data_sink set debug_       0
#
#    $phy_data_sink setSpectralMask     $data_mask
#    $phy_data_sink setPropagation      $propagation_sink
#    $phy_data_sink setInterference     $interf_data_sink
#
#    $mac_sink initialize
#}



###############################
# routing of nodes
###############################

proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_auv portnum_auv ipr_sink
	global ipif_sink portnum_sink ipr_auv

    $cbr($id1) set destAddr_ [$ipif_auv addr]
    $cbr($id1) set destPort_ $portnum_auv($id1)
    $ipr($id1) addRoute [$ipif_auv addr] [$ipif_auv addr]
}

###############################
# create nodes, sink, auv
###############################

for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	createNode $id1
}
#createSINK
createAUV

################################
#Setup flows
################################
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	connectNodes $id1
}

################################
#fill ARP tables
################################

for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
		$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]	
	}
	$mll($id1) addentry [$ipif_auv addr] [$mac_auv addr]
	$mll_auv addentry [$ipif($id1) addr] [$mac($id1) addr]
}
$mll_auv addentry [$ipif_auv addr] [$mac_auv addr]


################################
#Start cbr(s)
################################




$ns at [expr $opt(starttime) + 5] "$mac_auv run"

set opt(waypoint_file) "../dbs/wp_path/polling_backoff_wp.csv"
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set dist 195.1
set opt(t_wp) [expr $dist / $opt(speed)];#97.55;#1951;#390.2;#130.1;#195.1;#48.78;#
puts "t wp $opt(t_wp)"
set t 5
set opt(n_run) 4
for {set id1 0} {$id1 < $opt(n_run)} {incr id1}  {
	foreach line $data {
    if {[regexp {^(.*),(.*)$} $line -> x y]} {
			set t [expr $t + $opt(t_wp)]
        	$ns at $t "$position_auv setdest $x $y -100 $opt(speed)"
	    }
	}
}

set opt(stoptime) [expr $t + 1]
puts "stop $opt(stoptime)"

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)			"$cbr($id1) start"
    $ns at $opt(stoptime)   		"$cbr($id1) stop"
}

proc finish { } {
    global ns opt cbr outfile
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
	global cbr_auv mac_auv phy_data_auv udp_sink ipif_sink
    #if {$opt(verbose)} {
    #	puts "CBR_PERIOD : $opt(cbr_period)"
    #	puts "SEED: $opt(rngstream)"
    #	puts "NUMBER OF NODES: $opt(nn)"
    #} else {
    #    puts "End of simulation"
    #}
    set sum_cbr_throughput 	0
    set sum_per		0
    set sum_cbr_sent_pkts	0.0
    set sum_cbr_rcv_pkts	0.0
    set sum_mac_sent_pkts     0.0
	set sum_mac_probe_sent 0.0
    set cbr_rcv_pkts		0.0
    set mac_pkts		0.0
	
    for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		
		set cbr_pkts($id3)         	   [$cbr($id3) getsentpkts]
		set mac_tx_pkts($id3)         	   [$mac($id3) getDataPktsTx]
		set mac_probe_sent($id3)		[$mac($id3) getProbeSent]
		set mac_trigger_rx($id3)		[$mac($id3) getTriggerReceived]
		set mac_trigger_dropped($id3)	[$mac($id3) getTriggerDropped]
		set mac_poll_rx($id3)			[$mac($id3) getTimesPolled]
		set mac_poll_dropped($id3)		[$mac($id3) getPollDropped]
		set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_pkts($id3)]
		set sum_mac_sent_pkts [expr $sum_mac_sent_pkts + $mac_tx_pkts($id3) ]
		set sum_mac_probe_sent [expr $sum_mac_probe_sent + $mac_probe_sent($id3)]
    }

	set cbr_sink_rcv 0.0
	for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		set cbr_sink_rcv_pkts($id3)       [$cbr_auv($id3) getrecvpkts]
		set cbr_sink_thr($id3)			[$cbr_auv($id3) getthr]
		set cbr_per	           [$cbr_auv($id3) getper]
		set cbr_sink_rcv [expr $cbr_sink_rcv + $cbr_sink_rcv_pkts($id3)]
	}

    set mac_auv_rcv_pkts   [$mac_auv getDataPktsRx] 
	set mac_auv_sent_pkts   [$mac_auv getDataPktsTx] 
	set mac_auv_probe_rx	[$mac_auv getProbeReceived]
	set mac_auv_dropped_probe	[$mac_auv getDroppedProbePkts]
	set mac_auv_trigger_sent	[$mac_auv getTriggerSent]
	set mac_auv_poll_sent [$mac_auv getPollSent]
	set mac_auv_pkts_buff_overflow [$mac_auv getDiscardedPktsTx]
	for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		set mac_auv_rx_per_node($id3) [$mac_auv getRxFromNode [$mac($id3) addr]]
	}

	
	if {$opt(verbose)} {
		puts "Backoff time : $opt(T_backoff)"
		puts "-------------------------------------------------------------------------------------------"
		puts "TRIGGER packets"
		puts "mac auv sent trigger pkts : $mac_auv_trigger_sent"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) received trigger pkts : $mac_trigger_rx($id3)"
			puts "mac($id3) dropped trigger pkts : $mac_trigger_dropped($id3)"
		}
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "PROBE packets"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) sent probe pkts	: $mac_probe_sent($id3)"
		}
		puts "mac overall sent probe pkts : $sum_mac_probe_sent"
		puts "mac auv dropped probe	pkts : $mac_auv_dropped_probe"
		puts "mac auv received probe pkts : $mac_auv_probe_rx"
		puts "avg probe per round : [expr $mac_auv_probe_rx*1.0/$mac_auv_trigger_sent ]"
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "POLL packets"
		puts "mac auv sent poll pkts : $mac_auv_poll_sent"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) received poll pkts : $mac_poll_rx($id3)"
			puts "mac($id3) dropped poll pkts : $mac_poll_dropped($id3)"
		}
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "MAC DATA packets"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) sent pkts : $mac_tx_pkts($id3) "
			puts "mac auv received from node $id3 : $mac_auv_rx_per_node($id3)"
		}
		puts "mac nodes overall sent pkts : $sum_mac_sent_pkts"
		puts "mac auv received packets : $mac_auv_rcv_pkts"
		puts "mac auv transmitted packets : $mac_auv_sent_pkts"
		if {$sum_mac_sent_pkts == 0} {
			puts "mac pdr : 0"
		} else {
			puts "mac pdr : [expr $mac_auv_rcv_pkts*1.0/$sum_mac_sent_pkts]"
		}
		puts "mac auv discarded packets buffer overflow : $mac_auv_pkts_buff_overflow"
		
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "APP DATA packets"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "cbr($id3) sent pkts : $cbr_pkts($id3)"
		}
		puts "cbr nodes overall sent pkts : $sum_cbr_sent_pkts"
		puts "cbr auv received pkts : $cbr_sink_rcv"
		puts "overall throughput : [expr $cbr_sink_rcv*$opt(pktsize) / $opt(txduration)] \[Byte/s\]"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "node($id3)-sink throughput : $cbr_sink_thr($id3) \[Byte/s\]"
		}
		puts "-------------------------------------------------------------------------------------------"
	}

    set tot_time  	     [$mac_auv GetTotalReceivingTime]
    
    #if {$opt(verbose)} {  
    #	puts "---------------------------------------------------------------------"
    #	puts "Number of packets transmitted by CBR: [expr ($sum_cbr_sent_pkts)]"
    #	puts "Number of packets received by CBR: [expr ($sum_cbr_rcv_pkts)]"
    #	puts "------------------------------------------------------------------"
    #	puts "Number of packets received by MAC:	$mac_auv_rcv_pkts"
    #	puts "Number of packets transmitted by MAC:    $sum_mac_sent_pkts"
    #	puts "------------------------------------------------------------------"
    #	puts "Mean PER at CBR layer: [expr (1 - ($sum_cbr_rcv_pkts/($sum_cbr_sent_pkts)))]"
    #	puts "Mean PER at MAC layer: [expr (1- ($sum_cbr_rcv_pkts/($sum_mac_sent_pkts)))]"
#
    #	puts "Total receiving time: $tot_time"
    #	puts "Mean throughput at MAC layer: [expr (($mac_auv_rcv_pkts*125*8)/($tot_time))]"
    #	puts "Mean throughput at CBR layer: $sum_cbr_throughput"
    #	puts "done!"
    #}
	set opt(end_clock) [clock seconds]

    puts  "done in [expr $opt(end_clock) - $opt(start_clock)] seconds!"
	puts ""
	puts ""
    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################

puts -nonewline "\nSimulating...\n"

$ns at [expr $opt(stoptime) + 150.0]  "$mac_auv stop_count_time"
$ns at [expr $opt(stoptime) + 150.0]  "finish; $ns halt" 

$ns run
    
