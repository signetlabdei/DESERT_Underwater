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
###########################################################################
# This script is used to test uwrov. There is an ROV remotely controlled
# by a wireless telemetry system. The controller sends command packets 
# containing the next way-point to the rov, meanwhile the rov send monitoring
# packets to the base station.
# N.B.: UnderwaterChannel and UW/PHYSICAL are used for PHY layer and channel,
# while CSMA_ALOHA is employed as datalink.
# The way-point list is imported by an external file. 
###########################################################################
# Author: Filippo Campagnaro <campagn1@dei.unipd.it>
# Version: 1.0.0
#
# NOTE: tcl sample tested on Mint 17, 64 bits OS
#
# Stack of the node and the ROV
#   +-------------------------+        +-------------------------+
#   |  7. UW/ROV/CTR          |        |  7. UW/ROV              |
#   +-------------------------+        +-------------------------+
#   |  6. UW/UDP              |        |  6. UW/UDP              |
#   +-------------------------+        +-------------------------+
#   |  5. UW/STATICROUTING    |        |  5. UW/STATICROUTING    | 
#   +-------------------------+        +-------------------------+
#   |  4. UW/IP               |        |  4. UW/IP               |
#   +-------------------------+        +-------------------------+
#   |  3. UW/MLL              |        |  3. UW/MLL              |
#   +-------------------------+        +-------------------------+
#   |  2. UW/CSMA_ALOHA       |        |  2. UW/CSMA_ALOHA       |
#   +-------------------------+        +-------------------------+
#   |  1. UW/PHYSICAL         |        |  1. UW/PHYSICAL         |
#   +-------------------------+        +-------------------------+
#           |         |                        |         |     
#   +------------------------------------------------------------+                  
#   |            UnderwaterChannel                               |
#   +------------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 			1
set opt(trace_files)		1
set opt(bash_parameters) 	0
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
load libuwcsmaaloha.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphysical.so

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
set opt(ROV_pktsize)            1000;#125  ;# Pkt size in byte
set opt(CTR_pktsize)            1024;#125  ;# Pkt size in byte

set opt(ROV_period) 			2.5

set opt(starttime)          1
set opt(stoptime)           10000 
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation

set opt(txpower)            185.8  ;#Power transmitted in dB re uPa


set opt(maxinterval_)       20.0
set opt(freq)               25000.0 ;#Frequency used in Hz
set opt(bw)                 5000.0  ;#Bandwidth used in Hz
set opt(bitrate)            4800.0  ;#bitrate in bps
if {$opt(ACK_Active)} {
    set opt(ack_mode)           "setAckMode"    
} else {
    set opt(ack_mode)           "setNoAckMode"
}
set opt(waypoint_file)  "dbs/wp_path/rov_path.csv"

set rng [new RNG]
set rng_position [new RNG]

if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the first for the seed"
		puts "- the second one is for the Poisson ROV period"
		puts "- the third one is the ROV packet size (byte);"
		puts "example: ns test_uw_rov.tcl 1 60 125"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
		return
	} else {
		set opt(seedROV)    [lindex $argv 0]
		$rng seed         $opt(seedROV)
	}
} else {
	set opt(seedROV)	1
}

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwrovmovement.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwrovmovement.cltr"
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
MPropagation/Underwater set shipping_          1



set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)
$data_mask setPropagationSpeed  1500

#########################
# Module Configuration  #
#########################
Module/UW/ROV set packetSize_          $opt(ROV_pktsize)
Module/UW/ROV set period_              $opt(ROV_period)
#Module/UW/ROV set PoissonTraffic_      100
Module/UW/ROV set debug_               0

Module/UW/ROV/CTR set packetSize_          $opt(CTR_pktsize)
#Module/UW/ROV/CTR set period_              $opt(ROV_period)
#Module/UW/ROV/CTR set PoissonTraffic_      1
Module/UW/ROV/CTR set debug_               0



Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    15.0 
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

