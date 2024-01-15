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
# ----------------------------------------------------------------------------------
# This script depicts a very simple but complete stack in which two node_leaders send data
# to a common sink. The second node_leader is used by the first one as a relay to send data to the sink.
# The routes are configured by using UW/STATICROUTING.
# The application used to generate data is UW/CBR.
# ----------------------------------------------------------------------------------
# Stack
#             Node 1                         Node 2                        Sink
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
set opt(trace_files)        0
set opt(bash_parameters)    0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
load libuwcsmaaloha.so
load libuwmmac_clmsgs.so
load libuwaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwtdma.so
load libuwsmposition.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwposbasedrt.so
load libuwflooding.so
load libuwinterference.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwoptical_propagation.so
load libuwem_channel.so
load libuwoptical_channel.so
load libuwoptical_phy.so
load libuwmulti_traffic_control.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(n_diver)                 4 ;# Number of Nodes
set opt(starttime)          1
set opt(stoptime)           100000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(rngstream)            1

set opt(maxinterval_)       20.0
set opt(freq)               25000.0
set opt(bw)                 5000.0
set opt(freq_hf)               60000.0
set opt(bw_hf)                 10000.0
set opt(bitrate)            4800.0
set opt(bitrate_hf)            64000.0
set opt(ack_mode)           "setNoAckMode"


set opt(txpower)            135.0 
set opt(txpower_hf)         135.0 

set opt(tdma_frame) 20
set opt(tdma_gard)  1

set opt(pktsize)    125
set opt(cbr_period) 60

set opt(pktsize_health) 50
set opt(cbr_period_health) [expr $opt(tdma_frame)*2]

set opt(pktsize_control) 100
set opt(cbr_period_control)  [expr $opt(cbr_period_health)*4]

set opt(pktsize_image) 1000
set opt(cbr_period_image)  [expr $opt(cbr_period_health)*5]

set opt(pktsize_sos) 5
set opt(cbr_period_sos) [expr $opt(stoptime)/5]

set opt(op_freq)              10000000
set opt(op_bw)                100000
set opt(bitrate_op)           1000000
set opt(txpower_op)           30
set opt(acq_db_op)        10
set opt(temperatura)       293.15 ; # in Kelvin
set opt(txArea)            0.000010
set opt(rxArea)            0.0000011 ; # receveing area, it has to be the same for optical physical and propagation
set opt(c)                 0.4 ; # coastal water
set opt(theta)             1
set opt(id)                [expr 1.0e-9]
set opt(il)                [expr 1.0e-6]
set opt(shuntRes)          [expr 1.49e9]
set opt(sensitivity)       0.26
set opt(LUTpath)           "../dbs/optical_noise/LUT.txt"
set opt(rngstream) 13
set opt(cbr_period) 60

if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires two inputs:"
        puts "- the first for the Poisson CBR period"
        puts "- the second for the rngstream"
        puts "example: ns test_uw_rov.tcl 60 13"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        return
    } else {
        set opt(cbr_period) [lindex $argv 0]
        set opt(rngstream) [lindex $argv 1];
    }   
} 
#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
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

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

set data_mask_hf [new MSpectralMask/Rect]
$data_mask_hf setFreq       $opt(freq_hf)
$data_mask_hf setBandwidth  $opt(bw_hf)

#########################
# Module Configuration  #
#########################
#UW/CBR
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set debug_               0

# BPSK              
Module/MPhy/BPSK  set BitRate_          $opt(bitrate)
Module/MPhy/BPSK  set TxPower_          $opt(txpower)

#FLOODING
Module/UW/FLOODING set ttl_                       2
Module/UW/FLOODING set maximum_cache_time__time_  $opt(stoptime)


Module/UW/IP set debug_                      0
Module/UW/UDP set debug_                     0

#TRAFFIC_CTR
Module/UW/MULTI_TRAFFIC_RANGE_CTR set debug_            0
Module/UW/MULTI_TRAFFIC_RANGE_CTR set check_to_period_  50

Module/UW/CSMA_ALOHA set wait_costant_ 0.01
Module/UW/CSMA_ALOHA set listen_time_ 0.01

Module/UW/TDMA set frame_duration   $opt(tdma_frame)
Module/UW/TDMA set fair_mode        1
Module/UW/TDMA set guard_time       $opt(tdma_gard)
Module/UW/TDMA set tot_slots        $opt(n_diver)

# PHY

Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxRange_                 50000
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0


# OPTICAL PHY

