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
# Author: Sara Falleni <falleni.s@northeastern.edu>
# Version: 1.0.0
# NOTE: tcl sample tested on Ubuntu 20.04, 64 bits OS
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
#   |  2. UW/SMART_OFDM        |   |  2. UW/SMART_OFDM        |   |  2. UW/SMART_OFDM        |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  1. Module/PHY/OFDM      |   |  1. Module/PHY/OFDM      |   |  1. Module/PHY/OFDM     |
#   +--------------------------+   +--------------------------+   +--------------------------+
#            |         |                    |         |                   |         |       
#   +----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                  |
#   +----------------------------------------------------------------------------------------+

# @todo: Check parameters of the sink

######################################
# Flags to enable or disable options #
######################################
set opt(trace_files)        0
set opt(bash_parameters)    1

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
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libmsgdisplay.so
load libuwofdmphy.so
load libuwinterferenceofdm.so
load libuwsmartofdm.so


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
set opt(start_clock) [clock seconds]

set opt(nn)                 5 ;# Number of Nodes
set opt(starttime)          1  ;# simulation start time, default time start 1
set opt(stoptime)           5000 ;# stop time in sec previously 100000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(seedcbr)            1  ;# cbr seed value
set opt(trace_files)          0  ;

set opt(maxinterval_)       20.0  ;# max interval between two packets. upperlimit
set opt(pktsize)            1536;
set opt(freq)               125000.0 ;# frequency
set opt(bw)                 125000.0 ;# bandwidth
set opt(bitrate)            64000.0  ;# bitrate or transmission rate
set opt(subCarrier)         10  ;# used to define number of sub carrier. But not used as having trouble setting the value.
set opt(subCarSize)         [expr $opt(bw)/$opt(subCarrier) ]     ;# used to define the bandwidth of each subcarrier
set opt(ack_mode)           setNoAckMode
set opt(filename)           "expLog.xls"
set opt(init_mode)          "f"
set opt(modulation)         "BPSK"
set opt(AckInBand)          0
set opt(ctrlCar)            2
set opt(max_burst_size)     3
set opt(interfNodeEnabled)  0
set opt(dpktsize)           100
set opt(dpktcbr)            0.25
set opt(macdebug)           0
set opt(fullBand)           0


set opt(txpower)            130.0  ;# Transmitting sensitivity 130dB ±3dB re 1µPa/V at 1m at 100kHz TC4013

set rng [new RNG]
set rng_position [new RNG]

# this block is used for defining some of the simulation variable value by commandline
# argument. If no argument is found some default value is set. 
if {$opt(bash_parameters)} {
    if {$argc != 13} {
        puts "The script requires 13 inputs:"
        puts "- the first is the cbr period;"
        puts "- the second is the filename;"
        puts "- the third is the num of nodes"
        puts "- the fourth is the initialization mode of the subcarriers fullBand or Not"
        puts "- the fifth is the modulation"
        puts "- the sixth is AckInBand value"
        puts "- the seventh is the packet size value"
        puts "- the eighth is the number of timeslots kept value"
        puts "- the nineth is the length of each timeslot"
        puts "- the tenth is the max number of carriers reserved each time"
        puts "- the eleventh n of timeslots reserved each time"
        puts "- the twelveth min num of packets queued to ask for bandwidth"
        puts "- the thirteen is the aCK mode"
        puts "example: ns test_uwcbr_smartofdm.tcl 10 test.csv 5 0 BPSK 1 384 10 0.05 10 1 5 $opt(ack_mode)"
        puts "Please try again."
        return
    } else {
        set opt(cbr_period)    [lindex $argv 0]
        set opt(filename)      [lindex $argv 1]
        set opt(nn)            [lindex $argv 2] 
        set opt(subCarSize)    [expr $opt(bw)/$opt(subCarrier) ] 
        set opt(fullBand)      [lindex $argv 3]
        set opt(modulation)    [lindex $argv 4]
        set opt(AckInBand)     [lindex $argv 5]
        set opt(pktsize)       [lindex $argv 6]
        set opt(timeslots)     [lindex $argv 7]
        set opt(tslots_len)    [lindex $argv 8]
        set opt(car_reserved)  [lindex $argv 9]
        set opt(req_tslots)    [lindex $argv 10]
        set opt(max_burst_size) [lindex $argv 11]
        set opt(ack_mode)      [lindex $argv 12]
    }
} else {
    # default values for traffic generation and transmission
    set opt(pktsize)    1536
    set opt(cbr_period) 30
    set opt(seedcbr)	3
    set opt(timeslots)          10; #tslots kept in the occupancy table           
    set opt(tslots_len)         2; #length in seconds of each timeslot
    set opt(car_reserved)       2; #carriers reserved for each exchange
    set opt(req_tslots)         1; #tslots required for each exchange
    set opt(max_burst_size)          1; #pkt batch needed to ask for bandwidth
}
if {$opt(fullBand) == 1} {
    set opt(ctrlCar) 0
}



