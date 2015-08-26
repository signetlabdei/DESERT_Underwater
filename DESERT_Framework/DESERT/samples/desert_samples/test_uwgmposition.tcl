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
#
# ----------------------------------------------------------------------------------
# This script depicts a very simple but complete stack in which two nodes send data
# to a common sink. The routes are dinamic and decided by UW/ICRP protocol.
# The application used to generate data is UW/CBR.
# The node are not fixed, but they move accorgin to the UWGMPOSITION mobility model.
# ----------------------------------------------------------------------------------
# Stack
#             Node 1                         Node 2                        Sink
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  7. UW/CBR               |   |  7. UW/CBR               |   |  7. UW/CBR  | UW/CBR     |
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  6. UW/UDP               |   |  6. UW/UDP               |   |  6. UW/UDP               |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  5. UW/SUNNode           |   |  5. UW/SUNNode           |   |  5. UW/SUNSink           |
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
set opt(verbose)            1
set opt(bash_parameters)    0
set opt(trace_files)        0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libUwmStd.so
load libuwcsmaaloha.so
load libuwip.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libsun.so
load libuwgmposition.so

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
set opt(speedMean)          1

set opt(maxinterval_)       20.0
set opt(freq)               25000.0
set opt(bw)                 5000.0
set opt(bitrate)            4800.0
set opt(ack_mode)           "setNoAckMode"

# Parameters used to configure the BPSK module of WOSS
set opt(txpower)            135.0 
set opt(per_tgt)            0.1
set opt(rx_snr_penalty_db)  -10.0
set opt(tx_margin_db)       10.0

set rng [new RNG]
set rng_position [new RNG]

if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires two inputs:"
        puts "- the first one is the cbr packet size (byte);"
        puts "- the second one is the cbr poisson period (seconds);"
        puts "example: ns uwgmposition.tcl 125 60"
        puts "Please try again."
        return
    } else {
        set opt(pktsize)       [lindex $argv 0]
        set opt(cbr_period)    [lindex $argv 1]
        set opt(buffer_period) [expr $opt(cbr_period) / 3]
        if {$opt(buffer_period) > 20} {
            set opt(buffer_period) 20
        }
        $rng seed         $opt(seedcbr)
    }
} else {
    set opt(pktsize)       125
    set opt(cbr_period)    60
    set opt(buffer_period) [expr $opt(cbr_period) / 3]
    if {$opt(buffer_period) > 20} {
        set opt(buffer_period) 20
    }
        $rng seed         $opt(seedcbr)
}

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng

if {$opt(trace_files)} {
    set opt(tracefilename) "./test_uwgmposition.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./test_uwgmposition.cltr"
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

#########################
# Module Configuration  #
#########################
# UW/CBR
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1

# UW/SUN
Module/UW/SUNNode set period_data_                  $opt(buffer_period);
Module/UW/SUNNode set timer_route_validity_	    [expr $opt(stoptime)*100]
Module/UW/SUNNode set timer_sink_probe_validity_    [expr $opt(stoptime)*100]
Module/UW/SUNNode set max_ack_error_		    3
Module/UW/SUNNode set buffer_max_size_              5
Module/UW/SUNNode set timer_buffer_                 $opt(buffer_period);

Module/UW/SUNSink set periodPoissonTraffic_	    0.1
Module/UW/SUNSink set t_probe			    600

# BPSK              
Module/MPhy/BPSK  set BitRate_          $opt(bitrate)
Module/MPhy/BPSK  set TxPower_          $opt(txpower)

# GM3DMOBILITY
Position/UWGM set xFieldWidth_		5000
Position/UWGM set yFieldWidth_		5000
Position/UWGM set yFieldWidth_		200
Position/UWGM set alpha_		0.90
Position/UWGM set alphaPitch_		0.90
Position/UWGM set directionMean_	0
Position/UWGM set pitchMean_		0
Position/UWGM set sigmaPitch_		0
Position/UWGM set updateTime_		5

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    global channel propagation data_mask ns cbr position node udp portnum ipr ipif channel_estimator
    global phy posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global node_coordinates
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

    set cbr($id)  [new Module/UW/CBR] 
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/SUNNode]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/CSMA_ALOHA] 
    set phy($id)  [new Module/MPhy/BPSK] 

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
    
    set position($id) [new "Position/UWGM"]
    $position($id) bound "REBOUNCE"
    $position($id) speedMean $opt(speedMean)
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)
    
    set interf_data($id) [new "MInterference/MIV"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    $phy($id) setPropagation $propagation
    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
    $mac($id) $opt(ack_mode)
    $mac($id) initialize
}

