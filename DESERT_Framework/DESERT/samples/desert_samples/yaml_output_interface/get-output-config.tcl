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
# Authors: Vincenz Cimino, Filippo Donegà
# Version: 1.0.0
#

package require yaml

set MOD_CBR "Module/UW/CBR" 
set MOD_TDMA "Module/UW/TDMA"
set MOD_PHY "Module/UW/PHYSICAL"


# Compute application layer packet delivery ratio.
# Inputs
# - rx_app: application object receiving packets
# - tx_app: application object transmitting packets
proc get-app-pdr { rx_app tx_app} {
	set sent_packets [$tx_app getsentpkts]
	set received_packets [$rx_app getrecvpkts]

	return [expr 1.0 * $received_packets / $sent_packets]
}

# load-output-config reads output metrics from yaml config file and store them in a dictionary.
# Keys are module names and values list of metrics.
# Inputs
# - config_file: name of the yaml config file
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

# print-metrics-csv retrieves metrics from a list of modules and store them
# in csv files. A different file is created for each module.
# Modules are distinguished in:
# - Node-based: metrics are computed over all links
# - Link-based: metrics are computed over single links
# Inputs
# - input_modules: list of modules used in the simulation
# - config_file: name of the yaml config file
proc print-metrics-csv { input_modules config_file } {
	global MOD_CBR MOD_TDMA MOD_PHY

    set output_config [load-output-config $config_file]

    dict for {module_name requested_metrics} $output_config {
        if {[llength $requested_metrics] == 0 || ![dict exists $input_modules $module_name]} continue

        # Sanitize filename
        set safe_name [string map {"/" "_"} $module_name]
        set filename "${safe_name}.csv"
        
        set fp [open $filename w]
        
		# Write csv header
        set is_linkbased [expr {$module_name eq $MOD_CBR}]
        if {$is_linkbased} {
            puts $fp "rx_id,tx_id,[join $requested_metrics ","]"
        } else {
            puts $fp "node_id,[join $requested_metrics ","]"
        }

        set module_data [dict get $input_modules $module_name]
        set sorted_keys [lsort -dictionary [dict keys $module_data]]

		# Iterate over each module
		# Key: (module_name, rx_id, tx_id)
		# Value: (rx_object, tx_object)
        foreach id_key $sorted_keys {
            set objects [dict get $module_data $id_key]
            lassign $objects rx tx
            set row_metrics [list]

			# Iterate over each metric
            foreach item $requested_metrics {
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

            # Write the row
            puts $fp "$id_key,[join $row_metrics ","]"
        }
        
        close $fp
        puts "Saved metrics in $filename"
    }
}
