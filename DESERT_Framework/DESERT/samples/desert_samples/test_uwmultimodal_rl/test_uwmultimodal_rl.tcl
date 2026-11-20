#
# Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
# Authors: Filippo Donegà and Filippo Bragato
# Version: 1.0.0
# NOTE: Tcl sample tested on Ubuntu 24.04 LTS, 64 bits OS
#
# -----------------------------------------------------------------------------------------------
# This script tests the uwmultimodal_rl module in a multihop scenario involving 2 AUVs (Nodes 
# 0 and 2) and a static node (Node 1), all at the depth of 100 m, arranged as follows:
#
#                                              Node 0 (Mobile) <-------------------------------->
#                                              Node 1 (Static)
# <------------------------------------------> Node 2 (Mobile)
# |------------------------------------------|                 |--------------------------------|
#                  ~ 830 m                                                  ~ 660 m 
#                   
# Each link is bidirectional and multimodal (acoustic and optical). The communication articulates
# in 3 different application traffics, each one paired with a dedicated agent having a specific
# optimization goal:
# - Traffic 1 (idea: measurement data, variable bitrate) - Agent minimizes delay (max throughput)
# - Traffic 2 (idea: video streaming, variable bitrate) - Agent minimizes jitter
# - Traffic 3 (idea: control data, constant bitrate) - Agent maximizes PDR (max reliability)
#
# This totals to 6 RL agents for each node, one for each combination of traffic type and link.
# Each agent decides the medium to be used for a transmission of the corresponding traffic type 
# over the respective link.
#
# The AUV movement paths can be generated using the waypoints_generator.ipynb file.
# -----------------------------------------------------------------------------------------------
#
#                                    Network stack of each node
#
#   +----------------------+----------------------+----------------------+----------------------+
#   | 8.UW/CBR (Traffic 1) | 8.UW/CBR (Traffic 2) | 8.UW/CBR (Traffic 3) | 8. UW/CBR (Traffic 4 |
#   |                      |                      |                      | - Optical Probing)   |
#   +----------------------+----------------------+----------------------+----------------------+
#   | 7.UW/UDP                                                                                  |
#   +-------------------------------------------------------------------------------------------+
#   | 6.UW/STATICROUTING                                                                        |
#   +-------------------------------------------------------------------------------------------+
#   | 5.UW/IP                                                                                   |
#   +------------------------------+----------------------------+-------------------------------+
#   | 4.UW/MULTIMODAL_RL           | 4.UW/MULTIMODAL_RL         | 4.UW/MULTIMODAL_RL            |  
#   | (Agent for traffic 1)        | (Agent for traffic 2)      | (Agent for traffic 3)         |
#   +------------------------------+--------------+-------------+-------------------------------+
#   | 3.UW/MLL                                    | 3.UW/MLL                                    |
#   | (Acoustic)                                  | (Optical)                                   |
#   +---------------------------------------------+---------------------------------------------+
#   | 2.UW/CSMA_ALOHA_RL                          | 2.UW/CSMA_ALOHA_RL                          |
#   | (Acoustic)                                  | (Optical)                                   |
#   +---------------------------------------------+---------------------------------------------+
#   | 1.UW/PHYSICAL                               | 1.UW/OPTICALSTATSPHY                        |
#   | (Acoustic)                                  | (Optical)                                   |
#   +---------------------------------------------+---------------------------------------------+
#                                            |         |                       
#   +-------------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                     |
#   +-------------------------------------------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################

set opt(trace_files)        0
set opt(bash_parameters)    1
set opt(debug)              0

###################
# Library loading #
###################

load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
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
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwposbasedrt.so
load libuwoptical_propagation.so
load libuwem_channel.so
load libuwoptical_channel.so
load libuwoptical_phy.so
load libuwmultimodal_rl.so
load libuwopticalstatsphy.so
load libuwcsmaaloha.so
load libuwcsmaaloharl.so

#############################
# NS-Miracle initialization #
#############################

set ns [new Simulator]
$ns use-Miracle

########################
# Command line options #
########################

