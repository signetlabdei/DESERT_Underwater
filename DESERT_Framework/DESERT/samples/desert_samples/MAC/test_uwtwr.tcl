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
# Author: Yuehan Jiang
# Version: 1.0.0


# This script is used to test UW-TWR protocol
# There are 2 static nodes at (0,0,-100) and (0,100,-100)
# and an active AUV making a trajectory described in the Waypoints
# and a passive AUV static at (60,50,-50)
# Mobility model of DESERT: uwsmposition


# Stack of the nodes                               Stack of the AAUV                                Stack of the PAUV        
#	+-----------------------+                       +-----------------------+                       +-----------------------+
#	|    7.  UW/CBR  (tx)    |                      |      7. UW/CBR(rx)    |                       |      7. UW/CBR(rx)    |
#	+-----------------------+	                    +-----------------------+                       +-----------------------+
#	|       6. UW/UDP        |                      |       6. UW/UDP       |                       |       6. UW/UDP       |
#	+-----------------------+	                    +-----------------------+	                    +-----------------------+
#	|  5. UW/staticROUTING   |                      |  5. UW/staticROUTING  |                       |  5. UW/staticROUTING  |
#	+-----------------------+	                    +-----------------------+	                    +-----------------------+
#	|       4. UW/IP         |                      |       4. UW/IP        |                       |       4. UW/IP        |
#	+-----------------------+	                    +-----------------------+	                    +-----------------------+
#	|       3. UW/MLL        |                      |       3. UWMLL        |                       |       3. UWMLL        |
#	+-----------------------+                       +-----------------------+                       +-----------------------+
#	|2.   UW/TWR/NODE        |                      | 2.  UW/TWR/AAUV       |                       | 2.  UW/TWR/PAUV       |
#	+.......................+                       +.......................+                       +.......................+
#	: 1 Module/UW/PHYSICAL   :                      : 1. Module/UW/PHYSICAL  :                      : 1. Module/UW/PHYSICAL  :
#	+.......................+	                    +.......................+	                    +.......................+
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
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwphy_clmsgs.so

load libuwinterference.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwsmposition.so
load libuwtwr.so

set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(start_clock) [clock seconds]
set opt(nn)            2 ;#NUMBER OF SENSOR NODES
set opt(total_nn)   [expr $opt(nn) +  2] ;# +1 AAUV +1 PAUV
set opt(pktsize)       125 ; #POLL and ACK size are defined in UWTWR MAC protocol
set opt(starttime)         0.1
set opt(stoptime)      550.0
set opt(txduration) [expr $opt(stoptime) - $opt(starttime)]
set opt(cbr_period)     1; #?
set opt(speed)           0.5;#[expr $opt(knots) * 0.51444444444] ;#Speed of the AUV in m/s

set opt(txpower)            156.0  ;#Power transmitted in dB re uPa
set opt(rngstream)	1

set opt(maxinterval_)    	100.0; #?
set opt(freq) 			      150000.0
set opt(bw)              	60000.0
set opt(bitrate)	 	      200.0
set opt(rngstream)	1


########################################
# Random Number Generators
########################################

global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

#set opt(tracefilename) "/tmp/${argv0}.tr"
set opt(tracefilename) "/dev/null"
set opt(tracefile) [open $opt(tracefilename) w]

#set opt(cltracefilename) "/tmp/${argv0}.cltr"
set opt(cltracefilename) "/dev/null"
set opt(cltracefile) [open $opt(cltracefilename) w]

#########################
# Module Configuration  #
#########################

MPropagation/Underwater set practicalSpreading_ 1.8
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

#CBR MODULE
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set drop_out_of_order_   0
Module/UW/CBR set debug_      0

#TWR MODULE
Module/UW/TWR/AAUV set T_ack_timer_     2
Module/UW/TWR/AAUV set debug_			1

Module/UW/TWR/NODE set T_backoff_       0.01
Module/UW/TWR/NODE set debug_			1

Module/UW/TWR/PAUV set debug_			1

#PHY MODULE
Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    10.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                156
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 1000
Module/UW/PHYSICAL  set PER_target_                 0
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0


################################
# Procedure(s) to create nodes #
################################

proc createNode { id } {

	global channel propagation data_mask ns cbr position node udp portnum ipr ipif channel_estimator
	global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global node_coordinates

	set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	set cbr($id)  [new Module/UW/CBR]
    set udp($id) [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL]
    set mac($id)  [new Module/UW/TWR/NODE]
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

	#Set the IP address of the node
	$ipif($id) addr [expr $id +1]

	set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)
    
    #Setup positions
	$position($id) setX_ 0
    $position($id) setY_ [expr $id*100]
    $position($id) setZ_ -100

	puts "node $id at ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"

	#Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

	#Propagation model
	$phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
	$phy_data($id) setInterferenceModel "MEANPOWER"
    $phy_data($id) set debug_ 0

	$mac($id) set node_id_ [expr $id]
    $mac($id) initialize

}

