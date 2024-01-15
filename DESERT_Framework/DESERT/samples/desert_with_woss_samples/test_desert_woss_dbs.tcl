#
# Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University of Padova (SIGNET lab) nor the 
#    names of its contributors may be used to endorse or promote products 
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
# Author: Federico Favaro <fedefava86@gmail.com>
# Version: 2.0.0
#
#
#######################################################################################"
#                                  README                                             #"
#                                                                                     #"
# This example showcases the use of WOSS at the physical layer, along with DESERT's   #"
# Interference computation capabilities and network protocol stack. In particular,    #"
# this tcl sample requires the use of enviromental databases for SSP, bathymetry,     #"
# sediments, as well as for the characteristics of electro-acoustic transducers.      #"
# You can download the sediment and SSP databases at the following link:              #"
#     https://woss.dei.unipd.it/woss/files/WOSS-dbs-v1.6.0.tar.gz                     #"
# After the download, please set opt(db_path) to the correct path.                    #"
# (i.e. /usr/share/woss/dbs/ if you extracted WOSS databases in /usr/share/woss)      #"
# Please note that we cannot redistribute the GEBCO bathymetry database.              #"
# You can download the database by registering on the GEBCO web site at:              #"
#     https://www.bodc.ac.uk/data/open_download/gebco/gebco_2023/zip/                 #"
# After the download you can place the GEBCO file in the bathymetry folder of         #"
# WOSS databases.                                                                     #"
# Directory tree should be                                                            #"
#├── bathymetry                                                                       #"
#├── seafloor_sediment                                                                #"
#├── ssp                                                                              #"
#│   ├── WOA2001                                                                      #"
#│   ├── WOA2005                                                                      #"
#│   ├── WOA2009                                                                      #"
#│   ├── WOA2013                                                                      #"
#│   └── WOA2018                                                                      #"
#└── transducers                                                                      #"
#    ├── BTech                                                                        #"
#    ├── ITC                                                                          #"
#    ├── ITT                                                                          #"
#    ├── Neptune                                                                      #"
#    └── RESON                                                                        #"
#                                                                                     #"
#######################################################################################"
#
# Stack of the nodes
#   +-------------------------+
#   |  7. UW/CBR              |
#   +-------------------------+
#   |  6. UW/UDP              |
#   +-------------------------+
#   |  5. UW/STATICROUTING    |
#   +-------------------------+
#   |  4. UW/IP               |
#   +-------------------------+
#   |  3. UW/MLL              |
#   +-------------------------+
#   |  2. UW/CSMA_ALOHA       |
#   +-------------------------+
#   |  1. UW/PHYSICAL         |
#   +-------------------------+
#           |         |    
#   +-------------------------+
#   |    UnderwaterChannel    |
#   +-------------------------+

######################################
# Flags to enable or disable options #
######################################
set opt(verbose) 		1
set opt(trace_files)		0
set opt(bash_parameters) 	0

#####################
# Library Loading   #
#####################
load libMiracle.so
load libmphy.so
load libmmac.so
load libMiracleBasicMovement.so
load libUwmStd.so
load libWOSS.so
load libWOSSPhy.so
load libuwinterference.so
load libuwphy_clmsgs.so
load libuwstats_utilities.so
load libuwphysical.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwcsmaaloha.so


#############################
# NS-Miracle initialization #
#############################
set ns [new Simulator]
$ns use-Miracle

##################
# Tcl variables  #
##################

set opt(start_lat)      42.59 
set opt(start_long)     10.125
set opt(nn)             4.0    ;#Number of nodes
set opt(pktsize)        125     ;# Size of the packet in Bytes
set opt(stoptime)       100000  
set opt(dist_nodes)     200.0  ;# Distace of the nodes in m
set opt(nn_in_row)      2       ;# Number of a nodes in m
set opt(ack_mode)       "setNoAckMode"
set opt(rngstream)      1
set opt(cbr_period)     100
set opt(db_path)        "./dbs/"


