#!/usr/bin/env ns

# Three fixed LumaX-UV modems in a tree topology.
#
# Node 0 and node 2 exchange application messages through node 1:
#
#                         0
#                         |
#                         1
#                         |
#                         2
#
# The modem data address is the local UDP endpoint used by the driver.  The
# modem address is the endpoint used to configure that modem.

# Default simulation parameters.
set opt(nn) 3
set opt(start) 1.0
set opt(stop) 100.0
set opt(trace_files) 0
set opt(app_period) 0.1

# Command-line arguments for the three modem nodes.
if {$argc != 9} {
	puts "The script needs 9 input arguments to work:"
	puts "1 - Stop time"
	puts "2 - Node 0 local data endpoint (IP:port)"
	puts "3 - Node 0 modem configuration endpoint"
	puts "4 - Node 1 local data endpoint (IP:port)"
	puts "5 - Node 1 modem configuration endpoint"
	puts "6 - Node 2 local data endpoint (IP:port)"
	puts "7 - Node 2 modem configuration endpoint"
	puts "8 - Node 0 application TCP port for netcat"
	puts "9 - Node 2 application TCP port for netcat"
	puts "The local data endpoint is used for modem data transmission."
	puts "The modem configuration endpoint is used for modem control."
	puts "The application TCP ports are used by netcat, not by the modem."
	puts "Example:"
	puts "ns $argv0 100 192.168.102.50:55555 192.168.102.101 192.168.102.51:55555 192.168.102.102 192.168.102.52:55555 192.168.102.103 44440 44442"
	exit 1
}

# Read command-line values.
set opt(stop) [lindex $argv 0]
set opt(data0) [lindex $argv 1]
set opt(modem0) [lindex $argv 2]
set opt(data1) [lindex $argv 3]
set opt(modem1) [lindex $argv 4]
set opt(data2) [lindex $argv 5]
set opt(modem2) [lindex $argv 6]
set opt(app_port0) [lindex $argv 7]
set opt(app_port2) [lindex $argv 8]
set opt(time_stop) [expr {$opt(stop) + 5.0}]

# Load the simulator and DESERT modules.
load libMiracle.so
load libmphy.so
load libuwip.so
load libuwmll.so
load libuwstaticrouting.so
load libuwudp.so
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

# Create the simulator and enable real-time execution.
set ns [new Simulator]
$ns use-Miracle
$ns use-scheduler RealTime

# Initialize the random number generator and trace file.
global defaultRNG
$defaultRNG next-substream

if {$opt(trace_files)} {
	set opt(tracefile) [open "uwlumaxuvmodem_multihop.tr" w]
} else {
	set opt(tracefile) [open "/dev/null" w]
}
$ns trace-all $opt(tracefile)

# Configure the adaptation layer.
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 2048
Module/UW/AL set debug_ 0

# Configure packet header sizes.
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
NS2/MAC/Packer set debug_ 0
UW/UDP/Packer set SPort_Bits 8
UW/UDP/Packer set DPort_Bits 8
UW/UDP/Packer set debug_ 0

# Configure traffic, transport, and LumaXUV modem modules.
Module/UW/UDP set debug_ 0
Module/UW/APPLICATION set period_ $opt(app_period)
Module/UW/APPLICATION set Socket_Port_ $opt(app_port0)
Module/UW/APPLICATION set PoissonTraffic_ 0
Module/UW/APPLICATION set drop_out_of_order_ 0
Module/UW/APPLICATION set sea_trial_ 1
UW/APP/uwApplication/Packer set SN_FIELD_ 0
UW/APP/uwApplication/Packer set RFFT_FIELD_ 0
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 0
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 0
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 64
UW/APP/uwApplication/Packer set debug_ 0
Module/UW/UwModem/LumaXUV set debug_ 0
Module/UW/UwModem/LumaXUV set buffer_size 2048
Module/UW/UwModem/LumaXUV set max_read_size 2048

