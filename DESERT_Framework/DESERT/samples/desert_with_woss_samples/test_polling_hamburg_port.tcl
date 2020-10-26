# WOSS - World Ocean Simulation System -
# 
# Copyright (C) 2009 Federico Guerra 
# and regents of the SIGNET lab, University of Padova
# 
# Author: Federico Guerra - federico@guerra-tlc.com
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# This software has been developed by Federico Guerra
# and SIGNET lab, University of Padova, 
# in collaboration with the NATO Centre for Maritime Research 
# and Experimentation (http://www.cmre.nato.int ; 
# E-mail: pao@cmre.nato.int), 
# whose support is gratefully acknowledged.

######################################
# Flags to enable or disable options #
######################################
set opt(verbose)          1
set opt(trace_files)      0
set opt(bash_parameters)  1

# Module libraries

load libMiracle.so
load libmiraclecbr.so
load libmphy.so
load libmmac.so
load libMiracleBasicMovement.so
load libMiracleIp.so
load libMiracleIpRouting.so
load libmiracleport.so
load libmll.so

load libUwmStd.so
load libWOSS.so
load libWOSSPhy.so

load libcbrtracer.so
load libUwmStdPhyBpskTracer.so
load libuwphy_clmsgs.so
load libuwinterference.so
load libuwphysical.so
load libuwcsmaaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwphy_clmsgs.so
load libuwpolling.so

############################

#######################

set ns [new Simulator]
$ns use-Miracle

######################################
# global allocations & misc options
######################################

set opt(start_clock) [clock seconds]
 
set opt(start_lat)     53.541361
set opt(start_long)    9.928822
set opt(nn)            11 ;#NUMBER OF SENSOR NODES
set opt(total_nn)   [expr $opt(nn) +  2] ;# +1 AUV +1 SINK
set opt(pktsize)       125
set opt(starttime)         0.1
set opt(stoptime)      7200.0
set opt(txduration) [expr $opt(stoptime) - $opt(starttime)]
set opt(speed)    2.0;#4knots
set opt(cbr_period)    850;#850-->3 packets each AUV round
set opt(pos_file)	"polling_node_wp.txt"
set opt(n_laps)			2

if {$opt(bash_parameters)} {
  if {$argc != 2} {
    puts "The script requires one number to be inputed for seed"
    puts "For example, ns test_uwpolling.tcl 1 "
    puts "If you want to leave the default values, please set to 0"
    puts "the value opt(bash_parameters) in the tcl script"
    puts "Please try again."
  }   else { 
    set opt(rep_num)	 	[lindex $argv 0]
	set opt(cbr_period) 	[lindex $argv 1]
  }
} else {
  set opt(rep_num)    2
}
set opt(T_backoff)        10
set opt(maxinterval_)   500.0

set opt(freq) 			      150000.0
set opt(bw)              	60000.0
set opt(bitrate)	 	      7000

puts "new script"

set data_mask [new "MSpectralMask/Rect"]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)


#########################
# Node and WP location  #
#########################

set fp [open $opt(pos_file) r]
set file_data [read $fp]
set data [split $file_data "\n"]
set opt(n_wp) 0
foreach line $data {
    if {[regexp {^(.*),(.*),(.*)$} $line -> node_name lat lon]} {
		if {[regexp {NODE([0-9]*)} $node_name -> id]} {
			set lat_node($id) $lat
			set lon_node($id) $lon
		}
		if {[regexp {SINK} $node_name -> id]} {
			set lat_sink $lat
			set lon_sink $lon
		}
		if {[regexp {AUV} $node_name -> id]} {
			set start_lat_auv $lat
			set start_lon_auv $lon
		}
		if {[regexp {WP([0-9][0-9]*)} $node_name -> id]} {
			set lat_wp($id) $lat
			set lon_wp($id) $lon
			set opt(n_wp) [expr $id + 1]
		}
        if {[regexp {LAST_WP} $node_name]} {
			set lat_last_wp $lat
			set lon_last_wp $lon
		}
    }
}
puts "n wp $opt(n_wp)"


########################################
# Random Number Generators
########################################

global def_rng
set def_rng [new RNG]
$def_rng default

for {set k 0} {$k < $opt(rep_num)} {incr k} {
     $def_rng next-substream
}

#set opt(tracefilename) "/tmp/${argv0}.tr"
set opt(tracefilename) "/dev/null"
set opt(tracefile) [open $opt(tracefilename) w]

#set opt(cltracefilename) "/tmp/${argv0}.cltr"
set opt(cltracefilename) "/dev/null"
set opt(cltracefile) [open $opt(cltracefilename) w]

# set opt(db_path)        "insert_db_path_here"
set opt(db_path)        "/home/alberto_signori/woss_db/dbs/"

# set opt(db_path_gebco)  "insert_db_path_gebco_path_here"
set opt(db_path_gebco)  "/home/alberto_signori/woss_db/"

set opt(db_res_path)    "./res_hamburg/"

if { $opt(db_path) == "insert_db_path_here" } {
  puts "You have to set the database path first."
  exit
}

