################################
# Procedure(s) to create nodes #
################################
proc createNode {id} { 
	global ns node opt data_mask_bluec data_mask_evo_hs data_mask_subn_mf
	global propagation_op channel_op propagation_ac channel_ac
	global app_jp_ctr app_jp_ctr_ROV app_wp_ctr app_monit app_video_hd
	global app_video_gs app_video_vlq app_image_hd
	global udp_jp_ctr udp_wp_ctr udp_video_hd udp_video_gs udp_video_vlq udp_image_hd
	global ipr_jp_ctr ipr_wp_ctr ipr_video_hd ipr_video_gs ipr_video_vlq ipr_image_hd
	global ipif_jp_ctr ipif_wp_ctr ipif_video_hd ipif_video_gs ipif_video_vlq ipif_image_hd
	global multitraffic mll_bluec mac_bluec phy_bluec
	global mll_evo_hs mac_evo_hs phy_evo_hs mll_subn_mf mac_subn_mf phy_subn_mf
	global portnum_jp_ctr portnum_wp_ctr portnum_video_hd portnum_video_gs
	global portnum_video_vlq portnum_image_hd position
	global interference_bluec interference_evo_hs interference_subn_mf

	set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	#STACK jp_ctr /monit_jp
	if {$id == $opt(id_ROV)} { 
		#packet obtained from files
		Module/UW/ROV set packetSize_          	$opt(pkt_size_jp_ctr)
		Module/UW/ROV set period_              	$opt(pkt_period_jp_ctr)
		Module/UW/ROV set PoissonTraffic_      	$opt(poisson_jp_ctr)
		Module/UW/ROV set traffic_type_			$opt(traffic_id_jp_ctr)
		set app_jp_ctr_ROV 	[new Module/UW/ROV]

	}
	if {$id == $opt(id_CTR)} {
		#packet obtained from files
		Module/UW/ROV/CTR set packetSize_       $opt(pkt_size_jp_ctr)
		Module/UW/ROV/CTR set period_           $opt(pkt_period_jp_ctr) ;#We don't use wp retx
		Module/UW/ROV/CTR set PoissonTraffic_   $opt(poisson_jp_ctr)
		Module/UW/ROV/CTR set traffic_type_		$opt(traffic_id_jp_ctr)
		set app_jp_ctr 	[new Module/UW/ROV/CTR]
	}
	set udp_jp_ctr($id)    [new Module/UW/UDP]
  	set ipr_jp_ctr($id)    [new Module/UW/StaticRouting]
  	set ipif_jp_ctr($id)   [new Module/UW/IP]

	#STACK wp_ctr /monit (the only used)
	if {$id == $opt(id_ROV)} {
		#packet obtained from files
		Module/UW/ROV set packetSize_          	$opt(pkt_size_monit)
		Module/UW/ROV set period_              	$opt(pkt_period_monit)
		Module/UW/ROV set PoissonTraffic_		$opt(poisson_monit)
		Module/UW/ROV set traffic_type_			$opt(traffic_id_monit)
		set app_monit	[new Module/UW/ROV]
		set ipr_wp_ctr($id)   [new Module/UW/PosBasedRt/ROV] 

	}
	if {$id == $opt(id_CTR)} {
		#packet obtained from files
		Module/UW/ROV/CTR set packetSize_       $opt(pkt_size_wp_ctr)
		Module/UW/ROV/CTR set period_           $opt(pkt_period_wp_ctr) ;#We don't use wp retx
		Module/UW/ROV/CTR set PoissonTraffic_   $opt(poisson_wp_ctr)
		Module/UW/ROV/CTR set traffic_type_		$opt(traffic_id_wp_ctr)
		set app_wp_ctr	[new Module/UW/ROV/CTR]
		set ipr_wp_ctr($id)   [new Module/UW/PosBasedRt] 
	}
	set udp_wp_ctr($id)    [new Module/UW/UDP]
  	#set ipr_wp_ctr($id)    [new Module/UW/StaticRouting] ;#need to be changed with posBasedRt
  	set ipif_wp_ctr($id)   [new Module/UW/IP] 

	#STACK video_hd
	Module/UW/CBR set packetSize_       $opt(pkt_size_video_hd)
	Module/UW/CBR set period_           $opt(pkt_period_video_hd)
	Module/UW/CBR set PoissonTraffic_   $opt(poisson_video_hd)
	Module/UW/CBR set traffic_type_		$opt(traffic_id_video_hd)
	set app_video_hd($id)	[new Module/UW/CBR]
	set udp_video_hd($id)   [new Module/UW/UDP]
  	set ipr_video_hd($id)   [new Module/UW/StaticRouting]
  	set ipif_video_hd($id)  [new Module/UW/IP]

	#STACK video_gs
	Module/UW/CBR set packetSize_       $opt(pkt_size_video_gs)
	Module/UW/CBR set period_           $opt(pkt_period_video_gs)
	Module/UW/CBR set PoissonTraffic_   $opt(poisson_video_gs)
	Module/UW/CBR set traffic_type_		$opt(traffic_id_video_gs)
	set app_video_gs($id)	[new Module/UW/CBR]
	set udp_video_gs($id)   [new Module/UW/UDP]
  	set ipr_video_gs($id)   [new Module/UW/StaticRouting]
  	set ipif_video_gs($id)  [new Module/UW/IP]

	#STACK video_vlq
	Module/UW/CBR set packetSize_       $opt(pkt_size_video_vlq)
	Module/UW/CBR set period_           $opt(pkt_period_video_vlq)
	Module/UW/CBR set PoissonTraffic_   $opt(poisson_video_vlq)
	Module/UW/CBR set traffic_type_		$opt(traffic_id_video_vlq)
	set app_video_vlq($id)	[new Module/UW/CBR]
	set udp_video_vlq($id)   [new Module/UW/UDP]
  	set ipr_video_vlq($id)   [new Module/UW/StaticRouting]
  	set ipif_video_vlq($id)  [new Module/UW/IP]

	#STACK image_hd
	Module/UW/CBR set packetSize_       $opt(pkt_size_image_hd)
	Module/UW/CBR set period_           $opt(pkt_period_image_hd)
	Module/UW/CBR set PoissonTraffic_   $opt(poisson_image_hd)
	Module/UW/CBR set traffic_type_		$opt(traffic_id_image_hd)
	set app_image_hd($id)	[new Module/UW/CBR]
	set udp_image_hd($id)   [new Module/UW/UDP]
  	set ipr_image_hd($id)   [new Module/UW/StaticRouting]
  	set ipif_image_hd($id)  [new Module/UW/IP]

	set multitraffic($id)  [new Module/UW/MULTI_TRAFFIC_RANGE_CTR]

	#STACK bluec
	Module/UW/TDMA set max_packet_per_slot  10
	Module/UW/TDMA set queue_size_          2000000
	Module/UW/TDMA set frame_duration       $opt(frame_duration_bluec)
	Module/UW/TDMA set guard_time       	0.05; #grater than packet duration
	Module/UW/TDMA set tot_slots        	2 ;# only for fair mode
	Module/UW/TDMA set checkPriority_		1
	Module/UW/TDMA set mac2phy_delay_       [expr 1.0e-9] ;#should be smaller for bluec, 1e-9
	Module/UW/TDMA set debug_               0

	set mll_bluec($id)   [new Module/UW/MLL]
  	set mac_bluec($id)   [new Module/UW/TDMA]
  	set phy_bluec($id)   [new Module/UW/UWOPTICALBEAMPATTERN]

	#STACK evo_hs
	Module/UW/PHYSICAL  set MaxTxSPL_dB_    $opt(tx_power_evo_hs)
	Module/UW/PHYSICAL  set BitRate_ 		$opt(bitrate_evo_hs)
	Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    10.0

	set mll_evo_hs($id)   [new Module/UW/MLL]
  	set mac_evo_hs($id)   [new Module/UW/CSMA_ALOHA]
  	set phy_evo_hs($id)   [new Module/UW/PHYSICAL]

	#STACK subn_mf
	Module/UW/CSMA_ALOHA set listen_time_       [expr 4e-9]
	Module/UW/CSMA_ALOHA set wait_costant_      [expr 5.0e-4]
	Module/UW/CSMA_ALOHA set buffer_pkts_    	20
	Module/UW/CSMA_ALOHA set debug_     		0


	Module/UW/PHYSICAL  set MaxTxSPL_dB_    $opt(tx_power_subn_mf)
	Module/UW/PHYSICAL  set BitRate_ 		$opt(bitrate_subn_mf)
	Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0
	#Module/UW/PHYSICAL  set debug_    1

	set mll_subn_mf($id)   [new Module/UW/MLL]
	set mac_subn_mf($id)   [new Module/UW/CSMA_ALOHA]
  	set phy_subn_mf($id)   [new Module/UW/PHYSICAL]

	#Add module
	if {$id == $opt(id_ROV)} {
		$node($id) addModule 34 $app_jp_ctr_ROV 1 "APP_JP"
	}
	if {$id == $opt(id_CTR)} {
		$node($id) addModule 34 $app_jp_ctr 1	"APP_JP"
	}
	$node($id) addModule 33 $udp_jp_ctr($id) 1	"UDP_JP"
	$node($id) addModule 32 $ipr_jp_ctr($id) 1	"IPR_JP"
	$node($id) addModule 31 $ipif_jp_ctr($id) 1	"IPIF_JP"

	if {$id == $opt(id_ROV)} {
		$node($id) addModule 30 $app_monit 1	"APP_WP"
	}
	if {$id == $opt(id_CTR)} {
		$node($id) addModule 30 $app_wp_ctr	1	"APP_WP"
	}
	$node($id) addModule 29 $udp_wp_ctr($id) 1		"UDP_WP"
	$node($id) addModule 28 $ipr_wp_ctr($id) 1		"IPR_WP"
	$node($id) addModule 27 $ipif_wp_ctr($id)  1	"IPIF_WP"

	$node($id) addModule 26 $app_video_hd($id)	1 	"APP_HD"
	$node($id) addModule 25 $udp_video_hd($id)	1 	"UDP_HD"
	$node($id) addModule 24 $ipr_video_hd($id)	1 	"IPR_HD"
	$node($id) addModule 23 $ipif_video_hd($id)	1 	"IPIF_HD"

	$node($id) addModule 22 $app_video_gs($id)	1 	"APP_GS"
	$node($id) addModule 21 $udp_video_gs($id)	1 	"UDP_GS"
	$node($id) addModule 20 $ipr_video_gs($id)	1 	"IPR_GS"
	$node($id) addModule 19 $ipif_video_gs($id)	1 	"IPIF_GS"

	$node($id) addModule 18 $app_video_vlq($id)	 1	"APP_VLQ"
	$node($id) addModule 17 $udp_video_vlq($id)	 1	"UDP_VLQ"
	$node($id) addModule 16 $ipr_video_vlq($id)	 1	"IPR_VLQ"
	$node($id) addModule 15 $ipif_video_vlq($id) 1	"IPIF_VLQ"

	$node($id) addModule 14 $app_image_hd($id) 1	"APP_IMG"
	$node($id) addModule 13 $udp_image_hd($id) 1	"UDP_IMG"
	$node($id) addModule 12 $ipr_image_hd($id) 1	"IPR_IMG"
	$node($id) addModule 11 $ipif_image_hd($id) 1	"IPIF_IMG"

	$node($id) addModule 10 $multitraffic($id) 1	"MLT_TR"

	$node($id) addModule 9 $mll_bluec($id) 1	"MLL_BLUEC"
	$node($id) addModule 8 $mac_bluec($id) 1	"MAC_BLUEC"
	$node($id) addModule 7 $phy_bluec($id) 1	"PHY_BLUEC"

	$node($id) addModule 6 $mll_evo_hs($id)	1	"MLL_EVO_HS"
	$node($id) addModule 5 $mac_evo_hs($id)	1	"MAC_EVO_HS"
	$node($id) addModule 4 $phy_evo_hs($id)	1	"PHY_EVO_HS"

	$node($id) addModule 3 $mll_subn_mf($id) 1	"MLL_SUB_MF"
	$node($id) addModule 2 $mac_subn_mf($id) 1	"MAC_SUB_MF"
	$node($id) addModule 1 $phy_subn_mf($id) 1	"PHY_SUB_MF"

  	#SET CONNECTION
	if {$id == $opt(id_ROV)} {
		$node($id) setConnection $app_jp_ctr_ROV 	$udp_jp_ctr($id)	1
		set portnum_jp_ctr($id) [$udp_jp_ctr($id) assignPort $app_jp_ctr_ROV]
	}
	if {$id == $opt(id_CTR)} {
		$node($id) setConnection $app_jp_ctr 		$udp_jp_ctr($id)	1
		set portnum_jp_ctr($id) [$udp_jp_ctr($id) assignPort $app_jp_ctr]
	}
	$node($id) setConnection $udp_jp_ctr($id) 		$ipr_jp_ctr($id) 	1
  	$node($id) setConnection $ipr_jp_ctr($id) 		$ipif_jp_ctr($id) 	1
  	$node($id) setConnection $ipif_jp_ctr($id) 		$multitraffic($id) 	1

	if {$id == $opt(id_ROV)} {
		$node($id) setConnection $app_monit 		$udp_wp_ctr($id)	1
		set portnum_wp_ctr($id) [$udp_wp_ctr($id) assignPort $app_monit]
	}
	if {$id == $opt(id_CTR)} {
		$node($id) setConnection $app_wp_ctr 		$udp_wp_ctr($id)	1
		set portnum_wp_ctr($id) [$udp_wp_ctr($id) assignPort $app_wp_ctr]
	}
	$node($id) setConnection $udp_wp_ctr($id) 		$ipr_wp_ctr($id) 	1
  	$node($id) setConnection $ipr_wp_ctr($id) 		$ipif_wp_ctr($id) 	1
  	$node($id) setConnection $ipif_wp_ctr($id) 		$multitraffic($id) 	1  

	$node($id) setConnection $app_video_hd($id) 	$udp_video_hd($id)	1
	set portnum_video_hd($id) [$udp_video_hd($id) assignPort $app_video_hd($id)]
	$node($id) setConnection $udp_video_hd($id)		$ipr_video_hd($id) 	1
  	$node($id) setConnection $ipr_video_hd($id) 	$ipif_video_hd($id) 1
  	$node($id) setConnection $ipif_video_hd($id) 	$multitraffic($id) 	1 
	
	$node($id) setConnection $app_video_gs($id) 	$udp_video_gs($id)	1
	set portnum_video_gs($id) [$udp_video_gs($id) assignPort $app_video_gs($id)]
	$node($id) setConnection $udp_video_gs($id)		$ipr_video_gs($id) 	1
  	$node($id) setConnection $ipr_video_gs($id) 	$ipif_video_gs($id) 1
  	$node($id) setConnection $ipif_video_gs($id) 	$multitraffic($id) 	1 

	$node($id) setConnection $app_video_vlq($id) 	$udp_video_vlq($id)		1
	set portnum_video_vlq($id) [$udp_video_vlq($id) assignPort $app_video_vlq($id)]
	$node($id) setConnection $udp_video_vlq($id)	$ipr_video_vlq($id) 	1
  	$node($id) setConnection $ipr_video_vlq($id) 	$ipif_video_vlq($id)	1
  	$node($id) setConnection $ipif_video_vlq($id) 	$multitraffic($id) 		1 

	$node($id) setConnection $app_image_hd($id) 	$udp_image_hd($id)	1
	set portnum_image_hd($id) [$udp_image_hd($id) assignPort $app_image_hd($id)]
	$node($id) setConnection $udp_image_hd($id)		$ipr_image_hd($id) 	1
  	$node($id) setConnection $ipr_image_hd($id) 	$ipif_image_hd($id) 1
  	$node($id) setConnection $ipif_image_hd($id) 	$multitraffic($id) 	1

	$node($id) setConnection 	$multitraffic($id)	$mll_bluec($id)		1
	$node($id) setConnection 	$mll_bluec($id)		$mac_bluec($id)		1
	$node($id) setConnection 	$mac_bluec($id)		$phy_bluec($id)		1
	$node($id) addToChannel 	$channel_op 		$phy_bluec($id)		1

	$node($id) setConnection 	$multitraffic($id)	$mll_evo_hs($id)	1
	$node($id) setConnection 	$mll_evo_hs($id)	$mac_evo_hs($id)	1
	$node($id) setConnection 	$mac_evo_hs($id)	$phy_evo_hs($id)	1
	$node($id) addToChannel 	$channel_ac 		$phy_evo_hs($id)	1      

	$node($id) setConnection 	$multitraffic($id)	$mll_subn_mf($id)	1
	$node($id) setConnection 	$mll_subn_mf($id)	$mac_subn_mf($id)	1
	$node($id) setConnection 	$mac_subn_mf($id)	$phy_subn_mf($id)	1
	$node($id) addToChannel 	$channel_ac 		$phy_subn_mf($id)	1   

  	if {$id > 254} {
  	    puts "hostnum > 254!!! exiting"
  	    exit
  	}

	#PHY LAYERS SETTINGS
	set interference_bluec($id)		[new "MInterference/MIV"]
	$interference_bluec($id) set maxinterval_ 	$opt(interf_max_interval)
	$interference_bluec($id) set debug_			0

	$phy_bluec($id) setPropagation			$propagation_op
	$phy_bluec($id) setSpectralMask			$data_mask_bluec
	$phy_bluec($id) setInterference			$interference_bluec($id)

	if {$id == $opt(id_ROV)} {
        $phy_bluec($id) setInclinationAngle [expr 3.14 / 2] ;#-3.14;#[expr 3.14 / 2]
    } 
	if {$id == $opt(id_CTR)} {
        $phy_bluec($id) setInclinationAngle [expr -3.14 / 2] ;#0;
    }
	$phy_bluec($id) setMaxRangePath $opt(range_lut_path)
	$phy_bluec($id) setBeamPatternPath $opt(beam_lut_path)
	$phy_bluec($id) setLUTFileName $opt(noise_LUT_bluec)
	$phy_bluec($id) setLUTSeparator " "
	$phy_bluec($id) useLUT

	set interference_evo_hs($id)	[new "Module/UW/INTERFERENCE"]
	$interference_evo_hs($id) set maxinterval_ 	$opt(interf_max_interval)
	$interference_evo_hs($id) set debug_		0

	$phy_evo_hs($id) setPropagation			$propagation_ac
	$phy_evo_hs($id) setSpectralMask		$data_mask_evo_hs
	$phy_evo_hs($id) setInterference		$interference_evo_hs($id)
	$phy_evo_hs($id) setInterferenceModel	"MEANPOWER"

	set interference_subn_mf($id)	[new "Module/UW/INTERFERENCE"]
	$interference_subn_mf($id) set maxinterval_ $opt(interf_max_interval)
	$interference_subn_mf($id) set debug_		0

	$phy_subn_mf($id) setPropagation		$propagation_ac
	$phy_subn_mf($id) setSpectralMask		$data_mask_subn_mf
	$phy_subn_mf($id) setInterference		$interference_subn_mf($id)
	$phy_subn_mf($id) setInterferenceModel	"MEANPOWER"


	#MAC LAYERS SETTINGS
	$mac_bluec($id) setMacAddr	[expr 3*$id + 1]
	if {$id == $opt(id_CTR)} {
  	  	$mac_bluec($id) setStartTime     	0.0
  	  	$mac_bluec($id) setSlotDuration  	[expr 0.1 * $opt(frame_duration_bluec)]
  	  	$mac_bluec($id) setGuardTime		$opt(guard_time_CTR_bluec)
  	}
	if {$id == $opt(id_ROV)} {
  	  	$mac_bluec($id) setStartTime   		[expr 0.1 * $opt(frame_duration_bluec)]
  	  	$mac_bluec($id) setSlotDuration  	[expr 0.9 * $opt(frame_duration_bluec)]
  	  	$mac_bluec($id) setGuardTime		$opt(guard_time_ROV_bluec)
  	}
	

	$mac_evo_hs($id) setMacAddr		[expr 3*$id + 2]
	$mac_evo_hs($id) $opt(ack_mode_evo_hs)
	$mac_evo_hs($id) initialize

	$mac_subn_mf($id) setMacAddr 			[expr 3*$id + 3]
	$mac_subn_mf($id) $opt(ack_mode_subn_mf)
	$mac_subn_mf($id) initialize

	#IP LAYER SETTINGS
	$ipif_jp_ctr($id) 		addr 	[expr $id + 1]
	$ipif_wp_ctr($id) 		addr 	[expr $id + 1]
	$ipif_video_hd($id) 	addr 	[expr $id + 1]
	$ipif_video_gs($id) 	addr 	[expr $id + 1]
	$ipif_video_vlq($id) 	addr 	[expr $id + 1]
	$ipif_image_hd($id) 	addr 	[expr $id + 1]

	$ipr_wp_ctr($id)		addr 	[expr $id + 1]

	#MULTITRAFFIC LAYER SETTING
	$multitraffic($id) addRobustLowLayer	$opt(traffic_id_jp_ctr)		"MLL_BLUEC"
    $multitraffic($id) addUpLayer 			$opt(traffic_id_jp_ctr)		"IPIF_JP"
	$multitraffic($id) setBufferFeatures	$opt(traffic_id_jp_ctr) 10 1
	
	$multitraffic($id) addRobustLowLayer	$opt(traffic_id_wp_ctr)		"MLL_SUB_MF"
    $multitraffic($id) addUpLayer 			$opt(traffic_id_wp_ctr)		"IPIF_WP"
	if {$id == $opt(id_ROV)} {
  	  	$multitraffic($id) setBufferFeatures	$opt(traffic_id_wp_ctr) 50 0
  	}
	if {$id == $opt(id_CTR)} {
  	  	$multitraffic($id) setBufferFeatures	$opt(traffic_id_wp_ctr) 20 1
  	}
	

	$multitraffic($id) addRobustLowLayer	$opt(traffic_id_video_hd)	"MLL_BLUEC"
	$multitraffic($id) addUpLayer			$opt(traffic_id_video_hd)	"IPIF_HD"
	$multitraffic($id) setBufferFeatures	$opt(traffic_id_video_hd) 15 1

	$multitraffic($id) addRobustLowLayer	$opt(traffic_id_video_gs)	"MLL_BLUEC"
	$multitraffic($id) addUpLayer			$opt(traffic_id_video_gs)	"IPIF_GS"
	$multitraffic($id) setBufferFeatures	$opt(traffic_id_video_gs) 15 1

	$multitraffic($id) addFastLowLayer 		$opt(traffic_id_video_vlq)	"MLL_EVO_HS"
	$multitraffic($id) addUpLayer			$opt(traffic_id_video_vlq)	"IPIF_VLQ"
	$multitraffic($id) setBufferFeatures	$opt(traffic_id_video_vlq) 15 1

	$multitraffic($id) addFastLowLayer		$opt(traffic_id_image_hd)	"MLL_BLUEC"
	$multitraffic($id) addUpLayer			$opt(traffic_id_image_hd)	"IPIF_IMG"
	$multitraffic($id) setBufferFeatures	$opt(traffic_id_image_hd) 100000 0 $opt(send_down_delay_img)

  	#POSITION
    set position($id) [new "Position/UWSM"]
    $node($id) addPosition $position($id)
	$position($id) set debug_ 0

    if {$id == $opt(id_ROV)} {
		$position($id) setX_ 6.245
    	$position($id) setY_ -6.826
    	$position($id) setZ_ -10.458
      	$app_jp_ctr_ROV setPosition $position($id)
		$app_monit setPosition $position($id)
		#position need to be assigned also to the posBasedRt
    } 
	if {$id == $opt(id_CTR)} {
		$position($id) setX_ 0
    	$position($id) setY_ 0
    	$position($id) setZ_ -5
      	$app_jp_ctr setPosition $position($id)
		$app_wp_ctr setPosition $position($id)
		#position need to be assigned also to the posBasedRt
    } 

	if {$id == $opt(id_ROV)} {
		$app_jp_ctr_ROV setLogFileName "test_wireless_rov_pos$opt(rngstream).csv"
		$app_monit setLogFileName "test_wireless_rov_pos$opt(rngstream).csv"
		
	}
	if {$id == $opt(id_CTR)} {
		$app_video_hd($id) setLogSuffix "_rx_video_hd" 12
		$app_video_gs($id) setLogSuffix "_rx_video_gs" 12
		$app_video_vlq($id) setLogSuffix "_rx_video_vlq" 12
		$app_image_hd($id) setLogSuffix "_rx_image_hd" 12
	}
}

