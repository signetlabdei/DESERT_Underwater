#
# Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
# This script contains the API to use the YAML output interface.
# This file implements the reading functions for parsing
# the YAML configuration files and writing the CSV outputs.
#
# Authors: Vincenzo Cimino, Filippo Donegà
# Version: 1.0.0
#

package require yaml

# List of modules
set MOD_CBR         "Module/UW/CBR" 
set MOD_TDMA        "Module/UW/TDMA"
set MOD_CSMA_ALOHA  "Module/UW/CSMA_ALOHA"
set MOD_PHY         "Module/UW/PHYSICAL"

##
# get-app-pdr computes Module/UW/CBR packet delivery ratio.
#
# @paramm rx_app Application object receiving packets.
# @paramm tx_app application object transmitting packets
##
proc get-app-pdr { rx_app tx_app} {
	set sent_packets [$tx_app getsentpkts]
	set received_packets [$rx_app getrecvpkts]

	set pdr [format %.6f [expr 1.0 * $received_packets / $sent_packets]]

	return $pdr
}

##
# load-output-config reads output metrics from yaml config file and store them in a dictionary.
# Keys are module names and values list of metrics.
#
# @param config_file Name of the yaml config file.
#
# @return Dictionary containing the modules metrics.
#
proc load-output-config { config_file } {
    global opt

    set fileId [open $config_file r]
    set data [read $fileId]
    close $fileId
    set loaded_data [::yaml::yaml2dict $data]

    set module_names [dict keys $loaded_data]
	set module_metrics [dict create]

	# Iterate over each module
    foreach module $module_names {
		set metrics [list]

		# Iterate over metric
        foreach param [dict keys [dict get $loaded_data $module]] {
            set value [dict get $loaded_data $module $param]

			# Store only active metrics
			switch $value {
				1 { lappend metrics $param }
				0 { }
				default {
					puts "Invalid value for $param, ignoring."
				}
			}
        }

		dict append module_metrics $module $metrics
    }

	return $module_metrics
}

##
# write-input-node-based-modules creates a dictionary in the caller's scope
# with receiving (rx) modules, grouped by the module's class,for the specified node arrays.
# It iterates through node IDs up to the global limit opt(nn).
#
# @param dict_modules Dictionary in the caller's scope to be populated.
# @param args         One or more arrays containing module references indexed by node IDs (e.g., $i).
##
proc write-input-node-based-modules {dict_modules args} {
    global opt
    upvar 1 $dict_modules input_modules
    
    foreach type $args {
        upvar 1 $type curr_mod

        for {set i 0} {$i < $opt(nn)} {incr i} {
            if {![info exists curr_mod($i)]} continue
            
            set rx $curr_mod($i)
            set module_name [$rx info class] 
            
            # Use node id as key
            dict set input_modules $module_name $i [list $rx ""]
        }
    }
}

##
# write-input-link-based-modules creates a dictionary in the caller's scope
# with receiving (rx) and transmitting (tx) module pairs, grouped by 
# the receiving module's class, for the specified link arrays.
#
# @param dict_var_name Dictionary in the caller's scope.
# @param args          One or more array containing module object references indexed by node pairs.
##
proc write-input-link-based-modules {dict_var_name args} {
    upvar 1 $dict_var_name input_modules
    
    foreach type $args {
        upvar 1 $type curr_array

        # If the array doesn't exist in the caller's scope, skip it
        if {![array exists curr_array]} continue

        foreach link [lsort -dictionary [array names curr_array]] {
            scan $link "%d,%d" i j
            
            if {$i == $j || ![info exists curr_array($i,$j)] || ![info exists curr_array($j,$i)]} continue 
            
            set rx $curr_array($i,$j)
            set tx $curr_array($j,$i)
            set module_name [$rx info class] 
            
            # Use "i,j" as a key
            dict set input_modules $module_name "$i,$j" [list $rx $tx]
        }
    }
}

##
# write-sink-modules creates a dictionary in the caller's scope
# with receiving (rx) and transmitting (tx) module pairs.
# It automatically handles both array-based receivers (e.g., cbr_sink) 
# and scalar-based receivers (e.g., mac_sink).
#
# @param dict_var_name Dictionary in the caller's scope.
# @param args          One or more array couples containing module object references indexed by node pairs.
##
proc write-sink-modules {dict_var_name args} {
    upvar 1 $dict_var_name input_modules
    
    if { [expr [llength $args] % 2] != 0 } {
        puts "Invalid number of arguments. Please provide pairs (e.g., rx_var tx_array)."
        return
    }

    foreach {rx_name tx_array_name} $args {
        upvar 1 $rx_name rx_var
        upvar 1 $tx_array_name tx_array

        # The transmitter must be an array of nodes
        if {![array exists tx_array]} {
            puts "Warning: Transmitter $tx_array_name is not an array. Skipping."
            continue
        }

        # Check if the receiver is an array or a single scalar variable
        set rx_is_array [array exists rx_var]
        set rx_is_scalar [info exists rx_var]

        if {!$rx_is_array && !$rx_is_scalar} {
            puts "Warning: Receiver $rx_name does not exist. Skipping."
            continue
        }

        # Iterate through the transmitting nodes
        foreach tx_id [lsort -dictionary [array names tx_array]] {
            set tx $tx_array($tx_id)
            
            # Assign the correct receiver object
            if {$rx_is_array} {
                if {![info exists rx_var($tx_id)]} continue 
                set rx $rx_var($tx_id)
            } else {
                # If it's a scalar, all tx nodes report to the same single rx module
                set rx $rx_var
            }
            
            set module_name [$rx info class] 
            
            # Use "sink,tx_id" as a key to differentiate from node/link keys safely
            dict set input_modules $module_name "sink,$tx_id" [list $rx $tx]
        }
    }
}