if { $opt(db_path_gebco) == "insert_db_path_gebco_here" } {
  puts "You have to set the GEBCO database path first."
  exit
}

###########################################
# WOSS PARAMETERS DEFINITION
###########################################
WOSS/Definitions/RandomGenerator/NS2 set rep_number_ $opt(rep_num)

set ssp_creator         [new "WOSS/Definitions/SSP"]
set sediment_creator    [new "WOSS/Definitions/Sediment"]
set pressure_creator    [new "WOSS/Definitions/Pressure"]
set time_arr_creator    [new "WOSS/Definitions/TimeArr"]
set time_reference      [new "WOSS/Definitions/TimeReference/NS2"]
set transducer_creator  [new "WOSS/Definitions/Transducer"]
set altimetry_creator   [new "WOSS/Definitions/Altimetry/Bretschneider"]
set rand_generator      [new "WOSS/Definitions/RandomGenerator/NS2"]
$rand_generator initialize

set def_handler [new "WOSS/Definitions/Handler"]
$def_handler setSSPCreator         $ssp_creator
$def_handler setSedimentCreator    $sediment_creator
$def_handler setPressureCreator    $pressure_creator
$def_handler setTimeArrCreator     $time_arr_creator
$def_handler setTransducerCreator  $transducer_creator
$def_handler setTimeReference      $time_reference
$def_handler setRandomGenerator    $rand_generator
$def_handler setAltimetryCreator   $altimetry_creator

#WOSS RESULTS DB ASCII FILE
WOSS/Creator/Database/Textual/Results/TimeArr set debug           0
WOSS/Creator/Database/Textual/Results/TimeArr set woss_db_debug   0
WOSS/Creator/Database/Textual/Results/TimeArr set space_sampling 20.0

set db_res_arr [new "WOSS/Creator/Database/Textual/Results/TimeArr"]
$db_res_arr setDbPathName "${opt(db_res_path)}/test_hamburg_port_res_arr.txt"

#WOSS RESULTS DB BIN FILE
#WOSS/Creator/Database/Binary/Results/TimeArr set debug           0
#WOSS/Creator/Database/Binary/Results/TimeArr set woss_db_debug   0
#WOSS/Creator/Database/Binary/Results/TimeArr set space_sampling 0.0

#set db_res_arr [new "WOSS/Creator/Database/Binary/Results/TimeArr"]
#$db_res_arr setDbPathName "${opt(db_res_path)}/test_aloha_with_dbs_res_arr.dat"


WOSS/Creator/Database/NetCDF/Sediment/DECK41 set debug         0
WOSS/Creator/Database/NetCDF/Sediment/DECK41 set woss_db_debug 0

set db_sedim [new "WOSS/Creator/Database/NetCDF/Sediment/DECK41"]
$db_sedim setUpDeck41CoordinatesDb  "${opt(db_path)}/seafloor_sediment/DECK41_coordinates.nc"
$db_sedim setUpDeck41MarsdenDb      "${opt(db_path)}/seafloor_sediment/DECK41_mardsen_square.nc"
$db_sedim setUpDeck41MarsdenOneDb   "${opt(db_path)}/seafloor_sediment/DECK41_mardsen_one_degree.nc"


WOSS/Creator/Database/NetCDF/SSP/WOA2005/MonthlyAverage set debug          0
WOSS/Creator/Database/NetCDF/SSP/WOA2005/MonthlyAverage set woss_db_debug  0

set db_ssp [new "WOSS/Creator/Database/NetCDF/SSP/WOA2005/MonthlyAverage"]
#$db_ssp setDbPathName "${opt(db_path)}/ssp/standard_depth/2WOA2009_SSP_April.nc"
$db_ssp setDbPathName "${opt(db_path)}/ssp/2WOA2009_SSP_April.nc"


WOSS/Creator/Database/NetCDF/Bathymetry/GEBCO set debug           0
WOSS/Creator/Database/NetCDF/Bathymetry/GEBCO set woss_db_debug   0

set db_bathy [new "WOSS/Creator/Database/NetCDF/Bathymetry/HAMBURG_PORT"]
$db_bathy setDbPathName "${opt(db_path_gebco)}/bathymetry/hamburg_port.csv"

WOSS/Database/Manager set debug 0

WOSS/Definitions/Altimetry/Flat set evolution_time_quantum   -1
WOSS/Definitions/Altimetry/Flat set range                    -1
WOSS/Definitions/Altimetry/Flat set total_range_steps        -1
WOSS/Definitions/Altimetry/Flat set depth                    0.0
set cust_altimetry   [new "WOSS/Definitions/Altimetry/Flat"]

# WOSS/Definitions/Altimetry/Bretschneider set evolution_time_quantum   -1
# WOSS/Definitions/Altimetry/Bretschneider set range                    -1
# WOSS/Definitions/Altimetry/Bretschneider set total_range_steps        3000
# WOSS/Definitions/Altimetry/Bretschneider set characteristic_height    1.5
# WOSS/Definitions/Altimetry/Bretschneider set average_period           3.0
# set cust_altimetry   [new "WOSS/Definitions/Altimetry/Bretschneider"]


