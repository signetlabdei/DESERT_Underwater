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
# Author: Sara Falleni 
# Version: 1.0.0
# NOTE: tcl sample tested on Ubuntu 20.04, 64 bits OS
# This Test is to confirm that OFDM Physical layer and Interference can be intgrated in DESERT
#
#########################################################################################
##
## NOTE: This script uses the PHY model UW/OFDM/PHY implemented in DESERT
##
########################################################################################
# ----------------------------------------------------------------------------------
# This script depicts a very simple but complete stack in which two nodes send data
# to a common sink. The second node is used by the first one as a relay to send data to the sink.
# The routes are configured by using UW/STATICROUTING.
# The application used to generate data is UW/CBR.
# ----------------------------------------------------------------------------------
# Stack
#             Node 1                         Node 2          ..........     Sink
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
#   |  2. UW/OFDM_ALOHA        |   |  2. UW/OFDM_ALOHA        |   |  2. UW/OFDM_ALOHA        |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  1. UW/PHY/OFDMPHY       |   |  1. UW/PHY/OFDMPHY       |   |  1. UW/PHY/OFDMPHY       |
#   +--------------------------+   +--------------------------+   +--------------------------+
#            |         |                    |         |                   |         |       
#   +----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                  |
#   +----------------------------------------------------------------------------------------+


# ALOHA standard functioning, all bandwidth used and backoff 

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
load libmmac.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
#load libuwcsmaaloha.so
load libuwofdmaloha.so
load libuwinterference.so
load libuwinterferenceofdm.so
load libuwphy_clmsgs.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwstats_utilities.so
load libuwphysical.so
load libmsgdisplay.so
load libuwofdmphy.so


#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
# selective variables to run the simulation
##################
set opt(nn)                 15 ;# Number of Nodes
set opt(starttime)          1  ;# simulation start time, default time start 1
set opt(stoptime)           10000 ;# stop time in sec previously 100000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(seedcbr)            1  ;# cbr seed value
set opt(trace_files)          0  ;

set opt(maxinterval_)       20.0  ;# max interval between two packets. upperlimit
set opt(pktsize)            1536;
set opt(freq)               125000.0 ;# frequency
set opt(bw)                 125000.0 ;# bandwidth
set opt(bitrate)            64000.0  ;# bitrate or transmission rate
set opt(subCarrier)         8  ;# used to define number of sub carrier. But not used as having trouble setting the value.
set opt(subCarSize)         [expr $opt(bw)/$opt(subCarrier) ]     ;# used to define the bandwidth of each subcarrier
set opt(ack_mode)           "setAckMode"
set opt(print_mode)         "printTransitions"
set opt(filename)           "expLog.csv"
set opt(modulation)         "BPSK"
set opt(AckInBand)          1
set opt(ctrlCar)            0
set opt(alohaOnCarrier)     -1
set opt(txpower)            130.0  ;# Transmitting sensitivity 130dB ±3dB re 1µPa/V at 1m at 100kHz TC4013
# default values for traffic generation and transmission
set opt(pktsize)    1536
set opt(cbr_period) 15
set opt(seedcbr)	3
set opt(rngstream)  1
# this block is used for defining some of the simulation variable value by commandline
# argument. If no argument is found some default value is set. 
if {$opt(bash_parameters)} {
    if {$argc != 7} {
        puts "The script requires seven inputs:"
        puts "- the first one is the cbr period"
        puts "- the second one is the filename where results are saved"
        puts "- the third one is the num of nodes"
        puts "- the fourth one is the modulation"
        puts "- the fifth one is AckInBand value"
        puts "- the sixth one is the Packet Size"
        puts "- the seventh one is the rngstream"
        puts "example: ns test_uwcbr_macalohaofdm.tcl 5 fullBandexp.csv 5 BPSK 1 1536"
        puts "Please try again."
        return
    } else {
        set opt(cbr_period)    [lindex $argv 0]
        set opt(filename)      [lindex $argv 1]
        set opt(nn)            [lindex $argv 2] 
        set opt(subCarSize)    [expr $opt(bw)/$opt(subCarrier) ] 
        set opt(modulation)     [lindex $argv 3]
        set opt(AckInBand)     [lindex $argv 4]
        set opt(pktsize)        [lindex $argv 5]
        set opt(rngstream)	   [lindex $argv 6]
    }
}

