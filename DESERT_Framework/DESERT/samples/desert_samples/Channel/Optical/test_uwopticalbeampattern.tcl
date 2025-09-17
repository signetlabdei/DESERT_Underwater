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
# This script is used to test UW-OPTICAL-Propagation and UW-OPTICAL-PHY with 
# addition of ambient light noise provided by Hydrolight LUT.
# There are 2 nodes that can transmit each other packets in a point 2 point
# netwerk with a CBR (Constant Bit Rate) Application Module
#
# UW/Optical/Channel and UW/UWOPTICALBEAMPATTERN is used for PHY layer and channel
#
# Author: Filippo Campagnaro <campagn1@dei.unipd.it>
# Version: 1.0.0
#
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
#   |  2. UW/CSMA_ALOHA       |
#   +-------------------------+
#   |1.UW/UWOPTICALBEAMPATTERN| 
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |   UW/Optical/Channel    |
#   +-------------------------+

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
load libuwip.so
load libuwstaticrouting.so
load libmphy.so
load libmmac.so
load libuwcsmaaloha.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwoptical_propagation.so
load libuwem_channel.so
load libuwoptical_channel.so
load libuwoptical_phy.so
load libuwopticalbeampattern.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(nn)                 2.0 ;# Number of Nodes
set opt(pktsize)            125  ;# Pkt sike in byte
set opt(starttime)          1	
set opt(stoptime)           1000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation
set opt(ack_mode)           "setNoAckMode"
set opt(maxinterval_)       10.0

set opt(freq)              10000000
set opt(bw)                100000
set opt(bitrate)           1000000
set opt(txpower)           50
set opt(opt_acq_db)        10
set opt(temperatura)       293.15 ; # in Kelvin
set opt(txArea)            0.000010
set opt(rxArea)            0.0000011 ; # receveing area, it has to be the same for optical physical and propagation
set opt(c)                 0.043 ; # pure seawater attenation coefficient
set opt(theta)             1
set opt(id)                [expr 1.0e-9]
set opt(il)                [expr 1.0e-6]
set opt(shuntRes)          [expr 1.49e9]
set opt(sensitivity)       0.26
set opt(LUTpath)           "../../dbs/optical_noise/LUT.txt";#"dbs/optical_noise/ALOMEX_optical_E0/E0vsDEP_2015_11_04__12_34_41__035d45.470m_-004d54.920m_wl531.50_fileCTD020.txt";#"dbs/optical_noise/ALOMEX_optical_E0/E0vsDEP_2015_11_02__13_47_15__036d24.565m_001d40.534m_wl531.50_fileCTD001.txt";
set opt(atten_LUT)         "../../dbs/optical_attenuation/lut_532nm/lut_532nm_CTD025.csv"
set opt(beam_lut_path)     "../../dbs/bluecomm/beam_pattern/beam5mbps.csv";#"dbs/optical_noise/ALOMEX_optical_E0/E0vsDEP_2015_11_02__13_47_15__036d24.565m_001d40.534m_wl531.50_fileCTD001.txt";
set opt(range_lut_path)    "../../dbs/bluecomm/max_range/max_range5mbps.csv"
set opt(cbr_period) 0.1
set opt(pktsize)	125
set opt(rngstream)	1

if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the first for the rngstream"
		puts "- the second one is for the Poisson CBR period"
		puts "- the third one is the cbr packet size (byte);"
		puts "example: ns test_uw_csma_aloha_simple.tcl 1 60 125"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
		return
	} else {
		set opt(rngstream)    [lindex $argv 0]
		set opt(cbr_period) [lindex $argv 1]
		set opt(pktsize)    [lindex $argv 2]
	}
}

global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwcsmaaloha_simple.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwcsmaaloha_simple.cltr"
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
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set debug_               0

Module/UW/UWOPTICALBEAMPATTERN   set TxPower_                    $opt(txpower)
Module/UW/UWOPTICALBEAMPATTERN   set BitRate_                    $opt(bitrate)
Module/UW/UWOPTICALBEAMPATTERN   set AcquisitionThreshold_dB_    $opt(opt_acq_db)
Module/UW/UWOPTICALBEAMPATTERN   set Id_                         $opt(id)
Module/UW/UWOPTICALBEAMPATTERN   set Il_                         $opt(il)
Module/UW/UWOPTICALBEAMPATTERN   set R_                          $opt(shuntRes)
Module/UW/UWOPTICALBEAMPATTERN   set S_                          $opt(sensitivity)
Module/UW/UWOPTICALBEAMPATTERN   set T_                          $opt(temperatura)
Module/UW/UWOPTICALBEAMPATTERN   set Ar_                         $opt(rxArea)
Module/UW/UWOPTICALBEAMPATTERN   set debug_                      0

