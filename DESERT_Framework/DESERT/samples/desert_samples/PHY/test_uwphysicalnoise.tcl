#
# Copyright (c) 2023 Regents of the SIGNET lab, University of Padova.
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
#
# Author: Vincenzo Cimino
# Version: 1.0.0
#
# This script depicts a very simple but complete stack in which one node send data
# to a sink and a ship moving toward the sink generates noise.
# The data node sends packets using a LF and a MF modem.
# The physical layer is configured using UW/PHYSICALNOISE.
# The routes are configured by using UW/STATICROUTING.
# The application used to generate data is UW/CBR.
#
# Reference paper:
# Implementation of AUV and Ship Noise for Link Quality Evaluation in the DESERT Underwater.
# Emanuele Coccolo et al. 2018.
#
# Stack of the nodes
#        Ship  (NOISE/MOBILE)               Node 0/1 and sink
#   +--------------------------+   +--------------------------+
#   |  7. UW/CBR               |   |  7. UW/CBR               |
#   +--------------------------+   +--------------------------+
#   |  6. UW/UDP               |   |  6. UW/UDP               |
#   +--------------------------+   +--------------------------+
#   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |
#   +--------------------------+   +--------------------------+
#   |  4. UW/IP                |   |  4. UW/IP                |
#   +--------------------------+   +--------------------------+
#   |  3. UW/MLL               |   |  3. UW/MLL               |
#   +--------------------------+   +--------------------------+
#   |  2. UW/CSMA              |   |  2. UW/CSMA              |
#   +--------------------------+   +--------------------------+
#   |  1. UW/PHYSICAL          |   |  1. UW/PHYSICAL          |
#   +--------------------------+   +--------------------------+
#            |         |                    |         |
#   +---------------------------------------------------------+
#   |                     UnderwaterChannel                   |
#   +---------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose)    1
set opt(bash_parameters) 0
set opt(trace_files)  0

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
load libuwsmposition.so
load libuwcsmaaloha.so
load libuwinterference.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwphysicalnoise.so
load libuwmmac_clmsgs.so


#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(nn)			2.0
set opt(pktsize)    125; # Pkt sike in byte
set opt(starttime)  1
set opt(stoptime)   10001
set opt(txduration) [expr $opt(stoptime) - $opt(starttime)]; # Duration of the simulation

set opt(txpower)        180.0;  # Power transmitted in dB re uPa
set opt(maxinterval_)   200
set opt(Mfreq)          23000.0; # High Frequency used in Hz
set opt(Lfreq)          3500.0;  # Low Frequency used in Hz
set opt(Mbw)            16000.0; # Bandwidth (for HF) used in Hz
set opt(Lbw)            2000.0;  # Bandwidth (for LF) used in Hz
set opt(bitrate_MF)     4800.0;  # Bitrate in bps
set opt(bitrate_LF)     1000.0;  # Bitrate in bps

set opt(ack_mode)       "setNoAckMode"

set opt(cbr_period)     30
set opt(pktsize)        125

set opt(speed)          7.717;  # Ship speed in m/s (15 knots)
set opt(dist_x)         3000;  # Distance between sink and nodes along x-axis
set opt(rngstream)      1

# Ship destination along x-axis.
set opt(dest_noise)     [expr $opt(dist_x) - 1250]

if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires two input:"
        puts "- for the ship speed in m/s"
        puts "- for rngstream"
        puts "example: ns test_uw_rov.tcl 7.717 13"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        return
    } else {
        set opt(speed)  [lindex $argv 0]
        set opt(rngstream) [lindex $argv 1]
    }
}

Module/UW/CBR set packetSize_      $opt(pktsize)
Module/UW/CBR set period_          $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_  1
Module/UW/CBR set debug_           0

Module/UW/CSMA_ALOHA set debug_   0

Module/UW/PHYSICALNOISE  set AcquisitionThreshold_dB_    5.0
Module/UW/PHYSICALNOISE  set RxSnrPenalty_dB_            0
Module/UW/PHYSICALNOISE  set TxSPLMargin_dB_             0
Module/UW/PHYSICALNOISE  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICALNOISE  set MinTxSPL_dB_                10
Module/UW/PHYSICALNOISE  set MaxTxRange_                 3000
Module/UW/PHYSICALNOISE  set PER_target_                 0
Module/UW/PHYSICALNOISE  set CentralFreqOptimization_    0
Module/UW/PHYSICALNOISE  set BandwidthOptimization_      0
Module/UW/PHYSICALNOISE  set SPLOptimization_            0
Module/UW/PHYSICALNOISE  set debug_noise_                0
Module/UW/PHYSICALNOISE  set debug_                      0
Module/UW/PHYSICALNOISE  set granularity                 100
Module/UW/PHYSICALNOISE  set ship_stop                   0

