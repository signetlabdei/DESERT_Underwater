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
# Author: Alessia Ortile <alessia.ortile.1@studenti.unipd.it>
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
# This script simulates the behavior of n AUVs and 1 ASV. The AUVs follow a fixed trajectory 
# to inspect an area and generate errors with a certain probability. The ASV and AUVs share 
# information about the errors to collaboratively resolve them using the IER method.
# ----------------------------------------------------------------------------------
# Stack
#             AUV                                  ASV                  
#   +------------------------------+   +------------------------------+   
#   |  7. UW/AUV      | UW/AUV/ERR |   |  7. UW/AUV      | UW/AUV/CER | 
#   +------------------------------+   +------------------------------+   
#   |  6. UW/UDP                   |   |  6. UW/UDP                   |   
#   +------------------------------+   +------------------------------+   
#   |  5. UW/STATICROUTING         |   |  5. UW/STATICROUTING         |   
#   +------------------------------+   +------------------------------+   
#   |  4. UW/IP                    |   |  4. UW/IP                    |   
#   +------------------------------+   +------------------------------+
#   |  4. MULTI_TRAFFIC_RANGE      |   |  4. MULTI_TRAFFIC_RANGE      |
#   +------------------------------+   +------------------------------+
#   |  3. UW/MLL                   |   |  3. UW/MLL                   |   
#   +------------------------------+   +------------------------------+   
#   |  2. UW/CSMA_ALOHA            |   |  2. UW/CSMA_ALOHA            |   
#   +------------------------------+   +------------------------------+   
#   |  1. UW/PHYSICAL | UW/OPTICAL |   |  1. UW/PHYSICAL | UW/OPTICAL |     
#   +------------------------------+   +------------------------------+   
#            |         |                    |         |                   
#   +------------------------------------------------------------------+
#   |                            UnderwaterChannel                     |
#   +------------------------------------------------------------------+
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
load libuwsmwpposition.so
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
load libuwoptical_channel.so
load libuwoptical_phy.so
load libuwmulti_traffic_control.so
load libuwauv.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(n_auv)                4 ;# Number of Nodes
set opt(starttime)            1
set opt(stoptime)             150000;#150000
set opt(txduration)           [expr $opt(stoptime) - $opt(starttime)]
set opt(rngstream)            1

set opt(maxinterval_)         20.0
set opt(freq)                 20000.0 
set opt(bw)                   10000.0
set opt(bitrate)              4800.0
set opt(ack_mode)             "setNoAckMode"


set opt(txpower)              175.0 
set opt(pktsize)              125
set opt(pktsize_monitoring)   5
set opt(period)               60

set opt(op_freq)              10000000
set opt(op_bw)                100000
set opt(bitrate_op)           1000000
set opt(txpower_op)           50; #30
set opt(acq_db_op)            10
set opt(temperatura)          293.15 ; # in Kelvin
set opt(txArea)               0.000010
 # receveing area, it has to be the same for optical physical and propagation
set opt(rxArea)               0.0000011 ;
set opt(c)                    0.4; #0.4 coastal water
set opt(theta)                1
set opt(id)                   [expr 1.0e-9]
set opt(il)                   [expr 1.0e-6]
set opt(shuntRes)             [expr 1.49e9]
set opt(sensitivity)          0.26
set opt(LUTpath)              "..dbs/optical_noise/LUT.txt"
set opt(rngstream)            13

set opt(accuracy)             0.001
set opt(variance)             0.01
set opt(e_prob)               0.01

#set opt(ctr_period) 60

if {$opt(bash_parameters)} {
    if {$argc != 4} {
        puts "The script requires 4 inputs:"
        puts "- the first for accuracy"
        puts "- the second for variance"
        puts "- the third error probability"
        puts "- the fourth for the rngstream"
        puts "example: ns test_uwmultitraffic_auv.tcl 0.001 0.01 0.01 5"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        return
    } else {
        set opt(accuracy) [lindex $argv 0]
        set opt(variance) [lindex $argv 1]
        set opt(e_prob) [lindex $argv 2]
        set opt(rngstream) [lindex $argv 3];
    }   
} 

set opt(waypoint_file)  "../dbs/wp_path/rov_path_square.csv"

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

MPropagation/Underwater set shipping_ 1
MPropagation/Underwater set windspeed_ 5
MPropagation/Underwater set debug_ 0
MPropagation/Underwater set practicalSpreading_ 1.5

set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

#########################
# Module Configuration  #
#########################

Module/UW/IP set debug_                      0
Module/UW/UDP set debug_                     0

#TRAFFIC_CTR
Module/UW/MULTI_TRAFFIC_RANGE_CTR set debug_            0
Module/UW/MULTI_TRAFFIC_RANGE_CTR set check_to_period_  5

