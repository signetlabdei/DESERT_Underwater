#
# Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
# This script is used to test uwrov in a multihop network with 3 relays. 
# The MAC layers used is the TDMA FRAME, with the possibility to use
# pipeline technique
# The routing protocol is static routing
# N.B.: UnderwaterChannel and UW/PHYSICAL are used for PHY layer and channel.
# The way-point list is imported by an external file. 
###########################################################################
# Author: Alberto Signori 
# Version: 1.0.0
#
# NOTE: tcl sample tested on Mint 18, 64 bits OS
#
# Stack of the node and the ROV
#   +-------------------------+      +-------------------------+      +-------------------------+
#   |  7. UW/ROV/ROV          |      |  7. UW/CBR              |      |  7. UW/CTR              |
#   +-------------------------+      +-------------------------+      +-------------------------+
#   |  6. UW/UDP              |      |  6. UW/UDP              |      |  6. UW/UDP              |
#   +-------------------------+      +-------------------------+      +-------------------------+
#   |  5. UW/STATICROUTING    |      |  5. UW/STATICROUTING    |      |  5. UW/STATICROUTING    | 
#   +-------------------------+      +-------------------------+      +-------------------------+
#   |  4. UW/IP               |      |  4. UW/IP               |      |  4. UW/IP               |
#   +-------------------------+      +-------------------------+      +-------------------------+
#   |  3. UW/MLL              |      |  3. UW/MLL              |      |  3. UW/MLL              |
#   +-------------------------+      +-------------------------+      +-------------------------+
#   |  2. UW/TDMA_FRAME       |      |  2. UW/TDMA_FRAME       |      |  2. UW/TDMA_FRAME       |
#   +-------------------------+      +-------------------------+      +-------------------------+
#   |  1. UW/PHYSICAL         |      |  1. UW/PHYSICAL         |      |  1. UW/PHYSICAL         |
#   +-------------------------+      +-------------------------+      +-------------------------+
#           |         |                      |         |                      |         |     
#   +-------------------------------------------------------------------------------------------+                  
#   |                                     UnderwaterChannel                                                              |
#   +-------------------------------------------------------------------------------------------+ 

######################################
# Flags to enable or disable options #
######################################
set opt(verbose)            1
set opt(trace_files)        1  
set opt(bash_parameters)    0
set opt(ACK_Active)         0

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
load libuwrov.so
load libuwcbr.so
load libuwmmac_clmsgs.so
load libuwtdma.so
load libuwtdma_frame.so
load libuwcsmaaloha.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so

# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(nn)                 5.0 ;# Number of Nodes
set opt(rn)                 [expr int($opt(nn) - 2)]  ;# Number of Relay nodes
set opt(ROV_pktsize)        1000;#125  ;# Pkt size in byte
set opt(CTR_pktsize)        250;#125  ;# Pkt size in byte

set opt(ROV_period)         60
set opt(CTR_period)         60
set opt(guard_time)         50;#time bwtween 2 waypoints

set opt(pipeline)           1;#using of pipeline for TDMA

set opt(priority)           1
set opt(ACKpolicy)          3

set opt(starttime)          1
set opt(stoptime)           30000 
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation

set opt(txpower)            181.0  ;#Power transmitted in dB re uPa


set opt(maxinterval_)       20.0
set opt(freq)               25000.0 ;#Frequency used in Hz
set opt(bw)                 5000.0  ;#Bandwidth used in Hz
set opt(bitrate)            4800.0  ;#bitrate in bps
set opt(propagation_speed)  1500.0
set opt(max_distance)       3000    ;#Maximum distance between nodes in meter
set opt(rngstream) 1

if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires two inputs:"
        puts "- the first for the Poisson ROV period"
        puts "- the second for the rngstream"
        puts "example: ns test_uw_rov.tcl 60 13"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        return
    } else {
        set opt(ROV_period) [lindex $argv 0]
        set opt(rngstream) [lindex $argv 1];
    }   
} 

#duration slot in TDMA
set opt(max_slot_duration)  [expr ($opt(max_distance)/$opt(propagation_speed)) + ($opt(ROV_pktsize)*8/$opt(bitrate)) + 0.3]
if {$opt(pipeline) == 0} {
    set opt(tdma_frame_matrix)  "../dbs/tdma_frame_matrix/slot_rov_$opt(rn)relay.dat"
} else {
    set opt(tdma_frame_matrix)  "../dbs/tdma_frame_matrix/slot_rov_$opt(rn)relay_pipeline.dat" 
}

