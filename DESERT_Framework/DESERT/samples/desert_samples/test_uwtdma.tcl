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
# This script is used to test UW-TDMA protocol
# with a CBR (Constant Bit Rate) Application Module
# Here the complete stack used for each node in the simulation
#
# N.B.: UnderwaterChannel and UW/PHYSICAL are used for PHY layer and channel
#
# Authors: Filippo Campagnaro <campagn1@dei.unipd.it>
#          Roberto francescon <frances1@dei.unipd.it>
# Version: 1.0.0
#
# NOTE: tcl sample tested on Ubuntu 11.10, 64/32 bits OS
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
#   |  2. UW/TDMA             |
#   +-------------------------+
#   |  1. UW/PHYSICAL         |
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |   UnderwaterChannel     |
#   +-------------------------+

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
load libuwtdma.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphysical.so

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

set opt(nn)                 3 ;# Number of Nodes
set opt(starttime)          1	
set opt(stoptime)           1001
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation
set opt(txpower)            160;#158.263 ;#Power transmitted in dB re uPa 185.8 is the maximum
set opt(propagation_speed) 1500;# m/s

set opt(maxinterval_)       200
set opt(freq)               50000.0 ;#Frequency used in Hz
set opt(bw)                 26000.0 ;#Bandwidth used in Hz
set opt(bitrate)            20768.0 ;#150000;#bitrate in bps

set rng [new RNG]

if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the first for the seed"
		puts "- the second one is for the Poisson CBR period"
		puts "- the third one is the cbr packet size (byte);"
		puts "example: ns TDMA_exp.tcl 1 60 125"
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
	set opt(cbr_period)     10
	set opt(pktsize)	1250
	set opt(seedcbr)	1
}

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwtdma_simple.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwtdma_simple.cltr"
	set opt(cltracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}


#########################
# Module Configuration  #
#########################
### APP ###
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      2
Module/UW/CBR set debug_               0

### TDMA MAC ###
Module/UW/TDMA set frame_duration   3.5
Module/UW/TDMA set debug_           -7
Module/UW/TDMA set sea_trial_       1
Module/UW/TDMA set fair_mode        0
# FAIR Modality on
# Remeber to put silent the SetSlotDuration, SetGuardTime and setStartTime call
# down below
# Module/UW/TDMA set guard_time       0.1
# Module/UW/TDMA set tot_slots        3

### Channel ###
MPropagation/Underwater set practicalSpreading_ 2
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          10
MPropagation/Underwater set shipping_           1


set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq              $opt(freq)
$data_mask setBandwidth         $opt(bw)
$data_mask setPropagationSpeed  $opt(propagation_speed)


### PHY ###
Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    15.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 200
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    global channel ns cbr position node udp portnum ipr ipif
    global opt mll mac propagation data_mask interf_data
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		set cbr($id,$cnt)  [new Module/UW/CBR] 
	}
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/TDMA]
    set phy($id)  [new Module/UW/PHYSICAL]  
	
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node($id) addModule 7 $cbr($id,$cnt)   1  "CBR"
    }

    $node($id) addModule 6 $udp($id)   1  "UDP"
    $node($id) addModule 5 $ipr($id)   1  "IPR"
    $node($id) addModule 4 $ipif($id)  1  "IPF"   
    $node($id) addModule 3 $mll($id)   1  "MLL"
    $node($id) addModule 2 $mac($id)   1  "MAC"
    $node($id) addModule 1 $phy($id)   1  "PHY"

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node($id) setConnection $cbr($id,$cnt)   $udp($id)   0
        set portnum($id,$cnt) [$udp($id) assignPort $cbr($id,$cnt) ]
    }

    $node($id) setConnection $udp($id)   $ipr($id)   1
    $node($id) setConnection $ipr($id)   $ipif($id)  1
    $node($id) setConnection $ipif($id)  $mll($id)   1
    $node($id) setConnection $mll($id)   $mac($id)   1
    $node($id) setConnection $mac($id)   $phy($id)   1
    $node($id) addToChannel  $channel    $phy($id)   1


    #Set the IP address of the node
    #$ipif($id) addr "1.0.0.${id}"
    $ipif($id) addr [expr $id + 1]
    
    # Set the MAC address
    $mac($id) setMacAddr [expr $id + 5]
    # $mac($id) setSlotNumber $id

    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    
    #Setup positions
    $position($id) setX_ [expr $id*20]
    $position($id) setY_ [expr $id*20]
    $position($id) setZ_ -100

    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    #Propagation model
    $phy($id) setPropagation $propagation
    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
    $phy($id) setInterferenceModel "MEANPOWER"
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
    puts "Node $id created"
}


