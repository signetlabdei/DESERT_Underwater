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
# This script is used to test the YAML input/output interface.
# It make use of TDMA with a CBR (Constant Bit Rate) Application Module
# Here the complete stack used for each node in the simulation.
#
# Authors: Vincenzo Cimino, Filippo Donegà
# Version: 1.0.0
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
#   |  2. UW/TDMA             |
#   +-------------------------+
#   |  1. UW/PHYSICAL         |
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |   UnderwaterChannel     |
#   +-------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(trace_files)	0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libuwip.so
load libuwstaticrouting.so
load libmphy.so
load libmmac.so
load libuwmmac_clmsgs.so
load libuwphy_clmsgs.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwtdma.so
load libuwinterference.so
load libUwmStd.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

global positions

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]

########################
# Configure simulation #
########################
source "../yaml_input_interface/get-config.tcl"

set input_config_filename "./input_config.yaml"
load-config $input_config_filename

set positions_filename "./input_positions.yaml"
load-positions $positions_filename

source "get-output-config.tcl"
set output_config_filename "./output_config.yaml"

####################################
# Set the physical characteristics #
####################################
$data_mask setFreq              $opt(freq)
$data_mask setBandwidth         $opt(bw)
$data_mask setPropagationSpeed  $opt(propagation_speed)

##################################
# Set random seed and tracefiles #
##################################
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}
if {$opt(trace_files)} {
	set opt(tracefilename) "./test_uwtdma_simple.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_uwtdma_simple.cltr"
	set opt(cltracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {
    global opt ns node portnum
    global channel propagation data_mask interf_data positions position 
	global cbr udp ipr ipif mll mac phy
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
	for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if { $id == $cnt} { continue }
		set cbr($id,$cnt)  [new Module/UW/CBR] 
	}
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/TDMA]
    set phy($id)  [new Module/UW/PHYSICAL]  
	
    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if { $id == $cnt} { continue }
        $node($id) addModule 7 $cbr($id,$cnt)   1  "CBR"
    }

    $node($id) addModule 6 $udp($id)   1  "UDP"
    $node($id) addModule 5 $ipr($id)   1  "IPR"
    $node($id) addModule 4 $ipif($id)  1  "IPF"   
    $node($id) addModule 3 $mll($id)   1  "MLL"
    $node($id) addModule 2 $mac($id)   1  "TDMA"
    $node($id) addModule 1 $phy($id)   1  "PHY"

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        if { $id == $cnt} { continue }
        $node($id) setConnection $cbr($id,$cnt)   $udp($id)   0
        set portnum($id,$cnt) [$udp($id) assignPort $cbr($id,$cnt) ]
    }
    $node($id) setConnection $udp($id)   $ipr($id)   1
    $node($id) setConnection $ipr($id)   $ipif($id)  1
    $node($id) setConnection $ipif($id)  $mll($id)   1
    $node($id) setConnection $mll($id)   $mac($id)   1
    $node($id) setConnection $mac($id)   $phy($id)   1
    $node($id) addToChannel  $channel    $phy($id)   1


    #Set the IP address of the node
    $ipif($id) addr [expr $id + 1]
    
    # Set the MAC address
    $mac($id) setMacAddr [expr $id + 5]

    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    
    #Setup positions
    $position($id) setX_ $positions($id,x);
    $position($id) setY_ $positions($id,y);
    $position($id) setZ_ $positions($id,z);

    #Interference model
    set interf_data($id)  [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    #Propagation model
    $phy($id) setPropagation $propagation
    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
    $phy($id) setInterferenceModel "MEANPOWER"
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
    puts "Node $id created"
}


################################
# Inter-node module connection #
################################
proc connectNodes {id1 des1} {
    global ipif ipr portnum cbr opt 
    $cbr($id1,$des1) set destAddr_ [$ipif($des1) addr]
    $cbr($id1,$des1) set destPort_ $portnum($des1,$id1)
}

##################
# Setup flows    #
##################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
			connectNodes $id1 $id2
        }
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


########################
# Setup routing tables #
########################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
        if {$id1 != $id2} {
            $ipr($id1) addRoute [$ipif($id2) addr] [$ipif($id2) addr]
        }
    }
}


#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
# Set here the timers to start and/or stop modules (optional)
# e.g., 
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
		if {$id1 != $id2} {
		    $ns at $opt(starttime)    "$cbr($id1,$id2) start"
		    $ns at $opt(stoptime)     "$cbr($id1,$id2) stop"
		}
    }
}

for {set ii 0} {$ii < $opt(nn)} {incr ii} {
    $ns at $opt(starttime)    "$mac($ii) start"
    $ns at $opt(stoptime)     "$mac($ii) stop"
}


###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt output_config_filename
    global cbr udp ipr ipf mac phy channel propagation

	# Create dictionary containing simulation modules
	set input_modules [dict create]
	write-input-node-based-modules input_modules phy mac
	write-input-link-based-modules input_modules cbr

	# Print metrics to csv according to yaml output config
	print-metrics-csv $opt(rngstream) $input_modules $output_config_filename
    for {set i 0} {$i < $opt(nn)} {incr i}  {

    	set mac_sent_pkts        [$mac($i) get_sent_pkts]
    	set mac_recv_pkts        [$mac($i) get_recv_pkts]

    	for {set j 0} {$j < $opt(nn)} {incr j} {
    	    if {$i != $j} {
                set sent_pkts        [$cbr($i,$j) getsentpkts]
                set recv_pkts        [$cbr($i,$j) getrecvpkts]
				puts "sent: $sent_pkts"
				puts "recv: $recv_pkts"
		}
		}
	}
    
    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################
puts "\nStarting Simulation\n"
$ns at [expr $opt(stoptime) + 50.0]  "finish; $ns halt" 
$ns run