if {$opt(bash_parameters)} {
    if {$argc != 3} {
        puts "Tcl example need three parameters"
        puts "- the random generator substream"
        puts "- CBR period"
        puts "- dbs path"
        puts "For example, ns test_desert_woss_dbs.tcl 4 100 /usr/share/woss/dbs/"
        puts "If you want to leave the default values, please set to 0"
        puts "the value opt(bash_parameters) in the tcl script"
        puts "Please try again."
        exit
    } else { 
        set opt(rngstream)		[lindex $argv 0]
        set opt(cbr_period)	[lindex $argv 1]
        set opt(db_path) [lindex $argv 2]
    }
}

global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
  $defaultRNG next-substream
}

set opt(starttime)          0.1
set opt(txduration)         [expr $opt(stoptime) - $opt(starttime)]
set opt(extra_time)         250.0

#PHY PARAMETERS:

set opt(txpower)               190
set opt(per_tgt)               0.01
set opt(rx_snr_penalty_db)     -10.0
set opt(tx_margin_db)          10.0

set opt(node_min_angle)		 -90.0
set opt(node_max_angle)		  90.0
set opt(sink_min_angle)		 -90.0
set opt(sink_max_angle) 	  90.0
set opt(node_bathy_offset)	 -2.0

set opt(maxinterval_)    	  11500.0
set opt(freq) 			  25000.0
set opt(bw)              	  5000.0
set opt(bitrate)	 	  4800.0


########################
#     TRACE FILES      #
########################
if {$opt(trace_files)} {
	
	set opt(tracefilename) "./test_desert_woss_dbs_dbs.tr"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "./test_desert_woss_dbs_dbs.tr"
	set opt(cltracefile) [open $opt(cltracefilename) w]
} else {
	set opt(tracefilename) "/dev/null"
	set opt(tracefile) [open $opt(tracefilename) w]
	set opt(cltracefilename) "/dev/null/"
	set opt(cltracefile) [open $opt(cltracefilename) w]
}

set opt(db_res_path) "./test_desert_woss_dbs"



set exists_ssp [file exists "$opt(db_path)/ssp/WOA2018/WOA2018_SSP_April.nc"]
set exists_gebco [file exists "$opt(db_path)/bathymetry/GEBCO_2023.nc"]

if { $exists_ssp == 0 || $exists_gebco == 0 } {
    puts "#######################################################################################"
    puts "# Database files not found.                                                           #"
    puts "# You can download the sediment and SSP databases at the following link:              #"
    puts "#     https://woss.dei.unipd.it/woss/files/WOSS-dbs-v1.6.0.tar.gz                     #"
    puts "# After the download, please set opt(db_path) to the correct path.                    #"
    puts "# (i.e. /usr/share/woss/dbs/ if you extracted WOSS databases in /usr/share/woss)      #"
    puts "# (i.e. /usr/share/dbs/ if you extracted WOSS databases in /usr/share/)               #"
    puts "# Please note that we cannot redistribute the GEBCO bathymetry database.              #"
    puts "# You can download GEBCO database at:                                                 #"
    puts "#     https://www.bodc.ac.uk/data/open_download/gebco/gebco_2023/zip/                 #"
    puts "# After the download you can place the GEBCO file in the bathymetry folder of         #"
    puts "# WOSS databases .                                                                    #"
    puts "# Directory tree should be                                                            #"
    puts "#├── bathymetry                                                                       #"
    puts "#├── seafloor_sediment                                                                #"
    puts "#├── ssp                                                                              #"
    puts "#│   ├── WOA2001                                                                      #"
    puts "#│   ├── WOA2005                                                                      #"
    puts "#│   ├── WOA2009                                                                      #"
    puts "#│   ├── WOA2013                                                                      #"
    puts "#│   └── WOA2018                                                                      #"
    puts "#└── transducers                                                                      #"
    puts "#    ├── BTech                                                                        #"
    puts "#    ├── ITC                                                                          #"
    puts "#    ├── ITT                                                                          #"
    puts "#    ├── Neptune                                                                      #"
    puts "#    └── RESON                                                                        #"
    puts "#                                                                                     #"
    puts "#######################################################################################"

    exit
}

