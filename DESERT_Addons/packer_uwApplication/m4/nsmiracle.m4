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




AC_DEFUN([AC_ARG_WITH_NSMIRACLE],[
    NSMIRACLE_PATH=''
    NSMIRACLE_CPPLAGS=''
    NSMIRACLE_LDFLAGS=''
    NSMIRACLE_LIBADD=''

    AC_ARG_WITH([nsmiracle],
        [AS_HELP_STRING([--with-nsmiracle=<directory>],
            [use nsmiracle installation in <directory>])],
        [
            if test "x$withval" != "xno" ; then
                if test -d $withval ; then
                    NSMIRACLE_PATH="${withval}"

                    if test ! -f "${NSMIRACLE_PATH}/nsmiracle/module.h"  ; then
                        AC_MSG_WARN([could not find ${withval}/nsmiracle/module.h, is --with-nsmiracle=${withval} correct?])
                    fi

                    for dir in     \
                        nsmiracle  \
                        cbr        \
                        ip         \
                        link       \
                        mac802_11  \
                        marq       \
                        mobility   \
                        mphy       \
                        mmac       \
                        phy802_11  \
                        port       \
                        tcp        \
                        wirelessch \
                        aodv       \
                        mll        \
                        routing    \
                        aodv       \
                        uwm
                    do
                        #echo "considering dir \"$dir\""
                        NSMIRACLE_CPPFLAGS="$NSMIRACLE_CPPFLAGS -isystem${NSMIRACLE_PATH}/${dir}"
                        NSMIRACLE_LDFLAGS="$NSMIRACLE_LDFLAGS -L${NSMIRACLE_PATH}/${dir}"

                    done

                    for lib in               \
                        MiracleBasicMovement \
                        miracletcp           \
                        MiracleWirelessCh    \
                        miraclecbr           \
                        MiracleIp            \
                        MiraclePhy802_11     \
                        MiracleMac802_11     \
                        miracleport          \
                        Miracle              \
                        mphy                 \
                        marq                 \
                        mmac                 \
                        mll                  \
                        miraclelink          \
                        MiracleRouting       \
                        MiracleAodv          \
                        UwmStd               \
                        UwmStdPhyBpskTracer
                    do
                        NSMIRACLE_LIBADD="$NSMIRACLE_LIBADD -l${lib}"
                    done

                    NSMIRACLE_DISTCHECK_CONFIGURE_FLAGS="--with-nsmiracle=$withval"
                    AC_SUBST(NSMIRACLE_DISTCHECK_CONFIGURE_FLAGS)

                else
                    AC_MSG_WARN([nsmiracle path $withval is not a directory])
                fi
            fi
        ])

    AC_SUBST(NSMIRACLE_CPPFLAGS)
    AC_SUBST(NSMIRACLE_LDFLAGS)
    AC_SUBST(NSMIRACLE_LIBADD)
])

AC_DEFUN([AC_CHECK_NSMIRACLE],[
    # if test "x$NS_CPPFLAGS" = x ; then
    #     true
    #     AC_MSG_ERROR([NS_CPPFLAGS is empty!])
    # fi

    # if test "x$NSMIRACLE_CPPFLAGS" = x ; then
    #     true
    #     AC_MSG_ERROR([NSMIRACLE_CPPFLAGS is empty!])
    # fi

    # temporarily add NS_CPPFLAGS and NSMIRACLE_CPPFLAGS to CPPFLAGS
    BACKUP_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $NS_CPPFLAGS $NSMIRACLE_CPPFLAGS"

    AC_LANG_PUSH(C++)

    AC_MSG_CHECKING([for nsmiracle headers])

    AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([[
            #include<cltracer.h>
            ClMessageTracer* t;
            ]],[[
            ]]  )],
            [
             AC_MSG_RESULT([yes])
             found_nsmiracle=yes
            [$1]
            ],
            [
             AC_MSG_RESULT([no])
             found_nsmiracle=no
            [$2]
         AC_MSG_WARN([could not find nsmiracle])
            ])

    AM_CONDITIONAL([HAVE_NSMIRACLE], [test x$found_nsmiracle = xyes])

    # Restoring to the initial value
    CPPFLAGS="$BACKUP_CPPFLAGS"

    AC_LANG_POP(C++)
])

# AC_DEFUN([AC_PATH_NSMIRACLE], [
# AC_REQUIRE(AC_PATH_NS_ALLINONE)

# ########################################################
# # checking if ns-allinone path has been setup correctly
# ########################################################

# # temporarily add NS_CPPFLAGS and NSMIRACLE_CPPFLAGS to CPPFLAGS
# BACKUP_CPPFLAGS=$CPPFLAGS
# CPPFLAGS="$CPPFLAGS $NS_CPPFLAGS NSMIRACLE_CPPFLAGS"

# AC_MSG_CHECKING([if programs can be compiled against ns-miracle headers])
# AC_PREPROC_IFELSE(
# 	[AC_LANG_PROGRAM([[
# 		#include<cltracer.h>
# 		ClMessageTracer* t; 
# 		]],[[
# 		]]  )],
#         [AC_MSG_RESULT([yes])],
#         [
# 	  AC_MSG_RESULT([no])
# 	  AC_MSG_ERROR([could not compile a test program against ns-miracle headers. Is --with-ns-miracle set correctly? ])
#         ])

# # AC_CHECK_HEADERS([cltracer.h],,AC_MSG_ERROR([you must specify ns-miracle installation path using --with-ns-miracle=PATH]))

# # Restoring to the initial value
# CPPFLAGS=$BACKUP_CPPFLAGS
# ])
#
#
