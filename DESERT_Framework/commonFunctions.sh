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

# @name_file:   commonFunctions.sh
# @author:      Ivano Calabrese
# @last_update: 2014.01.14
# --
# @brief_description:


#COMMON FUNCTIONs
print_desert_logo() {
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "fuction_name is: print_desert_logo()"
    fi
    printf "__________________________________________________________________________________________\n"
    printf " _____                      _     _    _           _                         _            \n"
    printf "|  __ \                    | |   | |  | |         | |                       | |           \n"
    printf "| |  | | ___  ___  ___ _ __| |_  | |  | |_ __   __| | ___ _ ____      ____ _| |_ ___ _ __ \n"
    printf "| |  | |/ _ \\/ __|/ _ \ '__| __| | |  | | '_ \\ / _\` |/ _ \\ '__\\ \\ /\\ / / _\` | __/ _ \\ '__|\n"
    printf "| |__| |  __/\\__ \\  __/ |  | |_  | |__| | | | | (_| |  __/ |   \\ V  V / (_| | ||  __/ |   \n"
    printf "|_____/ \\___||___/\\___|_|   \\__|  \\____/|_| |_|\\__,_|\\___|_|    \\_/\\_/ \\__,_|\\__\\___|_|   \n"
    printf "_____Copyright (c) 2024 Regents of the SIGNET lab, University of Padova)_____\n"
    printf "\n"
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "return of print_desert_logo(): ${output}"
        debug__print_screen_L1 "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯"
    fi
    return 0
}

read_input() {
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "fuction_name is: read_input()"
    fi
    #read -p "choose your ${1}: " output
    read -p ": " output
    #read -s output
    eval ${1}='${output}'
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "${1} = ${output}"
        debug__print_screen_L1 "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯"
    fi
    #printf '%s\n' "${output}"
    return 0
}
# #
# ###
info_L0() {
    printf "\033[0m\033[1;34;40m$*\033[0m\n"
    return 0
}

info_L1() {
    printf "[$(date +"%Y-%m-%d %H:%M:%S")] \033[0m\033[1;34m* $*\033[0m\n"
    return 0
}

info_L2() {
    printf "[$(date +"%Y-%m-%d %H:%M:%S")] \033[0m\033[0;35m** $*\033[0m\n"
    return 0
}

ok_L1() {
    printf "[$(date +"%Y-%m-%d %H:%M:%S")] \033[0m\033[0;32m** $*\033[0m\n"
    return 0
}

err_L1() {
    printf "\033[0m\033[1;31mERROR: $*\033[0m\n"
    return 0
}

warn_L1() {
    printf "\033[0m\033[1;33;49mWARNING: $*\033[0m\n"
    return 0
}

warn_L2() {
    printf "\033[0m\033[0;33;49mWARNING: $*\033[0m\n"
    return 0
}
# #
# ###
log_L1() {
    printf '%s\n' "[$(date +"%Y-%m-%d %H:%M:%S")] $1\n" >> ${2}
    return 0
}

log_L2() {
    printf '%s\n' "[$(date +"%Y-%m-%d %H:%M:%S")]     $1\n" >> ${2}
    return 0
}

logERR() {
    printf '%s\n' "                     ----------------------" >> ${2}
    printf '%s\n' "[$(date +"%Y-%m-%d %H:%M:%S")]>>> ERROR: $1\n" >> ${2}
    printf '%s\n' "                     ----------------------" >> ${2}
    return 0
}

logWARN_L1() {
    printf '%s\n' "                     ------------------------" >> ${2}
    printf '%s\n' "[$(date +"%Y-%m-%d %H:%M:%S")]>>> WARNING: $1\n" >> ${2}
    printf '%s\n' "                     ------------------------" >> ${2}
    return 0
}
# #

wizard__print_L1() {
    printf "\033[0m\033[0;34;50m$*\033[0m\n"
}

wizard__print_L2() {
    printf "\033[0m\033[0;35;49m- $*\033[0m\n"
}

wizard__print_L3() {
    printf "\033[0m\033[0;33;49m* $*\033[0m\n"
}

wizard__print_L4() {
    printf "\033[0m\033[0;33;49m$*\033[0m\n"
}

wizard_conf_list() {
    if [ -f ${ROOT_DESERT}/${1} ]; then
        wizard__print_L4 "$(cat ${ROOT_DESERT}/${1} | sed -e 's/\(.\)/*  \1/')"
    fi
}

wizard_conf_target() {
    printf '\n' 
    wizard_conf_list ".wizard.target.list"
    printf '%s' "You can add (a) new targets, remove (r) someone of them or skip (s). "
    tOpt=""
    read_input "tOpt"
    if [ ${tOpt} = "s" ]; then
        return 0
    fi
    printf '%s\n' "Enter all TARGETs to be updated (separate entries with a space)"
    tList=""
    read_input "tList"
    wizard_conf_target_update ${tOpt} "${tList}"
    printf '%s' "Is the wizard configuration concluded? (y/n)"
    tOpt2=""
    read_input "tOpt2"
    case "${tOpt2}" in
        [Yy]|[Yy]es)
            return 0
            ;;
        [Nn]|[Nn]o)
            wizard_conf_target
            ;;
        *)
            wizard_conf_target
            ;;
    esac
}

