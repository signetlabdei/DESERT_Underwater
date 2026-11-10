# ----------------------------------------------------------------------------------
# This script depicts a very simple but complete stack in which multiple nodes send data
# to a common sink. The second node is used by the first one as a relay to send data to the sink.
# The routes are configured by using UW/STATICROUTING.
# The application used to generate data is UW/APPLICATION.
# ----------------------------------------------------------------------------------
# Stack
#             Node 1                         Node 2                        Sink
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  8. UW/APPLICATION       |   |  8. UW/APPLICATION       |   |  8. UW/APP  | UW/APP     |
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  7. UW/TLS               |   |  7. UW/TLS               |   |  7. UW/TLS  | UW/TLS     |
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  6. UW/UDP               |   |  6. UW/UDP               |   |  6. UW/UDP               |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  4. UW/IP                |   |  4. UW/IP                |   |  4. UW/IP                |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  3. UW/MLL               |   |  3. UW/MLL               |   |  3. UW/MLL               |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  2. UW/CSMA_ALOHA        |   |  2. UW/CSMA_ALOHA        |   |  2. UW/CSMA_ALOHA        |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  1. UW/PHYSICAL          |   |  1. UW/PHYSICAL          |   |  1. UW/PHYSICAL          |
#   +--------------------------+   +--------------------------+   +--------------------------+
#            |         |                    |         |                   |         |       
#   +----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                  |
#   +----------------------------------------------------------------------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(trace_files)        1
set opt(bash_parameters)    0

#####################
# Library Loading   #
#####################
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
load libuwphysical.so
load libuwtls.so

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
set opt(nn)                 2
set opt(starttime)          5

set opt(stoptime)           200

set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]

set opt(maxinterval_)       20.0
set opt(freq)               25000.0
set opt(bw)                 5000.0
set opt(bitrate)            48000.0
set opt(ack_mode)           "setNoAckMode"

set opt(txpower)            180.0 
set opt(propagation_speed)  1400

set opt(rngstream)          1
set opt(pktsize)            125
set opt(cbr_period)         0.2
set opt(socket_comm)        1

if {$opt(bash_parameters)} {
    if {$argc != 4} {
        puts "ERROR!!!!!!"
        puts "The script requires four inputs:"
        puts "---> The DATA packet size (byte)"
        puts "---> Period to generate DATA packets (s)"
        puts "---> socket_comm"
        puts "---> Random generator substream"
        puts "For example: ns test_uwApplicationTLS.tcl 125 60 0 13"
        puts "Please try again."
        return
    } else {
        if {[lindex $argv 0] > 4096} {
            puts "ERROR!!!!!!"
            puts "The payload size is too high. It must be less then 4096 charachters."
            puts "Please try again."
            return
        } else {
            set opt(pktsize)       [lindex $argv 0]
            set opt(cbr_period)    [lindex $argv 1]
            set opt(socket_comm)   [lindex $argv 2]
            set opt(rngstream)     [lindex $argv 3]
        }
    }
} 

global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
    $defaultRNG next-substream
}


if {$opt(trace_files)} {
    set opt(tracefilename) "./test_uwApplicationTLS.tr"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "./test_uwApplicationTCP.cltr"
    set opt(cltracefile) [open $opt(tracefilename) w]
} else {
    set opt(tracefilename) "/dev/null"
    set opt(tracefile) [open $opt(tracefilename) w]
    set opt(cltracefilename) "/dev/null"
    set opt(cltracefile) [open $opt(cltracefilename) w]
}

MPropagation/Underwater set debug_              1
MPropagation/Underwater set windspeed_          10
MPropagation/Underwater set shipping_           0

set channel [new Module/UnderwaterChannel]
set propagation [new MPropagation/Underwater]
set data_mask [new MSpectralMask/Rect]
$data_mask setFreq              $opt(freq)
$data_mask setBandwidth         $opt(bw)
$data_mask setPropagationSpeed  $opt(propagation_speed)

#########################
# Module Configuration  #
#########################
#UW/APPLICATION
Module/UW/APPLICATION set debug_             2
Module/UW/APPLICATION set period_            $opt(cbr_period)
Module/UW/APPLICATION set PoissonTraffic_    0
Module/UW/APPLICATION set Payload_size_      $opt(pktsize)
Module/UW/APPLICATION set drop_out_of_order_ 1
Module/UW/APPLICATION set EXP_ID_            1

# Physical
Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    4.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 200
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0