Module/UW/OPTICAL/PHY   set TxPower_                    $opt(txpower_op)
Module/UW/OPTICAL/PHY   set BitRate_                    $opt(bitrate_op)
Module/UW/OPTICAL/PHY   set AcquisitionThreshold_dB_    $opt(acq_db_op)
Module/UW/OPTICAL/PHY   set Id_                         $opt(id)
Module/UW/OPTICAL/PHY   set Il_                         $opt(il)
Module/UW/OPTICAL/PHY   set R_                          $opt(shuntRes)
Module/UW/OPTICAL/PHY   set S_                          $opt(sensitivity)
Module/UW/OPTICAL/PHY   set T_                          $opt(temperatura)
Module/UW/OPTICAL/PHY   set Ar_                         $opt(rxArea)
Module/UW/OPTICAL/PHY   set debug_                      0

Module/UW/OPTICAL/Propagation set Ar_       $opt(rxArea)
Module/UW/OPTICAL/Propagation set At_       $opt(txArea)
Module/UW/OPTICAL/Propagation set c_        $opt(c)
Module/UW/OPTICAL/Propagation set theta_    $opt(theta)
Module/UW/OPTICAL/Propagation set debug_    0
set propagation_op [new Module/UW/OPTICAL/Propagation]
$propagation_op setOmnidirectional
set channel_op [new Module/UW/Optical/Channel]
set data_mask_op [new MSpectralMask/Rect]
$data_mask_op setFreq       $opt(op_freq)
$data_mask_op setBandwidth  $opt(op_bw)

################################
# Procedure(s) to create node_leaders #
################################

source "divers.tcl"
source "rov_sink.tcl"
source "leader.tcl"

for {set id1 0} {$id1 < $opt(n_diver)} {incr id1}  {
    createDiver $id1
}
createLeader $opt(n_diver) 
createROV [expr $opt(n_diver) + 1]

################################
# Inter-node_leader module connection #
################################

proc connectNodes {id1} {
    global ipif_leader ipr_leader ipr2_leader ipif2_leader portnum_leader portnum4_rov
    global portnum2_leader portnum3_leader cbr2_leader portnum2_diver portnum2_rov cbr4_diver
    global ipif_diver cbr_diver cbr2_diver cbr3_diver ipif_rov portnum_rov portnum3_rov ipif2_rov
    $cbr2_leader($id1) set destAddr_ [$ipif_diver($id1) addr]
    $cbr2_leader($id1) set destPort_ $portnum2_diver($id1)

    $cbr_diver($id1) set destAddr_ [$ipif_rov addr]
    $cbr_diver($id1) set destPort_ $portnum_rov($id1)
    # $cbr2_diver($id1) set destAddr_ [$ipif_rov addr]
    # $cbr2_diver($id1) set destPort_ $portnum2_rov
    $cbr3_diver($id1) set destAddr_ [$ipif_rov addr]
    $cbr3_diver($id1) set destPort_ $portnum3_rov($id1)
    $cbr4_diver($id1) set destAddr_ [$ipif2_rov addr]
    $cbr4_diver($id1) set destPort_ $portnum4_rov($id1)
}

# Setup flows
$cbr2_leader($opt(n_diver)) set destAddr_ [$ipif_rov addr]
$cbr2_leader($opt(n_diver)) set destPort_ $portnum2_rov
$cbr_leader set destAddr_ [$ipif_rov addr]
$cbr_leader set destPort_ $portnum_rov($opt(n_diver))
$cbr3_leader set destAddr_ [$ipif_rov addr]
$cbr3_leader set destPort_ $portnum3_rov($opt(n_diver))
$cbr4_leader set destAddr_ [$ipif2_rov addr]
$cbr4_leader set destPort_ $portnum4_rov($opt(n_diver))
for {set id1 0} {$id1 < $opt(n_diver)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(n_diver)} {incr id2}  {
        if {$id1 != $id2} {
            connectNodes $id1
        }
    }
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(n_diver)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(n_diver)} {incr id2}  {
      $mll_diver($id1) addentry [$ipif_diver($id2) addr] [$mac_diver($id2) addr]
      $mll_hf_diver($id1) addentry [$ipif_diver($id2) addr] [$mac_hf_diver($id2) addr]
    }   
    $mll_diver($id1) addentry [$ipif_leader addr] [$mac_leader addr]
    # $mll_diver($id1) addentry [$ipif_rov addr] [$mac_rov addr]
    $mll_hf_diver($id1) addentry [$ipif_leader addr] [$mac_hf_leader addr]
    # $mll_hf_diver($id1) addentry [$ipif_rov addr] [$mac_rov addr]
    $mll_leader addentry [$ipif_diver($id1) addr] [$mac_diver($id1) addr]
    $mll_hf_leader addentry [$ipif_diver($id1) addr] [$mac_hf_diver($id1) addr]
}

$mll_leader addentry [$ipif_rov addr] [$mac_rov addr]
$mll_op_leader addentry [$ipif_rov addr] [$mac_op_rov addr]