#set the number of slot in a frame, reads from an external file
set fp [open $opt(tdma_frame_matrix) r]
set file_data [read $fp]
set data [split $file_data "\n"]
foreach line $data {
    set opt(slot_per_frame) [regexp -all {[0-1]} $line]
}
puts "slot per frame: $opt(slot_per_frame)"

if {$opt(ACK_Active)} {
    set opt(ack_mode)           "setAckMode"    
} else {
    set opt(ack_mode)           "setNoAckMode"
}
#file with waypoints
set opt(waypoint_file)      "../dbs/wp_path/rov_path_multihop$opt(guard_time).csv"

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

if {$opt(trace_files)} {
    set opt(tracefilename) "./test_uwrovmovement_tdma_frame_multihop.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./test_uwrovmovement_tdma_frame_multihop.cltr"
    set opt(cltracefile) [open $opt(tracefilename) w]
} else {
    set opt(tracefilename) "/dev/null"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "/dev/null"
    set opt(cltracefile) [open $opt(cltracefilename) w]
}

MPropagation/Underwater set practicalSpreading_ 2
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          10
MPropagation/Underwater set shipping_           1


set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)
$data_mask setPropagationSpeed  $opt(propagation_speed)

#########################
# Module Configuration  #
#########################
#ROV and CTR configuration
Module/UW/ROV set packetSize_          $opt(ROV_pktsize)
Module/UW/ROV set period_              $opt(ROV_period)
Module/UW/ROV set PoissonTraffic_      1
Module/UW/ROV set debug_               0
Module/UW/ROV set ackPriority_         $opt(priority)
Module/UW/ROV set ackTimeout_          10
Module/UW/ROV set drop_old_waypoints_   1
Module/UW/ROV set log_flag_            0

Module/UW/ROV/CTR set packetSize_      $opt(CTR_pktsize)
Module/UW/ROV/CTR set period_          $opt(CTR_period)
Module/UW/ROV/CTR set debug_           0
Module/UW/ROV/CTR set adaptiveRTO_     1
Module/UW/ROV/CTR set adaptiveRTO_parameter_ 0.5

#TDMA FRAME configuration
Module/UW/TDMA_FRAME set frame_duration [expr $opt(slot_per_frame)*$opt(max_slot_duration)]
Module/UW/TDMA_FRAME set guard_time     [expr $opt(max_slot_duration) - 0.2]
Module/UW/TDMA_FRAME set debug_         0
Module/UW/TDMA_FRAME set sea_trial_     0
Module/UW/TDMA_FRAME set fair_mode      1
Module/UW/TDMA_FRAME set max_packet_per_slot  1
Module/UW/TDMA_FRAME set queue_size_          100
Module/UW/TDMA_FRAME set drop_old_            0
Module/UW/TDMA_FRAME set checkPriority_ $opt(priority)


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


##########################################
# Procedure(s) to create nodes (ROV,CTR) #
##########################################
proc createNode {node application id} {

    global udp ipr ipif mll mac phy channel position portnum opt interf_data 
    global propagation data_mask
   
    #module creation
    set udp($id)    [new Module/UW/UDP]
    set ipr($id)    [new Module/UW/StaticRouting]
    set ipif($id)   [new Module/UW/IP]
    set mll($id)    [new Module/UW/MLL]
    set mac($id)    [new Module/UW/TDMA_FRAME]
    set phy($id)    [new Module/UW/PHYSICAL]

    #addition modules to the specific node
    $node addModule 7 $application  1   "ROV"
    $node addModule 6 $udp($id)     1   "UDP"
    $node addModule 5 $ipr($id)     1   "IPR"
    $node addModule 4 $ipif($id)    1   "IPF"
    $node addModule 3 $mll($id)     1   "MLL"
    $node addModule 2 $mac($id)     1   "MAC"
    $node addModule 1 $phy($id)     1   "PHY"

    #module connection
    $node setConnection $application  $udp($id)     1
    $node setConnection $udp($id)     $ipr($id)     1
    $node setConnection $ipr($id)     $ipif($id)    1
    $node setConnection $ipif($id)    $mll($id)     1
    $node setConnection $mll($id)     $mac($id)     1
    $node setConnection $mac($id)     $phy($id)     1
    $node addToChannel  $channel      $phy($id)     1

    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }

    #port number
    set portnum($id) [$udp($id) assignPort $application]

    #IP address
    set ip_addr_values [expr $id + 1]
    $ipif($id) addr $ip_addr_values

    set position($id)   [new "Position/UWSM"]  
    $node addPosition $position($id)
    #values for the position setted outside this proc    

    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    #Propagation model
    $phy($id) setPropagation $propagation
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
    $phy($id) setInterferenceModel "MEANPOWER"
    #$mac($id) $opt(ack_mode)

    if {$id == 1} {
        $phy($id) set MaxTxSPL_dB_  181
        $mac($id) set queue_size_   2
    }
}