MPropagation/Underwater set practicalSpreading_ 1.75
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          0
MPropagation/Underwater set shipping_           0

Module/UW/INTERFERENCE set use_maxinterval_ 0
Module/UW/INTERFERENCE set debug_ 0

Position/UWSM set debug_       0

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]

set data_mask_LF [new MSpectralMask/Rect]
$data_mask_LF setFreq              $opt(Lfreq)
$data_mask_LF setBandwidth         $opt(Lbw)
$data_mask_LF setPropagationSpeed  1500

set data_mask_MF [new MSpectralMask/Rect]
$data_mask_MF setFreq              $opt(Mfreq)
$data_mask_MF setBandwidth         $opt(Mbw)
$data_mask_MF setPropagationSpeed  1500


if {$opt(trace_files)} {
    set opt(tracefilename) "./test_uwphysicalnoise.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./test_uwphysicalnoise.cltr"
    set opt(cltracefile) [open $opt(tracefilename) w]
} else {
    set opt(tracefilename) "/dev/null"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "/dev/null"
    set opt(cltracefile) [open $opt(cltracefilename) w]
}

###########################
#Random Number Generators #
###########################
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
    $defaultRNG next-substream
}

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {
    global channel ns cbr position node udp portnum ipr ipif phy
    global opt mll mac propagation data_mask_LF data_mask_MF interf_data

    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr($id)  [new Module/UW/CBR]
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL]
    set mac($id)  [new Module/UW/CSMA_ALOHA]
    set phy($id)  [new Module/UW/PHYSICALNOISE]


    $node($id) addModule 7 $cbr($id)   1  "CBR"
    $node($id) addModule 6 $udp($id)   1  "UDP"
    $node($id) addModule 5 $ipr($id)   1  "IPR"
    $node($id) addModule 4 $ipif($id)  1  "IPF"
    $node($id) addModule 3 $mll($id)   1  "MLL"
    $node($id) addModule 2 $mac($id)   1  "MAC"
    $node($id) addModule 1 $phy($id)   1  "PHY"


    $node($id) setConnection $cbr($id)   $udp($id)   0
    set portnum($id) [$udp($id) assignPort $cbr($id)]

    $node($id) setConnection $udp($id)   $ipr($id)   1
    $node($id) setConnection $ipr($id)   $ipif($id)  1
    $node($id) setConnection $ipif($id)  $mll($id)   1
    $node($id) setConnection $mll($id)   $mac($id)   1
    $node($id) setConnection $mac($id)   $phy($id)   1
    $node($id) addToChannel  $channel    $phy($id)   1

    # Set the IP address of the node
    $ipif($id) addr [expr $id + 1]

    # Set the MAC address
    $mac($id) setMacAddr [expr $id + 5]

    set position($id) [new "Position"]
    $node($id) addPosition $position($id)

    $position($id) setX_ 0
    $position($id) setY_ 0
    $position($id) setZ_ -100

    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    #Propagation model
    $phy($id) setPropagation $propagation

    if {$id == 0} {
        $phy($id) setSpectralMask $data_mask_LF
        $phy($id) set BitRate_ $opt(bitrate_LF)
    } elseif {$id == 1} {
        $phy($id) setSpectralMask $data_mask_MF
        $phy($id) set BitRate_ $opt(bitrate_MF)
    }
    $phy($id) setInterference $interf_data($id)
    $phy($id) setInterferenceModel "MEANPOWER"

    $mac($id) $opt(ack_mode)
    $mac($id) initialize
}