if {$opt(bash_parameters)} {

    if {$argc > 23} {

        set opt(rng_stream)             [lindex $argv 0]
        set opt(initial_windspeed)      [lindex $argv 1]
        set opt(windspeed_variance)     [lindex $argv 2]
        set opt(initial_c)              [lindex $argv 3]
        set opt(c_variance)             [lindex $argv 4]
        set opt(epsilon)                [lindex $argv 5]
        set opt(alpha)                  [lindex $argv 6]
        set opt(first_run)              [lindex $argv 7]
        set opt(mode)                   [lindex $argv 8]
        set opt(cbr_period1)            [lindex $argv 9]
        set opt(pktsize1)               [lindex $argv 10]
        set opt(cbr_period2)            [lindex $argv 11]
        set opt(pktsize2)               [lindex $argv 12]
        set opt(cbr_period3)            [lindex $argv 13]
        set opt(pktsize3)               [lindex $argv 14]
        set opt(expire_time)            [lindex $argv 15]
        set opt(queues_max_size)        [lindex $argv 16]
        set opt(probing_period)         [lindex $argv 17]
        set opt(mask_type)              [lindex $argv 18]
        set opt(range_based_distance)   [lindex $argv 19]
        set opt(campaign_path)          [lindex $argv 20]
        set opt(epoch)                  [lindex $argv 21]
        set opt(agent_type)             [lindex $argv 22]
        set opt(eval_flag)              [lindex $argv 23]

    } else {

        puts "Error: the script expects 24 arguments:"
        puts "1) RNG stream (seed)"
        puts "2) Initial windspeed"
        puts "3) Windspeed variance"
        puts "4) Initial attenuation coefficient"
        puts "5) Attenuation coefficient variance"
        puts "6) Epsilon (exploration probability)"
        puts "7) Alpha (learning rate)"
        puts "8) First run flag [0 : NO | 1 : YES]"
        puts "9) Mode [0 : Acoustic only | 1 : Optical only | 2 : Multimodal | 3 : Range-based | 4 : Probing]"
        puts "10) CBR period for traffic 1"
        puts "11) Packet size for traffic 1"
        puts "12) CBR period for traffic 2"
        puts "13) Packet size for traffic 2"
        puts "14) CBR period for traffic 3"
        puts "15) Packet size for traffic 3"
        puts "16) Expire time"
        puts "17) Maximum acoustic and optical queues size"
        puts "18) Probing period"
        puts "19) Mask type [0-8]"
        puts "20) Range-based distance threshold"
        puts "21) Campaign path"
        puts "22) Epoch number"
        puts "23) Agent type"
        puts "24) Evaluation flag (0: training epoch | 1: evaluation epoch)"
    }
}

#################
# Tcl variables #
#################

set opt(nn)                             3   ;# Number of nodes

set opt(starttime)                      1
set opt(stoptime)                       30000.0
set opt(txduration)                     [expr $opt(stoptime) - $opt(starttime)]

set opt(waypoint_file_0)                "../node_0_path.csv"
set opt(waypoint_file_2)                "../node_2_path.csv"

# UW/INTERFERENCE
set opt(maxinterval_)                   20.0

# ACOUSTIC PHY
set opt(bitrate)                        2500.0
set opt(txpower)                        175.0
set opt(rx_snr_penalty_db)              0.0
set opt(tx_margin_db)                   10.0
set opt(per_tgt)                        0.1
set opt(freq)                           26000.0
set opt(bw)                             16000.0

# UW/CSMA_ALOHA_RL
set opt(ack_mode)                       "setNoAckMode"

# UW/CBR
set opt(seedcbr)	                    1

# PROPAGATION
set opt(shipping)                       1

# RNG configuration
global defaultRNG
$defaultRNG seed $opt(seedcbr)
for {set k 0} {$k < $opt(rng_stream)} {incr k} {

	$defaultRNG next-substream
}

if {$opt(trace_files)} {

    set opt(tracefilename) "./test_uwmultimodal_rl.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./test_uwmultimodal_rl.cltr"
    set opt(cltracefile) [open $opt(tracefilename) w]

} else {

    set opt(tracefilename) "/dev/null"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "/dev/null"
    set opt(cltracefile) [open $opt(cltracefilename) w]

}

# OPTICAL PHY
set opt(optical_freq)              10000000     ; # 10 MHz
set opt(optical_bw)                100000       ; # 100 kHz
set opt(optical_bitrate)           4000000      ; # 4 MHz
set opt(optical_txpower)           100          
set opt(opt_acq_db)                10
set opt(temperature)               293.15       ; # In Kelvin
set opt(txArea)                    0.000010
set opt(rxArea)                    0.0000011    ; # Receiving area, it has to be the same for optical physical and propagation
set opt(theta)                     1
set opt(id)                        [expr 1.0e-9]
set opt(il)                        [expr 1.0e-6]
set opt(shuntRes)                  [expr 1.49e9]
set opt(sensitivity)               0.26
set opt(LUTpath)                   "../../dbs/optical_noise/LUT.txt"
set opt(atten_LUT)                 "../../dbs/optical_attenuation/lut_532nm/lut_532nm_CTD001.csv"
set opt(threshold)                 10

########################
# Module Configuration #
########################

# CBR
Module/UW/CBR set debug_                                 $opt(debug)

# CSMA ALOHA
Module/UW/CSMA_ALOHA_RL set buffer_pkts_  $opt(queues_max_size)      ;# Set this to cap the CSMA buffers
Module/UW/CSMA_ALOHA_RL set debug_        $opt(debug)

