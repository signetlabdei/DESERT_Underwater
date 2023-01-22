#
# Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
# This script is used to test uwrov in a multihop scenario. The ROV is 
#remotely controlled by a wireless telemetry system. The controller sends 
#command packets containing the next way-point to the rov, meanwhile the rov
# send monitoring packets to the base station.
# N.B.: UnderwaterChannel and UW/PHYSICAL are used for PHY layer and channel,
# while an unbalanced TDMA is employed as datalink.
# The way-point list is imported by an external file. 
###########################################################################
#
# Author: William Rizzo <william.rizzo.wr@gmail.com>
# Version: 1.0.0
#
# NOTE: tcl sample tested on Mint 18, 64 bits OS
#
# Stack of the ROV, the relay and the CTR
#
#                                +-------------------------------+    
#                                | 11. UW/CBR                    |    
#   +----------------------+     +-------------------------------+     +---------------------+
#   |  7. UW/ROV           |     | 10. UW/UDP                    |     | 7. UW/ROV/CTR       |
#   +----------------------+     +-------------------------------+     +---------------------+
#   |  6. UW/UDP           |     | 9. UW/STATICROUTING           |     | 6. UW/UDP           | 
#   +----------------------+     +-------------------------------+     +---------------------+
#   |  5. UW/STATICROUTING |     | 8. UW/IP                      |     | 5. UW/STATICROUTING | 
#   +----------------------      +-------------------------------+     +---------------------+
#   |  4. UW/IP            |     | 7. UW/MULTI_DESTINATION       |     | 4. UW/IP            |
#   +----------------------+     +-------------------------------+     +---------------------+
#   |  3. UW/MLL           |     | 6.UW/MLL      | 5.UW/MLL      |     | 3. UW/MLL           |
#   +----------------------+     +-------------------------------+     +---------------------+
#   |  2. UW/TDMA          |     | 4.UW/TDMA     | 3.UW/TDMA     |     | 2. UW/TDMA          |
#   +----------------------+     +-------------------------------+     +---------------------+
#   |  1. UW/PHYSICAL      |     | 2.UW/PHYSICAL | 1.UW/PHYSICAL |     | 1. UW/PHYSICAL      |
#   +----------------------+     +-------------------------------+     +---------------------+
#   +----------------------------------------------------------------------------------------+                  
#   |                                     UnderwaterChannel                                  |  
#   +----------------------------------------------------------------------------------------+ 

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 			1
set opt(trace_files)		0
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
load libuwmmac_clmsgs.so
load libuwtdma.so
load libuwcbr.so
load libuwaloha.so
load libuwcsmaaloha.so
load libuwinterference.so
load libUwmStd.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwhermesphy.so
load libuwmulti_destination.so

# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
set opt(nn)                 3; # Number of Nodes
set opt(rn)                 [expr int($opt(nn) - 2)]; # Number of Relay nodes
set opt(ROV_pktsize)        1000; # Pkt size in byte
set opt(CTR_pktsize)        1000; # Pkt size in byte

set opt(ROV_period)         40;
set opt(CTR_period)         50;
set opt(guard_time)         50; #time between 2 waypoints

set opt(starttime)          1;
set opt(stoptime)           30000;
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]; # Duration of the simulation

set opt(propagation_speed)  1500.0; # m/s
set opt(maxinterval_)       200;

set opt(max_distance_rov)   3000   ;#Maximum distance between nodes in meter
set opt(max_distance_relay) 7000    ;#Maximum distance between nodes in meter
set opt(rngstream) 1
######################
#   Evologics 7-17   #      # used in CTR and Relay
######################
set opt(evo717_freq)            12000;    # Frequency used in Hz
set opt(evo717_bw)              5000.0;     # Bandwidth used in Hz
set opt(evo717_bitrate)         2400.0 ;     # bitrate in bps
set opt(evo717_txpower)         187.8;    # Power transmitted in dB re uPa
set opt(evo717_max_txrange)     8000;     # maximum transmission range

#########################
#  Evologics S2C 18/34  #   # used in all the nodes
#########################
set opt(evo1834_freq)            26000;    # Frequency used in Hz
set opt(evo1834_bw)              8000;    # Bandwidth used in Hz
set opt(evo1834_bitrate)         4800.0;     # bitrate in bps
set opt(evo1834_txpower)         184;     # Power transmitted in dB re uPa
set opt(evo1834_max_txrange)     3500;    # maximum transmission range


