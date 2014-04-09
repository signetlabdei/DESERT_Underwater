#
# Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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


set opt(n_nodes) 5
if {$opt(n_nodes) <= 0} {
  puts "Number of nodes equal to zero! Please put a number of nodes >= 2"
  exit(1)
}

#############################
#        Libraries          #
#############################
load libMiracle.so
load libmphy.so
load libuwip.so
load libuwmll.so
load libuwstaticrouting.so
load libuwudp.so
load libuwcbr.so
load libuwvbr.so
load libuwcbrtracer.so
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so
load libpackeruwcbr.so
load libuwmphy_modem.so
load libmstwoc_evologics.so
load libuwufetch.so
load libpackeruwufetch.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

#Declare the use of a Real Time Schedule (necessary for the interfacing with real hardware)
$ns use-scheduler RealTime

##################
# Tcl variables  #
##################
# Input variables (coming from framework)
# Terminal's parameter check
if {$argc != 10} {
    puts "The script needs 7 input to work"
    puts "1 - ID of the experiment "
    puts "2 - ID of the node "
    puts "3 - Traffic generation period"
    puts "4 - Start traffic"
    puts "5 - Stop traffic"
    puts "6 - ID of the sink"
    puts "7 - ID of HN 1"
    puts "8 - ID of HN 2"
    puts "9 - IP of EvoLogics modem"
    puts "10 - Port of EvoLogics modem"
    puts "Please try again."
    exit(1)
} else {
    set opt(n_run)              [lindex $argv 0]
    set opt(id_node)            [lindex $argv 1]
    set opt(generation_period)  [lindex $argv 2]
    set opt(start_traffic)      [lindex $argv 3]
    set opt(stop_traffic)       [lindex $argv 4]
    set opt(sink)    		    [lindex $argv 5]
    set opt(HN_1)    		    [lindex $argv 6]
    set opt(HN_2)    		    [lindex $argv 7]
    set opt(ip)                 [lindex $argv 8]
    set opt(port)               [lindex $argv 9]
}

set tf_name "S2C_EvoLogics_ufetch.tr"
set tf [open $tf_name w]
$ns trace-all $tf

set socket_port "${opt(ip)}:${opt(port)}"

set rng [new RNG]
$rng seed $opt(id_node)

set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng


# variables for the CBR module
Module/UW/CBR set packetSize_          30
Module/UW/CBR set period_              $opt(generation_period)
Module/UW/CBR set PoissonTraffic_      0
Module/UW/CBR set debug_               0

