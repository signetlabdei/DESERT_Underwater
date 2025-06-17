# Author: Davide Costa
# Version: 1.0.0
# ----------------------------------------------------------------------------------
# This script depicts a complete stack in which a node sends data to another one  
# through a SuM modem. It does not make use of the signaling, so it can be used also
# in combination with the JANUS modulation.
# ----------------------------------------------------------------------------------
# Stack
#              Node
#   +--------------------------+
#   |  8. UW/APPLICATION       |
#   +--------------------------+
#   |  7. UW/UDP               |
#   +--------------------------+
#   |  6. UW/STATICROUTING     |
#   +--------------------------+
#   |  5. UW/IP                |
#   +--------------------------+
#   |  4. UW/MLL               |
#   +--------------------------+
#   |  3. UW/TDMA              |
#   +--------------------------+
#   |  2. UW/AL                |
#   +--------------------------+
#   |  1. UwModem/ModemCSA     |
#   +--------------------------+
#            |         | 
#   +--------------------------+
#   |   UnderwaterChannel      |
#   +--------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(bash_parameters)    0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
load libuwinterference.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwapplication.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libpackeruwapplication.so
load libuwaloha.so
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so
load libuwconnector.so
load libuwmodem.so
load libuwmodemcsa.so
load libuwmmac_clmsgs.so
load libuwcbr.so
load libuwtdma.so

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
set opt(starttime)          5

set opt(stoptime)           400

set opt(rngstream)          1
set opt(pktsize)            64
set opt(cbr_period)         0.5
set opt(socket_comm)        1

set opt(node)               1
set opt(dest)               2

set opt(address)            "192.168.100.32"
set opt(port)               55555

set opt(app_port)           4000

set opt(exp_ID)             1

if {$opt(bash_parameters)} {
    if {$argc != 8} {
        puts "The script needs 10 input to work"
        puts "1 - ID of the node"
        puts "2 - ID of the receiver"
        puts "3 - Start time"
        puts "4 - Stop time"
        puts "5 - Packet generation period (0 if the node doesn't generate data)"
        puts "6 - IP of the modem"
        puts "7 - Port of the modem"
        puts "8 - Application socket port"
        puts "Please try again."
        exit
    } else {
        set opt(node)         [lindex $argv 0]
        set opt(dest)         [lindex $argv 1]
        set opt(starttime)    [lindex $argv 2]
        set opt(stoptime)     [lindex $argv 3]
        set opt(cbr_period)   [lindex $argv 4]
        set opt(address)      [lindex $argv 5]
        set opt(port)         [lindex $argv 6]
        set opt(app_port)     [lindex $argv 7]
    }
} 

global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
    $defaultRNG next-substream
}

set opt(tracefilename) "/dev/null"
set opt(tracefile) [open $opt(tracefilename) w]
set opt(cltracefilename) "/dev/null"
set opt(cltracefile) [open $opt(cltracefilename) w]

##################
# Tcl variables  #
##################

# address and port of the SuM modem
set address "${opt(address)}:${opt(port)}"

#########################
# Module Configuration  #
#########################

# variables for the TDMA module
Module/UW/TDMA set frame_duration   1
Module/UW/TDMA set debug_           10
Module/UW/TDMA set sea_trial_       1
Module/UW/TDMA set fair_mode        1 
Module/UW/TDMA set guard_time       0.3
Module/UW/TDMA set tot_slots        2
Module/UW/TDMA set queue_size_      100

# variables for the AL module
Module/UW/AL set Dbit 1
Module/UW/AL set PSDU 64
Module/UW/AL set debug_ 1
Module/UW/AL set interframe_period  0.e1
Module/UW/AL set frame_set_validity 0

# variables for the packer(s)
UW/AL/Packer set SRC_ID_Bits 8
UW/AL/Packer set PKT_ID_Bits 8
UW/AL/Packer set FRAME_OFFSET_Bits 15
UW/AL/Packer set M_BIT_Bits 1
UW/AL/Packer set DUMMY_CONTENT_Bits 0
UW/AL/Packer set debug_ 1

NS2/COMMON/Packer set PTYPE_Bits 8
NS2/COMMON/Packer set SIZE_Bits 8
NS2/COMMON/Packer set UID_Bits 8
NS2/COMMON/Packer set ERROR_Bits 0
NS2/COMMON/Packer set TIMESTAMP_Bits 8
NS2/COMMON/Packer set PREV_HOP_Bits 8
NS2/COMMON/Packer set NEXT_HOP_Bits 8
NS2/COMMON/Packer set ADDR_TYPE_Bits 0
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
NS2/MAC/Packer set debug_ 1

UW/UDP/Packer set SPort_Bits 2
UW/UDP/Packer set DPort_Bits 2
UW/UDP/Packer set debug_ 0

