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
set opt(bash_parameters)  1



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
load libuwphysical.so
load libuwsmposition.so

set ns [new Simulator] 
$ns use-Miracle

##################
# Tcl variables  #
##################

set opt(start_clock) [clock seconds]


set opt(nn) 			        200.0    ;# Number of nodes
# set opt(n_sink)					16.0; #Number of sinks 

set opt(pktsize)	 	      220    ;# Packet size in bytes
set opt(stoptime)        	46500;#7400;#2800  ;# Stoptime
set opt(speed)            2;#[expr $opt(knots) * 0.51444444444] ;#Speed of the AUV in m/s
set opt(cbr_period) 	5;#60;#850-->3 packets each AUV round
#set opt(n_laps)			10
set rng [new RNG]
#set opt(pos_file)	"dbs/wp_path/207_nodes_wp.txt"

if {$opt(bash_parameters)} {
  if {$argc != 4} {
    puts "The aloha.tcl script requires two numbers to be inputed. one for seed and one for cbr period"
    puts "For example, ns test_uwpolling.tcl 1 100 n_sink link_wp"
    puts "If you want to leave the default values, please set to 0"
    puts "the value opt(bash_parameters) in the tcl script"
    puts "Please try again."
  }   else { 
    #$rng seed 			[lindex $argv 0]
    set opt(rep_num)	 	[lindex $argv 0]
	set opt(cbr_period) 	[lindex $argv 1]
    set opt(n_sink)         [lindex $argv 2]
    set opt(pos_file)       [lindex $argv 3]
  }
} else {
  set opt(rep_num)    2
  #$rng seed [lindex $argv 0]
}
set opt(total_nn)    [expr 1 + $opt(nn) + $opt(n_sink)]

set opt(T_backoff)        10
set opt(starttime)       	0.1

set opt(txpower)	 	156.0 
set opt(per_tgt)	 	0.1
set opt(rx_snr_penalty_db)	0.0
set opt(tx_margin_db)		0.0

set opt(node_min_angle)		-90.0
set opt(node_max_angle)		90.0
set opt(sink_min_angle)		-90.0
set opt(sink_max_angle) 	90.0
set opt(node_bathy_offset)	-2.0

set opt(maxinterval_)    	100.0
set opt(freq) 			      150000.0
set opt(bw)              	60000.0
set opt(bitrate)	 	      7000