proc createSink { } {

    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set cbr_sink($cnt)  [new Module/UW/CBR] 
    }
    set udp_sink       [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/SUNSink]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/CSMA_ALOHA]
    set phy_data_sink  [new Module/MPhy/BPSK]  

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
    $ipr_sink setnumberofnodes $opt(nn)

    set position_sink [new "Position/UWGM"]
    $position_sink bound "REBOUNCE"
    $position_sink speedMean $opt(speedMean)
    $node_sink addPosition $position_sink
    set posdb_sink [new "PlugIn/PositionDB"]
    $node_sink addPlugin $posdb_sink 20 "PDB"
    $posdb_sink addpos [$ipif_sink addr] $position_sink

    set interf_data_sink [new "MInterference/MIV"]
    $interf_data_sink set maxinterval_ $opt(maxinterval_)
    $interf_data_sink set debug_       0

    $phy_data_sink setSpectralMask $data_mask
    $phy_data_sink setInterference $interf_data_sink
    $phy_data_sink setPropagation $propagation

    $mac_sink $opt(ack_mode)
    $mac_sink initialize
}

#################
# Node Creation #
#################
# Initialize Nodes and Sink
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

# Setup positions
$position(0) setX_ 0
$position(0) setY_ 0
$position(0) setZ_ -100

$position(1) setX_ 500
$position(1) setY_ 500
$position(1) setZ_ -100

$position_sink setX_ 1000
$position_sink setY_ 1000
$position_sink setZ_ -100