proc createSink { id } {
    global channel propagation data_mask_MF ns cbr_sink position_sink node_sink
    global udp_sink portnum_sink interf_data_sink ipr_sink ipif_sink bpsk interf_sink
    global phy_data_sink posdb_sink data_mask_LF opt mll_sink mac_sink


    set node_sink($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr_sink($id)       [new Module/UW/CBR]
    set udp_sink($id)       [new Module/UW/UDP]
    set ipr_sink($id)       [new Module/UW/StaticRouting]
    set ipif_sink($id)      [new Module/UW/IP]
    set mll_sink($id)       [new Module/UW/MLL]
    set mac_sink($id)       [new Module/UW/CSMA_ALOHA]
    set phy_data_sink($id)  [new Module/UW/PHYSICALNOISE]

    $node_sink($id) addModule 7 $cbr_sink($id)       0 "CBR"
    $node_sink($id) addModule 6 $udp_sink($id)       0 "UDP"
    $node_sink($id) addModule 5 $ipr_sink($id)       0 "IPR"
    $node_sink($id) addModule 4 $ipif_sink($id)      0 "IPF"
    $node_sink($id) addModule 3 $mll_sink($id)       0 "MLL"
    $node_sink($id) addModule 2 $mac_sink($id)       0 "MAC"
    $node_sink($id) addModule 1 $phy_data_sink($id)  0 "PHY"

    $node_sink($id) setConnection $cbr_sink($id)  $udp_sink($id)            0
    $node_sink($id) setConnection $udp_sink($id)  $ipr_sink($id)            0
    $node_sink($id) setConnection $ipr_sink($id)  $ipif_sink($id)           0
    $node_sink($id) setConnection $ipif_sink($id) $mll_sink($id)            0
    $node_sink($id) setConnection $mll_sink($id)  $mac_sink($id)            0
    $node_sink($id) setConnection $mac_sink($id)  $phy_data_sink($id)       0
    $node_sink($id) addToChannel  $channel   $phy_data_sink($id)            0

    set portnum_sink($id) [$udp_sink($id) assignPort $cbr_sink($id)]

    $ipif_sink($id) addr 254

    set position_sink($id) [new "Position/BM"]
    $node_sink($id) addPosition $position_sink($id)

    $position_sink($id) setX_ $opt(dist_x)
    $position_sink($id) setY_ 200
    $position_sink($id) setZ_ -100

    $mac_sink($id) setMacAddr [expr $id + 100]

    set interf_data_sink($id) [new "Module/UW/INTERFERENCE"]
    $interf_data_sink($id) set maxinterval_ $opt(maxinterval_)
    $interf_data_sink($id) set debug_       0

    if {$id == 0} {
        $phy_data_sink($id) setSpectralMask $data_mask_LF
    } elseif {$id == 1} {
        $phy_data_sink($id) setSpectralMask $data_mask_MF
    }
    $phy_data_sink($id) setInterference $interf_data_sink($id)
    $phy_data_sink($id) setInterferenceModel "MEANPOWER"
    $phy_data_sink($id) setPropagation $propagation

    $mac_sink($id) $opt(ack_mode)
    $mac_sink($id) initialize
}

proc createNoise { id } {
    global channel ns cbrN positionN nodeN udpN portnumN iprN ipifN
    global opt mllN macM propagation data_mask_LF data_mask_MF interf_dataN

    set nodeN($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbrN($id)  [new Module/UW/CBR]
    set udpN($id)  [new Module/UW/UDP]
    set iprN($id)  [new Module/UW/StaticRouting]
    set ipifN($id) [new Module/UW/IP]
    set mllN($id)  [new Module/UW/MLL]
    set macN($id)  [new Module/UW/CSMA_ALOHA]
    set phyN($id)  [new Module/UW/PHYSICALNOISE]


    $nodeN($id) addModule 7 $cbrN($id)   1  "CBR"
    $nodeN($id) addModule 6 $udpN($id)   1  "UDP"
    $nodeN($id) addModule 5 $iprN($id)   1  "IPR"
    $nodeN($id) addModule 4 $ipifN($id)  1  "IPF"
    $nodeN($id) addModule 3 $mllN($id)   1  "MLL"
    $nodeN($id) addModule 2 $macN($id)   1  "MAC"
    $nodeN($id) addModule 1 $phyN($id)   1  "PHY"


    $nodeN($id) setConnection $cbrN($id)   $udpN($id)   0
    set portnumN($id) [$udpN($id) assignPort $cbrN($id) ]

    $nodeN($id) setConnection $udpN($id)   $iprN($id)   1
    $nodeN($id) setConnection $iprN($id)   $ipifN($id)  1
    $nodeN($id) setConnection $ipifN($id)  $mllN($id)   1
    $nodeN($id) setConnection $mllN($id)   $macN($id)   1
    $nodeN($id) setConnection $macN($id)   $phyN($id)   1
    $nodeN($id) addToChannel  $channel    $phyN($id)   1


    # Set the IP address of the node
    $ipifN($id) addr [expr $id + 20]

    # Set the MAC address
    $macN($id) setMacAddr [expr $id + 15]

    set positionN($id) [new "Position/UWSM"]
    $nodeN($id) addPosition $positionN($id)

    $positionN($id) setX_ 0
    $positionN($id) setY_ 200
    $positionN($id) setZ_ -5

    $positionN($id) setdest $opt(dest_noise) [$positionN($id) getY_] [$positionN($id) getZ_] $opt(speed)

    #Interference model
    set interf_dataN($id)  [new "Module/UW/INTERFERENCE"]
    $interf_dataN($id) set maxinterval_ $opt(maxinterval_)
    $interf_dataN($id) set debug_       0

    #Propagation model
    $phyN($id) setPropagation $propagation
    $phyN($id) setSpectralMask $data_mask_MF
    $phyN($id) setInterference $interf_dataN($id)
    $phyN($id) setInterferenceModel "MEANPOWER"

    $macN($id) $opt(ack_mode)
    $macN($id) initialize
}


#################
# Node Creation #
#################
createNode 0; # LF node
createNode 1; # MF node

createSink 0; # LF sink
createSink 1; # MF sink

createNoise 5

puts "Start position sink : [$position_sink(0) getX_],[$position_sink(0) getY_],[$position_sink(0) getZ_]"
puts "Start position node : [$position(0) getX_],[$position(0) getY_],[$position(0) getZ_]"
puts "Start position ship : [$positionN(5) getX_],[$positionN(5) getY_],[$positionN(5) getZ_]"

################################
# Inter-node module connection #
################################
proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr($id1) set destAddr_ [$ipif_sink($id1) addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)

    $cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    $cbr_sink($id1) set destPort_ $portnum($id1)
}


# Setup flows
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
    connectNodes $id1
}