set opt(waypoint_file)  "../dbs/wp_path/rov_path_multi_destination_2.csv"


if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the Poisson ROV period"
        puts "- the evo717_bitrate"
        puts "- the rngstream"
		puts "example: ns test_uwrov_multi_simple.tcl 60"
		puts "If you want to leave the default values, please set to 0"
		puts "the value opt(bash_parameters) in the tcl script"
		puts "Please try again.$argc "
		return
	} else {
        set opt(ROV_period)   [lindex $argv 0];
        set b [lindex $argv 1];
        set opt(rngstream) [lindex $argv 2];
        set opt(evo717_bitrate) [expr $b*1.0];
	}
}

#random generator

global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwrov_multi_destination_simple.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwrov_multi_destination_simple.cltr"
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
MPropagation/Underwater set shipping_           1

##################
#     Channel    #
##################

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]

set evo717_data_mask [new MSpectralMask/Rect]
$evo717_data_mask setFreq       $opt(evo717_freq)
$evo717_data_mask setBandwidth  $opt(evo717_bw)
$evo717_data_mask setPropagationSpeed  $opt(propagation_speed)

set evo1834_data_mask [new MSpectralMask/Rect]
$evo1834_data_mask setFreq       $opt(evo1834_freq)
$evo1834_data_mask setBandwidth  $opt(evo1834_bw)
$evo1834_data_mask setPropagationSpeed  $opt(propagation_speed)

#########################
# Module Configuration  #
#########################
Module/UW/ROV set packetSize_          $opt(ROV_pktsize)
Module/UW/ROV set period_              $opt(ROV_period)

Module/UW/ROV/CTR set packetSize_      $opt(CTR_pktsize)
Module/UW/ROV/CTR set period_          $opt(CTR_period)
 
Module/UW/ROV set debug_               0
Module/UW/ROV/CTR set debug_           0


Module/UW/TDMA set tot_slots            2
Module/UW/TDMA set max_packet_per_slot  1
Module/UW/TDMA set queue_size_          4
Module/UW/TDMA set drop_old_            0

Module/UW/TDMA set sea_trial_          0
Module/UW/TDMA set fair_mode           1
Module/UW/TDMA set debug_              0

#TDMA (ROV <--> Relay)
set opt(max_slot_duration_rov)  [expr $opt(max_distance_rov)/$opt(propagation_speed) + ($opt(CTR_pktsize)*8/$opt(evo1834_bitrate)) + 0.02]
set opt(slot_per_frame_rov)     2

#TDMA (CTR <--> Relay)
set opt(max_slot_duration_relay)  [expr $opt(max_distance_relay)/$opt(propagation_speed) + $opt(CTR_pktsize)*8/$opt(evo717_bitrate) + 0.02]
set opt(slot_per_frame_relay)     2

Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    3.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MinTxSPL_dB_                0
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

# modulo multidestination
Module/UW/MULTI_DESTINATION set debug_  0