#first ssp, time key is 1st january 2011, 9:01 am
set time_evo_ssp_1 [new "WOSS/Definitions/Time"]
$time_evo_ssp_1 setTime 1 1 2010 0 0 1
set db_manager [new "WOSS/Database/Manager"]
$db_manager setCustomSSP        $time_evo_ssp_1 "./ssp-hamb-port.txt"
# $db_manager setCustomAltimetry  $cust_altimetry

WOSS/Creator/Bellhop set debug                        0.0
WOSS/Creator/Bellhop set woss_debug                   0.0
WOSS/Creator/Bellhop set woss_clean_workdir           1.0
WOSS/Creator/Bellhop set bellhop_arr_syntax           1
WOSS/Creator/Bellhop set evolution_time_quantum      -1.0
WOSS/Creator/Bellhop set total_runs                   1
WOSS/Creator/Bellhop set frequency_step               0.0 ;#automatically set to WOSS_CREATOR_MAX_FREQ_STEP (however ot useful since sim is performed at 1 frequency)
WOSS/Creator/Bellhop set total_range_steps            3000.0 ;#number of steps between rx and tx
WOSS/Creator/Bellhop set tx_min_depth_offset          0.0
WOSS/Creator/Bellhop set tx_max_depth_offset          0.0
WOSS/Creator/Bellhop set total_transmitters           1
WOSS/Creator/Bellhop set total_rx_depths              3
WOSS/Creator/Bellhop set rx_min_depth_offset          -0.1
WOSS/Creator/Bellhop set rx_max_depth_offset          0.1
WOSS/Creator/Bellhop set total_rx_ranges              3
WOSS/Creator/Bellhop set rx_min_range_offset          -0.1
WOSS/Creator/Bellhop set rx_max_range_offset          0.1
WOSS/Creator/Bellhop set total_rays                   0.0
WOSS/Creator/Bellhop set min_angle                    -20.0
WOSS/Creator/Bellhop set max_angle                    20.0
WOSS/Creator/Bellhop set ssp_depth_precision          1.0e-8
WOSS/Creator/Bellhop set normalized_ssp_depth_steps   100000
WOSS/Creator/Bellhop set box_depth                    100
WOSS/Creator/Bellhop set box_range                    5000


set woss_creator [new "WOSS/Creator/Bellhop"]
$woss_creator setWorkDirPath        "./bellhop_out_hamburg_port/"
$woss_creator setBellhopPath        ""
$woss_creator setBellhopMode        0 0 "A"
$woss_creator setBeamOptions        0 0 "B"
$woss_creator setBathymetryType     0 0 "C" 
$woss_creator setBathymetryMethod   0 0 "D" ;#CORRECT TYPE IS D
$woss_creator setAltimetryType      0 0 "L"
$woss_creator setSimulationTimes    0 0 1 1 2010 0 0 1 2 1 2010 0 0 1

#WOSS/Manager/Simple set debug                     1
#WOSS/Manager/Simple set is_time_evolution_active -1.0
#WOSS/Manager/Simple set space_sampling            20.0
#set woss_manager [new "WOSS/Manager/Simple"]
#

WOSS/Manager/Simple/MultiThread set debug                     0
WOSS/Manager/Simple/MultiThread set is_time_evolution_active -1.0
WOSS/Manager/Simple/MultiThread set concurrent_threads        0
WOSS/Manager/Simple/MultiThread set space_sampling            20.0
set woss_manager [new "WOSS/Manager/Simple/MultiThread"]

WOSS/Utilities set debug 0
set woss_utilities [new "WOSS/Utilities"]

WOSS/Definitions/TransducerHandler set debug 0
set transducer_handler [new WOSS/Definitions/TransducerHandler]

WOSS/Controller set debug 0
set woss_controller [new "WOSS/Controller"]
$woss_controller setBathymetryDbCreator      $db_bathy
$woss_controller setSedimentDbCreator        $db_sedim
#$woss_controller setSSPDbCreator             $db_ssp
$woss_controller setTimeArrResultsDbCreator  $db_res_arr
$woss_controller setWossDbManager            $db_manager
$woss_controller setWossManager              $woss_manager
$woss_controller setWossCreator              $woss_creator
$woss_controller setTransducerhandler        $transducer_handler
$woss_controller initialize

WOSS/PlugIn/ChannelEstimator set debug_ 0.0

WOSS/ChannelEstimator set debug_           0.0
WOSS/ChannelEstimator set space_sampling_  20.0
WOSS/ChannelEstimator set avg_coeff_       0.5
set channel_estimator [ new "WOSS/ChannelEstimator"]

WOSS/Module/Channel set channel_eq_snr_threshold_db_ 	 0
WOSS/Module/Channel set channel_symbol_resolution_   [expr 1.0/$opt(bitrate)];#  symbol_time (bit time since BPSK is used)
WOSS/Module/Channel set channel_eq_time_ 	 -1
WOSS/Module/Channel set channel_max_distance_ 	 700
WOSS/Module/Channel set debug_                    	 0