# ACOUSTIC PHY
Module/UW/PHYSICAL  set BitRate_                         $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_         2.0
Module/UW/PHYSICAL  set RxSnrPenalty_dB_                 $opt(rx_snr_penalty_db)
Module/UW/PHYSICAL  set TxSPLMargin_dB_                  $opt(tx_margin_db)
Module/UW/PHYSICAL  set MaxTxSPL_dB_                     $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                     10
Module/UW/PHYSICAL  set MaxTxRange_                      50000
Module/UW/PHYSICAL  set PER_target_                      $opt(per_tgt)  
Module/UW/PHYSICAL  set CentralFreqOptimization_         0
Module/UW/PHYSICAL  set BandwidthOptimization_           0
Module/UW/PHYSICAL  set SPLOptimization_                 1
Module/UW/PHYSICAL  set debug_                           $opt(debug)

MPropagation/Underwater set practicalSpreading_          1.5
MPropagation/Underwater set debug_                       $opt(debug)
MPropagation/Underwater set windspeed_                   $opt(initial_windspeed)
MPropagation/Underwater set shipping_                    $opt(shipping)

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

set acoustic_data_mask [new MSpectralMask/Rect]
$acoustic_data_mask setFreq       $opt(freq)
$acoustic_data_mask setBandwidth  $opt(bw)

# OPTICAL PHY
Module/UW/OPTICAL/STATSPHY   set TxPower_                    $opt(optical_txpower)
Module/UW/OPTICAL/STATSPHY   set BitRate_                    $opt(optical_bitrate)
Module/UW/OPTICAL/STATSPHY   set AcquisitionThreshold_dB_    $opt(opt_acq_db)
Module/UW/OPTICAL/STATSPHY   set Id_                         $opt(id)
Module/UW/OPTICAL/STATSPHY   set Il_                         $opt(il)
Module/UW/OPTICAL/STATSPHY   set R_                          $opt(shuntRes)
Module/UW/OPTICAL/STATSPHY   set S_                          $opt(sensitivity)
Module/UW/OPTICAL/STATSPHY   set T_                          $opt(temperature)
Module/UW/OPTICAL/STATSPHY   set Ar_                         $opt(rxArea)
Module/UW/OPTICAL/STATSPHY   set Threshold                   $opt(threshold)
Module/UW/OPTICAL/STATSPHY   set debug_                      $opt(debug)

Module/UW/OPTICAL/Propagation set Ar_                        $opt(rxArea)
Module/UW/OPTICAL/Propagation set At_                        $opt(txArea)
Module/UW/OPTICAL/Propagation set c_                         $opt(initial_c)
Module/UW/OPTICAL/Propagation set theta_                     $opt(theta)
Module/UW/OPTICAL/Propagation set debug_                     $opt(debug)

set optical_propagation [new Module/UW/OPTICAL/Propagation]
$optical_propagation setOmnidirectional

set optical_channel [new Module/UW/Optical/Channel]

set optical_data_mask [new MSpectralMask/Rect]
$optical_data_mask setFreq       $opt(optical_freq)
$optical_data_mask setBandwidth  $opt(optical_bw)

# MULTIMODAL_RL
Module/UW/MULTIMODAL_RL set debug_                        $opt(debug)
Module/UW/MULTIMODAL_RL set alpha_                        $opt(alpha)
Module/UW/MULTIMODAL_RL set beta_                         0.1
Module/UW/MULTIMODAL_RL set epsilon_                      $opt(epsilon)
Module/UW/MULTIMODAL_RL set expire_time_                  $opt(expire_time)
Module/UW/MULTIMODAL_RL set mask_type_                    $opt(mask_type)
Module/UW/MULTIMODAL_RL set range_based_distance_         $opt(range_based_distance)

# UW/MLL
Module/UW/MLL set debug_                                    $opt(debug)

################################
# Procedure(s) to create nodes #
################################

