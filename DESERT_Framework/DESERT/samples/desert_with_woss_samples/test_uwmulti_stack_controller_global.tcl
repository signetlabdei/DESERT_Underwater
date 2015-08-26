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
# Author: Filippo Campagnaro <campagn1@dei.unipd.it>
# Version: 1.0.0


###########
# This script is used to test data muling in a multimodal acoustic and optical 
# network. 
# There are 8 nodes in a rectangle of 4 x 2 nodes with nearest neighbour
# 1 km apart and an AUV that patrols the network retreiving data packets
# making a trajectory described in the Waypoints. 
# The MASTER is placed in the AUV and controls the switch between UW/PHYSICAL 
# and UW/OPTICAL/PHY layers, according with the received power metrics. 
# The slave switches according to the MASTER behavior.
# Both acoustic and optical channels and PHY layers are employed.
#
############
# 
# N.B.: This example uses the Waypoint mobility model provided by WOSS.
# For more informations please refer to http://telecom.dei.unipd.it/ns/woss/
#
#
#
#
# Stack of the nodes                                           Stack of the SINK
#	+------------------------------------------+            +-------------------------------------------+
#	|       9. UW/CBR  (tx)                    |            |       9. UW/CBR(rx)                       |
#	+------------------------------------------+	          +-------------------------------------------+
#	|       8. UW/UDP                          |            |       8. UW/UDP                           |
#	+------------------------------------------+	          +-------------------------------------------+
#	|       7. UW/staticROUTING                |            |       7. UW/staticROUTING                 |
#	+------------------------------------------+	          +-------------------------------------------+
#	|       6. UW/IP                           |            |       6. UW/IP                            |
#	+------------------------------------------+	          +-------------------------------------------+
#	|       5. UW/MLL                          |            |       5. UWMLL                            |
#	+------------------------------------------+            +-------------------------------------------+
#	|       4. UW/CSMA_ALOHA/TRIGGER/NODE      |            |       4. UW/CSMA_ALOHA/TRIGGER/SINK       |
# +------------------------------------------+            +-------------------------------------------+
#	|  3. UW/MULTI_STACK_CONTROLLER_PHY_SLAVE  |            |  3. UW/MULTI_STACK_CONTROLLER_PHY_MASTER  |
# +-----------------+------------------------+            +-----------------+-------------------------+
# | 2. UW/PHYSICAL  | 1. UW/OPTICAL/PHY      |            | 1. UW/OPTICAL/PHY      | 2. UW/PHYSICAL   |
#	+-----------------+------------------------+	          +------------------------+------------------+                      
#           |                    |                                 |                      |
#           |             +------------------------------------------------+              |
#           |             |                 UW/Optical/Channel             |              |
#           |             +------------------------------------------------+              |
#           |                                                                             |
#	      +-------------------------------------------------------------------------------------+
#       |                       UnderwaterChannel                                             |
#       +-------------------------------------------------------------------------------------+

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
load libuwcsmaalohatrigger.so
load libuwinterference.so
load libuwphysical.so
load libuwmulti_stack_controller.so
load libuwoptical_channel.so
load libuwoptical_propagation.so
load libuwoptical_phy.so

set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################

set opt(start_clock) [clock seconds]


set opt(start_lat)  	 	  44.51  ;# Starting Latitude
set opt(start_long)    		13.5   ;# Starting Longitude
set opt(nn) 			        8.0    ;# Number of nodes
set opt(pktsize)	 	      125    ;# Packet size in bytes
set opt(stoptime)        	820 ;# Stoptime
set opt(dist_nodes) 		  100    ;# Distance between nodes in m
set opt(nn_in_row) 		    4      ;# Number of nodes in a row
set opt(knots)        		2      ;# Speed of the SINK in knots
set opt(speed)            [expr $opt(knots) * 0.51444444444] ;#Speed of the SINK in m/s
set opt(node_depth)       100.0
set opt(time_in_wp)       1.0
set opt(trigger_time)     5.0 ;#>5
set opt(time_interval)    10.0
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
  set opt(cbr_period) 0.01
}