wizard_conf_target_update() {
    case ${1} in
        "a")
            for target in ${2}; do
                printf '%s\n' "${target}" >> ${ROOT_DESERT}/.wizard.target.list
            done
            ;;
        "r")
            for target in ${2}; do
                eval sed -i -e '/^${target}/d' .wizard.target.list
            done
            ;;
        *)
            ;;
    esac
}

wizard_conf_addon() {
    printf "\n"
    wizard_conf_list ".addon.list"
    printf '%s\n' "You can add (a) new addons, remove (r) someone of them or skip (s). "
    aOpt=""
    read_input "aOpt"
    if [ ${aOpt} = "s" ]; then
        return 0
    fi
    printf '%s\n' "Enter all ADDONs to be updated (separate entries with a space)"
    aList=""
    read_input "aList"
    wizard_conf_addon_update ${aOpt} "${aList}"
    printf '%s\n' "Is the wizard configuration concluded? (y/n)"
    aOpt2=""
    read_input "aOpt2"
    case "${aOpt2}" in
        [Yy]|[Yy]es)
            return 0
            ;;
        [Nn]|[Nn]o)
            wizard_conf_addon
            ;;
        *)
            wizard_conf_addon
            ;;
    esac

}

wizard_conf_addon_update() {
    case ${1} in
        "a")
            for addon in ${2}; do
                printf '%s\n' "${addon}" >> ${ROOT_DESERT}/.addon.list
            done
            ;;
        "r")
            for addon in ${2}; do
                eval sed -i -e '/^${addon}/d' .addon.list
            done
            ;;
        *)
            ;;
    esac
}

wizard_conf() {
    wizard__print_L1 "Setting of the TARGETs LIST:"
    wizard_conf_target
    wizard__print_L1 "Setting of the ADDONs LIST:"
    wizard_conf_addon
}

wizard_function_target() {
    sleep ${SLEEP05}
    printf "\n"
    wizard__print_L2 "Setting of the TARGET:"
    wizard__print_L3 "Available TARGETs:"
    wizard__print_L4 "$(cat ${ROOT_DESERT}/.wizard.target.list | sed -e 's/\(.\)/*  \1/')"
    printf "TARGET"
    TARGET=""
    TARGETLIST=""
    INDEX=0
    while read line
    do
	    TARGETLIST="$TARGETLIST $line"
    done <".target.list"
    read_input "TARGET"
    if [ -z ${TARGET} ]; then
        err_L1 "invalid target!!"
        wizard_function_target
    fi
    for NAME in ${TARGETLIST}; do
	    if [ "${NAME}" = "${TARGET}" ]; then
	        INDEX=`expr $INDEX + 1`
	    fi
    done
    if [ $INDEX -eq 0 ]; then
	    err_L1 "invalid target!!"
        wizard_function_target
    fi
    _TARGET=1
    log_L1 "_TARGET=${_TARGET}" install.log
    log_L2 "TARGET=${TARGET}" install.log
    printf '%s\n' "--target ${TARGET} " >> ${INSTALL_CONF}
}

wizard_function_instMode() {
    sleep ${SLEEP05}
    printf "\n"
    wizard__print_L2 "Setting of the INSTALLATION MODE:"
    wizard__print_L3 "Available INSTALLATION MODEs:"
    wizard__print_L3 " development   (development mode)"
    wizard__print_L3 " release       (standard mode - keeps only the bin/ and lib/ folders)"
    printf "Installation Mode"
    INST_MODE=""
    read_input "INST_MODE"
    if [ ! "${INST_MODE}" = "development" ] && [ ! "${INST_MODE}" = "release" ]; then
        err_L1 "invalid installation mode!!"
        wizard_function_instMode
    fi
    _INST_MODE=1
    log_L1 "_INST_MODE=${_INST_MODE}" install.log
    log_L2 "INST_MODE=${INST_MODE}" install.log
    printf '%s\n' "--inst_mode ${INST_MODE} " >> ${INSTALL_CONF}
}

wizard_function_destFolder() {
    sleep ${SLEEP05}
    printf "\n"
    DESERT_DEF_PATH=$(cd $(pwd)/../ && pwd)/DESERT_buildCopy_${TARGET}
    wizard__print_L2 "Setting of the DESTINATION FOLDER"
    wizard__print_L3 " the default path is:"
    wizard__print_L3 " ${DESERT_DEF_PATH}"
    printf '%s\n' "Use this path as the destination folder for this installation? [Y/n] (Y, by default)"
    dest_folder_requist=""
    read_input "dest_folder_requist"
    case "${dest_folder_requist}" in
        [Yy]|[Yy]es)
            DEST_FOLDER=${DESERT_DEF_PATH}
            ;;
        [Nn]|[Nn]o)
            printf '%s\n' "please enter your desired installation path"
            DEST_FOLDER=""
            read_input "DEST_FOLDER"
            ;;
        "")
            DEST_FOLDER=${DESERT_DEF_PATH}
            ;;
        *)
            err_L1 "Input ${dest_folder_requist} not valid. Please write yes (y) or no (n)"
            wizard_function_destFolder
            ;;
    esac
    _DEST_FOLDER=1
    log_L1 "_DEST_FOLDER=${_DEST_FOLDER}" install.log
    log_L2 "DEST_FOLDER=${DEST_FOLDER}" install.log
    printf '%s\n' "--dest_folder ${DEST_FOLDER} " >> ${INSTALL_CONF}
}