proc createNode { id } {

    global channel propagation data_mask ns cbr position node udp
    global portnum1 portnum2 portnum3 portnum4 ipr ipifphy opt acoustic_mll optical_mll 
    global acoustic_mac optical_mac optical_channel 
    global optical_propagation acoustic_data_mask
    global optical_data_mask ipif rl1 rl2 rl3 opt cbr1 cbr2 cbr3 cbr4

    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    Module/UW/CBR set packetSize_       $opt(pktsize1)
    Module/UW/CBR set period_           $opt(cbr_period1)
    Module/UW/CBR set PoissonTraffic_   1
    Module/UW/CBR set traffic_type_     1
    Module/UW/MULTIMODAL_RL set handled_traffic_type_         1
    Module/UW/MULTIMODAL_RL set rng_seed_                     $opt(rng_stream)
    Module/UW/MULTIMODAL_RL set range_based_distance_         $opt(range_based_distance)
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
			set cbr1($id,$cnt)              [new Module/UW/CBR]
            set rl1($id,$cnt)               [new Module/UW/MULTIMODAL_RL]
		}
	}

    Module/UW/CBR set packetSize_       $opt(pktsize2)
    Module/UW/CBR set period_           $opt(cbr_period2)
    Module/UW/CBR set PoissonTraffic_   1
    Module/UW/CBR set traffic_type_     2
    Module/UW/MULTIMODAL_RL set handled_traffic_type_         2
    Module/UW/MULTIMODAL_RL set rng_seed_                     $opt(rng_stream)
    Module/UW/MULTIMODAL_RL set range_based_distance_         $opt(range_based_distance)

	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {      
		    set cbr2($id,$cnt)              [new Module/UW/CBR] 
            set rl2($id,$cnt)               [new Module/UW/MULTIMODAL_RL]
        }
	}

    Module/UW/CBR set packetSize_       $opt(pktsize3)
    Module/UW/CBR set period_           $opt(cbr_period3)
    Module/UW/CBR set PoissonTraffic_   1
    Module/UW/CBR set traffic_type_     3
    Module/UW/MULTIMODAL_RL set handled_traffic_type_         3
    Module/UW/MULTIMODAL_RL set rng_seed_                     $opt(rng_stream)
    Module/UW/MULTIMODAL_RL set range_based_distance_         $opt(range_based_distance)

	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
			set cbr3($id,$cnt)              [new Module/UW/CBR]
            set rl3($id,$cnt)               [new Module/UW/MULTIMODAL_RL]
		}
	}

    Module/UW/CBR set packetSize_       8
    Module/UW/CBR set period_           $opt(probing_period)
    Module/UW/CBR set PoissonTraffic_   1
    Module/UW/CBR set traffic_type_     4
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
			set cbr4($id,$cnt)  [new Module/UW/CBR]
		}
	}

    set udp($id)                        [new Module/UW/UDP]
    set ipr($id)                        [new Module/UW/StaticRouting]
    set ipif($id)                       [new Module/UW/IP]
    set acoustic_mll($id)               [new Module/UW/MLL]
    set optical_mll($id)                [new Module/UW/MLL]
    set acoustic_mac($id)               [new Module/UW/CSMA_ALOHA_RL]
    set optical_mac($id)                [new Module/UW/CSMA_ALOHA_RL]
    set acoustic_phy($id)               [new Module/UW/PHYSICAL]
    set optical_phy($id)                [new Module/UW/OPTICAL/STATSPHY]

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
            $node($id) addModule 8 $cbr1($id,$cnt)   1  "CBR1"
            $node($id) addModule 8 $cbr2($id,$cnt)   1  "CBR2"
            $node($id) addModule 8 $cbr3($id,$cnt)   1  "CBR3"
            $node($id) addModule 8 $cbr4($id,$cnt)   1  "CBR4"
        }
    }
    $node($id) addModule 7  $udp($id)             1  "UDP"
    $node($id) addModule 6  $ipr($id)             1  "IPR"
    $node($id) addModule 5  $ipif($id)            1  "IPF"
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
            $node($id) addModule 4 $rl1($id,$cnt)     1  "RL1"
            $node($id) addModule 4 $rl2($id,$cnt)     1  "RL2"
            $node($id) addModule 4 $rl3($id,$cnt)     1  "RL3"
        }
    }
    $node($id) addModule 3  $acoustic_mll($id)    1  "MLL1"
    $node($id) addModule 3  $optical_mll($id)     1  "MLL2"
    $node($id) addModule 2  $acoustic_mac($id)    1  "MAC1"
    $node($id) addModule 2  $optical_mac($id)     1  "MAC2"
    $node($id) addModule 1  $acoustic_phy($id)    1  "PHY1"
    $node($id) addModule 1  $optical_phy($id)     1  "PHY2"

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
            $node($id) setConnection $cbr1($id,$cnt)   $udp($id)             1
            set portnum1($id,$cnt) [$udp($id) assignPort $cbr1($id,$cnt)]
            $node($id) setConnection $cbr2($id,$cnt)   $udp($id)             1
            set portnum2($id,$cnt) [$udp($id) assignPort $cbr2($id,$cnt)]
            $node($id) setConnection $cbr3($id,$cnt)   $udp($id)             1
            set portnum3($id,$cnt) [$udp($id) assignPort $cbr3($id,$cnt)]
            $node($id) setConnection $cbr4($id,$cnt)   $udp($id)             1
            set portnum4($id,$cnt) [$udp($id) assignPort $cbr4($id,$cnt)]
        }
    }
    $node($id) setConnection $udp($id)           $ipr($id)                   1
    $node($id) setConnection $ipr($id)           $ipif($id)                  1
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {  
            $node($id) setConnection $ipif($id)          $rl1($id,$cnt)      1
            $node($id) setConnection $ipif($id)          $rl2($id,$cnt)      1
            $node($id) setConnection $ipif($id)          $rl3($id,$cnt)      1
            $node($id) setConnection $rl1($id,$cnt)      $acoustic_mll($id)  1
            $node($id) setConnection $rl1($id,$cnt)      $optical_mll($id)   1
            $node($id) setConnection $rl2($id,$cnt)      $acoustic_mll($id)  1
            $node($id) setConnection $rl2($id,$cnt)      $optical_mll($id)   1
            $node($id) setConnection $rl3($id,$cnt)      $acoustic_mll($id)  1
            $node($id) setConnection $rl3($id,$cnt)      $optical_mll($id)   1
        }
    }
    $node($id) setConnection $acoustic_mll($id)  $acoustic_mac($id)      1
    $node($id) setConnection $optical_mll($id)   $optical_mac($id)       1
    $node($id) setConnection $acoustic_mac($id)  $acoustic_phy($id)      1
    $node($id) setConnection $optical_mac($id)   $optical_phy($id)       1
    $node($id) addToChannel  $channel            $acoustic_phy($id)      1
    $node($id) addToChannel  $optical_channel    $optical_phy($id)       1

    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }    

    $ipif($id) addr [expr $id + 1]
    $acoustic_mac($id) setMacAddr [expr $id + 1]
    $optical_mac($id) setMacAddr [expr $id + 1]

    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)

    if {$id == 0} {
        $position($id) setX_ -1
        $position($id) setY_ 0
        $position($id) setZ_ -100
    }
    if {$id == 1} {
        $position($id) setX_ 0
        $position($id) setY_ 0
        $position($id) setZ_ -100
    }
    if {$id == 2} {
        $position($id) setX_ 1
        $position($id) setY_ 0
        $position($id) setZ_ -100
    }

    set interf_data2($id) [new "Module/UW/INTERFERENCE"]
    $interf_data2($id) set maxinterval_ $opt(maxinterval_)
    $interf_data2($id) set debug_       0

    set optical_interf_data($id) [new "MInterference/MIV"]
    $optical_interf_data($id) set maxinterval_ $opt(maxinterval_)
    $optical_interf_data($id) set debug_       0

    $acoustic_phy($id) setPropagation $propagation
    $optical_phy($id)  setPropagation $optical_propagation

    $acoustic_phy($id) setSpectralMask $acoustic_data_mask
    $acoustic_phy($id) setInterference $interf_data2($id)
    $acoustic_phy($id) setInterferenceModel "MEANPOWER"

    $optical_phy($id) setSpectralMask $optical_data_mask
    $optical_phy($id) setInterference $optical_interf_data($id)
    $optical_phy($id) setLUTFileName  "$opt(LUTpath)"
    $optical_phy($id) setLUTSeparator " "
    $optical_phy($id) useLUT

    $acoustic_mac($id) $opt(ack_mode)
    $acoustic_mac($id) set wait_costant_    [expr 1.0e-4]
    $acoustic_mac($id) set listen_time_     [expr 1.0e-4]
    $optical_mac($id) $opt(ack_mode)
    $optical_mac($id) set wait_costant_     [expr 1.0e-12]
    $optical_mac($id) set listen_time_      [expr 1.0e-12]
    $acoustic_mac($id) initialize
    $acoustic_mac($id) sendQueuePointerClmsg 0
    $optical_mac($id) initialize
    $optical_mac($id) sendQueuePointerClmsg 1

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
            $rl1($id,$cnt) setAgentType $opt(agent_type)
            $rl1($id,$cnt) initialize "$opt(campaign_path)/logs/log_${opt(epoch)}_${id}_${cnt}_1.csv" $opt(probing_period) $opt(mask_type) $opt(eval_flag)
            $rl1($id,$cnt) setMode $opt(mode)

            $rl2($id,$cnt) setAgentType $opt(agent_type)
            $rl2($id,$cnt) initialize "$opt(campaign_path)/logs/log_${opt(epoch)}_${id}_${cnt}_2.csv" $opt(probing_period) $opt(mask_type) $opt(eval_flag)
            $rl2($id,$cnt) setMode $opt(mode)
            
            $rl3($id,$cnt) setAgentType $opt(agent_type)
            $rl3($id,$cnt) initialize "$opt(campaign_path)/logs/log_${opt(epoch)}_${id}_${cnt}_3.csv" $opt(probing_period) $opt(mask_type) $opt(eval_flag)
            $rl3($id,$cnt) setMode $opt(mode)
        }
    }
}