################################
# Procedure(s) to create nodes #
################################
proc createNode {node application id} {

    global channel propagation data_mask ns  position udp portnum ipr ipif channel_estimator
    global phy opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global node_coordinates interf_data
    
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/CSMA_ALOHA] 
    set phy($id)    [new Module/UW/PHYSICAL]

	$node addModule 7 $application   1  "ROV"
    $node addModule 6 $udp($id)   1  "UDP"
    $node addModule 5 $ipr($id)   1  "IPR"
    $node addModule 4 $ipif($id)  1  "IPF"   
    $node addModule 3 $mll($id)   1  "MLL"
    $node addModule 2 $mac($id)   1  "MAC"
    $node addModule 1 $phy($id)   0  "PHY"

	$node setConnection $application  $udp($id)   1
	set portnum($id) [$udp($id) assignPort $application ]
    $node setConnection $udp($id)      $ipr($id)   1
    $node setConnection $ipr($id)      $ipif($id)  1
    $node setConnection $ipif($id)     $mll($id)   1
    $node setConnection $mll($id)      $mac($id)   1
    $node setConnection $mac($id)      $phy($id)   1
    $node addToChannel  $channel       $phy($id)   1

    if {$id > 254} {
		puts "hostnum > 254!!! exiting"
		exit
    }
    #Set the IP address of the node
    set ip_addr_value [expr $id + 1]
    $ipif($id) addr $ip_addr_value
    
    set position($id) [new "Position/SM"]
    $node addPosition $position($id)
    #set posdb($id) [new "PlugIn/PositionDB"]
   # $node addPlugin $posdb($id) 20 "PDB"
 #   $posdb($id) addpos [$ipif($id) addr] $position($id)
    
    #Setup positions
    $position($id) setX_ [expr $id*5]
    $position($id) setY_ [expr $id*5]
    $position($id) setZ_ -10
    $application setPosition $position($id)
    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

	#Propagation model
    $phy($id) setPropagation $propagation
    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
    $phy($id) setInterferenceModel "MEANPOWER"
    $mac($id) $opt(ack_mode)
    $mac($id) initialize
}



#################
# Node Creation #
#################
# Create here all the nodes you want to network together
#node 0 is the CTR
#node 1 is the ROV
global nodeCTR nodeROV applicationCTR applicationROV
set nodeCTR [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
set applicationCTR  [new Module/UW/ROV/CTR]
createNode $nodeCTR $applicationCTR 0
set nodeROV [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
set applicationROV  [new Module/UW/ROV]
createNode $nodeROV $applicationROV 1
 
 puts "positions CTR: [$position(0) getX_]"
 puts "positions rov: [$position(1) getX_]"
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
			set ip_value [expr $id2 + 1]
            $ipr($id1) addRoute ${ip_value} ${ip_value}
	}
}

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

set outfile [open "test_uwrov_results.csv" "w"]
close $outfile
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
foreach line $data {
	if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {
        $ns at $t "update_and_check"
		$ns at $t "$applicationCTR sendPosition $x $y $z"
    }
}
#$ns at $opt(starttime)    "$applicationCTR start"
#$ns at $opt(stoptime)     "$applicationCTR stop"
$ns at $opt(starttime)    "$applicationROV start"
$ns at $opt(stoptime)     "$applicationROV stop"