wizard_function_wossRequire() {
    sleep ${SLEEP05}
    printf "\n"
    wizard__print_L2 "Setting of the WOSS_${WOSS_VERSION} libraries installation."
    wizard__print_L3 "WARNING: To install the WOSS libraries, you need the gfortran compiler "
    wizard__print_L3 "         to be available on your OS."
    wizard__print_L3 "         If you are using a Debian-based OS, you can try to execute"
    wizard__print_L3 "         this command: sudo apt-get install gfortran"
    wizard__print_L3 "WARNING: If you want to use enviromental databases for SSP, bathymetry,"
    wizard__print_L3 "         sediments, as well as for the characteristics of electro-acoustic transducers"
    wizard__print_L3 "         You can download the sediment and SSP databases at the following link:"
    wizard__print_L3 "         https://woss.dei.unipd.it/woss/files/WOSS-dbs-v1.6.0.tar.gz"
    wizard__print_L3 "         Please note that we cannot redistribute the GEBCO bathymetry database"
    wizard__print_L3 "         You can download the database by registering on the GEBCO web site at:"
    wizard__print_L3 "         http://http://www.gebco.net/"
    printf "Do you need the WOSS_${WOSS_VERSION} libraries (N, by default)?"
    woss_require=""
    read_input "woss_require"
    case "${woss_require}" in
        [Yy]|[Yy]es)
            WITHWOSS=1
            printf '%s\n' "--with-woss " >> ${INSTALL_CONF}
            ;;
        [Nn]|[Nn]o)
            WITHWOSS=0
            ;;
        "")
            WITHWOSS=0
            ;;
        *)
            err_L1 "Input ${woss_require} not valid. Please write yes (y) or no (n)"
            wizard_function_wossRequire
            ;;
    esac
    _WITHWOSS=1
    log_L1 "_WITHWOSS=${_WITHWOSS}" install.log
    log_L2 "woss_require=${woss_require}" install.log
    log_L2 "WITHWOSS=${WITHWOSS}" install.log
}

wizard_function_customPar() {
    sleep ${SLEEP05}
    printf "\n"
    wizard__print_L2 "Setting of the ${TARGET} CUSTOM PARAMETERs (OPTIONAL)"
    wizard__print_L3 " if you are using a custom TARGET_installer which needs additional parameters,"
    wizard__print_L3 " you can enter these parameters here."
    printf "Write your additional parameters"
    CUSTOM_PAR=""
    read_input "CUSTOM_PAR"
    _CUSTOM_PAR=1
    log_L1 "_CUSTOM_PAR=${_CUSTOM_PAR}" install.log
    log_L2 "CUSTOM_PAR=${CUSTOM_PAR}" install.log
    case "${CUSTOM_PAR}" in
        "")
            return 0
            ;;
        *)
            printf '%s\n' "--custom_par \"${CUSTOM_PAR}\" " >> ${INSTALL_CONF}
            ;;
    esac
}

cmd_line_function_addons() {
    # Handle the case where user put "ALL" Addons in non wizard modality
    if [ "$ADDONS" = "ALL" ]; then
        case "${WITHWOSS}" in
            "0")
                ALL_ADDONS=$(cat ./${ADDONS_FILE} | awk '{print $1}' | grep -v "ALL" | grep -v "wossgmmob3D" | grep -v "wossgroupmob3D" | awk '{printf "%s ",$0} END {print ""}')
                ADDONS=${ALL_ADDONS}
                printf '%s\n' "--addons \"${ADDONS}\"" >> ${INSTALL_CONF}
                ;;

            "1")
                ALL_ADDONS=$(cat ./${ADDONS_FILE} | awk '{print $1}' | grep -v "ALL" | awk '{printf "%s ",$0} END {print ""}')
                ADDONS=${ALL_ADDONS}
                printf '%s\n' "--addons \"${ADDONS}\"" >> ${INSTALL_CONF}
                ;;
        esac
    fi
}

