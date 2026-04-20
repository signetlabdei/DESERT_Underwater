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



AC_DEFUN([AC_PATH_NS_ALLINONE], [

NS_ALLINONE_PATH=''
NS_PATH=''
TCL_PATH=''
OTCL_PATH=''
NS_CPPFLAGS=''

AC_ARG_WITH([ns-allinone],
    [AS_HELP_STRING([--with-ns-allinone=<directory>],
        [use ns-allinone installation in <directory>, where it is expected to find ns, tcl, otcl and tclcl subdirs])],
    [
        if test ! -d $withval ; then
            AC_MSG_ERROR([ns-allinone path $withval is not valid])
        else

        NS_ALLINONE_PATH=$withval

        NS_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d ns-* | head -n 1`
        TCL_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d * | grep -e 'tcl[0-9].*' | head -n 1`
        TCLCL_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d tclcl-* | head -n 1`
        OTCL_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d otcl-* | head -n 1`

        NS_CPPFLAGS="-isystem $NS_ALLINONE_PATH/include -isystem $NS_PATH -isystem $TCLCL_PATH -isystem $OTCL_PATH"


        NS_ALLINONE_DISTCHECK_CONFIGURE_FLAGS="--with-ns-allinone=$withval"
        AC_SUBST(NS_ALLINONE_DISTCHECK_CONFIGURE_FLAGS)

        fi
    ])

    if test x$NS_ALLINONE_PATH = x ;    then
        AC_MSG_ERROR([you must specify ns-allinone installation path using --with-ns-allinone=PATH])
    fi

    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/mac"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/propagation"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/mobile"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/pcap"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/tcp"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/sctp"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/common"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/link"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/queue"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/trace"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/adc"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/apps"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/routing"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/tools"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/classifier"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/mcast"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/diffusion3/lib"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/diffusion3/lib/main"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/diffusion3/lib/nr"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/diffusion3/ns"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/diffusion3/filter_core"
    NS_CPPFLAGS="$NS_CPPFLAGS -isystem $NS_PATH/asim"

    AC_SUBST(NS_CPPFLAGS)
    AC_MSG_CHECKING([for NS_LDFLAGS and NS_LIBADD type])

    system=`uname -s`
    case $system in
        CYGWIN*)
            AC_MSG_RESULT([cygwin])
            echo "running cygwin"
            NS_LDFLAGS=" -shared -no-undefined -L${NS_PATH} -Wl,--export-all-symbols -Wl,--enable-auto-import  -Wl,--whole-archive  "
            NS_LIBADD=" -lns"
            ;;
        *)
            AC_MSG_RESULT([none needed])
            # OK for linux, should be fine for unix in general
            NS_LDFLAGS=""
            NS_LIBADD=""
            ;;
    esac

    AC_SUBST(NS_LDFLAGS)
    AC_SUBST(NS_LIBADD)


    ########################################################
    # checking if ns-allinone path has been setup correctly
    ########################################################

    # temporarily add NS_CPPFLAGS to CPPFLAGS
    BACKUP_CPPFLAGS=$CPPFLAGS
    CPPFLAGS=$NS_CPPFLAGS
    #BACKUP_CFLAGS=$CFLAGS
    #CFLAGS=$NS_CPPFLAGS


    dnl AC_CHECK_HEADERS([tcl.h],,AC_MSG_ERROR([could not find tcl.h]))
    dnl AC_CHECK_HEADERS([otcl.h],,AC_MSG_ERROR([could not find otcl.h]))

    dnl AC_CHECK_HEADERS([tclcl.h],,AC_MSG_ERROR([could not find tclcl.h])
    dnl         [
    dnl            #if HAVE_TCL_H
    dnl            #include <tcl.h>
    dnl            #endif
    dnl         ])

    AC_LANG_PUSH(C++)

    AC_MSG_CHECKING([for ns-allinone installation])

    AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([[
            #include<tcl.h>
            #include<otcl.h>
            #include<tclcl.h>
            #include<packet.h>
            Packet* p;
            ]],[[
            p = new packet;
            delete p;
            ]]  )],
            [AC_MSG_RESULT([ok])],
            [
          AC_MSG_RESULT([FAILED!])
          AC_MSG_ERROR([Could not find NS headers. Is --with-ns-allinone set correctly? ])
            ])


    AC_MSG_CHECKING([if ns-allinone installation has been patched for dynamic libraries])

    AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([[
            #include<tcl.h>
            #include<otcl.h>
            #include<tclcl.h>
            #include<packet.h>
            ]],[[
            p_info::addPacket("TEST_PKT");
            ]]  )],
            [AC_MSG_RESULT([yes])],
            [
          AC_MSG_RESULT([NO!])
          AC_MSG_ERROR([The ns-allinone installation in $NS_ALLINONE_PATH has not been patched for dynamic libraries. 
                    Either patch it or change the --with-ns-allinone switch so that it refers to a patched version.	])
            ])

    AC_LANG_POP(C++)

    # Restoring to the initial value
    CPPFLAGS=$BACKUP_CPPFLAGS
    #CFLAGS=$BACKUP_CFLAGS

    ## AC_ARG_VAR([TCLCL_PATH],[blah blah blah])
    ## AC_PATH_PROG([TCL2CPP],[tcl2c++],[none],[$PATH:$TCLCL_PATH])

    AC_ARG_VAR([TCL2CPP],[tcl2c++ executable])
    AC_PATH_PROG([TCL2CPP],[tcl2c++],[none],[$PATH:$TCLCL_PATH])
    if test "x$TCL2CPP" = "xnone" ;    then
        AC_MSG_ERROR([could not find tcl2c++])
    fi
])