set channel [new "WOSS/Module/Channel"]
$channel setWossManager      $woss_manager
$channel setChannelEstimator $channel_estimator


WOSS/MPropagation set practicalSpreading_ 1.75
WOSS/MPropagation set debug_              0
WOSS/MPropagation set windspeed_          5
WOSS/MPropagation set shipping_           1
WOSS/MPropagation set debug_ 0
set propagation [new "WOSS/MPropagation"]
$propagation setWossManager $woss_manager


###########################################
# DESERT PARAMETERS DEFINITION
###########################################

#CBR MODULE
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set drop_out_of_order_   0
Module/UW/CBR set debug_      0

#POLLING MODULE
Module/UW/POLLING/AUV set max_payload_        $opt(pktsize)
Module/UW/POLLING/AUV set T_min_              0
Module/UW/POLLING/AUV set T_max_              $opt(T_backoff);#MAX backoff time
Module/UW/POLLING/AUV set T_probe_guard_      2;#related to T_max + RTT
Module/UW/POLLING/AUV set T_guard_            1
Module/UW/POLLING/AUV set max_polled_node_    20
Module/UW/POLLING/AUV set max_buffer_size_	 	35000
Module/UW/POLLING/AUV set max_tx_pkts_ 			140
Module/UW/POLLING/AUV set n_run                $opt(rep_num);#used for c++ rng
Module/UW/POLLING/AUV set debug_			0

Module/UW/POLLING/NODE set T_poll_guard	      5 
Module/UW/POLLING/NODE set backoff_tuner_     1
Module/UW/POLLING/NODE set max_payload_       $opt(pktsize)
Module/UW/POLLING/NODE set buffer_data_pkts_  3500
Module/UW/POLLING/NODE set Max_DATA_Pkts_TX_  20
Module/UW/POLLING/NODE set useAdaptiveTpoll_   1
Module/UW/POLLING/NODE set n_run                $opt(rep_num);#used for c++ rng
Module/UW/POLLING/NODE set debug_			0

Module/UW/POLLING/SINK set T_data_guard			5 
Module/UW/POLLING/SINK set sink_id_				253
Module/UW/POLLING/SINK set n_run                $opt(rep_num);#used for c++ rng
Module/UW/POLLING/SINK set useAdaptiveTdata_    1
Module/UW/POLLING/SINK set debug_			0

#PHY MODULE
Module/UW/PHYSICAL  set BitRate_                    $opt(bitrate)
Module/UW/PHYSICAL  set AcquisitionThreshold_dB_    10.0 
Module/UW/PHYSICAL  set RxSnrPenalty_dB_            0
Module/UW/PHYSICAL  set TxSPLMargin_dB_             0
Module/UW/PHYSICAL  set MaxTxSPL_dB_                156
Module/UW/PHYSICAL  set MinTxSPL_dB_                10
Module/UW/PHYSICAL  set MaxTxRange_                 1000
Module/UW/PHYSICAL  set PER_target_                 0    
Module/UW/PHYSICAL  set CentralFreqOptimization_    0
Module/UW/PHYSICAL  set BandwidthOptimization_      0
Module/UW/PHYSICAL  set SPLOptimization_            0
Module/UW/PHYSICAL  set debug_                      0


################################
# Procedure(s) to create nodes #
################################

proc createNode { id } {
    
    global channel propagation data_mask ns cbr position node udp portnum ipr ipif channel_estimator
    global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global row lat_node lon_node 
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
    
    set cbr($id)  [new Module/UW/CBR] 
    set udp($id) [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/POLLING/NODE]
    set phy_data($id)  [new Module/UW/PHYSICAL]

    $node($id)  addModule 7 $cbr($id)   1  "CBR"
    $node($id)  addModule 6 $udp($id)  1  "PRT"
    $node($id)  addModule 5 $ipr($id)   1  "IPR"
    $node($id)  addModule 4 $ipif($id)  1  "IPF"   
    $node($id) addModule  3 $mll($id)   1  "MLL"
    $node($id)  addModule 2 $mac($id)   1  "MAC"
    $node($id)  addModule 1 $phy_data($id)   1  "PHY"

    $node($id) setConnection $cbr($id)   $udp($id)  0
    $node($id) setConnection $udp($id)  $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy_data($id)   1
    $node($id) addToChannel  $channel    $phy_data($id)   1


    set portnum($id) [$udp($id) assignPort $cbr($id) ]
    if {$id > 254} {
	     puts "hostnum > 254!!! exiting"
	     exit
    }

    #$ipif($id) addr "1.0.0.${id}"
    $ipif($id) addr [expr $id + 1]
	
    set curr_depth [ expr [ $db_manager getBathymetry $lat_node($id) $lon_node($id) ]  ]
    set curr_depth [expr $curr_depth - $curr_depth/2.0]
	#set curr_depth [expr $curr_depth - 1.0]

    set position($id) [new "WOSS/Position"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$ipif($id) addr] $position($id)

    #### please Note: if you want to use the legacy underwatermiracle, you should use standard Position class and
    #### setX_ , setY_ , setZ_ ( negative values ) for space positioning!
    $position($id) setLatitude_  $lat_node($id);#curr_lat
    $position($id) setLongitude_ $lon_node($id);#curr_lon
    $position($id) setAltitude_  [expr -1.0 * $curr_depth]  

    puts "node $id at ([$position($id) getLatitude_], [$position($id) getLongitude_], [$position($id) getAltitude_]) , ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"

    
    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0


    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
	$phy_data($id) setInterferenceModel "MEANPOWER"
    $phy_data($id) set debug_ 0

    $mac($id) set node_id_ [expr $id + 1]
    $mac($id) initialize

}