set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng

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
Module/UW/CBR set drop_out_of_order_ 0

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
Module/UW/OFDM/PHY  set powerScaling_               1; #to scale power with carriers 
Module/UW/OFDM/PHY  set debug_                      0
Module/UW/SMART_OFDM  set uwsmartofdm_debug_        $opt(macdebug)
Module/UW/SMART_OFDM set print_transitions_         $opt(macdebug)
Module/UW/SMART_OFDM  set DATA_size_                $opt(pktsize)
Module/UW/SMART_OFDM set bitrateCar_                [expr $opt(bitrate)/$opt(subCarrier)]
Module/UW/SMART_OFDM  set timeslots_                $opt(timeslots)
Module/UW/SMART_OFDM  set timeslot_length_          $opt(tslots_len)
Module/UW/SMART_OFDM  set max_car_reserved_         $opt(car_reserved)
Module/UW/SMART_OFDM  set req_tslots_               $opt(req_tslots)
Module/UW/SMART_OFDM  set max_burst_size_           $opt(max_burst_size)
Module/UW/SMART_OFDM  set fullBand_                 $opt(fullBand)
# Module/UW/SMART_OFDM  set max_rts_tries_            5




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
        puts "ns node creating " 
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]



    set cbr($id)  [new Module/UW/CBR] 
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/SMART_OFDM] 
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
    $mac($id) initialize
    $mac($id) init_macofdm_node $opt(subCarrier) $opt(subCarSize) $opt(ctrlCar) $opt(modulation)
    $cbr($id) set seedcbr_ $id
    $mac($id) set MAC_addr_ $id
    

}

########################################################
# Procedure for creating the sink node # 
########################################################


proc createSink { } {

    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set cbr_sink($cnt)  [new Module/UW/CBR] 
    }

    set udp_sink       [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/SMART_OFDM]
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

    $mac_sink $opt(ack_mode)
    $mac_sink initialize
    $mac_sink init_macofdm_node $opt(subCarrier) $opt(subCarSize) $opt(ctrlCar) $opt(modulation)
    $mac_sink set MAC_addr_ 254
    }

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}
Module/UW/CBR set period_              $opt(stoptime) 
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
    global ns opt bindex
    global mac propagation cbr_sink cbr_sink mac_sink phy_data phy_data_sink phy_data_fsink channel db_manager propagation id_s
    global node_coordinates bnum mnum
    global ipr_sink ipr ipif udp cbr phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats netdex

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
    puts "carriers mode      : $opt(init_mode)"

    set sum_cbr_throughput     0
    set sum_per                0
    set sum_cbr_sent_pkts      0
    set sum_cbr_rcv_pkts       0.0  
    set sum_cbr_rcv_pkts_fsink       0.0     
    set sum_cbr_rcv_pkts_fsink_base       0.0 
    set sum_cbr_rcv_pkts_fsink_mid       0.0 
    set sum_tx_power_consumed      0.0
    set sum_rx_power_consumed      0.0
    set sum_ftt       0.0
    set sum_txtime 0.0
    set sum_phy_sent_pkt 0 
    set sum_cbr_sent_pkts_sink 0 

    for {set i 0} {$i < $opt(nn)} {incr i}  {
        # set cbr_throughput              [$cbr_sink($id_s(0),$i) getthr]
        set cbr_sent_pkts               [$cbr($i) getsentpkts]
        set cbr_rcv_pkts_fsink                [$cbr_sink($i) getrecvpkts]
        set ftt_at_sink                 [$cbr_sink($i) getftt]
        set txtime_at_sink              [$cbr_sink($i) gettxtime]
        set tx_power_consumed           [$phy($i) getConsumedEnergyTx]
        set rx_power_consumed           [$phy($i) getConsumedEnergyRx]
        # set sum_cbr_throughput          [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts           [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts_fsink_base            [expr $sum_cbr_rcv_pkts_fsink_base + $cbr_rcv_pkts_fsink]
        set sum_tx_power_consumed       [expr $sum_tx_power_consumed + $tx_power_consumed]
        set sum_rx_power_consumed       [expr $sum_rx_power_consumed + $rx_power_consumed]
        set sum_ftt                     [expr $sum_ftt + $ftt_at_sink]
        set sum_txtime                  [expr $sum_txtime + $txtime_at_sink]
        # puts -nonewline $fileIDtemp                " $ftt_at_sink "
        puts "Packets received on $i interface = $cbr_rcv_pkts_fsink"
    }

    set sum_cbr_rcv_pkts_fsink   $sum_cbr_rcv_pkts_fsink_base
        
    set ipheadersize        [$ipif(1) getipheadersize]
    set udpheadersize       [$udp(1) getudpheadersize]
    set cbrheadersize       [$cbr(1) getcbrheadersize]
    
    set delay 0
    puts "                                                                     "
    puts "---------------------------------------------------------------------"
    puts "SINK APP LEVEL STATISTICS "
        puts "Average ftt              : [expr $sum_ftt / ($opt(nn))]"
        puts "Mean Throughput          : [expr ($sum_cbr_throughput/($opt(nn)))]"
        puts "Sent Packets             : $sum_cbr_sent_pkts"
        puts "Sent Packets by Sink     : $sum_cbr_sent_pkts_sink"
        puts "Received Packets m_sink  : $sum_cbr_rcv_pkts"
        puts "Received Packets fsink   : $sum_cbr_rcv_pkts_fsink"
        puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts_fsink / ($sum_cbr_sent_pkts + $sum_cbr_sent_pkts_sink)* 100]"
        puts "IP Pkt Header Size       : $ipheadersize"
        puts "UDP Header Size          : $udpheadersize"
        puts "CBR Header Size          : $cbrheadersize"
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 0]  "finish; $ns halt" 
$ns run
