set scr [info script]
proc createLeader { id } {

  global ns cbr_leader cbr2_leader cbr3_leader cbr4_leader position_leader node_leader udp_leader  
  global portnum_leader portnum_leader_sos portnum3_leader ipr_leader ipif_leader ipr_leader_sos ipif_leader_sos
  global channel channel_op propagation propagation_op data_mask data_mask_hf data_mask_op
  global phy_leader posdb_leader opt rvposx mll_leader mll_hf_leader mll_op_leader mac_leader mac_hf_leader 
  global mac_op_leader db_manager node_leader_coordinates woss_utilities defaultRNG
  
  set node_leader [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

  # TRAFFICO 1: HEALTH MONITORING DIVERS --> LEADER, LEADER --> ROV
  Module/UW/CBR set packetSize_      $opt(pktsize_health)
  Module/UW/CBR set period_        $opt(cbr_period_health)
  Module/UW/CBR set PoissonTraffic_    0

  Module/UW/CBR set traffic_type_ 1
  set cbr_leader  [new Module/UW/CBR]

  # TRAFFICO 2: CONTROL: LEADER --> DIVERS, LEADER --> ROV via long range in CSMA
  Module/UW/CBR set packetSize_      $opt(pktsize_control)
  Module/UW/CBR set period_        $opt(cbr_period_control)
  Module/UW/CBR set PoissonTraffic_    1
  Module/UW/CBR set traffic_type_ 2
  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    set cbr2_leader($id1)  [new Module/UW/CBR]
  }

  # TRAFFICO 3: IMMAGINI: DIVERS --> LEADER, LEADER --> ROV
  Module/UW/CBR set packetSize_      $opt(pktsize_image)
  Module/UW/CBR set period_        $opt(cbr_period_image)
  Module/UW/CBR set PoissonTraffic_    1
  Module/UW/CBR set traffic_type_ 3
  set cbr3_leader  [new Module/UW/CBR]

  # TRAFFICO 4: SOS: broadcast
  Module/UW/CBR set packetSize_      $opt(pktsize_sos)
  Module/UW/CBR set period_        $opt(cbr_period_sos)
  Module/UW/CBR set PoissonTraffic_    1
  Module/UW/CBR set traffic_type_ 4
  set cbr4_leader  [new Module/UW/CBR]

  set udp_leader  [new Module/UW/UDP]
  set ipr_leader  [new Module/UW/StaticRouting]
  set ipif_leader [new Module/UW/IP]

  set udp_leader_sos  [new Module/UW/UDP]
  set ipr_leader_sos  [new Module/UW/FLOODING]
  set ipif_leader_sos [new Module/UW/IP]

  Module/UW/MULTI_TRAFFIC_RANGE_CTR set debug_      0
  set ctr_leader  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]

  set mll_leader  [new Module/UW/MLL] 
  set mac_leader  [new Module/UW/CSMA_ALOHA] 
  Module/UW/PHYSICAL  set BitRate_          $opt(bitrate)
  Module/UW/PHYSICAL  set MaxTxSPL_dB_        $opt(txpower)
  set phy_leader  [new Module/UW/PHYSICAL]
  $mll_leader setstackid 1

  set mll_hf_leader  [new Module/UW/MLL] 
  set mac_hf_leader  [new Module/UW/CSMA_ALOHA]
  Module/UW/PHYSICAL  set BitRate_          $opt(bitrate_hf)
  Module/UW/PHYSICAL  set MaxTxSPL_dB_        $opt(txpower_hf)
  set phy_hf_leader  [new Module/UW/PHYSICAL]
  $mll_hf_leader setstackid 2

  set mll_op_leader  [new Module/UW/MLL] 
  Module/UW/CSMA_ALOHA set listen_time_      [expr 1.0e-12]
  Module/UW/CSMA_ALOHA set wait_costant_     [expr 1.0e-12]
  set mac_op_leader  [new Module/UW/CSMA_ALOHA]
  set phy_op_leader  [new Module/UW/OPTICAL/PHY]

  $node_leader addModule 8 $cbr_leader   0  "CBR1"
  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    $node_leader addModule 8 $cbr2_leader($id1)  0  "CBR2"
  }
  $node_leader addModule 8 $cbr3_leader  0  "CBR3"
  $node_leader addModule 7 $udp_leader   0  "UDP1"
  $node_leader addModule 6 $ipr_leader   0  "IPR1"
  $node_leader addModule 5 $ipif_leader  2  "IPF1"   
  
  $node_leader addModule 8 $cbr4_leader   0  "CBR4"
  $node_leader addModule 7 $udp_leader_sos   0  "UDP2"
  $node_leader addModule 6 $ipr_leader_sos   0  "IPR2"
  $node_leader addModule 5 $ipif_leader_sos  2  "IPF2" 

  $node_leader addModule 4 $ctr_leader   2  "CTR"   

  $node_leader addModule 3 $mll_leader   2  "MLL_LF"
  $node_leader addModule 2 $mac_leader   2  "MAC_LF"
  $node_leader addModule 1 $phy_leader   0  "PHY_LF"

  $node_leader addModule 3 $mll_hf_leader   2  "MLL_HF"
  $node_leader addModule 2 $mac_hf_leader   2  "MAC_HF"
  $node_leader addModule 1 $phy_hf_leader   0  "PHY_HF"

  $node_leader addModule 3 $mll_op_leader   2  "MLL_OP"
  $node_leader addModule 2 $mac_op_leader   2  "MAC_OP"
  $node_leader addModule 1 $phy_op_leader   0  "PHY_OP"

  $node_leader setConnection $cbr_leader   $udp_leader   0
  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    $node_leader setConnection $cbr2_leader($id1)  $udp_leader   0
  }
  $node_leader setConnection $cbr3_leader  $udp_leader   0
  $node_leader setConnection $udp_leader   $ipr_leader   0
  $node_leader setConnection $ipr_leader   $ipif_leader  2
  $node_leader setConnection $ipif_leader  $ctr_leader   2

  $node_leader setConnection $cbr4_leader  $udp_leader_sos   0
  $node_leader setConnection $udp_leader_sos  $ipr_leader_sos   0
  $node_leader setConnection $ipr_leader_sos  $ipif_leader_sos  2
  $node_leader setConnection $ipif_leader_sos $ctr_leader   2

  $node_leader setConnection $ctr_leader   $mll_leader   2
  $node_leader setConnection $mll_leader   $mac_leader   2
  $node_leader setConnection $mac_leader   $phy_leader   2
  $node_leader addToChannel  $channel  $phy_leader   0

  $node_leader setConnection $ctr_leader   $mll_hf_leader   2
  $node_leader setConnection $mll_hf_leader  $mac_hf_leader   2
  $node_leader setConnection $mac_hf_leader  $phy_hf_leader   2
  $node_leader addToChannel  $channel  $phy_hf_leader   0

  $node_leader setConnection $ctr_leader   $mll_op_leader   2
  $node_leader setConnection $mll_op_leader  $mac_op_leader   2
  $node_leader setConnection $mac_op_leader  $phy_op_leader   2
  $node_leader addToChannel  $channel_op  $phy_op_leader   0

  set portnum_leader [$udp_leader assignPort $cbr_leader ]

  for {set id1 0} {$id1 <= $opt(n_diver)} {incr id1} {
    set portnum_leader_sos($id1) [$udp_leader assignPort $cbr2_leader($id1) ]
  }
  set portnum3_leader [$udp_leader assignPort $cbr3_leader ]
  set portnum4_leader [$udp_leader_sos assignPort $cbr4_leader ]
  if {$id > 254} {
    puts "hostnum > 254!!! exiting"
    exit
  }
  set tmp_ [expr $id + 1]
  $ipif_leader addr $tmp_
  $ipif_leader_sos addr $tmp_
  $ipr_leader_sos addr $tmp_

  WOSS/Position/WayPoint set time_threshold_      [expr 1.0 / $opt(speed_rov)]  
  set position_leader [new "WOSS/Position/WayPoint"]
  $node_leader addPosition $position_leader
  set posdb_leader [new "PlugIn/PositionDB"]
  $node_leader addPlugin $posdb_leader 20 "PDB"
  $posdb_leader addpos [$ipif_leader addr] $position_leader
  
  set interf_data [new "Module/UW/INTERFERENCE"]
  $interf_data set maxinterval_ $opt(maxinterval_)

  set interf_data_hf [new "Module/UW/INTERFERENCE"]
  $interf_data_hf set maxinterval_ $opt(maxinterval_)

  set interf_data_op [new "MInterference/MIV"]
  $interf_data_op set maxinterval_ $opt(maxinterval_)

  $phy_leader setPropagation $propagation  
  $phy_leader setSpectralMask $data_mask
  $phy_leader setInterference $interf_data
  # $phy_leader setInterferenceModel "MEANPOWER"

  $phy_hf_leader setPropagation $propagation  
  $phy_hf_leader setSpectralMask $data_mask_hf
  $phy_hf_leader setInterference $interf_data_hf

  $phy_op_leader setPropagation $propagation_op  
  $phy_op_leader setSpectralMask $data_mask_op
  $phy_op_leader setInterference $interf_data_op
  $phy_op_leader useLUT

  $mac_leader $opt(ack_mode)
  $mac_leader setMacAddr $tmp_
  $mac_leader initialize

  $mac_hf_leader setMacAddr $tmp_
  $mac_hf_leader $opt(ack_mode)
  $mac_hf_leader initialize
  #$mac_hf_leader setSlotNumber $tmp_


  $mac_op_leader setMacAddr $tmp_
  $mac_op_leader $opt(ack_mode)
  $mac_op_leader initialize

#   HEALTH TRAFFIC
  $ctr_leader addRobustLowLayer 1  "MLL_LF"
  # $ctr_leader addFastLowLayer 1   "MLL_OP"
  $ctr_leader addUpLayer 1     "IPF1"

#   CONTROL TRAFFIC
  $ctr_leader addRobustLowLayer 2  "MLL_LF"
  # $ctr_leader addFastLowLayer 1   "MLL_HF"
  $ctr_leader addUpLayer 2     "IPF1"

#   IMAGE TRAFFIC
  $ctr_leader addFastLowLayer 3   "MLL_OP"
  $ctr_leader addUpLayer 3      "IPF1"

#   SOS TRAFFIC: to all (no spec needed for lower)
  $ctr_leader addUpLayer 4      "IPF2"
}