proc createAUV { } {
    global channel propagation smask data_mask ns cbr_auv position_auv node_auv udp_auv portnum_auv 
    global phy_data_auv posdb_auv opt mll_auv mac_auv ipr_auv ipif_auv bpsk interf_data_auv channel_estimator
    global woss_utilities woss_creator start_lat_auv start_lon_auv db_manager

    set node_auv [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set cbr_auv($cnt)  [new Module/UW/CBR] 
    }
    set udp_auv      [new Module/UW/UDP]
    set ipr_auv       [new Module/UW/StaticRouting]
    set ipif_auv      [new Module/UW/IP]
    set mll_auv       [new Module/UW/MLL] 
    set mac_auv       [new Module/UW/POLLING/AUV]
    set phy_data_auv  [new Module/UW/PHYSICAL]

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
       $node_auv addModule 7 $cbr_auv($cnt) 0 "CBR"
    }
    $node_auv addModule 6 $udp_auv      0 "PRT"
    $node_auv addModule 5 $ipr_auv       0 "IPR"
    $node_auv addModule 4 $ipif_auv      0 "IPF"   
    $node_auv addModule 3 $mll_auv       0 "MLL"
    $node_auv addModule 2 $mac_auv       1 "MAC"
    $node_auv addModule 1 $phy_data_auv  1 "PHY"

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       $node_auv setConnection $cbr_auv($cnt)  $udp_auv     1
    }
    $node_auv setConnection $udp_auv $ipr_auv      	0
    $node_auv setConnection $ipr_auv  $ipif_auv       	0
    $node_auv setConnection $ipif_auv $mll_auv        	0 
    $node_auv setConnection $mll_auv  $mac_auv        	0
    $node_auv setConnection $mac_auv  $phy_data_auv   	1
    $node_auv addToChannel $channel    $phy_data_auv   	1

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_auv($cnt) [$udp_auv assignPort $cbr_auv($cnt)]
    }

    #set start_lat_auv 53.54224
	#set start_lon_auv 9.93568
	set curr_depth [ expr [ $db_manager getBathymetry $start_lat_auv $start_lon_auv]  ]
    set curr_depth [expr $curr_depth - $curr_depth/2.0]
	
    set position_auv [new "WOSS/Position/WayPoint"]
    $node_auv addPosition $position_auv
	$position_auv addStartWayPoint $start_lat_auv $start_lon_auv $curr_depth $opt(speed) 0.0 

    puts "node auv at ([$position_auv getLatitude_], [$position_auv getLongitude_], [$position_auv getAltitude_]) , ([$position_auv getX_], [$position_auv getY_], [$position_auv getZ_])"

    $ipif_auv addr 253      

    set interf_data_auv [new Module/UW/INTERFERENCE]
    $interf_data_auv set maxinterval_ $opt(maxinterval_)
    $interf_data_auv set debug_       0

    $phy_data_auv setSpectralMask     $data_mask
    $phy_data_auv setPropagation      $propagation
    $phy_data_auv setInterference     $interf_data_auv
	$phy_data_auv setInterferenceModel "MEANPOWER"

    $mac_auv initialize

}