set opt(cbrpr) [expr int($opt(cbr_period))]
set opt(rnpr)  [expr int($opt(rep_num))]
set opt(starttime)       	0.1
set opt(txduration)     	[expr $opt(stoptime) - $opt(starttime)]

set opt(txpower)	 	        150.0 
set opt(txpower2)           10.0
set opt(per_tgt)	 	        0.1
set opt(rx_snr_penalty_db)  0.0
set opt(tx_margin_db)		    0.0

set opt(opt_acq_db)        20
set opt(temperatura)       293.15 ; # in Kelvin
set opt(txArea)            0.000010
set opt(rxArea)            0.0000011 ; # receveing area, it has to be the same for optical physical and propagation
set opt(c)                 0.15 ; # pure seawater attenation coefficient
set opt(theta)             1
set opt(id)                [expr 1.0e-9]
set opt(il)                [expr 1.0e-6]
set opt(shuntRes)          [expr 1.49e9]
set opt(sensitivity)       0.26

set opt(node_min_angle)		 -90.0
set opt(node_max_angle)		  90.0
set opt(sink_min_angle)		 -90.0
set opt(sink_max_angle) 	  90.0
set opt(node_bathy_offset) -2.0

set opt(maxinterval_)     	10.0
set opt(freq)               26000.0  ;#Frequency used in Hz
set opt(bw)                 16000.0  ;#Bandwidth used in Hz
set opt(bitrate)            10000.0  ;#bitrate in bps

set opt(freq2)              10000000 ;#Frequency used in Hz
set opt(bw2)                100000   ;#Bandwidth used in Hz
set opt(bitrate2)           1000000.0;#bitrate in bps

set opt(ctrOptThr)          [expr 6.5e-9] ; # 6.88e^-9 - hysteresis
set opt(ctrAcThr)           [expr 7.0e13] ; # 6.826e^13 + hysteresis