#random generator
global defaultRNG
for {set k 0} {$k < $opt(seedcbr)} {incr k} {
	$defaultRNG next-substream
}

# Defining trace file and log files for debugging
if {$opt(trace_files)} {
    set opt(tracefilename) "./test_uwcbr_ofdm.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./test_uwcbr_ofdm.cltr"
    set opt(cltracefile) [open $opt(tracefilename) w]
} else {
    set opt(tracefilename) "/dev/null"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "/dev/null"
    set opt(cltracefile) [open $opt(cltracefilename) w]
}

# variables related to channel and setting their values
MPropagation/Underwater set practicalSpreading_ 1.8
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

#########################
# Module Configuration  #
#########################
#UW/CBR
Module/UW/CBR set packetSize_          $opt(pktsize)
# Max period between pkts for poisson, or fixed period for not poisson 
Module/UW/CBR set period_              $opt(cbr_period)  
# If poisson distribution is used
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set seedcbr_              $opt(seedcbr)
Module/UW/CBR set debug_               0

#UW/OFDM/PHY variables THese variables setup are taken as the DESERT default values. 
# 
Module/UW/OFDM/PHY  set BitRate_                    $opt(bitrate)
Module/UW/OFDM/PHY  set AcquisitionThreshold_dB_    15.0 
Module/UW/OFDM/PHY  set RxSnrPenalty_dB_            0
Module/UW/OFDM/PHY  set TxSPLMargin_dB_             0
Module/UW/OFDM/PHY  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/OFDM/PHY  set MinTxSPL_dB_                10
Module/UW/OFDM/PHY  set MaxTxRange_                 200
Module/UW/OFDM/PHY  set PER_target_                 0    
Module/UW/OFDM/PHY  set CentralFreqOptimization_    0
Module/UW/OFDM/PHY  set BandwidthOptimization_      0
Module/UW/OFDM/PHY  set SPLOptimization_            0
Module/UW/OFDM/PHY  set debug_                      0
Module/UW/OFDM_ALOHA  set uwofdmaloha_debug_        0




# random number generator with a range. used for placing the source nodes
proc myRand {min max} {
    expr {int(rand() * ($max + 1 - $min)) + $min}
}


################################
# Procedure(s) to create source nodes #
# All nodes share same OFDM phy layer implementation #
################################
proc createNode { id } {

    puts "creating node $id"

    global channel propagation data_mask ns cbr position node udp portnum ipr ipif
    global phy posdb opt rvposx mll mac db_manager
    global node_coordinates
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr($id)  [new Module/UW/CBR] 
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/OFDM_ALOHA] 
    set phy($id)  [new Module/UW/OFDM/PHY]

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


    # placing the  nodes in cube of 70 meter side
    # The area in centered in 0 0 -35 
    set x_pos [myRand -35 35]
    set y_pos [myRand -35 35]
    set z_pos [myRand -70 0]

    $position($id) setX_ $x_pos
    $position($id) setY_ $y_pos
    $position($id) setZ_ $z_pos

    puts "Node Placed at $x_pos $y_pos $z_pos"
    
    #Setting up Interference Model: in particular number of subcarrier used in a node
    set interf_data($id) [new "Module/UW/INTERFERENCEOFDM"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0
    $interf_data($id) setInterfCarriers $opt(subCarrier)

    # SEtting up phy layer variable from script. 
    $phy($id) setPropagation $propagation
    $phy($id) init_ofdm_node $opt(nn) $opt(freq) $opt(subCarrier) $id 
    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
   
    $mac($id) $opt(ack_mode)
    $mac($id) $opt(print_mode)
    $mac($id) initialize
    $mac($id) init_macofdm_node $opt(subCarrier) $opt(subCarSize) $opt(ctrlCar) $opt(modulation)

    $cbr($id) set seedcbr_ $id
    

}