###############################
# MAC settings: Generic mode  #
###############################
# Node 1
$mac(0) setStartTime    0
$mac(0) setSlotDuration 2
$mac(0) setGuardTime    0.2
# Node 2
$mac(1) setStartTime    2
$mac(1) setSlotDuration 1
$mac(1) setGuardTime    0.2
# Node 3
$mac(2) setStartTime    3
$mac(2) setSlotDuration 0.5
$mac(2) setGuardTime    0.1


################################
# Inter-node module connection #
################################
proc connectNodes {id1 des1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink opt 
    $cbr($id1,$des1) set destAddr_ [$ipif($des1) addr]
    $cbr($id1,$des1) set destPort_ $portnum($des1,$id1)
}

##################
# Setup flows    #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
	    connectNodes $id1 $id2
        }
    }
}

###################
# Fill ARP tables #
###################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
	$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }
}



########################
# Setup routing tables #
########################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
            $ipr($id1) addRoute [$ipif($id2) addr] [$ipif($id2) addr]
        }
    }
}




#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
	if {$id1 != $id2} {
	    $ns at $opt(starttime)    "$cbr($id1,$id2) start"
	    $ns at $opt(stoptime)     "$cbr($id1,$id2) stop"
	}
    }
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
    global ipr_sink ipr ipif udp cbr phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats
    if ($opt(verbose)) {
       puts "-----------------------------------------------------------------"
       puts "Simulation summary"
       puts "-----------------------------------------------------------------"
       puts "Total simulation time    : [expr $opt(stoptime)-$opt(starttime)] s"
       puts "Number of nodes          : $opt(nn)"
       puts "Packet size              : $opt(pktsize) byte(s)"
       puts "CBR period               : $opt(cbr_period) s"
       puts "-----------------------------------------------------------------"
    }

    set sum_cbr_throughput    0
    set sum_mac_sent_pkts     0
    set sum_mac_recv_pkts     0    
    set sum_sent_pkts     0.0
    set sum_recv_pkts     0.0    
    set sum_pcks_in_buffer    0
    set sum_upper_pcks_rx     0
    set sum_mac_pcks_tx       0
    set cbr_throughput        0
    set sent_pkts 0
    set recv_pkts 0

    for {set i 0} {$i < $opt(nn)} {incr i}  {

	set mac_sent_pkts        [$mac($i) get_sent_pkts]
	set mac_recv_pkts        [$mac($i) get_recv_pkts]

	for {set j 0} {$j < $opt(nn)} {incr j} {
	    if {$i != $j} {
		set cbr_throughput        [$cbr($i,$j) getthr]
               	set sent_pkts        [$cbr($i,$j) getsentpkts]
                set recv_pkts        [$cbr($i,$j) getrecvpkts]
        }
   
        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_sent_pkts  [expr $sum_sent_pkts + $sent_pkts]
        set sum_recv_pkts  [expr $sum_recv_pkts + $recv_pkts]
	}
        set sum_upper_pcks_rx  [expr $sum_upper_pcks_rx + [$mac($i) get_upper_data_pkts_rx]]
        set sum_mac_pcks_tx    [expr $sum_mac_pcks_tx + [$mac($i) getDataPktsTx]]
        set sum_mac_sent_pkts  [expr $sum_mac_sent_pkts + $mac_sent_pkts]
        set sum_mac_recv_pkts  [expr $sum_mac_recv_pkts + $mac_recv_pkts]
   	set sum_pcks_in_buffer [expr $sum_pcks_in_buffer + [$mac($i) get_buffer_size]]
    }

    set per_cbr1 [$cbr(0,1) getper]
    set per_cbr2 [$cbr(1,0) getper]
    set per_cbr3 [$cbr(0,2) getper]
    set per_cbr4 [$cbr(2,0) getper]
    set per_cbr5 [$cbr(1,2) getper]
    set per_cbr6 [$cbr(2,1) getper]
    
    if ($opt(verbose)) {
        puts "Mean Throughput          : [expr ($sum_cbr_throughput/(($opt(nn))*($opt(nn)-1)))]"
        puts "MAC sent Packets         : $sum_mac_sent_pkts"
        puts "MAC received Packets     : $sum_mac_recv_pkts"
        puts "CBR sent Packets         : $sum_sent_pkts"
        puts "CBR received Packets     : $sum_recv_pkts"
        puts "Packets in buffer        : $sum_pcks_in_buffer"
        puts "PER CBR (0,1)            : $per_cbr1 "
        puts "PER CBR (1,0)            : $per_cbr2 "
	puts "PER CBR (0,2)            : $per_cbr3 "
        puts "PER CBR (2,0)            : $per_cbr4 "
	puts "PER CBR (1,2)            : $per_cbr5 "
        puts "PER CBR (2,1)            : $per_cbr6 "
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