### TRACE FILE
if {$opt(trace_files)} {
  set opt(tracefilename) "./test_uwcsma_trigger.tr"
  set opt(tracefile) [open $opt(tracefilename) w]
  set opt(cltracefilename) "./test_uwcsma_trigger.cltr"
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

Module/UW/CBR set debug_		         0
Module/UW/CBR set packetSize_        $opt(pktsize)
Module/UW/CBR set period_            $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_    1

# Module/UW/PHYSICAL   set debug_                    1
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

# Module/UW/OPTICAL/PHY   set debug_                      1
Module/UW/OPTICAL/PHY   set TxPower_                    $opt(txpower2)
Module/UW/OPTICAL/PHY   set BitRate_                    $opt(bitrate2)
Module/UW/OPTICAL/PHY   set AcquisitionThreshold_dB_    $opt(opt_acq_db)
Module/UW/OPTICAL/PHY   set Id_                         $opt(id)
Module/UW/OPTICAL/PHY   set Il_                         $opt(il)
Module/UW/OPTICAL/PHY   set R_                          $opt(shuntRes)
Module/UW/OPTICAL/PHY   set S_                          $opt(sensitivity)
Module/UW/OPTICAL/PHY   set T_                          $opt(temperatura)
Module/UW/OPTICAL/PHY   set Ar_                         $opt(rxArea)

set channel [new Module/UnderwaterChannel]
MPropagation/Underwater set practicalSpreading_  1.5
set propagation [new MPropagation/Underwater]

set channel2 [new Module/UW/Optical/Channel]

Module/UW/OPTICAL/Propagation set Ar_       $opt(rxArea)
Module/UW/OPTICAL/Propagation set At_       $opt(txArea)
Module/UW/OPTICAL/Propagation set c_        $opt(c)
Module/UW/OPTICAL/Propagation set theta_    $opt(theta)
# Module/UW/OPTICAL/Propagation set debug_    1

set propagation2 [new Module/UW/OPTICAL/Propagation]
$propagation2 setOmnidirectional
#

set data_mask  [new MSpectralMask/Rect]
$data_mask setFreq        $opt(freq)
$data_mask setBandwidth   $opt(bw)

set data_mask2 [new MSpectralMask/Rect]
$data_mask2 setFreq       $opt(freq2)
$data_mask2 setBandwidth  $opt(bw2)

Module/UW/CSMA_ALOHA/TRIGGER/SINK set TRIGGER_size_         1
Module/UW/CSMA_ALOHA/TRIGGER/SINK set buffer_pkts_          10000
Module/UW/CSMA_ALOHA/TRIGGER/SINK set tx_timer_duration_    $opt(trigger_time)
Module/UW/CSMA_ALOHA/TRIGGER/SINK set listen_time_          [expr 1.0e-8]
Module/UW/CSMA_ALOHA/TRIGGER/SINK set wait_costant_         [expr 1.0e-12]
# Module/UW/CSMA_ALOHA/TRIGGER/SINK set debug_     1

Module/UW/CSMA_ALOHA/TRIGGER/NODE set HDR_size_             1
Module/UW/CSMA_ALOHA/TRIGGER/NODE set buffer_pkts_          10000
Module/UW/CSMA_ALOHA/TRIGGER/NODE set listen_time_          [expr 1.0e-8]
Module/UW/CSMA_ALOHA/TRIGGER/NODE set wait_costant_         [expr 1.0e-12]
Module/UW/CSMA_ALOHA/TRIGGER/NODE set tx_timer_duration_    [expr $opt(trigger_time)-1];#3.2 for 500m distance
Module/UW/CSMA_ALOHA/TRIGGER/NODE set max_payload_          125

# Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-12]

WOSS/Position/WayPoint set time_threshold_            [expr 1.0 / $opt(speed)]
WOSS/Position/WayPoint set compDistance_              0.0
WOSS/Position/WayPoint set verticalOrientation_       0.0
WOSS/Position/WayPoint set minVerticalOrientation_    -40.0
WOSS/Position/WayPoint set maxVerticalOrientation_    40.0

set node_depth $opt(node_depth)
set sink_depth  [expr $node_depth - 3 ]

# Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE  set debug_     1
# Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER set debug_     1
Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER set alpha_     0.5

################################
# Procedure(s) to create nodes #
################################

proc createNode { id } {
    
  global channel channel2 propagation propagation2 data_mask data_mask2 ns cbr position node port portnum ipr ipif channel_estimator
  global phy posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager node_depth
  global row
  
  set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
  
  set cbr($id)      [new Module/UW/CBR] 
  set port($id)     [new Module/UW/UDP]
  set ipr($id)      [new Module/UW/StaticRouting]
  set ipif($id)     [new Module/UW/IP]
  set mll($id)      [new Module/UW/MLL] 
  set mac($id)      [new Module/UW/CSMA_ALOHA/TRIGGER/NODE]
  set ctr($id)      [new Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE]
  set phy($id)      [new Module/UW/PHYSICAL]
  set phy2($id)     [new Module/UW/OPTICAL/PHY];#[new Module/MPhy/BPSK]

  $node($id)  addModule 9 $cbr($id)   1  "CBR"
  $node($id)  addModule 8 $port($id)  1  "PRT"
  $node($id)  addModule 7 $ipr($id)   1  "IPR"
  $node($id)  addModule 6 $ipif($id)  1  "IPF"   
  $node($id)  addModule 5 $mll($id)   1  "MLL"
  $node($id)  addModule 4 $mac($id)   1  "MAC"
  $node($id)  addModule 3 $ctr($id)   1  "CTR"
  $node($id)  addModule 2 $phy($id)   1  "PHY"
  $node($id)  addModule 1 $phy2($id)  1  "PHY"

  $node($id) setConnection $cbr($id)   $port($id)  1
  $node($id) setConnection $port($id)  $ipr($id)   1
  $node($id) setConnection $ipr($id)   $ipif($id)  1
  $node($id) setConnection $ipif($id)  $mll($id)   1
  $node($id) setConnection $mll($id)   $mac($id)   1
  $node($id) setConnection $mac($id)   $ctr($id)   1
  $node($id) setConnection $ctr($id)   $phy($id)   1 
  $node($id) setConnection $ctr($id)   $phy2($id)  1
  $node($id) addToChannel  $channel    $phy($id)   1
  $node($id) addToChannel  $channel2   $phy2($id)  1


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
  set curr_depth $node_depth
  puts "$curr_x $curr_y $curr_depth"

  $position($id) setLatitude_  $curr_lat
  $position($id) setLongitude_ $curr_lon
  $position($id) setAltitude_  [expr -1.0 * $curr_depth]

  puts "node $id at ([$position($id) getLatitude_], [$position($id) getLongitude_], [$position($id) getAltitude_]) , ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"

  set interf_data($id) [new "Module/UW/INTERFERENCE"]
  $interf_data($id) set maxinterval_ $opt(maxinterval_)
  $interf_data($id) set debug_       0

  set interf_data2($id) [new "MInterference/MIV"]
  $interf_data2($id) set maxinterval_ $opt(maxinterval_)
  $interf_data2($id) set debug_       0

  $phy($id) setSpectralMask $data_mask
  $phy($id) setPropagation $propagation
  $phy($id) set debug_ 0
  $phy($id) setInterference $interf_data($id)
  $phy($id) setInterferenceModel "MEANPOWER"

  $phy2($id) setSpectralMask $data_mask2
  $phy2($id) setPropagation $propagation2
  $phy2($id) setInterference $interf_data2($id)

  $ctr($id) setManualLowerlId [$phy2($id) Id_]
  # $ctr($id) setManualSwitch
  $ctr($id) setAutomaticSwitch
  $mac($id) set node_id_ $id

}

proc createSink { } {
  global channel channel2 propagation propagation2 smask data_mask data_mask2 ns cbr_sink position_sink node_sink port_sink portnum_sink interf_data_sink
  global phy_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator sink_depth
  global woss_utilities woss_creator

  set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

  for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
    set cbr_sink($cnt)  [new Module/UW/CBR] 
  }

  set port_sink  [new Module/UW/UDP]
  set ipr_sink   [new Module/UW/StaticRouting]
  set ipif_sink  [new Module/UW/IP]
  set mll_sink   [new Module/UW/MLL] 
  set mac_sink   [new Module/UW/CSMA_ALOHA/TRIGGER/SINK]
  set ctr_sink   [new Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER]
  set phy_sink   [new Module/UW/PHYSICAL]
  set phy_sink2  [new Module/UW/OPTICAL/PHY];#[new Module/MPhy/BPSK]  

  for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
    $node_sink addModule 9 $cbr_sink($cnt) 1 "CBR"
  }
  $node_sink addModule 8 $port_sink      1 "PRT"
  $node_sink addModule 7 $ipr_sink       1 "IPR"
  $node_sink addModule 6 $ipif_sink      1 "IPF"   
  $node_sink addModule 5 $mll_sink       1 "MLL"
  $node_sink addModule 4 $mac_sink       1 "MAC"
  $node_sink addModule 3 $ctr_sink       1 "CTR"
  $node_sink addModule 2 $phy_sink       1 "PHY"
  $node_sink addModule 1 $phy_sink2      1 "PHY"

  for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
    $node_sink setConnection $cbr_sink($cnt)  $port_sink  1   
  }
  $node_sink setConnection $port_sink $ipr_sink    	      1
  $node_sink setConnection $ipr_sink  $ipif_sink   	      1
  $node_sink setConnection $ipif_sink $mll_sink    	      1 
  $node_sink setConnection $mll_sink  $mac_sink    	      1
  $node_sink setConnection $mac_sink  $ctr_sink           1
  $node_sink setConnection $ctr_sink  $phy_sink    	      1
  $node_sink setConnection $ctr_sink  $phy_sink2          1
  $node_sink addToChannel $channel    $phy_sink   	      1
  $node_sink addToChannel $channel2   $phy_sink2          1

  for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
    set portnum_sink($cnt) [$port_sink assignPort $cbr_sink($cnt)]
    if {$cnt > 252} {
      puts "hostnum > 252!!! exiting"
      exit
    }
  }

  set curr_depth $sink_depth

  set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 0 ]
  set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  0 ]

  set position_sink [new "WOSS/Position/WayPoint"]
  $position_sink addStartWayPoint $curr_lat $curr_lon [expr -1.0 * $curr_depth] $opt(speed) 0.0
  $node_sink addPosition $position_sink

  $ipif_sink addr 253

  puts "node sink at ([$position_sink getLatitude_], [$position_sink getLongitude_], [$position_sink getAltitude_]) , ([$position_sink getX_], [$position_sink getY_], [$position_sink getZ_])"

  set interf_data_sink [new "Module/UW/INTERFERENCE"]
  $interf_data_sink set maxinterval_ $opt(maxinterval_)
  $interf_data_sink set debug_       0

  set interf_data_sink2 [new "MInterference/MIV"]
  $interf_data_sink2 set maxinterval_ $opt(maxinterval_)
  $interf_data_sink2 set debug_       0

  $phy_sink setSpectralMask      $data_mask
  $phy_sink setPropagation       $propagation
  $phy_sink setInterference      $interf_data_sink
  $phy_sink setInterferenceModel "MEANPOWER"

  $phy_sink2 setSpectralMask     $data_mask2
  $phy_sink2 setPropagation      $propagation2
  $phy_sink2 setInterference     $interf_data_sink2
  
  $ctr_sink setManualSwitch
  $ctr_sink setAutomaticSwitch
  $ctr_sink setManualLowerlId [$phy_sink2 Id_]
  $ctr_sink addLayer          [$phy_sink Id_]  1 
  $ctr_sink addLayer          [$phy_sink2 Id_] 2
  $ctr_sink addThreshold      [$phy_sink Id_] [$phy_sink2 Id_] $opt(ctrAcThr)
  $ctr_sink addThreshold      [$phy_sink2 Id_] [$phy_sink Id_] $opt(ctrOptThr)

  # puts 
}

