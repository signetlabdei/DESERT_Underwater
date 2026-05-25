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
# This script is used to test the YAML input interface.
# This file implements the reading functions for parsing
# the YAML configuration files.
#
# Authors: Vincenzo Cimino, Filippo Donegà
# Version: 1.0.0
#

package require yaml

# List of modules
set MOD_CBR "Module/UW/CBR" 
set MOD_TDMA "Module/UW/TDMA"
set MOD_PHY "Module/UW/PHYSICAL"


# get-app-pdr computes Module/UW/CBR packet delivery ratio.
#
# @paramm rx_app Application object receiving packets.
# @paramm tx_app application object transmitting packets
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
	global MOD_CBR MOD_TDMA MOD_PHY
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
			$MOD_PHY {
			switch -exact -- $item {
				"getTotPktsLost"   { set val [$rx getTotPktsLost] }
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
#
# @param rngstream     Random seed used as run identifier.
# @param input_modules List of modules used in the simulation.
# @param config_file   Name of the yaml output config file.
proc print-metrics-csv { rngstream input_modules config_file } {

    set output_config [load-output-config $config_file]

	# Iterate over each module in the yaml config file
    dict for {module_name requested_metrics} $output_config {
        if { [llength $requested_metrics] == 0 
				|| ![dict exists $input_modules $module_name] } continue

        # Sanitize filename
        set safe_name [string map {"/" "_"} $module_name]
        set filename "${safe_name}.csv"
        
		# Check if file already exists
        set file_exists 0
        if { [file exists $filename] } {
            set file_exists 1
        }

		# Open file in append mode
        set fp [open $filename a]

		# Retrieve lists of nodes and keys per current module
        set module_data [dict get $input_modules $module_name]
        set sorted_keys [lsort -dictionary [dict keys $module_data]]

		# Link-based modules use "rx_id,tx_id" as dict keys
        set is_linkbased [expr {[string length [lindex $sorted_keys 0]] > 1}]

		# Write csv header
        if { !$file_exists } {
            if {$is_linkbased} {
                puts $fp "rngstream,rx_id,tx_id,[join $requested_metrics ","]"
            } else {
                puts $fp "rngstream,node_id,[join $requested_metrics ","]"
            }
        } else {
			# If file already exists check header consistency
			set string_metrics [join $requested_metrics ,]

			set fh [open $filename r]
			gets $fh line
			close $fh

            if {$is_linkbased} {
				set string_header [join [lrange [split $line ","] 3 end] ","]
            } else {
				set string_header [join [lrange [split $line ","] 2 end] ","]
            }

			if {$string_header != $string_metrics} {
				puts "File already exists with different header, exiting..."
				close $fp
				break
			}
		}

		# Iterate over each link/node
		# Key: (module_name, "rx_id,tx_id") or (module_name, rx_id)
		# Value: (rx_object, tx_object)
        foreach id_key $sorted_keys {
            set objects [dict get $module_data $id_key]
            lassign $objects rx tx

			set row_metrics [get-metrics $requested_metrics $module_name $rx $tx]

            puts $fp "$rngstream,$id_key,[join $row_metrics ","]"
        }
        
        close $fp
        puts "Saved $module_name metrics in $filename"
    }
}