if {$opt(id_node) != $opt(HN_1) && $opt(id_node) != $opt(HN_2) && $opt(id_node) != $opt(sink)} {
    #SENSOR NODE
    Module/UW/UFETCH/NODE set  TIME_BEFORE_START_COMU_HN_NODE_       60
    Module/UW/UFETCH/NODE set  MAXIMUM_VALUE_BACKOFF_PROBE_          6.0
    Module/UW/UFETCH/NODE set  MINIMUM_VALUE_BACKOFF_PROBE_          1.0
    Module/UW/UFETCH/NODE set  MAXIMUM_NODE_POLLED_                  5           
    Module/UW/UFETCH/NODE set  MAXIMUM_PAYLOAD_SIZE_                 30
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_PROBES_PCK_              15.0        
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_POLL_PCK_                120        
    Module/UW/UFETCH/NODE set  SEE_THE_TRANSITIONS_STATE_            1
    Module/UW/UFETCH/NODE set  GUARD_INTERVAL_                       6.0
    Module/UW/UFETCH/NODE set  MAXIMUM_BUFFER_SIZE_                  100     
    Module/UW/UFETCH/NODE set  MAXIMUM_CBEACON_TRANSMISSIONS_        2 
    Module/UW/UFETCH/NODE set  MAXIMUM_PCK_WANT_RX_HN_FROM_NODE_     10          
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_CTS_                     20.0        
    Module/UW/UFETCH/NODE set  NUMBER_OF_RUN_                        $opt(n_run)
    Module/UW/UFETCH/NODE set  TIME_BETWEEN_2_DATA_TX_HN_            6
    Module/UW/UFETCH/NODE set  TIME_BETWEEN_2_DATA_TX_NODE_          6
    Module/UW/UFETCH/NODE set  debug_                                1
    Module/UW/UFETCH/NODE set  MY_DEBUG_                             1
} elseif {$opt(id_node) == $opt(HN_1) || $opt(id_node) == $opt(HN_2)} {  
    #HEAD NODE
    Module/UW/UFETCH/NODE set  TIME_BEFORE_START_COMU_HN_NODE_       60
    Module/UW/UFETCH/NODE set  MAXIMUM_VALUE_BACKOFF_PROBE_          6.0
    Module/UW/UFETCH/NODE set  MINIMUM_VALUE_BACKOFF_PROBE_          1.0
    Module/UW/UFETCH/NODE set  MAXIMUM_NODE_POLLED_                  5           
    Module/UW/UFETCH/NODE set  MAXIMUM_PAYLOAD_SIZE_                 30
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_PROBES_PCK_              15.0        
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_POLL_PCK_                120        
    Module/UW/UFETCH/NODE set  SEE_THE_TRANSITIONS_STATE_            1
    Module/UW/UFETCH/NODE set  GUARD_INTERVAL_                       6.0
    Module/UW/UFETCH/NODE set  MAXIMUM_BUFFER_SIZE_                  100     
    Module/UW/UFETCH/NODE set  MAXIMUM_CBEACON_TRANSMISSIONS_        2
    Module/UW/UFETCH/NODE set  MAXIMUM_PCK_WANT_RX_HN_FROM_NODE_     10          
    Module/UW/UFETCH/NODE set  TIME_TO_WAIT_CTS_                     20.0        
    Module/UW/UFETCH/NODE set  NUMBER_OF_RUN_                        $opt(n_run)
    Module/UW/UFETCH/NODE set  TIME_BETWEEN_2_DATA_TX_HN_            6
    Module/UW/UFETCH/NODE set  TIME_BETWEEN_2_DATA_TX_NODE_          6
    Module/UW/UFETCH/NODE set  debug_                                1
    Module/UW/UFETCH/NODE set  MY_DEBUG_                             1
} elseif {$opt(id_node) == $opt(sink)} {
    #AUV NODE
    Module/UW/UFETCH/AUV    set T_min_RTS_                           1.0 
    Module/UW/UFETCH/AUV    set T_max_RTS_                           5.0
    Module/UW/UFETCH/AUV    set T_guard_                             10.0
    Module/UW/UFETCH/AUV    set t_RTS_                               15.0            
    Module/UW/UFETCH/AUV    set MAX_PAYLOAD                          30
    Module/UW/UFETCH/AUV    set num_max_DATA_AUV_want_receive_       50.0
    Module/UW/UFETCH/AUV    set TIME_BEFORE_TX_TRIGGER_PCK_          15
    Module/UW/UFETCH/AUV    set NUMBER_OF_RUN_                       $opt(n_run)
    Module/UW/UFETCH/AUV    set HEAD_NODE_1_                         2
    Module/UW/UFETCH/AUV    set HEAD_NODE_2_                         3
    Module/UW/UFETCH/AUV    set debug_                               1
    Module/UW/UFETCH/AUV    set MY_DEBUG_                            1

}
###########################
#     	 PACKERS	  #
###########################		
# variables for the AL module
Module/UW/AL set Dbit                   0
Module/UW/AL set PSDU                   64
Module/UW/AL set debug_                 0
Module/UW/AL set interframe_period      1
Module/UW/AL set frame_set_validity     600

# variables for the packer(s)
UW/AL/Packer set SRC_ID_Bits            8
UW/AL/Packer set PKT_ID_Bits            8
UW/AL/Packer set FRAME_OFFSET_Bits      9
UW/AL/Packer set M_BIT_Bits             1
UW/AL/Packer set DUMMY_CONTENT_Bits     0
UW/AL/Packer set debug_                 0

NS2/COMMON/Packer set PTYPE_Bits            8
NS2/COMMON/Packer set SIZE_Bits             8
NS2/COMMON/Packer set UID_Bits              16
NS2/COMMON/Packer set ERROR_Bits            0
NS2/COMMON/Packer set TIMESTAMP_Bits        8
NS2/COMMON/Packer set PREV_HOP_Bits         8
NS2/COMMON/Packer set NEXT_HOP_Bits         8
NS2/COMMON/Packer set ADRR_TYPE_Bits        0
NS2/COMMON/Packer set LAST_HOP_Bits         0
NS2/COMMON/Packer set TXTIME_Bits           0
NS2/COMMON/Packer set debug_                0

UW/IP/Packer set SAddr_Bits         8
UW/IP/Packer set DAddr_Bits         8
UW/IP/Packer set debug_             0

NS2/MAC/Packer set Ftype_Bits       0
NS2/MAC/Packer set SRC_Bits         8
NS2/MAC/Packer set DST_Bits         8
NS2/MAC/Packer set Htype_Bits       0
NS2/MAC/Packer set TXtime_Bits      0
NS2/MAC/Packer set SStime_Bits      0
NS2/MAC/Packer set Padding_Bits     0
NS2/MAC/Packer set debug_           0

