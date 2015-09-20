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
###########################################################################
# This script is used to test uwrov. There is an ROV remotely controlled
# by a wireless telemetry system. The controller sends command packets 
# containing the next way-point to the rov, meanwhile the rov send monitoring
# packets to the base station.
# N.B.: UnderwaterChannel and UW/PHYSICAL are used for PHY layer and channel,
# while an unbalanced TDMA is employed as datalink.
# The way-point list is imported by an external file. 
###########################################################################
#
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
#   |  2. UW/TDMA             |        |  2. UW/TDMA             |
#   +-------------------------+        +-------------------------+
#   |  1. UW/PHYSICAL         |        |  1. UW/PHYSICAL         |
#   +-------------------------+        +-------------------------+
#           |         |                        |         |     
#   +------------------------------------------------------------+              #   |                    UnderwaterChannel                       |
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
load libuwtdma.so
load libuwcbr.so
load libuwaloha.so
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

set opt(nn)                     2.0 ;# Number of Nodes
set opt(ROV_pktsize)            [expr 4024];#125  ;# Pkt size in byte
set opt(CTR_pktsize)            [expr 1024/8];#125  ;# Pkt size in byte

set opt(ROV_period) 			0.5

set opt(starttime)          1
set opt(stoptime)           3000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation

set opt(txpower)            160;#158.263 ;#Power transmitted in dB re uPa 185.8 is the maximum
set opt(propagation_speed) 1500;# m/s

set opt(maxinterval_)       200
set opt(freq)               375000.0 ;#Frequency used in Hz
set opt(bw)                 76000.0	;#Bandwidth used in Hz
set opt(bitrate)            87768.0 ;#150000;#bitrate in bps
set opt(max_rtt)            [expr 2*$opt(maxinterval_)/$opt(propagation_speed) + $opt(CTR_pktsize)/$opt(bitrate) + $opt(ROV_pktsize)/$opt(bitrate)];

set opt(CTR_timeout)             [expr 6+2*($opt(max_rtt)+$opt(ROV_period))];#time out before retransmission which [] ?
#if {$opt(ACK_Active)} {
#    set opt(ack_mode)           "setAckMode"    
#} else {
#    set opt(ack_mode)           "setNoAckMode"
#}
set opt(waypoint_file)  "dbs/wp_path/rov_path.csv"

set rng [new RNG]
set rng_position [new RNG]

if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the first for the seed"
		puts "- the second one is for the Poisson ROV period"
		puts "- the third one is the ROV packet size (byte);"
		puts "example: ns test_uw_rov_tdma.tcl 1 60 125"
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
$data_mask setPropagationSpeed  $opt(propagation_speed)

#########################
# Module Configuration  #
#########################
Module/UW/ROV set packetSize_          $opt(ROV_pktsize)
Module/UW/ROV set period_              $opt(ROV_period)

Module/UW/ROV/CTR set packetSize_          $opt(CTR_pktsize)
#timeout
Module/UW/ROV/CTR set period_              $opt(CTR_timeout)

Module/UW/ROV set debug_               0

Module/UW/ROV/CTR set debug_               0

#TDMA
Module/UW/TDMA set frame_duration       6
Module/UW/TDMA set debug_               0
Module/UW/TDMA set sea_trial_           0
Module/UW/TDMA set fair_mode            1
# FAIR Modality on
# Remeber to put silent the SetSlotDuration, SetGuardTime and setStartTime call
Module/UW/TDMA set guard_time           0.1
Module/UW/TDMA set tot_slots            2

Module/UW/TDMA set ACK_size_            0
Module/UW/TDMA set max_tx_tries_        1
Module/UW/TDMA set wait_constant_       0
Module/UW/TDMA set max_payload_         10000
Module/UW/TDMA set ACK_timeout_         10000.0


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
proc createNode {node application id} {

    global channel propagation data_mask ns  position udp portnum ipr ipif channel_estimator
    global phy opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global node_coordinates interf_data
    
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    # set mac($id)  [new Module/UW/ALOHA] 
    # set mac($id)  [new Module/UW/CSMA_ALOHA]
    
    set mac($id)  [new Module/UW/TDMA]
    if {$id == 1} {
        $mac($id) setMacAddr [expr $id + 1]
    } else {
        $mac($id) setMacAddr [expr $id + 1]
    }
    
    #$mac($id)  setNoAckMode
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
    
    set position($id) [new "Position/UWSM"]
    $node addPosition $position($id)

    #Setup positions
    $position($id) setX_ [expr $id*100]
    $position($id) setZ_ -15
    $position($id) setY_ 0
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
    # $mac($id) $opt(ack_mode)
    # $mac($id) initialize
}



#################
# Node Creation #
#################
# Create here all the nodes you want to network together
# node 0 is the CTR
# node 1 is the ROV
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

###############################
# TDMA settings: Generic mode  #
###############################
# # Node CTR
# $mac(0) setStartTime    0
# $mac(0) setSlotDuration 2
# $mac(0) setGuardTime    0.1
# # Node ROV
# $mac(1) setStartTime    2
# $mac(1) setSlotDuration 4
# $mac(1) setGuardTime    0.2

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

set outfile [open "test_uwrov_tdma_results.csv" "w"]
close $outfile
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set t_prec 0
foreach line $data {
	if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {
        set t [expr $t + 1*rand()]
        if { $t_prec > 0 } {
            for { set i 0 } { $i < 10 } { incr i } {
                set time_q [expr $t_prec + $i*($t-$t_prec)/10]
                $ns at $time_q "update_and_check $time_q"
            }
        }
        set t_prec $t
		$ns at $t "$applicationCTR sendPosition $x $y $z"
    }
}
$ns at [expr $opt(starttime)+0.0000000]    "$applicationROV start"
$ns at [expr $opt(starttime)+0.00000002]    "$applicationCTR start"
$ns at [expr $opt(starttime)+0.00000001]    "$mac(0) start"
$ns at [expr $opt(starttime)+0.00000003]    "$mac(1) start"
$ns at $opt(stoptime)     "$applicationROV stop"
$ns at $opt(stoptime)     "$applicationCTR stop"

