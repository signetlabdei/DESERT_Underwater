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
# This script is used to test MULTI_STACK_CONTROLLER_PHY MASTER and SLAVE module
# There are 2 nodes that can transmit each other packets with a CBR (Constant 
# Bit Rate) Application Module
# The MASTER controls the switch between three UW/PHYSICAL layers with different
# frequency and bandwidth, according with the received power metrics. The slave 
# switches according to the MASTER behavior. UnderwaterChannel is used as channel.
#
# Author: Filippo Campagnaro <campagn1@dei.unipd.it>
# Version: 1.0.0
#
# NOTE: tcl sample tested on Ubuntu 11.10, 64 bits OS
#
# Stack of the nodes
#                   MASTER                                         SLAVE 
#   +------------------------------------------+   +------------------------------------------+
#   |  10. UW/CBR                              |   |  10. UW/CBR                              |
#   +------------------------------------------+   +------------------------------------------+
#   |  9. UW/UDP                               |   |  9. UW/UDP                               |
#   +------------------------------------------+   +------------------------------------------+
#   |  8. UW/STATICROUTING                     |   |  8. UW/STATICROUTING                     |
#   +------------------------------------------+   +------------------------------------------+
#   |  7. UW/IP                                |   |  7. UW/IP                                |
#   +------------------------------------------+   +------------------------------------------+
#   |  6. UW/MLL                               |   |  6. UW/MLL                               | 
#   +------------------------------------------+   +------------------------------------------+
#   |  5. UW/CSMA_ALOHA                        |   |  5. UW/CSMA_ALOHA                        |
#   +------------------------------------------+   +------------------------------------------+
#   |  4. UW/MULTI_STACK_CONTROLLER_PHY_MASTER |   |  4. UW/MULTI_STACK_CONTROLLER_PHY_SLAVE  |
#   +--------------+-------------+-------------+   +--------------+-------------+-------------+
#   | 3.UW/PHYSICAL|2.UW/PHYSICAL|1.UW/PHYSICAL|   | 3.UW/PHYSICAL|2.UW/PHYSICAL|1.UW/PHYSICAL|
#   +--------------+-------------+-------------+   +--------------+-------------+-------------+ 
#           |             |              |                  |             |            |     
#   +-----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                   |   
#   +-----------------------------------------------------------------------------------------+   

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 			1
set opt(trace_files)		0
set opt(bash_parameters) 	0

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
load libuwcsmaaloha.so
load libuwaloha.so
load libuwinterference.so
load libuwphysical.so
load libuwmulti_stack_controller.so

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
set opt(pktsize)            125  ;# Pkt sike in byte
set opt(starttime)          1	
set opt(stoptime)           100000 
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation

set opt(txpower)            180.0  ;#Power transmitted in dB re uPa


set opt(maxinterval_)       20.0
set opt(freq)               25000.0 ;#Frequency used in Hz
set opt(bw)                 5000.0	;#Bandwidth used in Hz
set opt(freq2)               50000.0 ;#Frequency used in Hz
set opt(bw2)                 9000.0  ;#Bandwidth used in Hz
set opt(freq3)               60000.0 ;#Frequency used in Hz
set opt(bw3)                 9500.0  ;#Bandwidth used in Hz
set opt(bitrate)            4800.0	;#bitrate in bps
set opt(bitrate2)            8800.0  ;#bitrate in bps
set opt(bitrate3)            10800.0  ;#bitrate in bps
set opt(ack_mode)           "setNoAckMode"

set rng [new RNG]
set rng_position [new RNG]

if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the first for the seed"
		puts "- the second one is for the Poisson CBR period"
		puts "- the third one is the cbr packet size (byte);"
		puts "example: ns test_uw_csma_aloha_fully_connected.tcl 1 60 125"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
		return
	} else {
		set opt(seedcbr)    [lindex $argv 0]
		set opt(cbr_period) [lindex $argv 1]
		set opt(pktsize)    [lindex $argv 2]
		$rng seed         $opt(seedcbr)
	}
} else {
	set opt(cbr_period) 60
	set opt(pktsize)	125
	set opt(seedcbr)	1
}

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwcsmaaloha.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwcsmaaloha.cltr"
	set opt(cltracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}

MPropagation/Underwater set practicalSpreading_ 1.8
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)
set data_mask2 [new MSpectralMask/Rect]
$data_mask2 setFreq       $opt(freq2)
$data_mask2 setBandwidth  $opt(bw2)
set data_mask3 [new MSpectralMask/Rect]
$data_mask3 setFreq       $opt(freq3)
$data_mask3 setBandwidth  $opt(bw3)

#########################
# Module Configuration  #
#########################
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set debug_               0