NS2/MAC/uwUFetch/Packer set T_BCK_MAX_PROBE_                16
NS2/MAC/uwUFetch/Packer set T_BCK_MIN_PROBE_                16
NS2/MAC/uwUFetch/Packer set N_CBEACON_TX_                   16
NS2/MAC/uwUFetch/Packer set T_BCK_CHOICE_SENSOR_            16
NS2/MAC/uwUFetch/Packer set N_PCK_SENSOR_WANT_TX_           8
NS2/MAC/uwUFetch/Packer set MAC_ADDR_SENSOR_POLLED_         8
NS2/MAC/uwUFetch/Packer set N_PCK_HN_WANT_RX_               8
NS2/MAC/uwUFetch/Packer set MAX_CBEACON_TX_HN_              8
NS2/MAC/uwUFetch/Packer set T_BCK_MAX_RTS_                  16
NS2/MAC/uwUFetch/Packer set T_BCK_MIN_RTS_                  16
NS2/MAC/uwUFetch/Packer set N_PCK_AUV_WANT_RX_              16
NS2/MAC/uwUFetch/Packer set NUM_DATA_PCKS_					16
NS2/MAC/uwUFetch/Packer set BACKOFF_TIME_RTS_				16
NS2/MAC/uwUFetch/Packer set NUM_DATA_PCKS_MAX_RX_			16
NS2/MAC/uwUFetch/Packer set MAC_ADRR_HN_CTSED_				16
NS2/MAC/uwUFetch/Packer set debug_                          0

UW/UDP/Packer set SPort_Bits    8
UW/UDP/Packer set DPort_Bits    8
UW/UDP/Packer set debug_        0

UW/CBR/Packer set SN_bits               16
UW/CBR/Packer set RFFT_bits             0
UW/CBR/Packer set RFFT_VALID_bits       0
UW/CBR/Packer set debug_                0

# variables for the S2C modem's interface
# check
Module/UW/MPhy_modem/S2C set period_        1
Module/UW/MPhy_modem/S2C set debug_         1
Module/UW/MPhy_modem/S2C set log_           1
Module/UW/MPhy_modem/S2C set SetModemID_    0