WOSS/Definitions/RandomGenerator/NS2 set rep_number_ $opt(rngstream)

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

WOSS/Creator/Database/Binary/Results/TimeArr set debug           0
WOSS/Creator/Database/Binary/Results/TimeArr set woss_db_debug   0
WOSS/Creator/Database/Binary/Results/TimeArr set space_sampling 0.0

set db_res_arr [new "WOSS/Creator/Database/Binary/Results/TimeArr"]
$db_res_arr setDbPathName "${opt(db_res_path)}/test_aloha_with_dbs_res_arr.dat"


WOSS/Creator/Database/NetCDF/Sediment/DECK41 set debug         0
WOSS/Creator/Database/NetCDF/Sediment/DECK41 set woss_db_debug 0


set db_sedim [new "WOSS/Creator/Database/NetCDF/Sediment/DECK41"]
$db_sedim setDeck41DbTypeV2
$db_sedim setUpDeck41CoordinatesDb  "${opt(db_path)}/seafloor_sediment/DECK41_V2_coordinates.nc"
$db_sedim setUpDeck41MarsdenDb      "${opt(db_path)}/seafloor_sediment/DECK41_V2_marsden_square.nc"
$db_sedim setUpDeck41MarsdenOneDb   "${opt(db_path)}/seafloor_sediment/DECK41_V2_marsden_one_degree.nc"


WOSS/Creator/Database/NetCDF/SSP/WOA2013/MonthlyAverage set debug          0
WOSS/Creator/Database/NetCDF/SSP/WOA2013/MonthlyAverage set woss_db_debug  0

set db_ssp [new "WOSS/Creator/Database/NetCDF/SSP/WOA2013/MonthlyAverage"]
$db_ssp setDbPathName "${opt(db_path)}/ssp/WOA2018/WOA2018_SSP_June.nc"


WOSS/Creator/Database/NetCDF/Bathymetry/GEBCO set debug           0
WOSS/Creator/Database/NetCDF/Bathymetry/GEBCO set woss_db_debug   0

set db_bathy [new "WOSS/Creator/Database/NetCDF/Bathymetry/GEBCO"]
$db_bathy setDbPathName "${opt(db_path)}/bathymetry/GEBCO_2023.nc"
$db_bathy use2DFifteenSecondsPrecision


WOSS/Definitions/Altimetry/Flat set evolution_time_quantum   -1
WOSS/Definitions/Altimetry/Flat set range                    -1
WOSS/Definitions/Altimetry/Flat set total_range_steps        -1
WOSS/Definitions/Altimetry/Flat set depth                    0.0
set cust_altimetry   [new "WOSS/Definitions/Altimetry/Flat"]


WOSS/Database/Manager set debug 0
set db_manager [new "WOSS/Database/Manager"]

WOSS/Creator/Bellhop set debug                        0.0
WOSS/Creator/Bellhop set woss_debug                   0.0
WOSS/Creator/Bellhop set woss_clean_workdir           1.0
WOSS/Creator/Bellhop set bellhop_arr_syntax           2
WOSS/Creator/Bellhop set evolution_time_quantum      -1.0
WOSS/Creator/Bellhop set total_runs                   5
WOSS/Creator/Bellhop set frequency_step               0.0
WOSS/Creator/Bellhop set total_range_steps            3000.0
WOSS/Creator/Bellhop set tx_min_depth_offset          0.0
WOSS/Creator/Bellhop set tx_max_depth_offset          0.0
WOSS/Creator/Bellhop set total_transmitters           1
WOSS/Creator/Bellhop set total_rx_depths              2
WOSS/Creator/Bellhop set rx_min_depth_offset          -0.1
WOSS/Creator/Bellhop set rx_max_depth_offset          0.1
WOSS/Creator/Bellhop set total_rx_ranges              2
WOSS/Creator/Bellhop set rx_min_range_offset          -0.1
WOSS/Creator/Bellhop set rx_max_range_offset          0.1
WOSS/Creator/Bellhop set total_rays                   0.0
WOSS/Creator/Bellhop set min_angle                    -45.0
WOSS/Creator/Bellhop set max_angle                    45.0
WOSS/Creator/Bellhop set ssp_depth_precision          1.0e-8
WOSS/Creator/Bellhop set normalized_ssp_depth_steps   100000


