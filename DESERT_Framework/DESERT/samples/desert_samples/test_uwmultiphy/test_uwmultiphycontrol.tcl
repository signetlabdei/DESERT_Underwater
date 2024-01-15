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
# This script is used to test MULTI_STACK_CONTROLLER_PHY MASTER and SLAVE module
# There are 2 nodes that can transmit each other packets with a CBR (Constant
# Bit Rate) Application Module
# The MASTER controls the switch between three UW/PHYSICAL layers with different
# frequency and bandwidth, according with the received power metrics. The slave
# switches according to the MASTER behavior. UnderwaterChannel is used as channel.
#
# Author: Enrico Lovisotto <enricolovisotto@gmail.com>
# Version: 1.0.0
#
# NOTE: tcl sample tested on Ubuntu 11.10, 64 bits OS
#
# Stack of the nodes
#                   MASTER                                         SLAVE
#   +------------------------------------------+   +------------------------------------------+
#   |  10. UW/ROV/CTR                          |   |  10. UW/ROV                              |
#   +------------------------------------------+   +------------------------------------------+
#   |  9. UW/UDP                               |   |  9. UW/UDP                               |
#   +------------------------------------------+   +------------------------------------------+
#   |  8. UW/STATICROUTING                     |   |  8. UW/STATICROUTING                     |
#   +------------------------------------------+   +------------------------------------------+
#   |  7. UW/IP                                |   |  7. UW/IP                                |
#   +------------------------------------------+   +------------------------------------------+
#   |  6. UW/MLL                               |   |  6. UW/MLL                               |
#   +------------------------------------------+   +------------------------------------------+
#   |  5. UW/TDMA                              |   |  5. UW/TDMA                              |
#   +------------------------------------------+   +------------------------------------------+
#   |  4. UW/MULTI_STACK_CONTROLLER_PHY_MASTER |   |  4. UW/MULTI_STACK_CONTROLLER_PHY_SLAVE  |
#   +--------------+-------------+-------------+   +--------------+-------------+-------------+
#   | 3.UW/PHYSICAL|2.UW/PHYSICAL|1.UW/PHYSICAL|   | 3.UW/PHYSICAL|2.UW/PHYSICAL|1.UW/PHYSICAL|
#   +--------------+-------------+-------------+   +--------------+-------------+-------------+
#           |             |              |                  |             |            |
#   +-----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                   |
#   +-----------------------------------------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 		1	
set opt(trace_files)		0
set opt(bash_parameters) 	0

proc log {message} {
  global opt

  if { $opt(verbose) > 0 } {
    puts "TCL::$message"
  }
}

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwcsmaaloha.so
load libuwaloha.so
load libuwphy_clmsgs.so
load libuwinterference.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwhermesphy.so
load libuwoptical_propagation.so
load libuwem_channel.so
load libuwoptical_channel.so
load libuwoptical_phy.so
load libuwrov.so
load libuwsmposition.so
load libuwmmac_clmsgs.so
load libuwtdma.so
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
set opt(number_of_nodes)    2

# movement parameters
set opt(D)                  1000.0
set opt(speed)              2.0

set opt(starttime)          1.0
set opt(stoptime)           300.0
set opt(time_interval)      12
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)] ;# Duration of the simulation

set opt(maxinterval_)       20.0
set opt(ack_mode)           "setNoAckMode"

set opt(cbr_period)         1.5
set opt(pktsize)	          125
set opt(rngstream)	          1

####################
# Evologics' 48/78 #
####################

# set opt(freq)               63000.0 ;#Frequency used in Hz
# set opt(bw)                 30000.0	;#Bandwidth used in Hzz
# set opt(bitrate)            30000.0	;#bitrate in bps
# set opt(txpower)            160.0  ;#Power transmitted in dB re uPa

# #####################
# # Evologics' S2C HS #
# #####################

set opt(S2C_freq)               160000.0; # Frequency used in Hz
set opt(S2C_bw)                 80000.0;  # Bandwidth used in Hzz
set opt(S2C_bitrate)            60000.0;  # bitrate in bps, up to 62.5
set opt(S2C_txpower)            177.0;    # Power transmitted in dB re uPa (10W)

######################
# Hermes             #
######################

set opt(hermes_freq)               375000.0; #Frequency used in Hz
set opt(hermes_bw)                 76000.0;  #Bandwidth used in Hz
set opt(hermes_bitrate)            87768.0;  #150000;#bitrate in bps
set opt(hermes_txpower)            180.0;    #Power transmitted in dB re uPa (32 W)