################################
# Procedure(s) to create nodes #
################################
proc createNode { id } {

    global channel propagation data_mask ns app position node udp portnum ipr ipif
    global phy posdb opt rvposx mll mac db_manager
    global node_coordinates
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

    set app($id)  [new Module/UW/APPLICATION] 
    set tls($id)  [new Module/UW/TLS]
    set udp($id)  [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/CSMA_ALOHA] 
    set phy($id)  [new Module/UW/PHYSICAL]

    $node($id) addModule 8 $app($id)   0  "CBR"
    $node($id) addModule 7 $tls($id)   0  "TLS"
    $node($id) addModule 6 $udp($id)   0  "UDP"
    $node($id) addModule 5 $ipr($id)   0  "IPR"
    $node($id) addModule 4 $ipif($id)  0  "IPF"   
    $node($id) addModule 3 $mll($id)   0  "MLL"
    $node($id) addModule 2 $mac($id)   0  "MAC"
    $node($id) addModule 1 $phy($id)   0  "PHY"

    $node($id) setConnection $app($id)   $tls($id)   0
    $node($id) setConnection $tls($id)   $udp($id)   0
    $node($id) setConnection $udp($id)   $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy($id)   0
    $node($id) addToChannel  $channel    $phy($id)   0

    set portnum($id) [$udp($id) assignPort $tls($id) ]
    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }
    set tmp_ [expr ($id) + 1]
    $ipif($id) addr $tmp_
    
    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)
    
    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0

    $phy($id) setPropagation $propagation
    set prt_ [expr ($id) + 4000]
    $app($id) set Socket_Port_  $prt_
    $app($id) setSocketProtocol "TCP"
    $app($id) set node_ID_  $tmp_
    $mac($id) $opt(ack_mode)
    $mac($id) initialize

    $tls($id) loadPublicKey "/path/to/public_key.pem"
    $tls($id) loadPrivateKey "/path/to/private_key.pem"

    $tls($id) setLogLevel 3
    
    #Propagation model
    $phy($id) setPropagation $propagation
    
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf_data($id)
    $phy($id) setInterferenceModel "MEANPOWER"
}

proc createSink { } {

    global channel propagation smask data_mask ns app_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set app_sink($cnt)  [new Module/UW/APPLICATION]
        set tls_sink($cnt)  [new Module/UW/TLS]
    }
    set udp_sink       [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/CSMA_ALOHA]
    set phy_data_sink  [new Module/UW/PHYSICAL]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node_sink addModule 8 $app_sink($cnt) 0 "CBR"
        $node_sink addModule 7 $tls_sink($cnt) 0 "TLS"

        $tls_sink($cnt) loadPublicKey "/path/to/public_key.pem"
        $tls_sink($cnt) loadPrivateKey "/path/to/private_key.pem"

        $tls_sink($cnt) setLogLevel 3
    }
    $node_sink addModule 6 $udp_sink       0 "UDP"
    $node_sink addModule 5 $ipr_sink       0 "IPR"
    $node_sink addModule 4 $ipif_sink      0 "IPF"   
    $node_sink addModule 3 $mll_sink       0 "MLL"
    $node_sink addModule 2 $mac_sink       0 "MAC"
    $node_sink addModule 1 $phy_data_sink  0 "PHY"

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        $node_sink setConnection $app_sink($cnt)  $tls_sink($cnt)    0
        $node_sink setConnection $tls_sink($cnt)  $udp_sink          0
    }
    $node_sink setConnection $udp_sink  $ipr_sink            0
    $node_sink setConnection $ipr_sink  $ipif_sink           0
    $node_sink setConnection $ipif_sink $mll_sink            0 
    $node_sink setConnection $mll_sink  $mac_sink            0
    $node_sink setConnection $mac_sink  $phy_data_sink       0
    $node_sink addToChannel  $channel   $phy_data_sink       0

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set portnum_sink($cnt) [$udp_sink assignPort $tls_sink($cnt)]
        if {$cnt > 252} {
            puts "hostnum > 252!!! exiting"
            exit
        }    
    }
    
    $ipif_sink addr 6

    set position_sink [new "Position/BM"]
    $node_sink addPosition $position_sink
    set posdb_sink [new "PlugIn/PositionDB"]
    $node_sink addPlugin $posdb_sink 20 "PDB"
    $posdb_sink addpos [$ipif_sink addr] $position_sink

    set interf_data_sink [new "Module/UW/INTERFERENCE"]
    $interf_data_sink set maxinterval_ $opt(maxinterval_)
    $interf_data_sink set debug_       0

    for {set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
        set prt_ [expr ($cnt) + 5000]
        $app_sink($cnt) set Socket_Port_  $prt_
        $app_sink($cnt) setSocketProtocol "TCP"
        $app_sink($cnt) set node_ID_ 254
    }
    $mac_sink $opt(ack_mode)
    $mac_sink initialize
    
    #Propagation model
    $phy_data_sink setPropagation $propagation
    
    $phy_data_sink setSpectralMask $data_mask
    $phy_data_sink setInterference $interf_data_sink
    $phy_data_sink setInterferenceModel "MEANPOWER"
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}
createSink