wizard_function_addons(){
    #TODO replace the case statement with a single call to: wizard__print_L4
    sleep ${SLEEP05}
    printf "\n"
    wizard__print_L2 "Setting of the ADDONs (OPTIONAL)"
    wizard__print_L3 "list of the available ADDONs:"
    case "${OWNER_PERMISSION}" in
        "0")
            wizard__print_L4 "$(cat ${ROOT_DESERT}/${ADDONS_FILE} | sed -e 's/\(.\)/*  \1/')"
            ;;
        "1")
            wizard__print_L4 "$(cat ${ROOT_DESERT}/${ADDONS_FILE} | sed -e 's/\(.\)/*  \1/')"
            ;;
    esac

    printf "Enter all ADDONs to be installed (separate entries with a space)"
    ADDONS=""
    read_input "ADDONS"
    _ADDONS=1
    log_L1 "_ADDONS=${_ADDONS}" install.log
    log_L2 "ADDONS=${ADDONS}" install.log
    case "${ADDONS}" in
        "")
            continue
            ;;
        "ALL")
            case "${WITHWOSS}" in
                "0")
                    ALL_ADDONS=$(cat ./${ADDONS_FILE} | awk '{print $1}' | grep -v "ALL" | grep -v "wossgmmob3D" | grep -v "wossgroupmob3D" | awk '{printf "%s ",$0} END {print ""}')
                    ADDONS=${ALL_ADDONS}
                    printf '%s\n' "--addons \"${ADDONS}\"" >> ${INSTALL_CONF}
                    ;;

                "1")
                    ALL_ADDONS=$(cat ./${ADDONS_FILE} | awk '{print $1}' | grep -v "ALL" | awk '{printf "%s ",$0} END {print ""}')
                    ADDONS=${ALL_ADDONS}
                    printf '%s\n' "--addons \"${ADDONS}\"" >> ${INSTALL_CONF}
                    ;;
            esac
            ;;
        *)
            printf '%s\n' "--addons \"${ADDONS}\" " >> ${INSTALL_CONF}
            ;;
    esac
}

wizard_function() {
    info_L0 "WELCOME to the WIZARD MODE:"

    wizard_function_target
    wizard_function_instMode
    wizard_function_destFolder
    wizard_function_wossRequire
    wizard_function_customPar
    wizard_function_addons
}


install__print_help() {
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "fuction_name is: install__print_help()"
    fi
    printf '%s\n'  "\033[1mNAME\033[0m"
    printf '%s\n'  "       install.sh - installs the Desert Underwater framework"
    printf "\n"
    printf '%s\n'  "\033[1mSYNOPSIS\033[0m"
    printf '%s\n'  "       \033[1m./install.sh\033[0m [OPTION]"
    printf "\n"
    printf '%s\n'  "\033[1mDESCRIPTION\033[0m"
    printf '%s\n'  "       \033[1m-h, --help\033[0m"
    printf '%s\n'  "              display this help and exit"
    printf "\n"
    printf '%s\n'  "       \033[1m--wizard\033[0m"
    printf '%s\n'  "              install the Desert Framework using a step by step \"wizard\" mode" 
    printf "\n"
    printf '%s\n'  "       \033[1m--with-woss\033[0m"
    printf '%s\n'  "              install also the WOSS framework when installing Desert"
    printf "\n"
    printf '%s\n'  "       \033[1m--without-woss\033[0m"
    printf '%s\n'  "              install the Desert Framework without the WOSS framework"
    printf "\n"
    printf '%s\n'  "       \033[1m--target <YOUR_TARGET>\033[0m"
    printf '%s\n'  "              install Desert for YOUR_TARGET"
    printf "\n"
    printf '%s\n'  "       \033[1m--inst_mode <development|release> \033[0m"
    printf '%s\n'  "              install Desert in development or release mode"
    printf "\n"
    printf '%s\n'  "       \033[1m--dest_folder <YOUR_PATH>\033[0m"
    printf '%s\n'  "              set YOUR_PATH as the installation path"
    printf "\n"
    printf '%s\n'  "       \033[1m--custom_par <YOUR_PARAMETERS>\033[0m"
    printf '%s\n'  "              enters additional parameters for a custom installer (use \"...\" to group multiple parameters)"
    printf '%s\n'  ""
    printf '%s\n'  "       \033[1m--addons <ADDON_LIST>\033[0m"
    printf '%s\n'  "              specify THE LISt of ADDONs to be installed (use \"...\" to group multiple ADDONs)"
    printf "\n"
    printf '%s\n'  "\033[1mSEE ALSO\033[0m"
    printf '%s\n'  "            home-page: http://nautilus.dei.unipd.it/desert-underwater"
    printf '%s\n'  "         mailing-list: desert-usergroup@dei.unipd.it"
    printf '%s\n'  "                  git:"
    printf '%s\n'  "   desert-doxygen-doc: http://telecom.dei.unipd.it/ns/desert/DESERT_HTML_doxygen_doc/"
    printf "\n"
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "return of install__print_help(): ${output}"
        debug__print_screen_L1 "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯"
    fi
    return 0
}

