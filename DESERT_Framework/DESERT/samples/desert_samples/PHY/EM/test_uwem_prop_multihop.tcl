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
# Author: Riccardo Tumiati
# Version: 1.0.0
#
# Stack
#             Node 1                         Node 0                        Sink
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  7. UW/CBR               |   |  7. UW/CBR               |   |  7. UW/CBR  | UW/CBR     |
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  6. UW/UDP               |   |  6. UW/UDP               |   |  6. UW/UDP               |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  4. UW/IP                |   |  4. UW/IP                |   |  4. UW/IP                |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  3. UW/MLL               |   |  3. UW/MLL               |   |  3. UW/MLL               |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  2. UW/CSMA_ALOHA        |   |  2. UW/CSMA_ALOHA        |   |  2. UW/CSMA_ALOHA        |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |1. UW/ElectroMagnetic/PHY |   |1. UW/ElectroMagnetic/PHY |   |1. UW/ElectroMagnetic/PHY |
#   +--------------------------+   +--------------------------+   +--------------------------+
#            |         |                    |         |                   |         |       
#   +----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                  |
#   +----------------------------------------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(trace_files)        0
set opt(bash_parameters)    0
set opt(simulation_files)   0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libuwcsmaaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwem_channel.so
load libuwem_propagation.so
load libuwem_phy.so
load libuwem_antenna.so


#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(start_clock) [clock seconds]

set opt(distance)           5 ;# Height of the distance in m
set opt(nn)                 5 ;# Number of Nodes
set opt(pktsize)            125  ;# Pkt sike in byte
set opt(cbr_period)         60  ;# CBR period
set opt(starttime)          1
set opt(stoptime)           10000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(seedcbr)            0

set opt(maxinterval_)       20.0
set opt(freq)               868000000; # in Hz
set opt(bw)                 125000 ; # in Hz
set opt(bitrate)            27000
set opt(temp)               20 ; # in Celsius
set opt(sal)                0
set opt(antenna_gain)       2
set opt(txpower)            14.0 
set opt(PER_LUT)            "../../dbs/em_attenuation/CR_4-5/SF_7.csv"
set opt(ack_mode)           "setNoAckMode"
set opt(debug_phy)          0
set opt(debug_prop)         0
set opt(rep_num)            1
set opt(flows_num)          $opt(nn)
#set opt(flows_num)         1
set opt(seed)               0

if {$opt(bash_parameters)} {
	if {$argc != 5} {
		puts "The script requires four inputs:"
		puts "- the first for the replication number"
		puts "- the second one is for the distance"
		puts "- the third one is the number of nodes"
        puts "- the fourth one is the number of data flows"
        puts "- the fifth one is the seed"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
		return
	} else {
		set opt(rep_num)        [lindex $argv 0]    ; # Replication number
		set opt(distance)       [lindex $argv 1]    ; # Distance
        set opt(nn)             [lindex $argv 2]    ; # Number of nodes
        set opt(flows_num)      $opt(nn)

        set temp                [lindex $argv 3]    ; # CBR flow number
        if {$temp > 0 && $temp <= $opt(nn)} {
            set opt(flows_num)  $temp
        }
        set opt(seed)           [lindex $argv 4]    ; # Seed
	}
}

global defaultRNG
$defaultRNG seed $opt(seed)
for {set j 0} {$j < $opt(rep_num)} {incr j} {
    $defaultRNG next-substream
}

if {$opt(trace_files)} {
    set opt(tracefilename) "./data.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./data.cltr"
    set opt(cltracefile) [open $opt(tracefilename) w]
} else {
    set opt(tracefilename) "/dev/null"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "/dev/null"
    set opt(cltracefile) [open $opt(cltracefilename) w]
}

set channel [new Module/UW/ElectroMagnetic/Channel]

Module/UW/ElectroMagnetic/Propagation   set T_          $opt(temp)
Module/UW/ElectroMagnetic/Propagation   set S_          $opt(sal)
Module/UW/ElectroMagnetic/Propagation   set debug_      $opt(debug_prop)
set propagation [new Module/UW/ElectroMagnetic/Propagation]

Module/UW/ElectroMagnetic/PHY   set TxPower_                    $opt(txpower)
Module/UW/ElectroMagnetic/PHY   set BitRate_                    $opt(bitrate)
Module/UW/ElectroMagnetic/PHY   set debug_                      $opt(debug_phy)

