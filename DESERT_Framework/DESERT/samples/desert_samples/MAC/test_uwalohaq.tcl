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
#
#
# Authors: Aleksa Albijanic
# Version: 1.0.0
#
# This script is used to test UW-ALOHA_Q protocol - papaer reference below
# A. Albijanic, S. Tomovic and I. Radusinovic, "Simulation Analysis of the 
# Impact of Underwater Channel Reliability on Machine Learning-Optimized 
# Framed-Aloha MAC protocols," 
# 2024 28th International Conference on Information Technology (IT), 
# Zabljak, Montenegro, 2024, pp. 1-4,
# doi: 10.1109/IT61232.2024.10475776.
#
# NOTE: tcl sample tested on Ubuntu 22.04, 64 bits OS
#
# Stack of the nodes
#   +-------------------------+ 
#   |  7. UW/CBR              |
#   +-------------------------+
#   |  6. UW/UDP              |
#   +-------------------------+
#   |  5. UW/STATICROUTING    |
#   +-------------------------+
#   |  4. UW/IP               |
#   +-------------------------+
#   |  3. UW/MLL              |
#   +-------------------------+
#   |  2. UW/ALOHAQ_NODE      |
#   +-------------------------+
#   |  1. UW/PHYSICAL         |
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |   UnderwaterChannel     |
#   +-------------------------+
#
# Stack of the sink
#
#   +-------------------------+
#   |  7. UW/CBR              |
#   +-------------------------+
#   |  6. UW/UDP              |
#   +-------------------------+
#   |  5. UW/STATICROUTING    |
#   +-------------------------+
#   |  4. UW/IP               |
#   +-------------------------+
#   |  3. UW/MLL              |
#   +-------------------------+
#   |  2. UW/ALOHAQ_SINK      |
#   +-------------------------+
#   |  1. UW/PHYSICAL         |
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |   UnderwaterChannel     |
#   +-------------------------+
#
#
#   Nodes are randolmy distributed under the floating sink for the given maximal distance
#
#
######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 		1
set opt(trace_files)		1
set opt(bash_parameters) 	0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleWirelessCh.so 
load libMiracleBasicMovement.so
load libuwip.so
load libuwstaticrouting.so
load libmphy.so
load libmmac.so
load libuwmmac_clmsgs.so
load libuwphy_clmsgs.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwalohaq.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwmulti_stack_controller.so
load libuwmulti_traffic_control.so
load libuwhmmphysical.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(nn)                 10;# Number of Nodes
set opt(starttime)          0;
set opt(stoptime)           1500;
set opt(max_dist)	    500.0;#Maximal distance between sensor nodes and sink
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation
set opt(txpower)            173;#Power transmitted in dB re uPa 185.8 is the maximum
set opt(propagation_speed)  1500.0;# m/s
set opt(guard_time) 	    0.004;
set opt(maxinterval_)       20
set opt(freq)               25000.0 ;#Central frequency used for data channel in Hz
set opt(bw)                 4000.0 ;#Bandwidth used for data channel in Hz
set opt(freq_ack)           22500.0 ;#Central frequency used for ACK channel in Hz
set opt(bw_ack)             1000.0 ;#Bandwidth used for ACK channel in Hz
set opt(bitrate)            9200; #bitrate in bps
set opt(cbr_period)         0.01;
set opt(ack_size)	    20;
set opt(pktsize)  	    130.5;
set opt(indexB) 	    1.5

set opt(rngstream)	    1

if {$opt(bash_parameters)} {
	if {$argc != 2} {
		puts "The script requires two inputs:"
		puts "- the first for the maximal distance from sensor node to sink node"
		puts "- the second one is for backoff mode - 0 to backoff after"
		puts " every retransmission, and 1 after favorable slot change"
		puts "example: ns test_uwalohaq.tcl 500 1"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
		return
	} else {
		set opt(rngstream)    [lindex $argv 0]
		set opt(cbr_period)   [lindex $argv 1]
	}
}

global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_alohaq_simple.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_alohaq_simple.cltr"
	set opt(cltracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}

### Channel ###
MPropagation/Underwater set practicalSpreading_ 2
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          1
MPropagation/Underwater set shipping_           0.2


set channel [new Module/UnderwaterChannel]
set ack_channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq              $opt(freq)
$data_mask setBandwidth         $opt(bw)
$data_mask setPropagationSpeed  $opt(propagation_speed)
set data_mask_ack [new MSpectralMask/Rect]
$data_mask_ack setFreq              $opt(freq_ack)
$data_mask_ack setBandwidth         $opt(bw_ack)
$data_mask_ack setPropagationSpeed  $opt(propagation_speed)


set phy_data_tag "PHY"
set phy_ack_tag "APHY"

