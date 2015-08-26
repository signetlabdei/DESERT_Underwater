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

# @name_file:   install.sh
# @author:      Ivano Calabrese
# @last_update: 2014.01.14
# --
# @brief_description:

# #############################################################################
# TODO:
# (v) change "installationLOG" in "install.log"                                 [2014.01.15]
# (-) add mutual exclusiveness between "command-line" and "wizard procedure"    [2014.00.00]

# INCLUDE
. ./commonVariables.sh
. ./commonFunctions.sh

#------------------------------------------------------------------------------
print_desert_logo
#------------------------------------------------------------------------------

log_L1 "_________________________" ${INSTALL_LOG}
log_L1 "- install.sh is STARTED -" ${INSTALL_LOG}

echo -n "$0 " > ${INSTALL_CONF}

check_sh
#PRIVATE_VARIABLEs
SLEEP05=0.5


# Terminal parameters check
if [ $# -eq 0 ]; then
    install__print_help

    logERR "[Terminal_parameter_check]: exit_1" ${INSTALL_LOG}
    exit 1
fi

#------

#Getopt setting --
shortOpt="abcd:e:f:g:hi:lm"
longOpt="wizard,\
         with-woss,\
         without-woss,\
         target:,\
         inst_mode:,\
         dest_folder:,\
         custom_par:,\
         help,\
         addons:,\
         wizard-conf,\
         wizard-asOwner"
ARGS=$(getopt -o $shortOpt   \
              -l "$longOpt"  \
              -n "install.sh" \
              -- "$@");
RETOPT_GETOPT=$?
if [ "${_DEBUG}" = "1" ]; then
    debug__print_screen_L1 "RETURN CODE of GETOPT = ${RETOPT_GETOPT}"
fi

if [ "${_DEBUG}" = "1" ]; then
    debug__print_screen_L1 "AFTER GETOPT: \t\t$@"
fi

#Bad arguments after getopt --
if [ ${RETOPT_GETOPT} -ne 0 ]; then
    install__print_help
    logERR "some parameters are not correct" ${INSTALL_LOG}
    exit 1
fi

#Getopt setting --
eval set -- "$ARGS";
if [ "${_DEBUG}" = "1" ]; then
    debug__print_screen_L1 "AFTER SET -- \$ARGS: \t$@"
fi
while true; do
    case "$1" in
        -a|--wizard)
            shift;
            OWNER_PERMISSION=0
            ADDONS_FILE=".addon.list"
            _WIZARD_OPT=1
            log_L1 "_WIZARD_OPT=${_WIZARD_OPT}" ${INSTALL_LOG}
            if [ -n "$1" ]; then
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_WIZARD_OPT=${_WIZARD_OPT}"
                    debug__print_screen_L1 "OWNER_PERMISSION=${OWNER_PERMISSION}"
                    debug__print_screen_L1 "no parameter for --wizard option"
                fi
                #shift
            fi
            wizard_function
            break;
            ;;
        -m|--wizard-asOwner)
            shift;
            if [ -f .addon.priv.list ]; then
                OWNER_PERMISSION=1
                ADDONS_FILE=".addon.priv.list"
            else
                OWNER_PERMISSION=0
                ADDONS_FILE=".addon.list"
                logWARN_L1 "the .addon.priv.list file not found. instal.sh script will start with --wizard option" ${INSTALL_LOG}
                warn_L1 ".addon.priv.list file not found!"
                warn_L2 "instal.sh script will start with --wizard option"
            fi
            _WIZARD_OPT=1
            log_L1 "_WIZARD_OPT=${_WIZARD_OPT}" ${INSTALL_LOG}
            if [ -n "$1" ]; then
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_WIZARD_OPT=${_WIZARD_OPT}"
                    debug__print_screen_L1 "OWNER_PERMISSION=${OWNER_PERMISSION}"
                    debug__print_screen_L1 "no parameter for --wizard option"
                fi
                #shift
            fi
            wizard_function
            break;
            ;;
        -b|--with-woss)
            shift;
            _WITHWOSS=1
            WITHWOSS=1
            log_L1 "_WITHWOSS=${_WITHWOSS}" ${INSTALL_LOG}
            echo -n "--with-woss " >> ${INSTALL_CONF}
            if [ -n "$1" ]; then
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_WITHWOSS=${_WITHWOSS}"
                    debug__print_screen_L1 "no parameter for --with-woss option"
                fi
                #shift
            fi
            ;;
        -c|--without-woss)
            shift;
            _WITHOUTWOSS=1
            WITHWOSS=0
            log_L1 "_WITHOUTWOSS=${_WITHOUTWOSS}" ${INSTALL_LOG}
            if [ -n "$1" ]; then
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_WITHOUTWOSS=${_WITHOUTWOSS}"
                    debug__print_screen_L1 "no parameter for --without-woss option"
                fi
                #shift
            fi
            ;;
        -d|--target)
            shift;
            _TARGET=1
            log_L1 "_TARGET=${_TARGET}" ${INSTALL_LOG}
            if [ -n "${1}" ]; then
                TARGET="${1}"
                echo -n "--target ${1} " >> ${INSTALL_CONF}
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_TARGET=${_TARGET}"
                    debug__print_screen_L1 "parameter for --target option is: ${1}"
                fi
                shift
            fi
            ;;
        -e|--inst_mode)
            shift;
            _INST_MODE=1
            log_L1 "_INST_MODE=${_INST_MODE}" ${INSTALL_LOG}
            if [ -n "${1}" ]; then
                INST_MODE=${1}
                echo -n "--inst_mode ${1} " >> ${INSTALL_CONF}
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_INST_MODE=${_INST_MODE}"
                    debug__print_screen_L1 "parameter for --inst_mode option is: ${1}"
                fi
                shift
            fi
            ;;
        -f|--dest_folder)
            shift;
            _DEST_FOLDER=1
            log_L1 "_DEST_FOLDER=${_DEST_FOLDER}" ${INSTALL_LOG}
            if [ -n "${1}" ]; then
                DEST_FOLDER="${1}"
                echo -n "--dest_folder ${1} " >> ${INSTALL_CONF}
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_DEST_FOLDER=${_DEST_FOLDER}"
                    debug__print_screen_L1 "parameter for --dest_folder option is: ${1}"
                fi
                shift
            fi
            ;;
        -g|--custom_par)
            shift;
            _CUSTOM_PAR=1
            log_L1 "_CUSTOM_PAR=${_CUSTOM_PAR}" ${INSTALL_LOG}
            if [ -n "${1}" ]; then
                CUSTOM_PAR=${1}
                echo -n "--custom_par \"${1}\" " >> ${INSTALL_CONF}
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_CUSTOM_PAR=${_CUSTOM_PAR}"
                    debug__print_screen_L1 "parameter for --custom_par option is: ${1}"
                fi
                shift
            fi
            ;;
        -h|--help)
            shift;
            log_L1 "help option is called" ${INSTALL_LOG}
            if [ -n "$1" ]; then
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "help option is called"
                    debug__print_screen_L1 "no parameter for --help option"
                fi
                #shift
            fi
            install__print_help
            exit 0
            ;;
        -i|--addons)
            shift;
            _ADDONS=1
            log_L1 "_ADDONS=${_ADDONS}" ${INSTALL_LOG}
            if [ -n "${1}" ]; then
                ADDONS=${1}
                echo -n "--addons \"${1}\" " >> ${INSTALL_CONF}
                if [ "${_DEBUG}" = "1" ]; then
                    debug__print_screen_L1 "_ADDONS=${_ADDONS}"
                    debug__print_screen_L1 "parameter for --addons option is: ${1}"
                fi
                shift
            fi
            ;;
        -l|--wizard-conf)
            shift;
            wizard_conf
            exit 0
            ;;
        --)
            #shift;
            break;
            ;;
    esac