for {set id 0} {$id < $opt(nn)} {incr id}  {
    $ipr($id) initialize
}
$ipr_sink initialize

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)    "$cbr($id1) start"
    $ns at $opt(stoptime)     "$cbr($id1) stop"
}
# The sink starts to send probes to notify its presence
$ns at [expr $opt(starttime) + 1]	"$ipr_sink  start"
$ns at $opt(stoptime)	    		"$ipr_sink  stop"

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt outfile
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global node_coordinates
    global ipr_sink ipr ipif udp cbr phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats

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
    set total_energy           0.0
    set sum_ftt                ""
    set sum_fttstd             ""
    set distances              ""
    set sum_ipr_retx           0.0
    set ipr_retx               0
    set first_check_ftt        1
    set first_check_ftt_std    1

    for {set i 0} {$i < $opt(nn)} {incr i}  {
        set cbr_throughput           [$cbr_sink($i) getthr]
        set cbr_sent_pkts        [$cbr($i) getsentpkts]
        set cbr_rcv_pkts           [$cbr_sink($i) getrecvpkts]
        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
    }
    
    puts "Node Stats"
    for {set i 0} {$i < $opt(nn)} {incr i}  {
	set cbr_throughput      [$cbr_sink($i) getthr]
        set cbr_sent_pkts       [$cbr($i) getsentpkts]
        set cbr_rcv_pkts        [$cbr_sink($i) getrecvpkts]
        set cbr_per             [$cbr_sink($i) getper]
        set cbr_ftt             [$cbr_sink($i) getftt]
        set cbr_fttstd          [$cbr_sink($i) getfttstd]
        set ipr_retx            [$ipr($i) getmeanretx]
        for {set j 0} {$j < $opt(nn)} {incr j} {
            set tmp_node_stats($j)    [$ipr($i) getstats $j]
        }
        set node_stats($i) "$tmp_node_stats(0)    $tmp_node_stats(1)"

        for {set j 0} {$j < $opt(nn)} {incr j} {
            set tmp_sink_stats($j) [$ipr_sink getstats $i $j]
        }
        set sink_stats($i) "$tmp_sink_stats(0)    $tmp_sink_stats(1)"
        if ($opt(verbose)) {
            puts "node($i) throughput: $cbr_throughput"
            puts "node($i) per:        $cbr_per"
            puts "node($i) sent:       -> $cbr_sent_pkts"
            puts "node($i) received:   <- $cbr_rcv_pkts"
            puts "node($i) ftt:        $cbr_ftt"
            puts "node($i) ftt std:    $cbr_fttstd"
            puts "---------------------------------------"
        }

        if ($first_check_ftt) {
            set sum_ftt "$cbr_ftt"
            set first_check_ftt 0
        } else {
            set sum_ftt "$sum_ftt    $cbr_ftt"
        }
        
        if ($first_check_ftt_std) {
            set sum_ftt_std "$cbr_fttstd"
            set first_check_ftt_std 0
        } else {
            set sum_ftt_std "$sum_ftt_std    $cbr_fttstd"
        }
        
        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts   [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
        set sum_ipr_retx       [expr $sum_ipr_retx + $ipr_retx]
    }
        
    set probecount          [$ipr_sink getprobepktcount]
    set ackcountnode        [$ipr(0) getackcount]
    set ackcountsink        [$ipr_sink getackcount]
    set datacount           [$ipr(0) getdatapktcount]
    set forwardcount        [$ipr(0) getforwardedcount]
    set dropcountretx       [$ipr(0) getdatapktdroppedmaxretx]
    set dropcountbuffer     [$ipr(0) getdatapktdroppedbuffer]
    set pathestcount        [$ipr(0) getpathestablishmentpktcount]
    set probeheadersize     [$ipr_sink getprobepktheadersize]
    set ackheadersize       [$ipr(0) getackheadersize]
    set dataheadersize      [$ipr(0) getdatapktheadersize]
    set pathestheadersize   [$ipr(0) getpathestheadersize]
    set ipheadersize        [$ipif(0) getipheadersize]
    set udpheadersize       [$udp(0) getudpheadersize]
    set cbrheadersize       [$cbr(0) getcbrheadersize]
    set totaldata           [expr $probecount*($probeheadersize + $ipheadersize) + ($ackcountnode+$ackcountsink)*($ackheadersize + $ipheadersize) + $pathestcount*($pathestheadersize + $ipheadersize) + $datacount*($cbrheadersize + $dataheadersize + $opt(pktsize) + $udpheadersize + $ipheadersize)]
    set totaldata_clean     [expr $probecount*($probeheadersize + $ipheadersize) + ($ackcountnode+$ackcountsink)*($ackheadersize + $ipheadersize) + $pathestcount*($pathestheadersize + $ipheadersize) + ($datacount - $forwardcount)*($cbrheadersize + $dataheadersize + $opt(pktsize) + $udpheadersize + $ipheadersize)]
    set control_bit         [expr $probecount*($probeheadersize + $ipheadersize) + ($ackcountnode+$ackcountsink)*($ackheadersize + $ipheadersize) + $pathestcount*($pathestheadersize + $ipheadersize) + ($datacount)*($cbrheadersize + $udpheadersize + $dataheadersize + $ipheadersize)]
    set usefuldata          [expr $sum_cbr_rcv_pkts * $opt(pktsize)]

    
    puts "Metrics"
    puts "Mean Throughput          : [expr (double($sum_cbr_throughput)/($opt(nn)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts"
    puts "Received Packets         : $sum_cbr_rcv_pkts"
    puts "Packet Error Rate        : [expr ((1 - double($sum_cbr_rcv_pkts)/ $sum_cbr_sent_pkts) * 100)]"
    puts "Number of probe pkts     : $probecount"
    puts "Number of ack pkts       : [expr $ackcountnode + $ackcountsink]"
    puts "Number of data pkts      : $datacount"
    puts "Number of max_retx drop  : $dropcountretx"
    puts "Number of buf_full drop  : $dropcountbuffer"
    puts "Number of path est pkts  : $pathestcount"
    puts "Probe Header Size        : $probeheadersize"
    puts "Ack Header Size          : $ackheadersize"
    puts "Data Pkt Header Size     : $dataheadersize"
    puts "Path Est Pkt Header Size : $pathestheadersize"
    puts "IP Pkt Header Size       : $ipheadersize"
    puts "UDP Header Size          : $udpheadersize"
    puts "CBR Header Size          : $cbrheadersize"
    puts "Total Data               : $totaldata"
    puts "Useful bytes received    : $usefuldata"
    
    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run
