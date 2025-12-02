#
# Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
# All rights reserved.
#
# Modified to use uwApplication with Applicon Modem and TDMA
#

######################
# Simulation Options #
######################
set opt(bash_parameters) 1

# if set to 1 the Application listen from the socket port provided in input
set opt(AppSocket)  0r;
# Protocol to use for the Application socket, TCP or UDP
set opt(protocol) "TCP" ;

set opt(n_node)   3
set opt(node)     1
set opt(dest)     2
set opt(start)    1
set opt(stop)     100
set opt(traffic)  60
set opt(app_port) 22223
set opt(ip)       "10.42.144.1"
set opt(port)     9200
set opt(payload_size) 16
set opt(rngstream)    1

# Terminal's parameter check
if {$opt(bash_parameters)} {
  if {$opt(AppSocket) == 1} {
    if {$argc != 9} {
      puts "The script needs 9 inputs to work in Socket Mode"
      puts "1 - ID of the node"
      puts "2 - ID of the destination"
      puts "3 - Start time"
      puts "4 - Stop time"
      puts "5 - Packet generation period (0 if the node doesn't generate data)"
      puts "6 - IP of the modem"
      puts "7 - Port of the modem"
      puts "8 - Application socket port"
      puts "9 - Random generator stream"
      puts "Please try again."
      exit(1)
    } else {
      set opt(node)       [lindex $argv 0]
      set opt(dest)       [lindex $argv 1]
      set opt(start)      [lindex $argv 2]
      set opt(stop)       [lindex $argv 3]
      set opt(traffic)    [lindex $argv 4]
      set opt(ip)         [lindex $argv 5]
      set opt(port)       [lindex $argv 6]
      set opt(app_port)   [lindex $argv 7]
      set opt(rngstream)  [lindex $argv 8]
    }
  } else {
    if {$argc != 9} {
      puts "The script needs 9 inputs to work in Payload Mode"
      puts "1 - ID of the node"
      puts "2 - ID of the destination"
      puts "3 - Start time"
      puts "4 - Stop time"
      puts "5 - Packet generation period (0 if the node doesn't generate data)"
      puts "6 - IP of the modem"
      puts "7 - Port of the modem"
      puts "8 - Payload size (byte)"
      puts "9 - Random generator stream"
      puts "Please try again."
      exit(1)
    } else {
      set opt(node)          [lindex $argv 0]
      set opt(dest)          [lindex $argv 1]
      set opt(start)         [lindex $argv 2]
      set opt(stop)          [lindex $argv 3]
      set opt(traffic)       [lindex $argv 4]
      set opt(ip)            [lindex $argv 5]
      set opt(port)          [lindex $argv 6]
      set opt(payload_size)  [lindex $argv 7]
      set opt(rngstream)     [lindex $argv 8]
      puts "PARAM OK."
    }
  }
}

if {$opt(n_node) <= 0} {
  puts "Number of nodes equal to zero! Please put a number of nodes >= 2"
  exit(1)
}

#####################
# Library Loading   #
#####################
# Load here all the NS-Miracle libraries you need
load libMiracle.so
load libmphy.so
load libmmac.so
load libuwip.so
load libuwmll.so
load libuwstaticrouting.so
load libuwudp.so
load libuwapplication.so
load libpackeruwapplication.so
load libuwvbr.so
load libuwcbr.so
load libpackeruwcbr.so
load libuwal.so
load libuwphy_clmsgs.so
load libuwconnector.so
load libuwmodem.so
load libuwapplicon_driver.so
load libuwmmac_clmsgs.so
load libuwtdma.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so

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
# #Socket Ports to be used
set socket_port "${opt(ip)}:${opt(port)}"

set adrMAC $opt(node)

# time when actually to stop the simulation
set time_stop [expr "$opt(stop)+15"]

#Trace file name
set tf_name "Applicon_Uwtdma_app.tr"

#Open a file for writing the trace data
set tf [open $tf_name w]
$ns trace-all $tf

#random generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}

#########################
# Module Configuration  #
#########################

# Variables for the Application module
Module/UW/APPLICATION set period_ $opt(traffic)
if {$opt(AppSocket) == 1} {
  Module/UW/APPLICATION set Socket_Port_ $opt(app_port)
} else {
  Module/UW/APPLICATION set Payload_size_ $opt(payload_size)
}
Module/UW/APPLICATION set PoissonTraffic_ 0
Module/UW/APPLICATION set drop_out_of_order_ 0
Module/UW/APPLICATION set sea_trial_ 1
Module/UW/APPLICATION set debug_ 0

# Variables for TDMA
Module/UW/TDMA set frame_duration   15
Module/UW/TDMA set debug_           1
Module/UW/TDMA set sea_trial_       1
Module/UW/TDMA set fair_mode        1 
Module/UW/TDMA set guard_time       0.3
Module/UW/TDMA set tot_slots        2


# variables for the AL module
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 1400
Module/UW/AL set debug_ 0
Module/UW/AL set interframe_period 0.e1
Module/UW/AL set frame_set_validity 0

# variables for the packer(s)
UW/AL/Packer set SRC_ID_Bits 8
UW/AL/Packer set PKT_ID_Bits 8
UW/AL/Packer set FRAME_OFFSET_Bits 15
UW/AL/Packer set M_BIT_Bits 1
UW/AL/Packer set DUMMY_CONTENT_Bits 0
UW/AL/Packer set debug_ 0

