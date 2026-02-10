#!/bin/sh
#TODO:
# - ask for module deps
# - ask for other addons deps
#	- check if woss dep
# - ask for packet header
# - ask for clmessage

if [ $# -lt 2 ]; then
	echo "Insert name of the addon and author (in quotes)."
    exit 1
fi
addon_name=$1
author_name=$2

cd ../DESERT_Addons/
rm -rf $addon_name
mkdir $addon_name && cd $addon_name || exit 1

current_year=$(date +%Y)
copyright_header_sh=$(cat << EOF
#
# Copyright (c) ${current_year} Regents of the SIGNET lab, University of Padova.
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
EOF
)

copyright_header_cpp=$(cat << EOF
//
// Copyright (c) ${current_year} Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the 
//    names of its contributors may be used to endorse or promote products 
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
EOF
)

text_autogen_file=$(cat << EOF
#!/bin/sh
${copyright_header_sh}


aclocal -I m4 --force && libtoolize --force && automake --foreign --add-missing && autoconf
EOF
)

text_addon_init_file=$(cat << EOF
${copyright_header_sh}
# @file	${addon_name}-init.tcl
# @author	${author_name}
# @version	1.0.0
EOF
)

text_configure_file=$(cat << EOF
${copyright_header_sh}

AC_INIT(${addon_name}, 1.0.0)
AM_INIT_AUTOMAKE
AM_PROG_AR

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_MAKE_SET

AC_DISABLE_STATIC
 
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

AC_PATH_NS_ALLINONE

AC_ARG_WITH_NSMIRACLE

AC_CHECK_NSMIRACLE([have_nsmiracle=yes],[have_nsmiracle=no])
if test x$have_nsmiracle != xyes ; then
  AC_MSG_ERROR([Could not find nsmiracle, is --with-nsmiracle set correctly?])
fi  

AC_ARG_WITH_DESERT
AC_ARG_WITH_DESERT_BUILD

AC_CHECK_DESERT([have_desert=yes],[have_desert=no])
if test x$have_desert != xyes ; then
  AC_MSG_ERROR([Could not find desert, is --with-desert set correctly?])
fi  


AC_DEFINE(CPP_NAMESPACE,std)

AC_CONFIG_FILES([
		m4/Makefile
		Makefile
      ])

AC_OUTPUT
EOF
)

# TODO: this should change depending on the packets and clmessages
addon_fuc="$(echo "$addon_name" | sed 's/./\U&/')"
text_initlib_file=$(cat << EOF
${copyright_header_cpp}

/**
 * @file    initlib.cpp
 * @author  ${author_name}
 * @version 1.0.0
 *
 * \brief Provides the initialization of ${addon_name} libraries.
 *
 * Provides the initialization of uwtracker libraries. In addition,
 * it provides both <i>$(echo "$addon_name" | tr '[:lower:]' '[:upper:]')</i> monitoring and control packets header
 * description.
 *
 */

#include <tclcl.h>

extern EmbeddedTcl ${addon_fuc}TclCode;

extern "C" int
${addon_fuc}_Init()
{
    ${addon_fuc}TclCode.load();
    return 0;
}
EOF
)

touch "${addon_name}.cpp" "${addon_name}.h" "Makefile.am" "initlib.cpp" "${addon_name}-init.tcl" "configure.ac" "autogen.sh" || exit 1

# autogen.sh
printf "%s" "$text_autogen_file" > "autogen.sh"
chmod +x autogen.sh

# configure.ac
printf "%s" "$text_configure_file" > configure.ac

# addon-init.tcl
printf "%s" "$text_addon_init_file" > ${addon_name}-init.tcl

printf "%s" "$text_initlib_file" > initlib.cpp

#TODO: write makefile and m4s

mkdir m4 && cd m4/
touch "Makefile.am" "nsallinone.m4" "nsmiracle.m4" "desert.m4"