unpack_desertframework() {
    log_L1 "[unpack_desertframework]: -->" ${INSTALL_LOG}
    #ZLIB_DIR=$ROOT/zlib-$ZLIB_VERSION
    #TCL_DIR=$ROOT/tcl-$TCL_VERSION
    #OTCL_DIR=$ROOT/otcl-$OTCL_VERSION
    #TCLCL_DIR=$ROOT/tclcl-$TCLCL_VERSION
    #NS_DIR=$ROOT/ns-$NS_VERSION
    #NSMIRACLE_DIR=$ROOT/nsmiracle-$NSMIRACLE_VERSION
    #BELLHOP_DIR=$ROOT/at/bin
    #NETCDF_DIR=$ROOT/netcdf-$NETCDF_VERSION
    #NETCDFCXX_DIR=$ROOT/netcdf-cxx-$NETCDFCXX_VERSION
    #WOSS_DIR=$ROOT/woss-$WOSS_VERSION
    FOLDER_LIST="${PATCHES_TAR_FILE}   \
                 ${ZLIB_TAR_FILE}      \
                 ${TCL_TAR_FILE}       \
                 ${OTCL_TAR_FILE}      \
                 ${TCLCL_TAR_FILE}     \
                 ${NS_TAR_FILE}        \
                 ${NSMIRACLE_TAR_FILE} \
                 ${HDF5_TAR_FILE}      \
                 ${BELLHOP_TAR_FILE}   \
                 ${NETCDF_TAR_FILE}    \
                 ${NETCDFCXX_TAR_FILE} \
                 ${WOSS_TAR_FILE}"
    if [ -d ${UNPACKED_FOLDER} ]; then
        log_L2 "[unpack_desertframework]: the ${UNPACKED_FOLDER} exists." ${INSTALL_LOG}
        warn_L1 "Detected folder"
        warn_L1 " ${UNPACKED_FOLDER}"
        warn_L2 "Do you want to overwrite it [Y/n]?"
        read_input "TMP_INPUT"
        case "${TMP_INPUT}" in
            [Yy]|[Yy]es)
                rm -rf ${UNPACKED_FOLDER}
                log_L2 "[unpack_desertframework]: the folder above exists. Do you want replace it [Y/n]? -> ${TMP_INPUT} " ${INSTALL_LOG}
                log_L2 "[unpack_desertframework]: the folder is removed." ${INSTALL_LOG}
                #mkdir ${DEST_FOLDER}
                #return 0
                ;;
            [Nn]|[Nn]o)
                log_L2 "[unpack_desertframework]: the folder above exists. Do you want replace it [Y/n]? -> ${TMP_INPUT} " ${INSTALL_LOG}
                log_L2 "[unpack_desertframework]: passing to next procedure" ${INSTALL_LOG}
                return 0
                ;;
            *)
                err_L1 "Input ${TMP_INPUT} not valid. Please write yes (y) or no (n)"
                unpack_desertframework
                return 0
                ;;
        esac
    fi
    mkdir -p ${UNPACKED_FOLDER}
    log_L2 "[unpack_desertframework]: ${UNPACKED_FOLDER} created." ${INSTALL_LOG}
    log_L2 "[unpack_desertframework]: unpacking all tar files in ${UNPACKED_FOLDER} folder" ${INSTALL_LOG}
    for tmp_folder in ${FOLDER_LIST}; do
        eval tmp_namefolder="${tmp_folder##*[\/]}"
        info_L1 "uncompressing ${tmp_namefolder} ..."
        if [ "${_DEBUG}" = "1" ]; then
            debug__print_screen_L1 "\${tmp_folder}      ${tmp_folder}"
            debug__print_screen_L1 "\${UNPACKED_FOLDER} ${UNPACKED_FOLDER}"
        fi
        tar xzf ${tmp_folder} -C ${UNPACKED_FOLDER}
        #if [ $? ne 0 ]; then
            #logERR "[unpack_desertframework]: can't possible to uncompress ${tmp_namefolder}"
            #err_L1 "[unpack_desertframework]: can't possible to uncompress ${tmp_namefolder}"
            #return 1
        #fi
        log_L2 "[unpack_desertframework]: ${tmp_namefolder} has been unpacked" ${INSTALL_LOG}
        ok_L1 "done!"
    done
    return 0
}

make_dir() {
    log_L1 "[make_dir]: -->" ${INSTALL_LOG}
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "fuction_name is: make_dir()"
    fi
    if [ -d ${DEST_FOLDER} ]; then
        warn_L1 "Detected"
        warn_L1 "${DEST_FOLDER}"
        warn_L2 "Do you want to overwrite it [Y/n]?"
        TMP_INPUT=""
        read_input "TMP_INPUT"
        case "${TMP_INPUT}" in
            [Yy]|[Yy]es)
                rm -rf ${DEST_FOLDER}
                log_L2 "[make_dir]: the folder above exists. Do you want replace it [Y/n]? -> ${TMP_INPUT} " ${INSTALL_LOG}
                #mkdir ${DEST_FOLDER}
                #return 0
                ;;
            [Nn]|[Nn]o)
                log_L2 "[make_dir]: the folder above exists. Do you want replace it [Y/n]? -> ${TMP_INPUT} " ${INSTALL_LOG}
                return 0
                ;;
            *)
                err_L1 "Input ${TMP_INPUT} not valid. Please write yes (y) or no (n)"
                make_dir
                ;;
        esac
    fi
    mkdir -p ${DEST_FOLDER}
    log_L2 "[make_dir]: folder ${DEST_FOLDER} created" ${INSTALL_LOG}
    return 0
}