done

#***
# internal settings
#
BUILD_HOST="${DEST_FOLDER}/.buildHost"
BUILD_TARGET="${DEST_FOLDER}/.buildTarget"
#*

if [ "${_DEBUG}" = "1" ]; then
    debug__print_parameters
fi
if [ ${_TARGET} -ne 1 ] || [ ${_INST_MODE} -ne 1 ] || [ ${_DEST_FOLDER} -ne 1 ]; then
    err_L1 "NO OPTIONs SET! Try './install.sh --help' or check the log file: install.log"
    log_L2 ">>> TARGET       (TARGET)                       = ${TARGET}"       ${INSTALL_LOG}
    log_L2 ">>> INST_MODE    (Installation Mode)            = ${INST_MODE}"    ${INSTALL_LOG}
    log_L2 ">>> DEST_FOLDER  (Destination Folder)           = ${DEST_FOLDER}"  ${INSTALL_LOG}
    log_L2 ">>> BUILD_HOST   (host Destination Folder)      = ${BUILD_HOST}"   ${INSTALL_LOG}
    log_L2 ">>> BUILD_TARGET (target Destination Folder)    = ${BUILD_TARGET}" ${INSTALL_LOG}
    log_L2 ">>> CUSTOM_PAR   (Custom Parameters)            = ${CUSTOM_PAR}"   ${INSTALL_LOG}
    log_L2 ">>> ADDONS       (names of the Addons)          = ${ADDONS}"       ${INSTALL_LOG}
    exit 0
fi

info_L0 "make_dir"
make_dir
if [ $? -ne 0 ]; then
    err_L1 "EXIT FROM install.sh"
    exit
fi

info_L0 "unpack_desertframework"
unpack_desertframework
if [ $? -ne 0 ]; then
    err_L1 "EXIT FROM install.sh"
    exit
fi

info_L0 "call_installer"
call_installer
if [ $? -ne 0 ]; then
    err_L1 "EXIT FROM install.sh"
    exit
fi

delete_recursive_soft_link

info_L0 "after_building"
after_building
if [ $? -ne 0 ]; then
    err_L1 "EXIT FROM install.sh"
    exit
fi

exit 0