########################################################
# Procedure for creating the sink node # 
########################################################


proc createSink { } {

    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
    puts "creating sink"

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set cbr_sink($cnt)  [new Module/UW/CBR] 
    }

    set udp_sink       [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/OFDM_ALOHA]
    set phy_data_sink  [new Module/UW/OFDM/PHY] 

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

    $position_sink setX_ 0
    $position_sink setY_ 0
    $position_sink setZ_ -10

    set interf_data_sink [new "Module/UW/INTERFERENCEOFDM"]
    $interf_data_sink set maxinterval_ $opt(maxinterval_)
    $interf_data_sink set debug_       0
    $interf_data_sink setInterfCarriers $opt(subCarrier)

    $phy_data_sink setSpectralMask $data_mask
    $phy_data_sink setInterference $interf_data_sink
    $phy_data_sink setPropagation $propagation
    
    $phy_data_sink init_ofdm_node $opt(nn) $opt(freq) $opt(subCarrier) 254
    $phy_data_sink setBufferSize 1000 
    $phy_data_sink setBrokenCar 3 4

    $mac_sink $opt(ack_mode)
    $mac_sink $opt(print_mode)
    $mac_sink initialize
    $mac_sink init_macofdm_node $opt(subCarrier) $opt(subCarSize) $opt(ctrlCar) $opt(modulation)  
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}
Module/UW/CBR set period_              $opt(stoptime) 

# Module/UW/OFDM/PHY  set debug_                      1
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

# Setup flows connecting each source nodes to the sink
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


# routing table for each nodes. For onehop this is the flow between a source and sink node. 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ipr($id1) addRoute [$ipif_sink addr] [$ipif_sink addr]
}