Module/UW/OPTICAL/Propagation set Ar_       $opt(rxArea)
Module/UW/OPTICAL/Propagation set At_       $opt(txArea)
Module/UW/OPTICAL/Propagation set c_        $opt(c)
Module/UW/OPTICAL/Propagation set theta_    $opt(theta)
Module/UW/OPTICAL/Propagation set debug_    0
set propagation [new Module/UW/OPTICAL/Propagation]
#$propagation setOmnidirectional
$propagation setLUTFileName $opt(atten_LUT)
$propagation setLUT
$propagation setFixedC

set channel [new Module/UW/Optical/Channel]


set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

Module/UW/CSMA_ALOHA set listen_time_          [expr 1.0e-12]
Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-12]
################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    global channel ns cbr position node udp portnum ipr ipif
    global opt mll mac propagation data_mask phy
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
		set cbr($id,$cnt)  [new Module/UW/CBR] 
		
	}
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/CSMA_ALOHA] 
    set phy($id)  [new Module/UW/UWOPTICALBEAMPATTERN]
	
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
		$node($id) setConnection $cbr($id,$cnt)   $udp($id)   1
		set portnum($id,$cnt) [$udp($id) assignPort $cbr($id,$cnt) ]
	}
    $node($id) setConnection $udp($id)   $ipr($id)   1
    $node($id) setConnection $ipr($id)   $ipif($id)  1
    $node($id) setConnection $ipif($id)  $mll($id)   1
    $node($id) setConnection $mll($id)   $mac($id)   1
    $node($id) setConnection $mac($id)   $phy($id)   1
    $node($id) addToChannel  $channel    $phy($id)   1

    set interf_data($id) [new "MInterference/MIV"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    $phy($id) setInterference $interf_data($id)
    $phy($id) setPropagation $propagation
    $phy($id) setSpectralMask $data_mask
    $phy($id) setLUTFileName "$opt(LUTpath)"
    $phy($id) setLUTSeparator " "

    $phy($id) setBeamPatternPath "$opt(beam_lut_path)"
    $phy($id) setMaxRangePath "$opt(range_lut_path)"

    $phy($id) useLUT
    $phy($id) setVariableTemperature

    if {$id == 0} {
        $phy($id) setInclinationAngle [expr - 3.14 /2]
    } else {
        $phy($id) setInclinationAngle [expr 3.14 / 2]
    }

    $ipif($id) addr [expr $id +1]
    
    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    
    #Setup positions
    $position($id) setX_ [expr $id*0]
    $position($id) setY_ [expr $id*0]
    $position($id) setZ_ [expr -5 - $id*100]
    
    $mac($id) $opt(ack_mode)
    $mac($id) initialize
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
}

#################
# Setup Flows   #
#################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
		connectNodes $id1 $id2
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



##################
# Routing tables #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
		$ipr($id2) addRoute [$ipif($id1) addr] [$ipif($id1) addr]
	}
}




#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop the timers
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
		if {$id1 != $id2} {
			$ns at $opt(starttime)    "$cbr($id1,$id2) start"
			$ns at $opt(stoptime)     "$cbr($id1,$id2) stop"
		}
	}
}

# TO CHANGE LUT AT A CERTAIN MOMENT
# $ns at [expr $opt(stoptime)/2] "$phy(0) setLUTFileName dbs/optical_attenuation/lut_532nm/lut_532nm_CTD006.csv"
# $ns at [expr $opt(stoptime)/2] "$phy(0) useLUT"
# $ns at [expr $opt(stoptime)/2] "$phy(1) setLUTFileName dbs/optical_attenuation/lut_532nm/lut_532nm_CTD006.csv"
# $ns at [expr $opt(stoptime)/2] "$phy(1) useLUT"

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
        puts "---------------------------------------------------------------------"
    }
    set sum_cbr_throughput     0
    set sum_cbr_sent_pkts      0.0
    set sum_cbr_rcv_pkts       0.0    

    for {set i 0} {$i < $opt(nn)} {incr i}  {
		for {set j 0} {$j < $opt(nn)} {incr j} {
			set cbr_throughput           [$cbr($i,$j) getthr]
			if {$i != $j} {
				set cbr_sent_pkts        [$cbr($i,$j) getsentpkts]
				set cbr_rcv_pkts           [$cbr($i,$j) getrecvpkts]
                puts "Sent Packets             : $cbr_sent_pkts"
                puts "Received Packets         : $cbr_rcv_pkts"
			}
		}
        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
    }
        
    
    if ($opt(verbose)) {
        puts "Mean Throughput          : [expr ($sum_cbr_throughput/(($opt(nn))*($opt(nn)-1)))]"
        puts "Sent Packets             : $sum_cbr_sent_pkts"
        puts "Received Packets         : $sum_cbr_rcv_pkts"
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


$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 

$ns run
