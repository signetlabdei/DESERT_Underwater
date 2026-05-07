#
# Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
# Authors: Roberto francescon <frances1@dei.unipd.it>
# Version: 1.0.0
#

package require yaml

proc load-output-config { filename } {

    global opt

    set fileId [open $filename r]
    set data [read $fileId]
    close $fileId
    set loaded_data [::yaml::yaml2dict $data]

    set modules [dict keys $loaded_data]
	set module_metrics [dict create]

    foreach module $modules {
		set temp_list [list]
        foreach param [dict keys [dict get $loaded_data $module]] {
            set value [dict get $loaded_data $module $param]

			switch $value {
				1 {
					lappend temp_list $param
				}
				0 { }
				default {
					puts "Invalid value for $param, exiting..."
					exit
				}
			}
        }
		dict append module_metrics $module $temp_list
    }

	puts $module_metrics
}

load-output-config "./uwtdma_output_config.yaml"
