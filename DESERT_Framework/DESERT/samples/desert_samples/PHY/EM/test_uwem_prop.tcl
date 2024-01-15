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
# Author: Riccardo Tumiati
# Version: 1.0.0
# Stack
#              Node 0                        Sink
#   +--------------------------+   +--------------------------+   
#   |  7. UW/CBR               |   |  7. UW/CBR               |
#   +--------------------------+   +--------------------------+   
#   |  6. UW/UDP               |   |  6. UW/UDP               |
#   +--------------------------+   +--------------------------+   
#   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |
#   +--------------------------+   +--------------------------+   
#   |  4. UW/IP                |   |  4. UW/IP                |
#   +--------------------------+   +--------------------------+   
#   |  3. UW/MLL               |   |  3. UW/MLL               |
#   +--------------------------+   +--------------------------+   
#   |  2. UW/CSMA_ALOHA        |   |  2. UW/CSMA_ALOHA        |
#   +--------------------------+   +--------------------------+   
#   |1. UW/ElectroMagnetic/PHY |   |1. UW/ElectroMagnetic/PHY |
#   +--------------------------+   +--------------------------+   
#            |         |   propagation     |         |                          
#   +---------------------------------------------------------+
#   |                   UnderwaterChannel                     |
#   +---------------------------------------------------------+

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
load libuwcsmaaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwem_channel.so
load libuwem_propagation.so
load libuwem_phy.so
load libuwem_antenna.so


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

set opt(nn)                 1 ;# Number of Nodes
set opt(pktsize)            125  ;# Pkt sike in byte
set opt(cbr_period)         60  ;# CBR period
set opt(starttime)          1
set opt(stoptime)           100000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(seedcbr)            0

set opt(maxinterval_)       20.0
set opt(freq)               868000000; # in Hz
set opt(bw)                 125000 ; # in Hz
set opt(bitrate)            27000
set opt(temp)               20 ; # in Celsius
set opt(sal)                0
set opt(antenna_gain)       2
set opt(txpower)            14.0 
set opt(PER_LUT)            "../../dbs/em_attenuation/CR_4-5/SF_7.csv"
set opt(ack_mode)           "setNoAckMode"
set opt(debug_phy)          0
set opt(debug_prop)         0
set opt(rep_num)            1

if {$opt(bash_parameters)} {
	if {$argc != 1} {
		puts "The script requires four inputs:"
		puts "- the first for the replication number"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again."
		return
	} else {
		set opt(rep_num)        [lindex $argv 0]    ; # Replication number
	}
}

global defaultRNG
for {set j 0} {$j < $opt(rep_num)} {incr j} {
    $defaultRNG next-substream
}

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

set channel [new Module/UW/ElectroMagnetic/Channel]

Module/UW/ElectroMagnetic/Propagation   set T_          $opt(temp)
Module/UW/ElectroMagnetic/Propagation   set S_          $opt(sal)
Module/UW/ElectroMagnetic/Propagation   set debug_      $opt(debug_prop)
set propagation [new Module/UW/ElectroMagnetic/Propagation]

Module/UW/ElectroMagnetic/PHY   set TxPower_                    $opt(txpower)
Module/UW/ElectroMagnetic/PHY   set BitRate_                    $opt(bitrate)
Module/UW/ElectroMagnetic/PHY   set debug_                      $opt(debug_phy)

set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

set antenna [new Module/UW/ElectroMagnetic/Antenna]
$antenna   setGain   $opt(antenna_gain)

Module/UW/CSMA_ALOHA set listen_time_          [expr 1.0e-4]
Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-4]

#########################
# Module Configuration  #
#########################
#UW/CBR
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      0      

