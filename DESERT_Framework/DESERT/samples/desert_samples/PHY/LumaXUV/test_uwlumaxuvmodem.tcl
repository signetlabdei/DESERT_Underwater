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
# Author: Pietro Trabuio
# Version: 1.0.0
#

# Multicast communications with LumaXUV optical modems

# if set to 1 the Application listen from the socket port provided in input
set opt(AppSocket)  1;

##############################
# Terminal's parameter check #
##############################
if {$opt(AppSocket) == 1} {
    if {$argc != 8} {
        puts "The script needs 8 input arguments to work"
        # internal to DESERT
        puts "1 - ID of the node"                       ; # simulation ID
        puts "2 - Start time"
        puts "3 - Stop time"                            ; # stop time set very high in real-time mode
        puts "4 - Packet generation period (0 if the node doesn't generate data)"       ; # time between packet transmissions/receptions, e.g. 0.1
        puts "5 - IP of the interface on the network of the modems"
        puts "6 - Port for data transmission"           ; # data transmission port
        puts "7 - IP of the modem for configuration"
        puts "8 - Application socket port"              ; # application level port
        puts "Please try again."
        puts "e.g."
        puts "ns test_uwlumaxuvmodem.tcl 1 10 100000 0.1 192.168.102.50 55555 192.168.102.101 44444"
        exit
    } else {
        set opt(node)     [lindex $argv 0]
        set opt(start)    [lindex $argv 1]
        set opt(stop)     [lindex $argv 2]
        set opt(traffic)  [lindex $argv 3]
        set opt(address)  [lindex $argv 4]
        set opt(port)     [lindex $argv 5]
        set opt(modem_address)  [lindex $argv 6]
        set opt(app_port) [lindex $argv 7]
    }
}

# UW/IP addresses are one-byte addresses in DESERT and address 0 is invalid.
if {![string is integer -strict $opt(node)] || $opt(node) < 1 || $opt(node) > 255} {
    puts "Invalid node ID '$opt(node)': expected an integer in the range 1..255."
    exit 1
}
if {![string is double -strict $opt(start)] || $opt(start) < 0} {
    puts "Invalid start time '$opt(start)': expected a non-negative number."
    exit 1
}
if {![string is double -strict $opt(stop)] || $opt(stop) <= $opt(start)} {
    puts "Invalid stop time '$opt(stop)': expected a number greater than start time $opt(start)."
    exit 1
}
if {![string is double -strict $opt(traffic)] || $opt(traffic) < 0} {
    puts "Invalid packet generation period '$opt(traffic)': expected a non-negative number."
    exit 1
}

#####################
# Library Loading   #
#####################
# Load here all the NS-Miracle libraries you need
load libMiracle.so
load libmphy.so
load libuwip.so
load libuwmll.so
load libuwstaticrouting.so
load libuwudp.so
load libuwcbr.so
load libuwapplication.so
load libpackeruwapplication.so
load libuwaloha.so
load libuwcsmaaloha.so
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so
load libuwphy_clmsgs.so
load libuwconnector.so
load libuwmodem.so
load libuwlumaxuvmodem.so
load libuwmmac_clmsgs.so

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

#Declare the use of a Real Time Schedule (necessary for the interfacing with real hardware)
$ns use-scheduler RealTime                          ; # use the modem's scheduler

##################
# Tcl variables  #
##################
# address and port of the modem
set address "${opt(address)}:${opt(port)}"          ; # local interface address for data transmission
set opt(config_address) "${opt(modem_address)}"     ; # modem reception, transmission, etc. messages; in this case, configuration

# set MAC address for the modem
# set addrMAC $opt(node)                            ; # not necessary

# time when actually to stop the simulation
set time_stop [expr "$opt(stop)+5"]

#Trace file name
set tf_name "lumaxuv_test.tr"

#Open a file for writing the trace data
set tf [open $tf_name w]
$ns trace-all $tf

# random generator
global defaultRNG
for {set k 0} {$k < $opt(node)} {incr k} {
    $defaultRNG next-substream
}

set rng [new RNG]
set rnd_gen [new RandomVariable/Uniform]
$rnd_gen use-rng $rng

#########################
# Module Configuration  #
#########################
# Put here all the commands to set globally the initialization values of
# the binded variables (optional)

# variables for the AL module
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 64                            ; # physical service data unit, maximum packet length including everything
Module/UW/AL set debug_ 0
Module/UW/AL set interframe_period 0.1               ; # transmission time when fragmentation is enabled
Module/UW/AL set frame_set_validity 0               ; # 3