set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

set antenna [new Module/UW/ElectroMagnetic/Antenna]
$antenna   setGain   $opt(antenna_gain)

Module/UW/CSMA_ALOHA set listen_time_          [expr 1.0e-4]
Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-4]

#########################
# Module Configuration  #
#########################
#UW/CBR
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      0      

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    global channel propagation data_mask ns cbr position node udp portnum ipr ipif
    global phy posdb opt rvposx mll mac db_manager antenna
    global node_coordinates
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr($id)  [new Module/UW/CBR] 
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/CSMA_ALOHA] 
    set phy($id)  [new Module/UW/ElectroMagnetic/PHY]

    $node($id) addModule 7 $cbr($id)   0  "CBR"
    $node($id) addModule 6 $udp($id)   0  "UDP"
    $node($id) addModule 5 $ipr($id)   0  "IPR"
    $node($id) addModule 4 $ipif($id)  0  "IPF"   
    $node($id) addModule 3 $mll($id)   0  "MLL"
    $node($id) addModule 2 $mac($id)   0  "MAC"
    $node($id) addModule 1 $phy($id)   0  "PHY"

    $node($id) setConnection $cbr($id)   $udp($id)   0
    $node($id) setConnection $udp($id)   $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy($id)   0
    $node($id) addToChannel  $channel    $phy($id)   0

    set portnum($id) [$udp($id) assignPort $cbr($id) ]
    if {$id > 254} {
    puts "hostnum > 254!!! exiting"
    exit
    }
    set tmp_ [expr ($id) + 1]
    $ipif($id) addr $tmp_
    
    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)
    
    $phy($id) setPropagation $propagation
    $phy($id) setSpectralMask $data_mask
    $phy($id) setLUTFileName "$opt(PER_LUT)"
    $phy($id) setLUTSeparator ","
    $phy($id) setAntenna $antenna
    $phy($id) useLUT

    $mac($id) $opt(ack_mode)
    $mac($id) initialize
}

proc createSink { } {

    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink antenna

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set cbr_sink($cnt)  [new Module/UW/CBR] 
    }
    set udp_sink       [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/CSMA_ALOHA]
    set phy_data_sink  [new Module/UW/ElectroMagnetic/PHY] 

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node_sink addModule 7 $cbr_sink($cnt) 0 "CBR"
    }
    $node_sink addModule 6 $udp_sink       0 "UDP"
    $node_sink addModule 5 $ipr_sink       0 "IPR"
    $node_sink addModule 4 $ipif_sink      0 "IPF"   
    $node_sink addModule 3 $mll_sink       0 "MLL"
    $node_sink addModule 2 $mac_sink       0 "MAC"
    $node_sink addModule 1 $phy_data_sink  0 "PHY"

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node_sink setConnection $cbr_sink($cnt)  $udp_sink      0   
    }
    $node_sink setConnection $udp_sink  $ipr_sink            0
    $node_sink setConnection $ipr_sink  $ipif_sink           0
    $node_sink setConnection $ipif_sink $mll_sink            0 
    $node_sink setConnection $mll_sink  $mac_sink            0
    $node_sink setConnection $mac_sink  $phy_data_sink       0
    $node_sink addToChannel  $channel   $phy_data_sink       0

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set portnum_sink($cnt) [$udp_sink assignPort $cbr_sink($cnt)]
        if {$cnt > 252} {
            puts "hostnum > 252!!! exiting"
            exit
        }    
    }
    
    $ipif_sink addr 254

    set position_sink [new "Position/BM"]
    $node_sink addPosition $position_sink
    set posdb_sink [new "PlugIn/PositionDB"]
    $node_sink addPlugin $posdb_sink 20 "PDB"
    $posdb_sink addpos [$ipif_sink addr] $position_sink

    $phy_data_sink setSpectralMask $data_mask
    $phy_data_sink setPropagation $propagation
    $phy_data_sink setLUTFileName "$opt(PER_LUT)"
    $phy_data_sink setLUTSeparator ","
    $phy_data_sink useLUT
    $phy_data_sink setAntenna $antenna

    $mac_sink $opt(ack_mode)
    $mac_sink initialize
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}
createSink

