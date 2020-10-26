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
# This script test the performance of different configuration of the
# HyPoC mode
###########################################################################
# Author: Alberto Signori 
# Version: 1.0.0
#
# NOTE: tcl sample tested on Mint 18, 64 bits OS
#
# Stack of the node and the ROV
#   +---------------------------+      +-------------------------+
#   |  8. UW/CBR|UW/CBR|UW/CBR  |      |  8. UW/CBR|UW/CBR|UW/CBR|
#   +---------------------------+      +-------------------------+
#   |  7. UW/UDP                |      |  7. UW/UDP              |
#   +---------------------------+      +-------------------------+
#   |  6. UW/FLOODING           |      |  6. UW/FLOODING         | 
#   +---------------------------+      +-------------------------+
#   |  5. UW/IP                 |      |  5. UW/IP               |
#   +---------------------------+      +-------------------------+
#   |  4. UW/MULTI_TRAFFIC      |      |  4. UW/MULTI_TRAFFIC    |
#   +---------------------------+      +-------------------------+
#   |  3. UW/MLL1  |  UW/MLL2   |      |  3. UW/MLL1  |  UW/MLL2 |
#   +---------------------------+      +-------------------------+
#   |  2. UW/CSMA1 |  UW/CSMA"  |      |  2. UW/CSMA1 |  UW/CSMA2|
#   +---------------------------+      +-------------------------+
#   |  1. UW/PHY1  |  UW/PHY2   |      |  1. UW/PHY1  |  UW/PHY2 |
#   +---------------------------+      +-------------------------+
#            |         |                       |         |  
#   +------------------------------------------------------------+                  
#   |                     UnderwaterChannel                |
#   +------------------------------------------------------------+ 
######################################
# Flags to enable or disable options #
######################################
set opt(verbose)            1
set opt(trace_files)        0
set opt(bash_parameters)    1

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libUwmStd.so
load libuwcsmaaloha.so
load libuwmmac_clmsgs.so
load libuwaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwflooding.so
load libuwinterference.so
load libuwphy_clmsgs.so
load libuwphysical.so
load libuwoptical_propagation.so
load libuwoptical_channel.so
load libuwoptical_phy.so
load libuwopticalbeampattern.so
load libuwrov.so
load libuwsmposition.so
load libuwtdma.so
load libuwtdma_frame.so
load libuwmulti_traffic_control.so
load libuwposbasedrt.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################
if {$opt(bash_parameters)} {
    if {$argc != 2} {
        puts "The script requires one input:"
        puts "- for the random number generator"
        puts "example: ns test_wireless_rov.tcl 1"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        return
    } else {
        set opt(rep_num) 	[lindex $argv 0]
		set opt(send_down_delay_img) [lindex $argv 1]
    }   
} else {
	set opt(rep_num)	1
	set opt(send_down_delay_img) 0.01
}

if {$opt(trace_files)} {
    set opt(tracefilename) 		"./test_wireless_rov.tr"
    set opt(tracefile) 			[open $opt(tracefilename) w]
    set opt(cltracefilename) 	"./test_wireless_rov.cltr"
    set opt(cltracefile) 		[open $opt(tracefilename) w]
} else {
    set opt(tracefilename)	 	"/dev/null"
    set opt(tracefile) 			[open $opt(tracefilename) w]
    set opt(cltracefilename) 	"/dev/null"
    set opt(cltracefile) 		[open $opt(cltracefilename) w]
}

set opt(start_clock) [clock seconds]
global def_rng
set def_rng [new RNG]
$def_rng default
for {set k 0} {$k < $opt(rep_num)} {incr k} {
     $def_rng next-substream
}

puts "Send down img delay: $opt(send_down_delay_img)"

##############################################################################

####################
# Genereal Setting #
####################
set opt(nn)       				5.0; #number of nodes
set opt(rn)       				3.0; #number of relays
set opt(e2en)					[expr $opt(nn) - $opt(rn)] ; #number of e2e node (ROV and CTR)
set opt(starttime)    			0
set opt(stoptime)   			33000
set opt(txduration)   			[expr $opt(stoptime) - $opt(starttime)]
set opt(path_filename) 			"dbs/wrov_files/global_path.csv"

set opt(propagation_speed_ac)	1500
set opt(propagation_speed_op)	[expr 3e8/1.33]

set opt(interf_max_interval)	50

set opt(id_CTR)					0
set opt(id_ROV) 				1

##############################################################################

########################
# Applications Setting #
########################

######################################
# Joystick position control (jp_ctr) #
######################################
#Possiamo assumere che lo switch tra il controllo con joysitck
#e il controllo a waypoint sia manuale? se si, basta avere 2 
#APP distinte che trasmettono ognuna le sue posizion (una jp_ctr e una wp_ctr).
#Altrimenti devono avere la stessa APP, quindi stesso traffic type, quindi 
#nel multitraffic devo avere un phy FAST (l'ottico) e uno ROBUST (acustico MF).

set opt(filename_ROV_path) 	"dbs/wrov_files/global_path.csv"
set opt(traffic_id_jp_ctr)	1
set opt(pkt_size_jp_ctr) 	41; #4byte hdr? Need to adjust, removing headers size: 45-header size
set opt(pkt_period_jp_ctr) 	[expr $opt(stoptime)*2]
set opt(poisson_jp_ctr)		0

######################################
# Waypoint position control (wp_ctr) #
######################################
set opt(traffic_id_wp_ctr)	2
set opt(pkt_size_wp_ctr) 	41; #Need to adjust, removing headers size: 45-header size
set opt(pkt_period_wp_ctr) 	30;#[expr $opt(stoptime)*2]
set opt(poisson_wp_ctr) 		0