# Module/UW/CSMA_ALOHA set debug_ 1

Module/MPhy/BPSK  set TxPower_               $opt(txpower)

Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 50000
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

# Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE set debug_        1
# Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER set debug_       1
################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

  global channel propagation data_mask data_mask2 data_mask3 ns cbr position node udp portnum ipr ipif channel_estimator
  global phy posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
  global node_coordinates
  
  set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
    if {$id == 0} {
        Module/UW/CBR set period_              [expr $opt(cbr_period)*10]
    } else {
        Module/UW/CBR set period_              $opt(cbr_period)
    }

    set cbr($id,$cnt)  [new Module/UW/CBR] 
		set udp($id,$cnt)  [new Module/UW/UDP]
	}
  set ipr($id)  [new Module/UW/StaticRouting]
  set ipif($id) [new Module/UW/IP]
  set mll($id)  [new Module/UW/MLL] 
  set mac($id)  [new Module/UW/CSMA_ALOHA] 
  # set mac($id)  [new Module/UW/ALOHA] 
  if {$id > 0} {
      set ctr($id)  [new Module/UW/MULTI_STACK_CONTROLLER_PHY_SLAVE]
  } else {
      set ctr($id)  [new Module/UW/MULTI_STACK_CONTROLLER_PHY_MASTER]
  }    
  # set phy($id)  [new Module/MPhy/BPSK]  
  # set phy2($id)  [new Module/MPhy/BPSK] 
  Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
  set phy($id)  [new Module/UW/PHYSICAL] 
  Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate2)
  set phy2($id)  [new Module/UW/PHYSICAL]  
  Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate3)
  set phy3($id)  [new Module/UW/PHYSICAL]  
	
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		$node($id) addModule 10 $cbr($id,$cnt)   1  "CBR"
		$node($id) addModule 9 $udp($id,$cnt)   1  "UDP"
	}
  $node($id) addModule 8 $ipr($id)   1  "IPR"
  $node($id) addModule 7 $ipif($id)  1  "IPF"   
  $node($id) addModule 6 $mll($id)   1  "MLL"
  $node($id) addModule 5 $mac($id)   1  "MAC"
  $node($id) addModule 4 $ctr($id)   1  "CTR"
  $node($id) addModule 3 $phy($id)   1  "PHY"
  $node($id) addModule 2 $phy2($id)   1  "PHY"
  $node($id) addModule 1 $phy3($id)   1  "PHY"

	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		$node($id) setConnection $cbr($id,$cnt)   $udp($id,$cnt)   0
		$node($id) setConnection $udp($id,$cnt)   $ipr($id)   0
		set portnum($id,$cnt) [$udp($id,$cnt) assignPort $cbr($id,$cnt) ]
	}
  $node($id) setConnection $ipr($id)   $ipif($id)  1
  $node($id) setConnection $ipif($id)  $mll($id)   1
  $node($id) setConnection $mll($id)   $mac($id)   1
  $node($id) setConnection $mac($id)   $ctr($id)   1
  $node($id) setConnection $ctr($id)   $phy($id)   1
  $node($id) setConnection $ctr($id)   $phy2($id)  1
  $node($id) setConnection $ctr($id)   $phy3($id)  1
  $node($id) addToChannel  $channel    $phy($id)   1
  $node($id) addToChannel  $channel    $phy2($id)   1
  $node($id) addToChannel  $channel    $phy3($id)   1

  if {$id > 254} {
  	puts "hostnum > 254!!! exiting"
  	exit
  }
  #Set the IP address of the node
  set ip_value [expr $id + 1]
  $ipif($id) addr $ip_value
  
  set position($id) [new "Position/BM"]
  $node($id) addPosition $position($id)
  set posdb($id) [new "PlugIn/PositionDB"]
  $node($id) addPlugin $posdb($id) 20 "PDB"
  $posdb($id) addpos [$ipif($id) addr] $position($id)
  
  #Setup positions
  $position($id) setX_ [expr $id*200]
  $position($id) setY_ [expr $id*200]
  $position($id) setZ_ -100
  
  #Interference model
  set interf_data($id) [new "Module/UW/INTERFERENCE"]
  $interf_data($id) set maxinterval_ $opt(maxinterval_)
  $interf_data($id) set debug_       0
  
  set interf_data2($id) [new "Module/UW/INTERFERENCE"]
  $interf_data2($id) set maxinterval_ $opt(maxinterval_)
  $interf_data2($id) set debug_       0

  set interf_data3($id) [new "Module/UW/INTERFERENCE"]
  $interf_data3($id) set maxinterval_ $opt(maxinterval_)
  $interf_data3($id) set debug_       0
	#Propagation modelpr
  $phy($id) setPropagation $propagation
  $phy2($id) setPropagation $propagation
  $phy3($id) setPropagation $propagation

  $phy($id) setSpectralMask $data_mask
  $phy($id) setInterference $interf_data($id)
  $phy2($id) setSpectralMask $data_mask2
  $phy2($id) setInterference $interf_data2($id)
  $phy3($id) setSpectralMask $data_mask3
  $phy3($id) setInterference $interf_data3($id)
  $mac($id) $opt(ack_mode)
  $mac($id) initialize
  $ctr($id) setManualLowerlId [$phy2($id) Id_]
  $ctr($id) setAutomaticSwitch
  if {$id == 0} {
      #$ctr($id) setManualLowerlId [$phy($id) Id_]
      $ctr($id) addLayer [$phy($id) Id_] 1
      $ctr($id) addLayer [$phy2($id) Id_] 2
      $ctr($id) addLayer [$phy3($id) Id_] 3

      $ctr($id) addThreshold [$phy($id) Id_] [$phy2($id) Id_] 22
      $ctr($id) addThreshold [$phy2($id) Id_] [$phy($id) Id_] 18
      $ctr($id) addThreshold [$phy2($id) Id_] [$phy3($id) Id_] 30
      $ctr($id) addThreshold [$phy3($id) Id_] [$phy2($id) Id_] 28
      #$ctr($id) setManualSwitch
  }
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}


