proc createNodeL { id } {
  global channel ns position nodeL portnum_ctr portnum_trl ipr ipif
  global opt mll mac propagation data_mask interf_data udp phy
  global app_ctr app_trl app_mc
  
  set nodeL [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

  for {set cnt 1} {$cnt < $opt(nn)} {incr cnt} {
	set app_ctr($id,$cnt)  [new Module/UW/SC/CTR]
	set app_trl($id,$cnt)  [new Module/UW/SC/TRACKER] 
  }
  set udp($id)  [new Module/UW/UDP]
  set ipr($id)  [new Module/UW/StaticRouting]
  set ipif($id) [new Module/UW/IP]
  set mll($id)  [new Module/UW/MLL] 
  set mac($id)  [new Module/UW/CSMA_ALOHA]
  set phy($id)  [new Module/UW/PHYSICAL]
#  set phy($id)  				[new Module/UW/PHYSICAL]
  set phy($id)  				[new Module/UW/AHOI/PHY]
	
  for {set cnt 1} {$cnt < $opt(nn)} {incr cnt} {
	$nodeL addModule 7 $app_ctr($id,$cnt)   1  "ROV"
    $nodeL addModule 7 $app_trl($id,$cnt)   1  "CBR"
  }
  $nodeL addModule 6 $udp($id)   1  "UDP"
  $nodeL addModule 5 $ipr($id)   1  "IPR"
  $nodeL addModule 4 $ipif($id)  1  "IPF"   
  $nodeL addModule 3 $mll($id)   1  "MLL"
  $nodeL addModule 2 $mac($id)   1  "MAC"
  $nodeL addModule 1 $phy($id)   1  "PHY"

  for {set cnt 1} {$cnt < $opt(nn)} {incr cnt} {
	$nodeL setConnection $app_ctr($id,$cnt)	  $udp($id)  1
    $nodeL setConnection $app_trl($id,$cnt)   $udp($id)  0
  }
  $nodeL setConnection $udp($id)   $ipr($id)   1
  $nodeL setConnection $ipr($id)   $ipif($id)  1
  $nodeL setConnection $ipif($id)  $mll($id)   1
  $nodeL setConnection $mll($id)   $mac($id)   1
  $nodeL setConnection $mac($id)   $phy($id)   1
  $nodeL addToChannel  $channel    $phy($id)   1

  for {set cnt 1} {$cnt < $opt(nn)} {incr cnt} {
    set portnum_ctr($id,$cnt) [$udp($id) assignPort $app_ctr($id,$cnt)]
    set portnum_trl($id,$cnt) [$udp($id) assignPort $app_trl($id,$cnt)]
  }

  # Setup mission coordinator
  set app_mc($id) [new Plugin/UW/SC/MC]
  $nodeL addPlugin $app_mc($id)   1  "MC"
  for {set cnt 1} {$cnt < $opt(nn)} {incr cnt} {
    $app_mc($id) addAUV [$app_ctr($id,$cnt) Id_] [$app_trl($id,$cnt) Id_]
    $app_trl($id,$cnt) setLeaderId [$app_mc($id) Id_]
    $app_ctr($id,$cnt) setLeaderId [$app_mc($id) Id_]
  }

  #Setup positions
  set position($id) [new "Position/UWSM"]
  $nodeL addPosition $position($id)
  
  $position($id) setX_ -100 
  $position($id) setY_ 25
  $position($id) setZ_ -20

  for {set cnt 1} {$cnt < $opt(nn)} {incr cnt} {
	  $app_ctr($id,$cnt) setPosition $position($id)
  }

  if {$id > 254} {
    puts "hostnum > 254!!! exiting"
    exit
  }

  #Set the IP address of the nodeL
  $ipif($id) addr [expr $id + 1]

  # Set the MAC address
  $mac($id) $opt(ack_mode)
  $mac($id) initialize

  #Interference model
  set interf_data($id)  [new "Module/UW/INTERFERENCE"]
  $interf_data($id) set maxinterval_ $opt(maxinterval_)
  $interf_data($id) set debug_     0

  #Propagation model
  $phy($id) setPropagation $propagation
  
  $phy($id) setSpectralMask $data_mask
  $phy($id) setInterference $interf_data($id)
  $phy($id) setInterferenceModel "MEANPOWER"
  $phy($id) setRangePDRFileName "../dbs/ahoi/default_pdr.csv"
  $phy($id) setSIRFileName "../dbs/ahoi/default_sir.csv"
  $phy($id) initLUT
}