puts "Ready for ns simulation "

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)    "$cbr($id1) start"
    $ns at $opt(stoptime)     "$cbr($id1) stop"
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global node_coordinates
    global ipr_sink ipr ipif udp cbr phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats

    puts "---------------------------------------------------------------------"
    puts "SIMULATION SUMMARY "
    puts "number of nodes    : $opt(nn)"
    puts "packet size        : $opt(pktsize) byte"
    puts "cbr period         : $opt(cbr_period) s"
    puts "simulation length  : $opt(txduration) s"
    puts "tx frequency       : $opt(freq) Hz"
    puts "tx bandwidth       : $opt(bw) Hz"
    puts "bitrate            : $opt(bitrate) bps"
    puts "carrier size       : $opt(subCarSize) "
    puts "number of carriers : $opt(subCarrier)"

    set sum_cbr_throughput     0
    set sum_per                0
    set sum_cbr_sent_pkts      0
    set sum_cbr_rcv_pkts       0.0    
    set sum_tx_power_consumed      0.0
    set sum_ftt       0.0
    set sum_txtime 0.0
    set sum_phy_sent_pkt 0

    for {set i 0} {$i < $opt(nn)} {incr i}  {
        # set txtime                      [$phy($i) getTransmissionTime]
        set cbr_throughput              [$cbr_sink($i) getthr]
        set cbr_sent_pkts               [$cbr($i) getsentpkts]
        set cbr_rcv_pkts                [$cbr_sink($i) getrecvpkts]
        set ftt_at_sink                 [$cbr_sink($i) getftt]
        set txtime_at_sink              [$cbr_sink($i) gettxtime]
        set tx_power_consumed           [$phy($i) getConsumedEnergyTx]
        set phy_sent_pkt                [$phy($i) getPhyPktSent]
        set sum_cbr_throughput          [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts           [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts            [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
        set sum_tx_power_consumed       [expr $sum_tx_power_consumed + $tx_power_consumed]
        set sum_ftt                     [expr $sum_ftt + $ftt_at_sink]
        set sum_txtime                  [expr $sum_txtime + $txtime_at_sink]
        set sum_phy_sent_pkt            [expr $sum_phy_sent_pkt + $phy_sent_pkt]
    }
        
    set ipheadersize        [$ipif(1) getipheadersize]
    set udpheadersize       [$udp(1) getudpheadersize]
    set cbrheadersize       [$cbr(1) getcbrheadersize]
    


    set delay      [$phy_data_sink getRxTime]
    puts "                                                                     "
    puts "---------------------------------------------------------------------"
    puts "SINK APP LEVEL STATISTICS "
        puts "Average ftt           : [expr $sum_ftt / ($opt(nn))]"
        puts "Mean Throughput          : [expr ($sum_cbr_throughput/($opt(nn)))]"
        puts "Sent Packets             : $sum_cbr_sent_pkts"
        puts "Received Packets         : $sum_cbr_rcv_pkts"
        puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"
        puts "IP Pkt Header Size       : $ipheadersize"
        puts "UDP Header Size          : $udpheadersize"
        puts "CBR Header Size          : $cbrheadersize"
    puts "                                                                     "
    puts "---------------------------------------------------------------------"
    puts "SINK PHY LAYER STATISTICS"
        puts "Tot. pkts lost                    : [$phy_data_sink getTotPktsLost]"
        puts "Tot. collision CTRL               : [$phy_data_sink getCollisionsCTRL]"
        puts "Tot. collision DATA               : [$phy_data_sink getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL       : [$phy_data_sink getCollisionsDATAvsCTRL]"
        puts "Tot. delay time at sink (PHY)     : $delay "
        puts "Tot. RX energy consumed at sink   : [$phy_data_sink getConsumedEnergyRx]"
        puts "Tot. TX energy consumed per node  : [ expr ($sum_tx_power_consumed / ($opt(nn))) ]"
        puts "done!"

    puts "                                                                     "
    puts "----------------------------DEBUG------------------------------"
    puts "Packets Sent To MAC Layer   : [$phy_data_sink getSentUpPkts]"
    puts "Sent Packets             : $sum_cbr_sent_pkts"


    $ns flush-trace
    close $opt(tracefile)

    if { [file exists $opt(filename)] != 1} {
        set fileID [open $opt(filename) "a"]
        puts $fileID " nodes, car, ftt, pdr, coll, simlen, cbrT, totSentPkt, totRecvPkt, avgSentPkt, PktLost, CtrlPktLost, LowSnr, NErr, CErr, TxPen, FColl, MErr, TrTime, CtrlCErr, CtrlFColl, AckTx, PhySentPkt\n"
    } else {
        set fileID [open $opt(filename) "a"]
    }    
    puts -nonewline $fileID "     $opt(nn),    $opt(subCarrier), " 
    puts -nonewline $fileID [format "%.2f" [expr $sum_ftt / ($opt(nn))]],  
    puts -nonewline $fileID [format " %.2f" [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]], 
    puts -nonewline $fileID "   [$phy_data_sink getCollisionsDATA],    $opt(txduration),  $opt(cbr_period), $sum_cbr_sent_pkts, $sum_cbr_rcv_pkts,"
    puts -nonewline $fileID [format "  %.1f," [expr $sum_cbr_sent_pkts / $opt(nn)]]
    puts -nonewline $fileID "    [$phy_data_sink getTotPktsLost], [$phy_data_sink getTotCtrlPktsLost], [$phy_data_sink getLowSnrPktLost], [$phy_data_sink getNoiseErrPktLost], [$phy_data_sink getCollErrPktLost], "
    puts -nonewline $fileID "   [$phy_data_sink getTxPenPktLost],    [$phy_data_sink getFreqCollPktLost],    [$phy_data_sink getModErrPktLost], "  
    puts -nonewline $fileID [format "  %.5f, " [expr $sum_txtime / $opt(nn)]]
    puts $fileID "[$phy_data_sink getCtrlCerrPktLost], [$phy_data_sink getCtrlFCollPktLost], [$mac_sink getAckPktsTx], $sum_phy_sent_pkt"
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run