### TRACE FILE
if {$opt(trace_files)} {
  set opt(tracefilename) "./test_project.tr"
  set opt(tracefile) [open $opt(tracefilename) w]
  set opt(cltracefilename) "./test_project.cltr"
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
# Node and WP location  #
#########################



set fp [open $opt(pos_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set opt(n_wp) 0
set t 5
set curr_x 0
set curr_y 0
set dist 0
foreach line $data {
    if {[regexp {^(.*),(.*),(.*)$} $line -> node_name x y]} {
		if {[regexp {NODE([0-9]*)} $node_name -> id]} {
			set x_node($id) $x
			set y_node($id) $y
            #puts "Node $id located at $x m $y m"
		}
		if {[regexp {SINK([0-9]*)} $node_name -> id]} { 
			set x_sink($id) $x
			set y_sink($id) $y
            #puts "SINK node $id located at $x m $y m"
		}
		if {[regexp {AUV} $node_name -> id]} {
			set start_x_auv $x
			set start_y_auv $y
            #puts "AUV starting at $x m $y m"

            set curr_x $x
            set curr_y $y
		}

    }
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

MPropagation/Underwater set practicalSpreading_ 2
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          0
MPropagation/Underwater set shipping_           1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]

set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

Module/UW/CBR set debug_		    0
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set drop_out_of_order_   0
Module/UW/CBR set tracefile_enabler_   1



Module/UW/PHYSICAL  set BitRate_                   $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_   10.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_           0
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
Module/UW/POLLING/AUV set T_max_              $opt(T_backoff);#MAX backoff time
Module/UW/POLLING/AUV set T_probe_            [expr $opt(T_backoff) + 2];#related to T_max + RTT
Module/UW/POLLING/AUV set T_guard_            1
Module/UW/POLLING/AUV set max_polled_node_    200
Module/UW/POLLING/AUV set max_buffer_size_	 	350000
Module/UW/POLLING/AUV set max_tx_pkts_ 			14000
Module/UW/POLLING/AUV set n_run                $opt(rep_num);#used for c++ rng
Module/UW/POLLING/AUV set debug_			0
Module/UW/POLLING/AUV set ack_enabled_		0

Module/UW/POLLING/NODE set T_poll_            [expr $opt(T_backoff) + 5] ;#has to be bigger than T_probe(AUV)
Module/UW/POLLING/NODE set backoff_tuner_     1
Module/UW/POLLING/NODE set max_payload_       $opt(pktsize)
Module/UW/POLLING/NODE set buffer_data_pkts_  3500
Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_  20
Module/UW/POLLING/NODE set intra_data_guard_time_ 0.001
Module/UW/POLLING/NODE set useAdaptiveTpoll_   1
Module/UW/POLLING/NODE set n_run                $opt(rep_num);#used for c++ rng
Module/UW/POLLING/NODE set debug_			0


Module/UW/POLLING/SINK set T_data_			[expr $opt(T_backoff) + 5] ;#has to be bigger than T_probe(AUV)
Module/UW/POLLING/SINK set debug_			0
Module/UW/POLLING/SINK set sink_id_				240
Module/UW/POLLING/SINK set n_run                $opt(rep_num);#used for c++ rng
Module/UW/POLLING/SINK set useAdaptiveTdata_    1
Module/UW/POLLING/SINK set ack_enabled_		0



################################
# Procedure(s) to create nodes #
################################

proc createNode { id } {
    
    global channel propagation data_mask ns cbr position node udp portnum ipr ipif channel_estimator
    global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global row x_node y_node
    
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
    if {$id > 240} {
	     puts "hostnum > 240!!! exiting"
	     exit
    }

    #$ipif($id) addr "1.0.0.${id}"
    $ipif($id) addr [expr $id + 1]

    set curr_depth 10

    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    
    #Setup positions
    $position($id) setX_ $x_node($id)
    $position($id) setY_ $y_node($id)
    $position($id) setZ_ [expr -1.0 * $curr_depth]
     

   # puts "node $id at ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"

    
    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0


    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
	$phy_data($id) setInterferenceModel "MEANPOWER"
    $phy_data($id) set debug_ 0

    $mac($id) set node_id_ [expr $id + 1]
    $mac($id) setMacAddr [expr $id + 1]   
    $mac($id) initialize

}

proc createAUV { } {
    global channel propagation smask data_mask ns cbr_auv position_auv node_auv udp_auv portnum_auv 
    global phy_data_auv posdb_auv opt mll_auv mac_auv ipr_auv ipif_auv bpsk interf_data_auv channel_estimator
    global woss_utilities woss_creator start_x_auv start_y_auv

    set node_auv [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

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

    #set start_lat_auv 53.542802 
	#set start_lon_auv 9.943063
	set curr_depth 10
	
    set position_auv [new "Position/UWSM"]
	$position_auv set debug_ 0
    $node_auv addPosition $position_auv
    
    #Setup positions
    $position_auv setX_ $start_x_auv
    $position_auv setY_ $start_y_auv
    $position_auv setZ_ [expr -1.0 * $curr_depth]

    #puts "node auv at ([$position_auv getX_], [$position_auv getY_], [$position_auv getZ_])"

    $ipif_auv addr 253 
    $mac_auv setMacAddr 253    

    set interf_data_auv [new Module/UW/INTERFERENCE]
    $interf_data_auv set maxinterval_ $opt(maxinterval_)
    $interf_data_auv set debug_       0

    $phy_data_auv setSpectralMask     $data_mask
    $phy_data_auv setPropagation      $propagation
    $phy_data_auv setInterference     $interf_data_auv
	$phy_data_auv setInterferenceModel "MEANPOWER"

    $mac_auv set node_id_ 253
    $mac_auv initialize

}


#???
proc createSINK { id } {
    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator
    global woss_utilities woss_creator propagation_sink x_sink y_sink propagation

    set node_sink($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	Module/UW/PHYSICAL  set debug_ 0
    Module/UW/CBR set debug_		    0
    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set cbr_sink($id,$cnt)  [new Module/UW/CBR] 
    }
    

    set udp_sink($id)      [new Module/UW/UDP]
    set ipr_sink($id)       [new Module/UW/StaticRouting]
    set ipif_sink($id)      [new Module/UW/IP]
    set mll_sink($id)      [new Module/UW/MLL] 
    set mac_sink($id)       [new Module/UW/POLLING/SINK]
    set phy_data_sink($id)  [new Module/UW/PHYSICAL]

	
    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
       $node_sink($id) addModule 7 $cbr_sink($id,$cnt) 0 "CBR"
    }
    
	$node_sink($id) addModule 6 $udp_sink($id)      0 "PRT"
	$node_sink($id) addModule 5 $ipr_sink($id)      0 "IPR"
	$node_sink($id) addModule 4 $ipif_sink($id)      0 "IPF"   
	$node_sink($id) addModule 3 $mll_sink($id)       0 "MLL"
	$node_sink($id) addModule 2 $mac_sink($id)      1 "MAC"
	$node_sink($id) addModule 1 $phy_data_sink($id)  1 "PHY"

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       $node_sink($id) setConnection $cbr_sink($id,$cnt)  $udp_sink($id)     1
    }
    
    $node_sink($id) setConnection $udp_sink($id) $ipr_sink($id)      	0
    $node_sink($id) setConnection $ipr_sink($id)  $ipif_sink($id)      	0
    $node_sink($id) setConnection $ipif_sink($id) $mll_sink($id)       	0 
    $node_sink($id) setConnection $mll_sink($id)  $mac_sink($id)        	0
    $node_sink($id) setConnection $mac_sink($id)  $phy_data_sink($id)   	1
    $node_sink($id) addToChannel $channel    $phy_data_sink($id)   	1
	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_sink($cnt) [$udp_sink($id) assignPort $cbr_sink($id,$cnt)]
    }

    set curr_depth 10

    set position($id) [new "Position/BM"]
    $node_sink($id) addPosition $position($id)
    
    #Setup positions
    $position($id) setX_ $x_sink($id)
    $position($id) setY_ $y_sink($id)
    $position($id) setZ_ [expr -1.0 * $curr_depth]



    #$ipif_sink($id) addr [expr $id + 240]  
    $ipif_sink($id) addr 240
    $mac_sink($id) setMacAddr 240

   # puts "node sink $id with ip 240 at ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"  

    set interf_data_sink($id) [new Module/UW/INTERFERENCE]
    $interf_data_sink($id) set maxinterval_ $opt(maxinterval_)
    $interf_data_sink($id) set debug_       0

    $phy_data_sink($id) setSpectralMask     $data_mask
    $phy_data_sink($id) setPropagation      $propagation
    $phy_data_sink($id) setInterference     $interf_data_sink($id)
	$phy_data_sink($id) setInterferenceModel "MEANPOWER"


    $mac_sink($id) set node_id_ $id
    $mac_sink($id) initialize
    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
        $cbr_sink($id,$cnt) setLogSuffix [expr $id + 1]
    }

}