$mll_rov addentry [$ipif_leader addr] [$mac_leader addr]
$mll_op_rov addentry [$ipif_leader addr] [$mac_op_leader addr]

# Setup positions
$position_leader setX_ 0
$position_leader setY_ 0
$position_leader setZ_ -1000

$position_rov setX_ 0
$position_rov setY_ 10
$position_rov setZ_ -1000

for {set id 0} {$id < $opt(n_diver)} {incr id}  {
    $position_diver($id) setX_ [expr -50 * (1 + $id % $opt(n_diver)/2) - 25 + 50 * rand()]
    $position_diver($id) setY_ [expr -50 + 100 * ($id%2) - 25 + 50 * rand()]
    $position_diver($id) setZ_ -1000
    puts "x = [$position_diver($id) getX_]; y = [$position_diver($id) getY_]; z = [$position_diver($id) getZ_]"
}

# Setup routing table
for {set id 0} {$id < $opt(n_diver)} {incr id}  {
    $ipr_diver($id) addRoute [$ipif_rov addr] [$ipif_leader addr]
    $ipr_leader addRoute [$ipif_diver($id) addr] [$ipif_diver($id) addr]

    # $ipr2_diver($id) addRoute [$ipif2_rov addr] [$ipif2_leader addr]
    # $ipr2_leader addRoute [$ipif2_diver($id) addr] [$ipif2_diver($id) addr]
}
$ipr_leader addRoute [$ipif_rov addr] [$ipif_rov addr]


#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

$ns at $opt(starttime)    "$cbr_leader start"
$ns at $opt(stoptime)     "$cbr_leader stop"

$ns at $opt(starttime)    "$cbr2_leader($opt(n_diver)) start"
$ns at $opt(stoptime)     "$cbr2_leader($opt(n_diver)) stop"

