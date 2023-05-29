set scr [info script]
proc createSUV { id } {

    global ns suv_app suv_err position_suv node_suv udp_suv  
    global portnum_suv portnum2_suv ipr_suv ipif_suv 
    global channel channel_op propagation propagation_op data_mask data_mask_op
    global phy_suv posdb_suv opt rvposx mll_suv mll_op_suv mac_suv
    global mac_op_suv db_manager node_suv_coordinates
    
    set node_suv [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    # TRAFFICO 2: CONTROL: SUV --> AUV
    Module/UW/AUV/CTR set packetSize_          $opt(pktsize)
    Module/UW/AUV/CTR set period_              $opt(ctr_period)
    Module/UW/AUV/CTR set PoissonTraffic_      1
    Module/UW/AUV/CTR set traffic_type_ 2
    Module/UW/AUV/CTR set debug_ 0
    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        set suv_app($id1)  [new Module/UW/AUV/CTR]
    }

    # TRAFFICO 3: ERROR: AUV --> SUV
    Module/UW/AUV/CER set packetSize_          $opt(pktsize)
    Module/UW/AUV/CER set period_              $opt(ctr_period)
    Module/UW/AUV/CER set PoissonTraffic_      1
    Module/UW/AUV/CER set traffic_type_ 3
    Module/UW/AUV/CER set debug_ 1
    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        set suv_err($id1)  [new Module/UW/AUV/CER]
    }

    set udp_suv  [new Module/UW/UDP]
    set ipr_suv  [new Module/UW/StaticRouting]
    set ipif_suv [new Module/UW/IP]

    set ctr_suv  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]

    set mll_suv  [new Module/UW/MLL] 
    set mac_suv  [new Module/UW/CSMA_ALOHA] 
    Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
    Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
    set phy_suv  [new Module/UW/PHYSICAL]
    $mll_suv setstackid 1

    set mll_op_suv  [new Module/UW/MLL] 
    Module/UW/CSMA_ALOHA set listen_time_          [expr 1.0e-12]
    Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-12]
    set mac_op_suv  [new Module/UW/CSMA_ALOHA]
    set phy_op_suv  [new Module/UW/OPTICAL/PHY]

    #$node_suv addModule 8 $cbr_suv   0  "CBR1"
    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        #$node_suv addModule 8 $cbr_suv($id1)   0  "CBR1"
        $node_suv addModule 8 $suv_app($id1)  0  "CBR1"
        $node_suv addModule 8 $suv_err($id1)  0  "CBR2"
    }
    #$node_suv addModule 8 $cbr3_suv  0  "CBR3"
    $node_suv addModule 7 $udp_suv   0  "UDP1"
    $node_suv addModule 6 $ipr_suv   0  "IPR1"
    $node_suv addModule 5 $ipif_suv  2  "IPF1"   

    $node_suv addModule 4 $ctr_suv   2  "CTR"   

    $node_suv addModule 3 $mll_suv   2  "MLL_LF"
    $node_suv addModule 2 $mac_suv   2  "MAC_LF"
    $node_suv addModule 1 $phy_suv   0  "PHY_LF"

    $node_suv addModule 3 $mll_op_suv   2  "MLL_OP"
    $node_suv addModule 2 $mac_op_suv   2  "MAC_OP"
    $node_suv addModule 1 $phy_op_suv   0  "PHY_OP"

    #$node_suv setConnection $cbr_suv   $udp_suv   0
    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        #$node_suv setConnection $cbr_suv($id1)   $udp_suv   0
        $node_suv setConnection $suv_app($id1)  $udp_suv   0
        $node_suv setConnection $suv_err($id1)  $udp_suv   0
    }
    #$node_suv setConnection $cbr3_suv  $udp_suv   0
    $node_suv setConnection $udp_suv   $ipr_suv   0
    $node_suv setConnection $ipr_suv   $ipif_suv  2
    $node_suv setConnection $ipif_suv  $ctr_suv   2

    $node_suv setConnection $ctr_suv   $mll_suv   2
    $node_suv setConnection $mll_suv   $mac_suv   2
    $node_suv setConnection $mac_suv   $phy_suv   2
    $node_suv addToChannel  $channel   $phy_suv   0

    $node_suv setConnection $ctr_suv   $mll_op_suv   2
    $node_suv setConnection $mll_op_suv  $mac_op_suv   2
    $node_suv setConnection $mac_op_suv  $phy_op_suv   2
    $node_suv addToChannel  $channel_op    $phy_op_suv   0

    

    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        #set portnum_suv($id1) [$udp_suv assignPort $cbr_suv($id1) ]
        set portnum_suv($id1) [$udp_suv assignPort $suv_app($id1) ]
        set portnum2_suv($id1) [$udp_suv assignPort $suv_err($id1) ]
    }
    #set portnum3_suv [$udp_suv assignPort $cbr3_suv ]

    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }
    set tmp_ [expr $id + 1]
    $ipif_suv addr $tmp_
    
    set position_suv [new "Position/BM"]
    $node_suv addPosition $position_suv
    set posdb_suv [new "PlugIn/PositionDB"]
    $node_suv addPlugin $posdb_suv 20 "PDB"
    $posdb_suv addpos [$ipif_suv addr] $position_suv
    
    set interf_data [new "Module/UW/INTERFERENCE"]
    $interf_data set maxinterval_ $opt(maxinterval_)
    $interf_data set debug_       0

    set interf_data_op [new "MInterference/MIV"]
    $interf_data_op set maxinterval_ $opt(maxinterval_)
    $interf_data_op set debug_       0

    $phy_suv setPropagation $propagation    
    $phy_suv setSpectralMask $data_mask
    $phy_suv setInterference $interf_data

    $phy_op_suv setPropagation $propagation_op    
    $phy_op_suv setSpectralMask $data_mask_op
    $phy_op_suv setInterference $interf_data_op
    # $phy_op_leader useLUT

    $mac_suv $opt(ack_mode)
    $mac_suv setMacAddr $tmp_
    $mac_suv initialize


    $mac_op_suv setMacAddr $tmp_
    $mac_op_suv $opt(ack_mode)
    $mac_op_suv initialize

#   MONITORING TRAFFIC
    $ctr_suv addRobustLowLayer 1  "MLL_LF"
    $ctr_suv addFastLowLayer 1     "MLL_OP"
    $ctr_suv addUpLayer 1         "IPF1"
    $ctr_suv setBufferFeatures 1 1000 0

#   CONTROL TRAFFIC
    $ctr_suv addRobustLowLayer 2  "MLL_LF"
    $ctr_suv addFastLowLayer 2     "MLL_OP"
    $ctr_suv addUpLayer 2         "IPF1"
    $ctr_suv setBufferFeatures 2 1000 0

#   #ERROR TRAFFIC
    $ctr_suv addRobustLowLayer 3  "MLL_LF"
    $ctr_suv addFastLowLayer 3     "MLL_OP"
    $ctr_suv addUpLayer 3         "IPF1"
    $ctr_suv setBufferFeatures 3 1000 0

}