###############################
# routing of nodes???
###############################

proc connectNodes {id1 ids} {
    global ipif ipr portnum cbr cbr_sink ipif_auv portnum_auv ipr_sink
	global ipif_sink portnum_sink ipr_auv

    $cbr($id1) set destAddr_ [$ipif_sink($ids) addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    #$cbr($id1) set destPort_ $portnum_sink($ids)
    $ipr($id1) addRoute [$ipif_sink($ids) addr] [$ipif_auv addr]
	$ipr_auv  addRoute [$ipif_sink($ids) addr] [$ipif_sink($ids) addr]
    $ipr_sink($ids)  addRoute [$ipif($id1) addr] [$ipif($id1) addr]
}

###############################
# create nodes, sink, auv
###############################

for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	createNode $id1
}
for {set ids 0} {$ids < $opt(n_sink)} {incr ids} {
	createSINK $ids
	#puts "created SINK $ids"
}

createAUV

################################
#Setup flows
################################
# for {set ids 0} {$ids < $opt(n_sink)} {incr ids} {
    for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
        connectNodes $id1 0
    }
#}
################################
#fill ARP tables???
################################

for {set ids 0} {$ids < $opt(n_sink)} {incr ids} {
	for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
		for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
			$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]	
		}
		$mll($id1) addentry [$ipif_auv addr] [$mac_auv addr]
		$mll($id1) addentry [$ipif_sink($ids) addr] [$mac_sink($ids) addr]
		$mll_sink($ids) addentry [$ipif($id1) addr] [$mac($id1) addr]
		$mll_auv addentry [$ipif($id1) addr] [$mac($id1) addr]
	}
	$mll_sink($ids) addentry [$ipif_sink($ids) addr] [$mac_sink($ids) addr]
	$mll_sink($ids) addentry [$ipif_auv addr] [$mac_auv addr]
	$mll_auv addentry [$ipif_sink($ids) addr] [$mac_sink($ids) addr]
	$mll_auv addentry [$ipif_auv addr] [$mac_auv addr]
}