proc createAAUV { } {
	global channel propagation smask data_mask ns cbr_aauv position_aauv node_aauv udp_aauv portnum_aauv 
    global phy_data_aauv posdb_aauv opt mll_aauv mac_aauv ipr_aauv ipif_aauv bpsk interf_data_aauv channel_estimator
    global woss_utilities woss_creator db_manager propagation_aauv

	set node_aauv [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set cbr_aauv($cnt)  [new Module/UW/CBR] 
    }
	set udp_aauv      [new Module/UW/UDP]
    set ipr_aauv       [new Module/UW/StaticRouting]
    set ipif_aauv      [new Module/UW/IP]
    set mll_aauv       [new Module/UW/MLL]
    set mac_aauv       [new Module/UW/TWR/AAUV]
    set phy_data_aauv  [new Module/UW/PHYSICAL]

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
       $node_aauv addModule 7 $cbr_aauv($cnt) 0 "CBR"
    }
	$node_aauv addModule 6 $udp_aauv      0 "PRT"
    $node_aauv addModule 5 $ipr_aauv       0 "IPR"
    $node_aauv addModule 4 $ipif_aauv      0 "IPF"
    $node_aauv addModule 3 $mll_aauv       0 "MLL"
    $node_aauv addModule 2 $mac_aauv       1 "MAC"
    $node_aauv addModule 1 $phy_data_aauv  1 "PHY"

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       $node_aauv setConnection $cbr_aauv($cnt)  $udp_aauv     1
    }
    $node_aauv setConnection $udp_aauv $ipr_aauv      	0
    $node_aauv setConnection $ipr_aauv  $ipif_aauv       	0
    $node_aauv setConnection $ipif_aauv $mll_aauv        	0
    $node_aauv setConnection $mll_aauv  $mac_aauv        	0
    $node_aauv setConnection $mac_aauv  $phy_data_aauv   	1
    $node_aauv addToChannel $channel    $phy_data_aauv   	1

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_aauv($cnt) [$udp_aauv assignPort $cbr_aauv($cnt)]
    }

    set position_aauv [new "Position/UWSM"]
	$position_aauv set debug_ 0
    $node_aauv addPosition $position_aauv
    
    #Setup positions
    $position_aauv setX_ 30
    $position_aauv setY_ 20
    $position_aauv setZ_ -100

	puts "node aauv at ([$position_aauv getX_], [$position_aauv getY_], [$position_aauv getZ_])"

    $ipif_aauv addr 253

	set interf_data_aauv [new Module/UW/INTERFERENCE]
    $interf_data_aauv set maxinterval_ $opt(maxinterval_)
    $interf_data_aauv set debug_       0

	$phy_data_aauv setSpectralMask     $data_mask
    $phy_data_aauv setPropagation      $propagation
    $phy_data_aauv setInterference     $interf_data_aauv
	$phy_data_aauv setInterferenceModel "MEANPOWER"

	$mac_aauv initialize

}

