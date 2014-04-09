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




AC_DEFUN([AC_ARG_WITH_DESERT],[

    DESERT_PATH=''
    DESERT_CPPLAGS=''
    DESERT_LDFLAGS=''
    DESERT_LIBADD=''

    AC_ARG_WITH([desert],
        [AS_HELP_STRING([--with-desert=<directory>],
                [use desert installation in <directory>])],
        [
            if test "x$withval" != "xno" ; then
                if test -d $withval ; then
                    DESERT_PATH="${withval}"
                    if test ! -d "${DESERT_PATH}" ; then
                    AC_MSG_ERROR([could not find ${withval}, is --with-desert=${withval} correct?])
                fi      

                for dir in  \
                    application/uwcbr \
                    application/uwvbr \
                    application/uwmsg \
                    transport/uwudp \
                    network/uwsun \
                    network/uwmsun \
                    network/uwstaticrouting \
                    network/uwicrp \
                    network/uwip \
                    network/uwflooding \
                    data_link/uw-t-lohi \
                    data_link/uwmll \
                    data_link/uw-csma-aloha \
                    data_link/uw-csma-aloha-triggered \
                    data_link/uwdacap \
                    data_link/uwpolling \
                    data_link/uwaloha \
                    data_link/uwsr \
                    physical/uwmphy_modem \
                    physical/mfsk_whoi_mm \
                    physical/mpsk_whoi_mm \
                    physical/ms2c_evologics \
                    physical/uwmphypatch \
                    physical/uwphysical \
                    physical/uwgainfromdb \
                    mobility/uwdriftposition \
                    mobility/uwgmposition \
                    interference/uwinterference

                do

                    echo "considering dir \"$dir\""
                    DESERT_CPPFLAGS="$DESERT_CPPFLAGS -I${DESERT_PATH}/${dir}"
                    DESERT_LDFLAGS="$DESERT_LDFLAGS -L${DESERT_PATH}/${dir}"

                done

                for lib in \
                    uwgainfromdb \
                    uwmphypatch \
                    uwmphy_modem \
                    uwphysical \
                    uwcbr \ 
                    uwcbrtracer \
                    uwpolling \
                    uwmll \ 
                    uwaloha \
                    uwsr \
                    uwcsmaaloha \
                    uwcsmaalohatriggered \ 
                    uwstaticrouting \
                    uwicrp \
                    uwip \
                    uwflooding \
                    msun \
                    sun
                do
                    DESERT_LIBADD="$DESERT_LIBADD -l${lib}"
                done    

                DESERT_DISTCHECK_CONFIGURE_FLAGS="--with-desert=$withval"
                AC_SUBST(NSMIRACLE_DISTCHECK_CONFIGURE_FLAGS)

                else   
                    AC_MSG_ERROR([desert path $withval is not a directory])
                fi
            fi
        ])
        # end AC_ARG_WITH([desert],

    AC_SUBST(DESERT_CPPFLAGS)
    AC_SUBST(DESERT_LDFLAGS)
    AC_SUBST(DESERT_LIBADD)
])

AC_DEFUN([AC_ARG_WITH_DESERT_BUILD],[

    DESERT_PATH_BUILD=''
    DESERT_LDFLAGS_BUILD=''

    AC_ARG_WITH([desert-build],
        [AS_HELP_STRING([--with-desert-build=<directory>],
                [use desert installation in <directory>])],
        [
            if test "x$withval" != "xno" ; then
                if test -d $withval ; then
                    DESERT_PATH_BUILD="${withval}"
                    if test ! -d "${DESERT_PATH_BUILD}" ; then
                        AC_MSG_ERROR([could not find ${withval}, is --with-desert-build=${withval} correct?])
                    fi

                    for dir in                            \
                        application/uwcbr                 \
                        application/uwvbr                 \
                        application/uwmsg                 \
                        transport/uwudp                   \
                        network/uwsun                     \
                        network/uwmsun                    \
                        network/uwstaticrouting           \
                        network/uwicrp                    \
                        network/uwip                      \
                        network/uwflooding                \
                        data_link/uw-t-lohi               \
                        data_link/uwmll                   \
                        data_link/uw-csma-aloha           \
                        data_link/uw-csma-aloha-triggered \
                        data_link/uwdacap                 \
                        data_link/uwpolling               \
                        data_link/uwaloha                 \
                        data_link/uwsr                    \
                        physical/uwmphy_modem             \
                        physical/mfsk_whoi_mm             \
                        physical/mpsk_whoi_mm             \
                        physical/ms2c_evologics           \
                        physical/uwmphypatch              \
                        physical/uwphysical               \
                        physical/uwgainfromdb             \
                        mobility/uwdriftposition          \
                        mobility/uwgmposition             \
                        interference/uwinterference
                    do
                        echo "considering dir \"$dir\""
                        #DESERT_CPPFLAGS="$DESERT_CPPFLAGS -I${DESERT_PATH}/${dir}"
                        DESERT_LDFLAGS_BUILD="$DESERT_LDFLAGS_BUILD -L${DESERT_PATH_BUILD}/${dir}"
                    done

                else
                    AC_MSG_ERROR([desert path $withval is not a directory])
                fi
            fi
        ])
        # end AC_ARG_WITH([desert],

    #AC_SUBST(DESERT_CPPFLAGS)
    AC_SUBST(DESERT_LDFLAGS_BUILD)
])

AC_DEFUN([AC_CHECK_DESERT],[
    # temporarily add NS_CPPFLAGS and NSMIRACLE_CPPFLAGS to CPPFLAGS
    BACKUP_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $NS_CPPFLAGS $NSMIRACLE_CPPFLAGS"
    
    AC_LANG_PUSH(C++)
    
    AC_MSG_CHECKING([for desert headers])

    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[
                #include<cltracer.h>
                ClMessageTracer* t; 
                ]],[[
                ]]  )],
              [AC_MSG_RESULT([yes])
                found_desert=yes
                [$1]
                ],
              [AC_MSG_RESULT([no])
                found_desert=no
                [$2]
              ])


    AM_CONDITIONAL([HAVE_DESERT], [test x$found_desert = xyes])
    
    # Restoring to the initial value
    CPPFLAGS="$BACKUP_CPPFLAGS"
    
    AC_LANG_POP(C++)
])