######################
# Optical            #
######################

set opt(optical_freq)              10000000
# set opt(optical_bw)                100000
# set opt(optical_bitrate)           1000000
set opt(optical_bw)                200000
set opt(optical_bitrate)           2000000
set opt(optical_txpower)           100
set opt(opt_acq_db)        10
set opt(temperatura)       293.15 ; # in Kelvin
set opt(txArea)            0.000010
set opt(rxArea)            0.0000011 ; # receveing area, it has to be the same for optical physical and propagation
set opt(c)                 0.15 ; # seawater attenuation coefficient
set opt(theta)             1
set opt(id)                [expr 1.0e-9]
set opt(il)                [expr 1.0e-6]
set opt(shuntRes)          [expr 1.49e9]
set opt(sensitivity)       0.26
set opt(LUTpath)           "../dbs/optical_noise/LUT.txt"
set opt(cbr_period) 60
set opt(pktsize)	125
set opt(rngstream)	1

if {$opt(bash_parameters)} {
	if {$argc != 3} {
		puts "The script requires three inputs:"
		puts "- the first for the seed"
		puts "- the second one is for the Poisson CBR period"
		puts "- the third one is the cbr packet size (byte);"
		puts "example: ns test_uw_csma_aloha_fully_connected.tcl 1 60 125"
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

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwmultiphy.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwmultiphy.cttr"
	set opt(cltracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}

MPropagation/Underwater set practicalSpreading_ 1.5
MPropagation/Underwater set debug_              0
MPropagation/Underwater set windspeed_          1

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set S2C_data_mask [new MSpectralMask/Rect]
$S2C_data_mask setFreq       $opt(S2C_freq)
$S2C_data_mask setBandwidth  $opt(S2C_bw)

set hermes_data_mask [new MSpectralMask/Rect]
$hermes_data_mask setFreq       $opt(hermes_freq)
$hermes_data_mask setBandwidth  $opt(hermes_bw)

set optical_data_mask [new MSpectralMask/Rect]
$optical_data_mask setFreq       $opt(optical_freq)
$optical_data_mask setBandwidth  $opt(optical_bw)

#########################
# Module Configuration  #
#########################

Position/UWSM set debug_ 0

Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set debug_               0

Module/UW/StaticRouting set debug_ 0
Module/UW/IP            set debug_ 0

# TDMA
Module/UW/TDMA set frame_duration 6.0
Module/UW/TDMA set debug_               0
# Module/UW/TDMA set guard_time 0.5
#Module/UW/TDMA set guard_time 0.2
Module/UW/TDMA set ACK_size_           0
Module/UW/TDMA set max_tx_tries_               1
Module/UW/TDMA set max_payload_                10000
Module/UW/TDMA set ACK_timeout_                10000.0
Module/UW/TDMA set listen_time_          [expr 1.0e-8]
Module/UW/TDMA set wait_costant_         [expr 1.0e-12]
Module/UW/TDMA set fair_mode        0

Module/UW/CSMA_ALOHA set debug_        0

# three different PHY
Module/UW/PHYSICAL  set BitRate_                    $opt(S2C_bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(S2C_txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                0
Module/UW/PHYSICAL  set MaxTxRange_                 50000
Module/UW/PHYSICAL  set PER_target_                 0
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

Module/UW/HERMES/PHY  set BitRate_                    $opt(hermes_bitrate)
Module/UW/HERMES/PHY  set AcquisitionThreshold_dB_    15.0
Module/UW/HERMES/PHY  set RxSnrPenalty_dB_            0
Module/UW/HERMES/PHY  set TxSPLMargin_dB_             0
Module/UW/HERMES/PHY  set MaxTxSPL_dB_                $opt(hermes_txpower)
Module/UW/HERMES/PHY  set MinTxSPL_dB_                0
Module/UW/HERMES/PHY  set MaxTxRange_                 200
Module/UW/HERMES/PHY  set PER_target_                 0
Module/UW/HERMES/PHY  set CentralFreqOptimization_    0
Module/UW/HERMES/PHY  set BandwidthOptimization_      0
Module/UW/HERMES/PHY  set SPLOptimization_            0
Module/UW/HERMES/PHY  set debug_                      0

Module/UW/OPTICAL/PHY   set TxPower_                    $opt(optical_txpower)
Module/UW/OPTICAL/PHY   set BitRate_                    $opt(optical_bitrate)
Module/UW/OPTICAL/PHY   set AcquisitionThreshold_dB_    $opt(opt_acq_db)
Module/UW/OPTICAL/PHY   set Id_                         $opt(id)
Module/UW/OPTICAL/PHY   set Il_                         $opt(il)
Module/UW/OPTICAL/PHY   set R_                          $opt(shuntRes)
Module/UW/OPTICAL/PHY   set S_                          $opt(sensitivity)
Module/UW/OPTICAL/PHY   set T_                          $opt(temperatura)
Module/UW/OPTICAL/PHY   set Ar_                         $opt(rxArea)
Module/UW/OPTICAL/PHY   set debug                      0

Module/UW/OPTICAL/Propagation set Ar_       $opt(rxArea)
Module/UW/OPTICAL/Propagation set At_       $opt(txArea)
Module/UW/OPTICAL/Propagation set c_        $opt(c)
Module/UW/OPTICAL/Propagation set theta_    $opt(theta)
Module/UW/OPTICAL/Propagation set debug_    0

set optical_propagation [new Module/UW/OPTICAL/Propagation]
$optical_propagation setOmnidirectional

set optical_channel [new Module/UW/Optical/Channel]

Module/UW/MULTIPHY_CONTROLLER set debug_ 0

########################
# Handle node position #
########################
set pi [expr acos(-1.0)]

proc getPositionAtTime { angle D } {
  set x [expr $D * cos($angle) / (2 - cos(2 * $angle))]
  set y [expr $D * sin($angle) * cos($angle) / (2 - cos(2 * $angle))]
  set z -15

  return [list $x $y $z]
}

################################
# Procedure(s) to create nodes #
################################
proc createNode {id} {

  puts "Creating node $id"

  global channel propagation optical_propagation S2C_data_mask hermes_data_mask optical_data_mask ns application position node udp portnum cbr
  global S2C_mll S2C_mac
  global hermes_mll hermes_mac
  global optical_mll optical_mac
  global phy posdb opt rvposx rvposy rvposz mhrouting woss_utilities woss_creator db_manager channel_estimator
  global ipr ipif
  global node_coordinates optical_channel ctr

  set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

  ##################
  # Create modules #
  ##################

  # connect node with all others through applications
  for {set cnt 0} {$cnt < $opt(number_of_nodes)} {incr cnt} {
    set cbr($id,$cnt)  [new Module/UW/CBR]
	}
  set udp($id)  [new Module/UW/UDP]
  set ipr($id)  [new Module/UW/StaticRouting]
  set ipif($id) [new Module/UW/IP]

  set ctr($id) [new Module/UW/MULTIPHY_CONTROLLER]

  # TODO evaluate switching to TDMA

  # two mini-stacks below control layer
  set S2C_mll($id)  [new Module/UW/MLL]
  set S2C_mac($id)  [new Module/UW/CSMA_ALOHA]
  set S2C_phy($id)  [new Module/UW/PHYSICAL]

  set hermes_mll($id)  [new Module/UW/MLL]
  set hermes_mac($id)  [new Module/UW/CSMA_ALOHA]
  set hermes_phy($id)  [new Module/UW/HERMES/PHY]

  set optical_mll($id)  [new Module/UW/MLL]
  set optical_mac($id)  [new Module/UW/CSMA_ALOHA]
  set optical_phy($id)  [new Module/UW/OPTICAL/PHY]

  #######################
  # Add modules to node #
  #######################
	for {set cnt 0} {$cnt < $opt(number_of_nodes)} {incr cnt} {
		$node($id) addModule 14 $cbr($id,$cnt)   0  "CBR"
	}

  # node - addModule - layer level -  module - trace lavel in ClSAP
  $node($id) addModule 13 $udp($id)   0  "UDP"
  $node($id) addModule 12  $ipr($id)   0  "IPR"
  $node($id) addModule 11  $ipif($id)  0  "IPF"

  $node($id) addModule 10  $ctr($id)   0  "CTR"

  # three mini-stacks below control layer
  $node($id) addModule 9  $S2C_mll($id)  0  "MLL_S2C"
  $node($id) addModule 8  $S2C_mac($id)  0  "MAC_S2C"
  $node($id) addModule 7  $S2C_phy($id)  0  "PHY_S2C"

  $node($id) addModule 6  $hermes_mll($id)  0  "MLL_HER"
  $node($id) addModule 5  $hermes_mac($id)  0  "MAC_HER"
  $node($id) addModule 4  $hermes_phy($id)  0  "PHY_HER"

  $node($id) addModule 3  $optical_mll($id)  0  "MLL_OPT"
  $node($id) addModule 2  $optical_mac($id)  0  "MAC_OPT"
  $node($id) addModule 1  $optical_phy($id)  0  "PHY_OPT"

  ##################################
  # Set connections between layers #
  ##################################

  for {set cnt 0} {$cnt < $opt(number_of_nodes)} {incr cnt} {
		$node($id) setConnection $cbr($id,$cnt) $udp($id) 0
		set portnum($id,$cnt) [$udp($id) assignPort $cbr($id,$cnt) ]
	}

  $node($id) setConnection $udp($id)   $ipr($id)  0
  $node($id) setConnection $ipr($id)   $ipif($id) 0
  $node($id) setConnection $ipif($id)  $ctr($id)  0

  $node($id) setConnection $ctr($id)      $S2C_mll($id) 0
  $node($id) setConnection $S2C_mll($id)  $S2C_mac($id) 0
  $node($id) setConnection $S2C_mac($id)  $S2C_phy($id) 0
  $node($id) addToChannel  $channel       $S2C_phy($id) 0

  $node($id) setConnection $ctr($id)         $hermes_mll($id) 0
  $node($id) setConnection $hermes_mll($id)  $hermes_mac($id) 0
  $node($id) setConnection $hermes_mac($id)  $hermes_phy($id) 0
  $node($id) addToChannel  $channel          $hermes_phy($id) 0

  $node($id) setConnection $ctr($id)          $optical_mll($id) 0
  $node($id) setConnection $optical_mll($id)  $optical_mac($id) 0
  $node($id) setConnection $optical_mac($id)  $optical_phy($id) 0
  $node($id) addToChannel  $optical_channel   $optical_phy($id) 0

  if {$id > 254} {
  	puts "hostnum > 254!!! exiting"
  	exit
  }

  ##################################
  # Set the IP address of the node #
  ##################################
  $ipif($id) addr [expr $id + 1]

  ###################
  # Setup MAC layer #
  ###################
  $S2C_mac($id) $opt(ack_mode)
  $S2C_mac($id) initialize

  $hermes_mac($id) $opt(ack_mode)
  $hermes_mac($id) initialize

  $optical_mac($id) $opt(ack_mode)
  $optical_mac($id) initialize

  set position($id) [new "Position/UWSM"]
  $node($id) addPosition $position($id)

  #Setup positions
  if { $id == 1 } {
    set node_position [getPositionAtTime 0 $opt(D)]

    $position($id) setX_ [lindex $node_position 0]
    $position($id) setY_ [lindex $node_position 1]
    $position($id) setZ_ [lindex $node_position 2]

  } else {
    $position($id) setX_ 0
    $position($id) setY_ 0
    $position($id) setZ_ -15
  }

  ########################
  # Setup common channel #
  ########################

  #S2C_Interference model
  set S2C_interf_data($id) [new "Module/UW/INTERFERENCE"]
  $S2C_interf_data($id) set maxinterval_ $opt(maxinterval_)
  $S2C_interf_data($id) set debug_       0

  set hermes_interf_data($id) [new "Module/UW/INTERFERENCE"]
  $hermes_interf_data($id) set maxinterval_ $opt(maxinterval_)
  $hermes_interf_data($id) set debug_       0

  set optical_interf_data($id) [new "MInterference/MIV"]
  $optical_interf_data($id) set maxinterval_ $opt(maxinterval_)
  $optical_interf_data($id) set debug_       0

	#Propagation model
  $S2C_phy($id) setPropagation $propagation
  $S2C_phy($id) setSpectralMask $S2C_data_mask
  $S2C_phy($id) setInterference $S2C_interf_data($id)

  $hermes_phy($id) setPropagation $propagation
  $hermes_phy($id) setSpectralMask $hermes_data_mask
  $hermes_phy($id) setInterference $hermes_interf_data($id)
  $hermes_phy($id) setInterferenceModel "MEANPOWER"
  $hermes_phy($id) setLUTFileName "../dbs/hermes/default.csv"
  $hermes_phy($id) initLUT

  $optical_phy($id) setPropagation $optical_propagation
  $optical_phy($id) setSpectralMask $optical_data_mask
  $optical_phy($id) setInterference $optical_interf_data($id)
  $optical_phy($id) setLUTFileName "$opt(LUTpath)"
  $optical_phy($id) setLUTSeparator " "
  $optical_phy($id) useLUT

  ###################
  # Setup CTR layer #
  ###################

  # register down layer IDs and everything

  log "node($id)::S2C_mll::tclID([$S2C_mll($id) Id_])"
  log "node($id)::hermes_mll::tclID([$hermes_mll($id) Id_])"
  log "node($id)::optical_mll::tclID([$optical_mll($id) Id_])"
  log "node($id)::IP::addr([expr $id + 1])"

  $ctr($id) initialize

  # basically random numbers here: testing needed for assessing them
  $ctr($id) setResilienceTimeout [$S2C_mll($id) Id_]      20.
  $ctr($id) setResilienceTimeout [$hermes_mll($id) Id_]   20.
  $ctr($id) setResilienceTimeout [$optical_mll($id) Id_]  20.

  $ctr($id) setProbeTimeout      [$S2C_mll($id) Id_]      20.
  $ctr($id) setProbeTimeout      [$hermes_mll($id) Id_]   20.
  $ctr($id) setProbeTimeout      [$optical_mll($id) Id_]  20.

  # higher number corresponds to a more resilient, robust PHY
  $ctr($id) setMacResilience     [$S2C_mll($id) Id_]      3
  $ctr($id) setMacResilience     [$hermes_mll($id) Id_]   2
  $ctr($id) setMacResilience     [$optical_mll($id) Id_]  1

  # in general, prefer faster PHY
  $ctr($id) setDefaultPriority   [$S2C_mll($id) Id_]      1
  $ctr($id) setDefaultPriority   [$hermes_mll($id) Id_]   2
  $ctr($id) setDefaultPriority   [$optical_mll($id) Id_]  3

  # custom priorities are set to prefer a PHY given a local APP
  # and remote IP (will be set later, once all nodes are created)
}

#################
# Node Creation #
#################

# create here all the nodes you want to connect together
for {set id 0} {$id < $opt(number_of_nodes)} {incr id}  {
  createNode $id
}

################################
# Inter-node module connection #
################################

proc connectNodes {id1 des1} {
  global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink opt

  log "connectNodes::node($id1)::dest($des1)"

  $cbr($id1,$des1) set destAddr_ [$ipif($des1) addr]
  $cbr($id1,$des1) set destPort_ $portnum($des1,$id1)

  $cbr($des1,$id1) set destAddr_ [$ipif($id1) addr]
  $cbr($des1,$id1) set destPort_ $portnum($id1,$des1)
}

# create a mesh network
for {set id1 0} {$id1 < $opt(number_of_nodes)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(number_of_nodes)} {incr id2}  {
    if { $id1 != $id2 } {
      connectNodes $id1 $id2
    }
	}
}

##################
# ARP tables     #
##################
for {set id1 0} {$id1 < $opt(number_of_nodes)} {incr id1}  {
  for {set id2 0} {$id2 < $opt(number_of_nodes)} {incr id2}  {
    $S2C_mll($id1) addentry [$ipif($id2) addr] [$S2C_mac($id2) addr]
    $hermes_mll($id1) addentry [$ipif($id2) addr] [$hermes_mac($id2) addr]
    $optical_mll($id1) addentry [$ipif($id2) addr] [$optical_mac($id2) addr]
  }
}

##################
# Routing tables #
##################
for {set id1 0} {$id1 < $opt(number_of_nodes)} {incr id1}  {
	for {set id2 0} {$id2 < $opt(number_of_nodes)} {incr id2}  {
      $ipr($id1) addRoute [$ipif($id2) addr] [$ipif($id2) addr]
	}
}


######################
# Start applications #
######################
for {set id 0} {$id < $opt(number_of_nodes)} {incr id}  {
  for {set other_id 0} {$other_id < $opt(number_of_nodes)} {incr other_id} {
    if { $id != $other_id } {
      log "CBR::start::node($id)::destination($other_id)"

      $ns at $opt(starttime)    "$cbr($id,$other_id) start"
      $ns at $opt(stoptime)     "$cbr($id,$other_id) stop"
    }
  }
}

#################
# Node movement #
#################
proc formatPosition { id } {
  global position
  return "[$position($id) getX_],[$position($id) getY_],[$position($id) getZ_]"
}

# previous position
set oldX [$position(1) getX_]
set oldY [$position(1) getY_]
set oldZ [$position(1) getZ_]

# create shape points
set n_points 1000

set t 0.0
set point 0
while {$t < $opt(stoptime)} {
  # log TIME::$t

  # find next position to reach
  set alpha [expr 2 * $pi * $point / $n_points]
  set pos [getPositionAtTime $alpha $opt(D)]
  set x [lindex $pos 0]
  set y [lindex $pos 1]
  set z [lindex $pos 2]

  # move the node
  # log "MOVETO::(t,x,y,z)=($t,$x,$y,$z)"
  $ns at $t "$position(1) setdest $x $y $z $opt(speed)"
  $ns at $t "log \"POSITION::(t,x,y,z)=($t,\[formatPosition 1\])\""

  # update loop variables
  set dist [expr sqrt(pow($x - $oldX, 2) + \
                      pow($y - $oldY, 2) + \
                      pow($z - $oldZ, 2))]

  # log DIST::$dist
  set t [expr $t + $dist / $opt(speed)]

  set oldX $x
  set oldY $y
  set oldZ $z

  incr point
}

#$ns at 5 "$position(1) setdest 600 0 -15 $opt(speed)"
#$ns at 5 "log \"POSITION::(t,x,y,z)=($t,\[formatPosition 1\])\""

#####################
# Throughput report #
#####################
set result_path "throughput-$opt(rngstream).csv"

# write titles on output csv
set outfile [open $result_path "w"]
puts $outfile "t,tx_id,tx_x,tx_y,tx_z,rx_id,rx_x,rx_y,rx_z,n_pkts"
close $outfile

proc putThroughput { t } {
  global cbr opt result_path

  set outfile [open $result_path "a"]

  for {set i 0} {$i < $opt(number_of_nodes)} {incr i}  {
    for {set j 0} {$j < $opt(number_of_nodes)} {incr j} {
      if { $i != $j } {
        log "CBR::t($t)::i($i)::j($j)::n_recv([$cbr($i,$j) getrecvpkts])"
        puts $outfile "$t,$i,[formatPosition $i],$j,[formatPosition $j],[$cbr($i,$j) getrecvpkts]"
      }
    }
  }

  close $outfile
}

set t $opt(starttime)
set updateThr 0.5

while {$t < $opt(stoptime)} {
  $ns at $t "putThroughput $t"
  set t [expr $t + $updateThr]
}

# Define here the procedure to call at the end of the simulation
proc finish {} {
  global ns opt outfile

  global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
  global node_coordinates;      #
  global ipr_sink ipr ipif udp cbr phy phy_data_sink
  global node_stats tmp_node_stats sink_stats tmp_sink_stats
  if ($opt(verbose)) {          #
    puts "---------------------------------------------------------------------"
    puts "Simulation summary";  #
    puts "number of nodes  : $opt(number_of_nodes)"
    puts "packet size      : $opt(pktsize) byte"
    puts "cbr period       : $opt(cbr_period) s"
    puts "number of nodes  : $opt(number_of_nodes)"
    puts "---------------------------------------------------------------------"
  }
  set sum_cbr_throughput     0
  set sum_per                0
  set sum_cbr_sent_pkts      0.0
  set sum_cbr_rcv_pkts       0.0

  for {set i 0} {$i < $opt(number_of_nodes)} {incr i}  {
    for {set j 0} {$j < $opt(number_of_nodes)} {incr j} {
      set cbr_throughput           [$cbr($i,$j) getthr]
      if {$i != $j} {
        set cbr_sent_pkts        [$cbr($i,$j) getsentpkts]
        set cbr_rcv_pkts         [$cbr($i,$j) getrecvpkts]
        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]

        if ($opt(verbose)) {
          puts "cbr($i,$j) throughput                    : $cbr_throughput"
        }
      }
    }
  }

  set ipheadersize        [$ipif(1) getipheadersize]
  set udpheadersize       [$udp(1) getudpheadersize]
  set cbrheadersize       [$cbr(1,0) getcbrheadersize]

  if ($opt(verbose)) {
    puts "Mean Throughput          : [expr ($sum_cbr_throughput/(($opt(number_of_nodes))*($opt(number_of_nodes)-1)))]"
    puts "Sent Packets             : $sum_cbr_sent_pkts"
    puts "Received Packets         : $sum_cbr_rcv_pkts"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"
    puts "IP Pkt Header Size       : $ipheadersize"
    puts "UDP Header Size          : $udpheadersize"
    puts "CBR Header Size          : $cbrheadersize"
    puts "done!"
  }

  $ns flush-trace
  close $opt(tracefile)
}

####################
# start simulation #
####################
if ($opt(verbose)) {
  puts "\nStarting Simulation\n"
  puts "----------------------------------------------"
}

$ns at [expr $opt(stoptime) + 50.0]  "finish; $ns halt"

$ns run
