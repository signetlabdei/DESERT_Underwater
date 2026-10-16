#!/usr/bin/env ns

# Local, no-hardware multihop test using simulated underwater PHY modules.
#
# Node 0 and node 2 exchange TCP application messages through node 1:
#
#                         0
#                         |
#                         1
#                         |
#                         2
#
# The nodes are fixed at 0 m, 100 m, and 200 m. The simulated PHY channel
# models the radio path; it does not use any LumaX-UV hardware.

# Simulation and socket defaults.
set opt(nn) 3
set opt(start) 1.0
set opt(stop) 60.0
set opt(app_period) 0.1
set opt(app_port0) 44440
set opt(app_port2) 44442
set opt(trace_files) 0
set opt(tracefilename) "uwlumaxuvmodem_multihop_sim.tr"

# Optional command-line arguments: stop-time, node-0 TCP port, node-2 TCP port.
if {$argc != 0 && $argc != 3} {
	puts "Usage: ns $argv0 ?stop_time node0_tcp_port node2_tcp_port?"
	puts "The TCP ports are used by netcat, not by a modem."
	puts "Example: ns $argv0 60 44440 44442"
	exit 1
}
if {$argc == 3} {
	set opt(stop) [lindex $argv 0]
	set opt(app_port0) [lindex $argv 1]
	set opt(app_port2) [lindex $argv 2]
}
set opt(time_stop) [expr {$opt(stop) + 1.0}]

# Load the simulator, DESERT stack, and simulated PHY libraries.
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
load libuwcsmaaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwapplication.so
load libpackeruwapplication.so
load libuwaloha.so
load libuwal.so
load libpackeruwip.so
load libpackercommon.so
load libpackermac.so
load libpackeruwudp.so
load libuwphy_clmsgs.so
load libuwinterference.so
load libUwmStdPhyBpskTracer.so

# Use real-time scheduling so netcat can connect during the run.
set ns [new Simulator]
$ns use-Miracle
$ns use-scheduler RealTime

# Initialize the random number generator and trace output.
global defaultRNG
$defaultRNG seed 1
if {$opt(trace_files)} {
	set opt(tracefile) [open $opt(tracefilename) w]
} else {
	set opt(tracefile) [open "/dev/null" w]
}
$ns trace-all $opt(tracefile)

# Create the shared simulated underwater channel and propagation model.
set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
MPropagation/Underwater set practicalSpreading_ 1.5
MPropagation/Underwater set shipping_ 1

set data_mask [new MSpectralMask/Rect]
$data_mask setFreq 25000.0
$data_mask setBandwidth 5000.0

# Configure the simulated physical layer and packet stack.
Module/MPhy/BPSK set BitRate_ 4800.0
Module/MPhy/BPSK set TxPower_ 135.0
Module/UW/CSMA_ALOHA set wait_costant_ 0.0001
Module/UW/CSMA_ALOHA set listen_time_ 0.0001
Module/UW/UDP set debug_ 0
Module/UW/APPLICATION set debug_ 1
Module/UW/APPLICATION set period_ $opt(app_period)
Module/UW/APPLICATION set Socket_Port_ $opt(app_port0)
Module/UW/APPLICATION set PoissonTraffic_ 0
Module/UW/APPLICATION set drop_out_of_order_ 0
Module/UW/APPLICATION set sea_trial_ 0

# Configure the AL and application packet fields.
Module/UW/AL set Dbit 0
Module/UW/AL set PSDU 2048
Module/UW/AL set debug_ 0
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
UW/APP/uwApplication/Packer set SN_FIELD_ 0
UW/APP/uwApplication/Packer set RFFT_FIELD_ 0
UW/APP/uwApplication/Packer set RFFTVALID_FIELD_ 0
UW/APP/uwApplication/Packer set PRIORITY_FIELD_ 0
UW/APP/uwApplication/Packer set PAYLOADMSG_FIELD_SIZE_ 64
UW/APP/uwApplication/Packer set debug_ 0