################################
# Inter-node module connection #
################################
proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    $cbr_sink($id1) set destPort_ $portnum($id1)
}

# Setup flows
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    connectNodes $id1
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }   
    $mll($id1) addentry [$ipif_sink addr] [ $mac_sink addr]
    $mll_sink addentry [$ipif($id1) addr] [ $mac($id1) addr]
}

set effective_distance [expr $opt(distance) - 0.5]
# Setup positions
for {set id 1} {$id < $opt(nn)} {incr id}  {
    $position($id) setX_ 0
    $position($id) setY_ 0
    $position($id) setZ_ [expr [expr 0.5 + [expr $id * $effective_distance / [expr $opt(nn) - 1]]]* -1]
}

$position(0) setX_ 0
$position(0) setY_ 0
if {$opt(nn) > 1} {
    $position(0) setZ_ -0.5
} else {
    $position(0) setZ_ [expr $opt(distance) * -1]
}

$position_sink setX_ 0
$position_sink setY_ 0
$position_sink setZ_ 0.5

# Setup routing table
for {set id 1} {$id < $opt(nn)} {incr id}  {
    $ipr($id) addRoute [$ipif_sink addr] [$ipif([expr $id - 1]) addr]
}
$ipr(0) addRoute [$ipif_sink addr] [$ipif_sink addr]

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 [expr $opt(nn) - 1]} {$id1 >= [expr $opt(nn) - $opt(flows_num)]} {incr id1 -1}  {
    $ns at $opt(starttime)    "$cbr($id1) start"
    $ns at $opt(stoptime)     "$cbr($id1) stop"
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation position position_sink
    global node_coordinates
    global ipr_sink ipr ipif udp cbr phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats

    if { !$opt(simulation_files) } {
        puts "---------------------------------------------------------------------"
        puts "Simulation summary"
        puts "number of nodes  : $opt(nn)"
        puts "distance to cover: $opt(distance) m"
        puts "packet size      : $opt(pktsize) byte"
        puts "cbr period       : $opt(cbr_period) s"
        puts "simulation length: $opt(txduration) s"
        puts "tx frequency     : $opt(freq) Hz"
        puts "tx bandwidth     : $opt(bw) Hz"
        puts "bitrate          : $opt(bitrate) bps"
        puts "---------------------------------------------------------------------"
    }

    set sum_cbr_throughput     0
    set sum_per                0
    set sum_cbr_sent_pkts      0.0
    set sum_cbr_rcv_pkts       0.0
    set sum_cbr_ftt            0.0     

    for  {set i [expr $opt(nn) - 1]} {$i >= [expr $opt(nn) - $opt(flows_num)]} {incr i -1}  {
        set cbr_throughput          [$cbr_sink($i) getthr]
        set cbr_sent_pkts           [$cbr($i) getsentpkts]
        set cbr_rcv_pkts            [$cbr_sink($i) getrecvpkts]
        set cbr_ftt                 [$cbr_sink($i) getftt]
        set depth                   [$position($i) getZ_]
        if { !$opt(simulation_files) } {
            puts "cbr_sink($i)($depth) throughput                    : $cbr_throughput"
            puts "cbr_sink($i)($depth) forward-trip-time             : $cbr_ftt"
        }

        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts   [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
        set sum_cbr_ftt        [expr $sum_cbr_ftt + $cbr_ftt]
    }
        
    set ipheadersize        [$ipif(0) getipheadersize]
    set udpheadersize       [$udp(0) getudpheadersize]
    set cbrheadersize       [$cbr(0) getcbrheadersize]
    
    if { !$opt(simulation_files) } {
        puts "Mean Throughput          : [expr ($sum_cbr_throughput/$opt(flows_num))]"
        puts "Mean Forward-Trip-Time   : [expr ($sum_cbr_ftt/$opt(flows_num))]"
        puts "Sent Packets             : $sum_cbr_sent_pkts"
        puts "Received Packets         : $sum_cbr_rcv_pkts"
        puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"
        puts "IP Pkt Header Size       : $ipheadersize"
        puts "UDP Header Size          : $udpheadersize"
        puts "CBR Header Size          : $cbrheadersize"
    } else {
        puts "[expr ($sum_cbr_throughput/$opt(flows_num))],[expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100],[expr ($sum_cbr_ftt/$opt(flows_num))]"
    }
  
    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run