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
# Author: Federico Favaro
# Version: 1.0.0


#######################################################################################"
#                                  README                                             #"
#                                                                                     #"
# This example showcases the use of WOSS at the physical layer, along with DESERT's   #"
# Interference computation capabilities and network protocol stack. In particular,    #"
# this tcl sample requires the use of enviromental databases for SSP, bathymetry      #"
# of the Hamburg port, sediments, as well as for the characteristics of               #"
# electro-acoustic transducers.                                                       #"
# You can download the databases at the following link:                               #"
#     https://woss.dei.unipd.it/woss/files/WOSS-dbs-v1.6.0.tar.gz                     #"
# After the download, please set opt(db_path) to the correct path.                    #"
# (i.e. /usr/share/woss/dbs/ if you extracted WOSS databases in /usr/share/woss)      #"
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

######################################
# Flags to enable or disable options #
######################################
set opt(verbose)          1
set opt(trace_files)      0
set opt(bash_parameters)  0


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
load libuwstats_utilities.so
load libuwphysical.so
load libuwcsmaaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
load libuwphy_clmsgs.so
load libuwcsmaaloha.so


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
set opt(nn)            2
set opt(pktsize)       512
set opt(cbr_period)    400
set opt(stoptime)      100000.0
set opt(rngstream)       1
set opt(ack_mode) "setNoAckMode"

set opt(node_bathy_offset) 0.5 

#sink depth [m]
set opt(sink_depth)           5
set opt(sink_min_angle)       0.0
set opt(sink_max_angle)       90.0
set opt(node_min_angle)       -90.0
set opt(node_max_angle)       0.0

set opt(starttime)         0.1
set opt(txduration) [expr $opt(stoptime) - $opt(starttime)]

set opt(maxinterval_)   500.0

set opt(freq)           11500.0
set opt(bw)             5000.0
set opt(bitrate)        4800.0

#set opt(tracefilename) "/tmp/${argv0}.tr"
set opt(tracefilename) "/dev/null"
set opt(tracefile) [open $opt(tracefilename) w]

#set opt(cltracefilename) "/tmp/${argv0}.cltr"
set opt(cltracefilename) "/dev/null"
set opt(cltracefile) [open $opt(cltracefilename) w]

set opt(db_path) "/usr/share/woss/dbs"

set opt(db_res_path)    "./bellhop_out_hamburg_port"

if {$opt(bash_parameters)} {
  if {$argc != 3} {
    puts "The script requires 3 params"
    puts "- the random generator substream"
    puts "- CBR packet generation period"
    puts "- DBs path"
    puts "For example, ns test_uwpolling.tcl 1 10 /usr/share/woss/dbs"
    puts "If you want to leave the default values, please set to 0"
    puts "the value opt(bash_parameters) in the tcl script"
    puts "Please try again."
  }   else { 
    set opt(rngstream)    [lindex $argv 0]
    set opt(cbr_period)   [lindex $argv 1]
    set opt(db_path)      [lindex $argv 2]
  }
}

set exists_dbs [file exists "$opt(db_path)/bathymetry/hamburg_port.csv"]