set woss_creator [new "WOSS/Creator/Bellhop"]
$woss_creator setWorkDirPath        "./test_desert_woss_dbs/"
$woss_creator setBellhopPath        ""
$woss_creator setBellhopMode        0 0 "A"
$woss_creator setBeamOptions        0 0 "B"
$woss_creator setBathymetryType     0 0 "L"
$woss_creator setBathymetryMethod   0 0 "S"
$woss_creator setAltimetryType      0 0 "L"
$woss_creator setSimulationTimes    0 0 1 1 2013 0 0 1 2 1 2013 0 0 1

WOSS/Manager/Simple/MultiThread set debug                     0
WOSS/Manager/Simple/MultiThread set is_time_evolution_active -1.0
WOSS/Manager/Simple/MultiThread set space_sampling            0.0
set woss_manager [new "WOSS/Manager/Simple/MultiThread"]
$woss_manager setConcurrentThreads 0


WOSS/Utilities set debug 0
set woss_utilities [new "WOSS/Utilities"]


WOSS/Definitions/TransducerHandler set debug 0
set transducer_handler [new WOSS/Definitions/TransducerHandler]

WOSS/Controller set debug 0
set woss_controller [new "WOSS/Controller"]
$woss_controller setBathymetryDbCreator      $db_bathy
$woss_controller setSedimentDbCreator        $db_sedim
$woss_controller setSSPDbCreator             $db_ssp
$woss_controller setTimeArrResultsDbCreator  $db_res_arr
$woss_controller setWossDbManager            $db_manager
$woss_controller setWossManager              $woss_manager
$woss_controller setWossCreator              $woss_creator
$woss_controller setTransducerhandler        $transducer_handler
$woss_controller initialize


WOSS/PlugIn/ChannelEstimator set debug_ 0.0

WOSS/ChannelEstimator set debug_           0.0
WOSS/ChannelEstimator set space_sampling_  0.0
WOSS/ChannelEstimator set avg_coeff_       0.5
set channel_estimator [ new "WOSS/ChannelEstimator"]


WOSS/Module/Channel set channel_eq_snr_threshold_db_ 	 0
WOSS/Module/Channel set channel_symbol_resolution_   5e-3
WOSS/Module/Channel set channel_eq_time_ 	 -1
WOSS/Module/Channel set debug_                    	 0
WOSS/Module/Channel set windspeed_                       0.0
WOSS/Module/Channel set shipping_                        0.0
WOSS/Module/Channel set practical_spreading_             1.75
WOSS/Module/Channel set prop_speed_                      1500.0

set channel [new "WOSS/Module/Channel"]
$channel setWossManager      $woss_manager
$channel setChannelEstimator $channel_estimator

WOSS/MPropagation set debug_ 0
set propagation [new "WOSS/MPropagation"]
$propagation setWossManager $woss_manager


set data_mask [new "MSpectralMask/Rect"]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

Module/UW/CBR set debug_		       0
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1