NS2/COMMON/Packer set PTYPE_Bits 8
NS2/COMMON/Packer set SIZE_Bits 8
NS2/COMMON/Packer set UID_Bits 8
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 8
NS2/COMMON/Packer set PREV_HOP_Bits 8
NS2/COMMON/Packer set NEXT_HOP_Bits 38
NS2/COMMON/Packer set ADRR_TYPE_Bits 0
NS2/COMMON/Packer set LAST_HOP_Bits 0
NS2/COMMON/Packer set TXTIME_Bits 0
NS2/COMMON/Packer set debug_ 0

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

UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 0

# UW/CBR/Packer set SN_bits 32
# UW/CBR/Packer set RFFT_bits 0
# UW/CBR/Packer set RFFT_VALID_bits 0
# UW/CBR/Packer set debug_ 0

UW/APP/uwApplication/Packer set SN_FIELD_ 8
UW/APP/uwApplication/Packer set RFFT_FIELD_ 5
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 2
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 8
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 8
UW/APP/uwApplication/Packer set debug_ 0


# variables for the AppliconSeaModem's interface
#####
Module/UwModem/Applicon set debug_		 1
Module/UwModem/Applicon set buffer_size   4096
Module/UwModem/Applicon set max_read_size 4096
#######

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { } {
    global ns opt socket_port node_ app_ transport_ port_ routing_ ipif_ mac_ modem_ ipif_ mll_
    global uwal_ 
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
    set mac_ [new Module/UW/TDMA]
    $mac_ setMacAddr     $opt(node)
    $mac_ setSlotNumber [expr $opt(node) -1]
    
    set uwal_             [new Module/UW/AL]

    # PHY LAYER
    set modem_ [new Module/UwModem/Applicon]    
    puts "Creating node..."
    
    # insert the module(s) into the node
   
    $node_ addModule 8 $app_ 1 "UWA"
    $node_ addModule 7 $transport_ 1 "UDP"
    $node_ addModule 6 $routing_ 1 "IPR"
    $node_ addModule 5 $ipif_ 1 "IPIF"
    $node_ addModule 4 $mll_ 1 "ARP"  
    $node_ addModule 3 $mac_ 1 "TDMA"
    $node_ addModule 2 $uwal_ 1 "UWAL"
    $node_ addModule 1 $modem_ 1 "SEAM" 

    $node_ setConnection $app_ $transport_ trace
  	$node_ setConnection $transport_ $routing_ trace
  	$node_ setConnection $routing_ $ipif_ trace
  	$node_ setConnection $ipif_ $mll_ trace
  	$node_ setConnection $mll_ $mac_ trace
  	$node_ setConnection $mac_ $uwal_ trace
  	$node_ setConnection $uwal_ $modem_ trace
    
    # Enable log for uwapplication module
    $app_ setLog 1 "uwapplication_$opt(node)_log"
    
    if {$opt(AppSocket) == 1} {
        $app_ setSocketProtocol $opt(protocol)
        $app_ set Socket_Port_ $opt(app_port)
    }
    $app_ set node_ID_  $opt(node)

    # assign a port number to the application considered (CBR or VBR)
    set port_ [$transport_ assignPort $app_]
    $ipif_ addr $opt(node)
    $mac_ setMacAddr $opt(node)
    $modem_ set ID_ $opt(node)
    $modem_ setModemAddress $socket_port
    $modem_ setLogLevel DBG

    # set packer for Adaptation Layer
    set packer_ [new UW/AL/Packer]

    set packer_payload0 [new NS2/COMMON/Packer]  
    set packer_payload1 [new UW/IP/Packer]
    set packer_payload2 [new NS2/MAC/Packer]
    set packer_payload3 [new UW/UDP/Packer]
    set packer_payload4 [new UW/APP/uwApplication/Packer]

    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4

    $uwal_ linkPacker $packer_
    
    $uwal_ set nodeID $opt(node)
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
createNode

################################
# Inter-node module connection #
################################
# Connections at the application level
$app_ set destAddr_ [expr $opt(dest)]
$app_ set destPort_ 1

for {set cnt 0} {$cnt < $opt(n_node)} {incr cnt} {
  $routing_ addRoute [expr $cnt + 1] [expr $cnt + 1]
  $mll_ addentry  [expr $cnt + 1] [expr $cnt + 1]
}


#####################
# Start/Stop Timers #
#####################
$ns at 0 "$modem_ start"
      
if {$opt(traffic) != 0} {
  $ns at $opt(start) "$app_ start"
  $ns at $opt(stop) "$app_ stop"
}
$ns at $opt(start) "$mac_ start"
$ns at $opt(stop) "$mac_ stop"

$ns at $time_stop "$modem_ stop"

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
   
   global ns tf tf_name	app_ mac_
   
   # display messages
   puts "done!"
   puts "tracefile: $tf_name"

   # Note: Standard CBR statistics (PER, THR) from the original file 
   # are not directly available in uwApplication in the same way.
   # You should check the generated log file uwapplication_ID_log.
   
   puts "RX DATA packets             : [$mac_ getDataPktsRx]"
   puts "TX DATA packets             : [$mac_ getDataPktsTx]"

   
   # save traces
   $ns flush-trace
   
   # close files
   close $tf
}

##################
# Run simulation #
##################
# Specify the time at which to call the finish procedure and halt ns

$ns at $time_stop "finish; $ns halt"

# You always need the following line to run the NS-Miracle simulator
$ns run