#RELAY

proc createRelay {id} { 
	global ns node opt data_mask_subn_mf propagation_ac channel_ac
	global app_relay udp_relay ipr_relay ipif_relay
	global mll_relay mac_relay phy_relay portnum_relay
	global interference_relay position_relay

	set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	set app_relay($id)	[new Module/UW/CBR]
	set udp_relay($id)	[new Module/UW/UDP]
  	set ipr_relay($id)  [new Module/UW/PosBasedRt]
  	set ipif_relay($id) [new Module/UW/IP]

	Module/UW/CSMA_ALOHA set listen_time_       [expr 4e-9]
	Module/UW/CSMA_ALOHA set wait_costant_      [expr 5.0e-4]
	Module/UW/CSMA_ALOHA set buffer_pkts_    	20
	Module/UW/CSMA_ALOHA set debug_     		0

	Module/UW/PHYSICAL  set MaxTxSPL_dB_    $opt(tx_power_subn_mf)
	Module/UW/PHYSICAL  set BitRate_ 		$opt(bitrate_subn_mf)
	Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    5.0

	set mll_relay($id)   [new Module/UW/MLL]
  	set mac_relay($id)   [new Module/UW/CSMA_ALOHA]
  	set phy_relay($id)   [new Module/UW/PHYSICAL]

	#Add module

	$node($id) addModule 7 $app_relay($id) 		1	"APP_R"
	$node($id) addModule 6 $udp_relay($id) 		1	"UDP_R"
	$node($id) addModule 5 $ipr_relay($id) 		1	"IPR_R"
	$node($id) addModule 4 $ipif_relay($id) 	1	"IPIF_R"
	$node($id) addModule 3 $mll_relay($id)	1	"MLL_R"
	$node($id) addModule 2 $mac_relay($id) 	1	"MAC_R"
	$node($id) addModule 1 $phy_relay($id) 	1	"PHY_R"

  	#SET CONNECTION
	$node($id) setConnection 	$app_relay($id) 	$udp_relay($id)		1
	set portnum_relay($id) [$udp_relay($id) assignPort $app_relay($id)]
	$node($id) setConnection 	$udp_relay($id)		$ipr_relay($id) 	1
  	$node($id) setConnection 	$ipr_relay($id) 	$ipif_relay($id) 	1
	$node($id) setConnection 	$ipif_relay($id)	$mll_relay($id)	1
	$node($id) setConnection 	$mll_relay($id)		$mac_relay($id)	1
	$node($id) setConnection 	$mac_relay($id)		$phy_relay($id)	1
	$node($id) addToChannel 	$channel_ac 		$phy_relay($id)	1   

  	if {$id > 254} {
  	    puts "hostnum > 254!!! exiting"
  	    exit
  	}

	#PHY LAYERS SETTINGS
	set interference_relay($id)	[new "Module/UW/INTERFERENCE"]
	$interference_relay($id) set maxinterval_ $opt(interf_max_interval)
	$interference_relay($id) set debug_		0

	$phy_relay($id) setPropagation		$propagation_ac
	$phy_relay($id) setSpectralMask		$data_mask_subn_mf
	$phy_relay($id) setInterference		$interference_relay($id)
	$phy_relay($id) setInterferenceModel	"MEANPOWER"


	#MAC LAYERS SETTINGS
	$mac_relay($id) setMacAddr 	[expr 3*$id + 3]
	$mac_relay($id) $opt(ack_mode_subn_mf)
	$mac_relay($id) initialize


	#IP LAYER SETTINGS
	$ipif_relay($id) 		addr 	[expr $id + 1]
	$ipr_relay($id)  		addr 	[expr $id + 1]

  	#POSITION
    set position_relay($id) [new "Position/BM"]
    $node($id) addPosition $position_relay($id)
	$position_relay($id) setX_ [expr ($id - 1) * 3000]
    $position_relay($id) setY_ 100
    $position_relay($id) setZ_ -5
	$position_relay($id) set debug_ 0

}