################################
#Start cbr(s) and wp???
################################




$ns at [expr $opt(starttime) + 5] "$mac_auv run"

set fp [open $opt(pos_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set opt(n_wp) 0
set t 5
set dist 0
foreach line $data {
    if {[regexp {^(.*),(.*),(.*)$} $line -> node_name x y]} {
		if {[regexp {WP([0-9][0-9]*)} $node_name -> id]} {

            #euclidean distance
            set nx [expr {$curr_x - $x}]
            set ny [expr {$curr_y - $y}]
            set nx2 [expr {$nx * $nx}]
            set ny2 [expr {$ny * $ny}]
            set totxy [expr {$nx2 + $ny2}]
            set dist [expr {sqrt($totxy)} ]
            puts "Distance: $dist"

            set opt(t_wp) [expr $dist / $opt(speed)];#97.55;#1951;#390.2;#130.1;#195.1;#48.78;#
            puts "t wp $opt(t_wp)"

            set t [expr $t + $opt(t_wp)]
            set curr_x $x
            set curr_y $y
            $ns at $t "$position_auv setdest $x $y -10 $opt(speed)"
            #waits in position
            #set t [expr $t + 1000]
            
            puts "AUV will go to $x $y"
			set opt(n_wp) [expr $id + 1]
		}

    }
}


set opt(stoptime) [expr $t + 1000]
puts "stop $opt(stoptime)"

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)			"$cbr($id1) start"
    $ns at $opt(stoptime)   		"$cbr($id1) stop"
}
set opt(txduration)     	[expr $opt(stoptime) - $opt(starttime)]