# Build one complete node stack and connect its modules.
proc createNode {id data_address modem_address} {
	global ns opt node app udp routing ipif mll mac uwal modem app_port

	set node($id) [$ns create-M_Node]
	set app($id) [new Module/UW/APPLICATION]
	set udp($id) [new Module/UW/UDP]
	set routing($id) [new Module/UW/StaticRouting]
	set ipif($id) [new Module/UW/IP]
	set mll($id) [new Module/UW/MLL]
	set mac($id) [new Module/UW/CSMA_ALOHA]
	set uwal($id) [new Module/UW/AL]
	set modem($id) [new Module/UW/UwModem/LumaXUV]

	$node($id) addModule 8 $app($id) 1 "UWA"
	$node($id) addModule 7 $udp($id) 1 "UDP"
	$node($id) addModule 6 $routing($id) 1 "IPR"
	$node($id) addModule 5 $ipif($id) 1 "IPIF"
	$node($id) addModule 4 $mll($id) 1 "MLL"
	$node($id) addModule 3 $mac($id) 1 "ALOHA"
	$node($id) addModule 2 $uwal($id) 1 "UWAL"
	$node($id) addModule 1 $modem($id) 1 "LumaXUV"

	$node($id) setConnection $app($id) $udp($id) trace
	$node($id) setConnection $udp($id) $routing($id) trace
	$node($id) setConnection $routing($id) $ipif($id) trace
	$node($id) setConnection $ipif($id) $mll($id) trace
	$node($id) setConnection $mll($id) $mac($id) trace
	$node($id) setConnection $mac($id) $uwal($id) trace
	$node($id) setConnection $uwal($id) $modem($id) trace

	$ipif($id) addr [expr {$id + 1}]
	$mac($id) setMacAddr [expr {$id + 1}]
	$modem($id) set ID_ $id
	$modem($id) setLocalAddress $data_address
	$modem($id) setModemAddress $modem_address
	$modem($id) setLogLevel ERR
	$modem($id) start_state 1
	$app($id) set node_ID_ $id
	set app_port($id) [$udp($id) assignPort $app($id)]
	if {$id == 0 || $id == 2} {
		$app($id) setSocketProtocol "TCP"
		if {$id == 0} {
			$app($id) set Socket_Port_ $opt(app_port0)
		} else {
			$app($id) set Socket_Port_ $opt(app_port2)
		}
	}

	set packer [new UW/AL/Packer]
	$packer addPacker [new NS2/COMMON/Packer]
	$packer addPacker [new NS2/MAC/Packer]
	$packer addPacker [new UW/IP/Packer]
	$packer addPacker [new UW/UDP/Packer]
	$packer addPacker [new UW/APP/uwApplication/Packer]
	$uwal($id) linkPacker $packer
	$uwal($id) set nodeID $id

	$mac($id) setNoAckMode
	$mac($id) initialize
	$routing($id) enableLog
}

# Create the three fixed nodes.
createNode 0 $opt(data0) $opt(modem0)
createNode 1 $opt(data1) $opt(modem1)
createNode 2 $opt(data2) $opt(modem2)

# Fill the IP-to-MAC tables on every node.
for {set id 0} {$id < $opt(nn)} {incr id} {
	for {set other 0} {$other < $opt(nn)} {incr other} {
		if {$id != $other} {
			$mll($id) addentry [$ipif($other) addr] [$mac($other) addr]
		}
	}
}

# Configure the tree routes through node 1.
$routing(0) addRoute [$ipif(2) addr] [$ipif(1) addr]
$routing(2) addRoute [$ipif(0) addr] [$ipif(1) addr]
$routing(1) addRoute [$ipif(0) addr] [$ipif(0) addr]
$routing(1) addRoute [$ipif(2) addr] [$ipif(2) addr]

# Configure the three traffic classes between the two leaf nodes.
# Set the final destinations for both application sockets.
$app(0) set destAddr_ [$ipif(2) addr]
$app(0) set destPort_ $app_port(2)
$app(2) set destAddr_ [$ipif(0) addr]
$app(2) set destPort_ $app_port(0)

# Start and stop the two external application endpoints.
$ns at $opt(start) "$app(0) start"
$ns at $opt(start) "$app(2) start"
$ns at $opt(stop) "$app(0) stop"
$ns at $opt(stop) "$app(2) stop"

# Start and stop each physical modem with the simulation.
for {set id 0} {$id < $opt(nn)} {incr id} {
	$ns at 0 "$modem($id) start"
	$ns at $opt(time_stop) "$modem($id) stop"
}

# Flush traces and close the output file.
proc finish {} {
	global ns opt
	$ns flush-trace
	close $opt(tracefile)
	puts "LumaXUV multihop test completed"
}

# Schedule cleanup and run the experiment.
$ns at $opt(time_stop) "finish; $ns halt"
$ns run
