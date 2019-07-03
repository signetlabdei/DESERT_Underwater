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
# Author: Giovanni Toso <tosogiov@dei.unipd.it>
# Version: 1.0.0
# NOTE: tcl sample tested on Ubuntu 12.04, 64 bits OS
#
#########################################################################################
##
## NOTE: This script uses the PHY model "Module/MPhy/BPSK" of NS-Miracle in addPosition
## with the module "MInterference/MIV" for the computation of interference. 
## These two modules is used in this script to demonstrate their compatibility with
## DESERT stack.
## If you decide to use Module/UW/PHYSICAL from DESERT, it is suggested to use also 
## Module/UW/INTERFERENCE (which is an extension of the one coming from NS-Miracle)
## Anyways, it is possibile to use Module/UW/INTERFERENCE with Module/MPhy/BPSK whereas
## it is not possibile to use MInterference/MIV with Module/UW/INTERFERENCE for compatibility
## reasons
##
########################################################################################
# ----------------------------------------------------------------------------------
# This script depicts a very simple but complete stack in which two nodes send data
# to a common sink. The second node is used by the first one as a relay to send data to the sink.
# The routes are configured by using UW/STATICROUTING.
# The application used to generate data is UW/CBR.
# ----------------------------------------------------------------------------------
# Stack
#             Node 1                         Node 2                        Sink
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
#   |  1. Module/MPhy/BPSK     |   |  1. Module/MPhy/BPSK     |   |  1. Module/MPhy/BPSK     |
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

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libUwmStd.so
load libuwcsmaaloha.so
load libuwmmac_clmsgs.so
load libuwaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwtdma.so
load libuwflooding.so
load libuwmulti_traffic_control.so

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

set opt(nn)                 2.0 ;# Number of Nodes
set opt(starttime)          1
set opt(stoptime)           100000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(seedcbr)            0

set opt(maxinterval_)       20.0
set opt(freq)               25000.0
set opt(bw)                 5000.0
set opt(freq2)               50000.0
set opt(bw2)                 10000.0
set opt(bitrate)            4800.0
set opt(ack_mode)           "setNoAckMode"


set opt(txpower)            135.0 

set rng [new RNG]
set rng_position [new RNG]

if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires two inputs:"
        puts "- the first one is the cbr packet size (byte);"
        puts "- the second one is the cbr poisson period (seconds);"
        puts "example: ns uwcbr.tcl 125 60"
        puts "Please try again."
        return
    } else {
        set opt(pktsize)       [lindex $argv 0]
        set opt(cbr_period)    [lindex $argv 1]
    }
} else {
    set opt(pktsize)    125
    set opt(cbr_period) 60
}



set rnd_gen [new RandomVariable/Uniform]
$rng seed $opt(seedcbr)
$rnd_gen use-rng $rng

if {$opt(trace_files)} {
    set opt(tracefilename) "./test_uwcbr.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./test_uwcbr.cltr"
    set opt(cltracefile) [open $opt(tracefilename) w]
} else {
    set opt(tracefilename) "/dev/null"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "/dev/null"
    set opt(cltracefile) [open $opt(cltracefilename) w]
}

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

set data_mask2 [new MSpectralMask/Rect]
$data_mask2 setFreq       $opt(freq2)
$data_mask2 setBandwidth  $opt(bw2)

#########################
# Module Configuration  #
#########################
#UW/CBR
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1

# BPSK              
Module/MPhy/BPSK  set BitRate_          $opt(bitrate)
Module/MPhy/BPSK  set TxPower_          $opt(txpower)

#FLOODING
Module/UW/FLOODING set ttl_                       2
Module/UW/FLOODING set maximum_cache_time__time_  $opt(stoptime)

#TRAFFIC_CTR
Module/UW/MULTI_TRAFFIC_RANGE_CTR set debug_ 0
Module/UW/MULTI_TRAFFIC_RANGE_CTR set check_to_period_  50

Module/UW/CSMA_ALOHA set wait_costant_ 0.001
Module/UW/CSMA_ALOHA set listen_time_ 0.001