proc createSINK { } {
    global channel propagation smask data_mask ns cbr_sink position_sink node_sink udp_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator
    global woss_utilities woss_creator propagation_sink lat_sink lon_sink db_manager

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set cbr_sink($cnt)  [new Module/UW/CBR] 
    }
    set udp_sink      [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/POLLING/SINK]
    set phy_data_sink  [new Module/UW/PHYSICAL]

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} { 
       $node_sink addModule 7 $cbr_sink($cnt) 0 "CBR"
    }
	$node_sink addModule 6 $udp_sink      0 "PRT"
	$node_sink addModule 5 $ipr_sink       0 "IPR"
	$node_sink addModule 4 $ipif_sink      0 "IPF"   
	$node_sink addModule 3 $mll_sink       0 "MLL"
	$node_sink addModule 2 $mac_sink       1 "MAC"
	$node_sink addModule 1 $phy_data_sink  1 "PHY"

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       $node_sink setConnection $cbr_sink($cnt)  $udp_sink     1
    }
    $node_sink setConnection $udp_sink $ipr_sink      	0
    $node_sink setConnection $ipr_sink  $ipif_sink       	0
    $node_sink setConnection $ipif_sink $mll_sink        	0 
    $node_sink setConnection $mll_sink  $mac_sink        	0
    $node_sink setConnection $mac_sink  $phy_data_sink   	1
    $node_sink addToChannel $channel    $phy_data_sink   	1

	for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_sink($cnt) [$udp_sink assignPort $cbr_sink($cnt)]
    }
	 
	#set lat_sink 53.54285
    #set lon_sink 9.94066
    set curr_depth [ expr [ $db_manager getBathymetry $lat_sink $lon_sink]  ]
    set curr_depth [expr $curr_depth - $curr_depth/2.0]
	#set curr_depth 1.0

    set position_sink [new "WOSS/Position"]
    $node_sink addPosition $position_sink
    set posdb_sink [new "PlugIn/PositionDB"]
    $node_sink addPlugin $posdb_sink 20 "PDB"
    $posdb_sink addpos [$ipif_sink addr] $position_sink

    $position_sink setLatitude_  $lat_sink
    $position_sink setLongitude_ $lon_sink
    $position_sink setAltitude_  [expr -1.0 * $curr_depth]  

    puts "node sink at ([$position_sink getLatitude_], [$position_sink getLongitude_], [$position_sink getAltitude_]) , ([$position_sink getX_], [$position_sink getY_], [$position_sink getZ_])"


    $ipif_sink addr 252      

    set interf_data_sink [new Module/UW/INTERFERENCE]
    $interf_data_sink set maxinterval_ $opt(maxinterval_)
    $interf_data_sink set debug_       0

    $phy_data_sink setSpectralMask     $data_mask
    $phy_data_sink setPropagation      $propagation
    $phy_data_sink setInterference     $interf_data_sink
	$phy_data_sink setInterferenceModel "MEANPOWER"

    $mac_sink initialize
}


###############################
# routing of nodes
###############################

proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_auv portnum_auv ipr_sink
	global ipif_sink portnum_sink ipr_auv

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $ipr($id1) addRoute [$ipif_sink addr] [$ipif_auv addr]
	$ipr_auv  addRoute [$ipif_sink addr] [$ipif_sink addr]
    $ipr_sink  addRoute [$ipif($id1) addr] [$ipif($id1) addr]
}

###############################
# create nodes, sink, auv
###############################

for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	createNode $id1
}
createSINK
createAUV

################################
#Setup flows
################################
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	connectNodes $id1
}

################################
#fill ARP tables
################################

for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	for {set id2 0} {$id2 < $opt(nn)} {incr id2} {
		$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]	
	}
	$mll($id1) addentry [$ipif_auv addr] [$mac_auv addr]
	$mll($id1) addentry [$ipif_sink addr] [$mac_sink addr]
	$mll_sink addentry [$ipif($id1) addr] [$mac($id1) addr]
	$mll_auv addentry [$ipif($id1) addr] [$mac($id1) addr]
}
$mll_sink addentry [$ipif_sink addr] [$mac_sink addr]
$mll_sink addentry [$ipif_auv addr] [$mac_auv addr]
$mll_auv addentry [$ipif_sink addr] [$mac_sink addr]
$mll_auv addentry [$ipif_auv addr] [$mac_auv addr]

################################
#Waypoints AUV
################################
proc createSinkWaypoints { } {
 	global position_auv opt position woss_utilities db_manager ns lat_wp lon_wp
	global lat_last_wp lon_last_wp toa

	for {set lap 0} {$lap < $opt(n_laps)} {incr lap} {
		for {set id_wp 0} {$id_wp < $opt(n_wp)} {incr id_wp} {
			if {$lap == 0 && $id_wp == 0} {
				set toa 0.0
				set curr_lat $lat_wp($id_wp) 
				set curr_lon $lon_wp($id_wp)
				set curr_depth [ expr [ $db_manager getBathymetry $curr_lat $curr_lon]  ]
    			set curr_depth [expr $curr_depth - $curr_depth/2.0]
				$position_auv addStartWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0
				puts "waypoint [expr 1 + $id_wp + $lap * $opt(n_wp)] lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"
			} else {
  				set toa 0.0
				set curr_lat $lat_wp($id_wp) 
				set curr_lon $lon_wp($id_wp)
				set curr_depth [ expr [ $db_manager getBathymetry $curr_lat $curr_lon]  ]
    			set curr_depth [expr $curr_depth - $curr_depth/2.0]
				set toa      [$position_auv addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
				puts "waypoint [expr 1 + $id_wp + $lap * $opt(n_wp)] lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"
			}
		}
	}
	set toa 0.0
	set curr_lat $lat_wp(0)
	set curr_lon $lon_wp(0)
	set curr_depth [ expr [ $db_manager getBathymetry $curr_lat $curr_lon]  ]
    set curr_depth [expr $curr_depth - $curr_depth/2.0]
	set toa      [$position_auv addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
	puts "waypoint [expr 1 + $opt(n_wp) + ($opt(n_laps) - 1) * $opt(n_wp)] lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"

	set toa 0.0
	set curr_lat $lat_last_wp
	set curr_lon $lon_last_wp
	set curr_depth [ expr [ $db_manager getBathymetry $curr_lat $curr_lon]  ]
    set curr_depth [expr $curr_depth - $curr_depth/2.0]
	set toa      [$position_auv addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) 0.0]
	puts "waypoint LAST(SINK)  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; toa = $toa"

  #1 TOTAL LOOPS
  #0.0 loop_id
}

createSinkWaypoints
set opt(stoptime) [expr $toa + 50]
set opt(txduration)     	[expr $opt(stoptime) - $opt(starttime)]
puts "stop $opt(stoptime)"


################################
#Start cbr(s)
################################

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)			"$cbr($id1) start"
    $ns at $opt(stoptime)   		"$cbr($id1) stop"
}
$ns at $opt(starttime) "$mac_auv run"