call_installer() {
    log_L1 "[call_installer]: -->" ${INSTALL_LOG}
    #***
    # call_installer()
    #*
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "fuction_name is: call_installer()"
    fi

    cmd_line_function_addons
    PARAMETER4INSTALLER="${TARGET} ${INST_MODE} ${DEST_FOLDER} ${CUSTOM_PAR} ${ADDONS}"

    cd Installer
    log_L2 "[call_installer]: launching the installDESERT_${TARGET}.sh script " ${INSTALL_LOG}
    ./installDESERT_${TARGET}.sh ${PARAMETER4INSTALLER}
    if [ $? -ne 0 ]; then
        exit 1
    fi

    return 0
}

check_pktInstallation () {
    if [ -e .installed.check ]; then
        #warn_L1 "${package_DIR} seems installed!!!"
        warn_L1 "It seems that ${1} has been already installed."
        warn_L2 "Do you want to reinstall? [y/N] (N, by default)"
        read_input "TMP_checkPKT"
        case "${TMP_checkPKT}" in
            [Yy]|[Yy]es)
                rm -rf .installed.check
                ;;
            [Nn]|[Nn]o)
                return 1
                ;;
            "")
                return 1
                ;;
            *)
                warn_L2 "Input ${TMP_checkPKT} not valid."
                warn_L2 "THE CURRENT PACKAGE WILL NOT BE REINSTALLED."
                return 1
                ;;
        esac
    fi
}

handle_package() {
    log_L1 "[handle_package]: -->" ${INSTALL_LOG}
    log_L2 "[handle_package]: $*" ${INSTALL_LOG}

    if [ ! -e ${UNPACKED_FOLDER} ]; then
        err_L1 "ERROR: folder ${UNPACKED_FOLDER} NOT found!"
        exit 1
    fi

    eval package_DIR="\$${2}_DIR"

    #BUILD_TYPE=${1/\// }
    BUILD_TYPE=$(echo $1 | sed 's/\// /g')
    for build_type in $BUILD_TYPE; do

        case ${build_type} in
            host)
                if [ ! -d ${BUILD_HOST}/${package_DIR} ]; then
                    #TODO:check the following code lines
                    if [ ${2} = ${DESERT_DIR} ]; then
                        mkdir -p ${BUILD_HOST}/${DESERT_DIR}
                        ln -s ${BUILD_HOST}/${DESERT_DIR} ${DEST_FOLDER}
                        mv ${DEST_FOLDER}/${DESERT_DIR} ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-build
                        ln -s ${ROOT_DESERT}/${DESERT_DIR} ${DEST_FOLDER}
                        mv ${DEST_FOLDER}/${DESERT_DIR} ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-src
                    else
                        cp -r -i ${UNPACKED_FOLDER}/${package_DIR} ${BUILD_HOST}/${package_DIR}
                    fi
                fi

                cd ${BUILD_HOST}/${package_DIR}
                check_pktInstallation ${package_DIR}
                # exec the following commands in a subshell
                if [ "$?" -ne "1" ]; then
                    (
                        currentBuildLog=${BUILD_HOST}
                        ARCH=$HOST
                        build_$2 host
                    )
                fi
                if [ $? -ne 0 ]; then
                    err_L1 "EXIT FROM THE INSTALLATION PROCEDURE!"
                    exit 1
                fi

                touch .installed.check
                ;;
            target)
                if [ ! -d ${BUILD_TARGET}/${package_DIR} ]; then
                    if [ ${2} = ${DESERT_DIR} ]; then
                        mkdir -p ${BUILD_TARGET}/${DESERT_DIR}
                        ln -sf ${BUILD_TARGET}/${DESERT_DIR} ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-build
                        ln -sf ${ROOT_DESERT}/${DESERT_DIR} ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-src
                    else
                        cp -r -i ${UNPACKED_FOLDER}/${package_DIR} ${BUILD_TARGET}/${package_DIR}
                    fi
                fi
                cd ${BUILD_TARGET}/${package_DIR}
                check_pktInstallation ${package_DIR}
                # exec the following commands in a subshell
                if [ "$?" -ne "1" ]; then
                    (
                        currentBuildLog=${BUILD_TARGET}
                        . $CROSS_ENV_FILE
                        build_$2 target
                    )
                fi
                if [ $? -ne 0 ]; then
                    err_L1 "EXIT FROM THE INSTALLATION PROCEDURE!"
                    exit 1
                fi
                touch .installed.check
                ;;
            *)
                err_L1 "ERROR: handle_package() - ${build_type} parameter NOT matched!!"
                exit 1
                ;;
        esac
    done
}

