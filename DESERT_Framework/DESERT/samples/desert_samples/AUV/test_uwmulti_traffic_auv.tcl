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
load libuwsmeposition.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwposbasedrt.so
#load libuwnoderep.so;                  #non riesco ad importarla
#load libuwsecurity_clmsg.so;           #non riesco ad importarla
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
set opt(n_auv)                 2 ;# Number of Nodes
set opt(starttime)          1
set opt(stoptime)           15000
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(rngstream)            1

set opt(maxinterval_)       20.0
set opt(freq)               25000.0
set opt(bw)                 5000.0
#set opt(freq_hf)               60000.0
#set opt(bw_hf)                 10000.0
set opt(bitrate)            4800.0
#set opt(bitrate_hf)            64000.0
set opt(ack_mode)           "setNoAckMode"


set opt(txpower)            135.0 
#set opt(txpower_hf)         135.0 

set opt(tdma_frame) 20
set opt(tdma_gard)  1

set opt(pktsize)    125
set opt(ctr_period) 60

set opt(pktsize_monitoring) 50
set opt(auv_period) 40

#set opt(pktsize_control) 100
#set opt(cbr_period_control)  [expr $opt(cbr_period_monitoring)*4]

set opt(pktsize_error) 1000
set opt(auv_period_error)  [expr $opt(auv_period)*5]

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
set opt(LUTpath)           "..dbs/optical_noise/LUT.txt"
set opt(rngstream) 13
#set opt(ctr_period) 60

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
        set opt(ctr_period) [lindex $argv 0]
        set opt(rngstream) [lindex $argv 1];
    }   
} 

set opt(waypoint_file)  "../dbs/wp_path/rov_path.csv"

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

#set data_mask_hf [new MSpectralMask/Rect]
#$data_mask_hf setFreq       $opt(freq_hf)
#$data_mask_hf setBandwidth  $opt(bw_hf)

#########################
# Module Configuration  #
#########################
#UW/AUV
Module/UW/AUV set packetSize_          $opt(pktsize)
Module/UW/AUV set period_              $opt(auv_period)
Module/UW/AUV set PoissonTraffic_      1
Module/UW/AUV set debug_               0

#UW/AUV/ERR
Module/UW/AUV/ERR set packetSize_          $opt(pktsize)
Module/UW/AUV/ERR set period_              $opt(auv_period)
Module/UW/AUV/ERR set PoissonTraffic_      1
Module/UW/AUV/ERR set debug_               0

#UW/AUV/CERR
Module/UW/AUV/CER set packetSize_          $opt(pktsize)
Module/UW/AUV/CER set period_              $opt(auv_period)
Module/UW/AUV/CER set PoissonTraffic_      1
Module/UW/AUV/CER set debug_               0


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
Module/UW/TDMA set tot_slots        $opt(n_auv)

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
#createROV [expr $opt(n_auv) + 1]

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
    #for {set id2 0} {$id2 < $opt(n_auv)} {incr id2}  {
    #    if {$id1 != $id2} {
            connectNodes $id1
    #    }
    #}
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  {
    #for {set id2 0} {$id2 < $opt(n_auv)} {incr id2}  {
    #  $mll_auv($id1) addentry [$ipif_auv($id2) addr] [$mac_auv($id2) addr]
    #  $mll_op_auv($id1) addentry [$ipif_auv($id2) addr] [$mac_op_auv($id2) addr]
    #}   
    $mll_auv($id1) addentry [$ipif_asv addr] [$mac_asv addr]
    # $mll_diver($id1) addentry [$ipif_rov addr] [$mac_rov addr]
    $mll_op_auv($id1) addentry [$ipif_asv addr] [$mac_op_asv addr]
    # $mll_hf_diver($id1) addentry [$ipif_rov addr] [$mac_rov addr]
    $mll_asv addentry [$ipif_auv($id1) addr] [$mac_auv($id1) addr]
    $mll_op_asv addentry [$ipif_auv($id1) addr] [$mac_op_auv($id1) addr]
}

#$mll_asv addentry [$ipif_rov addr] [$mac_rov addr]
#$mll_op_asv addentry [$ipif_rov addr] [$mac_op_rov addr]

#$mll_rov addentry [$ipif_leader addr] [$mac_leader addr]
#$mll_op_rov addentry [$ipif_leader addr] [$mac_op_leader addr]
Position/UWSME debug_ 1
# Setup positions
set position_asv [new "Position/UWSME"]
$position_asv setX_ 0
$position_asv setY_ 0
$position_asv setZ_ -1

#$position_rov setX_ 0
#$position_rov setY_ 10
#$position_rov setZ_ -1000