#################
# Node creation #
#################

for {set id 0} {$id < $opt(nn)} {incr id}  {

    createNode $id
}

################################
# Inter-node module connection #
################################

proc connectNodes { id des } {

    global ipif portnum1 portnum2 portnum3 portnum4
    global cbr1 cbr2 cbr3 cbr4

    $cbr1($id,$des) set destAddr_ [$ipif($des) addr]
    $cbr1($id,$des) set destPort_ $portnum1($des,$id)
    $cbr2($id,$des) set destAddr_ [$ipif($des) addr]
    $cbr2($id,$des) set destPort_ $portnum2($des,$id)
    $cbr3($id,$des) set destAddr_ [$ipif($des) addr]
    $cbr3($id,$des) set destPort_ $portnum3($des,$id)
    $cbr4($id,$des) set destAddr_ [$ipif($des) addr]
    $cbr4($id,$des) set destPort_ $portnum4($des,$id)
}

# Setup flows
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
            connectNodes $id1 $id2
        }
    }
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        $acoustic_mll($id1) addentry [$ipif($id2) addr] [$acoustic_mac($id2) addr]
        $optical_mll($id1) addentry [$ipif($id2) addr] [$optical_mac($id2) addr]
    }   
}

# Setup routing tables
$ipr(0) addRoute [$ipif(2) addr] [$ipif(1) addr]
$ipr(1) addRoute [$ipif(2) addr] [$ipif(2) addr]
$ipr(2) addRoute [$ipif(0) addr] [$ipif(1) addr]
$ipr(1) addRoute [$ipif(0) addr] [$ipif(0) addr]

