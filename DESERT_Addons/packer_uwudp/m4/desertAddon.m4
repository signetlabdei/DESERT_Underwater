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




AC_DEFUN([AC_ARG_WITH_DESERT_ADDON],[

    DESERT_ADDON_PATH=''
    DESERT_ADDON_CPPLAGS=''
    DESERT_ADDON_LDFLAGS=''
    DESERT_ADDON_LIBADD=''

    AC_ARG_WITH([desertAddon],
        [AS_HELP_STRING([--with-desertAddon=<directory>],
                [use desert_addon installation in <directory>])],
        [
            if test "x$withval" != "xno" ; then
                if test -d $withval ; then
                    DESERT_ADDON_PATH="${withval}"
                    if test ! -d "${DESERT_ADDON_PATH}" ; then
                    AC_MSG_ERROR([could not find ${withval}, is --with-desertAddon=${withval} correct?])
                fi

                for dir in        
                do
                    echo "considering dir \"$dir\""
                    DESERT_ADDON_CPPFLAGS="$DESERT_ADDON_CPPFLAGS -I${DESERT_ADDON_PATH}/${dir}"
                    DESERT_ADDON_LDFLAGS="$DESERT_ADDON_LDFLAGS -L${DESERT_ADDON_PATH}/${dir}"

                done

                for lib in        
                do
                    DESERT_ADDON_LIBADD="$DESERT_ADDON_LIBADD -l${lib}"
                done

                DESERT_ADDON_DISTCHECK_CONFIGURE_FLAGS="--with-desertAddon=$withval"
                AC_SUBST(DESERT_ADDON_DISTCHECK_CONFIGURE_FLAGS)

                else
                    AC_MSG_ERROR([desertAddon path $withval is not a directory])
                fi
            fi
        ])
        # end AC_ARG_WITH([desert],

    AC_SUBST(DESERT_ADDON_CPPFLAGS)
    AC_SUBST(DESERT_ADDON_LDFLAGS)
    AC_SUBST(DESERT_ADDON_LIBADD)
])

AC_DEFUN([AC_ARG_WITH_DESERT_ADDON_BUILD],[

    DESERT_ADDON_PATH_BUILD=''
    DESERT_ADDON_LDFLAGS_BUILD=''

    AC_ARG_WITH([desertAddon-build],
        [AS_HELP_STRING([--with-desertAddon-build=<directory>],
                [use desert_addon_build installation in <directory>])],
        [
            if test "x$withval" != "xno" ; then
                if test -d $withval ; then
                    DESERT_ADDON_PATH_BUILD="${withval}"
                    if test ! -d "${DESERT_ADDON_PATH_BUILD}" ; then
                        AC_MSG_ERROR([could not find ${withval}, is --with-desertAddon-build=${withval} correct?])
                    fi

                    for dir in        
                    do
                        echo "considering dir \"$dir\""
                        DESERT_ADDON_LDFLAGS_BUILD="$DESERT_ADDON_LDFLAGS_BUILD -L${DESERT_ADDON_PATH_BUILD}/${dir}"
                    done

                else
                    AC_MSG_ERROR([desert addon build path $withval is not a directory])
                fi
            fi
        ])

    AC_SUBST(DESERT_ADDON_LDFLAGS_BUILD)
])