proc createPAUV { } {

	global channel propagation smask data_mask ns cbr_pauv position_pauv node_pauv udp_pauv portnum_pauv interf_data_pauv
    global phy_data_pauv posdb_pauv opt mll_pauv mac_pauv ipr_pauv ipif_pauv bpsk interf_pauv channel_estimator
    global woss_utilities woss_creator propagation_pauv db_manager

	set node_pauv [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set cbr_pauv($cnt)  [new Module/UW/CBR]
    }
	set cbr_pauv(101)  [new Module/UW/CBR]

    set udp_pauv      [new Module/UW/UDP]
    set ipr_pauv       [new Module/UW/StaticRouting]
    set ipif_pauv      [new Module/UW/IP]
    set mll_pauv       [new Module/UW/MLL]
    set mac_pauv       [new Module/UW/TWR/PAUV]
    set phy_data_pauv  [new Module/UW/PHYSICAL]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
       $node_pauv addModule 7 $cbr_pauv($cnt) 0 "CBR"
    }
    $node_pauv addModule 7 $cbr_pauv(101) 0 "CBR"
	$node_pauv addModule 6 $udp_pauv      0 "PRT"
	$node_pauv addModule 5 $ipr_pauv       0 "IPR"
	$node_pauv addModule 4 $ipif_pauv      0 "IPF"
	$node_pauv addModule 3 $mll_pauv       0 "MLL"
	$node_pauv addModule 2 $mac_pauv       1 "MAC"
	$node_pauv addModule 1 $phy_data_pauv  1 "PHY"

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       $node_pauv setConnection $cbr_pauv($cnt)  $udp_pauv     1
    }
	$node_pauv setConnection $cbr_pauv(101)  $udp_pauv     1

	$node_pauv setConnection $udp_pauv $ipr_pauv      	0
    $node_pauv setConnection $ipr_pauv  $ipif_pauv       	0
    $node_pauv setConnection $ipif_pauv $mll_pauv        	0
    $node_pauv setConnection $mll_pauv  $mac_pauv        	0
    $node_pauv setConnection $mac_pauv  $phy_data_pauv   	1
    $node_pauv addToChannel $channel    $phy_data_pauv   	1

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_pauv($cnt) [$udp_pauv assignPort $cbr_pauv($cnt)]
    }
	set portnum_pauv(101) [$udp_pauv assignPort $cbr_pauv(101)]

	set position_pauv [new "Position/UWSM"]
	$position_pauv set debug_ 0
    $node_pauv addPosition $position_pauv
    
    #Setup positions
    $position_pauv setX_ 60
    $position_pauv setY_ 50
    $position_pauv setZ_ -50

	puts "node pauv at ([$position_pauv getX_], [$position_pauv getY_], [$position_pauv getZ_])"

    $ipif_pauv addr 252

	set interf_data_pauv [new Module/UW/INTERFERENCE]
    $interf_data_pauv set maxinterval_ $opt(maxinterval_)
    $interf_data_pauv set debug_       0

    $phy_data_pauv setSpectralMask     $data_mask
    $phy_data_pauv setPropagation      $propagation
    $phy_data_pauv setInterference     $interf_data_pauv
	$phy_data_pauv setInterferenceModel "MEANPOWER"

    $mac_pauv initialize

}

###############################
# create nodes, aauv, pauv
###############################
createAAUV
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	createNode $id1
}
createPAUV

###############################
# routing of nodes
###############################
proc connectNodes {id1} {
	
	global ipif ipr portnum cbr cbr_pauv ipif_aauv portnum_aauv ipr_pauv
	global ipif_pauv portnum_pauv ipr_aauv
	# how to set destAddr_?
	$cbr($id1) set destAddr_ [$ipif_aauv addr]
	$cbr($id1) set destPort_ $portnum_aauv($id1)
	$cbr($id1) set destAddr_ [$ipif_pauv addr]
	$cbr($id1) set destPort_ $portnum_pauv($id1)

	# $cbr(101) set destAddr_ [$id1 addr]
	# $cbr(101) set destPort_ $id1(101)
	# $cbr(101) set destAddr_ [$ipif_pauv addr]
	# $cbr(101) set destPort_ $portnum_pauv(101)
}

################################
#Setup flows
################################
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	connectNodes $id1
}

##################
# ARP tables     #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
	}
	$mll($id1) addentry [$ipif_aauv addr] [$mac_aauv addr]
	$mll($id1) addentry [$ipif_pauv addr] [$mac_pauv addr]
	$mll_pauv addentry [$ipif($id1) addr] [$mac($id1) addr]
	$mll_aauv addentry [$ipif($id1) addr] [$mac($id1) addr]
}
$mll_pauv addentry [$ipif_pauv addr] [$mac_pauv addr]
$mll_pauv addentry [$ipif_aauv addr] [$mac_aauv addr]
$mll_aauv addentry [$ipif_pauv addr] [$mac_pauv addr]
$mll_aauv addentry [$ipif_aauv addr] [$mac_aauv addr]

################################
#Start cbr(s)
################################

$ns at [expr $opt(starttime) + 5] "$mac_aauv run"

set opt(waypoint_file) "../dbs/wp_path/twr_aauv_wp_squa_ccw.csv"
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set dist 20
set opt(t_wp) [expr $dist / $opt(speed)];
puts "t wp $opt(t_wp)"
set t 360
set opt(n_run) 1
for {set id1 0} {$id1 < $opt(n_run)} {incr id1}  {
	foreach line $data {
    if {[regexp {^(.*),(.*)$} $line -> x y]} {
        	$ns at $t "$position_aauv setdest $x $y -100 $opt(speed)"
            set t [expr $t + $opt(t_wp)]
	    }
	}
}

set opt(stoptime) [expr $t + 1]
puts "stop $opt(stoptime)"


proc finish { } {
	puts "End of simulation"
	set opt(end_clock) [clock seconds]
    # puts  "done in [expr $opt(end_clock) - $opt(start_clock)] seconds!"

	# $ns flush-trace
    # close $opt(tracefile)
}

###################
# start simulation
###################

puts -nonewline "\nSimulating...\n"

$ns at [expr $opt(stoptime) + 60.0]  "finish; $ns halt" 

$ns run