#######################
# HD video (video_hd) #
#######################
set opt(filename_video_hd) 		"dbs/wrov_files/reduced_video_c2_hq_parsed.csv"
set opt(traffic_id_video_hd)	3
set opt(pkt_size_video_hd) 		1000
set opt(pkt_period_video_hd) 	[expr $opt(stoptime)*2]
set opt(poisson_video_hd) 		0

###############################
# Gray-scale video (video_gs) #
###############################
set opt(filename_video_gs) 		"dbs/wrov_files/reduced_video_c4_grayscale_hq_parsed.csv"
set opt(traffic_id_video_gs)	4
set opt(pkt_size_video_gs) 		1000
set opt(pkt_period_video_gs) 	[expr $opt(stoptime)*2]
set opt(poisson_video_gs) 		0

######################################
# Very-low-quality video (video_vlq) #
######################################
set opt(filename_video_vlq) 	"dbs/wrov_files/reduced_video_vp9_vlq_parsed.csv"
set opt(traffic_id_video_vlq)	5
set opt(pkt_size_video_vlq) 	1000
set opt(pkt_period_video_vlq) 	[expr $opt(stoptime)*2]
set opt(poisson_video_vlq) 		0

#######################
# HD image (image_hd) #
#######################
# set opt(filename_image_hd) 		"dbs/wrov_files/img_time.csv"
set opt(traffic_id_image_hd)	6
set opt(pkt_size_image_hd) 		2000
set opt(pkt_period_image_hd) 	1800
set opt(poisson_image_hd) 		0
set opt(size_image_hd)			2500000

######################
# Monitoring (monit) #
######################
set opt(traffic_id_monit)	$opt(traffic_id_wp_ctr)
set opt(pkt_size_monit) 	55 
set opt(pkt_period_monit) 	6
set opt(poisson_monit) 		1
set opt(ack_priority_monit)	1

##############################################################################

###########################
# Physical layer settings #
###########################

########################################
# STACK Bluecomm optical modem (bluec) #
########################################

set opt(wavelength_bluec) 	[expr 532e-9]
set opt(freq_bluec) 		[expr $opt(propagation_speed_op)/$opt(wavelength_bluec)]; #not used in opticalbeampattern
set opt(bitrate_bluec) 		4000000
set opt(bw_bluec)    	 	5000000; #not used in opticalbeampattern
set opt(tx_power_bluec)		15; #W, not used in opticalbeampattern
set opt(beam_lut_path)     "dbs/bluecomm/beam_pattern/beam5mbps.csv"
set opt(range_lut_path)    "dbs/bluecomm/max_range/max_range5mbps.csv"

set opt(temp_bluec)       	293.15; #not used in opticalbeampattern
set opt(tx_area_bluec)      0.000010; #not used in opticalbeampattern
set opt(rx_area_bluec)      0.0000011; #not used in opticalbeampattern
set opt(c)                 	0.168 ;# Not used if variable c is set in the propagation model
set opt(theta)             	1; #not used in opticalbeampattern
set opt(id)                	[expr 1.0e-9]; #not used in opticalbeampattern
set opt(il)                	[expr 1.0e-6]; #not used in opticalbeampattern
set opt(shuntRes)          	[expr 1.49e9]; #not used in opticalbeampattern
set opt(sensitivity)       	0.39; #not used in opticalbeampattern
set opt(noise_LUT_bluec)   	"dbs/optical_noise/LUT.txt"
set opt(atten_LUT_bluec)   	"dbs/optical_attenuation/lut_532nm/lut_532nm_CTD025.csv"
set opt(noise_thr_bluec)	0


set opt(frame_duration_bluec)	[expr 40.0e-3] 
set opt(max_pkt_size_ROV_bluec) 41953 ;# Byte, video HD ;#10000;# 
set opt(guard_time_ROV_bluec)	[expr 200.0/$opt(propagation_speed_op)];#[expr 8.0*$opt(max_pkt_size_ROV_bluec)/$opt(bitrate_bluec)] ;# + 0.1e-3
set opt(guard_time_CTR_bluec)	[expr 200.0/$opt(propagation_speed_op)];#[expr 8.0*$opt(pkt_size_jp_ctr)/$opt(bitrate_bluec) + 0.02e-3]

#TODO: set inclination angle e noise threshold (threshold shoul be set to 0 to always be
# in the high noise condition)

# Set spectral mask
set data_mask_bluec [new MSpectralMask/Rect]
$data_mask_bluec setFreq       			$opt(freq_bluec)
$data_mask_bluec setBandwidth  			$opt(bw_bluec)
$data_mask_bluec setPropagationSpeed  	$opt(propagation_speed_op); #value not used

####################################
# STACK Evologic HS modem (evo_hs) #
####################################
set opt(ack_mode_evo_hs)	"setNoAckMode"

set opt(tx_power_evo_hs)	156 ;#need to be tuned to obtain correct tx range
set opt(freq_evo_hs) 		150000
set opt(bw_evo_hs) 			60000
set opt(bitrate_evo_hs) 	30000

#Set spectral mask
set data_mask_evo_hs [new MSpectralMask/Rect]
$data_mask_evo_hs setFreq       		$opt(freq_evo_hs)
$data_mask_evo_hs setBandwidth  		$opt(bw_evo_hs)
$data_mask_evo_hs setPropagationSpeed  	$opt(propagation_speed_ac); #value not used

####################################
# STACK Subnero MF modem (subn_mf) #
####################################
set opt(ack_mode_subn_mf)	"setNoAckMode"