Module/UW/TDMA set frame_duration   0.5
Module/UW/TDMA set guard_time       0.04
Module/UW/TDMA set tot_slots        2

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    global channel propagation data_mask data_mask2 ns cbr cbr2 cbr3 position node udp portnum portnum2 portnum3 ipr ipif
    global phy posdb opt rvposx mll mll2 mac mac2 db_manager ipr2 ipif2
    global node_coordinates
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
    Module/UW/CBR set traffic_type_ 1
    set cbr($id)  [new Module/UW/CBR]
    Module/UW/CBR set traffic_type_ 2
    set cbr2($id)  [new Module/UW/CBR]
    Module/UW/CBR set traffic_type_ 3
    set cbr3($id)  [new Module/UW/CBR]

    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]

    set udp2($id)  [new Module/UW/UDP]
    set ipr2($id)  [new Module/UW/FLOODING]
    set ipif2($id) [new Module/UW/IP]

    set ctr($id)  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]

    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/CSMA_ALOHA] 
    set phy($id)  [new Module/MPhy/BPSK]
    $mll($id) setstackid 1

    set mll2($id)  [new Module/UW/MLL] 
    #set mac2($id)  [new Module/UW/TDMA] 
    set mac2($id)  [new Module/UW/CSMA_ALOHA]
    set phy2($id)  [new Module/MPhy/BPSK]
    $mll2($id) setstackid 2

    $node($id) addModule 8 $cbr($id)   0  "CBR1"
    $node($id) addModule 8 $cbr2($id)  0  "CBR2"
    $node($id) addModule 7 $udp($id)   0  "UDP1"
    $node($id) addModule 6 $ipr($id)   0  "IPR1"
    $node($id) addModule 5 $ipif($id)  2  "IPF1"   
    
    $node($id) addModule 8 $cbr3($id)   0  "CBR3"
    $node($id) addModule 7 $udp2($id)   0  "UDP2"
    $node($id) addModule 6 $ipr2($id)   0  "IPR2"
    $node($id) addModule 5 $ipif2($id)  2  "IPF2" 

    $node($id) addModule 4 $ctr($id)   2  "CTR"   

    $node($id) addModule 3 $mll($id)   2  "MLL1"
    $node($id) addModule 2 $mac($id)   2  "MAC1"
    $node($id) addModule 1 $phy($id)   0  "PHY1"

    $node($id) addModule 3 $mll2($id)   2  "MLL2"
    $node($id) addModule 2 $mac2($id)   2  "MAC2"
    $node($id) addModule 1 $phy2($id)   0  "PHY2"

    $node($id) setConnection $cbr($id)   $udp($id)   0
    $node($id) setConnection $cbr2($id)  $udp($id)   0
    $node($id) setConnection $udp($id)   $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  2
    $node($id) setConnection $ipif($id)  $ctr($id)   2

    $node($id) setConnection $cbr3($id)  $udp2($id)   0
    $node($id) setConnection $udp2($id)  $ipr2($id)   0
    $node($id) setConnection $ipr2($id)  $ipif2($id)  2
    $node($id) setConnection $ipif2($id) $ctr($id)   2

    $node($id) setConnection $ctr($id)   $mll($id)   2
    $node($id) setConnection $mll($id)   $mac($id)   2
    $node($id) setConnection $mac($id)   $phy($id)   2
    $node($id) addToChannel  $channel    $phy($id)   0

    $node($id) setConnection $ctr($id)   $mll2($id)   2
    $node($id) setConnection $mll2($id)  $mac2($id)   2
    $node($id) setConnection $mac2($id)  $phy2($id)   2
    $node($id) addToChannel  $channel    $phy2($id)   0


    set portnum($id) [$udp($id) assignPort $cbr($id) ]
    set portnum2($id) [$udp($id) assignPort $cbr2($id) ]
    set portnum3($id) [$udp2($id) assignPort $cbr3($id) ]
    if {$id > 254} {
    puts "hostnum > 254!!! exiting"
    exit
    }
    set tmp_ [expr ($id) + 1]
    $ipif($id) addr $tmp_
    $ipif2($id) addr $tmp_
    $ipr2($id) addr $tmp_
    
    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)
    
    set interf_data($id) [new "MInterference/MIV"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0
    set interf_data2($id) [new "MInterference/MIV"]
    $interf_data2($id) set maxinterval_ $opt(maxinterval_)
    $interf_data2($id) set debug_       0

    $phy($id) setPropagation $propagation    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)

    $phy2($id) setPropagation $propagation    
    $phy2($id) setSpectralMask $data_mask2
    $phy2($id) setInterference $interf_data2($id)

    $mac($id) $opt(ack_mode)
    $mac($id) setMacAddr $tmp_
    $mac($id) initialize

    $mac2($id) setMacAddr $tmp_
    $mac2($id) $opt(ack_mode)
    $mac2($id) initialize
    #$mac2($id) setSlotNumber $tmp_

    $ctr($id) addRobustLowLayer 1  "MLL1"
    $ctr($id) addUpLayer 1         "IPF1"

    $ctr($id) addRobustLowLayer 2  "MLL1"
    $ctr($id) addUpLayer 2         "IPF1"

    #$ctr($id) addRobustLowLayer 3   "MLL1"
    $ctr($id) addFastLowLayer 3     "MLL2"
    $ctr($id) addUpLayer 3          "IPF2"
}

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    createNode $id1
}

################################
# Inter-node module connection #
################################

proc connectNodes {id1 id2} {
    global ipif ipr ipr2 ipif2 portnum portnum2 portnum3 cbr cbr2 cbr3
    $cbr($id1) set destAddr_ [$ipif($id2) addr]
    $cbr($id1) set destPort_ $portnum($id2)
    $cbr2($id1) set destAddr_ [$ipif($id2) addr]
    $cbr2($id1) set destPort_ $portnum2($id2)
    $cbr3($id1) set destAddr_ [$ipif2($id2) addr]
    $cbr3($id1) set destPort_ $portnum3($id2)
}