Module/UW/CSMA_ALOHA set wait_costant_ 0.01
Module/UW/CSMA_ALOHA set listen_time_ 0.01

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

source "auv.tcl"
source "asv.tcl"

for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  {
    createAUV $id1
}
createASV $opt(n_auv) 

################################
# Inter-node_leader module connection #
################################

proc connectNodes {id1} {
    global ipif_asv ipr_asv portnum_asv portnum_auv portnum2_asv portnum2_auv
    global asv_app asv_err
    global ipif_auv auv_app auv_err
    $asv_app($id1) set destAddr_ [$ipif_auv($id1) addr]
    $asv_app($id1) set destPort_ $portnum_auv($id1)

    $asv_err($id1) set destAddr_ [$ipif_auv($id1) addr]
    $asv_err($id1) set destPort_ $portnum2_auv($id1)
    

    $auv_app($id1) set destAddr_ [$ipif_asv addr]
    $auv_app($id1) set destPort_ $portnum_asv($id1)

    $auv_err($id1) set destAddr_ [$ipif_asv addr]
    $auv_err($id1) set destPort_ $portnum2_asv($id1)   
}

# Setup flows

for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  {
    connectNodes $id1
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  {
  
    $mll_auv($id1) addentry [$ipif_asv addr] [$mac_asv addr]
    $mll_op_auv($id1) addentry [$ipif_asv addr] [$mac_op_asv addr]
    $mll_asv addentry [$ipif_auv($id1) addr] [$mac_auv($id1) addr]
    $mll_op_asv addentry [$ipif_auv($id1) addr] [$mac_op_auv($id1) addr]
}

Position/UWSMWP debug_ 1

# Setup positions
set position_asv [new "Position/UWSMWP"]
$position_asv setX_ 0
$position_asv setY_ 0
$position_asv setZ_ -1

for {set id 0} {$id < $opt(n_auv)} {incr id}  { 

    $asv_app($id) setPosition $position_asv 
    $asv_err($id) setPosition $position_asv 
}

$position_auv(0) setX_ -1
$position_auv(0) setY_  1

$position_auv(1) setX_ 1
$position_auv(1) setY_  -1

$position_auv(2) setX_ 1
$position_auv(2) setY_  1

$position_auv(3) setX_ -1
$position_auv(3) setY_  -1

for {set id 0} {$id < $opt(n_auv)} {incr id}  { 

    set position_auv($id) [new "Position/UWSMWP"]
    $position_auv($id) setZ_ -100

    $auv_app($id) setPosition $position_auv($id) 
    $auv_err($id) setPosition $position_auv($id) 

    #puts "x = [$position_auv($id) getX_]; y = [$position_auv($id) getY_]; z = [$position_auv($id) getZ_]"
}


# Setup routing table
for {set id 0} {$id < $opt(n_auv)} {incr id}  {
    $ipr_auv($id) addRoute [$ipif_asv addr] [$ipif_asv addr]
    $ipr_asv addRoute [$ipif_auv($id) addr] [$ipif_auv($id) addr]
}

#####################  
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g.,

set outfile [open "test_uwauv_results.csv" "w"]
close $outfile
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
foreach line $data {
	if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {
        $ns at $t "update_and_check $t"
        $ns at $t "$asv_app(0) sendPosition [expr -$x] [expr $y] [expr $z]"
        $ns at $t "$asv_app(1) sendPosition [expr $x] [expr -$y] [expr $z]"
        $ns at $t "$asv_app(2) sendPosition [expr $x] [expr $y] [expr $z]"
        $ns at $t "$asv_app(3) sendPosition [expr -$x] [expr -$y] [expr $z]"
        #for {set id 0} {$id < $opt(n_auv)} {incr id}  {  
		#    $ns at $t "$asv_app($id) sendPosition [expr $x*($id*(2))-$x] [expr $y*($id*(-2))+$y] [expr $z]"
        #}
    }
}

set min 0
set max 100

for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  {

    set time_ [new RandomVariable/Uniform]
    $time_ set min_ 0
    $time_ set max_ 100
    $time_ use-rng $defaultRNG

    $ns at [expr $opt(starttime) + [$time_ value]]    "$asv_app($id1) start"
    $ns at $opt(stoptime)     "$asv_app($id1) stop"

    $ns at [expr $opt(starttime) + [$time_ value]]   "$auv_app($id1) start"
    $ns at $opt(stoptime)     "$auv_app($id1) stop"

    $ns at [expr $opt(starttime) + [$time_ value]]   "$auv_err($id1) start"
    $ns at $opt(stoptime)     "$auv_err($id1) stop"

    $ns at [expr $opt(starttime) + [$time_ value]]    "$asv_err($id1) start"
    $ns at $opt(stoptime)     "$asv_err($id1) stop"
    
}

proc update_and_check {t} {
    set outfile_auv [open "test_uwauv0_results.csv" "a"]
    set outfile_auv1 [open "test_uwauv1_results.csv" "a"]
    set outfile_asv [open "test_uwasv_results.csv" "a"]
    global position_auv position_asv auv_app asv_app opt n_auv auv_err asv_err
    for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  { 
        $position_auv($id1) update
        puts $outfile_auv "positions AUV($id1): x = [$auv_app($id1) getX], y = [$auv_app($id1) getY], z =  [$auv_app($id1) getZ]" 
    }


    $position_asv update
    puts $outfile_asv "$t,[$asv_app(0) getX],[$asv_app(0) getY],[$asv_app(0) getZ]"
      #puts "positions AUV: x = [$applicationAUV getX], y = [$applicationAUV getY], z =  [$applicationAUV getZ]"
    
    close $outfile_auv
    close $outfile_asv


    
}
###

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt n_auv
    global mac propagation phy_data channel db_manager propagation
    global node_asv_coordinates 
    global ipr_asv ipif_asv udp_asv phy 
    global asv_app asv_err
    global auv_app auv_err mac_auv
    global node_asv_stats tmp_node_asv_stats ipif_auv ipif_asv

    set sum_asv_ctr_throughput     0
    set sum_asv_ctr_sent_pkts      0.0
    set sum_asv_ctr_rcv_pkts       0.0

    set sum_auv_mon_throughput     0
    set sum_auv_mon_sent_pkts      0.0
    set sum_auv_mon_rcv_pkts       0.0   

    set sum_asv_err_throughput     0
    set sum_asv_err_sent_pkts      0.0
    set sum_asv_err_rcv_pkts       0.0

    set sum_auv_err_throughput     0
    set sum_auv_err_sent_pkts      0.0
    set sum_auv_err_rcv_pkts       0.0  


    for {set i 0} {$i < $opt(n_auv)} {incr i}  {
        set asv_ctr_thr           [$asv_app($i) getthr]
        set asv_ctr_sent_pkts        [$asv_app($i) getsentpkts]
        set asv_ctr_rcv_pkts           [$asv_app($i) getrecvpkts]

        set auv_mon_thr           [$auv_app($i) getthr]
        set auv_mon_sent_pkts        [$auv_app($i) getsentpkts]
        set auv_mon_rcv_pkts           [$auv_app($i) getrecvpkts]

        set asv_err_thr           [$asv_err($i) getthr]
        set asv_err_sent_pkts        [$asv_err($i) getsentpkts]
        set asv_err_rcv_pkts           [$asv_err($i) getrecvpkts]

        set auv_err_thr           [$auv_err($i) getthr]
        set auv_err_sent_pkts        [$auv_err($i) getsentpkts]
        set auv_err_rcv_pkts           [$auv_err($i) getrecvpkts]

        set sum_asv_ctr_throughput [expr $sum_asv_ctr_throughput + $asv_ctr_thr]
        set sum_asv_ctr_sent_pkts  [expr $sum_asv_ctr_sent_pkts + $asv_ctr_sent_pkts]
        set sum_asv_ctr_rcv_pkts   [expr $sum_asv_ctr_rcv_pkts + $asv_ctr_rcv_pkts]

        set sum_auv_mon_throughput [expr $sum_auv_mon_throughput + $auv_mon_thr]
        set sum_auv_mon_sent_pkts  [expr $sum_auv_mon_sent_pkts + $auv_mon_sent_pkts]
        set sum_auv_mon_rcv_pkts   [expr $sum_auv_mon_rcv_pkts + $auv_mon_rcv_pkts]

        set sum_asv_err_throughput [expr $sum_asv_err_throughput + $asv_err_thr]
        set sum_asv_err_sent_pkts  [expr $sum_asv_err_sent_pkts + $asv_err_sent_pkts]
        set sum_asv_err_rcv_pkts   [expr $sum_asv_err_rcv_pkts + $asv_err_rcv_pkts]

        set sum_auv_err_throughput [expr $sum_auv_err_throughput + $auv_err_thr]
        set sum_auv_err_sent_pkts  [expr $sum_auv_err_sent_pkts + $auv_err_sent_pkts]
        set sum_auv_err_rcv_pkts   [expr $sum_auv_err_rcv_pkts + $auv_err_rcv_pkts]
    }

    puts "$sum_asv_ctr_sent_pkts $sum_asv_ctr_rcv_pkts $sum_auv_mon_sent_pkts $sum_auv_mon_rcv_pkts $sum_asv_err_sent_pkts $sum_asv_err_rcv_pkts $sum_auv_err_sent_pkts $sum_auv_err_rcv_pkts"
    
    
    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run