set opt(tx_power_subn_mf) 	168.0  ;#need to be tuned to obtain correct tx range
set opt(freq_subn_mf)		24000
set opt(bw_subn_mf) 		12000
set opt(bitrate_subn_mf) 	4000

set data_mask_subn_mf [new MSpectralMask/Rect]
$data_mask_subn_mf setFreq       		$opt(freq_subn_mf)
$data_mask_subn_mf setBandwidth  		$opt(bw_subn_mf)
$data_mask_subn_mf setPropagationSpeed	$opt(propagation_speed_ac); #value not used

####################
# Channel settings #
####################

###################
# Optical channel #
###################
Module/UW/OPTICAL/Propagation set Ar_		$opt(rx_area_bluec)
Module/UW/OPTICAL/Propagation set At_		$opt(tx_area_bluec)
Module/UW/OPTICAL/Propagation set c_		$opt(c)
Module/UW/OPTICAL/Propagation set theta_	$opt(theta)
Module/UW/OPTICAL/Propagation set debug_	0

set propagation_op	[new Module/UW/OPTICAL/Propagation]
$propagation_op setVariableC
$propagation_op setLUTFileName	$opt(atten_LUT_bluec)
$propagation_op setLUT

set channel_op [new Module/UW/Optical/Channel]

####################
# Acoustic channel #
####################
MPropagation/Underwater set practicalSpreading_ 1.75
MPropagation/Underwater set windspeed_          5
MPropagation/Underwater set shipping_           1
MPropagation/Underwater set debug_              0

set propagation_ac [new MPropagation/Underwater]

Module/UnderwaterChannel set propSpeed_	$opt(propagation_speed_ac)
set channel_ac 	[new Module/UnderwaterChannel]

##############################################################################

##################
# Stack settings #
##################

#############
# APP layer #
#############

#ROV/Monitoring app
Module/UW/ROV set packetSize_          	$opt(pkt_size_monit)
Module/UW/ROV set period_              	$opt(pkt_period_monit)
Module/UW/ROV set PoissonTraffic_      	$opt(poisson_monit)
Module/UW/ROV set ackPriority_         	$opt(ack_priority_monit); #Need to be set also in MAC layer
Module/UW/ROV set ackTimeout_          	[expr 2*$opt(stoptime)]; #Not used if ack policy is piggyback
Module/UW/ROV set drop_old_waypoints_  	0
Module/UW/ROV set log_flag_            	1
Module/UW/ROV set debug_				0
# Use piggyback policy for ACK

#CTR app
Module/UW/ROV/CTR set packetSize_          		$opt(pkt_size_wp_ctr)
Module/UW/ROV/CTR set period_              		$opt(pkt_period_wp_ctr) ;#We don't use wp retx
Module/UW/ROV/CTR set PoissonTraffic_      		$opt(poisson_wp_ctr)
Module/UW/ROV/CTR set adaptiveRTO_     			0
Module/UW/ROV/CTR set adaptiveRTO_parameter_ 	1
Module/UW/ROV/CTR set debug_           			0

#General app
Module/UW/CBR set PoissonTraffic_     0
Module/UW/CBR set drop_out_of_order_  0
Module/UW/CBR set debug_              0
Module/UW/CBR set tracefile_enabler_  1

###################
# Transport layer #
###################
Module/UW/UDP set debug_ 	0

#################
# Network layer #
#################
Module/UW/IP set debug_ 	0

Module/UW/PosBasedRt set ROV_speed_   1
Module/UW/PosBasedRt set maxTxRange_  3000
Module/UW/PosBasedRt set debug_       0

Module/UW/PosBasedRt/ROV set ROV_speed_ 1
Module/UW/PosBasedRt/ROV set maxTxRange_	3000
Module/UW/PosBasedRt/ROV set debug_       0

####################
# Multimodal layer #
####################
Module/UW/MULTI_TRAFFIC_RANGE_CTR set check_to_period_    	10
Module/UW/MULTI_TRAFFIC_RANGE_CTR set signaling_pktSize_	2
Module/UW/MULTI_TRAFFIC_RANGE_CTR set debug_  				0

#############
# MAC layer #
#############
Module/UW/CSMA_ALOHA set listen_time_       [expr 4e-9]
Module/UW/CSMA_ALOHA set wait_costant_      [expr 5.0e-4]
Module/UW/CSMA_ALOHA set buffer_pkts_    	2000000
Module/UW/CSMA_ALOHA set debug_     		0
#No ack mode need to be set

Module/UW/TDMA set fair_mode            0
Module/UW/TDMA set max_packet_per_slot  10
Module/UW/TDMA set queue_size_          2000000
Module/UW/TDMA set frame_duration       $opt(frame_duration_bluec)
Module/UW/TDMA set guard_time       	[expr 75e-3]; #grater than packet duration
Module/UW/TDMA set tot_slots        	2 ;# only for fair mode
Module/UW/TDMA set drop_old_			0
Module/UW/TDMA set checkPriority_		$opt(ack_priority_monit)
Module/UW/TDMA set mac2phy_delay_       [expr 5.0e-5] ;#should be smaller for bluec, 1e-9
Module/UW/TDMA set debug_               0


#############
# PHY layer #
#############