if { $exists_dbs == 0 } {
    puts "#######################################################################################"
    puts "Database files not found.                                                             #"
    puts "# You can download the sediment and SSP databases at the following link:              #"
    puts "#     https://woss.dei.unipd.it/woss/files/WOSS-dbs-v1.6.0.tar.gz                     #"
    puts "# After the download, please set opt(db_path) to the correct path.                    #"
    puts "# (i.e. /usr/share/woss/dbs/ if you extracted WOSS databases in /usr/share/woss)      #"
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
WOSS/Definitions/RandomGenerator/C   set seed_       $opt(rngstream)

set ssp_creator         [new "WOSS/Definitions/SSP"]
set sediment_creator    [new "WOSS/Definitions/Sediment"]
set pressure_creator    [new "WOSS/Definitions/Pressure"]
set time_arr_creator    [new "WOSS/Definitions/TimeArr"]
set time_reference      [new "WOSS/Definitions/TimeReference/NS2"]
set transducer_creator  [new "WOSS/Definitions/Transducer"]
set altimetry_creator   [new "WOSS/Definitions/Altimetry/Flat"]
#set altimetry_creator   [new "WOSS/Definitions/Altimetry/Bretschneider"]
set rand_generator      [new "WOSS/Definitions/RandomGenerator/NS2"]
#set rand_generator      [new "WOSS/Definitions/RandomGenerator/C"]
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

WOSS/Creator/Database/Textual/Results/TimeArr set debug           0
WOSS/Creator/Database/Textual/Results/TimeArr set woss_db_debug   0
WOSS/Creator/Database/Textual/Results/TimeArr set space_sampling 0.0

set db_res_arr [new "WOSS/Creator/Database/Textual/Results/TimeArr"]
$db_res_arr setDbPathName "${opt(db_res_path)}/test_hamburg_port_res_arr.txt"


#WOSS/Creator/Database/Binary/Results/TimeArr set debug           0
#WOSS/Creator/Database/Binary/Results/TimeArr set woss_db_debug   0
#WOSS/Creator/Database/Binary/Results/TimeArr set space_sampling 0.0

#set db_res_arr [new "WOSS/Creator/Database/Binary/Results/TimeArr"]
#$db_res_arr setDbPathName "${opt(db_res_path)}/test_aloha_with_dbs_res_arr.dat"


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
$db_ssp setDbPathName "${opt(db_path)}/ssp/WOA2018/WOA2018_SSP_April.nc"


WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set debug                0
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set woss_db_debug        0
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set db_spacing             1.0
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set total_northing_values  2013
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set total_easting_values   3473
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set range_easting_start    561288.113
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set range_easting_end      564760.113
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set range_northing_start   5931548.911
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set range_northing_end     5933650.911
WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV set approx_land_to_sea_surface 0

set db_bathy [new "WOSS/Creator/Database/Textual/Bathymetry/UMT_CSV"]
$db_bathy setDbPathName "${opt(db_path)}/bathymetry/hamburg_port.csv"


WOSS/Definitions/Altimetry/Flat set evolution_time_quantum   -1
WOSS/Definitions/Altimetry/Flat set range                    -1
WOSS/Definitions/Altimetry/Flat set total_range_steps        -1
WOSS/Definitions/Altimetry/Flat set depth                    0.0
set cust_altimetry   [new "WOSS/Definitions/Altimetry/Flat"]



$cust_altimetry setDebug 0


#first ssp, time key is 1st january 2011, 9:01 am
set time_evo_ssp_1 [new "WOSS/Definitions/Time"]
$time_evo_ssp_1 setTime 1 1 2011 9 0 1

WOSS/Database/Manager set debug 0
set db_manager [new "WOSS/Database/Manager"]
$db_manager setCustomSSP        $time_evo_ssp_1 "./ssp-hamb-port.txt"
$db_manager setCustomAltimetry  $cust_altimetry

WOSS/Creator/Bellhop set debug                        0.0
WOSS/Creator/Bellhop set woss_debug                   0.0
WOSS/Creator/Bellhop set woss_clean_workdir           1.0
WOSS/Creator/Bellhop set bellhop_arr_syntax           2
WOSS/Creator/Bellhop set evolution_time_quantum      -1.0
WOSS/Creator/Bellhop set total_runs                   1
WOSS/Creator/Bellhop set frequency_step               0.0
WOSS/Creator/Bellhop set total_range_steps            3000.0 ;#number of steps between rx and tx
WOSS/Creator/Bellhop set tx_min_depth_offset          0.0
WOSS/Creator/Bellhop set tx_max_depth_offset          0.0
WOSS/Creator/Bellhop set total_transmitters           1
WOSS/Creator/Bellhop set total_rx_depths              20
WOSS/Creator/Bellhop set rx_min_depth_offset          -20.1
WOSS/Creator/Bellhop set rx_max_depth_offset          20.1
WOSS/Creator/Bellhop set total_rx_ranges              100
WOSS/Creator/Bellhop set rx_min_range_offset          -800.1
WOSS/Creator/Bellhop set rx_max_range_offset          800.1
WOSS/Creator/Bellhop set total_rays                   0.0
WOSS/Creator/Bellhop set min_angle                    -20.0
WOSS/Creator/Bellhop set max_angle                    20.0
WOSS/Creator/Bellhop set ssp_depth_precision          1.0e-8
WOSS/Creator/Bellhop set normalized_ssp_depth_steps   100000
WOSS/Creator/Bellhop set box_depth                    100
WOSS/Creator/Bellhop set box_range                    15000


set woss_creator [new "WOSS/Creator/Bellhop"]
$woss_creator setWorkDirPath        "./bellhop_out_hamburg_port/"
$woss_creator setBellhopPath        ""
$woss_creator setBellhopMode        0 0 "A"
$woss_creator setBeamOptions        0 0 "B"
$woss_creator setBathymetryType     0 0 "C" 
$woss_creator setBathymetryMethod   0 0 "D"
$woss_creator setAltimetryType      0 0 "L"
$woss_creator setSimulationTimes    0 0 1 1 2010 0 0 1 2 1 2010 0 0 1


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
#$woss_controller setTimeArrResultsDbCreator  $db_res_arr
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


###########################################
# DESERT PARAMETERS DEFINITION
###########################################

#CBR MODULE
Module/UW/CBR set packetSize_          $opt(pktsize)
Module/UW/CBR set period_              $opt(cbr_period)
Module/UW/CBR set PoissonTraffic_      1
Module/UW/CBR set drop_out_of_order_   0
Module/UW/CBR set debug_      0

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

# Module/CBR set packetSize_          $opt(pktsize)
# Module/CBR set period_              $opt(cbr_period)
# Module/CBR set PoissonTraffic_      1


# #### MaxTxSPL_dB_ was previously named MaxTxPower_dB_
# #### MinTxSPL_dB_ was previously named MinTxPower_dB_
# #### TxSPLMargin_dB_ was previously named TxPowerMargin_dB_
# #### SPLOptimization_, CentralFreqOptimization_, BandwidthOptimization_ added
# WOSS/Module/MPhy/BPSK  set debug_                     0
# WOSS/Module/MPhy/BPSK  set AcquisitionThreshold_dB_   10.0 
# WOSS/Module/MPhy/BPSK  set BitRate_                   $opt(bitrate)
# WOSS/Module/MPhy/BPSK  set MaxTxSPL_dB_               160
# WOSS/Module/MPhy/BPSK  set MinTxSPL_dB_               10
# WOSS/Module/MPhy/BPSK  set MaxTxRange_                10000
# WOSS/Module/MPhy/BPSK  set PER_target_                0
# WOSS/Module/MPhy/BPSK  set TxSPLMargin_dB_            10
# WOSS/Module/MPhy/BPSK  set RxSnrPenalty_dB_           0
# WOSS/Module/MPhy/BPSK  set SPLOptimization_           0
# WOSS/Module/MPhy/BPSK  set CentralFreqOptimization_   0
# WOSS/Module/MPhy/BPSK  set BandwidthOptimization_     0

########################################
# Random Number Generators
########################################

global def_rng
set def_rng [new RNG]
$def_rng default

for {set k 0} {$k < $opt(rngstream)} {incr k} {
     $def_rng next-substream
}

###############################
# Procedure for creating nodes
###############################

proc createNode { id  }  {

     global channel propagation data_mask ns cbr position node port portnum ipr ipif channel_estimator
     global phy_data posdb opt rvposx rvposy rvposz mhrouting mll mac woss_utilities woss_creator db_manager

     set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

     #### we don't need the IP routing anymore layer for basic sample.
     #### ChannelEstimatorPlugin now expects as input MAC addresses.
     #### MLL is therefore needed to properly map IP to MAC addresses.
    set cbr($id)  [new Module/UW/CBR] 
    set udp($id) [new Module/UW/UDP]
    set ipr($id)  [new Module/UW/StaticRouting]
    set ipif($id) [new Module/UW/IP]
    set mll($id)  [new Module/UW/MLL] 
    set mac($id)  [new Module/UW/CSMA_ALOHA]
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


     $ipif($id) addr [expr $id + 1]
     #interface can reach directly all nodes if needed

     set position($id) [new "WOSS/Position"]
     $node($id) addPosition $position($id)
     set posdb($id) [new "PlugIn/PositionDB"]
     $node($id) addPlugin $posdb($id) 20 "PDB"
     $posdb($id) addpos [$ipif($id) addr] $position($id)


    set opt(start_lat) 53.541302
    set opt(start_long)   9.949799
    set curr_depth [ expr [ $db_manager getBathymetry $opt(start_lat) $opt(start_long) ]  ]
    set curr_depth [expr $curr_depth - $curr_depth/2.0]

     #### please Note: if you want to use the legacy underwatermiracle, you should use standard Position class and
     #### setX_ , setY_ , setZ_ ( negative values ) for space positioning!
     $position($id) setLatitude_  $opt(start_lat);#curr_lat
     $position($id) setLongitude_ $opt(start_long);#curr_lon
     $position($id) setAltitude_  [expr -1.0 * $curr_depth]

     set ch_estimator_plugin($id) [ new "WOSS/PlugIn/ChannelEstimator"]
     $ch_estimator_plugin($id) setChannelEstimator $channel_estimator
     $ch_estimator_plugin($id) insertNode [$mac($id) addr] $position($id)
     $node($id) addPlugin $ch_estimator_plugin($id) 19 "CHE"

     puts  "node $id at ([$position($id) getLatitude_], [$position($id) getLongitude_], [$position($id) getAltitude_])\n"


    set interf_data($id) [new "Module/UW/INTERFERENCE"]
    $interf_data($id) set maxinterval_ $opt(maxinterval_)
    $interf_data($id) set debug_       0


    $phy_data($id) setSpectralMask $data_mask
    $phy_data($id) setInterference $interf_data($id)
    $phy_data($id) setPropagation $propagation
	$phy_data($id) setInterferenceModel "MEANPOWER"
    $phy_data($id) set debug_ 0
    $mac($id) $opt(ack_mode)
    $mac($id) initialize

}


proc createSink { } {

     global channel propagation tone_mask data_mask ns cbr_sink position_sink node_sink port_sink portnum_sink 
     global phy_data_sink posdb_sink opt rvposx rvposy rvposz mhrouting mll_sink mac_sink ipr_sink ipif_sink 
     global woss_creator woss_utilities channel channel_estimator db_manager

    set node_sink [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    for { set cnt 0} {$cnt < $opt(nn)} {incr cnt} {
       set cbr_sink($cnt)  [new Module/UW/CBR] 
    }
    set udp_sink      [new Module/UW/UDP]
    set ipr_sink       [new Module/UW/StaticRouting]
    set ipif_sink      [new Module/UW/IP]
    set mll_sink       [new Module/UW/MLL] 
    set mac_sink       [new Module/UW/CSMA_ALOHA]
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

     $ipif_sink addr 253
     #interface can reach directly all nodes if needed

     set position_sink [new "WOSS/Position"]
     $node_sink addPosition $position_sink
     set posdb_sink [new "PlugIn/PositionDB"]
     $node_sink addPlugin $posdb_sink 20 "PDB"
     $posdb_sink addpos [$ipif_sink addr] $position_sink

     set coord_x [ expr (($opt(nn) - 1.0) * 1000.0) / 2.0 ]
     #set curr_lon [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $coord_x ]
 
    set curr_lat 53.543097
    set curr_lon 9.945293
    set curr_depth [ expr [ $db_manager getBathymetry $curr_lat $curr_lon]  ]
    set curr_depth [expr $curr_depth - $curr_depth/2.0]

     $position_sink setLatitude_  $curr_lat
     $position_sink setLongitude_ $curr_lon
     $position_sink setAltitude_  [expr -1.0 * $curr_depth]

     puts  "node_sink at ([$position_sink getLatitude_], [$position_sink getLongitude_], [$position_sink getAltitude_])"

     set ch_estimator_plugin_sink [ new "WOSS/PlugIn/ChannelEstimator"]
     $ch_estimator_plugin_sink setChannelEstimator $channel_estimator
     $ch_estimator_plugin_sink insertNode [$mac_sink addr] $position_sink
     $node_sink addPlugin $ch_estimator_plugin_sink 19 "CHE"


    set interf_data_sink [new Module/UW/INTERFERENCE]
    $interf_data_sink set maxinterval_ $opt(maxinterval_)
    $interf_data_sink set debug_       0

    $phy_data_sink setSpectralMask     $data_mask
    $phy_data_sink setPropagation      $propagation
    $phy_data_sink setInterference     $interf_data_sink
	$phy_data_sink setInterferenceModel "MEANPOWER"
        $mac_sink $opt(ack_mode)
    $mac_sink initialize

}

proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    $cbr_sink($id1) set destPort_ $portnum($id1)  
}
###############################
# routing of nodes
###############################

proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_auv portnum_auv ipr_sink
	global ipif_sink portnum_sink ipr_auv

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $ipr($id1) addRoute [$ipif_sink addr] [$ipif_sink addr]
    $ipr_sink  addRoute [$ipif($id1) addr] [$ipif($id1) addr]
}

