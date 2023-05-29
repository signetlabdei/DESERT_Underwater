set scr [info script]
proc createAUV { id } {

    global ns auv_app auv_err position_auv node_auv udp portnum_auv portnum2_auv ipr_auv ipif_auv
    global channel propagation propagation_op data_mask data_mask_op phy_auv posdb_auv opt rvposx mll_auv mll_op_auv mac_auv mac_op_auv 
    global db_manager node_auv_coordinates
    
    # TRAFFICO 1: MONITORING AUV --> SUV
    set node_auv($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
    Module/UW/AUV set packetSize_          $opt(pktsize_monitoring)
    Module/UW/AUV set period_              $opt(auv_period)
    Module/UW/AUV set PoissonTraffic_      1
    Module/UW/AUV set traffic_type_ 1
    Module/UW/AUV set debug_ 0
    set auv_app($id)  [new Module/UW/AUV]

    # TRAFFICO 2: CONTROL:SUV --> AUV
    #Module/UW/ROV/CTR set packetSize_          $opt(pktsize_control)
    #Module/UW/ROV/CTR set period_              $opt(cbr_period_control)
    #Module/UW/ROV/CTR set PoissonTraffic_      1
    #Module/UW/ROV/CTR set traffic_type_ 2
    # Module/UW/CBR set debug_ 0
    #set cbr2_auv($id)  [new Module/UW/ROV/CTR]

    # TRAFFICO 3: ERROR AUV --> SUV
    set node_auv($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 
    Module/UW/AUV/ERR set packetSize_          $opt(pktsize)
    Module/UW/AUV/ERR set period_              $opt(auv_period)
    Module/UW/AUV/ERR set PoissonTraffic_      1
    Module/UW/AUV/ERR set traffic_type_ 3
    Module/UW/AUV/ERR set debug_ 1
    set auv_err($id)  [new Module/UW/AUV/ERR]


    set udp_auv($id)  [new Module/UW/UDP]
    set ipr_auv($id)  [new Module/UW/StaticRouting]
    set ipif_auv($id) [new Module/UW/IP]

    Module/UW/MULTI_TRAFFIC_RANGE_CTR set debug_            0
    set ctr_auv($id)  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]


    Module/UW/TDMA set frame_duration   $opt(tdma_frame)
    Module/UW/TDMA set fair_mode        1   
    Module/UW/TDMA set guard_time       $opt(tdma_gard)
    Module/UW/TDMA set tot_slots        $opt(n_auv)
    Module/UW/TDMA set debug_ 0
    Module/UW/TDMA set queue_size_ 100

    Module/UW/CSMA_ALOHA set listen_time_          [expr 1.0e-12]
    Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-12]
    Module/UW/CSMA_ALOHA set debug_ 0

    set mll_auv($id)  [new Module/UW/MLL] 
    set mac_auv($id)  [new Module/UW/CSMA_ALOHA] 
    Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
    Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
    set phy_auv($id)  [new Module/UW/PHYSICAL]
    $mll_auv($id) setstackid 1

    set mll_op_auv($id)  [new Module/UW/MLL] 
    Module/UW/CSMA_ALOHA set listen_time_          [expr 1.0e-12]
    Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-12]
    Module/UW/CSMA_ALOHA set debug_ 0
    set mac_op_auv($id)  [new Module/UW/CSMA_ALOHA]
    set phy_op_auv($id)  [new Module/UW/OPTICAL/PHY]

    $node_auv($id) addModule 8 $auv_app($id)   0  "CBR1"
    $node_auv($id) addModule 8 $auv_err($id)  0  "CBR2"
    #$node_auv($id) addModule 8 $cbr3_auv($id)  0  "CBR3"
    $node_auv($id) addModule 7 $udp_auv($id)   0  "UDP1"
    $node_auv($id) addModule 6 $ipr_auv($id)   0  "IPR1"
    $node_auv($id) addModule 5 $ipif_auv($id)  2  "IPF1"   
    
    #$node_diver($id) addModule 7 $udp2_diver($id)   0  "UDP2"
    #$node_diver($id) addModule 6 $ipr2_diver($id)   0  "IPR2"
    #$node_diver($id) addModule 5 $ipif2_diver($id)  2  "IPF2" 

    $node_auv($id) addModule 4 $ctr_auv($id)   2  "CTR"   

    $node_auv($id) addModule 3 $mll_auv($id)   2  "MLL_LF"
    $node_auv($id) addModule 2 $mac_auv($id)   2  "MAC_LF"
    $node_auv($id) addModule 1 $phy_auv($id)   0  "PHY_LF"

    $node_auv($id) addModule 3 $mll_op_auv($id)   2  "MLL_OP"
    $node_auv($id) addModule 2 $mac_op_auv($id)   2  "MAC_OP"
    $node_auv($id) addModule 1 $phy_op_auv($id)   0  "PHY_OP"

    $node_auv($id) setConnection $auv_app($id)   $udp_auv($id)   0
    $node_auv($id) setConnection $auv_err($id)  $udp_auv($id)   0
    #$node_auv($id) setConnection $cbr3_auv($id)  $udp_auv($id)   0
    $node_auv($id) setConnection $udp_auv($id)   $ipr_auv($id)   0
    $node_auv($id) setConnection $ipr_auv($id)   $ipif_auv($id)  2
    $node_auv($id) setConnection $ipif_auv($id)  $ctr_auv($id)   2

    $node_auv($id) setConnection $ctr_auv($id)   $mll_auv($id)   2
    $node_auv($id) setConnection $mll_auv($id)   $mac_auv($id)   2
    $node_auv($id) setConnection $mac_auv($id)   $phy_auv($id)   2
    $node_auv($id) addToChannel  $channel    $phy_auv($id)   0

    $node_auv($id) setConnection $ctr_auv($id)   $mll_op_auv($id)   2
    $node_auv($id) setConnection $mll_op_auv($id)  $mac_op_auv($id)   2
    $node_auv($id) setConnection $mac_op_auv($id)  $phy_op_auv($id)   2
    $node_auv($id) addToChannel  $channel    $phy_op_auv($id)   0

    set portnum_auv($id) [$udp_auv($id) assignPort $auv_app($id) ]
    set portnum2_auv($id) [$udp_auv($id) assignPort $auv_err($id) ]
    #set portnum3_auv($id) [$udp_auv($id) assignPort $cbr3_auv($id) ]
    if {$id > 254} {
    puts "hostnum > 254!!! exiting"
    exit
    }
    set tmp_ [expr ($id) + 1]
    $ipif_auv($id) addr $tmp_
    
    set position_auv($id) [new "Position/UWSM"] ; #Position/BM (?)
    $node_auv($id) addPosition $position_auv($id)
    #set posdb_auv($id) [new "PlugIn/PositionDB"]
    #$node_auv($id) addPlugin $posdb_auv($id) 20 "PDB"
    #$posdb_auv($id) addpos [$ipif_auv($id) addr] $position_auv($id)
    
    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0


    set interf_data_op($id) [new "MInterference/MIV"]
    $interf_data_op($id) set maxinterval_ $opt(maxinterval_)
    $interf_data_op($id) set debug_       0

    $phy_auv($id) setPropagation $propagation    
    $phy_auv($id) setSpectralMask $data_mask
    $phy_auv($id) setInterference $interf_data($id)

    $phy_op_auv($id) setPropagation $propagation_op    
    $phy_op_auv($id) setSpectralMask $data_mask_op
    $phy_op_auv($id) setInterference $interf_data_op($id)
    #$phy_hf_diver($id) setInterference $interf_data_hf($id)

    $mac_auv($id) setMacAddr $tmp_
    #$mac_auv($id) setSlotNumber [expr $id + 1]
    $mac_auv($id) $opt(ack_mode)
    $mac_auv($id) initialize

    $mac_op_auv($id) setMacAddr $tmp_
    $mac_op_auv($id) $opt(ack_mode)
    $mac_op_auv($id) initialize

    #MONITORING 
    $ctr_auv($id) addRobustLowLayer 1  "MLL_LF"
    $ctr_auv($id) addFastLowLayer 1     "MLL_OP"
    $ctr_auv($id) addUpLayer 1         "IPF1"
    $ctr_auv($id) setBufferFeatures 1 1000 0

    #CONTROL 
    $ctr_auv($id) addRobustLowLayer 2  "MLL_LF"
    $ctr_auv($id) addFastLowLayer 2     "MLL_OP"
    $ctr_auv($id) addUpLayer 2         "IPF1"
    $ctr_auv($id) setBufferFeatures 2 1000 0

    #ERROR
    $ctr_auv($id) addRobustLowLayer 3  "MLL_LF"
    $ctr_auv($id) addFastLowLayer 3     "MLL_OP"
    $ctr_auv($id) addUpLayer 3         "IPF1"
    $ctr_auv($id) setBufferFeatures 3 1000 0

}