set runi [new RandomVariable/Uniform]
$runi set min_ 0.0
$runi set max_ 1.0
$runi use-rng $defaultRNG

#########################
# Module Configuration  #
#########################
### APP ###
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set debug_               0

set T_dp [expr $opt(pktsize) * 8 / $opt(bitrate)]
set t_p [expr $opt(max_dist) / $opt(propagation_speed)]
set T_ap [expr $opt(ack_size) * 8 / $opt(bitrate)]

set slot_dur [expr $T_dp + $T_ap + $opt(guard_time) + 2 * $t_p]

set total_slots [expr ceil(($opt(nn) * $opt(indexB) * $T_dp / (2 * $t_p + $T_dp)))]


### TDMA MAC ###
Module/UW/ALOHAQ_NODE set debug_ 	       0
Module/UW/ALOHAQ_NODE set HDR_size_ 	       0
Module/UW/ALOHAQ_NODE set wait_constant_       0.1
Module/UW/ALOHAQ_NODE set tot_slots            $total_slots
Module/UW/ALOHAQ_NODE set max_packet_per_slot  2
Module/UW/ALOHAQ_NODE set slot_duration        $slot_dur
Module/UW/ALOHAQ_NODE set queue_size_          10
Module/UW/ALOHAQ_NODE set mac2phy_delay_       [expr 1.0e-9]
Module/UW/ALOHAQ_NODE set sea_trial_ 	       1
Module/UW/ALOHAQ_NODE set start_time 	       $opt(starttime)
Module/UW/ALOHAQ_NODE set backoff_mode         1 


Module/UW/ALOHAQ_SINK set sea_trial_ 	       1
Module/UW/ALOHAQ_SINK set debug_ 	       0
Module/UW/ALOHAQ_SINK set HDR_size_ 	       0
Module/UW/ALOHAQ_SINK set ACK_size_ 	       2
Module/UW/ALOHAQ_SINK set wait_constant_       0.1
Module/UW/ALOHAQ_SINK set max_packet_per_slot  1
Module/UW/ALOHAQ_SINK set max_queue_size_      0
Module/UW/ALOHAQ_SINK set mac2phy_delay_       [expr 1.0e-9]


Module/UW/HMMPHYSICAL  set TxPower_                    $opt(txpower)
Module/UW/HMMPHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/HMMPHYSICAL  set AcquisitionThreshold_dB_    15.0 
Module/UW/HMMPHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/HMMPHYSICAL  set TxSPLMargin_dB_             0
Module/UW/HMMPHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/HMMPHYSICAL  set MinTxSPL_dB_                5
Module/UW/HMMPHYSICAL  set MaxTxRange_                 1500
Module/UW/HMMPHYSICAL  set PER_target_                 0    
Module/UW/HMMPHYSICAL  set CentralFreqOptimization_    0
Module/UW/HMMPHYSICAL  set BandwidthOptimization_      0
Module/UW/HMMPHYSICAL  set SPLOptimization_            0
Module/UW/HMMPHYSICAL  set ConsumedEnergy_             0
Module/UW/HMMPHYSICAL  set NoiseSPD_                   0
Module/UW/HMMPHYSICAL  set debug_                      0