##############
# ARP tables #
##############
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
 for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
   $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
 }

 $mll($id1) addentry [$ipif_sink($id1) addr] [ $mac_sink($id1) addr]
 $mll_sink($id1) addentry [$ipif($id1) addr] [ $mac($id1) addr]
}


# Setup routing table
$ipr(0) addRoute [$ipif_sink(0) addr] [$ipif_sink(0) addr]
$ipr(1) addRoute [$ipif_sink(1) addr] [$ipif_sink(1) addr]

#####################
# Start/Stop Timers #
#####################

# set opt(nn) 1.0 ;# Activate only LF
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)    "$cbr($id1) start"
    $ns at $opt(stoptime)     "$cbr($id1) stop"
}

for {set id 0} {$id < $opt(nn)} {incr id} {
    puts "Add noise to id $id"
    $phy_data_sink($id) addSource 5 180 "cargo" $positionN(5)
}

proc update_and_check { t id } {
    global positionN

    $positionN($id) update

    puts "Ship position at time $t : [$positionN($id) getX_],[$positionN($id) getY_],[$positionN($id) getZ_]"
}

for {set t $opt(starttime)} {$t < 250} {incr t 10}  {
    $ns at $t "update_and_check $t 5"
}

###################
# Final Procedure #
###################
proc finish {} {
    global ns opt
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global node_coordinates
    global ipr_sink ipr ipif udp cbr phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats

    puts "---------------------------------------------------------------------"
    puts "Simulation summary"
    puts "number of nodes  : $opt(nn)"
    puts "packet size      : $opt(pktsize) byte"
    puts "cbr period       : $opt(cbr_period) s"
    puts "simulation length: $opt(txduration) s"
    puts "tx frequency MF  : $opt(Mfreq) Hz"
    puts "tx frequency LF  : $opt(Lfreq) Hz"
    puts "tx bandwidth MF  : $opt(Mbw) Hz"
    puts "tx bandwidth LF  : $opt(Lbw) Hz"
    puts "bitrate MF       : $opt(bitrate_MF) bps"
    puts "bitrate LF       : $opt(bitrate_LF) bps"
    puts "---------------------------------------------------------------------"

    set sum_cbr_throughput     0.0
    set sum_per                0.0
    set sum_cbr_sent_pkts      0.0
    set sum_cbr_rcv_pkts       0.0


    set cbr_throughput_LF  [$cbr_sink(0) getthr]
    set cbr_throughput_MF  [$cbr_sink(1) getthr]
    puts "CBR throughput LF  : $cbr_throughput_LF"
    puts "CBR throughput MF  : $cbr_throughput_MF"

    set cbr_sent_pkts_LF  [$cbr(0) getsentpkts]
    set cbr_sent_pkts_MF  [$cbr(1) getsentpkts]
    set cbr_rcv_pkts_LF   [$cbr_sink(0) getrecvpkts]
    set cbr_rcv_pkts_MF   [$cbr_sink(1) getrecvpkts]

    puts ""
    puts "dist_x : $opt(dist_x)"
    puts "PDR_LF: [expr 1.0 * $cbr_rcv_pkts_LF / $cbr_sent_pkts_LF * 100]"
    puts "PDR_MF: [expr 1.0 * $cbr_rcv_pkts_MF / $cbr_sent_pkts_MF * 100]"
    puts ""

    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt"
$ns run