################################
# Procedure(s) to create nodes #
################################
proc createNode { } {

    global channel propagation data_mask ns cbr position node udp portnum ipr ipif
    global phy posdb opt rvposx mll mac db_manager antenna
    global node_coordinates
    
    set node [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr  [new Module/UW/CBR] 
    set udp  [new Module/UW/UDP]
    set ipr  [new Module/UW/StaticRouting]
    set ipif [new Module/UW/IP]
    set mll  [new Module/UW/MLL] 
    set mac  [new Module/UW/CSMA_ALOHA] 
    set phy  [new Module/UW/ElectroMagnetic/PHY]

    $node addModule 7 $cbr   0  "CBR"
    $node addModule 6 $udp   0  "UDP"
    $node addModule 5 $ipr   0  "IPR"
    $node addModule 4 $ipif  0  "IPF"   
    $node addModule 3 $mll   0  "MLL"
    $node addModule 2 $mac   0  "MAC"
    $node addModule 1 $phy   0  "PHY"

    $node setConnection $cbr   $udp   0
    $node setConnection $udp   $ipr   0
    $node setConnection $ipr   $ipif  0
    $node setConnection $ipif  $mll   0
    $node setConnection $mll   $mac   0
    $node setConnection $mac   $phy   0
    $node addToChannel  $channel    $phy   0

    set portnum [$udp assignPort $cbr ]

    set tmp_ [expr  + 1]
    $ipif addr $tmp_
    
    set position [new "Position/BM"]
    $node addPosition $position
    set posdb [new "PlugIn/PositionDB"]
    $node addPlugin $posdb 20 "PDB"
    $posdb addpos [$ipif addr] $position
    
    $phy setPropagation $propagation
    $phy setSpectralMask $data_mask
    $phy setLUTFileName "$opt(PER_LUT)"
    $phy setLUTSeparator ","
    $phy setAntenna $antenna
    $phy useLUT

    $mac $opt(ack_mode)
    $mac initialize
}

proc createSink { } {

    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink antenna

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr_sink       [new Module/UW/CBR] 
    set udp_sink       [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/CSMA_ALOHA]
    set phy_data_sink  [new Module/UW/ElectroMagnetic/PHY] 

    $node_sink addModule 7 $cbr_sink 0 "CBR"
    $node_sink addModule 6 $udp_sink       0 "UDP"
    $node_sink addModule 5 $ipr_sink       0 "IPR"
    $node_sink addModule 4 $ipif_sink      0 "IPF"   
    $node_sink addModule 3 $mll_sink       0 "MLL"
    $node_sink addModule 2 $mac_sink       0 "MAC"
    $node_sink addModule 1 $phy_data_sink  0 "PHY"

    $node_sink setConnection $cbr_sink  $udp_sink            0   
    $node_sink setConnection $udp_sink  $ipr_sink            0
    $node_sink setConnection $ipr_sink  $ipif_sink           0
    $node_sink setConnection $ipif_sink $mll_sink            0 
    $node_sink setConnection $mll_sink  $mac_sink            0
    $node_sink setConnection $mac_sink  $phy_data_sink       0
    $node_sink addToChannel  $channel   $phy_data_sink       0

    set portnum_sink [$udp_sink assignPort $cbr_sink]
    
    $ipif_sink addr 254

    set position_sink [new "Position/BM"]
    $node_sink addPosition $position_sink
    set posdb_sink [new "PlugIn/PositionDB"]
    $node_sink addPlugin $posdb_sink 20 "PDB"
    $posdb_sink addpos [$ipif_sink addr] $position_sink

    $phy_data_sink setSpectralMask $data_mask
    $phy_data_sink setPropagation $propagation
    $phy_data_sink setLUTFileName "$opt(PER_LUT)"
    $phy_data_sink setLUTSeparator ","
    $phy_data_sink useLUT
    $phy_data_sink setAntenna $antenna

    $mac_sink $opt(ack_mode)
    $mac_sink initialize
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
createNode
createSink

################################
# Inter-node module connection #
################################
proc connectNodes {} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr set destAddr_ [$ipif_sink addr]
    $cbr set destPort_ $portnum_sink
    $cbr_sink set destAddr_ [$ipif addr]
    $cbr_sink set destPort_ $portnum
}

connectNodes

# Fill ARP tables
$mll addentry [$ipif_sink addr] [ $mac_sink addr]
$mll_sink addentry [$ipif addr] [ $mac addr]

# Setup positions
$position setX_ 0
$position setY_ 0
$position setZ_ -1

$position_sink setX_ 0
$position_sink setY_ 0
$position_sink setZ_ 1

# Setup routing table
$ipr addRoute [$ipif_sink addr] [$ipif_sink addr]

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
$ns at $opt(starttime)    "$cbr start"
$ns at $opt(stoptime)     "$cbr stop"

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
    puts "Simulation summary"
    puts "packet size      : $opt(pktsize) byte"
    puts "cbr period       : $opt(cbr_period) s"
    puts "simulation length: $opt(txduration) s"
    puts "tx frequency     : $opt(freq) Hz"
    puts "tx bandwidth     : $opt(bw) Hz"
    puts "bitrate          : $opt(bitrate) bps"
    puts "---------------------------------------------------------------------"

    set cbr_throughput         [$cbr_sink getthr]
    set cbr_sent_pkts          "[$cbr getsentpkts].0"
    set cbr_rcv_pkts           "[$cbr_sink getrecvpkts].0"
        
    set ipheadersize        [$ipif getipheadersize]
    set udpheadersize       [$udp getudpheadersize]
    set cbrheadersize       [$cbr getcbrheadersize]
    
    puts "Throughput               : $cbr_throughput"
    puts "Sent Packets             : $cbr_sent_pkts "
    puts "Received Packets         : $cbr_rcv_pkts"
    puts "Packet Delivery Ratio    : [expr $cbr_rcv_pkts / $cbr_sent_pkts * 100]"
    puts "IP Pkt Header Size       : $ipheadersize"
    puts "UDP Header Size          : $udpheadersize"
    puts "CBR Header Size          : $cbrheadersize"
  
    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run