#### MaxTxSPL_dB_ was previously named MaxTxPower_dB_
#### MinTxSPL_dB_ was previously named MinTxPower_dB_
#### TxSPLMargin_dB_ was previously named TxPowerMargin_dB_
#### SPLOptimization_, CentralFreqOptimization_, BandwidthOptimization_ added
WOSS/Module/MPhy/BPSK  set debug_                     0
WOSS/Module/MPhy/BPSK  set AcquisitionThreshold_dB_   10.0 
WOSS/Module/MPhy/BPSK  set BitRate_                   $opt(bitrate)
WOSS/Module/MPhy/BPSK  set MaxTxSPL_dB_               $opt(txpower)
WOSS/Module/MPhy/BPSK  set MinTxSPL_dB_               10
WOSS/Module/MPhy/BPSK  set MaxTxRange_                10000
WOSS/Module/MPhy/BPSK  set PER_target_                $opt(per_tgt)
WOSS/Module/MPhy/BPSK  set TxSPLMargin_dB_            $opt(tx_margin_db)
WOSS/Module/MPhy/BPSK  set RxSnrPenalty_dB_           $opt(rx_snr_penalty_db)
WOSS/Module/MPhy/BPSK  set SPLOptimization_           1
WOSS/Module/MPhy/BPSK  set CentralFreqOptimization_   0
WOSS/Module/MPhy/BPSK  set BandwidthOptimization_     0


proc createNode { id } {
    
    global channel propagation data_mask ns cbr position node port portnum ipr ipif channel_estimator
    global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager
    global row
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    set cbr($id)       [new Module/UW/CBR] 
    set port($id)      [new Module/UW/UDP]
    set ipr($id)       [new Module/UW/StaticRouting]
    set ipif($id)      [new Module/UW/IP]
    set mll($id)       [new Module/UW/MLL] 
    set mac($id)       [new Module/UW/CSMA_ALOHA]
    set phy_data($id)  [new WOSS/Module/MPhy/BPSK]


    $node($id)  addModule 7 $cbr($id)   1  "CBR"
    $node($id)  addModule 6 $port($id)  1  "PRT"
    $node($id)  addModule 5 $ipr($id)   1  "IPR"
    $node($id)  addModule 4 $ipif($id)  1  "IPF"   
    $node($id)  addModule 3 $mll($id)   1  "MLL"
    $node($id)  addModule 2 $mac($id)   1  "MAC"
    $node($id)  addModule 1 $phy_data($id)   1  "PHY"

    $node($id) setConnection $cbr($id)   $port($id)  0
    $node($id) setConnection $port($id)  $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy_data($id)   1
    $node($id) addToChannel  $channel    $phy_data($id)   1


    set portnum($id) [$port($id) assignPort $cbr($id) ]

    $ipif($id) addr [expr $id + 1]

    set position($id) [new "WOSS/Position"]
    $node($id) addPosition $position($id)
     ##########################
     #  NODES PLACEMENT	      #
     ##########################
    set curr_x 0.0
    set curr_y 0.0
    if { $id < 5 } {
    	set curr_x [expr $opt(dist_nodes) * $id ]
    	set row 0
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 5)  &&  ($id < 10) } {
    	set curr_x  [expr ($id -($opt(nn)/5 )) * $opt(dist_nodes) ]
    	set row 1
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 10) && ($id < 15) } {
	set curr_x  [expr ($id -($opt(nn)/5 )*2) * $opt(dist_nodes) ]
    	set row 2
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 15) && ($id < 20) } {
	set curr_x  [expr ($id -($opt(nn)/5 )*3) * $opt(dist_nodes) ]
    	set row 3
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    } elseif { ($id >= 20) && ($id < 25) } {
	set curr_x  [expr ($id -($opt(nn)/5 )*4) * $opt(dist_nodes) ]
    	set row 4
    	set curr_y  [expr $row * $opt(dist_nodes) ]
    }

    set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
    set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $curr_x ]
    set curr_depth 50
    puts "Coordinates: x: $curr_x, y: $curr_y, depth: $curr_depth"

    
    $position($id) setLatitude_  $curr_lat
    $position($id) setLongitude_ $curr_lon
    $position($id) setAltitude_  [expr -1.0 * $curr_depth]

    $woss_creator setCustomAngles $position($id) 0 $opt(node_min_angle) $opt(node_max_angle)

    set ch_estimator_plugin($id) [ new "WOSS/PlugIn/ChannelEstimator"]
    $ch_estimator_plugin($id) setChannelEstimator $channel_estimator
    $ch_estimator_plugin($id) insertNode [$mac($id) addr] $position($id)
    $node($id) addPlugin $ch_estimator_plugin($id) 19 "CHE"


    puts "node $id at ([$position($id) getLatitude_], [$position($id) getLongitude_], [$position($id) getAltitude_]) , ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"


    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0


    
    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
    $phy_data($id) set debug_ 0


    $mac($id) $opt(ack_mode)
    $mac($id) initialize

}