proc createNode {} {
    global ns opt socket_port node_ app_ transport_ port_ routing_ ipif_ mac_ modem_ ipif_ mll_
    global uwal_ params opt app_sink
    # build the NS-Miracle node
    set node_ [$ns create-M_Node]

    if {$opt(id_node) != $opt(sink)} {
        set app_ [new Module/UW/CBR]
    } else {
        for {set cnt 0} {$cnt < $opt(n_nodes)} {incr cnt} {
            set app_sink($cnt) [new Module/UW/CBR]
        }
    }
    set transport_ 	[new Module/UW/UDP]
    set routing_    	[new Module/UW/StaticRouting]
    set ipif_       	[new Module/UW/IP]
    set mll_        	[new Module/UW/MLL]

    if {$opt(id_node) == $opt(sink)} {
        set mac_ [new Module/UW/UFETCH/AUV]
    } else {
        set mac_ [new Module/UW/UFETCH/NODE]
    }
    set uwal_       	[new Module/UW/AL]
    set modem_ [new "Module/UW/MPhy_modem/S2C" $socket_port]

     if {$opt(id_node) != $opt(sink)} {
		# Structure of SENSOR NODE or HEAD NODE

        $node_ addModule 8 $app_				1 "CBR"
        $node_ addModule 7 $transport_		1 "UDP"
        $node_ addModule 6 $routing_		1 "IPR"
        $node_ addModule 5 $ipif_			1 "IPIF"
        $node_ addModule 4 $mll_				1 "ARP"  
        $node_ addModule 3 $mac_ 			1 "U"
        $node_ addModule 2 $uwal_ 			1 "UWAL"
        $node_ addModule 1 $modem_ 			1 "S2C" 

        $node_ 	setConnection 	$app_ 			$transport_	trace
        $node_ 	setConnection	$transport_ 	$routing_ 	trace
        $node_ 	setConnection 	$routing_ 		$ipif_ 		trace
        $node_ 	setConnection 	$ipif_ 			$mll_ 		trace
        $node_ 	setConnection 	$mll_				$mac_ 		trace
        $node_ 	setConnection 	$mac_ 			$uwal_ 		trace
        $node_ 	setConnection 	$uwal_ 			$modem_ 		trace
    } else {
		#Structure of the SINK node	

        for {set cnt 0} {$cnt < $opt(n_nodes)} {incr cnt} {
            $node_ addModule 8 $app_sink($cnt) 1 "CBR"
        }
        $node_ addModule 	7 	$transport_ 1 	"UDP"
        $node_ addModule 	6	$routing_ 	1 	"IPR"
        $node_ addModule 	5 	$ipif_ 		1 	"IPIF"
        $node_ addModule 	4 	$mll_ 		1 	"U"  
        $node_ addModule 	3 	$mac_ 		1 	"ALOHA"
        $node_ addModule 	2 	$uwal_ 		1 	"UWAL"
        $node_ addModule 	1 	$modem_ 		1 	"S2C" 

        for {set cnt 0} {$cnt < $opt(n_nodes)} {incr cnt} {
            $node_ setConnection $app_sink($cnt) $transport_
        }
        $node_ 	setConnection 	$transport_ 	$routing_	trace
        $node_ 	setConnection	$routing_ 		$ipif_		trace
        $node_ 	setConnection 	$ipif_ 			$mll_			trace
        $node_ 	setConnection 	$mll_ 			$mac_			trace
        $node_ 	setConnection 	$mac_ 			$uwal_		trace
        $node_ 	setConnection 	$uwal_ 			$modem_		trace
    }

    if {$opt(id_node) != $opt(sink)} {
        set port_ 	[$transport_ assignPort $app_]
    } else {
        for {set cnt 0} {$cnt < $opt(n_nodes)} {incr cnt} {
            set port_($cnt) [$transport_ assignPort $app_sink($cnt)]
        }
    }

    $ipif_  	addr        $opt(id_node)
    $mac_   	setMacAddr  $opt(id_node)
    $modem_ 	set ID_     $opt(id_node)


    set packer_ [new UW/AL/Packer]

    set packer_payload0 [new NS2/COMMON/Packer]   
    set packer_payload1 [new UW/IP/Packer]
    set packer_payload2 [new NS2/MAC/Packer]
    set packer_payload3 [new NS2/MAC/uwUFetch/Packer]
    set packer_payload4 [new UW/UDP/Packer]
    set packer_payload5 [new UW/CBR/Packer]

    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4
    $packer_ addPacker $packer_payload5

    $uwal_ linkPacker $packer_
    
    $uwal_ set nodeID $opt(id_node)

    if {$opt(id_node) == $opt(HN_1) || $opt(id_node) == $opt(HN_2)} {
        $mac_ BeHeadNode
    } 

    if {$opt(id_node) != $opt(sink)} {
        $mac_ set node_id_ $opt(id_node)
    }

    $mac_ initialize
}	;#end createNode procedure

##########################################
# 				Node Creation 					  #
##########################################
createNode

##########################################
# 				CBR connection 				  #
##########################################
if {$opt(id_node) != $opt(sink)} {
    $app_ set destAddr_ $opt(sink)
    $app_ set destPort_ 1
} else {
    for {set cnt 0} {$cnt < $opt(n_nodes)} {incr cnt} {
        if {$cnt != $opt(sink)} {
            $app_sink($cnt) set destAddr_ $cnt
            $app_sink($cnt) set destPort_ 1 
        }
    }
}

##############################
# Routing and MLL connection #
##############################
for {set cnt 0} {$cnt < $opt(n_nodes)} {incr cnt} {
    $routing_ 	addRoute		[expr $cnt + 1] [expr $cnt + 1]
    $mll_ 		addentry		[expr $cnt + 1] [expr $cnt + 1]
}

#######################################
#       start CBR/HN/SENSOR/AUV       #
#######################################
if {$opt(id_node) != $opt(sink)} {
    $ns at 	$opt(start_traffic) 	"$app_ start"
    $ns at 	$opt(stop_traffic) 	"$app_ stop"
    if {$opt(id_node) == $opt(HN_1) || $opt(id_node) == $opt(HN_2)} {
        $ns at 0 "$mac_ HeadNodeStart"
    } else {
        $ns at 0 "$mac_ SimpleNodeStart"
    }
} else {
    $ns at 0 "$mac_ AUVNodeStart"
}

proc finish { } {
    puts "UWUFETCH TRIAL DONE!"
}
##############################
#       start the modem      #
##############################
$ns at 0 "$modem_ start"
$ns at [expr $opt(stop_traffic) + 20] 	"$modem_ stop"
$ns at [expr $opt(stop_traffic) + 20] 	"finish; $ns halt"

close $tf
$ns run