handle_desert_ADDON() {
    log_L1 "[handle_desert_ADDON]: -->" ${INSTALL_LOG}
    log_L2 "[handle_desert_ADDON]: $*" ${INSTALL_LOG}

    #BUILD_TYPE=${1/\// }
    BUILD_TYPE=$(echo $1 | sed 's/\// /g')
    for build_type in $BUILD_TYPE; do

        case ${build_type} in
            host)
                if [ ! -d ${BUILD_HOST}/DESERT_ADDON/${2} ]; then
                    mkdir -p ${BUILD_HOST}/DESERT_ADDON/${2}
                    ln -sf ${BUILD_HOST}/DESERT_ADDON/ ${DEST_FOLDER}
                    mv ${DEST_FOLDER}/DESERT_ADDON ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-build
                    ln -sf ${ROOT_DESERT}/../DESERT_Addons/ ${DEST_FOLDER}
                    mv ${DEST_FOLDER}/DESERT_Addons ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-src
                fi

                cd ${BUILD_HOST}/DESERT_ADDON/${2}
                check_pktInstallation ${2}
                # exec the following commands in a subshell
                if [ "$?" -ne "1" ]; then
                    (
                        currentBuildLog=${BUILD_HOST}
                        ARCH=$HOST
                        build_DESERT_addon $2 host
                    )
                fi
                if [ $? -ne 0 ]; then
                    err_L1 "EXIT FROM THE INSTALLATION PROCEDURE!"
                    exit 1
                fi

                touch .installed.check
                ;;
            target)
                if [ ! -d ${BUILD_TARGET}/DESERT_ADDON/${2} ]; then
                    mkdir -p ${BUILD_TARGET}/DESERT_ADDON/${2}
                    ln -sf ${BUILD_TARGET}/DESERT_ADDON/ ${DEST_FOLDER}
                    mv ${DEST_FOLDER}/DESERT_ADDON ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-build
                    ln -sf ${ROOT_DESERT}/../DESERT_Addons/ ${DEST_FOLDER}
                    mv ${DEST_FOLDER}/DESERT_Addons ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-src
                fi

                cd ${BUILD_TARGET}/DESERT_ADDON/${2}
                check_pktInstallation ${2}
                # exec the following commands in a subshell
                if [ "$?" -ne "1" ]; then
                    (
                        currentBuildLog=${BUILD_TARGET}
                        . $CROSS_ENV_FILE
                        build_DESERT_addon $2 target
                    )
                fi
                if [ $? -ne 0 ]; then
                    err_L1 "EXIT FROM THE INSTALLATION PROCEDURE!"
                    exit 1
                fi
                touch .installed.check
                ;;
            *)
                err_L1 "ERROR: handle_package() - ${build_type} parameter NOT matched!!"
                exit 1
                ;;
        esac
    done
}

addon_installation_list() {
    info_L1 "installation of ADD-ON libraries ..."

    build_type=${1}
    addonList="${2}"
    for addon in ${addonList}; do
        handle_desert_ADDON ${build_type} ${addon}
    done
}