#procedure to create relays
proc createRelay {id} {

    global application_relay udp_relay ipr_relay ipif_relay mll_relay
    global mac_relay phy_relay node_relay channel portnum_relay position_relay
    global opt interf_data_relay propagation data_mask

    #module creation
    set application_relay($id) [new Module/UW/CBR]
    set udp_relay($id)         [new Module/UW/UDP]
    set ipr_relay($id)         [new Module/UW/StaticRouting]
    set ipif_relay($id)        [new Module/UW/IP]
    set mll_relay($id)         [new Module/UW/MLL]
    set mac_relay($id)         [new Module/UW/TDMA_FRAME]
    set phy_relay($id)         [new Module/UW/PHYSICAL]

    #addition modules to the specific node
    $node_relay($id) addModule 7 $application_relay($id)  1   "CBR"
    $node_relay($id) addModule 6 $udp_relay($id)          1   "UDP"
    $node_relay($id) addModule 5 $ipr_relay($id)          1   "IPR"
    $node_relay($id) addModule 4 $ipif_relay($id)         1   "IPF"
    $node_relay($id) addModule 3 $mll_relay($id)          1   "MLL"
    $node_relay($id) addModule 2 $mac_relay($id)          1   "MAC"
    $node_relay($id) addModule 1 $phy_relay($id)          1   "PHY"

    #module connection
    $node_relay($id) setConnection $application_relay($id)  $udp_relay($id)     1
    $node_relay($id) setConnection $udp_relay($id)          $ipr_relay($id)     1
    $node_relay($id) setConnection $ipr_relay($id)          $ipif_relay($id)    1
    $node_relay($id) setConnection $ipif_relay($id)         $mll_relay($id)     1
    $node_relay($id) setConnection $mll_relay($id)          $mac_relay($id)     1
    $node_relay($id) setConnection $mac_relay($id)          $phy_relay($id)     1
    $node_relay($id) addToChannel  $channel                 $phy_relay($id)     1

    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }

    #port number
    set portnum_relay($id) [$udp_relay($id) assignPort $application_relay($id)]

    #IP address
    set ip_addr_values [expr 254 - $id]
    $ipif_relay($id) addr $ip_addr_values

    #position  -->VERIFICARE SE Position/UWSM va tra "" <--
    set position_relay($id) [new "Position/BM"]
    $node_relay($id) addPosition $position_relay($id)
    #values for the position setted outside this proc


    #Interference model
    set interf_data_relay($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data_relay($id) set maxinterval_ $opt(maxinterval_)
    $interf_data_relay($id) set debug_       0

    #Propagation model
    $phy_relay($id) setPropagation $propagation
    $phy_relay($id) setSpectralMask $data_mask
    $phy_relay($id) setInterference $interf_data_relay($id)
    $phy_relay($id) setInterferenceModel "MEANPOWER"
}


#################
# Node Creation #
#################
# Create here all the nodes you want to network together
global nodeCTR node_relay nodeRov applicationCTR application_relay applicationROV