###############################
# create nodes, sink, auv
###############################

for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	createNode $id1
}
createSink

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
	$mll($id1) addentry [$ipif_sink addr] [$mac_sink addr]
	$mll_sink addentry [$ipif($id1) addr] [$mac($id1) addr]
}
$mll_sink addentry [$ipif_sink addr] [$mac_sink addr]

###############################
# finish 
###############################

proc finish {} {
    global ns opt cbr mac propagation cbr_sink mac_sink phy_data phy_data_sink channel db_manager propagation
    global woss_manager

    puts "\n"

    for {set id3 0} {$id3 < $opt(nn)} {incr id3}  {

      set cbr_throughput   [$cbr_sink($id3) getthr]
      set cbr_per          [$cbr_sink($id3) getper]
      set cbr_pkts 			   [$cbr($id3) getsentpkts]
      set cbr_rxpkts       [$cbr_sink($id3) getrecvpkts]

      puts "cbr($id3)      app data pkts created       : $cbr_pkts"
      puts "cbr_sink($id3) app data pkts received      : $cbr_rxpkts"
      puts "cbr_sink($id3) throughput                  : $cbr_throughput"
      puts "cbr_sink($id3) packet error rate           : $cbr_per"
      puts ""
    }

    set opt(end_clock) [clock seconds]

    puts  "done in [expr $opt(end_clock) - $opt(start_clock)] seconds!"
    puts  "tracefile: $opt(tracefilename)"

    $ns flush-trace
    close $opt(tracefile)

    puts "Delete Woss objects to force write"
    delete $woss_manager
    delete $db_manager
}

###################
# start CBR(s)
###################

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    $ns at $opt(starttime)   "$cbr($id1) start"
    $ns at $opt(stoptime)    "$cbr($id1) stop"
}


###################
# start simulation
###################

puts "\nSimulating...\n"

$ns at [expr $opt(stoptime) + 10000.0]  " finish; $ns halt"
$ns run