UW/APP/uwApplication/Packer set SN_FIELD_ 6
UW/APP/uwApplication/Packer set RFFT_FIELD_ 5
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 2
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 2
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 8
UW/APP/uwApplication/Packer set debug_ 10

Module/UW/APPLICATION set debug_ -1
Module/UW/APPLICATION set period_ $opt(cbr_period)
Module/UW/APPLICATION set PoissonTraffic_ 0
Module/UW/APPLICATION set drop_out_of_order_ 1
Module/UW/APPLICATION set EXP_ID_ $opt(exp_ID)

# variables for the MODA modem interface
Module/UW/UwModem/ModemCSA set debug_	     1
Module/UW/UwModem/ModemCSA set period_	     0.01
Module/UW/UwModem/ModemCSA set buffer_size   2048
Module/UW/UwModem/ModemCSA set max_read_size 2048

################################
# Procedure(s) to create nodes #
################################
proc createNode { } {

    global ns app_  node_ udp_ portnum_ ipr_ ipif_ uwal_ address
    global phy_ opt  mll_ mac_
    
    set node_ [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

    set app_  [new Module/UW/APPLICATION] 
    set udp_  [new Module/UW/UDP]
    set ipr_  [new Module/UW/StaticRouting]
    set ipif_ [new Module/UW/IP]
    set mll_  [new Module/UW/MLL] 
    set mac_  [new Module/UW/TDMA] 
    set uwal_ [new Module/UW/AL]
    set phy_  [new Module/UW/UwModem/ModemCSA]

    $node_ addModule 8 $app_   1  "CBR"
    $node_ addModule 7 $udp_   1  "UDP"
    $node_ addModule 6 $ipr_   1  "IPR"
    $node_ addModule 5 $ipif_  1  "IPF"   
    $node_ addModule 4 $mll_   1  "MLL"
    $node_ addModule 3 $mac_   1  "MAC"
    $node_ addModule 2 $uwal_  1  "UWAL"
    $node_ addModule 1 $phy_   1  "PHY"

    $node_ setConnection $app_   $udp_   trace
    $node_ setConnection $udp_   $ipr_   trace
    $node_ setConnection $ipr_   $ipif_  trace
    $node_ setConnection $ipif_  $mll_   trace
    $node_ setConnection $mll_   $mac_   trace
    $node_ setConnection $mac_   $uwal_  trace
    $node_ setConnection $uwal_  $phy_   trace

    set portnum_ [$udp_ assignPort $app_ ]
    
    set general_address_ $opt(node)
    $ipif_ addr $general_address_
    $mac_ setMacAddr $general_address_
    $mac_ setSlotNumber $opt(node)
    $phy_ setTCP
    $phy_ set ID_ $general_address_
    $phy_ setModemAddress $address
    $phy_ setLogLevel DBG
    
    # set packer for Adaptation Layer
    set packer_ [new UW/AL/Packer]

    set packer_payload0 [new NS2/COMMON/Packer]
    set packer_payload1 [new UW/IP/Packer]
    set packer_payload2 [new NS2/MAC/Packer]
    set packer_payload3 [new UW/UDP/Packer]
    set packer_payload4 [new UW/APP/uwApplication/Packer]

    $packer_payload4 printMap

    $packer_ addPacker $packer_payload0
    $packer_ addPacker $packer_payload1
    $packer_ addPacker $packer_payload2
    $packer_ addPacker $packer_payload3
    $packer_ addPacker $packer_payload4

    $app_ set Socket_Port_  $opt(app_port)
    $app_ setSocketProtocol "TCP"
    $app_ set node_ID_  $general_address_

    $uwal_ linkPacker $packer_
    $uwal_ set nodeID $general_address_
}

#################
# Node Creation #
#################
createNode

################################
# Inter-node module connection #
################################

$app_ set destAddr_ [expr $opt(dest)]
$app_ set destPort_ 1


$ipr_ addRoute $opt(dest) $opt(dest)
$mll_ addentry  $opt(dest) $opt(dest)

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt
  
    $ns flush-trace
    close $opt(tracefile)
}

#####################
# Start/Stop Timers #
#####################
$ns at 0                  "$phy_ start"
$ns at $opt(starttime)    "$mac_ start"
$ns at $opt(starttime)    "$app_ start"
$ns at $opt(stoptime)     "$app_ stop"
$ns at $opt(stoptime)     "$mac_ stop"
$ns at $opt(stoptime)     "$phy_ stop"

##################
# Run simulation #
##################
# Specify the time at which to call the finish procedure and halt ns
$ns at [expr $opt(stoptime)] "finish; $ns halt"

# You always need the following line to run the NS-Miracle simulator
$ns run