# Setup flows
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
            connectNodes $id1 $id2
        }
    }
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
      $mll2($id1) addentry [$ipif($id2) addr] [$mac2($id2) addr]
    }   
}

# Setup positions
$position(0) setX_ 0
$position(0) setY_ 0
$position(0) setZ_ -1000

$position(1) setX_ 100
$position(1) setY_ 100
$position(1) setZ_ -1000

# Setup routing table
$ipr(0) addRoute [$ipif(1) addr] [$ipif(1) addr]
$ipr(1) addRoute [$ipif(0) addr] [$ipif(0) addr]

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)    "$cbr($id1) start"
    $ns at $opt(stoptime)     "$cbr($id1) stop"
    $ns at $opt(starttime)    "$cbr2($id1) start"
    $ns at $opt(stoptime)     "$cbr2($id1) stop"
    #$ns at $opt(starttime)    "$mac2($id1) start"
    #$ns at $opt(stoptime)     "$mac2($id1) stop"
    $ns at $opt(starttime)    "$cbr3($id1) start"
    $ns at $opt(stoptime)     "$cbr3($id1) stop"
}

    # $ns at $opt(starttime)    "$cbr3(1) start"
    # $ns at $opt(stoptime)     "$cbr3(1) stop"
###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt
    global mac propagation phy_data channel db_manager propagation
    global node_coordinates
    global ipr ipif udp cbr phy cbr2 cbr3
    global node_stats tmp_node_stats

    puts "---------------------------------------------------------------------"
    puts "Simulation summary"
    puts "number of nodes  : $opt(nn)"
    puts "packet size      : $opt(pktsize) byte"
    puts "cbr period       : $opt(cbr_period) s"
    puts "number of nodes  : $opt(nn)"
    puts "simulation length: $opt(txduration) s"
    puts "tx frequency     : $opt(freq) Hz"
    puts "tx bandwidth     : $opt(bw) Hz"
    puts "bitrate          : $opt(bitrate) bps"
    puts "---------------------------------------------------------------------"

    set sum_cbr_throughput     0
    set sum_per                0
    set sum_cbr_sent_pkts      0.0
    set sum_cbr_rcv_pkts       0.0

    set sum_cbr_throughput2     0
    set sum_per2                0
    set sum_cbr_sent_pkts2      0.0
    set sum_cbr_rcv_pkts2       0.0   

    set sum_cbr_throughput3     0
    set sum_per3                0
    set sum_cbr_sent_pkts3      0.0
    set sum_cbr_rcv_pkts3       0.0       

    for {set i 0} {$i < $opt(nn)} {incr i}  {
        set cbr_throughput           [$cbr($i) getthr]
        set cbr_sent_pkts        [$cbr($i) getsentpkts]
        set cbr_rcv_pkts           [$cbr($i) getrecvpkts]

        set cbr_throughput2           [$cbr2($i) getthr]
        set cbr_sent_pkts2        [$cbr2($i) getsentpkts]
        set cbr_rcv_pkts2           [$cbr2($i) getrecvpkts]

        set cbr_throughput3           [$cbr3($i) getthr]
        set cbr_sent_pkts3        [$cbr3($i) getsentpkts]
        set cbr_rcv_pkts3           [$cbr3($i) getrecvpkts]
        
        puts "cbr($i) throughput                    : $cbr_throughput"

        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts   [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
        set sum_cbr_throughput2 [expr $sum_cbr_throughput2 + $cbr_throughput2]
        set sum_cbr_sent_pkts2  [expr $sum_cbr_sent_pkts2 + $cbr_sent_pkts2]
        set sum_cbr_rcv_pkts2   [expr $sum_cbr_rcv_pkts2 + $cbr_rcv_pkts2]
        set sum_cbr_throughput3 [expr $sum_cbr_throughput3 + $cbr_throughput3]
        set sum_cbr_sent_pkts3  [expr $sum_cbr_sent_pkts3 + $cbr_sent_pkts3]
        set sum_cbr_rcv_pkts3   [expr $sum_cbr_rcv_pkts3 + $cbr_rcv_pkts3]
    }
        
    set ipheadersize        [$ipif(1) getipheadersize]
    set udpheadersize       [$udp(1) getudpheadersize]
    set cbrheadersize       [$cbr(1) getcbrheadersize]
    
    puts "Traffic 1 ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput/($opt(nn)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts"
    puts "Received Packets         : $sum_cbr_rcv_pkts"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"
    puts "IP Pkt Header Size       : $ipheadersize"
    puts "UDP Header Size          : $udpheadersize"
    puts "CBR Header Size          : $cbrheadersize"


    puts "Traffic 2 ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput2/($opt(nn)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts2"
    puts "Received Packets         : $sum_cbr_rcv_pkts2"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts2 / $sum_cbr_sent_pkts2 * 100]"


    puts "Traffic 3 ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput3/($opt(nn)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts3"
    puts "Received Packets         : $sum_cbr_rcv_pkts3"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts3 / $sum_cbr_sent_pkts3 * 100]"
  
    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run
