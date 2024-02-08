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

# @name_file:   installDESERT_LOCAL.sh
# @author:      Ivano Calabrese
# @last_update: 2014.05.07
# --
# @brief_description:

# INCLUDE
#. ./commonVariables.sh
. ${ROOT_DESERT}/commonFunctions.sh

if [ "${_DEBUG}" = "1" ]; then
    debug__print_screen_L1 "inside --- $0"
fi

# curr_ver="$(gcc --version | head -n1 | cut -d" " -f3)"
# last_ver="6.2.0"
# if [ "$(printf "$last_ver\n$curr_ver" | sort -V | head -n1)" = "$curr_ver" -a "$curr_ver" != "$last_ver" ]; then
#     export GCC6_FLAG=""
# else
#     export GCC6_FLAG="-std=c++98"
# fi

DIRFOLDER="${ZLIB_DIR}      \
           ${TCL_DIR}       \
           ${OTCL_DIR}      \
           ${TCLCL_DIR}     \
           ${NS_DIR}        \
           ${NSMIRACLE_DIR} \
           ${BELLHOP_DIR}   \
           ${NETCDF_DIR}    \
           ${NETCDFCXX_DIR} \
           ${WOSS_DIR}      \
          "

#for dirfolder in ${DIRFOLDER}; do
    #printf " * ${dirfolder}"
    #cp -r ${UNPACKED_FOLDER}/${dirfolder} ${DEST_FOLDER}/.buildHost
#done

#***
# check and make the build folder for the host device
#*
if [ ! -d ${BUILD_HOST} ]; then
   mkdir -p ${BUILD_HOST}
fi

#***
# check and make the build folder for the target device
#*
if [ ! -d ${BUILD_TARGET} ]; then
   mkdir -p ${BUILD_TARGET}
fi

main() {
    #******************************************************************************
    # MAIN
    #     e.g handle_package host/target <pkt-name>
    #     e.g addon_installation_list host/target <addon-list>

    ## only for the cross-compilation session
    export CROSS_ENV_DIR=""
    export CROSS_ENV_FILE=""
    #*

    handle_package host ZLIB
    handle_package host TCL
    export PATH="${BUILD_HOST}/bin:$PATH"
    export LD_LIBRARY_PATH="${BUILD_HOST}/lib"
    handle_package host OTCL
    handle_package host TCLCL
    handle_package host NS
    handle_package host NSMIRACLE
    handle_package host DESERT
    if [ ${WITHWOSS} -eq 1 ]; then
        handle_package host HDF5
        handle_package host NETCDF
        handle_package host NETCDFCXX
        handle_package host BELLHOP
        handle_package host WOSS
    fi
    if [ ! -z "${ADDONS}" ]; then
        addon_installation_list host "${ADDONS}"
    fi
    #******************************************************************************
}