#Optical modem
Module/UW/UWOPTICALBEAMPATTERN	set TxPower_                    $opt(tx_power_bluec)
Module/UW/UWOPTICALBEAMPATTERN	set BitRate_                    $opt(bitrate_bluec)
Module/UW/UWOPTICALBEAMPATTERN	set AcquisitionThreshold_dB_    0 ;#Not used in opticalbeampattern
Module/UW/UWOPTICALBEAMPATTERN	set Id_                         $opt(id) ;#Not used in opticalbeampattern
Module/UW/UWOPTICALBEAMPATTERN	set Il_                         $opt(il) ;#Not used in opticalbeampattern
Module/UW/UWOPTICALBEAMPATTERN	set R_                          $opt(shuntRes) ;#Not used in opticalbeampattern
Module/UW/UWOPTICALBEAMPATTERN	set S_                          $opt(sensitivity) ;#Not used in opticalbeampattern
Module/UW/UWOPTICALBEAMPATTERN	set T_                          $opt(temp_bluec) ;#Not used in opticalbeampattern
Module/UW/UWOPTICALBEAMPATTERN	set Ar_                         $opt(rx_area_bluec) ;#Not used in opticalbeampattern
Module/UW/UWOPTICALBEAMPATTERN	set noise_threshold             $opt(noise_thr_bluec)         
Module/UW/UWOPTICALBEAMPATTERN	set debug_                      0

#Acoustic modem
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(tx_power_subn_mf)
Module/UW/PHYSICAL  set BitRate_          			$opt(bitrate_subn_mf)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MinTxSPL_dB_                0
Module/UW/PHYSICAL  set MaxTxRange_                 3000
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

########################################################################################

#in this file the procedure to create nodes and relays are defined
source test_wireless_rov_nodes.tcl

#################
# Node Creation #
#################
createNode $opt(id_CTR)
createNode $opt(id_ROV)

createRelay 2
createRelay 3
createRelay 4

puts "CTR position:	X=[$position($opt(id_CTR)) getX_], Y=[$position($opt(id_CTR)) getY_], Z=[$position($opt(id_CTR)) getZ_]"
puts "Initial ROV position: X=[$position($opt(id_ROV)) getX_], Y=[$position($opt(id_ROV)) getY_], Z=[$position($opt(id_ROV)) getZ_]"

###############
# Setup flows #
###############
for {set id_src 0} {$id_src < $opt(nn)} {incr id_src}  {

	if {$id_src == $opt(id_ROV) || $id_src == $opt(id_CTR)} {
		if {$id_src == $opt(id_ROV)} {
			set id_dest $opt(id_CTR)
			$app_jp_ctr_ROV	set destAddr_ 	[$ipif_jp_ctr($id_dest) addr]
			$app_jp_ctr_ROV	set destPort_ 	$portnum_jp_ctr($id_dest)
			$app_monit set destAddr_ 		[$ipif_wp_ctr($id_dest) addr]
			$app_monit set destPort_ 		$portnum_wp_ctr($id_dest)
		} else { 
			# id_src == $opt(id_CTR)
			set id_dest $opt(id_ROV)
			$app_jp_ctr set destAddr_ 		[$ipif_jp_ctr($id_dest) addr]
			$app_jp_ctr set destPort_ 		$portnum_jp_ctr($id_dest)
			$app_wp_ctr set destAddr_ 		[$ipif_wp_ctr($id_dest) addr]
			$app_wp_ctr set destPort_ 		$portnum_wp_ctr($id_dest)
		}

		$app_video_hd($id_src)	set	destAddr_	[$ipif_video_hd($id_dest) addr]
		$app_video_hd($id_src)	set	destPort_	$portnum_video_hd($id_dest)
		$app_video_gs($id_src)	set	destAddr_	[$ipif_video_gs($id_dest) addr]
		$app_video_gs($id_src)	set	destPort_	$portnum_video_gs($id_dest)
		$app_video_vlq($id_src)	set	destAddr_	[$ipif_video_vlq($id_dest) addr]
		$app_video_vlq($id_src)	set	destPort_	$portnum_video_vlq($id_dest)
		$app_image_hd($id_src)	set	destAddr_	[$ipif_image_hd($id_dest) addr]
		$app_image_hd($id_src)	set	destPort_	$portnum_image_hd($id_dest)
	}
}

##############
# ARP tables #
##############
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
		if {$id1 == $opt(id_ROV) || $id1 == $opt(id_CTR)} {
			if {$id2 == $opt(id_ROV) || $id2 == $opt(id_CTR)} {
				$mll_bluec($id1) addentry [$ipif_video_hd($id2) addr] [$mac_bluec($id2) addr]
      			$mll_evo_hs($id1) addentry [$ipif_video_hd($id2) addr] [$mac_evo_hs($id2) addr]
      			$mll_subn_mf($id1) addentry [$ipif_video_hd($id2) addr] [$mac_subn_mf($id2) addr]
			} else {
      			$mll_subn_mf($id1) addentry [$ipif_relay($id2) addr] [$mac_relay($id2) addr]
			}	
		} else {
			if {$id2 == $opt(id_ROV) || $id2 == $opt(id_CTR)} {
				$mll_relay($id1) addentry [$ipif_video_hd($id2) addr] [$mac_subn_mf($id2) addr]
			} else {
				$mll_relay($id1) addentry [$ipif_relay($id2) addr] [$mac_relay($id2) addr]
			}
		}
      	
  	}
}