################################
# Procedure(s) to create nodes #
################################
proc createROV {node id} {

    global applicationROV udp ipr ipif mll mac phy_evo1834 portnum channel
    global position interf_data evo1834_data_mask propagation opt 
    global evo_bitrate evo_txpower

    # module creation
    set applicationROV  [new Module/UW/ROV]
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    
    # TDMA ROV parameters
    Module/UW/TDMA set frame_duration       [expr $opt(slot_per_frame_rov)*$opt(max_slot_duration_rov)]
    Module/UW/TDMA set guard_time           [expr $opt(max_distance_rov)/$opt(propagation_speed) + 0.01]
    
    set mac($id)  [new Module/UW/TDMA]

    # Evologics 18-34 modem parameters
    Module/UW/PHYSICAL  set BitRate_      $opt(evo1834_bitrate)
    Module/UW/PHYSICAL  set MaxTxSPL_dB_  $opt(evo1834_txpower)
    Module/UW/PHYSICAL  set MaxTxRange_   $opt(evo1834_max_txrange)

    set phy_evo1834($id)  [new Module/UW/PHYSICAL]
    
    # module addition
    $node addModule 7 $applicationROV   1  "ROV"
    $node addModule 6 $udp($id)         1  "UDP"
    $node addModule 5 $ipr($id)         1  "IPR"
    $node addModule 4 $ipif($id)        1  "IPF" 
    $node addModule 3 $mll($id)         1  "MLL"
    $node addModule 2 $mac($id)         1  "MAC"
    $node addModule 1 $phy_evo1834($id) 0  "PHY"

    # module connection
    $node setConnection $applicationROV  $udp($id)     1
    set portnum($id) [$udp($id) assignPort $applicationROV]
    $node setConnection $udp($id)      $ipr($id)       1
    $node setConnection $ipr($id)      $ipif($id)      1
    $node setConnection $ipif($id)     $mll($id)       1
    $node setConnection $mll($id)      $mac($id)       1
    $node setConnection $mac($id)      $phy_evo1834($id)   1
    $node addToChannel  $channel       $phy_evo1834($id)   1
    
    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }

    # set the IP address of the node
    set ip_addr_value [expr $id + 1]
    $ipif($id) addr $ip_addr_value

    # set MAC address and slot number in the TDMA frame
    $mac($id) setMacAddr    $opt(nn)
    $mac($id) setSlotNumber 1
    
    # node position
    set position($id) [new "Position/UWSM"]
    $node addPosition $position($id)

    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    #Propagation model
    $phy_evo1834($id) setPropagation $propagation
    $phy_evo1834($id) setSpectralMask $evo1834_data_mask
    $phy_evo1834($id) setInterference $interf_data($id)
    $phy_evo1834($id) setInterferenceModel "MEANPOWER"
}

# procedure to create the CTR
proc createCTR {node id} {

    global applicationCTR udp ipr ipif mll mac opt propagation
    global phy_evo717 channel portnum position interf_data evo717_data_mask

    # module creation
    set applicationCTR  [new Module/UW/ROV/CTR]
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    
    # TDMA relay parameters
    Module/UW/TDMA set frame_duration       [expr $opt(slot_per_frame_relay)*$opt(max_slot_duration_relay)]
    Module/UW/TDMA set guard_time           [expr $opt(max_distance_relay)/$opt(propagation_speed) + 0.01]

    set mac($id)  [new Module/UW/TDMA]
    
    # Evologics 7-17 modem parameters
    Module/UW/PHYSICAL set BitRate_      $opt(evo717_bitrate)
    Module/UW/PHYSICAL  set MaxTxSPL_dB_  $opt(evo717_txpower)
    Module/UW/PHYSICAL  set MaxTxRange_   $opt(evo717_max_txrange)

    set phy_evo717($id)  [new Module/UW/PHYSICAL]

    # module addition
	$node addModule 7 $applicationCTR    1  "ROV"
    $node addModule 6 $udp($id)          1  "UDP"
    $node addModule 5 $ipr($id)          1  "IPR"
    $node addModule 4 $ipif($id)         1  "IPF" 
    $node addModule 3 $mll($id)          1  "MLL"
    $node addModule 2 $mac($id)          1  "MAC"
    $node addModule 1 $phy_evo717($id)   0  "PHY"

    # module connection
	$node setConnection $applicationCTR  $udp($id)     1
	set portnum($id) [$udp($id) assignPort $applicationCTR]
    $node setConnection $udp($id)      $ipr($id)       1
    $node setConnection $ipr($id)      $ipif($id)      1
    $node setConnection $ipif($id)     $mll($id)       1
    $node setConnection $mll($id)      $mac($id)       1
    $node setConnection $mac($id)      $phy_evo717($id)   1
    $node addToChannel  $channel       $phy_evo717($id)   1

    if {$id > 254} {
		puts "hostnum > 254!!! exiting"
		exit
    }

    # set the IP address of the node
    set ip_addr_value [expr $id + 1]
    $ipif($id) addr $ip_addr_value

    # set MAC address and slot number in the TDMA frame
    $mac($id) setMacAddr    1
    $mac($id) setSlotNumber 1
    
    # node position
    set position($id) [new "Position/UWSM"]
    $node addPosition $position($id)
    
    # interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0
	
    # propagation model
    $phy_evo717($id) setPropagation $propagation
    $phy_evo717($id) setSpectralMask $evo717_data_mask
    $phy_evo717($id) setInterference $interf_data($id)
    $phy_evo717($id) setInterferenceModel "MEANPOWER"
}