#***
# << ZLIB package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables ""build_zlib ()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_ZLIB() {
    info_L1 "zlib-${ZLIB_VERSION}"
    start="$(date +%s)"

    if [ $ARCH = $HOST ]; then
        if [ -f Makefile ]; then
            make distclean > "${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log" 2>&1
        fi
        info_L2 "configure  [$*]"
        ./configure --prefix=${DEST_FOLDER} >> "${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log" 2>&1
        if [ $? -ne 0 ] ; then
            err_L1 "Error during the configuration of zlib-${ZLIB_VERSION}! Exiting ..."
            tail -n 50 ${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log
            exit 1
        fi
    else
        if [ -f Makefile ]; then
            make distclean
        fi
        info_L2 "configure  [$*]"
        CC=$ARCH-gcc
        ./configure --prefix=${DEST_FOLDER} > "${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log"  2>&1
        if [ $? -ne 0 ] ; then
            err_L1 "Error during the configuration of zlib-${ZLIB_VERSION}! Exiting ..."
            tail -n 50 ${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log
            exit 1
        fi
    fi
    info_L2 "make       [$*]"
    make -j $MAKE_JOBS >> "${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log"  2>&1
    make install >> "${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the compilation of zlib-${ZLIB_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/zlib-${ZLIB_VERSION}-$*.log
        exit 1
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << TCL package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_TCL ()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_TCL() {
    info_L1 "tcl-${TCL_VERSION}"
    start="$(date +%s)"

    cd unix
    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/tcl-${TCL_VERSION}-$*.log"  2>&1
    fi

    info_L2 "configure  [$*]"
    ./configure --target=${ARCH}         \
                --host=${ARCH}           \
                --build=${HOST}          \
                --enable-threads         \
                --enable-load            \
                --enable-dll-unloading   \
                --prefix=${DEST_FOLDER}  \
                >> "${currentBuildLog}/tcl-${TCL_VERSION}-$*.log"  2>&1

    if [ $? -ne 0 ] ; then
        err_L1 "Error during the configuration of tcl-${TCL_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/tcl-${TCL_VERSION}-$*.log
        exit 1
    fi
    printf "\n" > ../compat/fixstrtod.c

    info_L2 "make       [$*]"
    make -j ${MAKE_JOBS} >> "${currentBuildLog}/tcl-${TCL_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the compilation of tcl-${TCL_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/tcl-${TCL_VERSION}-$*.log
        exit 1
    fi

    if [ ${ARCH} != ${HOST} ]; then
        sed -i -e "s,\.\(/\${TCL_EXE}\),LD_LIBRARY_PATH=${BUILD_HOST}/tcl${TCL_VERSION}/unix ${BUILD_HOST}/tcl${TCL_VERSION}/unix/\1,g" Makefile
    fi

    info_L2 "make inst  [$*]"
    make install-strip >> "${currentBuildLog}/tcl-${TCL_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the installation of tcl-${TCL_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/tcl-${TCL_VERSION}-$*.log
        exit 1
    fi

    cd ${DEST_FOLDER}/bin 
    ln -fs tclsh${TCL_VERSION%.*} tclsh
    cd - >> "${currentBuildLog}/tcl-${TCL_VERSION}-$*.log"  2>&1

    #TODO: check the following code lines
    if [ ${ARCH} = ${HOST} ]; then
        mkdir -p ${BUILD_HOST}/bin
        cp ${DEST_FOLDER}/bin/tclsh${TCL_VERSION%.*} ${BUILD_HOST}/bin
        cp -r ${DEST_FOLDER}/lib ${BUILD_HOST}/
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << OTCL package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_OTCL ()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_OTCL() {
    info_L1 "otcl-${OTCL_VERSION}"
    start="$(date +%s)"

    info_L2 "patch      [$*]"
    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/otcl-${OTCL_VERSION}-$*.log" 2>&1
    fi
    patch -p1 < ${UNPACKED_FOLDER}/${PATCHES_DIR}/otcl_${OTCL_VERSION}-remove-x.patch >> "${currentBuildLog}/otcl-${OTCL_VERSION}-$*.log"  2>&1
    #---
    mv configure.in configure.ac
    #---
    autoreconf >> "${currentBuildLog}/otcl-${OTCL_VERSION}-$*.log"  2>&1
    CFLAGS=-I../tcl-${TCL_VERSION}/unix/

    info_L2 "configure  [$*]"
    ./configure --target=${ARCH}                  \
                --host=${ARCH}                    \
                --build=${HOST}                   \
                --with-tcl=../tcl-${TCL_VERSION}/ \
                --with-tcl-ver=8.4                \
                --exec-prefix=${DEST_FOLDER}      \
                >> "${currentBuildLog}/otcl-${OTCL_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the configuration of otcl-${OTCL_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/otcl-${OTCL_VERSION}-$*.log
        exit 1
    fi

    info_L2 "make       [$*]"
    make >> "${currentBuildLog}/otcl-${OTCL_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the compilation of otcl-${OTCL_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/otcl-${OTCL_VERSION}-$*.log
        exit 1
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << TCLCL package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_TCLCL ()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_TCLCL() {
    info_L1 "tclcl-${TCLCL_VERSION}"
    start="$(date +%s)"

    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/tclcl-${TCLCL_VERSION}-$*.log"  2>&1
    fi

    if [ ${ARCH} != ${HOST} ]; then
        sed -i -e '/^\$(TCL2C):/,+2d' Makefile.in
        cp ${BUILD_HOST}/${TCLCL_DIR}/tcl2c++ ${BUILD_TARGET}/${TCLCL_DIR}/tcl2c++
    fi

    info_L2 "patch      [$*]"
    patch -p1 < ${UNPACKED_FOLDER}/${PATCHES_DIR}/tclcl_${TCLCL_VERSION}-remove-x.patch >> "${currentBuildLog}/tclcl-${TCLCL_VERSION}-$*.log"  2>&0
    #---
    mv configure.in configure.ac
    #---
    info_L2 "autoreconf [$*]"
    autoreconf >> "${currentBuildLog}/tclcl-${TCLCL_VERSION}-$*.log"  2>&1
    CFLAGS=-I../tcl-${TCL_VERSION}/unix/
    info_L2 "configure  [$*]"
    ./configure --target=${ARCH}                    \
                --host=${ARCH}                      \
                --build=${HOST}                     \
                --with-tcl=../tcl-${TCL_VERSION}/   \
                --with-tcl-ver=8.4                  \
                --with-zlib=${DEST_FOLDER} \
                --exec-prefix=${DEST_FOLDER}        \
                >> "${currentBuildLog}/tclcl-${TCLCL_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the configuration of tclcl-${TCLCL_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/tclcl-${TCLCL_VERSION}-$*.log
        exit 1
    fi

    info_L2 "make       [$*]"
    make >> "${currentBuildLog}/tclcl-${TCLCL_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the compilation of tclcl-${TCLCL_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/tclcl-${TCLCL_VERSION}-$*.log
        exit 1
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << NS package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_NS ()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_NS() {
    info_L1 "ns-${NS_VERSION}"
    start="$(date +%s)"
    #------------------------------------------------------------------------------------------
    # The follow operation is necessary to only cross-compile with 32bit machines.
    # This trick maybe isn't the best way to resolve the not cross-compilation for the 32bit machines,
    # but at the moment is of sure the fastest and less invasive solution.
    #mv ${DEST_FOLDER}/lib/libtcl8.4.so ${DEST_FOLDER}/lib/libtcl8.4.so.BAK
    #------------------------------------------------------------------------------------------

    info_L2 "patch      [$*]"
    patch -p1 < ${UNPACKED_FOLDER}/${PATCHES_DIR}/ns-${NS_VERSION}-without-x-cross.patch > "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    patch -p1 < ${UNPACKED_FOLDER}/${PATCHES_DIR}/ns-${NS_VERSION}-compile.patch >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    patch -p1 < ${UNPACKED_FOLDER}/${PATCHES_DIR}/ns-${NS_VERSION}-fix-gcc-4.7.patch >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    patch -p1 < ${UNPACKED_FOLDER}/${PATCHES_DIR}/ns-real-time.patch >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    mv configure.in configure.ac
    # autoreconf || true >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1

    autoreconf >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1

    info_L2 "configure  [$*]"
    if [ -f Makefile ] ; then
        make distclean >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    fi
    ./configure --enable-static                                        \
                --target=${ARCH}                                       \
                --host=${ARCH}                                         \
                --build=${HOST}                                        \
                --with-tcl=${currentBuildLog}/tcl-${TCL_VERSION}       \
                --with-tcl-ver=8.4                                     \
                --with-otcl=${currentBuildLog}/otcl-${OTCL_VERSION}    \
                --with-tclcl=${currentBuildLog}/tclcl-${TCLCL_VERSION} \
                --prefix=${DEST_FOLDER}                                \
                --exec-prefix=${DEST_FOLDER}                           \
                >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the configuration of ns-${NS_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/ns-${NS_VERSION}-$*.log
        exit 1
    fi
    #FIXME
    printf '#define RANDOM_RETURN_TYPE long\n' >> autoconf.h
    printf 'using namespace std;\n'            >> autoconf.h
    printf '#define HAVE_STL 1\n'              >> autoconf.h

    info_L2 "make       [$*]"
    make -j ${MAKE_JOBS} >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the compilation of ns-${NS_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/ns-${NS_VERSION}-$*.log
        exit 1
    fi

    mkdir -p ${DEST_FOLDER}/bin
    make install-ns >> "${currentBuildLog}/ns-${NS_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the installation of ns-${NS_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/ns-${NS_VERSION}-$*.log
        exit 1
    fi

    if [ ${ARCH} != ${HOST} ]; then
        ${STRIP} ${DEST_FOLDER}/bin/ns
    fi

    #------------------------------------------------------------------------------------------
    # To restore the libtcl8.4.so library
    #mv ${DEST_FOLDER}/lib/libtcl8.4.so.BAK ${DEST_FOLDER}/lib/libtcl8.4.so
    #------------------------------------------------------------------------------------------
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << NSMIRACLE package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_NSMIRACLE ()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_NSMIRACLE() {
    info_L1 "nsmiracle-${NSMIRACLE_VERSION}"
    start="$(date +%s)"

    cd ..
    ln -sf tcl-${TCL_VERSION}/generic include
    cd - > "${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log"  2>&1

    info_L2 "patch      [$*]"
    patch -p1 < ${UNPACKED_FOLDER}/${PATCHES_DIR}/nsmiracle-trunk-fix-libs-dependence.patch >> "${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log"  2>&1
    if [ -e Makefile ]; then
        make distclean >> "${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log"  2>&1
    fi

    ./autogen.sh >> "${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log"  2>&1
    info_L2 "configure  [$*]"
    CXXFLAGS="-Wno-write-strings"                     \
      CFLAGS="-Wno-write-strings"                     \
    ./configure --target=${ARCH}                      \
                --host=${ARCH}                        \
                --build=${HOST}                       \
                --with-ns-allinone=${currentBuildLog} \
                --prefix=${DEST_FOLDER}               \
                >> "${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the configuration of nsmiracle-${NSMIRACLE_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log
        exit 1
    fi

    info_L2 "make       [$*]"
    make >> "${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the compilation of nsmiracle-${NSMIRACLE_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log
        exit 1
    fi

    make install-strip >> "${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the installation of nsmiracle-${NSMIRACLE_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}-$*.log
        exit 1
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << DESERT package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_DESERT ()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_DESERT() {
    info_L1 "desert-${DESERT_VERSION}"
    start="$(date +%s)"

    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/desert-${DESERT_VERSION}-$*.log"  2>&1
        info_L2 "make-clean [$*]"
        if [ $? -ne 0 ] ; then
            err_L1 "Error during the make distclean of DESERT! Exiting ..."
            tail -n 50 ${currentBuildLog}/desert-${DESERT_VERSION}-$*.log
            exit 1
        fi
    fi

    (
        cd ${ROOT_DESERT}/${DESERT_DIR}
            ./autogen.sh >> /dev/null  2>&1
            ./autogen.sh >> "${currentBuildLog}/desert-${DESERT_VERSION}-$*.log"  2>&1
    )

    info_L2 "configure  [$*]"
    CXXFLAGS="-Wno-write-strings -std=c++11"                                                                  \
      CFLAGS="-Wno-write-strings"                                                                             \
    ${ROOT_DESERT}/${DESERT_DIR}/configure --target=${ARCH}                                                   \
                                           --host=${ARCH}                                                     \
                                           --build=${HOST}                                                    \
                                           --with-ns-allinone=${currentBuildLog}                              \
                                           --with-nsmiracle=${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION} \
                                           --prefix=${DEST_FOLDER}                                            \
                                           >> "${currentBuildLog}/desert-${DESERT_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the configuration of DESERT! Exiting ..."
        tail -n 50 ${currentBuildLog}/desert-${DESERT_VERSION}-$*.log
        exit 1
    fi
    info_L2 "patch      [$*]"
    (
        cat "${UNPACKED_FOLDER}/${PATCHES_DIR}/001-desert-2.0.0-libtool-no-verbose.patch" | patch -p1 >> "${currentBuildLog}/desert-${DESERT_VERSION}-$*.log"  2>&1
    )

    info_L2 "make       [$*]"
    make >> "${currentBuildLog}/desert-${DESERT_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the compilation of DESERT! Exiting ..."
        tail -n 50 ${currentBuildLog}/desert-${DESERT_VERSION}-$*.log
        exit 1
    fi

    info_L2 "make inst  [$*]"
    make install-strip >> "${currentBuildLog}/desert-${DESERT_VERSION}-$*.log"  2>&1
    if [ $? -ne 0 ] ; then
        err_L1 "Error during the installation of DESERT! Exiting ..."
        tail -n 50 ${currentBuildLog}/desert-${DESERT_VERSION}-$*.log
        exit 1
    fi

    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << DESERT_addon package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_DESERT_addon()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_DESERT_addon() {
    info_L2 "INSTALLATION of ${1} as ADD-ON"
    src_addon_path="${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-src/${1}"
    start="$(date +%s)"
    if [ -d ${src_addon_path} ]; then
        (
            cd ${src_addon_path}
            ./autogen.sh > "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
        )
        if [ -e Makefile ] ; then
            info_L2 "make-clean [${2}]"
            make distclean >> "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
            if [ $? -ne 0 ] ; then
                err_L1 "Error during the make distclean of ${1}! Exiting ..."
                tail -n 50 ${currentBuildLog}/DESERT_ADDON/${1}-${2}.log
                exit 1
            fi
        fi

        (
            cd ${src_addon_path}
            ./autogen.sh >> "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
        )

        info_L2 "configure  [${2}]"
        case ${WITHWOSS} in
            0)
                CXXFLAGS="-Wno-write-strings"                                                                         \
                CFLAGS="-Wno-write-strings"                                                                                      \
                ${src_addon_path}/configure --target=$ARCH                                                                       \
                                            --host=$ARCH                                                                         \
                                            --build=$HOST                                                                        \
                                            --with-ns-allinone=${currentBuildLog}                                                \
                                            --with-nsmiracle=${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}                   \
                                            --with-desert=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-src                     \
                                            --with-desert-build=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-build             \
                                            --with-desertAddon=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-src         \
                                            --with-desertAddon-build=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-build \
                                            --prefix=${DEST_FOLDER}                                                              \
                                            >> "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
                ;;
            1)
                CXXFLAGS="-Wno-write-strings"                                                                         \
                CFLAGS="-Wno-write-strings"                                                                                      \
                CPPFLAGS=-I${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}/cxx                                                \
                LDFLAGS=-L${DEST_FOLDER}/lib                                                                                     \
                ${src_addon_path}/configure --target=$ARCH                                                                       \
                                            --host=$ARCH                                                                         \
                                            --build=$HOST                                                                        \
                                            --with-ns-allinone=${currentBuildLog}                                                \
                                            --with-nsmiracle=${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION}                   \
                                            --with-desert=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-src                     \
                                            --with-desert-build=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-build             \
                                            --with-desertAddon=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-src         \
                                            --with-desertAddon-build=${DEST_FOLDER}/${DESERT_DIR}-${DESERT_VERSION}-ADDONS-build \
                                            --with-woss=${currentBuildLog}/woss-${WOSS_VERSION}                                  \
                                            --prefix=${DEST_FOLDER}                                                              \
                                            >> "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
                ;;
            *)
                exit 1
                ;;
        esac
        if [ $? -ne 0 ]; then
            err_L1 "Error during the configuration of ${1}! Exiting ..."
            tail -n 50 ${currentBuildLog}/DESERT_ADDON/${1}-${2}.log
            exit 1
        fi

        info_L2 "patch      [${2}]"
        (
            cat "${UNPACKED_FOLDER}/${PATCHES_DIR}/001-desert-2.0.0-addon-libtool-no-verbose.patch" | patch -p1 >> "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
        )

        info_L2 "make       [${2}]"
        make >> "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
        if [ $? -ne 0 ]; then
            err_L1 "Error during the compilation of ${1}! Exiting ..."
            tail -n 50 ${currentBuildLog}/DESERT_ADDON/${1}-${2}.log
            exit 1
        fi

        info_L2 "make inst  [${2}]"
        make install >> "${currentBuildLog}/DESERT_ADDON/${1}-${2}.log"  2>&1
        if [ $? -ne 0 ]; then
            err_L1 "Error during the installation of ${1}! Exiting ..."
            tail -n 50 ${currentBuildLog}/DESERT_ADDON/${1}-${2}.log
            exit 1
        fi
    else
        err_L1 "No add-on '${1}' found. Is the name correct?"
        err_L1 "You have these add-ons available:"
        err_L1 "$(ls ../ | sed -e 's/\(ADDON_\)/                             - \1/' | grep ADDON_) "
        exit 1
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"

    return 0
}

#***
# << NETCDF package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_NETCDF()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*

build_HDF5() {
    info_L1 "hdf-5-${HDF5_VERSION}"
    start="$(date +%s)"

    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/hdf5-${HDF5_VERSION}-$*.log" 2>&1
    fi

    info_L2 "configure  [$*]"
    CXXFLAGS="-Wno-write-strings"       \
    CFLAGS="-Wno-write-strings"       \
    ./configure --target=${ARCH}        \
                --host=${ARCH}          \
                --build=${HOST}         \
                --enable-shared         \
                --with-zlib=${DEST_FOLDER} \
                --prefix=${DEST_FOLDER} \
                >> "${currentBuildLog}/hdf5-${HDF5_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the configuration of hdf5-${HDF5_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/hdf5-${HDF5_VERSION}-$*.log
        exit 1
    fi

    info_L2 "make       [$*]"
    make install >> "${currentBuildLog}/hdf5-${HDF5_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the compilation or installation of hdf5-${HDF5_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/hdf5-${HDF5_VERSION}-$*.log
        exit 1
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

build_NETCDF() {
    info_L1 "netcdf-${NETCDF_VERSION}"
    start="$(date +%s)"

    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/netcdf-${NETCDF_VERSION}-$*.log" 2>&1
    fi

    info_L2 "configure  [$*]"
    CXXFLAGS="-Wno-write-strings"       \
    CFLAGS="-Wno-write-strings"       \
#    CPPFLAGS="$CPPFLAGS -I${DEST_FOLDER}/include" \
#    LDFLAGS="$LDFLAGS -L${DEST_FOLDER}/include/lib" \
    ./configure --target=${ARCH}        \
                --host=${ARCH}          \
                --build=${HOST}         \
                --enable-shared         \
                --enable-netcdf-4      \
                --disable-dap           \
                --disable-byterange     \
                --disable-libxml2       \
                --prefix=${DEST_FOLDER} \
                CPPFLAGS="$CPPFLAGS -I${DEST_FOLDER}/include" \
                LDFLAGS="$LDFLAGS -L${DEST_FOLDER}/lib" \
                >> "${currentBuildLog}/netcdf-${NETCDF_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the configuration of netcdf-${NETCDF_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/netcdf-${NETCDF_VERSION}-$*.log
        exit 1
    fi

    info_L2 "make       [$*]"
    make install >> "${currentBuildLog}/netcdf-${NETCDF_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the compilation or installation of netcdf-${NETCDF_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/netcdf-${NETCDF_VERSION}-$*.log
        exit 1
    fi
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << NETCDFCXX package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_NETCDFCXX()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_NETCDFCXX() {
    info_L1 "netcdf-cxx-${NETCDFCXX_VERSION}"
    start="$(date +%s)"

    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}-$*.log" 2>&1
    fi
    info_L2 "configure  [$*]"
    CXXFLAGS="-Wno-write-strings"                           \
    CFLAGS="-Wno-write-strings"                           \
    ./configure --target=${ARCH}                            \
                --host=${ARCH}                              \
                --build=${HOST}                             \
                --enable-shared                             \
                --prefix=${DEST_FOLDER}                     \
                CPPFLAGS="$CPPFLAGS -I${DEST_FOLDER}/include" \
                LDFLAGS="$LDFLAGS -L${DEST_FOLDER}/lib" \
                >> "${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the configuration of netcdf-cxx-${NETCDFCXX_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}-$*.log
        exit 1
    fi

    info_L2 "make       [$*]"
    make install >> "${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the compilation or installation of netcdf-cxx-${NETCDFCXX_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}-$*.log
        exit 1
    fi

    # ln -sf ${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}/cxx/netcdfcpp.h ../netcdf-4.2.1.1/include/ ????
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << BELLHOP package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_BELLHOP()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_BELLHOP() {
    info_L1 "bellhop"
    start="$(date +%s)"

    type gfortran > /dev/null
    if [ $? -ne 0 ]; then
        err_L1 "gfortran not found. gfortran is required to compile Bellhop! Exiting ..."
        err_L1 "if you are using a Debian-based OS, try to execute this command:"
        err_L1 "sudo apt-get install gfortran"
        exit 1
    fi

    if [ -f Makefile ]; then
        make clean > "${currentBuildLog}/bellhop-$*.log" 2>&1
    fi

    info_L2 "make       [$*]"
    make >> "${currentBuildLog}/bellhop-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the compilation or installation of Bellhop! Exiting ..."
        exit 1
    fi
    cp Bellhop/bellhop.exe ${DEST_FOLDER}/bin
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

#***
# << WOSS package >>
# -------
# This function allows the compilation/cross-compilation of the zlib package.
# Through the:
#    ${ARCH}
#    ${HOST}
# variables "build_WOSS()" decides if do a compilation or a cross-compilation:
#    ${ARCH} = ${HOST}  -> compile
#    ${ARCH} != ${HOST} -> cross-compile

#TODO:
# (v) add the "return check" after each compile command. Moreover add "tail -n 50" command when a error compile happen.
#*
build_WOSS() {
    info_L1 "woss-$WOSS_VERSION"
    start="$(date +%s)"

    if [ -f Makefile ]; then
        make distclean > "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log" 2>&1
    fi
    # In case of patches...
    #patch -p1 -i $PATCHES_DIR/woss-1.2.0-fix-gcc-4.7.patch >> "$LOG_DIR/woss.log" 2>&0
    #if [ $? -ne 0 ]
    #then
    #    print_error "woss-$WOSS_VERSION error during patching! Exiting ..."
    #    exit
    #fi
    ./autogen.sh >> "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log" 2>&1
    printf ">>>\n" >> "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log"
    ./autogen.sh >> "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error with autogen.sh! Exiting ..."
        tail -n 50 ${currentBuildLog}/woss-${WOSS_VERSION}-$*.log
        exit 1
    fi
    info_L2 "configure  [$*]"
    CXXFLAGS="-Wno-write-strings"                                                  \
      CFLAGS="-Wno-write-strings"                                                  \
    CPPFLAGS=-I${currentBuildLog}/netcdf-cxx-${NETCDFCXX_VERSION}/cxx               \
     LDFLAGS=-L${DEST_FOLDER}/lib                                                  \
    ./configure --target=${ARCH}                                                   \
                --host=${ARCH}                                                     \
                --build=${HOST}                                                    \
                --with-ns-allinone=${currentBuildLog}                              \
                --with-nsmiracle=${currentBuildLog}/nsmiracle-${NSMIRACLE_VERSION} \
                --with-netcdf4=${DEST_FOLDER}                                       \
                --with-pthread                                                     \
                --prefix=${DEST_FOLDER}                                            \
                >> "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log" 2>&1

    if [ $? -ne 0 ]; then
        err_L1 "Error during the configuration of woss-${WOSS_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/woss-${WOSS_VERSION}-$*.log
        exit 1
    fi

    if [ -f Makefile ]; then
        make clean >>  "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log" 2>&1
    fi
    info_L2 "make       [$*]"
    make >>  "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the compilation of woss-${WOSS_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/woss-${WOSS_VERSION}-$*.log
        exit 1
    fi
    make check >> ${currentBuildLog}/woss-${WOSS_VERSION}-$*.log 2>&1
    if [ $? -ne 0 ]; then
	err_L1 "Error during make check of woss-${WOSS_VERSION}! Exiting..."
	tail -n 50 ${currentBuildLog}/woss-${WOSS_VERSION}-$*.log
	exit 1
    fi
    make install >>  "${currentBuildLog}/woss-${WOSS_VERSION}-$*.log" 2>&1
    if [ $? -ne 0 ]; then
        err_L1 "Error during the installation of woss-${WOSS_VERSION}! Exiting ..."
        tail -n 50 ${currentBuildLog}/woss-${WOSS_VERSION}-$*.log
        exit 1
    fi
    info_L2 "make inst  [$*]"
    elapsed=`expr $(date +%s) - $start`
    ok_L1 "completed in ${elapsed}s"
}

main