##################
# Routing tables #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
  	for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
		if { ($id1 == $opt(id_ROV) || $id1 == $opt(id_CTR)) && ($id2 == $opt(id_ROV) || $id2 == $opt(id_CTR))} {
  	    	$ipr_jp_ctr($id1) addRoute 	[$ipif_jp_ctr($id2) addr] [$ipif_jp_ctr($id2) addr]
  	    	#$ipr_wp_ctr($id1) addRoute 	[$ipif_wp_ctr($id2) addr] [$ipif_wp_ctr($id2) addr]
			$ipr_video_hd($id1) addRoute 	[$ipif_video_hd($id2) addr] [$ipif_video_hd($id2) addr]
			$ipr_video_gs($id1) addRoute 	[$ipif_video_gs($id2) addr] [$ipif_video_gs($id2) addr]
			$ipr_video_vlq($id1) addRoute [$ipif_video_vlq($id2) addr] [$ipif_video_vlq($id2) addr]
			$ipr_image_hd($id1) addRoute 	[$ipif_image_hd($id2) addr] [$ipif_image_hd($id2) addr]
	 	}
  	} ;#Need to be changed when multihop will be used
}
############################
# Setting multihop routing #
############################
$ipr_relay(2) addRoute [$ipif_wp_ctr($opt(id_CTR)) addr] [$ipif_wp_ctr($opt(id_CTR)) addr] toFixedNode
$ipr_relay(3) addRoute [$ipif_wp_ctr($opt(id_CTR)) addr] [$ipif_relay(2) addr] toFixedNode
$ipr_relay(4) addRoute [$ipif_wp_ctr($opt(id_CTR)) addr] [$ipif_relay(3) addr] toFixedNode

$ipr_wp_ctr($opt(id_CTR)) addRoute [$ipif_wp_ctr($opt(id_ROV)) addr] [$ipif_relay(2) addr] toMovingNode
$ipr_relay(2) addRoute [$ipif_wp_ctr($opt(id_ROV)) addr] [$ipif_relay(3) addr] toMovingNode
$ipr_relay(3) addRoute [$ipif_wp_ctr($opt(id_ROV)) addr] [$ipif_relay(4) addr] toMovingNode
$ipr_relay(4) addRoute [$ipif_wp_ctr($opt(id_ROV)) addr] [$ipif_wp_ctr($opt(id_ROV)) addr] toMovingNode


$ipr_wp_ctr($opt(id_ROV)) setROVPosition	$position($opt(id_ROV))
$ipr_wp_ctr($opt(id_CTR)) setNodePosition	$position($opt(id_CTR))
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	if {$id1 != $opt(id_ROV) && $id1 != $opt(id_CTR)} {
		$ipr_relay($id1) setNodePosition	$position_relay($id1)
	}

}
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
	if {$id1 != $opt(id_ROV)} {
		if {$id1 == $opt(id_CTR)} {
			$ipr_wp_ctr($opt(id_ROV)) addPosition_IPotherNodes $position($id1) 		[$ipif_wp_ctr($id1) addr]
		} else {
			$ipr_wp_ctr($opt(id_ROV)) addPosition_IPotherNodes $position_relay($id1) [$ipif_relay($id1) addr]
		}			
	}
	
}

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 

#SET WAIPOINT APPLICATIONS
set f_wp [open $opt(path_filename) r]
set file_data [read $f_wp]
set data [split $file_data "\n"]
foreach line $data {
  	if {[regexp {^(.*),(.*),(.*),(.*),(.*),(.*)$} $line -> t y x z v m]} {
		# puts "time=$t, x=$x, y=$y, z=$z, v=$v, modem=$m"  
    
		if {$m == "OPT"} {
			$ns at $t "update_and_check $m $t"
    		$ns at $t "$app_jp_ctr sendPosition $x $y $z $v"
		} else {
			$ns at $t "update_and_check $m $t"
    		$ns at $t "$app_wp_ctr sendPosition $x $y $z $v"
		}
  	}
}
close $f_wp
$ns at $opt(starttime)    "$app_monit start"
$ns at $opt(stoptime)     "$app_monit stop"

#SET VIDEO HQ APPLICATION
set video_hd [open $opt(filename_video_hd) r]
set file_data [read $video_hd]
set data [split $file_data "\n"]
foreach line $data {
  	if {[regexp {^(.*),(.*)$} $line -> t size]} {
		# puts "time=$t, packet size=$size"
		$ns at $t "$app_video_hd($opt(id_ROV)) set packetSize_ $size"
		$ns at $t "$app_video_hd($opt(id_ROV)) sendPkt"
	}
}
close $video_hd

#SET VIDEO GS APPLICATION
set video_gs [open $opt(filename_video_gs) r]
set file_data [read $video_gs]
set data [split $file_data "\n"]
foreach line $data {
  	if {[regexp {^(.*),(.*)$} $line -> t size]} {
		# puts "time=$t, packet size=$size"  
		$ns at $t "$app_video_gs($opt(id_ROV)) set packetSize_ $size"
		$ns at $t "$app_video_gs($opt(id_ROV)) sendPkt"
	}
}
close $video_gs

#SET VIDEO VLQ APPLICATION
set video_vlq [open $opt(filename_video_vlq) r]
set file_data [read $video_vlq]
set data [split $file_data "\n"]
foreach line $data {
  	if {[regexp {^(.*),(.*)$} $line -> t size]} {
		# puts "time=$t, packet size=$size"  
		$ns at $t "$app_video_vlq($opt(id_ROV)) set packetSize_ $size"
		$ns at $t "$app_video_vlq($opt(id_ROV)) sendPkt"
	}
}
close $video_vlq
 
#SET IMAGE HD APPLICATION

set a [expr srand($opt(rep_num))]
set epsiolon [expr 1e-6]
set n_pkt [expr $opt(size_image_hd)/$opt(pkt_size_image_hd)]
puts "Packets per image $n_pkt"
set k [expr -log(rand())]
set time_img [expr $k*$opt(pkt_period_image_hd)] ;#exponential random time from uniform rv
set t $time_img
set n_imgs 0
while {$t < $opt(stoptime)} {
	set n_imgs [expr $n_imgs + 1]
	puts "Image generated at $t"
	for {set i 0} {$i < $n_pkt} {incr i} {
		set send_time [expr $t + $i*$epsiolon]
		$ns at $send_time "$app_image_hd($opt(id_ROV)) set packetSize_ $opt(pkt_size_image_hd)"
		$ns at $send_time "$app_image_hd($opt(id_ROV)) sendPkt"
	}
	set k [expr -log(rand())]
	set time_img [expr $k*$opt(pkt_period_image_hd)]
	set t [expr $t + $time_img]
}
puts "Total images sent $n_imgs"