# Set node tracking and pass the pointer to info_vec
for {set id 0} {$id < $opt(nn)} {incr id} {

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if {$id != $cnt} {
            $rl1($id,$cnt) addInfoVecPtr [$rl1($cnt,$id) getInfoVecPtr]
            $rl2($id,$cnt) addInfoVecPtr [$rl2($cnt,$id) getInfoVecPtr]
            $rl3($id,$cnt) addInfoVecPtr [$rl3($cnt,$id) getInfoVecPtr]
            $rl1($id,$cnt) addStatePtr [$rl1($cnt,$id) getStatePtr]
            $rl2($id,$cnt) addStatePtr [$rl2($cnt,$id) getStatePtr]
            $rl3($id,$cnt) addStatePtr [$rl3($cnt,$id) getStatePtr]
            $rl1($id,$cnt) setOtherAddr [$ipif($cnt) addr]
            $rl2($id,$cnt) setOtherAddr [$ipif($cnt) addr]
            $rl3($id,$cnt) setOtherAddr [$ipif($cnt) addr]
        }
    }
}

######################
# Setup ROV movement #
######################

proc computeDistance {x1 y1 z1 x2 y2 z2} {

    set temp [expr pow(($x2 - $x1), 2) + pow(($y2 - $y1), 2) + pow(($z2 - $z1), 2)] 
    return [expr {sqrt($temp)}]
}

set fp_0 [open $opt(waypoint_file_0)]
set file_data_0 [read $fp_0]
close $fp_0
set data_0 [split $file_data_0 "\n"]

set fp_2 [open $opt(waypoint_file_2)]
set file_data_2 [read $fp_2]
close $fp_2
set data_2 [split $file_data_2 "\n"]

array set coords_node_0 {}
array set coords_node_2 {}

foreach line $data_0 {
    if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {
        set coords_node_0($t) [list $x $y $z]
    }
}

foreach line $data_2 {
    if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {
        set coords_node_2($t) [list $x $y $z]
    }
}

set node_1_x 0
set node_1_y 0
set node_1_z -100

foreach line $data_0 {

	if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {

        $ns at $t "$position(0) setX_ $x"
        $ns at $t "$position(0) setY_ $y"
        $ns at $t "$position(0) setZ_ $z"

        set distance_to_1 [computeDistance $node_1_x $node_1_y $node_1_z $x $y $z]
        
        $ns at $t "$rl1(0,1) updateDistance $distance_to_1"
        $ns at $t "$rl1(0,1) updateDepth $z"
        $ns at $t "$rl2(0,1) updateDistance $distance_to_1"
        $ns at $t "$rl2(0,1) updateDepth $z"
        $ns at $t "$rl3(0,1) updateDistance $distance_to_1"
        $ns at $t "$rl3(0,1) updateDepth $z"
        $ns at $t "$rl1(1,0) updateDistance $distance_to_1"
        $ns at $t "$rl1(1,0) updateDepth $z"
        $ns at $t "$rl2(1,0) updateDistance $distance_to_1"
        $ns at $t "$rl2(1,0) updateDepth $z"
        $ns at $t "$rl3(1,0) updateDistance $distance_to_1"
        $ns at $t "$rl3(1,0) updateDepth $z"

        if {[info exists coords_node_2($t)]} {
            # Extract the stored coordinates for node 2
            set node_2_x [lindex $coords_node_2($t) 0]
            set node_2_y [lindex $coords_node_2($t) 1]
            set node_2_z [lindex $coords_node_2($t) 2]

            set distance_to_2 [computeDistance $node_2_x $node_2_y $node_2_z $x $y $z]
            
            $ns at $t "$rl1(0,2) updateDistance $distance_to_2"
            $ns at $t "$rl1(0,2) updateDepth $z"
            $ns at $t "$rl2(0,2) updateDistance $distance_to_2"
            $ns at $t "$rl2(0,2) updateDepth $z"
            $ns at $t "$rl3(0,2) updateDistance $distance_to_2"
            $ns at $t "$rl3(0,2) updateDepth $z"
            $ns at $t "$rl1(2,0) updateDistance $distance_to_2"
            $ns at $t "$rl1(2,0) updateDepth $z"
            $ns at $t "$rl2(2,0) updateDistance $distance_to_2"
            $ns at $t "$rl2(2,0) updateDepth $z"
            $ns at $t "$rl3(2,0) updateDistance $distance_to_2"
            $ns at $t "$rl3(2,0) updateDepth $z"
        }
    }
}