# variables for the packer(s)
# if there is no fragmentation (the entire packet fits in one frame), set everything in this section to zero
UW/AL/Packer set SRC_ID_Bits 8
UW/AL/Packer set PKT_ID_Bits 8
UW/AL/Packer set FRAME_OFFSET_Bits 15
UW/AL/Packer set M_BIT_Bits 1                       ; # packet fragmentation is enabled
UW/AL/Packer set DUMMY_CONTENT_Bits 0
UW/AL/Packer set debug_ 0

NS2/COMMON/Packer set PTYPE_Bits 8                  ; # type of packet being received or sent, always 8
NS2/COMMON/Packer set SIZE_Bits 8                   ; # typically payload size, 0
NS2/COMMON/Packer set UID_Bits 8                    ; # application level
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 8              ; # packet transmission timestamp
# the following 4 entries are only for multihop; otherwise set to 0
NS2/COMMON/Packer set PREV_HOP_Bits 8
NS2/COMMON/Packer set NEXT_HOP_Bits 8               ; # set to 0 if multihop is not needed
NS2/COMMON/Packer set ADDR_TYPE_Bits 0
NS2/COMMON/Packer set LAST_HOP_Bits 0
NS2/COMMON/Packer set TXTIME_Bits 0
NS2/COMMON/Packer set debug_ 0

# IP and MAC must have the same bit size
UW/IP/Packer set SAddr_Bits 8
UW/IP/Packer set DAddr_Bits 8
UW/IP/Packer set debug_ 0

NS2/MAC/Packer set Ftype_Bits 0
NS2/MAC/Packer set SRC_Bits 8
NS2/MAC/Packer set DST_Bits 8
NS2/MAC/Packer set Htype_Bits 0
NS2/MAC/Packer set TXtime_Bits 0
NS2/MAC/Packer set SStime_Bits 0
NS2/MAC/Packer set Padding_Bits 0
NS2/MAC/Packer set debug_ 0
# changed the sport and dport from 2 to 8, to accomodate port 255 for ns broadcast
UW/UDP/Packer set SPort_Bits 8                      ; # internal DESERT ports
UW/UDP/Packer set DPort_Bits 8
UW/UDP/Packer set debug_ 0

UW/APP/uwApplication/Packer set SN_FIELD_ 0                     ; # 0 sequence number; increase if more than 8 bits are needed
UW/APP/uwApplication/Packer set RFFT_FIELD_ 0                   ; # 0
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 0              ; # 0
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 0               ; # 0
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 64        ; # application-level payload length arriving from the socket; increase above 256 bytes for optical modems
UW/APP/uwApplication/Packer set debug_ 0

# In socket mode period_ is also the queue-polling interval.  A zero period
# would repeatedly reschedule the application at the same simulation time.
set app_period $opt(traffic)
if {$opt(AppSocket) == 1 && $app_period == 0} {
    set app_period 0.1
}
Module/UW/APPLICATION set period_ $app_period
if {$opt(AppSocket) == 1} {
    Module/UW/APPLICATION set Socket_Port_ $opt(app_port)
    Module/UW/APPLICATION set max_read_length 64
} else {
    Module/UW/APPLICATION set Payload_size_ $opt(payload_size)
}
Module/UW/APPLICATION set PoissonTraffic_ 0
Module/UW/APPLICATION set drop_out_of_order_ 0
Module/UW/APPLICATION set sea_trial_ 1                      ; # when active, prints real-time timestamps in the logs

# variables for the LumaXUV modem interface
Module/UW/UwModem/LumaXUV set debug_	 0
Module/UW/UwModem/LumaXUV set buffer_size   2048
Module/UW/UwModem/LumaXUV set max_read_size 2048

