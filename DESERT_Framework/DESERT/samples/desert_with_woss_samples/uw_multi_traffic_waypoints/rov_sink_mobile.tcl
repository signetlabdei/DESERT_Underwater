set scr [info script]
proc createROV { id } {

  global ns cbr_rov cbr2_rov cbr3_rov cbr4_rov position_rov node_rov udp_rov  
  global portnum_rov portnum_rov_sos portnum3_rov ipr_rov ipif_rov ipr_rov_sos ipif_rov_sos
  global channel channel_op propagation propagation_op data_mask data_mask_hf data_mask_op
  global phy_rov posdb_rov opt rvposx mll_rov mll_hf_rov mll_op_rov mac_rov mac_hf_rov 
  global mac_op_rov db_manager node_rov_coordinates portnum4_rov woss_utilities defaultRNG
  
  set node_rov [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

  # TRAFFICO 1: HEALTH MONITORING DIVERS --> LEADER, LEADER --> ROV
  Module/UW/CBR set traffic_type_ 1
  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    set cbr_rov($id1)  [new Module/UW/CBR]
  }

  # TRAFFICO 2: CONTROL: LEADER --> DIVERS, LEADER --> ROV via long range in CSMA
  Module/UW/CBR set traffic_type_ 2
  set cbr2_rov  [new Module/UW/CBR]

  # TRAFFICO 3: IMMAGINI: DIVERS --> LEADER, LEADER --> ROV
  Module/UW/CBR set traffic_type_ 3
  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    set cbr3_rov($id1)  [new Module/UW/CBR]
  }

  # TRAFFICO 4: SOS: broadcast
  Module/UW/CBR set traffic_type_ 4
  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    set cbr4_rov($id1)  [new Module/UW/CBR]
  }
  
  set udp_rov  [new Module/UW/UDP]
  set ipr_rov  [new Module/UW/StaticRouting]
  set ipif_rov [new Module/UW/IP]

  set udp_rov_sos  [new Module/UW/UDP]
  set ipr_rov_sos  [new Module/UW/FLOODING]
  set ipif_rov_sos [new Module/UW/IP]

  set ctr_rov  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]

  set mll_rov  [new Module/UW/MLL] 
  set mac_rov  [new Module/UW/CSMA_ALOHA] 
  Module/UW/PHYSICAL  set BitRate_          $opt(bitrate)
  Module/UW/PHYSICAL  set MaxTxSPL_dB_        $opt(txpower)
  set phy_rov  [new Module/UW/PHYSICAL]
  $mll_rov setstackid 1

  set mll_hf_rov  [new Module/UW/MLL] 
  set mac_hf_rov  [new Module/UW/CSMA_ALOHA]
  Module/UW/PHYSICAL  set BitRate_          $opt(bitrate_hf)
  Module/UW/PHYSICAL  set MaxTxSPL_dB_        $opt(txpower_hf)
  set phy_hf_rov  [new Module/UW/PHYSICAL]
  $mll_hf_rov setstackid 2

  set mll_op_rov  [new Module/UW/MLL] 
  Module/UW/CSMA_ALOHA set listen_time_      [expr 1.0e-12]
  Module/UW/CSMA_ALOHA set wait_costant_     [expr 1.0e-12]
  set mac_op_rov  [new Module/UW/CSMA_ALOHA]
  set phy_op_rov  [new Module/UW/OPTICAL/PHY]

  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    $node_rov addModule 8 $cbr_rov($id1)   0  "CBR1"
    $node_rov addModule 8 $cbr3_rov($id1)  0  "CBR3"
  }

  $node_rov addModule 8 $cbr2_rov  0  "CBR2"
  $node_rov addModule 7 $udp_rov   0  "UDP1"
  $node_rov addModule 6 $ipr_rov   0  "IPR1"
  $node_rov addModule 5 $ipif_rov  2  "IPF1"   
  
  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    $node_rov addModule 8 $cbr4_rov($id1)   0  "CBR4"
  }
  $node_rov addModule 7 $udp_rov_sos   0  "UDP2"
  $node_rov addModule 6 $ipr_rov_sos   0  "IPR2"
  $node_rov addModule 5 $ipif_rov_sos  2  "IPF2" 

  $node_rov addModule 4 $ctr_rov   2  "CTR"   

  $node_rov addModule 3 $mll_rov   2  "MLL_LF"
  $node_rov addModule 2 $mac_rov   2  "MAC_LF"
  $node_rov addModule 1 $phy_rov   0  "PHY_LF"

  $node_rov addModule 3 $mll_op_rov   2  "MLL_OP"
  $node_rov addModule 2 $mac_op_rov   2  "MAC_OP"
  $node_rov addModule 1 $phy_op_rov   0  "PHY_OP"

  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    $node_rov setConnection $cbr_rov($id1)   $udp_rov   0
    $node_rov setConnection $cbr3_rov($id1)  $udp_rov   0
  }
  $node_rov setConnection $cbr2_rov  $udp_rov   0
  $node_rov setConnection $udp_rov   $ipr_rov   0
  $node_rov setConnection $ipr_rov   $ipif_rov  2
  $node_rov setConnection $ipif_rov  $ctr_rov   2

  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    $node_rov setConnection $cbr4_rov($id1)  $udp_rov_sos   0
  }
  $node_rov setConnection $udp_rov_sos  $ipr_rov_sos   0
  $node_rov setConnection $ipr_rov_sos  $ipif_rov_sos  2
  $node_rov setConnection $ipif_rov_sos $ctr_rov   2

  $node_rov setConnection $ctr_rov   $mll_rov   2
  $node_rov setConnection $mll_rov   $mac_rov   2
  $node_rov setConnection $mac_rov   $phy_rov   2
  $node_rov addToChannel  $channel  $phy_rov   0

  $node_rov setConnection $ctr_rov   $mll_op_rov   2
  $node_rov setConnection $mll_op_rov  $mac_op_rov   2
  $node_rov setConnection $mac_op_rov  $phy_op_rov   2
  $node_rov addToChannel  $channel_op  $phy_op_rov   0

  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    set portnum_rov($id1) [$udp_rov assignPort $cbr_rov($id1) ]
    set portnum3_rov($id1) [$udp_rov assignPort $cbr3_rov($id1) ]
    set portnum4_rov($id1) [$udp_rov_sos assignPort $cbr4_rov($id1) ]
  }
  set portnum_rov_sos [$udp_rov assignPort $cbr2_rov ]

  if {$id > 254} {
    puts "hostnum > 254!!! exiting"
    exit
  }
  set tmp_ [expr $id + 1]
  $ipif_rov addr $tmp_
  $ipif_rov_sos addr $tmp_
  $ipr_rov_sos addr $tmp_
  
  WOSS/Position/WayPoint set time_threshold_      [expr 1.0 / $opt(speed_rov)]
  set position_rov [new "WOSS/Position/WayPoint"]
  $node_rov addPosition $position_rov
  set posdb_rov [new "PlugIn/PositionDB"]
  $node_rov addPlugin $posdb_rov 20 "PDB"
  $posdb_rov addpos [$ipif_rov addr] $position_rov

  set interf_data [new "Module/UW/INTERFERENCE"]
  $interf_data set maxinterval_ $opt(maxinterval_)

  set interf_data_hf [new "Module/UW/INTERFERENCE"]
  $interf_data_hf set maxinterval_ $opt(maxinterval_)

  set interf_data_op [new "MInterference/MIV"]
  $interf_data_op set maxinterval_ $opt(maxinterval_)

  $phy_rov setPropagation $propagation  
  $phy_rov setSpectralMask $data_mask
  $phy_rov setInterference $interf_data

  $phy_op_rov setPropagation $propagation_op  
  $phy_op_rov setSpectralMask $data_mask_op
  $phy_op_rov setInterference $interf_data_op
  $phy_op_rov useLUT

  $mac_rov $opt(ack_mode)
  $mac_rov setMacAddr $tmp_
  $mac_rov initialize

  $mac_op_rov setMacAddr $tmp_
  $mac_op_rov $opt(ack_mode)
  $mac_op_rov initialize

#   HEALTH TRAFFIC
  $ctr_rov addRobustLowLayer 1  "MLL_LF"
  $ctr_rov addFastLowLayer 1   "MLL_OP"
  $ctr_rov addUpLayer 1     "IPF1"

#   CONTROL TRAFFIC
  $ctr_rov addRobustLowLayer 2  "MLL_LF"
  $ctr_rov addUpLayer 2     "IPF1"

#   IMAGE TRAFFIC
  $ctr_rov addFastLowLayer 3   "MLL_OP"
  $ctr_rov addUpLayer 3      "IPF1"
#   SOS TRAFFIC: to all (no spec needed)

  $ctr_rov addUpLayer 4      "IPF2"
}