for {set id 0} {$id < $opt(n_auv)} {incr id}  { 

    #$cbr_asv($id) setPosition $position_asv
    $asv_app($id) setPosition $position_asv 
    $asv_err($id) setPosition $position_asv 

    Position/UWSME debug_ 1

    set position_auv($id) [new "Position/UWSME"]
    #$position_auv($id) setX_ [expr -50 * (1 + $id % $opt(n_auv)/2) - 25 + 50 * rand()]
    $position_auv($id) setX_ 0
    #$position_auv($id) setY_ [expr -50 + 100 * ($id%2) - 25 + 50 * rand()]
    $position_auv($id) setY_  0
    $position_auv($id) setZ_ -1000

    $auv_app($id) setPosition $position_auv($id) 
    $auv_err($id) setPosition $position_auv($id) 

    puts "x = [$position_auv($id) getX_]; y = [$position_auv($id) getY_]; z = [$position_auv($id) getZ_]"
}


# Setup routing table
for {set id 0} {$id < $opt(n_auv)} {incr id}  {
    $ipr_auv($id) addRoute [$ipif_asv addr] [$ipif_asv addr]
    $ipr_asv addRoute [$ipif_auv($id) addr] [$ipif_auv($id) addr]

    # $ipr2_diver($id) addRoute [$ipif2_rov addr] [$ipif2_leader addr]
    # $ipr2_leader addRoute [$ipif2_diver($id) addr] [$ipif2_diver($id) addr]
}
#$ipr_leader addRoute [$ipif_rov addr] [$ipif_rov addr]


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
        for {set id 0} {$id < $opt(n_auv)} {incr id}  {  
		    $ns at $t "$asv_app($id) sendPosition [expr $x*($id*(-2))+$x] [expr $y*($id*(-2))+$y] [expr $z]"
        }
    }
}


#$ns at $opt(starttime)    "$cbr3_asv($opt(n_auv)) start"
#$ns at $opt(stoptime)     "$cbr3_asv($opt(n_auv)) stop"

for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  {
    #$ns at $opt(starttime)    "$cbr_asv($id1) start"
    #$ns at $opt(stoptime)     "$cbr_asv($id1) stop"

    $ns at $opt(starttime)    "$asv_app($id1) start"
    $ns at $opt(stoptime)     "$asv_app($id1) stop"

    $ns at $opt(starttime)    "$auv_app($id1) start"
    $ns at $opt(stoptime)     "$auv_app($id1) stop"

    $ns at $opt(starttime)    "$auv_err($id1) start"
    $ns at $opt(stoptime)     "$auv_err($id1) stop"

    $ns at $opt(starttime)    "$asv_err($id1) start"
    $ns at $opt(stoptime)     "$asv_err($id1) stop"

    #$ns at $opt(starttime)    "$cbr3_auv($id1) start"
    #$ns at $opt(stoptime)     "$cbr3_auv($id1) stop"
    
    #$ns at $opt(starttime)    "$mac_auv($id1) start"
    #$ns at $opt(stoptime)     "$mac_auv($id1) stop"

    #$ns at $opt(starttime)    "$mac_op_auv($id1) start"
    #$ns at $opt(stoptime)     "$mac_op_auv($id1) stop"

    
}

#$ns at $opt(starttime)    "$mac_asv start"
#$ns at $opt(stoptime)     "$mac_asv stop"

#$ns at $opt(starttime)    "$mac_op_asv start"
#$ns at $opt(stoptime)     "$mac_op_asv stop"