$ns at $opt(starttime)    				"$mac_bluec($opt(id_CTR)) start"
$ns at [expr $opt(stoptime) + 250.0]	"$mac_bluec($opt(id_CTR)) stop"
$ns at $opt(starttime)    				"$mac_bluec($opt(id_ROV)) start"
$ns at [expr $opt(stoptime) + 250.0]   	"$mac_bluec($opt(id_ROV)) stop"


proc update_and_check {modem time} {
	global opt position app_monit app_jp_ctr_ROV
	$position($opt(id_ROV)) update
	set outfile [open "test_wireless_rov_results.csv" "a"]
	if {$modem=="OPT"} {
		puts $outfile "position ROV at t=$time: x=[$app_jp_ctr_ROV getX], y=[$app_jp_ctr_ROV getY], z=[$app_jp_ctr_ROV getZ]" 
  	} else {
		puts $outfile "position ROV at t=$time: x=[$app_monit getX], y=[$app_monit getY], z=[$app_monit getZ]" 
  	}
  close $outfile
}


###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
  	global ns opt outfile 
  	global app_jp_ctr app_jp_ctr_ROV app_wp_ctr app_monit app_video_hd
	global app_video_gs app_video_vlq app_image_hd multitraffic
	global mac_bluec mac_evo_hs mac_subn_mf mac_relay
  	
	update_and_check "HF" [expr $opt(stoptime) + 250]
	if ($opt(verbose)) {
    	puts "---------------------------------------------------------------------"
    	puts "Simulation summary"
    	puts "Number of nodes: $opt(nn)"
		puts "JP packet size: $opt(pkt_size_jp_ctr) byte"
		puts "WP packet size: $opt(pkt_size_wp_ctr) byte"
    	puts "Monitoring packet size: $opt(pkt_size_monit) byte"
    	puts "Monitoring period: $opt(pkt_period_monit) s"
    	puts "Simulation length: $opt(txduration) s"
    	puts "---------------------------------------------------------------------"
    	puts "Tx power subnero mf: $opt(tx_power_subn_mf) dB"
    	puts "Tx frequency subnero mf: $opt(freq_subn_mf) Hz"
    	puts "Tx bandwidth subnero mf: $opt(bw_subn_mf) Hz"
    	puts "Bitrate subnero mf: $opt(bitrate_subn_mf) bps"
    	puts "---------------------------------------------------------------------"
    	puts "Tx power evologics hs: $opt(tx_power_evo_hs) dB"
    	puts "Tx frequency evologics hs: $opt(freq_evo_hs) Hz"
    	puts "Tx bandwidth evologics hs: $opt(bw_evo_hs) Hz"
    	puts "Bitrate evologics hs: $opt(bitrate_evo_hs) bps"
    	puts "---------------------------------------------------------------------"
    	puts "Tx power bluecomm: $opt(tx_power_bluec) dB"
    	puts "Tx frequency bluecomm: $opt(freq_bluec) Hz"
    	puts "Tx bandwidth bluecomm: $opt(bw_bluec) Hz"
    	puts "Bitrate bluecomm: $opt(bitrate_bluec) bps"
		puts "Bluecomm wavelength: $opt(wavelength_bluec)"
    	puts "---------------------------------------------------------------------"
  	}
	set jp_ctr_sent			[$app_jp_ctr getsentpkts]  
	set jp_ctr_recv			[$app_jp_ctr getrecvpkts]
	set jp_ctr_rov_sent		[$app_jp_ctr_ROV getsentpkts]  
	set jp_ctr_rov_recv		[$app_jp_ctr_ROV getrecvpkts]
	set wp_ctr_sent			[$app_wp_ctr getsentpkts]  
	set wp_ctr_recv			[$app_wp_ctr getrecvpkts]
	set monit_sent			[$app_monit getsentpkts]  
	set monit_recv			[$app_monit getrecvpkts]

	for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
		if {$id1 == $opt(id_ROV) || $id1 == $opt(id_CTR)} {
			set video_hd_sent($id1)			[$app_video_hd($id1) getsentpkts]  
	 		set video_hd_recv($id1)			[$app_video_hd($id1) getrecvpkts]
			set video_gs_sent($id1)			[$app_video_gs($id1) getsentpkts]  
	 		set video_gs_recv($id1)			[$app_video_gs($id1) getrecvpkts]
			set video_vlq_sent($id1)		[$app_video_vlq($id1) getsentpkts]  
	 		set video_vlq_recv($id1)		[$app_video_vlq($id1) getrecvpkts]
			set image_hd_sent($id1)			[$app_image_hd($id1) getsentpkts]  
	 		set image_hd_recv($id1)			[$app_image_hd($id1) getrecvpkts]
		} 
	}

	if ($opt(verbose)) {
    	puts "---------------------------------------------------------------------"
    	puts "Application layers"
    	puts "---------------------------------------------------------------------"
		puts "joystic position sent: $jp_ctr_sent"
		puts "joystic position recv: $jp_ctr_rov_recv"
		if {$jp_ctr_sent != 0} {
			puts "joystic position APP PDR: [expr 1.0*$jp_ctr_rov_recv/$jp_ctr_sent]"
		} else {
			puts "joystic position APP PDR: 0"
		}
		# puts "jp ctr recv: $jp_ctr_recv"
		# puts "jp ctr rov sent: $jp_ctr_rov_sent"
		puts "waypoint sent: $wp_ctr_sent"
		puts "waypoint recv: $monit_recv"
		if {$wp_ctr_sent != 0} {
			puts "waypoint APP PDR: [expr 1.0*$monit_recv/$wp_ctr_sent]"
		} else {
			puts "waypoint APP PDR: 0"
		}
		puts "monitoring sent: $monit_sent"
		puts "monitoring recv: $wp_ctr_recv"
		if {$monit_sent != 0} {
			puts "monitoring APP PDR: [expr 1.0*$wp_ctr_recv/$monit_sent]"
		} else {
			puts "monitoring APP PDR: 0"
		}
		puts "video hd sent: $video_hd_sent($opt(id_ROV))"  
	 	puts "video hd recv: $video_hd_recv($opt(id_CTR))"
		if {$video_hd_sent($opt(id_ROV)) != 0} {
			puts "video hd APP PDR: [expr 1.0*$video_hd_recv($opt(id_CTR))/$video_hd_sent($opt(id_ROV))]"
		} else {
			puts "video hd APP PDR: 0"
		}
		puts "video gs sent: $video_gs_sent($opt(id_ROV))"  
	 	puts "video gs recv: $video_gs_recv($opt(id_CTR))"
		if {$video_gs_sent($opt(id_ROV)) != 0} {
			puts "video gs APP PDR: [expr 1.0*$video_gs_recv($opt(id_CTR))/$video_gs_sent($opt(id_ROV))]"
		} else {
			puts "video gs APP PDR: 0"
		}
		puts "video vlq sent: $video_vlq_sent($opt(id_ROV))"  
	 	puts "video vlq recv: $video_vlq_recv($opt(id_CTR))"
		if {$video_vlq_sent($opt(id_ROV)) != 0} {
			puts "video vlq APP PDR: [expr 1.0*$video_vlq_recv($opt(id_CTR))/$video_vlq_sent($opt(id_ROV))]"
		} else {
			puts "video vlq APP PDR: 0"
		}
		puts "image hd sent: $image_hd_sent($opt(id_ROV))"
		puts "image hd recv: $image_hd_recv($opt(id_CTR))"
		if {$image_hd_sent($opt(id_ROV)) != 0} {
			puts "image hd APP PDR: [expr 1.0*$image_hd_recv($opt(id_CTR))/$image_hd_sent($opt(id_ROV))]"
		} else {
			puts "image hd APP PDR: 0"
		}
  	}
	for {set i 0} {$i < $opt(nn)} {incr i} {
		if {$i == $opt(id_ROV) || $i == $opt(id_CTR)} {
			puts "---------------------------------------------------------------------"
    		puts "MULTITRAFFIC layer"
    		puts "---------------------------------------------------------------------"
			puts "Discarded packet node $i traffic $opt(traffic_id_jp_ctr): [$multitraffic($i) getDiscardedPacket $opt(traffic_id_jp_ctr)]"
			puts "Discarded packet node $i traffic $opt(traffic_id_wp_ctr): [$multitraffic($i) getDiscardedPacket $opt(traffic_id_wp_ctr)]"
			puts "Discarded packet node $i traffic $opt(traffic_id_video_hd): [$multitraffic($i) getDiscardedPacket $opt(traffic_id_video_hd)]"
			puts "Discarded packet node $i traffic $opt(traffic_id_video_gs): [$multitraffic($i) getDiscardedPacket $opt(traffic_id_video_gs)]"
			puts "Discarded packet node $i traffic $opt(traffic_id_video_vlq): [$multitraffic($i) getDiscardedPacket $opt(traffic_id_video_vlq)]"
			puts "Discarded packet node $i traffic $opt(traffic_id_image_hd): [$multitraffic($i) getDiscardedPacket $opt(traffic_id_image_hd)]"

			puts "Probe packet sent node $i traffic $opt(traffic_id_jp_ctr): [$multitraffic($i) getProbeSent $opt(traffic_id_jp_ctr)]"
			puts "Probe packet sent node $i traffic $opt(traffic_id_wp_ctr): [$multitraffic($i) getProbeSent $opt(traffic_id_wp_ctr)]"
			puts "Probe packet sent node $i traffic $opt(traffic_id_video_hd): [$multitraffic($i) getProbeSent $opt(traffic_id_video_hd)]"
			puts "Probe packet sent node $i traffic $opt(traffic_id_video_gs): [$multitraffic($i) getProbeSent $opt(traffic_id_video_gs)]"
			puts "Probe packet sent node $i traffic $opt(traffic_id_video_vlq): [$multitraffic($i) getProbeSent $opt(traffic_id_video_vlq)]"
			puts "Probe packet sent node $i traffic $opt(traffic_id_image_hd): [$multitraffic($i) getProbeSent $opt(traffic_id_image_hd)]"

			puts "Probe packet recv node $i traffic $opt(traffic_id_jp_ctr): [$multitraffic($i) getProbeRecv $opt(traffic_id_jp_ctr)]"
			puts "Probe packet recv node $i traffic $opt(traffic_id_wp_ctr): [$multitraffic($i) getProbeRecv $opt(traffic_id_wp_ctr)]"
			puts "Probe packet recv node $i traffic $opt(traffic_id_video_hd): [$multitraffic($i) getProbeRecv $opt(traffic_id_video_hd)]"
			puts "Probe packet recv node $i traffic $opt(traffic_id_video_gs): [$multitraffic($i) getProbeRecv $opt(traffic_id_video_gs)]"
			puts "Probe packet recv node $i traffic $opt(traffic_id_video_vlq): [$multitraffic($i) getProbeRecv $opt(traffic_id_video_vlq)]"
			puts "Probe packet recv node $i traffic $opt(traffic_id_image_hd): [$multitraffic($i) getProbeRecv $opt(traffic_id_image_hd)]"

			puts "Probe ACK packet sent node $i traffic $opt(traffic_id_jp_ctr): [$multitraffic($i) getProbeAckSent $opt(traffic_id_jp_ctr)]"
			puts "Probe ACK packet sent node $i traffic $opt(traffic_id_wp_ctr): [$multitraffic($i) getProbeAckSent $opt(traffic_id_wp_ctr)]"
			puts "Probe ACK packet sent node $i traffic $opt(traffic_id_video_hd): [$multitraffic($i) getProbeAckSent $opt(traffic_id_video_hd)]"
			puts "Probe ACK packet sent node $i traffic $opt(traffic_id_video_gs): [$multitraffic($i) getProbeAckSent $opt(traffic_id_video_gs)]"
			puts "Probe ACK packet sent node $i traffic $opt(traffic_id_video_vlq): [$multitraffic($i) getProbeAckSent $opt(traffic_id_video_vlq)]"
			puts "Probe ACK packet sent node $i traffic $opt(traffic_id_image_hd): [$multitraffic($i) getProbeAckSent $opt(traffic_id_image_hd)]"

			puts "Probe ACK packet recv node $i traffic $opt(traffic_id_jp_ctr): [$multitraffic($i) getProbeAckRecv $opt(traffic_id_jp_ctr)]"
			puts "Probe ACK packet recv node $i traffic $opt(traffic_id_wp_ctr): [$multitraffic($i) getProbeAckRecv $opt(traffic_id_wp_ctr)]"
			puts "Probe ACK packet recv node $i traffic $opt(traffic_id_video_hd): [$multitraffic($i) getProbeAckRecv $opt(traffic_id_video_hd)]"
			puts "Probe ACK packet recv node $i traffic $opt(traffic_id_video_gs): [$multitraffic($i) getProbeAckRecv $opt(traffic_id_video_gs)]"
			puts "Probe ACK packet recv node $i traffic $opt(traffic_id_video_vlq): [$multitraffic($i) getProbeAckRecv $opt(traffic_id_video_vlq)]"
			puts "Probe ACK packet recv node $i traffic $opt(traffic_id_image_hd): [$multitraffic($i) getProbeAckRecv $opt(traffic_id_image_hd)]"
		}
	}

	for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
		if {$id1 == $opt(id_ROV) || $id1 == $opt(id_CTR)} {
			set mac_bluec_sent($id1)	[$mac_bluec($id1) getDataPktsTx] 
			set mac_bluec_recv($id1)    [$mac_bluec($id1) getDataPktsRx]
			set mac_bluec_queue($id1)	[$mac_bluec($id1) get_buffer_size]
			set mac_evo_hs_sent($id1)	[$mac_evo_hs($id1) getDataPktsTx] 
			set mac_evo_hs_recv($id1)   [$mac_evo_hs($id1) getDataPktsRx]
			set mac_evo_hs_queue($id1)	[$mac_evo_hs($id1) getQueueSize]
			set mac_subn_mf_sent($id1)	[$mac_subn_mf($id1) getDataPktsTx]
			set mac_subn_mf_recv($id1)    [$mac_subn_mf($id1) getDataPktsRx] 
			set mac_subn_mf_queue($id1)    [$mac_subn_mf($id1) getQueueSize] 
		} else {
			set mac_relay_sent($id1)	[$mac_relay($id1) getDataPktsTx]
			set mac_relay_recv($id1)    [$mac_relay($id1) getDataPktsRx]
			set mac_relay_queue($id1)    [$mac_relay($id1) getQueueSize] 
		}
	}
	if ($opt(verbose)) {
    	puts "---------------------------------------------------------------------"
    	puts "MAC layers"
    	puts "---------------------------------------------------------------------"
		for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
			if {$id1 == $opt(id_ROV) || $id1 == $opt(id_CTR)} {
				puts "bluec mac($id1) sent: $mac_bluec_sent($id1)"	 
				puts "bluec mac($id1) recv: $mac_bluec_recv($id1)"
				puts "bluec mac($id1) queue: $mac_bluec_queue($id1)"    
				puts "evo hs mac($id1) sent: $mac_evo_hs_sent($id1)"	 
				puts "evo hs mac($id1) recv: $mac_evo_hs_recv($id1)"
				puts "evo hs mac($id1) queue: $mac_evo_hs_queue($id1)"   
				puts "subn mf mac($id1) sent: $mac_subn_mf_sent($id1)"	
				puts "subn_mf mac($id1) recv: $mac_subn_mf_recv($id1)"
				puts "subn_mf mac($id1) queue: $mac_subn_mf_queue($id1)"   
			} else {
				puts "relay mac($id1) sent: $mac_relay_sent($id1)"	
				puts "relay mac($id1) recv: $mac_relay_recv($id1)"
				puts "relay mac($id1) queue: $mac_relay_queue($id1)"     
			}
		}
	}
   
#  set ipheadersize        [$ipif_ROV(0) getipheadersize]
#  set udpheadersize       [$udp_ROV(0) getudpheadersize]
#  set ROVheadersize       [$application_ROV getROVMonheadersize]
#  set CTRheadersize       [$application_CTR getROVctrheadersize]
#  set CBRheadersize     [$application_ROV getcbrheadersize]

    
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