proc finish { } {
    global ns opt cbr outfile
    global mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
	global cbr_auv mac_auv phy_data_auv udp_sink ipif_sink
    if {$opt(verbose)} {
    	puts "CBR_PERIOD : $opt(cbr_period)"
    	puts "SEED: $opt(rep_num)"
    	puts "NUMBER OF SENSOR NODES: $opt(nn)"
    	puts "OVERALL NUMBER OF NODES: $opt(total_nn)"
		puts "START TIME: $opt(starttime)"
		puts "STOP TIME: $opt(stoptime)"

    } else {
        puts "End of simulation"
    }
	set cbr_header	[$cbr_sink(0) getcbrheadersize]
	set udp_header	[$udp_sink getudpheadersize]
	set ipif_header	[$ipif_sink getipheadersize]
	if {$opt(verbose)} {
		puts "HEADERS SIZE"
		puts "cbr header : $cbr_header \[bytes\]"
		puts "udp header : $udp_header \[bytes\]"
		puts "ip header : $ipif_header \[bytes\]"
	}
    set sum_cbr_throughput 	0
    set sum_per		0
    set sum_cbr_sent_pkts	0.0
    set sum_cbr_rcv_pkts	0.0
    set sum_mac_sent_pkts     0.0
	set sum_mac_probe_sent 0.0
    set cbr_rcv_pkts		0.0
    set mac_pkts		0.0
	
    for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		
		set cbr_pkts($id3)         	   [$cbr($id3) getsentpkts]
		set mac_tx_pkts($id3)         	   [$mac($id3) getDataPktsTx]
		set mac_probe_sent($id3)		[$mac($id3) getProbeSent]
		set mac_trigger_rx($id3)		[$mac($id3) getTriggerReceived]
		set mac_trigger_dropped($id3)	[$mac($id3) getTriggerDropped]
		set mac_poll_rx($id3)			[$mac($id3) getTimesPolled]
		set mac_poll_dropped($id3)		[$mac($id3) getPollDropped]
		set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_pkts($id3)]
		set sum_mac_sent_pkts [expr $sum_mac_sent_pkts + $mac_tx_pkts($id3) ]
		set sum_mac_probe_sent [expr $sum_mac_probe_sent + $mac_probe_sent($id3)]
    }

	set cbr_sink_rcv 0.0
	for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		set cbr_sink_rcv_pkts($id3)       [$cbr_sink($id3) getrecvpkts]
		set cbr_throughput	   [$cbr_sink($id3) getthr]
		set cbr_per	           [$cbr_sink($id3) getper]
		set cbr_sink_rcv [expr $cbr_sink_rcv + $cbr_sink_rcv_pkts($id3)]
	}

    set mac_auv_rcv_pkts   [$mac_auv getDataPktsRx] 
	set mac_auv_sent_pkts   [$mac_auv getDataPktsTx] 
	set mac_auv_probe_rx	[$mac_auv getProbeReceived]
	set mac_auv_dropped_probe	[$mac_auv getDroppedProbePkts]
	set mac_auv_trigger_sent	[$mac_auv getTriggerSent]
	set mac_auv_poll_sent [$mac_auv getPollSent]
	set mac_auv_pkts_buff_overflow [$mac_auv getDiscardedPktsTx]
	for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		set mac_auv_rx_per_node($id3) [$mac_auv getRxFromNode [$mac($id3) addr]]
	}
	

	set mac_sink_rcv_pkts  [$mac_sink getDataPktsRx]
	set mac_sink_duplicated	[$mac_sink getDuplicatedPkts]
	set mac_sink_probe_sent [$mac_sink getProbeSent]
	set mac_sink_trigger_rx		[$mac_sink getTriggerReceived]
	set mac_sink_trigger_dropped	[$mac_sink getTriggerDropped]
	set sum_mac_probe_sent [expr $sum_mac_probe_sent + $mac_sink_probe_sent]

	for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
		set cbr_sink_delay($id3)       [$cbr_sink($id3) getftt]
		set cbr_sink_delay_std($id3)	   [$cbr_sink($id3) getfttstd]
		set cbr_sink_thr($id3)			[$cbr_sink($id3) getthr]
	}
	
	if {$opt(verbose)} {
		puts "Backoff time : $opt(T_backoff)"
		puts "-------------------------------------------------------------------------------------------"
		puts "TRIGGER packets"
		puts "mac auv sent trigger pkts : $mac_auv_trigger_sent"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) received trigger pkts : $mac_trigger_rx($id3)"
			puts "mac($id3) dropped trigger pkts : $mac_trigger_dropped($id3)"
		}
		puts "mac_sink received trigger pkts : $mac_sink_trigger_rx"
		puts "mac_sink dropped trigger pkts : $mac_sink_trigger_dropped"
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "PROBE packets"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) sent probe pkts : $mac_probe_sent($id3)"
		}
		puts "mac sink sent probe pkts : $mac_sink_probe_sent"
		puts "mac overall sent probe pkts : $sum_mac_probe_sent"
		puts "mac auv dropped probe	pkts : $mac_auv_dropped_probe"
		puts "mac auv received probe pkts : $mac_auv_probe_rx"
		puts "avg probe per round : [expr $mac_auv_probe_rx*1.0/$mac_auv_trigger_sent ]"
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "POLL packets"
		puts "mac auv sent poll pkts : $mac_auv_poll_sent"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) received poll pkts : $mac_poll_rx($id3)"
			puts "mac($id3) dropped poll pkts : $mac_poll_dropped($id3)"
		}
		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "MAC DATA packets"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "mac($id3) sent pkts : $mac_tx_pkts($id3) "
			puts "mac auv rcv from node $id3 : $mac_auv_rx_per_node($id3)"
		}
		puts "mac nodes overall sent pkts : $sum_mac_sent_pkts"
		puts "mac auv received packets : $mac_auv_rcv_pkts"
		puts "mac auv transmitted packets : $mac_auv_sent_pkts"
		if {$sum_mac_sent_pkts == 0} {
			puts "mac pdr : 0"
		} else {
			puts "mac pdr : [expr $mac_auv_rcv_pkts*1.0/$sum_mac_sent_pkts]"
		}
		puts "mac auv discarded packets buffer overflow : $mac_auv_pkts_buff_overflow"
		puts "mac sink received packets : $mac_sink_rcv_pkts"
		puts "mac sink duplicated packets : $mac_sink_duplicated"

		puts "-------------------------------------------------------------------------------------------"
	}
	if {$opt(verbose)} {
		puts "-------------------------------------------------------------------------------------------"
		puts "APP DATA packets"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "cbr($id3) sent pkts : $cbr_pkts($id3)"
			puts "cbr($id3) sink rcv pkts : $cbr_sink_rcv_pkts($id3)"
		}
		puts "cbr nodes overall sent pkts : $sum_cbr_sent_pkts"
		puts "cbr sink received pkts : $cbr_sink_rcv"
		puts "overall throughput : [expr $cbr_sink_rcv*$opt(pktsize) / $opt(txduration)] \[Byte/s\]"
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "node($id3)-sink throughput : $cbr_sink_thr($id3) \[Byte/s\]"
		}
		for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
			puts "node($id3)-sink delay : $cbr_sink_delay($id3) \[s\]"
			puts "node($id3)-sink delay std : $cbr_sink_delay_std($id3) \[s\]"
		}
		puts "-------------------------------------------------------------------------------------------"
	}

    set tot_time  	     [$mac_auv GetTotalReceivingTime]
    
    #if {$opt(verbose)} {  
    #	puts "---------------------------------------------------------------------"
    #	puts "Number of packets transmitted by CBR: [expr ($sum_cbr_sent_pkts)]"
    #	puts "Number of packets received by CBR: [expr ($sum_cbr_rcv_pkts)]"
    #	puts "------------------------------------------------------------------"
    #	puts "Number of packets received by MAC: $mac_auv_rcv_pkts"
    #	puts "Number of packets transmitted by MAC: $sum_mac_sent_pkts"
    #	puts "------------------------------------------------------------------"
    #	puts "Mean PER at CBR layer: [expr (1 - ($sum_cbr_rcv_pkts/($sum_cbr_sent_pkts)))]"
    #	puts "Mean PER at MAC layer: [expr (1- ($sum_cbr_rcv_pkts/($sum_mac_sent_pkts)))]"
	#
    #	puts "Total receiving time: $tot_time"
    #	puts "Mean throughput at MAC layer: [expr (($mac_auv_rcv_pkts*125*8)/($tot_time))]"
    #	puts "Mean throughput at CBR layer: $sum_cbr_throughput"
    #	puts "done!"
    #}

	set opt(end_clock) [clock seconds]
    puts  "done in [expr $opt(end_clock) - $opt(start_clock)] seconds!"

    $ns flush-trace
    close $opt(tracefile)
}


###################
# start simulation
###################

puts -nonewline "\nSimulating...\n"

$ns at [expr $opt(stoptime) + 150.0]  "$mac_auv stop_count_time"
$ns at [expr $opt(stoptime) + 150.0]  "finish; $ns halt" 

$ns run
    