$ns at $opt(starttime)    "$cbr3_leader start"
$ns at $opt(stoptime)     "$cbr3_leader stop"
$ns at $opt(starttime)    "$cbr4_leader start"
$ns at $opt(stoptime)     "$cbr4_leader stop"
for {set id1 0} {$id1 < $opt(n_diver)} {incr id1}  {
    $ns at $opt(starttime)    "$cbr2_leader($id1) start"
    $ns at $opt(stoptime)     "$cbr2_leader($id1) stop"

    $ns at $opt(starttime)    "$cbr_diver($id1) start"
    $ns at $opt(stoptime)     "$cbr_diver($id1) stop"
    $ns at $opt(starttime)    "$cbr3_diver($id1) start"
    $ns at $opt(stoptime)     "$cbr3_diver($id1) stop"
    $ns at $opt(starttime)    "$cbr4_diver($id1) start"
    $ns at $opt(stoptime)     "$cbr4_diver($id1) stop"
    
    $ns at $opt(starttime)    "$mac_diver($id1) start"
    $ns at $opt(stoptime)     "$mac_diver($id1) stop"
}

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt
    global mac propagation phy_data channel db_manager propagation
    global node_leader_coordinates 
    global ipr_leader ipif_leader udp_leader phy 
    global cbr_leader  cbr2_leader cbr3_leader cbr4_leader
    global cbr_rov  cbr2_rov cbr3_rov cbr4_rov
    global cbr_diver  cbr2_diver cbr3_diver cbr4_diver mac_diver
    global node_leader_stats tmp_node_leader_stats ipif_rov ipif_diver ipif_leader

    puts "---------------------------------------------------------------------"
    puts "Simulation summary"
    puts "ROV addr = [$ipif_rov addr]"
    puts "Leader addr = [$ipif_leader addr]"
    for {set i 0} {$i < $opt(n_diver)} {incr i}  {
        puts "Diver addr = [$ipif_diver($i) addr]"
        puts "Packets in buffer [$mac_diver($i) get_buffer_size]"
    }
    puts "number of divers  : $opt(n_diver)"
    puts "simulation length: $opt(txduration) s"
    puts "---------------------------------------------------------------------"

    set sum_cbr_throughput     0
    set sum_cbr_sent_pkts      0.0
    set sum_cbr_rcv_pkts       0.0

    set sum_cbr_throughput2     0
    set sum_cbr_sent_pkts2      0.0
    set sum_cbr_rcv_pkts2       0.0   

    set sum_cbr_throughput3     0
    set sum_cbr_sent_pkts3      0.0
    set sum_cbr_rcv_pkts3       0.0 

    set sum_cbr_throughput4     0
    set sum_cbr_sent_pkts4      0.0
    set sum_cbr_rcv_pkts4       0.0

    for {set i 0} {$i < $opt(n_diver)} {incr i}  {
        set cbr_throughput           [$cbr_rov($i) getthr]
        set cbr_sent_pkts        [$cbr_diver($i) getsentpkts]
        set cbr_rcv_pkts           [$cbr_rov($i) getrecvpkts]

        set cbr_throughput2           [$cbr2_diver($i) getthr]
        set cbr_sent_pkts2        [$cbr2_leader($i) getsentpkts]
        set cbr_rcv_pkts2           [$cbr2_diver($i) getrecvpkts]

        set cbr_throughput3           [$cbr3_rov($i) getthr]
        set cbr_sent_pkts3        [$cbr3_diver($i) getsentpkts]
        set cbr_rcv_pkts3           [$cbr3_rov($i) getrecvpkts]

        set cbr_throughput4           [$cbr4_rov($i) getthr]
        set cbr_sent_pkts4        [$cbr4_diver($i) getsentpkts]
        set cbr_rcv_pkts4           [$cbr4_rov($i) getrecvpkts]

        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts   [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]

        set sum_cbr_throughput2 [expr $sum_cbr_throughput2 + $cbr_throughput2]
        set sum_cbr_sent_pkts2  [expr $sum_cbr_sent_pkts2 + $cbr_sent_pkts2]
        set sum_cbr_rcv_pkts2   [expr $sum_cbr_rcv_pkts2 + $cbr_rcv_pkts2]

        set sum_cbr_throughput3 [expr $sum_cbr_throughput3 + $cbr_throughput3]
        set sum_cbr_sent_pkts3  [expr $sum_cbr_sent_pkts3 + $cbr_sent_pkts3]
        set sum_cbr_rcv_pkts3  [expr $sum_cbr_rcv_pkts3 + $cbr_rcv_pkts3]

        set sum_cbr_throughput4 [expr $sum_cbr_throughput4 + $cbr_throughput4]
        set sum_cbr_sent_pkts4  [expr $sum_cbr_sent_pkts4 + $cbr_sent_pkts4]
        set sum_cbr_rcv_pkts4   [expr $sum_cbr_rcv_pkts4 + $cbr_rcv_pkts4]
    }

    set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + [$cbr_leader getsentpkts]]
    set sum_cbr_sent_pkts3 [expr $sum_cbr_sent_pkts3 + [$cbr3_leader getsentpkts]]
    set sum_cbr_sent_pkts4 [expr $sum_cbr_sent_pkts4 + [$cbr4_leader getsentpkts]]

    set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + [$cbr_rov($opt(n_diver)) getrecvpkts]]
    set sum_cbr_rcv_pkts3  [expr $sum_cbr_rcv_pkts3 + [$cbr3_rov($opt(n_diver)) getrecvpkts]]
    set sum_cbr_rcv_pkts4  [expr $sum_cbr_rcv_pkts4 + [$cbr4_rov($opt(n_diver)) getrecvpkts]]

    set sum_cbr_throughput  [expr $sum_cbr_throughput + [$cbr_rov($opt(n_diver)) getthr]]
    set sum_cbr_throughput3  [expr $sum_cbr_throughput3 + [$cbr3_rov($opt(n_diver)) getthr]]
    set sum_cbr_throughput4  [expr $sum_cbr_throughput4 + [$cbr4_rov($opt(n_diver)) getthr]]

    set sum_cbr_throughput2       [expr [$cbr2_rov getthr] + $sum_cbr_throughput2]
    set sum_cbr_rcv_pkts2         [expr [$cbr2_rov getrecvpkts] + $sum_cbr_rcv_pkts2]
    set sum_cbr_sent_pkts2        [expr $sum_cbr_sent_pkts2 + [$cbr2_leader($opt(n_diver)) getsentpkts]]

    set ipheadersize        [$ipif_leader getipheadersize]
    set udpheadersize       [$udp_leader getudpheadersize]
    set cbrheadersize       [$cbr_leader getcbrheadersize]


    puts "IP Pkt Header Size       : $ipheadersize"
    puts "UDP Header Size          : $udpheadersize"
    puts "CBR Header Size          : $cbrheadersize"
    
    puts "Traffic 1 ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput/(1+$opt(n_diver)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts"
    puts "Received Packets         : $sum_cbr_rcv_pkts"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"

    puts "Traffic 2 ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput2/($opt(n_diver)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts2"
    puts "Received Packets         : $sum_cbr_rcv_pkts2"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts2 / $sum_cbr_sent_pkts2 * 100]"


    puts "Traffic 3 ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput3/(1+$opt(n_diver)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts3"
    puts "Received Packets         : $sum_cbr_rcv_pkts3"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts3 / $sum_cbr_sent_pkts3 * 100]"

    puts "Traffic 4 ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput4/($opt(n_diver)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts4"
    puts "Received Packets         : $sum_cbr_rcv_pkts4"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts4 / $sum_cbr_sent_pkts4 * 100]"
  
    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run