##################
# Sink WayPoints #
##################

proc createSinkWaypoints { } {
  global position_sink opt position woss_utilities sink_depth

  set toa 0.0
  set curr_lat [$position(1) getLatitude_]
  set curr_lon [$position(1) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  puts "waypoint 1  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(1) getX_]; y = [$position(1) getY_]; z = [$position(1) getZ_]; toa = $toa"


  set curr_lat [$position(2) getLatitude_]
  set curr_lon [$position(2) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  puts "waypoint 2  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(2) getX_]; y = [$position(2) getY_]; z = [$position(2) getZ_]; toa = $toa"


  set curr_lat [$position(3) getLatitude_]
  set curr_lon [$position(3) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  puts "waypoint 3  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(3) getX_]; y = [$position(3) getY_]; z = [$position(3) getZ_]; toa = $toa"


  set curr_lat [$position(7) getLatitude_]
  set curr_lon [$position(7) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  puts "waypoint 4  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(7) getX_]; y = [$position(7) getY_]; z = [$position(7) getZ_]; toa = $toa"


  set curr_lat [$position(6) getLatitude_]
  set curr_lon [$position(6) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  puts "waypoint 5  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(6) getX_]; y = [$position(6) getY_]; z = [$position(6) getZ_]; toa = $toa"

  set curr_lat [$position(5) getLatitude_]
  set curr_lon [$position(5) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  puts "waypoint 6  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(5) getX_]; y = [$position(5) getY_]; z = [$position(5) getZ_]; toa = $toa"


  set curr_lat [$position(4) getLatitude_]
  set curr_lon [$position(4) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  puts "waypoint 7  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(4) getX_]; y = [$position(4) getY_]; z = [$position(4) getZ_]; toa = $toa"

  set curr_lat [$position(0) getLatitude_]
  set curr_lon [$position(0) getLongitude_]
  set curr_depth [expr -1.0 * $sink_depth]
  set toa      [$position_sink addLoopPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp) 0 1.0]
  puts "waypoint 0  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(0) getX_]; y = [$position(0) getY_]; z = [$position(0) getZ_]; toa = $toa"
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

# proc printPos {} {
#   global position_sink
#   set x_pos [$position_sink getX_]
#   set y_pos [$position_sink getY_]
#   set z_pos [$position_sink getZ_]
#   puts -nonewline " Position = $x_pos $y_pos $z_pos"
# }
proc printPos {} {
  global position_sink
  set lat [$position_sink getLatitude_]
  set lon [$position_sink getLongitude_]
  puts -nonewline " Position = $lat $lon "
}

set partial_tot_rx 0.0

proc printInstantThgp { } {
  global mac_sink partial_tot_rx opt
  set mac_auv_rcv_pkts   [$mac_sink getDataPktsRx]
  set thgp [expr ($mac_auv_rcv_pkts-$partial_tot_rx)*$opt(pktsize)*8/$opt(time_interval)];#bps
  set partial_tot_rx     $mac_auv_rcv_pkts
  puts " Throughput = $thgp"
}
################################
#Start cbr(s)
################################

set force_stop $opt(stoptime)

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
  $ns at $opt(starttime)	     "$cbr($id1) start"
  $ns at $opt(stoptime)        "$cbr($id1) stop"
}
$ns at 20 "$mac_sink sinkRun"

for {set t 30} {$t <= $opt(stoptime)} {set t [expr $t + $opt(time_interval)]} {
  # $ns at $t "printPos"
  $ns at $t "puts -nonewline $t; printPos; printInstantThgp"
}

proc finish { } {
  global ns opt cbr outfile
  global mac cbr_sink mac_sink db_manager propagation
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
		set cbr_pkts         	 [$cbr($id3) getsentpkts]
		set mac_pkts         	 [$mac($id3) getDataPktsTx]
    set den_mac_pkts       $mac_pkts
    if {$den_mac_pkts == 0} { set den_mac_pkts 1 }
		set cbr_rcv_pkts       [$cbr_sink($id3) getrecvpkts]
    if {$opt(verbose)} {
		  puts "cbr_sink($id3) throughput                : $cbr_throughput"
		  puts "cbr_sink($id3) received packets          : $cbr_rcv_pkts"
		  puts "mac($id3) sent pkts                      : $mac_pkts"
		  puts "MAC level PDR of node		       : [expr {double($cbr_rcv_pkts)/double($den_mac_pkts)}]"
		  puts "-------------------------------------------------------------------------------------------"
    }

		set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
		set sum_per [expr $sum_per + $cbr_per]
		set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_pkts]
		set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
		set sum_mac_sent_pkts [expr $sum_mac_sent_pkts + $mac_pkts]
  }	
  set mac_auv_rcv_pkts   [$mac_sink getDataPktsRx] 
  #set tot_time  	     [$mac_sink GetTotalReceivingTime]
  
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

    #puts "Total receiving time: $tot_time"
    #puts "Mean throughput at MAC layer: [expr (($mac_auv_rcv_pkts*125*8)/($tot_time))]"
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

#$ns at [expr $opt(stoptime) + 345.0]  "$mac_sink stop_count_time"
$ns at [expr $opt(stoptime) + 350.0]  "finish; $ns halt" 

$ns run
    
