#!/bin/bash
#
# Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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

if [ $# -lt 5 ]
then
	echo "1 - ID of the node"
	echo "2 - ID of the sink"
	echo "3 - ID of the first Head Node"
	echo "4 - ID of the second Head Node"
	echo "5 - IP of the EvoLogics modem"
	echo "6 - Port of the EvoLogics modem"
	echo "7 - ID of the experiment"

else

	if [ $1 == "--help" ] 
	then
		echo "1 - ID of the node"
		echo "2 - ID of the sink"
		echo "3 - IP of the modem"
		echo "4 - Port of the modem"
		echo "5 - ID of the experiment"
	else
		nc -w4 -z ${5} ${6} > /dev/null
		err_check=$?
		if [ ${err_check} -eq 1 ]
		then
			echo "The socket ${1}:${2} is not active! Check the IP and the port associated!"
		else
			rm -rf S2C_EvoLogics_ufetch.tr
			rm -rf MODEM_log_*
			ns S2C_ufetch_sample.tcl $5 $1 10 1 3600 $2 $3 $4 $5 $6
		fi
	fi
fi