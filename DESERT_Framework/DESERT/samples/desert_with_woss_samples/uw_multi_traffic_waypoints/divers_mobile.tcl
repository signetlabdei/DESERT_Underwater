set scr [info script]
proc createDiver { id } {

  global ns cbr_diver cbr2_diver cbr3_diver cbr4_diver position_diver node_diver udp portnum_diver portnum_diver_sos ipr_diver ipif_diver ipr_diver_sos ipif_diver_sos
  global channel propagation data_mask data_mask_hf phy_diver posdb_diver opt rvposx mll_diver mll_hf_diver mac_diver mac_hf_diver 
  global db_manager node_diver_coordinates woss_utilities ipif_diver_image ipr_diver_image defaultRNG
  
  # TRAFFICO 1: HEALTH MONITORING DIVERS --> LEADER, LEADER --> ROV
  set node_diver($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
  Module/UW/CBR set packetSize_      $opt(pktsize_health)
  Module/UW/CBR set period_        $opt(cbr_period_health)
  Module/UW/CBR set PoissonTraffic_    0
  Module/UW/CBR set traffic_type_ 1
  set cbr_diver($id)  [new Module/UW/CBR]

  # TRAFFICO 2: CONTROL: LEADER --> DIVERS, LEADER --> ROV via long range in CSMA
  Module/UW/CBR set packetSize_      $opt(pktsize_control)
  Module/UW/CBR set period_        $opt(cbr_period_control)
  Module/UW/CBR set PoissonTraffic_    1
  Module/UW/CBR set traffic_type_ 2
  set cbr2_diver($id)  [new Module/UW/CBR]

  # TRAFFICO 3: IMMAGINI: DIVERS --> LEADER, LEADER --> ROV
  Module/UW/CBR set packetSize_      $opt(pktsize_image)
  Module/UW/CBR set period_        $opt(cbr_period_image)
  Module/UW/CBR set PoissonTraffic_    1
  Module/UW/CBR set traffic_type_ 3
  set cbr3_diver($id)  [new Module/UW/CBR]

  # TRAFFICO 4: SOS: broadcast
  Module/UW/CBR set packetSize_      $opt(pktsize_sos)
  Module/UW/CBR set period_        $opt(cbr_period_sos)
  Module/UW/CBR set PoissonTraffic_    1
  Module/UW/CBR set traffic_type_ 4
  set cbr4_diver($id)  [new Module/UW/CBR]

  set udp_diver($id)  [new Module/UW/UDP]
  set ipr_diver($id)  [new Module/UW/StaticRouting]
  set ipif_diver($id) [new Module/UW/IP]

  set udp_diver_image($id)  [new Module/UW/UDP]
  set ipr_diver_image($id)  [new Module/UW/StaticRouting]
  set ipif_diver_image($id) [new Module/UW/IP]

  set udp_diver_sos($id)  [new Module/UW/UDP]
  set ipr_diver_sos($id)  [new Module/UW/FLOODING]
  set ipif_diver_sos($id) [new Module/UW/IP]


  Module/UW/MULTI_TRAFFIC_RANGE_CTR set check_to_period_  10
  Module/UW/MULTI_TRAFFIC_RANGE_CTR set debug_      0
  set ctr_diver($id)  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]

  set mll_diver($id)  [new Module/UW/MLL] 
  set mac_diver($id)  [new Module/UW/TDMA] 
  Module/UW/PHYSICAL  set BitRate_          $opt(bitrate)
  Module/UW/PHYSICAL  set MaxTxSPL_dB_        $opt(txpower)
  set phy_diver($id)  [new Module/UW/PHYSICAL]
  $mll_diver($id) setstackid 1

  set mll_hf_diver($id)  [new Module/UW/MLL] 
  set mac_hf_diver($id)  [new Module/UW/CSMA_ALOHA]
  Module/UW/PHYSICAL  set BitRate_          $opt(bitrate_hf)
  Module/UW/PHYSICAL  set MaxTxSPL_dB_        $opt(txpower_hf)
  set phy_hf_diver($id)  [new Module/UW/PHYSICAL]
  $mll_hf_diver($id) setstackid 2

  $node_diver($id) addModule 8 $cbr_diver($id)   0  "CBR1"
  $node_diver($id) addModule 8 $cbr2_diver($id)  0  "CBR2"
  $node_diver($id) addModule 7 $udp_diver($id)   0  "UDP1"
  $node_diver($id) addModule 6 $ipr_diver($id)   0  "IPR1"
  $node_diver($id) addModule 5 $ipif_diver($id)  2  "IPF1"   

  $node_diver($id) addModule 8 $cbr3_diver($id)  0  "CBR3"  
  $node_diver($id) addModule 7 $udp_diver_image($id)   0  "UDP2"
  $node_diver($id) addModule 6 $ipr_diver_image($id)   0  "IPR2"
  $node_diver($id) addModule 5 $ipif_diver_image($id)  2  "IPF2" 
  
  $node_diver($id) addModule 8 $cbr4_diver($id)   0  "CBR4"
  $node_diver($id) addModule 7 $udp_diver_sos($id)   0  "UDP4"
  $node_diver($id) addModule 6 $ipr_diver_sos($id)   0  "IPR4"
  $node_diver($id) addModule 5 $ipif_diver_sos($id)  2  "IPF4" 

  $node_diver($id) addModule 4 $ctr_diver($id)   2  "CTR"   

  $node_diver($id) addModule 3 $mll_diver($id)   2  "MLL_LF"
  $node_diver($id) addModule 2 $mac_diver($id)   2  "MAC_LF"
  $node_diver($id) addModule 1 $phy_diver($id)   0  "PHY_LF"

  $node_diver($id) addModule 3 $mll_hf_diver($id)   2  "MLL_HF"
  $node_diver($id) addModule 2 $mac_hf_diver($id)   2  "MAC_HF"
  $node_diver($id) addModule 1 $phy_hf_diver($id)   0  "PHY_HF"

  $node_diver($id) setConnection $cbr_diver($id)   $udp_diver($id)   0
  $node_diver($id) setConnection $cbr2_diver($id)  $udp_diver($id)   0
  $node_diver($id) setConnection $udp_diver($id)   $ipr_diver($id)   0
  $node_diver($id) setConnection $ipr_diver($id)   $ipif_diver($id)  2
  $node_diver($id) setConnection $ipif_diver($id)  $ctr_diver($id)   2

  $node_diver($id) setConnection $cbr3_diver($id)        $udp_diver_image($id)   0
  $node_diver($id) setConnection $udp_diver_image($id)   $ipr_diver_image($id)   0
  $node_diver($id) setConnection $ipr_diver_image($id)   $ipif_diver_image($id)  2
  $node_diver($id) setConnection $ipif_diver_image($id)  $ctr_diver($id)         2

  $node_diver($id) setConnection $cbr4_diver($id)  $udp_diver_sos($id)   0
  $node_diver($id) setConnection $udp_diver_sos($id)  $ipr_diver_sos($id)   0
  $node_diver($id) setConnection $ipr_diver_sos($id)  $ipif_diver_sos($id)  2
  $node_diver($id) setConnection $ipif_diver_sos($id) $ctr_diver($id)   2

  $node_diver($id) setConnection $ctr_diver($id)   $mll_diver($id)   2
  $node_diver($id) setConnection $mll_diver($id)   $mac_diver($id)   2
  $node_diver($id) setConnection $mac_diver($id)   $phy_diver($id)   2
  $node_diver($id) addToChannel  $channel  $phy_diver($id)   0

  $node_diver($id) setConnection $ctr_diver($id)   $mll_hf_diver($id)   2
  $node_diver($id) setConnection $mll_hf_diver($id)  $mac_hf_diver($id)   2
  $node_diver($id) setConnection $mac_hf_diver($id)  $phy_hf_diver($id)   2
  $node_diver($id) addToChannel  $channel  $phy_hf_diver($id)   0

  set portnum_diver($id) [$udp_diver($id) assignPort $cbr_diver($id) ]
  set portnum_diver_sos($id) [$udp_diver($id) assignPort $cbr2_diver($id) ]
  set portnum3_diver($id) [$udp_diver_image($id) assignPort $cbr3_diver($id) ]
  set portnum4_diver($id) [$udp_diver_sos($id) assignPort $cbr4_diver($id) ]
  if {$id > 254} {
  puts "hostnum > 254!!! exiting"
  exit
  }
  set tmp_ [expr ($id) + 1]
  $ipif_diver($id) addr $tmp_
  $ipif_diver_image($id) addr $tmp_
  $ipif_diver_sos($id) addr $tmp_
  $ipr_diver_sos($id) addr $tmp_
  
  WOSS/Position/WayPoint set time_threshold_      [expr 1.0 / $opt(speed_diver)]  
  set position_diver($id) [new "WOSS/Position/WayPoint"]
  $node_diver($id) addPosition $position_diver($id)
  set posdb_diver($id) [new "PlugIn/PositionDB"]
  $node_diver($id) addPlugin $posdb_diver($id) 20 "PDB"
  $posdb_diver($id) addpos [$ipif_diver($id) addr] $position_diver($id)
  
  set interf_data($id) [new "Module/UW/INTERFERENCE"]
  $interf_data($id) set maxinterval_ $opt(maxinterval_)

  set interf_data_hf($id) [new "Module/UW/INTERFERENCE"]
  $interf_data_hf($id) set maxinterval_ $opt(maxinterval_)

  set interf_data_op($id) [new "MInterference/MIV"]
  $interf_data_op($id) set maxinterval_ $opt(maxinterval_)

  $phy_diver($id) setPropagation $propagation  
  $phy_diver($id) setSpectralMask $data_mask
  $phy_diver($id) setInterference $interf_data($id)
  # $phy_diver($id) setInterferenceModel "MEANPOWER"

  $phy_hf_diver($id) setPropagation $propagation  
  $phy_hf_diver($id) setSpectralMask $data_mask_hf
  $phy_hf_diver($id) setInterference $interf_data_hf($id)

  $mac_diver($id) setMacAddr $tmp_
  $mac_diver($id) setSlotNumber [expr $id + 1]

  $mac_hf_diver($id) setMacAddr $tmp_
  $mac_hf_diver($id) $opt(ack_mode)
  $mac_hf_diver($id) initialize

  #HEALTH 
  $ctr_diver($id) addRobustLowLayer 1  "MLL_LF"
  $ctr_diver($id) addUpLayer 1         "IPF1"

  #SENDING CONTROL IS NOT PERMITTED, cbr will not start
  $ctr_diver($id) addRobustLowLayer 2  "MLL_LF"
  $ctr_diver($id) addUpLayer 2         "IPF1"

  #IMAGES WITH FAST ACOUSTIC
  # $ctr_diver($id) addRobustLowLayer 3   "MLL_LF"
  $ctr_diver($id) addRobustLowLayer 3   "MLL_HF"
  # $ctr_diver($id) addFastLowLayer 3   "MLL_HF"
  $ctr_diver($id) addUpLayer 3        "IPF1"

  #SOS
  $ctr_diver($id) addUpLayer 4        "IPF4"
  # $ctr_diver($id) addFastLowLayer 4   "MLL_HF"
}