proc update_and_check {t} {
    set outfile_auv0 [open "test_uwauv0_results.csv" "a"]
    set outfile_auv1 [open "test_uwauv1_results.csv" "a"]
    set outfile_asv [open "test_uwasv_results.csv" "a"]
    global position_auv position_asv auv_app asv_app opt n_auv auv_err asv_err
    #for {set id1 0} {$id1 < $opt(n_auv)} {incr id1}  { 
    #    $position_auv($id1) update
    #    $position_asv update
    #    puts $outfile "positions AUV: x( $id1 ) = [$auv_app($id1) getX], y = [$auv_app($id1) getY], z =  [$auv_app($id1) getZ]" 
    #    puts $outfile "positions AUV_ERR: x( $id1 ) = [$auv_err($id1) getX], y = [$auv_err($id1) getY], z =  [$auv_err($id1) getZ]"  
    #}

    $position_auv(0) update
    $position_auv(1) update
    $position_asv update

    puts $outfile_auv0 "$t,[$auv_app(0) getX],[$auv_app(0) getY],[$auv_app(0) getZ]" 
    puts $outfile_auv1 "$t,[$auv_app(1) getX],[$auv_app(1) getY],[$auv_app(1) getZ]" 
    puts $outfile_asv "$t,[$asv_app(0) getX],[$asv_app(0) getY],[$asv_app(0) getZ]"
      #puts "positions AUV: x = [$applicationAUV getX], y = [$applicationAUV getY], z =  [$applicationAUV getZ]"
    
    close $outfile_auv0
    close $outfile_auv1
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


    puts "---------------------------------------------------------------------"
    puts "Simulation summary"
    puts "Leader addr = [$ipif_asv addr]"
    for {set i 0} {$i < $opt(n_auv)} {incr i}  {
        puts "Diver addr = [$ipif_auv($i) addr]"
        #puts "Packets in buffer [$mac_auv($i) get_buffer_size]"
    }
    puts "number of divers  : $opt(n_auv)"
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


    for {set i 0} {$i < $opt(n_auv)} {incr i}  {
        set cbr_throughput           [$asv_app($i) getthr]
        set cbr_sent_pkts        [$auv_app($i) getsentpkts]
        set cbr_rcv_pkts           [$asv_app($i) getrecvpkts]

        set cbr_throughput2           [$auv_app($i) getthr]
        set cbr_sent_pkts2        [$asv_app($i) getsentpkts]
        set cbr_rcv_pkts2           [$auv_app($i) getrecvpkts]

        set cbr_throughput4           [$asv_err($i) getthr]
        set cbr_sent_pkts4        [$auv_err($i) getsentpkts]
        set cbr_rcv_pkts4           [$asv_err($i) getrecvpkts]

        #set cbr_throughput3           [$cbr3_asv($i) getthr]
        #set cbr_sent_pkts3        [$cbr3_auv($i) getsentpkts]
        #set cbr_rcv_pkts3           [$cbr3_asv($i) getrecvpkts]

        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts   [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]

        set sum_cbr_throughput2 [expr $sum_cbr_throughput2 + $cbr_throughput2]
        set sum_cbr_sent_pkts2  [expr $sum_cbr_sent_pkts2 + $cbr_sent_pkts2]
        set sum_cbr_rcv_pkts2   [expr $sum_cbr_rcv_pkts2 + $cbr_rcv_pkts2]

        set sum_cbr_throughput4 [expr $sum_cbr_throughput4 + $cbr_throughput4]
        set sum_cbr_sent_pkts4  [expr $sum_cbr_sent_pkts4 + $cbr_sent_pkts4]
        set sum_cbr_rcv_pkts4   [expr $sum_cbr_rcv_pkts4 + $cbr_rcv_pkts4]
    }

    #set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + [$cbr_asv($opt(n_auv)) getsentpkts]]
    #set sum_cbr_sent_pkts3 [expr $sum_cbr_sent_pkts3 + [$cbr3_asv getsentpkts]]

    #set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + [$cbr_asv($opt(n_auv)) getrecvpkts]]
    #set sum_cbr_rcv_pkts3  [expr $sum_cbr_rcv_pkts3 + [$cbr3_asv($opt(n_auv)) getrecvpkts]]

    #set sum_cbr_throughput  [expr $sum_cbr_throughput + [$cbr_asv($opt(n_auv)) getthr]]
    #set sum_cbr_throughput3  [expr $sum_cbr_throughput3 + [$cbr3_asv($opt(n_auv)) getthr]]

    #set sum_cbr_throughput2       [expr [$cbr2_asv(0) getthr] + $sum_cbr_throughput2]
    #set sum_cbr_rcv_pkts2         [expr [$cbr2_asv(0) getrecvpkts] + $sum_cbr_rcv_pkts2]
    #set sum_cbr_sent_pkts2        [expr $sum_cbr_sent_pkts2 + [$cbr2_asv($opt(n_auv)) getsentpkts]]

    set ipheadersize        [$ipif_asv getipheadersize]
    set udpheadersize       [$udp_asv getudpheadersize]
    set cbrheadersize       [$asv_app(0) getcbrheadersize]


    puts "IP Pkt Header Size       : $ipheadersize"
    puts "UDP Header Size          : $udpheadersize"
    puts "CBR Header Size          : $cbrheadersize"
    
    puts "Traffic MONITORING ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput/(1+$opt(n_auv)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts"
    puts "Received Packets         : $sum_cbr_rcv_pkts"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"

    puts "Traffic CONTROL ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput2/($opt(n_auv)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts2"
    puts "Received Packets         : $sum_cbr_rcv_pkts2"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts2 / $sum_cbr_sent_pkts2 * 100]"

    puts "Traffic ERROR  ---------------------------------------------"
    puts "Mean Throughput          : [expr ($sum_cbr_throughput4/($opt(n_auv)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts4"
    puts "Received Packets         : $sum_cbr_rcv_pkts4"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts4 / $sum_cbr_sent_pkts4 * 100]"


    #puts "Traffic 3 ---------------------------------------------"
    #puts "Mean Throughput          : [expr ($sum_cbr_throughput3/(1+$opt(n_diver)))]"
    #puts "Sent Packets             : $sum_cbr_sent_pkts3"
    #puts "Received Packets         : $sum_cbr_rcv_pkts3"
    #puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts3 / $sum_cbr_sent_pkts3 * 100]"

    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
$ns at [expr $opt(stoptime) + 250.0]  "finish; $ns halt" 
$ns run