### PHY ###
Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    15.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 1500
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    global channel ns cbr position node udp portnum ipr ipif phy_data_tag phy_ack_tag runi
    global phy_data opt mll mac propagation data_mask interf_data data_mask_ack phy_ack ack_channel
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr($id)  [new Module/UW/CBR] 
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/ALOHAQ_NODE]
    #set phy_data($id) [new Module/UW/HMMPHYSICAL] #uncomment and comment last 2 lines if HMM PHYSICAL is used
    #set phy_ack($id)  [new Module/UW/HMMPHYSICAL] 
    set phy_data($id) [new Module/UW/PHYSICAL]  
    set phy_ack($id)  [new Module/UW/PHYSICAL]
    
    $node($id) addModule 8 $cbr($id)   1  "CBR"
    $node($id) addModule 7 $udp($id)   1  "UDP"
    $node($id) addModule 6 $ipr($id)   1  "IPR"
    $node($id) addModule 5 $ipif($id)  1  "IPF"   
    $node($id) addModule 4 $mll($id)   1  "MLL"
    $node($id) addModule 3 $mac($id)   1  "MAC"
    $node($id) addModule 2 $phy_data($id)   1  $phy_data_tag
    $node($id) addModule 1 $phy_ack($id)   1  $phy_ack_tag

    $node($id) setConnection $cbr($id)   $udp($id)   1
    $node($id) setConnection $udp($id)   $ipr($id)   1
    $node($id) setConnection $ipr($id)   $ipif($id)  1
    $node($id) setConnection $ipif($id)  $mll($id)   1
    $node($id) setConnection $mll($id)   $mac($id)   1
    $node($id) setConnection $mac($id)   $phy_data($id)   1
    $node($id) setConnection $mac($id)   $phy_ack($id)   1
    $node($id) addToChannel  $channel    $phy_data($id)   1
    $node($id) addToChannel  $ack_channel    $phy_ack($id)   1
    
    set portnum($id) [$udp($id) assignPort $cbr($id) ]
    	if {$id > 254} {
    		puts "Too many nodes"
    	exit
    }

    #Set the IP address of the node
    $ipif($id) addr [expr $id + 1]
    
    # Set the MAC address
    $mac($id) setMacAddr [expr $id + 3]
    
    #Setup positions
    
    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
   
    	
    	set dist [expr sqrt($opt(max_dist) * $opt(max_dist) / 6)]
    	set random_x [$runi value]
    	set random_y [$runi value]
    	set random_z [$runi value]
    	$position($id) setX_ [expr ($random_x * 2 * $dist) - $dist]
   	$position($id) setY_ [expr ($random_y * 2 * $dist) - $dist]
   	$position($id) setZ_ [expr (((-1) * 2 * $dist) * $random_z)]
   		
   	set x [expr ($random_x * 2 * $dist) - $dist]
   	set y [expr ($random_y * 2 * $dist) - $dist]
   	set z [expr (((-1) * 2 *$dist) * $random_z)]
   		
   	puts "node ID $id positon is X----> $x , Y-----> $y , Z-----> $z "
    	
    
    $mac($id) setStartTime    0
    
    $mac($id) setPhyDataTag $phy_data_tag

    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0
    
    set interf_data_ack($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data_ack($id) set maxinterval_ $opt(maxinterval_)
    $interf_data_ack($id) set debug_       0

    #Propagation model
    $phy_data($id) setPropagation $propagation 
    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setInterferenceModel "MEANPOWER"
    
    $phy_ack($id) setPropagation $propagation 
    $phy_ack($id) setSpectralMask $data_mask_ack
    $phy_ack($id) setInterference $interf_data_ack($id)
    $phy_ack($id) setInterferenceModel "MEANPOWER"
    
    #$mac($id) initialize
}


proc createSink { } {

    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink data_mask_ack phy_data_tag phy_ack_tag
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator phy_data_sink phy_ack_sink ack_channel

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set cbr_sink($cnt)  [new Module/UW/CBR] 
    }
    set udp_sink       [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/ALOHAQ_SINK]
    #set phy_data_sink  [new Module/UW/HMMPHYSICAL] #uncomment and comment last 2 lines if HMM PHYSICAL is used
    #set phy_ack_sink   [new Module/UW/HMMPHYSICAL] 
    set phy_data_sink [new Module/UW/PHYSICAL]  
    set phy_ack_sink  [new Module/UW/PHYSICAL] 

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node_sink addModule 8 $cbr_sink($cnt) 0 "CBR"
    }
    $node_sink addModule 7 $udp_sink       1 "UDP"
    $node_sink addModule 6 $ipr_sink       1 "IPR"
    $node_sink addModule 5 $ipif_sink      1 "IPF"   
    $node_sink addModule 4 $mll_sink       1 "MLL"
    $node_sink addModule 3 $mac_sink       1  "MAC"
    $node_sink addModule 2 $phy_data_sink   1  $phy_data_tag
    $node_sink addModule 1 $phy_ack_sink   1  $phy_ack_tag

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node_sink setConnection $cbr_sink($cnt)  $udp_sink      1   
    }
    $node_sink setConnection $udp_sink  $ipr_sink            1
    $node_sink setConnection $ipr_sink  $ipif_sink           1
    $node_sink setConnection $ipif_sink $mll_sink            1 
    $node_sink setConnection $mll_sink  $mac_sink            1
    $node_sink setConnection $mac_sink  $phy_data_sink       1
    $node_sink setConnection $mac_sink  $phy_ack_sink        1
    $node_sink addToChannel  $channel   $phy_data_sink       1
    $node_sink addToChannel  $ack_channel   $phy_ack_sink    1

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set portnum_sink($cnt) [$udp_sink assignPort $cbr_sink($cnt)]
        if {$cnt > 252} {
            puts "hostnum > 252!!! exiting"
            exit
        }    
    }
    
    $mac_sink setPhyAckTag $phy_ack_tag
    
    $ipif_sink addr 253
    
    #Setup positions
    
    set position_sink [new "Position/BM"]
    $node_sink addPosition $position_sink
    
    set dist [expr $opt(max_dist) / 1.41]
    
    $position_sink setX_ 0
    $position_sink setY_ 0
    $position_sink setZ_ 0
  

    set interf_data_sink [new "Module/UW/INTERFERENCE"]
    $interf_data_sink set maxinterval_ $opt(maxinterval_)
    $interf_data_sink set debug_       0

    $phy_data_sink setSpectralMask $data_mask
    $phy_data_sink setInterference $interf_data_sink
    $phy_data_sink setPropagation $propagation
    $phy_data_sink setInterferenceModel "MEANPOWER"
    
    $phy_ack_sink setSpectralMask $data_mask_ack
    $phy_ack_sink setInterference $interf_data_sink
    $phy_ack_sink setPropagation $propagation
    $phy_ack_sink setInterferenceModel "MEANPOWER"
    

    $mac_sink setMacAddr 50
    #$mac_sink initialize
    puts "Sink mac is $mac_sink "
}