################################
# Inter-node module connection #
################################
proc connectNodes {id1} {
    global ipif ipr portnum app app_sink ipif_sink portnum_sink ipr_sink

    $app($id1) set destAddr_ [$ipif_sink addr]
    $app($id1) set destPort_ $portnum_sink($id1)
    $app_sink($id1) set destAddr_ [$ipif($id1) addr]
    $app_sink($id1) set destPort_ $portnum($id1)
}

# Setup flows
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    connectNodes $id1
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }   
    $mll($id1) addentry [$ipif_sink addr] [ $mac_sink addr]
    $mll_sink addentry [$ipif($id1) addr] [ $mac($id1) addr]
}

# Setup positions
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $position($id1) setX_ 500
    $position($id1) setY_ 0
    $position($id1) setZ_ -1000
}

$position_sink setX_ 0
$position_sink setY_ 0
$position_sink setZ_ -1000

# Setup routing table
for {set id1 0} {$id1 < [expr $opt(nn) - 1]} {incr id1}  {
    set id2 [expr $id1 + 1]
    $ipr($id1) addRoute [$ipif_sink addr] [$ipif($id2) addr]
    $ipr_sink addRoute [$ipif($id1) addr] [$ipif($id1) addr]
}
set last_id [expr int($opt(nn) - 1)]
$ipr($last_id) addRoute [$ipif_sink addr] [$ipif_sink addr]
$ipr_sink addRoute [$ipif($last_id) addr] [$ipif($last_id) addr]


###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
    global ns opt
    global mac propagation app_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global node_coordinates
    global ipr_sink ipr ipif udp app phy phy_data_sink
    global node_stats tmp_node_stats sink_stats tmp_sink_stats

    puts "---------------------------------------------------------------------"
    puts "Simulation summary"
    puts "number of nodes  : $opt(nn)"
    puts "packet size      : $opt(pktsize) byte"
    puts "app period       : $opt(cbr_period) s"
    puts "number of nodes  : $opt(nn)"
    puts "simulation length: $opt(txduration) s"
    puts "tx frequency     : $opt(freq) Hz"
    puts "tx bandwidth     : $opt(bw) Hz"
    puts "bitrate          : $opt(bitrate) bps"
    puts "---------------------------------------------------------------------"

    set sum_cbr_throughput     0
    set sum_per                0
    set sum_cbr_sent_pkts      0.0
    set sum_cbr_rcv_pkts       0.0    
    set sum_cbr_queue_pkts		 0.0

    for {set i 0} {$i < $opt(nn)} {incr i}  {
        set cbr_throughput [$app_sink($i) getthr]
        set cbr_sent_pkts [$app($i) getsentpkts]
        
        set cbr_rcv_pkts [$app_sink($i) getrecvpkts]
        if { $opt(socket_comm) == 1 } {	
            set cbr_queue_pkts [$app($i) getrecvpktsqueue]
        }
        set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
        set sum_cbr_sent_pkts  [expr $sum_cbr_sent_pkts + $cbr_sent_pkts]
        set sum_cbr_rcv_pkts   [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
        if { $opt(socket_comm) == 1 } {
            set sum_cbr_queue_pkts [expr $sum_cbr_queue_pkts + $cbr_queue_pkts]
        }
    }
        
    puts "Mean Throughput          : [expr ($sum_cbr_throughput/($opt(nn)))]"

    puts "Sent Packets             : $sum_cbr_sent_pkts"
    puts "Received Packets         : $sum_cbr_rcv_pkts"
    puts "Packet Delivery Ratio    : [expr $sum_cbr_rcv_pkts / $sum_cbr_sent_pkts * 100]"
  
    $ns flush-trace
    close $opt(tracefile)
}

###################
# start simulation
###################
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)    "$app($id1) start"
    $ns at $opt(stoptime)     "$app($id1) stop"
    $ns at $opt(starttime)    "$app_sink($id1) start"
    $ns at $opt(stoptime)     "$app_sink($id1) stop"
}

$ns at [expr $opt(stoptime) + 30.0] "finish; $ns halt"
$ns run
