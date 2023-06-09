set scr [info script]
proc createASV { id } {

    global ns asv_app asv_err position_asv node_asv udp_asv  
    global portnum_asv portnum2_asv ipr_asv ipif_asv 
    global channel channel_op propagation propagation_op data_mask data_mask_op
    global phy_asv posdb_asv opt rvposx mll_asv mll_op_asv mac_asv
    global mac_op_asv db_manager node_asv_coordinates
    
    set node_asv [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    # TRAFFICO 2: CONTROL: ASV --> AUV
    Module/UW/AUV/CTR set packetSize_          $opt(pktsize)
    Module/UW/AUV/CTR set period_              $opt(ctr_period)
    Module/UW/AUV/CTR set PoissonTraffic_      1
    Module/UW/AUV/CTR set traffic_type_ 2
    Module/UW/AUV/CTR set debug_ 0

    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        set asv_app($id1)  [new Module/UW/AUV/CTR]
    }

    # TRAFFICO 3: ERROR: AUV --> ASV
    Module/UW/AUV/CER set packetSize_          $opt(pktsize)
    Module/UW/AUV/CER set period_              500
    Module/UW/AUV/CER set PoissonTraffic_      1
    Module/UW/AUV/CER set traffic_type_ 3
    Module/UW/AUV/CER set debug_                0
    Module/UW/AUV/CER set log_flag_            1
    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        set asv_err($id1)  [new Module/UW/AUV/CER]
    }

    set udp_asv  [new Module/UW/UDP]
    set ipr_asv  [new Module/UW/StaticRouting]
    set ipif_asv [new Module/UW/IP]

    set ctr_asv  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]

    set mll_asv  [new Module/UW/MLL] 
    set mac_asv  [new Module/UW/CSMA_ALOHA] 
    Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
    Module/UW/PHYSICAL  set MaxTxSPL_dB_                $opt(txpower)
    set phy_asv  [new Module/UW/PHYSICAL]
    $mll_asv setstackid 1

    set mll_op_asv  [new Module/UW/MLL] 
    Module/UW/CSMA_ALOHA set listen_time_          [expr 1.0e-12]
    Module/UW/CSMA_ALOHA set wait_costant_         [expr 1.0e-12]
    set mac_op_asv  [new Module/UW/CSMA_ALOHA]
    set phy_op_asv  [new Module/UW/OPTICAL/PHY]

    #$node_asv addModule 8 $cbr_asv   0  "CBR1"
    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        #$node_asv addModule 8 $cbr_asv($id1)   0  "CBR1"
        $node_asv addModule 8 $asv_app($id1)  0  "CBR1"
        $node_asv addModule 8 $asv_err($id1)  0  "CBR2"
    }
    #$node_asv addModule 8 $cbr3_asv  0  "CBR3"
    $node_asv addModule 7 $udp_asv   0  "UDP1"
    $node_asv addModule 6 $ipr_asv   0  "IPR1"
    $node_asv addModule 5 $ipif_asv  2  "IPF1"   

    $node_asv addModule 4 $ctr_asv   2  "CTR"   

    $node_asv addModule 3 $mll_asv   2  "MLL_LF"
    $node_asv addModule 2 $mac_asv   2  "MAC_LF"
    $node_asv addModule 1 $phy_asv   0  "PHY_LF"

    $node_asv addModule 3 $mll_op_asv   2  "MLL_OP"
    $node_asv addModule 2 $mac_op_asv   2  "MAC_OP"
    $node_asv addModule 1 $phy_op_asv   0  "PHY_OP"

    #$node_asv setConnection $cbr_asv   $udp_asv   0
    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        #$node_asv setConnection $cbr_asv($id1)   $udp_asv   0
        $node_asv setConnection $asv_app($id1)  $udp_asv   0
        $node_asv setConnection $asv_err($id1)  $udp_asv   0
    }
    #$node_asv setConnection $cbr3_asv  $udp_asv   0
    $node_asv setConnection $udp_asv   $ipr_asv   0
    $node_asv setConnection $ipr_asv   $ipif_asv  2
    $node_asv setConnection $ipif_asv  $ctr_asv   2

    $node_asv setConnection $ctr_asv   $mll_asv   2
    $node_asv setConnection $mll_asv   $mac_asv   2
    $node_asv setConnection $mac_asv   $phy_asv   2
    $node_asv addToChannel  $channel   $phy_asv   0

    $node_asv setConnection $ctr_asv   $mll_op_asv   2
    $node_asv setConnection $mll_op_asv  $mac_op_asv   2
    $node_asv setConnection $mac_op_asv  $phy_op_asv   2
    $node_asv addToChannel  $channel_op    $phy_op_asv   0

    

    for {set id1 0} {$id1 <= $opt(n_auv)} {incr id1} {
        #set portnum_asv($id1) [$udp_asv assignPort $cbr_asv($id1) ]
        set portnum_asv($id1) [$udp_asv assignPort $asv_app($id1) ]
        set portnum2_asv($id1) [$udp_asv assignPort $asv_err($id1) ]
    }
    #set portnum3_asv [$udp_asv assignPort $cbr3_asv ]

    if {$id > 254} {
        puts "hostnum > 254!!! exiting"
        exit
    }
    set tmp_ [expr $id + 1]
    $ipif_asv addr $tmp_
    
    set position_asv [new "Position/BM"]
    $node_asv addPosition $position_asv
    set posdb_asv [new "PlugIn/PositionDB"]
    $node_asv addPlugin $posdb_asv 20 "PDB"
    $posdb_asv addpos [$ipif_asv addr] $position_asv
    
    set interf_data [new "Module/UW/INTERFERENCE"]
    $interf_data set maxinterval_ $opt(maxinterval_)
    $interf_data set debug_       0

    set interf_data_op [new "MInterference/MIV"]
    $interf_data_op set maxinterval_ $opt(maxinterval_)
    $interf_data_op set debug_       0

    $phy_asv setPropagation $propagation    
    $phy_asv setSpectralMask $data_mask
    $phy_asv setInterference $interf_data

    $phy_op_asv setPropagation $propagation_op    
    $phy_op_asv setSpectralMask $data_mask_op
    $phy_op_asv setInterference $interf_data_op
    # $phy_op_leader useLUT

    $mac_asv $opt(ack_mode)
    $mac_asv setMacAddr $tmp_
    $mac_asv initialize


    $mac_op_asv setMacAddr $tmp_
    $mac_op_asv $opt(ack_mode)
    $mac_op_asv initialize

#   MONITORING TRAFFIC
    $ctr_asv addRobustLowLayer 1  "MLL_LF"
    $ctr_asv addFastLowLayer 1     "MLL_OP"
    $ctr_asv addUpLayer 1         "IPF1"
    $ctr_asv setBufferFeatures 1 1000 0

#   CONTROL TRAFFIC
    $ctr_asv addRobustLowLayer 2  "MLL_LF"
    $ctr_asv addFastLowLayer 2     "MLL_OP"
    $ctr_asv addUpLayer 2         "IPF1"
    $ctr_asv setBufferFeatures 2 1000 0

#   #ERROR TRAFFIC
    $ctr_asv addRobustLowLayer 3  "MLL_LF"
    $ctr_asv addFastLowLayer 3     "MLL_OP"
    $ctr_asv addUpLayer 3         "IPF1"
    $ctr_asv setBufferFeatures 3 1000 0

}