################################
# Inter-node module connection #
################################
proc connectNodes {id1 des1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink opt 

    $cbr($id1,$des1) set destAddr_ [$ipif($des1) addr]
    $cbr($id1,$des1) set destPort_ $portnum($des1,$id1)

    $cbr($des1,$id1) set destAddr_ [$ipif($id1) addr]
    $cbr($des1,$id1) set destPort_ $portnum($id1,$des1) 

}

##################
# Setup flows    #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
		connectNodes $id1 $id2
	}
}

##################
# ARP tables     #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
	}
}



##################
# Routing tables #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
			$ipr($id1) addRoute [$ipif($id2) addr] [$ipif($id2) addr]
	}
}

#Print the routing tables of the nodes
#if {$opt(verbose)} {
#	for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
#		$ipr($id1) printroutes
#	}
#}



#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 1} {$id1 < $opt(nn)} {incr id1}  {
	$ns at $opt(starttime)    "$cbr($id1,0) start"
	$ns at $opt(stoptime)     "$cbr($id1,0) stop"
    $ns at $opt(starttime)    "$cbr(0,$id1) start"
    $ns at $opt(stoptime)     "$cbr(0,$id1) stop"
}

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
    if ($opt(verbose)) {
        puts "---------------------------------------------------------------------"
        puts "Simulation summary"
        puts "number of nodes  : $opt(nn)"
        puts "packet size      : $opt(pktsize) byte"
        puts "cbr period       : $opt(cbr_period) s"
        puts "number of nodes  : $opt(nn)"
        puts "simulation length: $opt(txduration) s"
        puts "tx power         : $opt(txpower) dB"
        puts "tx frequency     : $opt(freq) Hz"
        puts "tx bandwidth     : $opt(bw) Hz"
        puts "bitrate          : $opt(bitrate) bps"
        puts "---------------------------------------------------------------------"
    }
    set sum_cbr_throughput     0
    set sum_per                0
    set sum_cbr_sent_pkts      0.0
    set sum_cbr_rcv_pkts       0.0    

    for {set i 0} {$i < $opt(nn)} {incr i}  {
		for {set j 0} {$j < $opt(nn)} {incr j} {
			set cbr_throughput           [$cbr($i,$j) getthr]
			if {$i != $j} {
				set cbr_sent_pkts        [$cbr($i,$j) getsentpkts]
				set cbr_rcv_pkts           [$cbr($i,$j) getrecvpkts]
			}
			if ($opt(verbose)) {
				puts "cbr($i,$j) throughput                    : $cbr_throughput"
			}
		}
        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
    }
        
    set ipheadersize        [$ipif(1) getipheadersize]
    set udpheadersize       [$udp(1,0) getudpheadersize]
    set cbrheadersize       [$cbr(1,0) getcbrheadersize]
    
    if ($opt(verbose)) {
        puts "Mean Throughput          : [expr ($sum_cbr_throughput/(($opt(nn))*($opt(nn)-1)))]"
        puts "Sent Packets             : $sum_cbr_sent_pkts"
        puts "Received Packets         : $sum_cbr_rcv_pkts"
        puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"
        puts "IP Pkt Header Size       : $ipheadersize"
        puts "UDP Header Size          : $udpheadersize"
        puts "CBR Header Size          : $cbrheadersize"
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
