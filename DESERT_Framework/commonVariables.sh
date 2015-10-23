#!/bin/sh

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

# @name_file:   commonVariable.sh
# @author:      Ivano Calabrese
# @last_update: 2014.01.14
# --
# @brief_description:


# Global_variables for install.sh
#COMMON VARIABLEs
# Versions

#***
# _DEBUG: 0(no messages printed) 
#         1(messages printed)
export _DEBUG=0
#*

# System_set
export ARCH="arm-linux"
export HOST=$(uname -m)-linux-gnu
export CPU_NUM=$(grep -c '^processor' /proc/cpuinfo)

# Paths
export ROOT_DESERT=$(pwd)
export INSTALL_LOG="${ROOT_DESERT}/install.log"
export INSTALL_CONF="${ROOT_DESERT}/install.conf"
#export LOG_DIR=${ROOT_DESERT}/LOG
export LOG_DIR_BUILD=""

export PATCHES_TAR_FILE=${ROOT_DESERT}/patches.tar.gz
export PATCHES_DIR=patches

export ZLIB_VERSION=1.2.7
export ZLIB_TAR_FILE=${ROOT_DESERT}/zlib-${ZLIB_VERSION}.tar.gz
export ZLIB_DIR=zlib-${ZLIB_VERSION}

export TCL_VERSION=8.4.19
export TCL_TAR_FILE=${ROOT_DESERT}/tcl-${TCL_VERSION}.tar.gz
export TCL_DIR=tcl-${TCL_VERSION}

export OTCL_VERSION=1.14
export OTCL_TAR_FILE=${ROOT_DESERT}/otcl-${OTCL_VERSION}.tar.gz
export OTCL_DIR=otcl-${OTCL_VERSION}

export TCLCL_VERSION=1.20
export TCLCL_TAR_FILE=${ROOT_DESERT}/tclcl-${TCLCL_VERSION}.tar.gz
export TCLCL_DIR=tclcl-${TCLCL_VERSION}

export NS_VERSION=2.34
export NS_TAR_FILE=${ROOT_DESERT}/ns-${NS_VERSION}.tar.gz
export NS_DIR=ns-${NS_VERSION}

export NSMIRACLE_VERSION=1.1.1
export NSMIRACLE_TAR_FILE=${ROOT_DESERT}/nsmiracle-${NSMIRACLE_VERSION}.tar.gz
export NSMIRACLE_DIR=nsmiracle-${NSMIRACLE_VERSION}

export BELLHOP_TAR_FILE=${ROOT_DESERT}/at.tar.gz
export BELLHOP_DIR=at

export NETCDF_VERSION=4.2.1.1
export NETCDF_TAR_FILE=${ROOT_DESERT}/netcdf-${NETCDF_VERSION}.tar.gz
export NETCDF_DIR=netcdf-${NETCDF_VERSION}

export NETCDFCXX_VERSION=4.2
export NETCDFCXX_TAR_FILE=${ROOT_DESERT}/netcdf-cxx-${NETCDFCXX_VERSION}.tar.gz
export NETCDFCXX_DIR=netcdf-cxx-${NETCDFCXX_VERSION}

export WOSS_VERSION=1.3.6
export WOSS_TAR_FILE=${ROOT_DESERT}/woss-${WOSS_VERSION}.tar.gz
export WOSS_DIR=woss-${WOSS_VERSION}

export DESERT_VERSION=2.1.0
export DESERT_DIR=DESERT

export UNPACKED_FOLDER="${ROOT_DESERT}/.unpacked_folder"
export ZXX="${ROOT_DESERT}/.zxx"


#***
# _WIZARD_OPT: -1(no variable setted)
#               0(wizard mode is tour-off) 
#               1(wizard mode is tour-on)
export _WIZARD_OPT=-1
#*

#***
# _WITHWOSS: -1(no variable setted)
#             1(the with-woss setting is done)
export _WITHWOSS=-1
#*

#***
# _WITHOUTWOSS: -1(no variable setted)
#                1(the without-woss setting is done)
export _WITHOUTWOSS=-1
#*

#***
# _TARGET: -1(no variable setted)
#           1(the target setting is done)
export _TARGET=-1
#*

#***
# _INST_MODE: -1(no variable setted)
#              1(the target setting is done)
export _INST_MODE=-1
#*

#***
# _DEST_FOLDER: -1(no variable setted)
#                1(the target setting is done)
export _DEST_FOLDER=-1
#*

#***
# _CUSTOM_PAR: -1(no variable setted)
#               1(the target setting is done)
export _CUSTOM_PAR=-1
#*

#***
# _ADDONS: -1(no variable setted)
#           1(the target setting is done)
export _ADDONS=-1
#*

#***
# WITHWOSS:   0(--without-woss options is passed to installer procedure) 
#             1(--with-woss options in passed to installer procedure)
export WITHWOSS=0
#*

#***
# OWNER_PERMISSION: 0(default setting. In this case the installer will be set for a public user)
#                   1(to set this value you must pass the --wizard-asOwner options to install.sh script)
#
export OWNER_PERMISSION=0
#*

export TARGET=""
export INST_MODE=""
export DEST_FOLDER=""

#***
# attention:
# BUILD_HOST and BUILD_TARGET will be setting in the install.sh.
# DON'T SETTING THESE TWO VARIABLES!!!
export BUILD_HOST=""
export BUILD_TARGET=""
#*

export CUSTOM_PAR=""
export ADDONS=""
export ADDONS_FILE=".addon.list"
# #
# ###
export TMP_INPUT=""
export PARAMETER4INSTALLER=""
# #