foreach line $data_2 {

	if {[regexp {^(.*),(.*),(.*),(.*)$} $line -> t x y z]} {

        $ns at $t "$position(2) setX_ $x"
        $ns at $t "$position(2) setY_ $y"
        $ns at $t "$position(2) setZ_ $z"

        set distance_to_1 [computeDistance $node_1_x $node_1_y $node_1_z $x $y $z]
        
        $ns at $t "$rl1(2,1) updateDistance $distance_to_1"
        $ns at $t "$rl1(2,1) updateDepth $z"
        $ns at $t "$rl2(2,1) updateDistance $distance_to_1"
        $ns at $t "$rl2(2,1) updateDepth $z"
        $ns at $t "$rl3(2,1) updateDistance $distance_to_1"
        $ns at $t "$rl3(2,1) updateDepth $z"
        $ns at $t "$rl1(1,2) updateDistance $distance_to_1"
        $ns at $t "$rl1(1,2) updateDepth $z"
        $ns at $t "$rl2(1,2) updateDistance $distance_to_1"
        $ns at $t "$rl2(1,2) updateDepth $z"
        $ns at $t "$rl3(1,2) updateDistance $distance_to_1"
        $ns at $t "$rl3(1,2) updateDepth $z"

        if {[info exists coords_node_0($t)]} {
            # Extract the stored coordinates for node 0
            set node_0_x [lindex $coords_node_0($t) 0]
            set node_0_y [lindex $coords_node_0($t) 1]
            set node_0_z [lindex $coords_node_0($t) 2]

            set distance_to_0 [computeDistance $node_0_x $node_0_y $node_0_z $x $y $z]
            
            $ns at $t "$rl1(2,0) updateDistance $distance_to_0"
            $ns at $t "$rl1(2,0) updateDepth $z"
            $ns at $t "$rl2(2,0) updateDistance $distance_to_0"
            $ns at $t "$rl2(2,0) updateDepth $z"
            $ns at $t "$rl3(2,0) updateDistance $distance_to_0"
            $ns at $t "$rl3(2,0) updateDepth $z"
            $ns at $t "$rl1(0,2) updateDistance $distance_to_0"
            $ns at $t "$rl1(0,2) updateDepth $z"
            $ns at $t "$rl2(0,2) updateDistance $distance_to_0"
            $ns at $t "$rl2(0,2) updateDepth $z"
            $ns at $t "$rl3(0,2) updateDistance $distance_to_0"
            $ns at $t "$rl3(0,2) updateDepth $z"
        }
    }
}

####################
# Finish procedure #
####################

proc finish { } {

    global ns opt

    $ns flush-trace
    close $opt(tracefile)
}

####################
# Start simulation #
####################

# Set variable windspeed and attenuation coefficient
set w_s $opt(initial_windspeed)
set delta_w_s [$defaultRNG normal 0 1]
set alpha_w_s 0.001

set a_c $opt(initial_c)
set delta_a_c [$defaultRNG normal 0 0.03]
set alpha_a_c 0.0003

for {set t 0} {$t < $opt(txduration)} {incr t} {

    if {[expr $t % 100] == 0} {

        if {$opt(windspeed_variance) != 0} {
            set w_s [expr $w_s + $alpha_w_s * $delta_w_s]
            set delta_w_s [expr $delta_w_s + [$defaultRNG normal 0 $opt(windspeed_variance)]]
            if {$delta_w_s > $opt(windspeed_variance)} {
                set $delta_w_s $opt(windspeed_variance)
            }
            if {$delta_w_s < [expr -$opt(windspeed_variance)]} {
                set $delta_w_s [expr -$opt(windspeed_variance)]
            }
            if {$w_s < 0} {

                set w_s 0
            }
            if {$w_s > 20} {

                set w_s 20
            }
            $ns at $t "$propagation set windspeed_ $w_s"
        }

        if {$opt(c_variance) != 0} {
            set a_c [expr $a_c + $alpha_a_c * $delta_a_c]
            set delta_a_c [expr $delta_a_c + [$defaultRNG normal 0 $opt(c_variance)]]
            if {$delta_a_c > $opt(c_variance)} {
                set $delta_a_c $opt(c_variance)
            }
            if {$delta_a_c < [expr -$opt(c_variance)]} {
                set $delta_a_c [expr -$opt(c_variance)]
            }
            if {$a_c < 0.15} {
            
                set a_c 0.15
            }
            if {$a_c > 0.40} {
            
                set a_c 0.40
            }
            $ns at $t "$optical_propagation set c_ $a_c"
        }
        
        for {set id 0} {$id < $opt(nn)} {incr id} {
            for {set cnt 0} {$cnt < $opt(nn)} {incr cnt}  {
                if {$id != $cnt} {
                    $ns at $t "$rl1($id,$cnt) updateWindspeed $w_s"
                    $ns at $t "$rl1($id,$cnt) updateC $a_c"
                    $ns at $t "$rl2($id,$cnt) updateWindspeed $w_s"
                    $ns at $t "$rl2($id,$cnt) updateC $a_c"
                    $ns at $t "$rl3($id,$cnt) updateWindspeed $w_s"
                    $ns at $t "$rl3($id,$cnt) updateC $a_c"
                }
            }
        }
    }
}

