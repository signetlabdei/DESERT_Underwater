#
# Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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




AC_DEFUN([AC_ARG_WITH_WOSS],[

    WOSS_PATH=''
    WOSS_CPPLAGS=''
    WOSS_LDFLAGS=''
    WOSS_LIBADD=''

    AC_ARG_WITH([woss],
        [AS_HELP_STRING([--with-woss=<directory>],[use woss installation in <directory>])],
        [
            if test "x$withval" != "xno" ; then
                if test -d $withval ; then
                    WOSS_PATH="${withval}"
                    relevantheaderfile="${WOSS_PATH}/woss/woss.h"
        
                    if test ! -f "${relevantheaderfile}"; then
                        AC_MSG_ERROR([could not find ${relevantheaderfile}, is --with-woss=${withval} correct?])
                    fi      

                    for dir in  \ 
                        uwm \
                        woss \
                        woss/woss_def \
                        woss/woss_db \
                        woss_phy 
                    do
                        WOSS_CPPFLAGS="$WOSS_CPPFLAGS -I${WOSS_PATH}/${dir}"
                        WOSS_LDFLAGS="$WOSS_LDFLAGS -L${WOSS_PATH}/${dir}"
                    done

                    for lib in \
                        UwmStd \
                        WOSS \
                        WOSSPhy
                    do
                        WOSS_LIBADD="$WOSS_LIBADD -l${lib}"
                    done    

                    WOSS_DISTCHECK_CONFIGURE_FLAGS="--with-woss=$withval"
                    AC_SUBST(WOSS_DISTCHECK_CONFIGURE_FLAGS)

                    AC_MSG_WARN([---------------------------------------------------------------------])
                    AC_MSG_WARN([--with-woss parameter has been set when you ran configure file])
                    AC_MSG_WARN([the Makefile.in file has been modified deleding:])
                    AC_MSG_WARN([  - uwm])
                    AC_MSG_WARN([---------------------------------------------------------------------])
                    sed -i -e '/uwm/d' ./Makefile.in

                else   
                    AC_MSG_ERROR([woss path $withval is not a directory])
                fi
            fi

        ],
        [
            AC_MSG_WARN([---------------------------------------------------------------------])
            AC_MSG_WARN([--with-woss parameter has been omitted when you ran configure file])
            AC_MSG_WARN([the Makefile.in file has been modified deleding:])
            AC_MSG_WARN([  - uw-t-lohi])
#            AC_MSG_WARN([  - bpsk_db])
#            AC_MSG_WARN([  - uwphysical])
#            AC_MSG_WARN([  - uwgainfromdb])
#            AC_MSG_WARN([  - uwphysicalracun])
            AC_MSG_WARN([  - wossgmmob3D])
            AC_MSG_WARN([  - wossgroupmob3D])
            AC_MSG_WARN([---------------------------------------------------------------------])
            sed -i -e '/uw-t-lohi/d' ./Makefile.in
#            sed -i -e '/bpsk_db/d' ./Makefile.in
#            sed -i -e '/uwphysical/d' ./Makefile.in
#            sed -i -e '/uwgainfromdb/d' ./Makefile.in
#            sed -i -e '/uwphysicalracun/d' ./Makefile.in
            sed -i -e '/wossgmmob3D/d' ./Makefile.in
            sed -i -e '/wossgroupmob3D/d' ./Makefile.in
        ])
    AC_SUBST(WOSS_CPPFLAGS)
    AC_SUBST(WOSS_LDFLAGS)
    AC_SUBST(WOSS_LIBADD)
])


AC_DEFUN([AC_CHECK_WOSS],
    [
    # temporarily add NS_CPPFLAGS and WOSS_CPPFLAGS to CPPFLAGS
    BACKUP_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $NS_CPPFLAGS $WOSS_CPPFLAGS"

    AC_LANG_PUSH(C++)

    AC_MSG_CHECKING([for woss headers])

    AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([[
            #include<res-reader.h>
            ResReader* r; 
            ]],[[
            ]]  )],
            [
                AC_MSG_RESULT([yes])
                found_woss=yes
                [$1]
            ],
            [
                AC_MSG_RESULT([no])
                found_woss=no
            [$2]
            #AC_MSG_ERROR([could not find woss])
            ])

    AM_CONDITIONAL([HAVE_WOSS], [test x$found_woss = xyes])

    # Restoring to the initial value
    CPPFLAGS="$BACKUP_CPPFLAGS"

    AC_LANG_POP(C++)

    ])

