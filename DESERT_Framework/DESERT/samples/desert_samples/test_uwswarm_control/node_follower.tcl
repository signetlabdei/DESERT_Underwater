proc createNodeF { id } {
  global channel ns position nodeF ipr ipif portnum_rov portnum_trf
  global opt mll mac propagation data_mask interf_data udp phy
  global app_trf app_rov leader_id
  
  set nodeF($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

  set app_rov($id)				[new Module/UW/ROV]
  set app_trf($id,$leader_id)   [new Module/UW/SC/TRACKERF] 
  set udp($id)  				[new Module/UW/UDP]
  set ipr($id)  				[new Module/UW/StaticRouting]
  set ipif($id) 				[new Module/UW/IP]
  set mll($id)  				[new Module/UW/MLL] 
  set mac($id)  				[new Module/UW/CSMA_ALOHA]
  #set phy($id)  				[new Module/UW/PHYSICAL]
  set phy($id)  				[new Module/UW/AHOI/PHY]
	
  $nodeF($id) addModule 7 $app_rov($id)				  1  "CBR"
  $nodeF($id) addModule 7 $app_trf($id,$leader_id)    1  "CBR"
  $nodeF($id) addModule 6 $udp($id) 				  1  "UDP"
  $nodeF($id) addModule 5 $ipr($id)					  1  "IPR"
  $nodeF($id) addModule 4 $ipif($id)  				  1  "IPF"   
  $nodeF($id) addModule 3 $mll($id)   				  1  "MLL"
  $nodeF($id) addModule 2 $mac($id)   				  1  "MAC"
  $nodeF($id) addModule 1 $phy($id)   				  1  "PHY"

  $nodeF($id) setConnection $app_rov($id)				$udp($id)   1
  $nodeF($id) setConnection $app_trf($id,$leader_id)    $udp($id)   0
  $nodeF($id) setConnection $udp($id)   				$ipr($id)	1
  $nodeF($id) setConnection $ipr($id)				    $ipif($id)  1
  $nodeF($id) setConnection $ipif($id)  			    $mll($id)   1
  $nodeF($id) setConnection $mll($id)   			    $mac($id)   1
  $nodeF($id) setConnection $mac($id)   			    $phy($id)   1
  $nodeF($id) addToChannel  $channel    			    $phy($id)   1

  set portnum_rov($id) [$udp($id) assignPort $app_rov($id)]
  set portnum_trf($id) [$udp($id) assignPort $app_trf($id,$leader_id)]

  set position($id) [new "Position/UWSM"]
  $nodeF($id) addPosition $position($id)
  
  #Setup positions
  if { [expr $id % 2] == 0 } {
	  $position($id) setX_ [expr [$position($leader_id) getX_] - 12.5*$id/2]
	  $position($id) setY_ [expr [$position($leader_id) getY_] + 12.5*$id/2]
  } else {
	  $position($id) setX_ [expr [$position($leader_id) getX_] - 12.5*($id+1)/2]
	  $position($id) setY_ [expr [$position($leader_id) getY_] - 12.5*($id+1)/2]
  }
  $position($id) setZ_ -15

  $app_rov($id) setPosition $position($id)

  #Set the IP address of the nodeF
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