proc createSink { } {
    global channel propagation smask data_mask ns cbr_sink position_sink node_sink port_sink portnum_sink interf_data_sink
    global phy_data_sink posdb_sink opt mll_sink mac_sink ipr_sink ipif_sink bpsk interf_sink channel_estimator
    global woss_utilities woss_creator
    global auv_curr_depth

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
	   set cbr_sink($cnt)  [new Module/UW/CBR] 
    }
    
    set port_sink      [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL]
    set mac_sink       [new Module/UW/CSMA_ALOHA]
    set phy_data_sink  [new WOSS/Module/MPhy/BPSK]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
	$node_sink addModule 7 $cbr_sink($cnt) 0 "CBR"
     }
     $node_sink addModule 6 $port_sink      0 "PRT"
     $node_sink addModule 5 $ipr_sink       0 "IPR"
     $node_sink addModule 4 $ipif_sink      0 "IPF"   
     $node_sink addModule 3 $mll_sink       0 "MLL"
     $node_sink addModule 2 $mac_sink       1 "MAC"
     $node_sink addModule 1 $phy_data_sink  1 "PHY"

     for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
     $node_sink setConnection $cbr_sink($cnt)  $port_sink     	1   
      }
     $node_sink setConnection $port_sink $ipr_sink      	0
     $node_sink setConnection $ipr_sink  $ipif_sink       	0
     $node_sink setConnection $ipif_sink $mll_sink        	0 
     $node_sink setConnection $mll_sink  $mac_sink        	0
     $node_sink setConnection $mac_sink  $phy_data_sink   	1
     $node_sink addToChannel $channel    $phy_data_sink   	1

     for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set portnum_sink($cnt) [$port_sink assignPort $cbr_sink($cnt)]
     }

     set sink_depth 10
     
     set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 100 ]
     set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  100]

     set position_sink [new "WOSS/Position"]
     $node_sink addPosition $position_sink

     $position_sink setLatitude_  $curr_lat
     $position_sink setLongitude_ $curr_lon
     $position_sink setAltitude_  [expr -1.0 * $sink_depth]

     $woss_creator setCustomAngles $position_sink 0 $opt(sink_min_angle) $opt(sink_max_angle)

     $ipif_sink addr 253

     set ch_estimator_plugin_sink [ new "WOSS/PlugIn/ChannelEstimator"]
     $ch_estimator_plugin_sink setChannelEstimator $channel_estimator
     $ch_estimator_plugin_sink insertNode [$mac_sink addr] $position_sink
     $node_sink addPlugin $ch_estimator_plugin_sink 19 "CHE"

    
      
     puts "node sink at ([$position_sink getLatitude_], [$position_sink getLongitude_], [$position_sink getAltitude_]) , ([$position_sink getX_], [$position_sink getY_], [$position_sink getZ_])"

     set interf_data_sink [new "Module/UW/INTERFERENCE"]
     $interf_data_sink set maxinterval_ $opt(maxinterval_)
     $interf_data_sink set debug_       0

     $phy_data_sink setSpectralMask     $data_mask
     $phy_data_sink setPropagation      $propagation
     $phy_data_sink setInterference     $interf_data_sink


     $mac_sink $opt(ack_mode)
     $mac_sink initialize

}