after_building() {
    log_L1 "[after_building]: -->" ${INSTALL_LOG}
    case "${INST_MODE}" in
        "development")
            (
                cd ${DEST_FOLDER}
                touch environment
                printf "export PATH=${DEST_FOLDER}/bin:\${PATH}\n" > environment
                printf "export LD_LIBRARY_PATH=${DEST_FOLDER}/lib:\${LD_LIBRARY_PATH}\n" >> environment
            )
            log_L2 "[after_building]: the environment file has been created" ${INSTALL_LOG}
            continue
            ;;
        "release")
            (
                cd ${DEST_FOLDER}
                MAKE_ENV="make_environment.sh"
                touch ${MAKE_ENV}
                chmod +x ${MAKE_ENV}
                printf "#!/bin/sh\n" > make_environment.sh
                printf "#\n" >> make_environment.sh
                printf "# Generated by ${DESERT_DIR} ${DESERT_VERSION} installation tool\n" >> make_environment.sh
                printf "#\n" >> make_environment.sh
                printf "# Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.\n" >> make_environment.sh
                printf "# All rights reserved.\n" >> make_environment.sh
                printf "#\n" >> make_environment.sh
                printf "# Redistribution and use in source and binary forms, with or without\n" >> make_environment.sh
                printf "# modification, are permitted provided that the following conditions\n" >> make_environment.sh
                printf "# are met:\n" >> make_environment.sh
                printf "# 1. Redistributions of source code must retain the above copyright\n" >> make_environment.sh
                printf "#    notice, this list of conditions and the following disclaimer.\n" >> make_environment.sh
                printf "# 2. Redistributions in binary form must reproduce the above copyright\n" >> make_environment.sh
                printf "#    notice, this list of conditions and the following disclaimer in the\n" >> make_environment.sh
                printf "#    documentation and/or other materials provided with the distribution.\n" >> make_environment.sh
                printf "# 3. Neither the name of the University of Padova (SIGNET lab) nor the\n" >> make_environment.sh
                printf "#    names of its contributors may be used to endorse or promote products\n" >> make_environment.sh
                printf "#    derived from this software without specific prior written permission.\n" >> make_environment.sh
                printf "#\n" >> make_environment.sh
                printf "# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n" >> make_environment.sh
                printf "# 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED\n" >> make_environment.sh
                printf "# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n" >> make_environment.sh
                printf "# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR\n" >> make_environment.sh
                printf "# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n" >> make_environment.sh
                printf "# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n" >> make_environment.sh
                printf "# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;\n" >> make_environment.sh
                printf "# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n" >> make_environment.sh
                printf "# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR\n" >> make_environment.sh
                printf "# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF\n" >> make_environment.sh
                printf "# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n" >> make_environment.sh
                printf "\n" >> make_environment.sh
                printf "touch environment\n" >> make_environment.sh
                printf 'printf "export PATH=$(pwd)/bin:\${PATH}\n" > environment\n' >> make_environment.sh
                printf 'printf "export LD_LIBRARY_PATH=$(pwd)/lib:\${LD_LIBRARY_PATH}\n" >> environment\n' >> make_environment.sh
            )
            log_L2 "[after_building]: release mode: removing the BUILD FOLDER" ${INSTALL_LOG}
            rm_build_folder
            ;;
        *)
            err_L1 "FATAL ERROR during the installation procedure. INST_MODE = ${INST_MODE} is NOT CORRECT!!!"
            exit 1
            ;;
    esac
    mv ${INSTALL_CONF} ${DEST_FOLDER}

    log_L1 "THE INSTALLATION PROCEDURE HAS BEEN COMPLETED" ${INSTALL_LOG}
    info_L1 "THE INSTALLATION PROCEDURE HAS BEEN COMPLETED"
    printf "\n"
    printf '%s\n'  "------------------------------------------------------------------------------"
    printf '%s\n'  "THE DESTINATION FOLDER IS: ${DEST_FOLDER}"
    printf "\n"
    printf '%s\n'  " WARNING: to use Desert and its dependencies (ns, ns-miracle, etc.) you must update your environment variables."
    printf '%s\n'  "          To do so, go to the folder above and run the following command BEFORE STARTING ANY TCL SCRIPTS:"
    printf "\n"
    case "${INST_MODE}" in
        "development")
            printf '%s\n'  "          $ source ${DEST_FOLDER}/environment"
            ;;
        "release")
            printf '%s\n'  "          $ ${DEST_FOLDER}/make_environment.sh"
            printf '%s\n'  "          then"
            printf '%s\n'  "          $ source ${DEST_FOLDER}/environment"
            ;;
        *)
            err_L1 "FATAL ERROR during the installation procedure. INST_MODE = ${INST_MODE} is NOT CORRECT!!!"
            exit 1
            ;;
    esac
    printf "\n"
    printf "DESERT doxygen documentation: https://signetlabdei.github.io/DESERT_Underwater_doc/\n"
    printf "\n"
    printf " THANK YOU FOR INSTALLING DESERT UNDERWATER.\n"
    printf '%s\n'  "------------------------------------------------------------------------------"
}

rm_build_folder(){
    RM_BUILD_FOLDER="${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-build        \
                     ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-src          \
                     ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-build \
                     ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-src   \
                     ${DEST_FOLDER}/include                                      \
                     ${DEST_FOLDER}/man                                          \
                     ${DEST_FOLDER}/share                                        \
                     ${BUILD_HOST}                                               \
                     ${BUILD_TARGET}                                             \
                     "
    for rm_folder in ${RM_BUILD_FOLDER}; do
        if [ -e ${rm_folder} ]; then
            rm -rf ${rm_folder}
        fi
    done
}

#-DEBUG FUNCTION---------------------------------------------------------------
debug__print_screen_L1() {
    printf "\033[40m\033[37m[DEBUG] $* \033[0m\n"
    return 0
}

check_sh() {
    SH_PATH=$(which sh)
    SH_PATH_LN=$(ls -la ${SH_PATH})
    if [ "${_DEBUG}" = "1" ]; then
        debug__print_screen_L1 "[which sh]: ${SH_PATH_LN}"
    fi
    return 0
}

debug__print_parameters() {
    debug__print_screen_L1 "TARGET       (TARGET)                       = ${TARGET}"
    debug__print_screen_L1 "INST_MODE    (Installation Mode)            = ${INST_MODE}"
    debug__print_screen_L1 "DEST_FOLDER  (Destination Folder)           = ${DEST_FOLDER}"
    debug__print_screen_L1 "BUILD_HOST   (host Destination Folder)      = ${BUILD_HOST}"
    debug__print_screen_L1 "BUILD_TARGET (target Destination Folder)    = ${BUILD_TARGET}"
    debug__print_screen_L1 "CUSTOM_PAR   (Custom Parameters)            = ${CUSTOM_PAR}"
    debug__print_screen_L1 "ADDONS       (names of the Addons)          = ${ADDONS}"
    return 0
}

delete_recursive_soft_link() {
    cd ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-build
    rm -rf DESERT_ADDON
    cd ${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-src
    rm -rf DESERT_Addons
    return 0
}