#creation node CTR and set position
set nodeCTR [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
set applicationCTR [new Module/UW/ROV/CTR]
createNode $nodeCTR $applicationCTR 0
$position(0) setX_ [expr $opt(rn)*$opt(max_distance) + 1000]
$position(0) setY_ 0
$position(0) setZ_ -1000
$applicationCTR setPosition $position(0)

$mac(0) setMacAddr 1
$mac(0) setTopologyIndex 1
$mac(0) setSTopologyFileName $opt(tdma_frame_matrix)

#creation node ROV and set position
set nodeROV [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
set applicationROV [new Module/UW/ROV]
createNode $nodeROV $applicationROV 1
$position(1) setX_ -100
$position(1) setY_ -100
$position(1) setZ_ -1000
$applicationROV setPosition $position(1)
$applicationROV setAckPolicy $opt(ACKpolicy)

$mac(1) setMacAddr [expr $opt(nn)]
$mac(1) setTopologyIndex [expr $opt(nn)]
$mac(1) setSTopologyFileName $opt(tdma_frame_matrix)

#creation node relay and set position
for {set id 0} {$id < $opt(rn)} {incr id} {
    set node_relay($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
    createRelay $id
    $position_relay($id) setX_ [expr 1000 + $opt(max_distance)*($opt(rn) - 1 - $id)]
    $position_relay($id) setY_ 0
    $position_relay($id) setZ_ -1000

    $mac_relay($id) setMacAddr [expr 2 + $id]
    $mac_relay($id) setTopologyIndex [expr 2 + $id]
    $mac_relay($id) setSTopologyFileName $opt(tdma_frame_matrix)
}

puts "Position CTR, X:[$position(0) getX_], Y:[$position(0) getY_], Z:[$position(0) getZ_]"
puts "Initial position ROV, X:[$position(1) getX_], Y:[$position(1) getY_], Z:[$position(1) getZ_]"
for {set id 0} {$id < $opt(rn)} {incr id} {
    puts "Position relay, X:[$position_relay($id) getX_], Y:[$position_relay($id) getY_], Z:[$position_relay($id) getZ_]"    
}



##################
# Setup flows    #
##################
 $applicationCTR set destAddr_ [$ipif(1) addr]
 $applicationCTR set destPort_ $portnum(1)
 $applicationROV set destAddr_ [$ipif(0) addr]
 $applicationROV set destPort_ $portnum(0)


##################
# ARP tables     #
##################
for {set id1 0} {$id1 < $opt(nn)-$opt(rn)} {incr id1} {
    for {set id2 0} {$id2 < $opt(nn)-$opt(rn)} {incr id2} {
        $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }
    for {set id_relay 0} {$id_relay < $opt(rn)} {incr id_relay} {
        $mll($id1) addentry [$ipif_relay($id_relay) addr] [$mac_relay($id_relay) addr]
        $mll_relay($id_relay) addentry [$ipif($id1) addr] [$mac($id1) addr]
    }
}
for {set id_relay 0} {$id_relay < $opt(rn)} {incr id_relay} {
    for {set id_relay2 0} {$id_relay2 < $opt(rn)} {incr id_relay2} {
    $mll_relay($id_relay) addentry [$ipif_relay($id_relay2) addr] [$mac_relay($id_relay2) addr]
    }
}


##################
# Routing tables #
##################
set ipCTR   [$ipif(0) addr]
set ipROV   [$ipif(1) addr]
for {set id 0} {$id < $opt(rn)} {incr id} {
    set ipRelay($id) [$ipif_relay($id) addr]
}

puts "IP CTR: $ipCTR"
puts "IP ROV: $ipROV"
for {set id 0} {$id < $opt(rn)} {incr id} {
    puts "IP Relay R[expr $id + 1]: $ipRelay($id)"
}

puts "MAC CTR: [$mac(0) addr]"
puts "MAC ROV: [$mac(1) addr]"
for {set id 0} {$id < $opt(rn)} {incr id} {
    puts "MAC Relay R[expr $id + 1]: [$mac_relay($id) addr]"
}

$ipr(0) addRoute $ipROV $ipRelay(0)
$ipr(1) addRoute $ipCTR $ipRelay([expr $opt(rn) - 1])

for {set id 0} {$id < $opt(rn)} {incr id} {
    if {$id < [expr $opt(rn)-1]} {
        $ipr_relay($id) addRoute $ipROV $ipRelay([expr $id + 1])
    } else {
        $ipr_relay($id) addRoute $ipROV $ipROV
    }

    if {$id > 0} {
        $ipr_relay($id) addRoute $ipCTR $ipRelay([expr $id - 1])
    } else {
        $ipr_relay($id) addRoute $ipCTR $ipCTR
    }
    
}

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

set outfile [open "test_uwrov_tdma_frame_multihop_results.csv" "w"]
close $outfile
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
foreach line $data {
    if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {
        $ns at $t "update_and_check $t"
        $ns at $t "$applicationCTR sendPosition $x $y $z"


    }
}
$ns at $opt(starttime)      "$applicationROV start"
$ns at $opt(stoptime)       "$applicationROV stop"
$ns at $opt(starttime)      "$mac(0) start"
$ns at $opt(starttime)      "$mac(1) start"
for {set id 0} {$id < $opt(rn)} {incr id} {
    $ns at $opt(starttime)      "$mac_relay($id) start"
    $ns at $opt(stoptime)       "$mac_relay($id) stop"

}
$ns at $opt(stoptime)       "$mac(0) stop"
$ns at $opt(stoptime)       "$mac(1) stop"

proc update_and_check {t} {
    global position applicationROV opt applicationCTR mac_relay mac
    $position(1) update
    set outfile [open "test_uwrov_tdma_frame_multihop_results.csv" "a"]
    puts $outfile "time: $t, positions ROV: x = [$applicationROV getX], y = [$applicationROV getY], z = [$applicationROV getZ]" 
    close $outfile
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
    global node_stats tmp_node_stats sink_stats tmp_sink_stats application_relay
    global phy_relay
    if ($opt(verbose)) {
        update_and_check $opt(stoptime)
        puts "---------------------------------------------------------------------"
        puts "Simulation summary"
        puts "number of nodes  : $opt(nn)"
        puts "ROV packet size      : $opt(ROV_pktsize) byte"
        puts "ROV period       : $opt(ROV_period) s"
        puts "CTR packet size      : $opt(CTR_pktsize) byte"
        puts "simulation length: $opt(txduration) s"
        puts "tx power         : $opt(txpower) dB"
        puts "tx frequency     : $opt(freq) Hz"
        puts "tx bandwidth     : $opt(bw) Hz"
        puts "bitrate          : $opt(bitrate) bps"
        puts "slot duration    : $opt(max_slot_duration)"
        puts "---------------------------------------------------------------------"
    } 
    set ROV_throughput              [$applicationROV getthr]
    set ROV_per                     [$applicationROV getper]
    set ROV_sent_pkts               [$mac(1) get_sent_pkts]
    set ROV_rcv_pkts                [$mac(1) get_recv_pkts]

    set CTR_throughput              [$applicationCTR getthr]
    set CTR_per                     [$applicationCTR getper]
    set CTR_sent_pkts               [$mac(0) get_sent_pkts]
    set CTR_rcv_pkts                [$mac(0) get_recv_pkts]

    set cbr_throughput              [$application_relay(0) getthr]
    set cbr_per                     [$application_relay(0) getper]
    set cbr_sent_pkts               [$application_relay(0) getsentpkts]
    set cbr_rcv_pkts                [$application_relay(0) getrecvpkts]

    set CTR_ftt                     [$applicationROV getftt]
    set CTR_ftt_std                 [$applicationROV getfttstd]
    set CTR_rtt                     [$applicationCTR getrtt]

    set ROV_ftt                     [$applicationCTR getftt]
    set ROV_ftt_std                 [$applicationCTR getfttstd]

    set ROV_sent_pkts_APP               [$applicationROV getsentpkts]
    set ROV_rcv_pkts_APP                [$applicationROV getrecvpkts]
    set CTR_sent_pkts_APP               [$applicationCTR getsentpkts]
    set CTR_rcv_pkts_APP                [$applicationCTR getrecvpkts]

    if ($opt(verbose)) {
        puts "applicationROV Throughput     : $ROV_throughput"
        puts "applicationROV PER            : $ROV_per       "
        puts "ROV packet delivery delay     : $ROV_ftt"
        puts "ROV std packet delivery delay : $ROV_ftt_std"
        puts "-------------------------------------------"
        puts "applicationCTR Throughput     : $CTR_throughput"
        puts "applicationCTR PER            : $CTR_per       "
        puts "CTR packet delivery delay     : $CTR_ftt"
        puts "CTR std packet delivery delay : $CTR_ftt_std"
        puts "-------------------------------------------"
        puts "applicationRelay Throughput   : $cbr_throughput"
        puts "-------------------------------------------"
    }

        set sum_throughput [expr $ROV_throughput + $CTR_throughput]
        set sum_sent_pkts [expr $ROV_sent_pkts + $CTR_sent_pkts]
        set sum_rcv_pkts  [expr $ROV_rcv_pkts + $CTR_rcv_pkts]
       
        set ipheadersize        [$ipif(1) getipheadersize]
        set udpheadersize       [$udp(1) getudpheadersize]
        set ROVheadersize       [$applicationROV getROVMonheadersize]
        set CTRheadersize       [$applicationCTR getROVctrheadersize]

        set transmitting_node [expr $opt(nn) - $opt(rn)]

    
    if ($opt(verbose)) {
        puts "Mean Throughput           : [expr ($sum_throughput/(($transmitting_node)*($transmitting_node-1)))]"
        puts "Sent Packets CTR --> ROV     : $CTR_sent_pkts"
        puts "Received Packets CTR --> ROV     : $ROV_rcv_pkts"
        puts "Sent Packets ROV --> CTR   : $ROV_sent_pkts"
        puts "Received Packets ROV --> CTR   : $CTR_rcv_pkts"
        puts "App Packets Sent CTR --> ROV     : $CTR_sent_pkts_APP"
        puts "App Packets Received CTR --> ROV     : $ROV_rcv_pkts_APP"
        puts "App Packets Sent ROV --> CTR   : $ROV_sent_pkts_APP"
        puts "App Packets Received ROV --> CTR   : $CTR_rcv_pkts_APP"
        puts "Received Packets CBR :    $cbr_rcv_pkts"
        puts "Sent Packets CBR :    $cbr_sent_pkts"
        puts "---------------------------------------------------------------------"
        puts "Sent Packets     : $sum_sent_pkts"
        puts "Received   : $sum_rcv_pkts"
        puts "Packet Delivery Ratio     : [expr 1.0 * $sum_rcv_pkts / $sum_sent_pkts * 100 ]"
        puts "IP Pkt Header Size        : $ipheadersize"
        puts "UDP Header Size           : $udpheadersize"
        puts "ROV Header Size           : $ROVheadersize"
        puts "CTR Header Size           : $CTRheadersize"
        puts "CTR round trip time       : $CTR_rtt"
        if {$opt(ack_mode) == "setAckMode"} {
            puts "MAC-level average retransmissions per node : [expr $sum_rtx/($opt(nn))]"
        }
        puts "---------------------------------------------------------------------"
        set ROV_packet_lost             [$phy(1) getTotPktsLost]
        set CTR_packet_lost             [$phy(0) getTotPktsLost]
        set Relay_paket_lost            [$phy_relay(0) getTotPktsLost]
        set packet_lost                 [expr $CTR_packet_lost + $ROV_packet_lost + $Relay_paket_lost]
        puts "- PHY layer statistics for the ROV -"
        puts "Tot. pkts lost            : $ROV_packet_lost"
        puts "Tot. collision CTRL       : [$phy(1) getCollisionsCTRL]"
        puts "Tot. collision DATA       : [$phy(1) getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL : [$phy(1) getCollisionsDATAvsCTRL]"
        puts "Tot. CTRL pkts lost       : [$phy(1) getTotCtrlPktsLost]"
        puts "Tot. CTRL pkts lost due to Interference   : [$phy(1) getErrorCtrlPktsInterf]"
        puts "---------------------------------------------------------------------"
        puts "- PHY layer statistics for the CTR -"
        puts "Tot. pkts lost            : $CTR_packet_lost"
        puts "Tot. collision CTRL       : [$phy(0) getCollisionsCTRL]"
        puts "Tot. collision DATA       : [$phy(0) getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL : [$phy(0) getCollisionsDATAvsCTRL]"
        puts "Tot. ROV pkts lost       : [$phy(0) getTotCtrlPktsLost]"
        puts "Tot. CTRL pkts lost due to Interference   : [$phy(0) getErrorCtrlPktsInterf]"
        puts "---------------------------------------------------------------------"
        puts "- PHY layer statistics for the relay"
        puts "Tot. pkts lost            : $Relay_paket_lost"
        puts "Tot. collision CTRL       : [$phy_relay(0) getCollisionsCTRL]"
        puts "Tot. collision DATA       : [$phy_relay(0) getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL : [$phy_relay(0) getCollisionsDATAvsCTRL]"
        puts "Tot. CTRL pkts lost       : [$phy_relay(0) getTotCtrlPktsLost]"
        puts "Tot. CTRL pkts lost due to Interference   : [$phy_relay(0) getErrorCtrlPktsInterf]"
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
    puts "----------------------------------------------"
}


$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 

$ns run