###############################
# routing of nodes
###############################

proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    $cbr_sink($id1) set destPort_ $portnum($id1)  
    $ipr($id1) addRoute [$ipif_sink addr] [$ipif_sink addr]
    $ipr_sink addRoute [$ipif($id1) addr] [$ipif($id1) addr]
}

###############################
# create nodes
###############################

for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}

###############################
# create sink
###############################

createSink
################################
#Setup flows
################################

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    connectNodes $id1
}

################################
#fill ARP tables
################################

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
	$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }   
    $mll($id1) addentry [$ipif_sink addr] [ $mac_sink addr]
    $mll_sink addentry [$ipif($id1) addr] [ $mac($id1) addr]
}


################################
#Start cbr(s)
################################


set force_stop $opt(stoptime)

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
      $ns at $opt(starttime)	     "$cbr($id1) start"
      $ns at $opt(stoptime)          "$cbr($id1) stop"
}




proc finish { } {
    global ns opt cbr mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global woss_manager outfile outfile_sink

    if {$opt(verbose)} {
            puts "\n"
            puts "CBR_PERIOD : $opt(cbr_period)"
            puts "SEED: $opt(rngstream)"
            puts "NUMBER OF NODES: $opt(nn)"
    } else {
            puts "Simulation done!"
    }

    set sum_cbr_throughput 	0
    set sum_per		0
    set sum_cbr_sent_pkts	0.0
    set sum_cbr_rcv_pkts	0.0
    set consumed_energy_tx_node	0.0
    set consumed_energy_rx_node	0.0
    set stdby_time		0.0
    set total_stdby_time	0.0

	
    for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {
	set cbr_throughput	   [$cbr_sink($id3) getthr]
	set cbr_per	           [$cbr_sink($id3) getper]
	set cbr_pkts         [$cbr($id3) getsentpkts]
	set cbr_rcv_pkts       [$cbr_sink($id3) getrecvpkts]
	################################################
	set ftt					[$cbr_sink($id3) getftt]
	set ftt_std				[$cbr_sink($id3) getfttstd]
	#################################################
	if {$opt(verbose)} {
		puts "cbr_sink($id3) throughput                : $cbr_throughput"
		puts "cbr_sink($id3) packet error rate         : $cbr_per"
		puts "cbr($id3) sent packets 	       	       : $cbr_pkts"
		puts "cbr_sink($id3) received packets          : $cbr_rcv_pkts"
	}
	

	set sum_cbr_throughput [expr $sum_cbr_throughput + $cbr_throughput]
	set sum_per [expr $sum_per + $cbr_per]
	set sum_cbr_sent_pkts [expr $sum_cbr_sent_pkts + $cbr_pkts]
	set sum_cbr_rcv_pkts  [expr $sum_cbr_rcv_pkts + $cbr_rcv_pkts]
	###############################################
	###############################################
    }
    if {$opt(verbose)} {
		puts "Mean throughput:  [expr ($sum_cbr_throughput/($opt(nn)))]"
		puts "Total transmitted packets: [expr ($sum_cbr_sent_pkts)]"
		puts "Total received packets: [expr ($sum_cbr_rcv_pkts)]"
		puts "Mean PER: [expr (1 - ($sum_cbr_rcv_pkts/($sum_cbr_sent_pkts)))]"
	}
    $ns flush-trace
    close $opt(tracefile)

    puts "Delete WOSS objects to force file operations"
    delete $woss_manager
    delete $db_manager
}


###################
# start simulation
###################

puts -nonewline "\nSimulating...\n"

$ns at [expr $opt(stoptime) + $opt(extra_time)]  "finish; $ns halt" 

$ns run