# Build one simulated node with an application TCP endpoint.
proc createNode {id x_coordinate} {
	global ns channel propagation data_mask opt node app udp routing ipif mll mac phy port position

	set node($id) [$ns create-M_Node $opt(tracefile) $opt(tracefile)]
	set app($id) [new Module/UW/APPLICATION]
	set udp($id) [new Module/UW/UDP]
	set routing($id) [new Module/UW/StaticRouting]
	set ipif($id) [new Module/UW/IP]
	set mll($id) [new Module/UW/MLL]
	set mac($id) [new Module/UW/CSMA_ALOHA]
	set phy($id) [new Module/MPhy/BPSK]

	$node($id) addModule 8 $app($id) 0 "UWA"
	$node($id) addModule 7 $udp($id) 0 "UDP"
	$node($id) addModule 6 $routing($id) 0 "IPR"
	$node($id) addModule 5 $ipif($id) 0 "IPIF"
	$node($id) addModule 4 $mll($id) 0 "MLL"
	$node($id) addModule 3 $mac($id) 0 "CSMA"

	$node($id) setConnection $app($id) $udp($id) 0
	$node($id) setConnection $udp($id) $routing($id) 0
	$node($id) setConnection $routing($id) $ipif($id) 0
	$node($id) setConnection $ipif($id) $mll($id) 0
	$node($id) setConnection $mll($id) $mac($id) 0

	$ipif($id) addr [expr {$id + 1}]
	$mac($id) setMacAddr [expr {$id + 1}]
	$app($id) set node_ID_ [expr {$id + 1}]
	set port($id) [$udp($id) assignPort $app($id)]

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
	set uwal($id) [new Module/UW/AL]
	$node($id) addModule 2 $uwal($id) 0 "UWAL"
	$node($id) addModule 1 $phy($id) 0 "PHY"
	$node($id) setConnection $mac($id) $uwal($id) 0
	$node($id) setConnection $uwal($id) $phy($id) 0
	$node($id) addToChannel $channel $phy($id) 0
	$uwal($id) linkPacker $packer
	$uwal($id) set nodeID [expr {$id + 1}]

	set position($id) [new Position/BM]
	$node($id) addPosition $position($id)
	$position($id) setX_ $x_coordinate
	$position($id) setY_ 0
	$position($id) setZ_ -100

	$phy($id) setPropagation $propagation
	$phy($id) setSpectralMask $data_mask
	$mac($id) setNoAckMode
	$mac($id) initialize
	$routing($id) enableLog
}

# Create the fixed three-node chain.
createNode 0 0
createNode 1 100
createNode 2 200

# Populate neighbor address resolution tables.
for {set id 0} {$id < $opt(nn)} {incr id} {
	for {set other 0} {$other < $opt(nn)} {incr other} {
		if {$id != $other} {
			$mll($id) addentry [$ipif($other) addr] [$mac($other) addr]
		}
	}
}

# Route leaf-to-leaf traffic through node 1.
$routing(0) addRoute [$ipif(2) addr] [$ipif(1) addr]
$routing(2) addRoute [$ipif(0) addr] [$ipif(1) addr]
$routing(1) addRoute [$ipif(0) addr] [$ipif(0) addr]
$routing(1) addRoute [$ipif(2) addr] [$ipif(2) addr]

# Set application destinations using internal UW/UDP ports.
$app(0) set destAddr_ [$ipif(2) addr]
$app(0) set destPort_ $port(2)
$app(2) set destAddr_ [$ipif(0) addr]
$app(2) set destPort_ $port(0)

# Start and stop the two TCP application endpoints.
$ns at $opt(start) "$app(0) start"
$ns at $opt(start) "$app(2) start"
$ns at $opt(stop) "$app(0) stop"
$ns at $opt(stop) "$app(2) stop"

proc finish {} {
	global ns opt app
	puts "node 0 sent: [$app(0) getsentpkts]"
	puts "node 0 received: [$app(0) getrecvpkts]"
	puts "node 2 sent: [$app(2) getsentpkts]"
	puts "node 2 received: [$app(2) getrecvpkts]"
	$ns flush-trace
	close $opt(tracefile)
}

$ns at $opt(time_stop) "finish; $ns halt"
$ns run