##
# get-metrics retrieves specified metric values for a given module.
#
# @param metrics_names List of the names of the metrics to be collected.
# @param module_name   Name of the module containing the metrics.
# @param rx            The receiving module object reference.
# @param tx            The transmitting module object reference (used in link-based modules only).
#
# @return List containing the requested metric values.
##
proc get-metrics { metrics_names module_name rx tx} {
	global MOD_CBR MOD_TDMA MOD_CSMA_ALOHA MOD_PHY
    set row_metrics [list]

	# Iterate over each metric
	foreach item $metrics_names {
		set val "nan"
		switch -exact -- $module_name \
			$MOD_CBR  {
				switch -exact -- $item {
					"PDR"              { set val [get-app-pdr $rx $tx] }
					"throughput"       { set val [$rx getthr] }
					"sent_packets"     { set val [$rx getsentpkts] }
					"received_packets" { set val [$rx getrecvpkts] }
				}
			} \
			$MOD_TDMA {
				switch -exact -- $item {
					"sent_packets"     { set val [$rx get_sent_pkts] }
					"received_packets" { set val [$rx get_recv_pkts] }
				}
			} \
			$MOD_CSMA_ALOHA {
				switch -exact -- $item {
					"sent_packets"     { set val [$rx getDataPktsTx] }
					"received_packets" { set val [$rx getDataPktsRx] }
				}
			} \
			$MOD_PHY {
				switch -exact -- $item {
					"packets_lost"   { set val [$rx getTotPktsLost] }
				}
			}

		lappend row_metrics $val
	}

	return $row_metrics
}

##
# print-metrics-csv retrieves metrics from a list of modules and store them
# in csv files. A different file is created for each module.
# Modules are distinguished in:
# - Node-based: metrics are computed over all links.
# - Link-based: metrics are computed over single links.
# - Sink-based: metrics are compute over links to sink.
#
# @param rngstream     Random seed used as run identifier.
# @param input_modules List of modules used in the simulation.
# @param config_file   Name of the yaml output config file.
proc print-metrics-csv { rngstream input_modules config_file } {
    set output_config [load-output-config $config_file]

    dict for {module_name requested_metrics} $output_config {
        if { [llength $requested_metrics] == 0 || ![dict exists $input_modules $module_name] } continue

        set safe_name [string map {"/" "_"} $module_name]
        set module_data [dict get $input_modules $module_name]
        
        # Separate keys by configuration type
        set node_keys [list]
        set link_keys [list]
        set sink_keys [list]
        
        foreach k [dict keys $module_data] {
            if {[string match "sink,*" $k]} {
                lappend sink_keys $k
            } elseif {[string match "*,*" $k]} {
                lappend link_keys $k
            } else {
                lappend node_keys $k
            }
        }

        if {[llength $node_keys] > 0} {
            write-group-csv $rngstream "${safe_name}.csv" "node_id" $requested_metrics $module_data $node_keys $module_name
        }
        if {[llength $link_keys] > 0} {
            write-group-csv $rngstream "${safe_name}.csv" "rx_id,tx_id" $requested_metrics $module_data $link_keys $module_name
        }
        if {[llength $sink_keys] > 0} {
            write-group-csv $rngstream "${safe_name}_sink.csv" "tx_id" $requested_metrics $module_data $sink_keys $module_name
        }
    }
}

##
# Helper procedure to manage the file writing and header logic for grouped layout types.
##
proc write-group-csv { rngstream filename header_prefix requested_metrics module_data keys_list module_name } {
    set file_exists [file exists $filename]
    set fp [open $filename a]
    
    if { !$file_exists } {
        puts $fp "rngstream,${header_prefix},[join $requested_metrics ","]"
    } else {
        set string_metrics [join $requested_metrics ,]
        set fh [open $filename r]
        gets $fh line
        close $fh
        
        # Calculate dynamic offset to skip prefix columns (e.g., skip 'rngstream,tx_id')
        set prefix_cols [llength [split $header_prefix ","]]
        set string_header [join [lrange [split $line ","] [expr {$prefix_cols + 1}] end] ","]
        
        if {$string_header != $string_metrics} {
            puts "File $filename already exists with different header, exiting..."
            close $fp
            return
        }
    }
    
    foreach id_key [lsort -dictionary $keys_list] {
        set objects [dict get $module_data $id_key]
        lassign $objects rx tx
        set row_metrics [get-metrics $requested_metrics $module_name $rx $tx]
        
        # Strip the "sink," tag for the final CSV print
        set print_id $id_key
        if {[string match "sink,*" $id_key]} {
            set print_id [string range $id_key 5 end]
        }
        
        puts $fp "$rngstream,$print_id,[join $row_metrics ","]"
    }

    close $fp
    puts "Saved $module_name metrics in $filename"
}
