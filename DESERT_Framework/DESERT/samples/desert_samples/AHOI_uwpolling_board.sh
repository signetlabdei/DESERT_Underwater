#!/bin/bash
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
#
# Author: Emanuele Coccolo
# Version: 1.0.0
ID_NODE=13
ASV=13
START=1
STOP=1800
PERIOD=0.1
IFACE="ttyUSB0"
APP_PORT=55551
REP_NUM=1

VALID=1


while test $# -gt 0
do
	case "$1" in
		--id) echo "Node ID setup"
			shift
			TEMP_ID=$1
			;;
		--start) echo "Start setup"
			shift
			START=$1
			;;
		--stop) echo "Stop setup"
			shift
			STOP=$1
			;;
		--offline) echo "Offline mode"
			shift
			export PATH=/home/linaro/DESERT_Underwater_private/DESERT_buildCopy_LOCAL/bin:$PATH
			export LD_LIBRARY_PATH=/home/linaro/DESERT_Underwater_private/DESERT_buildCopy_LOCAL/lib:$LD_LIBRARY_PATH
			source /home/linaro/DESERT_Underwater_private/DESERT_buildCopy_LOCAL/environment
			STOP=$1
			;;
		--period) echo "Period setup"
			shift
			PERIOD=$1
			;;
		--iface) echo "Serial interface setup"
			shift
			IFACE=$1
			;;
		--port) echo "Application port setup"
			shift
			APP_PORT=$1
			;;
		--rep_num) echo "Repetition number"
			shift
			REP_NUM=$1
			;;
		--help)
			echo "Use the following parameters for manual setup"
			echo "<to be completed>"
			echo "otherwise the default will be applied:"
			echo "START = 1 second"
			echo "STOP = 30 minutes (1800 seconds)"
			echo "PERIOD = 0.1 second"
			echo "IFACE = ttyUSB0"
			echo "APP_PORT = 55551"
			echo "REP_NUM = 1"
			exit 0
			;;
		--*)
			echo "Parameter not recognized."
			VALID=0
			exit 0
			;;
	esac
	shift
done

if [ "$VALID" -eq 1 ]; then

	if [ -z "$TEMP_ID" ]; then
		echo "Command: ns AHOI_uwpolling_sink.tcl $ID_NODE $ASV $START $STOP $PERIOD $IFACE $APP_PORT $REP_NUM"
		ns AHOI_uwpolling_sink.tcl $ID_NODE $ASV $START $STOP $PERIOD $IFACE $APP_PORT $REP_NUM
	else
		echo "TEMP ID mode"
		echo "Command: ns AHOI_uwpolling_sink.tcl $TEMP_ID $ASV $START $STOP $PERIOD $IFACE $APP_PORT $REP_NUM"
		ns AHOI_uwpolling_sink.tcl $TEMP_ID $ASV $START $STOP $PERIOD $IFACE $APP_PORT $REP_NUM
	fi
else
	echo "Invalid parameters."
fi