proc finish { } {
    global ns opt cbr outfile
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
	global cbr_auv mac_auv phy_data_auv udp_sink ipif_sink
    if {$opt(verbose)} {
    	puts "CBR_PERIOD : $opt(cbr_period)"
    	puts "SEED: $opt(rep_num)"
    	puts "NUMBER OF SENSOR NODES: $opt(nn)"
    	puts "OVERALL NUMBER OF NODES: $opt(total_nn)"
		puts "START TIME: $opt(starttime)"
		puts "STOP TIME: $opt(stoptime)"

    } else {
        puts "End of simulation"
    }
	set cbr_header	[$cbr_sink(0,0) getcbrheadersize]
	set udp_header	[$udp_sink(0) getudpheadersize]
	set ipif_header	[$ipif_sink(0) getipheadersize]
	if {$opt(verbose)} {
		puts "HEADERS SIZE"
		puts "cbr header	: $cbr_header \[bytes\]"
		puts "udp header	: $udp_header \[bytes\]"
		puts "ip header	: $ipif_header \[bytes\]"
	}
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
    for {set ids 0} {$ids < $opt(n_sink)} {incr ids}  {
        for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
            set cbr_sink_rcv_pkts($ids,$id3)       [$cbr_sink($ids,$id3) getrecvpkts]
            set cbr_throughput	   [$cbr_sink($ids,$id3) getthr]
            set cbr_per	           [$cbr_sink($ids,$id3) getper]
            set cbr_sink_rcv [expr $cbr_sink_rcv + $cbr_sink_rcv_pkts($ids,$id3)]
        }
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
	
    for {set ids 0} {$ids < $opt(n_sink)} {incr ids}  {
        set mac_sink_rcv_pkts  [$mac_sink($ids) getDataPktsRx]
        set mac_sink_duplicated	[$mac_sink($ids) getDuplicatedPkts]
        set mac_sink_probe_sent [$mac_sink($ids) getProbeSent]
        set mac_sink_trigger_rx		[$mac_sink($ids) getTriggerReceived]
        set mac_sink_trigger_dropped	[$mac_sink($ids) getTriggerDropped]
        set sum_mac_probe_sent [expr $sum_mac_probe_sent + $mac_sink_probe_sent]
    }

    for {set ids 0} {$ids < $opt(n_sink)} {incr ids}  {
        for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
            set cbr_sink_delay($ids,$id3)       [$cbr_sink($ids,$id3) getftt]
            set cbr_sink_delay_std($ids,$id3)	   [$cbr_sink($ids,$id3) getfttstd]
            set cbr_sink_thr($ids,$id3)			[$cbr_sink($ids,$id3) getthr]
        }
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
		puts "mac_sink received trigger pkts : $mac_sink_trigger_rx"
		puts "mac_sink dropped trigger pkts : $mac_sink_trigger_dropped"
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "PROBE packets"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) sent probe pkts	: $mac_probe_sent($id3)"
		}
		puts "mac sink sent probe pkts : $mac_sink_probe_sent"
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
		puts "mac sink received packets : $mac_sink_rcv_pkts"
		puts "mac sink duplicated packets : $mac_sink_duplicated"

		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "APP DATA packets"
        for {set ids 0} {$ids < $opt(n_sink)} {incr ids}  {
            for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
                puts "cbr($id3) sent pkts : $cbr_pkts($id3)"
                puts "cbr($id3) sink rcv pkts : $cbr_sink_rcv_pkts($ids,$id3)"
            }
        }
		puts "cbr nodes overall sent pkts : $sum_cbr_sent_pkts"
		puts "cbr sink received pkts : $cbr_sink_rcv"
    
		# puts "overall throughput : [expr $cbr_sink_rcv*$opt(pktsize) / $opt(txduration)] \[Byte/s\]"
        # for {set ids 0} {$ids < $opt(n_sink)} {incr ids}  {
        #     for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
        #         puts "node($id3)-sink($ids) throughput : $cbr_sink_thr($ids,$id3) \[Byte/s\]"
        #     }
        # }
        #overall throughput
        for {set id3 0} {$id3 < $opt(nn)} {incr id3}  { 
            set cbr_overthr($id3) $cbr_sink_thr(0,$id3)
            set cbr_sink_overdelay($id3) $cbr_sink_delay(0,$id3)
            
        }

        for {set ids 1} {$ids < $opt(n_sink)} {incr ids}  {
            for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
                set cbr_overthr($id3) [expr $cbr_overthr($id3) + $cbr_sink_thr($ids,$id3)]
                set cbr_sink_overdelay($id3) [expr $cbr_sink_overdelay($id3) + $cbr_sink_delay($ids,$id3)]
                
            }
        }
        for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
                puts "overall throughput node($id3)-sinks : $cbr_overthr($id3) \[Byte/s\]"
                puts "overall delay node($id3)-sinks : $cbr_sink_overdelay($id3) \[s\]"
              
        }


        # for {set ids 0} {$ids < $opt(n_sink)} {incr ids}  {
        #     for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
        #         puts "node($id3)-sink($ids) delay : $cbr_sink_delay($ids,$id3) \[s\]"
        #         puts "node($id3)-sink($ids) delay std : $cbr_sink_delay_std($ids,$id3) \[s\]"
        #     }
        # }
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
    