proc update_and_check {} {
    global position applicationROV
    $position(1) update
    #puts "positions ROV: x = [$applicationROV getX], y = [$applicationROV getY], z =  [$applicationROV getZ]"
    set outfile [open "test_uwrov_results.csv" "a"]
    puts $outfile "positions ROV: x = [$applicationROV getX], y = [$applicationROV getY], z =  [$applicationROV getZ]" 
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
    global node_stats tmp_node_stats sink_stats tmp_sink_stats
    if ($opt(verbose)) {
        puts "positions CTR: [$applicationCTR getX]"
        update_and_check 
        puts "---------------------------------------------------------------------"
        puts "Simulation summary"
        puts "number of nodes  : $opt(nn)"
        puts "ROV packet size      : $opt(ROV_pktsize) byte"
        puts "ROV period       : $opt(ROV_period) s"
        puts "CTR packet size      : $opt(CTR_pktsize) byte"
        puts "number of nodes  : $opt(nn)"
        puts "simulation length: $opt(txduration) s"
        puts "tx power         : $opt(txpower) dB"
        puts "tx frequency     : $opt(freq) Hz"
        puts "tx bandwidth     : $opt(bw) Hz"
        puts "bitrate          : $opt(bitrate) bps"
        if {$opt(ack_mode) == "setNoAckMode"} {
            puts "ACKNOWLEDGEMENT   : disabled"
        } else {
            puts "ACKNOWLEDGEMENT   : active"
        }
        puts "---------------------------------------------------------------------"
    } 
    set ROV_throughput              [$applicationROV getthr]
    set ROV_per                     [$applicationROV getper]
	set ROV_sent_pkts               [$applicationROV getsentpkts]
	set ROV_rcv_pkts                [$applicationROV getrecvpkts]

    set CTR_throughput              [$applicationCTR getthr]
    set CTR_per                     [$applicationCTR getper]
    set CTR_sent_pkts               [$applicationCTR getsentpkts]
    set CTR_rcv_pkts                [$applicationCTR getrecvpkts]
	if ($opt(verbose)) {
		    puts "applicationROV Throughput     : $ROV_throughput"
            puts "applicationROV PER            : $ROV_per       "
            puts "-------------------------------------------"
            puts "applicationCTR Throughput     : $CTR_throughput"
            puts "applicationCTR PER            : $CTR_per       "
            puts "-------------------------------------------"

        if {$opt(ack_mode) == "setAckMode"} {
            set DataPktsTx_CTR                  [$mac(0) getDataPktsTx]
            set UpDataPktsRx_CTR                [$mac(0) getUpLayersDataRx]
            set DataPktsTx_ROV                  [$mac(1) getDataPktsTx]
            set UpDataPktsRx_ROV                [$mac(1) getUpLayersDataRx]
            set rtx_CTR                         [expr (($DataPktsTx_CTR/$ROV_rcv_pkts) - 1)]
            set rtx_ROV                         [expr (($DataPktsTx_ROV/$CTR_rcv_pkts) - 1)]
        }

        set sum_throughput [expr $ROV_throughput + $CTR_throughput]
        set sum_sent_pkts [expr $ROV_sent_pkts + $CTR_sent_pkts]
        set sum_rcv_pkts  [expr $ROV_rcv_pkts + $CTR_rcv_pkts]
        if {$opt(ack_mode) == "setAckMode"} {
            set sum_rtx           [expr $rtx_ROV + $rtx_CTR]
        }
    }
        
    set ipheadersize        [$ipif(1) getipheadersize]
    set udpheadersize       [$udp(1) getudpheadersize]
    set ROVheadersize       [$applicationROV getROVMonheadersize]
    set CTRheadersize       [$applicationCTR getROVctrheadersize]
    
    if ($opt(verbose)) {
        puts "Mean Throughput           : [expr ($sum_throughput/(($opt(nn))*($opt(nn)-1)))]"
        puts "Sent Packets CTR --> ROV     : $CTR_sent_pkts"
        puts "Received Packets CTR --> ROV     : $ROV_rcv_pkts"
        puts "Sent Packets  ROV --> CTR   : $ROV_sent_pkts"
        puts "Received Packets ROV --> CTR   : $CTR_rcv_pkts"
        puts "---------------------------------------------------------------------"
        puts "Sent Packets     : $sum_sent_pkts"
        puts "Received   : $sum_rcv_pkts"
        puts "Packet Delivery Ratio     : [expr $sum_rcv_pkts / $sum_sent_pkts * 100]"
        puts "IP Pkt Header Size        : $ipheadersize"
        puts "UDP Header Size           : $udpheadersize"
        puts "ROV Header Size           : $ROVheadersize"
        puts "CTR Header Size           : $CTRheadersize"
        if {$opt(ack_mode) == "setAckMode"} {
            puts "MAC-level average retransmissions per node : [expr $sum_rtx/($opt(nn))]"
        }
        puts "---------------------------------------------------------------------"
        set ROV_packet_lost             [$phy(1) getTotPktsLost]
        set CTR_packet_lost             [$phy(0) getTotPktsLost]
        set packet_lost                 [expr $CTR_packet_lost + $ROV_packet_lost]
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