Module/UW/UDP set debug_ 0

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
    global ns opt socket_port node_ address
    global app_ transport_ port_ routing_ mac_ modem_ ipif_ mll_ uwal_ app_sink

    # build the NS-Miracle node
    set node_ [$ns create-M_Node]

    # define the module(s) you want to put in the node
    # APPLICATION LAYER
    set app_ [new Module/UW/APPLICATION]

    # TRANSPORT LAYER
    set transport_ [new Module/UW/UDP]

    # NETWORK LAYER
    # Static Routing
    set routing_ [new Module/UW/StaticRouting]

    # IP interface
    set ipif_ [new Module/UW/IP]

    # DATA LINK LAYER - MEDIA LINK LAYER
    set mll_ [new Module/UW/MLL]

    # DATA LINK LAYER - MAC LAYER
    set mac_ [new Module/UW/CSMA_ALOHA]

    # ADAPTATION LAYER
    set uwal_ [new Module/UW/AL]

    # PHY LAYER
    set modem_ [new Module/UW/UwModem/LumaXUV]

    puts "Creating node..."

    # insert the module(s) into the node
    $node_ addModule 8 $app_ 1 "UWA"
    $node_ addModule 7 $transport_ 1 "UDP"
    $node_ addModule 6 $routing_ 1 "IPR"
    $node_ addModule 5 $ipif_ 1 "IPIF"
    $node_ addModule 4 $mll_ 1 "ARP"
    $node_ addModule 3 $mac_ 1 "ALOHA"
    $node_ addModule 2 $uwal_ 1 "UWAL"
    $node_ addModule 1 $modem_ 1 "LumaXUV"

    $node_ setConnection $app_ $transport_ trace
    $node_ setConnection $transport_ $routing_ trace
    $node_ setConnection $routing_ $ipif_ trace
    $node_ setConnection $ipif_ $mll_ trace
    $node_ setConnection $mll_ $mac_ trace
    $node_ setConnection $mac_ $uwal_ trace
    $node_ setConnection $uwal_ $modem_ trace

    # Enable log for uwapplication module
    # $app_ setLog 3 "uwapplication_$opt(node)_log"
    $app_ setLogLevel 3                            ; # prints to console; only errors at level 1

    $routing_ enableLog

    # $mac_ enableLog                     ; # if the log level is already set, this is sufficient to enable all others
    if {$opt(AppSocket) == 1} {
        $app_ setSocketProtocol "TCP"              ; # application-level protocol
        $app_ set Socket_Port_ $opt(app_port)
    }

    $app_ set node_ID_  $opt(node)

    # assign a port number to the application considered (CBR or VBR)
    set port_ [$transport_ assignPort $app_]
    $app_ set destPort_ $port_

    $ipif_ addr $opt(node)
    $mac_ setMacAddr $opt(node)
    $modem_ set ID_ $opt(node)
    $modem_ setLocalAddress $address
    $modem_ setModemAddress $opt(config_address)
    $modem_ setLogLevel DBG                        ; #  ERR, INFO, DBG
    # LumaXUV modem settings. Available settings are the following (see LumaXUV manual for more informations about them):
    # start_state 0|1
    # optical_speed {1,4,6,8,10}
    # nb_led [1-5]
    # encoding 0|1
    # led_tx_pwr_normal [0-100]
    # led_tx_pwr_cooldown [0-100]
    # auto_gain_control 0|1
    # manual_gain [0-4000]
    # nb_receivers [1-4]
    # agc_amplitude_adjust [0-4000]
    # keep_alive_pkt 0|1
    # autogain_speed {0,1,2}
    # signal_filtering_type {0,1,2,3}
    # auto_power 0|1
    # auto_speed 0|1
    $modem_ start_state 1

    # set packer for Adaptation Layer
    set packer_ [new UW/AL/Packer]

    # create the packers
    set packer_payload0 [new NS2/COMMON/Packer]
    set packer_payload1 [new NS2/MAC/Packer]
    set packer_payload2 [new UW/IP/Packer]
    set packer_payload3 [new UW/UDP/Packer]
    set packer_payload4 [new UW/APP/uwApplication/Packer]

    # add the packers; order is important
    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4

    # assemble the packers together
    $uwal_ linkPacker $packer_

    $uwal_ set nodeID $opt(node)

    # useful with CSMA
    $mac_ setNoAckMode
    $mac_ initialize

}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
createNode

################################
# Inter-node module connection #
################################
# Put here all the commands required to connect nodes in the network (optional), namely, specify end to end connections, fill ARP tables, define routing settings

# connections at the application level
$app_ set destAddr_ 255                               ; # broadcast address (255)

$routing_ addRoute 255 255                            ; # Route destination 255 to next-hop 255, broadcast mode
$mll_ addentry 255 255                                ; # Map IP 255 to MAC 255

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g.,

$ns at 0 "$modem_ start"

if {$opt(AppSocket) == 1 || $opt(traffic) != 0} {
    $ns at $opt(start) "$app_ start"
    $ns at $opt(stop) "$app_ stop"
}

$ns at $time_stop "$modem_ stop"                    ; # command as written, or procedure name in quotes

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {

    global ns tf tf_name
    # computation of the statics

    # display messages
    puts "done!"
    puts "tracefile: $tf_name"

    # save traces
    $ns flush-trace

    # close files
    close $tf
}

##################
# Run simulation #
##################
# Specify the time at which to call the finish procedure and halt ns
$ns at [expr $time_stop] "finish; $ns halt"

# You always need the following line to run the NS-Miracle simulator
$ns run