# procedure to create the relay
proc createRelay {id} {

    global application_relay udp_relay ipr_relay ipif_relay ctr_relay mll_relay mll_relay_2
    global mac_relay mac_relay_2 phy_relay_evo717 phy_relay_evo1834 channel portnum_relay 
    global position_relay interf_data_relay interf_data_relay_2 propagation opt node_relay
    global evo717_data_mask evo1834_data_mask evo717_bitrate evo717_txpower evo1834_bitrate 
    global evo1834_txpower

    # module creation
    set application_relay($id)  [new Module/UW/CBR]
    set udp_relay($id)  [new Module/UW/UDP]
    set ipr_relay($id)  [new Module/UW/StaticRouting]
    set ipif_relay($id) [new Module/UW/IP]
    set ctr_relay($id) [new Module/UW/MULTI_DESTINATION]
    set mll_relay($id)  [new Module/UW/MLL] 
    set mll_relay_2($id)  [new Module/UW/MLL] 
    
    # TDMA ROV parameters
    Module/UW/TDMA set frame_duration       [expr $opt(slot_per_frame_rov)*$opt(max_slot_duration_rov)]
    Module/UW/TDMA set guard_time           [expr $opt(max_distance_rov)/$opt(propagation_speed) + 0.01]

    set mac_relay($id)  [new Module/UW/TDMA]

    # Evologics 18-34 modem parameters
    Module/UW/PHYSICAL  set BitRate_      $opt(evo1834_bitrate)
    Module/UW/PHYSICAL  set MaxTxSPL_dB_  $opt(evo1834_txpower)
    Module/UW/PHYSICAL  set MaxTxRange_   $opt(evo1834_max_txrange)

    set phy_relay_evo1834($id)  [new Module/UW/PHYSICAL]

    # TDMA relay parameters
    Module/UW/TDMA set frame_duration       [expr $opt(slot_per_frame_relay)*$opt(max_slot_duration_relay)]
    Module/UW/TDMA set guard_time           [expr $opt(max_distance_relay)/$opt(propagation_speed) + 0.01]

    set mac_relay_2($id)  [new Module/UW/TDMA]

    # Evologics 7-17 modem parameters
    Module/UW/PHYSICAL  set BitRate_      $opt(evo717_bitrate)
    Module/UW/PHYSICAL  set MaxTxSPL_dB_  $opt(evo717_txpower)
    Module/UW/PHYSICAL  set MaxTxRange_   $opt(evo717_max_txrange)
    
    set phy_relay_evo717($id)  [new Module/UW/PHYSICAL]

    # module addition
    $node_relay($id) addModule 11 $application_relay($id)   1  "ROV"
    $node_relay($id) addModule 10 $udp_relay($id)           1  "UDP"
    $node_relay($id) addModule 9 $ipr_relay($id)            1  "IPR"
    $node_relay($id) addModule 8 $ipif_relay($id)           1  "IPF" 
    $node_relay($id) addModule 7 $ctr_relay($id)            1  "CTR"
    $node_relay($id) addModule 6 $mll_relay($id)            1  "MLL"
    $node_relay($id) addModule 5 $mll_relay_2($id)          1  "MLL"
    $node_relay($id) addModule 4 $mac_relay($id)            1  "MAC"
    $node_relay($id) addModule 3 $mac_relay_2($id)          1  "MAC"
    $node_relay($id) addModule 2 $phy_relay_evo1834($id)    0  "PHY"
    $node_relay($id) addModule 1 $phy_relay_evo717($id)     0  "PHY"

    # module connection
    $node_relay($id) setConnection $application_relay($id)  $udp_relay($id)    1
    set portnum_relay($id) [$udp_relay($id) assignPort $application_relay($id)]
    $node_relay($id) setConnection $udp_relay($id)      $ipr_relay($id)        1
    $node_relay($id) setConnection $ipr_relay($id)      $ipif_relay($id)       1
    $node_relay($id) setConnection $ipif_relay($id)     $ctr_relay($id)        1
    $node_relay($id) setConnection $ctr_relay($id)      $mll_relay($id)        1
    $node_relay($id) setConnection $ctr_relay($id)      $mll_relay_2($id)      1
    $node_relay($id) setConnection $mll_relay($id)      $mac_relay($id)        1
    $node_relay($id) setConnection $mac_relay($id)      $phy_relay_evo1834($id)    1
    $node_relay($id) setConnection $mll_relay_2($id)    $mac_relay_2($id)      1
    $node_relay($id) setConnection $mac_relay_2($id)    $phy_relay_evo717($id)    1
    $node_relay($id) addToChannel  $channel       $phy_relay_evo1834($id)          1
    $node_relay($id) addToChannel  $channel       $phy_relay_evo717($id)          1

    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }

    # set IP address
    set ip_addr_values [expr 254 - $id]
    $ipif_relay($id) addr $ip_addr_values

    # set MAC address and slot number in the TDMA frame
    # relay <-> ROV
    $mac_relay($id) setMacAddr [expr 2 + $id]
    $mac_relay($id) setSlotNumber 2
    # relay <-> CTR
    $mac_relay_2($id) setMacAddr [expr 2 + $id]
    $mac_relay_2($id) setSlotNumber 2
    
    # node position
    set position_relay($id) [new "Position/BM"]
    $node_relay($id) addPosition $position_relay($id)

    # interference model
    set interf_data_relay($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data_relay($id) set maxinterval_ $opt(maxinterval_)
    $interf_data_relay($id) set debug_       0

    set interf_data_relay_2($id) [new "Module/UW/INTERFERENCE"]
    $interf_data_relay_2($id) set maxinterval_ $opt(maxinterval_)
    $interf_data_relay_2($id) set debug_      0
    
    # propagation model
    $phy_relay_evo1834($id) setPropagation $propagation
    $phy_relay_evo1834($id) setSpectralMask $evo1834_data_mask
    $phy_relay_evo1834($id) setInterference $interf_data_relay($id)
    $phy_relay_evo1834($id) setInterferenceModel "MEANPOWER"
 
    $phy_relay_evo717($id) setPropagation $propagation
    $phy_relay_evo717($id) setSpectralMask $evo717_data_mask
    $phy_relay_evo717($id) setInterference $interf_data_relay_2($id)
    $phy_relay_evo717($id) setInterferenceModel "MEANPOWER"

    # multidestination module setup
    $ctr_relay($id) setManualLowerlId [$mll_relay($id) Id_]
    $ctr_relay($id) setAutomaticSwitch

}