proc update_and_check {t} {
    global position applicationROV
    $position(1) update
    set outfile [open "test_uwrov_tdma_results.csv" "a"]
    #puts $outfile "$t positions ROV: x = [$applicationROV getX], y = [$applicationROV getY], z =  [$applicationROV getZ]" 
    puts $outfile "$t,[$applicationROV getX],[$applicationROV getY],[$applicationROV getZ]" 
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
        update_and_check $opt(stoptime)
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
   #     if {$opt(ack_mode) == "setNoAckMode"} {
    #        puts "ACKNOWLEDGEMENT   : disabled"
     #   } else {
      #      puts "ACKNOWLEDGEMENT   : active"
      #  }
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
            puts "----------------------------------------------"
            puts "applicationCTR Throughput     : $CTR_throughput"
            puts "applicationCTR PER            : $CTR_per       "
            puts "----------------------------------------------"

 #       if {$opt(ack_mode) == "setAckMode"} {
 #           set DataPktsTx_CTR                  [$mac(0) getDataPktsTx]
 #           set UpDataPktsRx_CTR                [$mac(0) getUpLayersDataRx]
 #           set DataPktsTx_ROV                  [$mac(1) getDataPktsTx]
 #           set UpDataPktsRx_ROV                [$mac(1) getUpLayersDataRx]
 #           set rtx_CTR                         [expr (($DataPktsTx_CTR/$ROV_rcv_pkts) - 1)]
 #           set rtx_ROV                         [expr (($DataPktsTx_ROV/$CTR_rcv_pkts) - 1)]
 #       }

        set sum_throughput [expr $ROV_throughput + $CTR_throughput]
        set sum_sent_pkts [expr $ROV_sent_pkts + $CTR_sent_pkts]
        set sum_rcv_pkts  [expr $ROV_rcv_pkts + $CTR_rcv_pkts]
 #       if {$opt(ack_mode) == "setAckMode"} {
 #           set sum_rtx           [expr $rtx_ROV + $rtx_CTR]
 #       }
 #   }
        
    set ipheadersize        [$ipif(1) getipheadersize]
    set udpheadersize       [$udp(1) getudpheadersize]
    set ROVheadersize       [$applicationROV getROVMonheadersize]
    set CTRheadersize       [$applicationCTR getROVctrheadersize]
    
    if ($opt(verbose)) {
        puts "Mean Throughput              : [expr ($sum_throughput/(($opt(nn))*($opt(nn)-1)))]"
        puts "Sent Packets     CTR --> ROV : $CTR_sent_pkts"
        puts "Received Packets CTR --> ROV : $ROV_rcv_pkts"
        puts "Sent Packets     ROV --> CTR : $ROV_sent_pkts"
        puts "Received Packets ROV --> CTR : $CTR_rcv_pkts"
        puts "---------------------------------------------------------------------"
        puts "Sent Packets     : $sum_sent_pkts"
        puts "Received   : $sum_rcv_pkts"
        puts "Packet Delivery Ratio        : [expr 100*$sum_rcv_pkts / $sum_sent_pkts]"
        puts "IP Pkt Header Size           : $ipheadersize"
        puts "UDP Header Size              : $udpheadersize"
        puts "ROV Header Size              : $ROVheadersize"
        puts "CTR Header Size              : $CTRheadersize"
 #       if {$opt(ack_mode) == "setAckMode"} {
 #           puts "MAC-level average retransmissions per node : [expr $sum_rtx/($opt(nn))]"
 #       }
        puts "---------------------------------------------------------------------"
        set ROV_packet_lost             [$phy(1) getTotPktsLost]
        set CTR_packet_lost             [$phy(0) getTotPktsLost]
        set packet_lost                 [expr $CTR_packet_lost + $ROV_packet_lost]
        puts "- PHY layer statistics for the ROV -"
        puts "Tot. pkts lost               : $ROV_packet_lost"
        puts "Tot. collision CTRL          : [$phy(1) getCollisionsCTRL]"
        puts "Tot. collision DATA          : [$phy(1) getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL  : [$phy(1) getCollisionsDATAvsCTRL]"
        puts "Tot. CTRL pkts lost          : [$phy(1) getTotCtrlPktsLost]"
        puts "Tot. CTRL pkts lost due to Interference   : [$phy(1) getErrorCtrlPktsInterf]"
        puts "---------------------------------------------------------------------"
        puts "- PHY layer statistics for the CTR -"
        puts "Tot. pkts lost               : $CTR_packet_lost"
        puts "Tot. collision CTRL          : [$phy(0) getCollisionsCTRL]"
        puts "Tot. collision DATA          : [$phy(0) getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL  : [$phy(0) getCollisionsDATAvsCTRL]"
        puts "Tot. ROV pkts lost           : [$phy(0) getTotCtrlPktsLost]"
        puts "Tot. CTRL pkts lost due to Interference   : [$phy(0) getErrorCtrlPktsInterf]"
        puts "---------------------------------------------------------------------"
        puts "- Global situation -"
        puts "Tot. pkts lost               : $packet_lost"
	set sum_pcks_in_buffer [expr [$mac(1) get_buffer_size] + [$mac(0) get_buffer_size]]
   	puts "Pckts in buffer              : $sum_pcks_in_buffer"
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


$ns at [expr $opt(stoptime) + 50.0]  "finish; $ns halt" 

$ns run