# Load weights if it's not the first run
if {$opt(first_run) != 1} {
    for {set id 0} {$id < $opt(nn)} {incr id} {
        for {set cnt 0} {$cnt < $opt(nn)} {incr cnt}  {
            if {$id != $cnt} {
                if {$opt(epoch) != 0} {
                    set epoch_prior [expr $opt(epoch) - 1]
                    $ns at [expr $opt(starttime) - 0.5] "$rl1($id,$cnt) loadModel $opt(campaign_path)/models/out_model_${epoch_prior}_${id}_${cnt}_1"
                    $ns at [expr $opt(starttime) - 0.5] "$rl2($id,$cnt) loadModel $opt(campaign_path)/models/out_model_${epoch_prior}_${id}_${cnt}_2"
                    $ns at [expr $opt(starttime) - 0.5] "$rl3($id,$cnt) loadModel $opt(campaign_path)/models/out_model_${epoch_prior}_${id}_${cnt}_3"
                }
            }
        }
    }
}

# Start and stop timers
if {$opt(pktsize1) != 0} {
    
    $ns at [expr $opt(starttime) + 60.0]      "$cbr1(0,2) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr1(0,2) stop"
    $ns at [expr $opt(starttime) + 60.0]      "$cbr1(2,0) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr1(2,0) stop"

}

if {$opt(pktsize2) != 0} {

    $ns at [expr $opt(starttime) + 60.0]      "$cbr2(0,2) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr2(0,2) stop"
    $ns at [expr $opt(starttime) + 60.0]      "$cbr2(2,0) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr2(2,0) stop"
}

if {$opt(pktsize3) != 0} {

    $ns at [expr $opt(starttime) + 60.0]      "$cbr3(0,2) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr3(0,2) stop"
    $ns at [expr $opt(starttime) + 60.0]      "$cbr3(2,0) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr3(2,0) stop"
}

if {$opt(mode) == 4} {

    $ns at [expr $opt(starttime) + 60.0]      "$cbr4(0,1) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr4(0,1) stop"
    $ns at [expr $opt(starttime) + 60.0]      "$cbr4(1,0) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr4(1,0) stop"
    $ns at [expr $opt(starttime) + 60.0]      "$cbr4(2,1) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr4(2,1) stop"
    $ns at [expr $opt(starttime) + 60.0]      "$cbr4(1,2) start"
    $ns at [expr $opt(stoptime) + 60.0]       "$cbr4(1,2) stop"
}

for {set id 0} {$id < $opt(nn)} {incr id} {
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt}  {
        if {$id != $cnt} {
            $ns at [expr $opt(stoptime) + 58.0]      "$rl1($id,$cnt) startFinalProcedure $opt(cbr_period1)"
            $ns at [expr $opt(stoptime) + 59.0]      "$rl1($id,$cnt) stopFinalProcedure"
            $ns at [expr $opt(stoptime) + 58.0]      "$rl2($id,$cnt) startFinalProcedure $opt(cbr_period1)"
            $ns at [expr $opt(stoptime) + 59.0]      "$rl2($id,$cnt) stopFinalProcedure"
            $ns at [expr $opt(stoptime) + 58.0]      "$rl3($id,$cnt) startFinalProcedure $opt(cbr_period1)"
            $ns at [expr $opt(stoptime) + 59.0]      "$rl3($id,$cnt) stopFinalProcedure"
        }
    }
}

# Save weights
for {set id 0} {$id < $opt(nn)} {incr id} {
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt}  {
        if {$id != $cnt} {
            if {$opt(eval_flag) == 0} {
                set filename "$opt(campaign_path)/models/out_model_${opt(epoch)}_${id}_${cnt}"
                $ns at [expr $opt(stoptime) + 120.0] "$rl1($id,$cnt) saveModel ${filename}_1"
                $ns at [expr $opt(stoptime) + 120.0] "$rl2($id,$cnt) saveModel ${filename}_2"
                $ns at [expr $opt(stoptime) + 120.0] "$rl3($id,$cnt) saveModel ${filename}_3"
            }
        }
    }
}

# Set simulation stop time and run
$ns at [expr $opt(stoptime) + 601.0 + $opt(expire_time)] "finish; $ns halt"
$ns run