#################
# Node Creation #
#################

global nodeCTR nodeROV node_relay

# create the CTR (node 0) and set the position
set nodeCTR [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
createCTR $nodeCTR 0

$position(0) setX_ [expr $opt(rn)*$opt(max_distance_relay)]
$position(0) setY_ 0
$position(0) setZ_ -1000
$applicationCTR setPosition $position(0)

# create the ROV (node 1)
set nodeROV [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
createROV $nodeROV 1

$position(1) setX_ -1000
$position(1) setY_ 0
$position(1) setZ_ -1000
$applicationROV setPosition $position(1)

# create the relays
for {set id 0} {$id < $opt(rn)} {incr id} {

    set node_relay($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
    createRelay $id

    $position_relay($id) setX_ [expr $opt(max_distance_relay)*($opt(rn) - 1 - $id)]
    $position_relay($id) setY_ 0
    $position_relay($id) setZ_ -1000

    # add the ROV IP to the relay multidestination module
	$ctr_relay($id) setDefaultLowerId [$mll_relay_2($id) Id_]
	$ctr_relay($id) addLayer [$ipif(1) addr] [$mll_relay($id) Id_]
}


puts "Position CTR, X:[$position(0) getX_], Y:[$position(0) getY_], Z:[$position(0) getZ_]"
puts "Initial position ROV, X:[$position(1) getX_], Y:[$position(1) getY_], Z:[$position(1) getZ_]"
for {set id 0} {$id < $opt(rn)} {incr id} {
    puts "Position relay  R[expr $id+1] , X:[$position_relay($id) getX_], Y:[$position_relay($id) getY_], Z:[$position_relay($id) getZ_]"    
}

##################
# Setup flows    #
##################
$applicationCTR set destAddr_ [$ipif(1) addr]
$applicationCTR set destPort_ $portnum(1)
$applicationROV set destAddr_ [$ipif(0) addr]
$applicationROV set destPort_ $portnum(0)

#################################
# ARP tables                    #
#################################

$mll(0) addentry [$ipif(1) addr] [$mac(1) addr];  # CTR
$mll(0) addentry [$ipif_relay(0) addr] [$mac_relay_2(0) addr];  # CTR

$mll(1) addentry [$ipif(0) addr] [$mac(0) addr];  # ROV
$mll(1) addentry [$ipif_relay(0) addr] [$mac_relay(0) addr];  # ROV

$mll_relay(0) addentry [$ipif(1) addr] [$mac(1) addr];  # Relay -> ROV
$mll_relay_2(0) addentry [$ipif(0) addr] [$mac(0) addr];  # Relay -> CTR

##################
# Routing tables #
##################
global ipCTR ipROV

set ipCTR   [$ipif(0) addr]
set ipROV   [$ipif(1) addr]
for {set id 0} {$id < $opt(rn)} {incr id} {
    set ipRelay($id) [$ipif_relay($id) addr]
}

puts "IP CTR: $ipCTR"
puts "IP ROV: $ipROV"
for {set id 0} {$id < $opt(rn)} {incr id} {
    puts "IP Relay R[expr $id + 1]: $ipRelay($id)"
}

puts "MAC CTR: [$mac(0) addr]"
puts "MAC ROV: [$mac(1) addr]"
for {set id 0} {$id < $opt(rn)} {incr id} {
    puts "MAC1 Relay R[expr $id + 1]: [$mac_relay($id) addr]"
    puts "MAC2 Relay R[expr $id + 1]: [$mac_relay_2($id) addr]"
}

$ipr(0) addRoute $ipROV $ipRelay(0)
$ipr(0) addRoute $ipRelay(0) $ipRelay(0)

$ipr(1) addRoute $ipCTR $ipRelay(0)
$ipr(1) addRoute $ipRelay(0) $ipRelay(0)

$ipr_relay(0) addRoute $ipROV $ipROV
$ipr_relay(0) addRoute $ipCTR $ipCTR


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

set outfile [open "test_uwrov_multi_destination_simple.csv" "w"]
set fp [open $opt(waypoint_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
foreach line $data {
    if {[regexp {^(.*);(.*);(.*);(.*)$} $line -> t x y z]} {
        $ns at $t "update_and_check $t"
        $ns at $t "$applicationCTR sendPosition $x $y $z"
    }
}

$ns at $opt(starttime) "$applicationCTR start"
$ns at $opt(starttime) "$applicationROV start"
$ns at $opt(starttime) "$mac(0) start"
$ns at $opt(starttime) "$mac(1) start"

for {set id 0} {$id < $opt(rn)} {incr id} {
    $ns at $opt(starttime) "$mac_relay($id) start"
    $ns at $opt(starttime) "$mac_relay_2($id) start"
    $ns at $opt(stoptime) "$mac_relay($id) stop"
    $ns at $opt(stoptime) "$mac_relay_2($id) stop"
}

#$ns at $opt(stoptime) "$applicationCTR stop"
$ns at $opt(stoptime) "$mac(0) stop"
$ns at $opt(stoptime) "$mac(1) stop"




########################################
# proc to obtain buffer size every slot#
########################################

# set max [expr $opt(txduration) / $opt(max_slot_duration_relay)]
# set time 100
# for {set i 0} {$i < $max} {incr i} {
#     set time [expr $time + $opt(max_slot_duration_relay)]
#     $ns at $time "prova_buffer $time"
# }


# proc prova_buffer {time} {

#     global position applicationROV opt applicationCTR mac_relay_2 mac

#     puts "$time"
#     puts "buffer CTR: [$mac(0) get_buffer_size]"
#   #  for {set id 0} {$id < $opt(rn)} {incr id} {
#      puts "buffer R[expr 0 + 1]: [$mac_relay_2(0) get_buffer_size]"
#   #  }
#     puts "buffer ROV: [$mac(1) get_buffer_size]"

# }














proc update_and_check {t} {
    global position applicationROV
    $position(1) update
    set outfile [open "test_uwrov_multi_destination_simple.csv" "a"]
    puts $outfile "$t,[$applicationROV getX],[$applicationROV getY],[$applicationROV getZ]" 
    close $outfile
}
###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation

proc finish {} {
    global ns opt outfile
    global mac propagation phy_evo717 phy_evo1834 phy_relay_evo717 phy_relay_evo1834
    global ipr ipif udp position applicationROV applicationCTR application_relay
    
    if ($opt(verbose)) {
        puts "positions CTR: [$applicationCTR getX]"
        update_and_check $opt(stoptime)
        puts "---------------------------------------------------------------------"
        puts "Simulation summary"
        puts "number of nodes     : $opt(nn)"
        puts "ROV packet size     : $opt(ROV_pktsize) byte"
        puts "ROV period          : $opt(ROV_period) s"
        puts "CTR packet size     : $opt(CTR_pktsize) byte"
        puts "CTR period          : $opt(CTR_period) s"
        puts "simulation length   : $opt(txduration) s"
        puts "---------------------------------------------------------------------"
        puts "Evologic 7-17 specs"
        puts "tx power:           : $opt(evo717_txpower) dB"
        puts "tx frequency        : $opt(evo717_freq) Hz"
        puts "tx bandwidth        : $opt(evo717_bw) Hz"
        puts "bitrate             : $opt(evo717_bitrate) bps"
        puts "---------------------------------------------------------------------"
        puts "Evologic 18-34 specs"
        puts "tx power:           : $opt(evo1834_txpower) dB"
        puts "tx frequency        : $opt(evo1834_freq) Hz"
        puts "tx bandwidth        : $opt(evo1834_bw) Hz"
        puts "bitrate             : $opt(evo1834_bitrate) bps"
        puts "---------------------------------------------------------------------"        
        puts "Slot duration ROV   : $opt(max_slot_duration_rov) s"
        puts "Slot duration Relay : $opt(max_slot_duration_relay) s"
        puts "---------------------------------------------------------------------"
    } 

    set ROV_throughput              [$applicationROV getthr]
    set ROV_per                     [$applicationROV getper]
    set ROV_sent_pkts               [$mac(1) get_sent_pkts]
    set ROV_rcv_pkts                [$mac(1) get_recv_pkts]

    set CTR_throughput              [$applicationCTR getthr]
    set CTR_per                     [$applicationCTR getper]
    set CTR_sent_pkts               [$mac(0) get_sent_pkts]
    set CTR_rcv_pkts                [$mac(0) get_recv_pkts]

    set cbr_throughput              [$application_relay(0) getthr]
    set cbr_per                     [$application_relay(0) getper]
    set cbr_sent_pkts               [$application_relay(0) getsentpkts]
    set cbr_rcv_pkts                [$application_relay(0) getrecvpkts]

    set CTR_ftt                     [$applicationROV getftt]
    set CTR_ftt_std                 [$applicationROV getfttstd]
    set CTR_rtt                     [$applicationCTR getrtt]

    set ROV_ftt                     [$applicationCTR getftt]
    set ROV_ftt_std                 [$applicationCTR getfttstd]

    if ($opt(verbose)) {
        puts "applicationROV Throughput     : $ROV_throughput"
        puts "applicationROV PER            : $ROV_per       "
        puts "ROV packet delivery delay     : $ROV_ftt"
        puts "ROV std packet delivery delay : $ROV_ftt_std"
        puts "-------------------------------------------"
        puts "applicationCTR Throughput     : $CTR_throughput"
        puts "applicationCTR PER            : $CTR_per       "
        puts "CTR packet delivery delay     : $CTR_ftt"
        puts "CTR std packet delivery delay : $CTR_ftt_std"
        puts "-------------------------------------------"
    }

        set sum_throughput [expr $ROV_throughput + $CTR_throughput]
        set sum_sent_pkts [expr $ROV_sent_pkts + $CTR_sent_pkts]
        set sum_rcv_pkts  [expr $ROV_rcv_pkts + $CTR_rcv_pkts]
       
        set ipheadersize        [$ipif(1) getipheadersize]
        set udpheadersize       [$udp(1) getudpheadersize]
        set ROVheadersize       [$applicationROV getROVMonheadersize]
        set CTRheadersize       [$applicationCTR getROVctrheadersize]

        set transmitting_node [expr $opt(nn) - $opt(rn)]
    
    if ($opt(verbose)) {
        puts "Mean Throughput               : [expr ($sum_throughput/(($transmitting_node)*($transmitting_node-1)))]"
        puts "Sent Packets      CTR --> ROV : $CTR_sent_pkts"
        puts "Received Packets  CTR --> ROV : $ROV_rcv_pkts"
        puts "Sent Packets      ROV --> CTR : $ROV_sent_pkts"
        puts "Received Packets  ROV --> CTR : $CTR_rcv_pkts"
        puts "Sent Packets CBR              : $cbr_sent_pkts"
        puts "Received Packets  CBR         : $cbr_rcv_pkts"
        puts "---------------------------------------------------------------------"
        puts "Sent Packets                  : $sum_sent_pkts"
        puts "Received                      : $sum_rcv_pkts"
        puts "Packet Delivery Ratio         : [expr 1.0 * $sum_rcv_pkts / $sum_sent_pkts * 100 ]"
        puts "IP Pkt Header Size            : $ipheadersize"
        puts "UDP Header Size               : $udpheadersize"
        puts "ROV Header Size               : $ROVheadersize"
        puts "CTR Header Size               : $CTRheadersize"
        puts "CTR round trip time           : $CTR_rtt"
        puts "---------------------------------------------------------------------"
    }
        set ROV_packet_lost        [$phy_evo1834(1) getTotPktsLost]
        set CTR_packet_lost        [$phy_evo717(0) getTotPktsLost]
        set Relay_packet_lost      [expr [$phy_relay_evo1834(0) getTotPktsLost] + [$phy_relay_evo717(0) getTotPktsLost]]
        set packet_lost            [expr $CTR_packet_lost + $ROV_packet_lost + $Relay_packet_lost]
    
    if ($opt(verbose)) {   
        puts "- PHY layer statistics for the ROV -"
        puts "Consumed energy Tx          : [$phy_evo1834(1) getConsumedEnergyTx]"
        puts "Consumed energy Rx          : [$phy_evo1834(1) getConsumedEnergyRx]"
        puts "Tot. pkts lost              : $ROV_packet_lost"
        puts "Tot. collision CTRL         : [$phy_evo1834(1) getCollisionsCTRL]"
        puts "Tot. collision DATA         : [$phy_evo1834(1) getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL : [$phy_evo1834(1) getCollisionsDATAvsCTRL]"
        puts "Tot. CTRL pkts lost         : [$phy_evo1834(1) getTotCtrlPktsLost]"
        puts "Tot. CTRL pkts lost due to Interference   : [$phy_evo1834(1) getErrorCtrlPktsInterf]"
        puts "---------------------------------------------------------------------"
        puts "- PHY layer statistics for the CTR -"
        puts "Consumed energy Tx          : [$phy_evo717(0) getConsumedEnergyTx]"
        puts "Consumed energy Rx          : [$phy_evo717(0) getConsumedEnergyRx]"
        puts "Tot. pkts lost              : $CTR_packet_lost"
        puts "Tot. collision CTRL         : [$phy_evo717(0) getCollisionsCTRL]"
        puts "Tot. collision DATA         : [$phy_evo717(0) getCollisionsDATA]"
        puts "Tot. collision DATA vs CTRL : [$phy_evo717(0) getCollisionsDATAvsCTRL]"
        puts "Tot. ROV pkts lost          : [$phy_evo717(0) getTotCtrlPktsLost]"
        puts "Tot. CTRL pkts lost due to Interference   : [$phy_evo717(0) getErrorCtrlPktsInterf]"
        puts "---------------------------------------------------------------------"
        puts "- PHY layer statistics for the Relay -"
        puts "Tot. pkts lost              : $Relay_packet_lost"
        puts "Tot. collision CTRL         : [expr [$phy_relay_evo717(0) getCollisionsCTRL] + [$phy_relay_evo1834(0) getCollisionsCTRL]]"
        puts "Tot. collision DATA         : [expr [$phy_relay_evo717(0) getCollisionsDATA] + [$phy_relay_evo1834(0) getCollisionsDATA]]"
        puts "Tot. collision DATA vs CTRL : [expr [$phy_relay_evo717(0) getCollisionsDATAvsCTRL] + [$phy_relay_evo1834(0) getCollisionsDATAvsCTRL]]"
        puts "Tot. ROV pkts lost          : [expr [$phy_relay_evo717(0) getTotCtrlPktsLost] + [$phy_relay_evo1834(0) getTotCtrlPktsLost]]"
        puts "Tot. CTRL pkts lost due to Interference   : [expr [$phy_relay_evo717(0) getErrorCtrlPktsInterf] + [$phy_relay_evo1834(0) getErrorCtrlPktsInterf]]"
        puts "---------------------------------------------------------------------"
        puts "- Global situation -"
        puts "Tot. pkts lost              : $packet_lost"
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