#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
    puts "Node $id created"
}
createSink


##################################
# Setup MCLinks if using HMM PHYSICAL model
##################################

#Please note that code is written as reference to new BER-based HMM module

#set mclink1 [new Module/UW/HMMPHYSICAL/MCLINK/EXTENDED 0.00006 0.0002 0.0005 0.04 0.125 0.314 0.005 0.09 0.32 300 BAD]

#for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
#            $phy_data($id1) setMCLink [$mac_sink addr] $mclink1 
#            $phy_ack($id1) setMCLink [$mac_sink addr] $mclink1    
#            $phy_ack_sink setMCLink [$mac($id1) addr] $mclink1    
#            $phy_data_sink setMCLink [$mac($id1) addr] $mclink1  
#}


################################
# Inter-node module connection #
################################
proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink opt 
    
    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    $cbr_sink($id1) set destPort_ $portnum($id1)
}

##################
# Setup flows    #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    connectNodes $id1
}

###################
# Fill ARP tables #
###################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
	$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }
    $mll($id1) addentry [$ipif_sink addr] [$mac_sink addr]
    $mll_sink addentry [$ipif($id1) addr] [$mac($id1) addr]
}



########################
# Setup routing tables #
########################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
            $ipr($id1) addRoute [$ipif($id2) addr] [$ipif($id2) addr]
        }
    $ipr($id1) addRoute [$ipif_sink addr] [$ipif_sink addr]
    $ipr_sink addRoute [$ipif($id1) addr] [$ipif($id1) addr]
    }
}


#####################
# Start/Stop Timers #
#####################

# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {	
	    $ns at $opt(starttime)    "$cbr($id1) start"
	    $ns at $opt(stoptime)     "$cbr($id1) stop"  
}

for {set ii 0} {$ii < $opt(nn)} {incr ii} {
    $ns at $opt(starttime)    "$mac($ii) start"
    $ns at $opt(stoptime)     "$mac($ii) stop"
}
###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt outfile
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global node_coordinates
    global ipr_sink ipr ipif udp cbr phy phy_data_sink phy_ack phy_ack_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats

    set sum_cbr_throughput 	0
    set sum_per		0
    set sum_mac_sent_pkts	0.0
    set sum_mac_rcv_pkts	0.0


	
    for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
	set mac_tx_pkts($id3)	   [$mac($id3) getDataPktsTx]
	set mac_rx_pkts($id3)	    [$mac($id3) getDataPktsRx]
	
	set mac_sink_recv_pkts       [$mac_sink getDataPktsRx]
	set mac_sink_sent_pkts       [$mac_sink getDataPktsTx]
	
	set sum_mac_sent_pkts [expr $sum_mac_sent_pkts + $mac_tx_pkts($id3)]
	set sum_mac_rcv_pkts [expr $sum_mac_rcv_pkts + $mac_rx_pkts($id3)]

    }
    
    if ($opt(verbose)) {
       puts "-----------------------------------------------------------------"
       puts "Simulation summary"
       puts "-----------------------------------------------------------------"
       puts "Total simulation time    : [expr $opt(stoptime)-$opt(starttime)] s"
       puts "Number of nodes          : $opt(nn)"
       puts "Packet size              : $opt(pktsize) byte(s)"
       puts "-----------------------------------------------------------------"
       
       puts "sink uccessfully received       : $mac_sink_recv_pkts  data packets"
       puts "nodes sent combined             : $sum_mac_sent_pkts   data packets"
       set channel_utilization [expr ($sum_mac_rcv_pkts * (($opt(pktsize) * 8) / $opt(bitrate))) / $opt(txduration)]
       puts "channel utilization             : $channel_utilization"
    }
	
    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################
if ($opt(verbose)) {
    puts "\nStarting Simulation\n"
}


$ns at [expr $opt(stoptime) + 50.0]  "finish; $ns halt" 

$ns run
