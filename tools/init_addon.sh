#!/bin/sh
# todo: read everything and fix
# todo: add packet header and clmessages

remove_from_list() {
  local list="$1"
  local to_remove="$2"
  local new_list=""
  
  for item in $list; do
    if [ "$item" != "$to_remove" ]; then
      if [ -z "$new_list" ]; then
        new_list="$item"
      else
        new_list="$new_list $item"
      fi
    fi
  done
  
  echo "$new_list"
}

if [ $# -lt 2 ]; then
    echo "Usage: $0 <addon_name> \"<author_name>\""
    exit 1
fi

addon_name=$1
author_name=$2

if [ ! -d "../DESERT_Addons/$addon_name/" ]; then
    rm -rf ../DESERT_Addons/$addon_name; # TODO: remove, used for debug purposes
    cd ../DESERT_Addons/
    mkdir "$addon_name" && cd "$addon_name" || exit 1
else 
    echo "Error, another addon with the same name alredy exists."; exit 1
fi

echo "--- Dependency Configuration ---"
# TODO list available dependencies or something like that
read -p "Enter DESERT module dependencies (e.g., network/uwip transport/uwudp): " module_deps

# TODO better formatting for the depencecies or remove
# echo "$(ls ../../DESERT_Framework/DESERT/)"

for dep in $module_deps; do    
    if [ ! -d "../../DESERT_Framework/DESERT/$dep" ]; then
        module_deps=$(remove_from_list "$module_deps" "$dep")
        read -p "Dependecy $dep does not exist, it will not be installed. Press CTLR+C to EXIT or ENTER to continue." stop
    fi
done

read -p "Enter other ADDON dependencies (e.g., uwrov uwtracker): " addon_deps
for add in $addon_deps; do    
    if [ ! -d "../../DESERT_Addons/$add" ]; then
        addon_deps=$(remove_from_list "$addon_deps" "$add")
        # TODO better exit strategy removing the folder alredy created
        read -p "Dependecy $add does not exist, it will not be installed. Press CTLR+C to EXIT or ENTER to continue." stop
    fi
done

if [ -z "$addon_deps" ]; then
	has_deps=false
else
	has_deps=true
fi

read -p "Does this addon depend on WOSS? [y/N]: " woss_choice

has_woss=false
if [ "$woss_choice" = "y" ] || [ "$woss_choice" = "Y" ]; then
    has_woss=true
fi

current_year=$(date +%Y)

#### ---------------------------- Text blocks ---------------------------- ####
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

copyright_header_cpp=$(echo "$copyright_header_sh" | sed 's/^#/ \/\//g')

### autogen.sh ###
text_autogen_file=$(cat << EOF
#!/bin/sh
${copyright_header_sh}

aclocal -I m4 --force && libtoolize --force && automake --foreign --add-missing && autoconf
EOF
)
### autogen.sh ###

### configure.ac ###
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
if test x\$have_nsmiracle != xyes ; then
  AC_MSG_ERROR([Could not find nsmiracle, is --with-nsmiracle set correctly?])
fi  

AC_ARG_WITH_DESERT
AC_ARG_WITH_DESERT_BUILD

AC_CHECK_DESERT([have_desert=yes],[have_desert=no])
if test x\$have_desert != xyes ; then
  AC_MSG_ERROR([Could not find desert, is --with-desert set correctly?])
fi  

$( [ "$has_deps" = true ] && echo "AC_ARG_WITH_DESERT_ADDON" )
$( [ "$has_deps" = true ] && echo "AC_ARG_WITH_DESERT_ADDON_BUILD" )

$( [ "$has_woss" = true ] && echo "AC_ARG_WITH_WOSS" )

AC_DEFINE(CPP_NAMESPACE,std)

AC_CONFIG_FILES([
    m4/Makefile
    Makefile
])
AC_OUTPUT
EOF
)
### configure.ac ###

### Makefile.am ###
# Logic for Initlib name (First letter uppercase)
addon_fuc="$(echo "$addon_name" | sed 's/./\U&/')"

text_makefile_am=$(cat << EOF
${copyright_header_sh}

AM_CXXFLAGS = -Wall -ggdb3

lib_LTLIBRARIES = lib${addon_name}.la

lib${addon_name}_la_SOURCES = initlib.cpp ${addon_name}.cpp

lib${addon_name}_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ @DESERT_CPPFLAGS@$( [ "$has_deps" = true ] && echo " @DESERT_ADDON_CPPFLAGS@" )$( [ "$has_woss" = true ] && echo " @WOSS_CPPFLAGS@" )
lib${addon_name}_la_LDFLAGS = @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ @DESERT_LDFLAGS@ @DESERT_LDFLAGS_BUILD@$( [ "$has_deps" = true ] && echo " @DESERT_ADDON_LDFLAGS@ @DESERT_ADDON_LDFLAGS_BUILD@" )$( [ "$has_woss" = true ] && echo " @WOSS_LDFLAGS@" )
lib${addon_name}_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ @DESERT_LIBADD@$( [ "$has_deps" = true ] && echo " @DESERT_ADDON_LIBADD@" )$( [ "$has_woss" = true ] && echo " @WOSS_LIBADD@" )

nodist_lib${addon_name}_la_SOURCES = initTcl.cc

BUILT_SOURCES = initTcl.cc

CLEANFILES = initTcl.cc

TCL_FILES =  ${addon_name}-init.tcl

initTcl.cc: Makefile \$(TCL_FILES)
		cat \$(VPATH)/\$(TCL_FILES) | @TCL2CPP@ ${addon_fuc}TclCode > initTcl.cc 

EXTRA_DIST = \$(TCL_FILES)
EOF
)
### Makefile.am ###

### initlib.cpp ###
text_initlib_file=$(cat << EOF
${copyright_header_cpp}

/**
 * @file    initlib.cpp
 * @author  ${author_name}
 * @version 1.0.0
 * \brief Provides the initialization of ${addon_name} libraries.
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
### initlib.cpp ###

#### ---------------------------- Write files ---------------------------- ####
touch "${addon_name}.cpp" "${addon_name}.h"

printf "%s" "$text_autogen_file" > "autogen.sh"
chmod +x autogen.sh

printf "%s" "$text_configure_file" > "configure.ac"

printf "%s" "$text_makefile_am" > "Makefile.am"

printf "%s\n%s\n%s\n%s" "${copyright_header_sh}" \
	"# @file ${addon_name}-init.tcl" \
	"# @author ${author_name}" \
	"# @version 1.0.0" \
	> "${addon_name}-init.tcl"

printf "%s" "$text_initlib_file" > "initlib.cpp"

#### --------------------------- M4 Text blocks --------------------------- ####
mkdir m4

### Makefile.am ###
m4_text_makefile_am=$(cat << EOF
${copyright_header_sh}

EXTRA_DIST = nsallinone.m4 nsmiracle.m4 desert.m4 $( [ "$has_deps" = true ] && echo "desertAddon.m4" ) $( [ "$has_woss" = true ] && echo "woss.m4" )
EOF
)
### Makefile.am ###

### nsallinone.m4 ###
m4_text_nsallinone=$(cat << EOF
${copyright_header_sh}

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
        if test ! -d \$withval ; then
            AC_MSG_ERROR([ns-allinone path \$withval is not valid])
        else

        NS_ALLINONE_PATH=\$withval

        NS_PATH=\$NS_ALLINONE_PATH/\`cd \$NS_ALLINONE_PATH; ls -d ns-* | head -n 1\`
        TCL_PATH=\$NS_ALLINONE_PATH/\`cd \$NS_ALLINONE_PATH; ls -d * | grep -e 'tcl[0-9].*' | head -n 1\`
        TCLCL_PATH=\$NS_ALLINONE_PATH/\`cd \$NS_ALLINONE_PATH; ls -d tclcl-* | head -n 1\`
        OTCL_PATH=\$NS_ALLINONE_PATH/\`cd \$NS_ALLINONE_PATH; ls -d otcl-* | head -n 1\`

        NS_CPPFLAGS="-isystem \$NS_ALLINONE_PATH/include -isystem \$NS_PATH -isystem \$TCLCL_PATH -isystem \$OTCL_PATH"


        NS_ALLINONE_DISTCHECK_CONFIGURE_FLAGS="--with-ns-allinone=\$withval"
        AC_SUBST(NS_ALLINONE_DISTCHECK_CONFIGURE_FLAGS)

        fi
    ])

    if test x\$NS_ALLINONE_PATH = x ;    then
        AC_MSG_ERROR([you must specify ns-allinone installation path using --with-ns-allinone=PATH])
    fi

    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/mac"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/propagation"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/mobile"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/pcap"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/tcp"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/sctp"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/common"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/link"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/queue"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/trace"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/adc"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/apps"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/routing"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/tools"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/classifier"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/mcast"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/diffusion3/lib"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/diffusion3/lib/main"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/diffusion3/lib/nr"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/diffusion3/ns"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/diffusion3/filter_core"
    NS_CPPFLAGS="\$NS_CPPFLAGS -isystem \$NS_PATH/asim"

    AC_SUBST(NS_CPPFLAGS)
    AC_MSG_CHECKING([for NS_LDFLAGS and NS_LIBADD type])

    system=\`uname -s\`
    case \$system in
        CYGWIN*)
            AC_MSG_RESULT([cygwin])
            echo "running cygwin"
            NS_LDFLAGS=" -shared -no-undefined -L\${NS_PATH} -Wl,--export-all-symbols -Wl,--enable-auto-import  -Wl,--whole-archive  "
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
    BACKUP_CPPFLAGS=\$CPPFLAGS
    CPPFLAGS=\$NS_CPPFLAGS

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
          AC_MSG_ERROR([The ns-allinone installation in \$NS_ALLINONE_PATH has not been patched for dynamic libraries. 
                    Either patch it or change the --with-ns-allinone switch so that it refers to a patched version.	])
            ])

    AC_LANG_POP(C++)

    # Restoring to the initial value
    CPPFLAGS=\$BACKUP_CPPFLAGS

    AC_ARG_VAR([TCL2CPP],[tcl2c++ executable])
    AC_PATH_PROG([TCL2CPP],[tcl2c++],[none],[\$PATH:\$TCLCL_PATH])
    if test "x\$TCL2CPP" = "xnone" ;    then
        AC_MSG_ERROR([could not find tcl2c++])
    fi
])
EOF
)
### nsallinone.m4 ###

### nsmiracle.m4 ###
m4_text_nsmiracle=$(cat << EOF
${copyright_header_sh}

AC_DEFUN([AC_ARG_WITH_NSMIRACLE],[
    NSMIRACLE_PATH=''
    NSMIRACLE_CPPFLAGS=''
    NSMIRACLE_LDFLAGS=''
    NSMIRACLE_LIBADD=''

    AC_ARG_WITH([nsmiracle],
        [AS_HELP_STRING([--with-nsmiracle=<directory>],
            [use nsmiracle installation in <directory>])],
        [
            if test "x\$withval" != "xno" ; then
                if test -d \$withval ; then
                    NSMIRACLE_PATH="\${withval}"

                    if test ! -f "\${NSMIRACLE_PATH}/nsmiracle/module.h"  ; then
                        AC_MSG_WARN([could not find \${withval}/nsmiracle/module.h, is --with-nsmiracle=\${withval} correct?])
                    fi

                    for dir in \
                        nsmiracle \
                        cbr \
                        ip \
                        mobility \
                        mphy \
                        mmac \
                        uwm
                    do
                        #echo "considering dir \"\$dir\""
                        NSMIRACLE_CPPFLAGS="\$NSMIRACLE_CPPFLAGS -isystem\${NSMIRACLE_PATH}/\${dir}"
                        NSMIRACLE_LDFLAGS="\$NSMIRACLE_LDFLAGS -L\${NSMIRACLE_PATH}/\${dir}"

                    done

                    for lib in               \
                        MiracleBasicMovement \
                        Miracle              \
                        miraclecbr           \
                        MiracleIp            \
                        mphy                 \
                        mmac                 \
                        UwmStd               \
                        UwmStdPhyBpskTracer
                    do
                        NSMIRACLE_LIBADD="\$NSMIRACLE_LIBADD -l\${lib}"
                    done

                    NSMIRACLE_DISTCHECK_CONFIGURE_FLAGS="--with-nsmiracle=\$withval"
                    AC_SUBST(NSMIRACLE_DISTCHECK_CONFIGURE_FLAGS)

                else
                    AC_MSG_WARN([nsmiracle path \$withval is not a directory])
                fi
            fi
        ])

    AC_SUBST(NSMIRACLE_CPPFLAGS)
    AC_SUBST(NSMIRACLE_LDFLAGS)
    AC_SUBST(NSMIRACLE_LIBADD)
])

AC_DEFUN([AC_CHECK_NSMIRACLE],[
    # if test "x\$NS_CPPFLAGS" = x ; then
    #     true
    #     AC_MSG_ERROR([NS_CPPFLAGS is empty!])
    # fi

    # if test "x\$NSMIRACLE_CPPFLAGS" = x ; then
    #     true
    #     AC_MSG_ERROR([NSMIRACLE_CPPFLAGS is empty!])
    # fi

    # temporarily add NS_CPPFLAGS and NSMIRACLE_CPPFLAGS to CPPFLAGS
    BACKUP_CPPFLAGS="\$CPPFLAGS"
    CPPFLAGS="\$CPPFLAGS \$NS_CPPFLAGS \$NSMIRACLE_CPPFLAGS"

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
            [\$1]
            ],
            [
             AC_MSG_RESULT([no])
             found_nsmiracle=no
            [\$2]
         AC_MSG_WARN([could not find nsmiracle])
            ])

    AM_CONDITIONAL([HAVE_NSMIRACLE], [test x\$found_nsmiracle = xyes])

    # Restoring to the initial value
    CPPFLAGS="\$BACKUP_CPPFLAGS"

    AC_LANG_POP(C++)
])
EOF
)
### nsmiracle.m4 ###

### desert.m4 ###
m4_text_desert=$(cat << EOF
${copyright_header_sh}

AC_DEFUN([AC_ARG_WITH_DESERT],[
    DESERT_PATH=''
    DESERT_CPPFLAGS=''
    DESERT_LDFLAGS=''
    DESERT_LIBADD=''

    AC_ARG_WITH([desert], 
        [AS_HELP_STRING([--with-desert=<directory>],
        [use desert in <directory>])],
        [
            if test "x\$withval" != "xno" ; then

                if test -d \$withval ; then
                    DESERT_PATH="\${withval}"
                else 
                    AC_MSG_ERROR([could not find \${withval}, is --with-desert=\${withval} correct?])
                fi
        
                $([ ! -z "$module_deps" ] && echo "
                    for dir in ${module_deps} ; do 
                        DESERT_CPPFLAGS=\$DESERT_CPPFLAGS\\ -I\${DESERT_PATH}/\${dir} 
                        DESERT_LDFLAGS=\$DESERT_LDFLAGS\\ -L\${DESERT_PATH}/\${dir}
                    done  

                    for lib in $(echo $module_deps | xargs -n1 basename | sed 's/-//g' | xargs) ; do 
                        DESERT_LIBADD=\$DESERT_LIBADD\\ -l\${lib} 
                    done"
                )

                DESERT_DISTCHECK_CONFIGURE_FLAGS="--with-desert=\$withval"
                AC_SUBST(DESERT_DISTCHECK_CONFIGURE_FLAGS)

            fi
        ])
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
            if test "x\$withval" != "xno" ; then

                if test -d \$withval ; then
                    DESERT_PATH_BUILD="\${withval}"

                    $([ ! -z "$module_deps" ] && echo "
                        for dir in ${module_deps} ; do 
                            DESERT_LDFLAGS_BUILD=\"\$DESERT_LDFLAGS_BUILD -L\${DESERT_PATH_BUILD}/\${dir}\" 
                        done"  
                    )
                else 
                    AC_MSG_ERROR([could not find \${withval}, is --with-desert=\${withval} correct?])
                fi
            fi    
        ])

    #AC_SUBST(DESERT_CPPFLAGS)
    AC_SUBST(DESERT_LDFLAGS_BUILD)
])

AC_DEFUN([AC_CHECK_DESERT],[
    BACKUP_CPPFLAGS="\$CPPFLAGS"
    CPPFLAGS="\$CPPFLAGS \$NS_CPPFLAGS \$NSMIRACLE_CPPFLAGS"
    AC_LANG_PUSH(C++)
    AC_MSG_CHECKING([for desert headers])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[
                #include<cltracer.h>
                ClMessageTracer* t;
                ]],[[]])],
        [AC_MSG_RESULT([yes]); found_desert=yes; [\$1]],
        [AC_MSG_RESULT([no]); found_desert=no; [\$2]])
    AM_CONDITIONAL([HAVE_DESERT], [test x\$found_desert = xyes])
    CPPFLAGS="\$BACKUP_CPPFLAGS"
    AC_LANG_POP(C++)
])
EOF
)
### desert.m4 ###

### desertAddon.m4 ###
if [ "$has_deps" = true ]; then

m4_text_desertAddon=$(cat << EOF
${copyright_header_sh}

AC_DEFUN([AC_ARG_WITH_DESERT_ADDON],[
    DESERT_ADDON_PATH=''
    DESERT_ADDON_CPPFLAGS=''
    DESERT_ADDON_LDFLAGS=''
    DESERT_ADDON_LIBADD=''
    
    AC_ARG_WITH([desertAddon], 
        [AS_HELP_STRING([--with-desertAddon=<directory>],
        [use addons in <directory>])], 
        [
            if test "x\$withval" != "xno" ; then
                if test -d \$withval ; then
                    DESERT_ADDON_PATH="\${withval}"
                    
                    $([ ! -z "$addon_deps" ] && echo "
                    for dir in ${addon_deps} ; do 
                        DESERT_ADDON_CPPFLAGS=\"\$DESERT_ADDON_CPPFLAGS -I\${DESERT_ADDON_PATH}/\${dir}\" 
                        DESERT_ADDON_LDFLAGS=\"\$DESERT_ADDON_LDFLAGS -L\${DESERT_ADDON_PATH}/\${dir}/.libs\" 
                        DESERT_ADDON_LIBADD=\"\$DESERT_ADDON_LIBADD -l\${dir}\" 
                    done
                    ")

                    DESERT_ADDON_DISTCHECK_CONFIGURE_FLAGS="--with-desertAddon=\$withval"
                    AC_SUBST(DESERT_ADDON_DISTCHECK_CONFIGURE_FLAGS)
                else 
                    AC_MSG_ERROR([could not find \${withval}, is --with-desertAddon=\${withval} correct?])
                fi
            fi
        ])

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
            if test "x\$withval" != "xno" ; then
                if test -d \$withval ; then
                    DESERT_ADDON_PATH_BUILD="\${withval}"

                    $([ ! -z "$addon_deps" ] && echo "
                        for dir in ${addon_deps} ; do      
                            echo \"considering dir \$dir\"
                            DESERT_ADDON_LDFLAGS_BUILD=\"\$DESERT_ADDON_LDFLAGS_BUILD -L\${DESERT_ADDON_PATH_BUILD}/\${dir}\"
                        done
                     ")
                else   
                    AC_MSG_ERROR([could not find \${withval}, is --with-desertAddon-build=\${withval} correct?])
                fi
            fi
        ])

    AC_SUBST(DESERT_ADDON_LDFLAGS_BUILD)
])
EOF
)
fi
### desertAddon.m4 ###



#### --------------------------- M4 write files --------------------------- ####
printf "%s" "$m4_text_makefile_am" > m4/Makefile.am

printf "%s" "$m4_text_nsallinone" > m4/nsallinone.m4

printf "%s" "$m4_text_nsmiracle" > m4/nsmiracle.m4

printf "%s" "$m4_text_desert" > m4/desert.m4

if [ "$has_deps" = true ]; then
    printf "%s" "$m4_text_desertAddon" > m4/desertAddon.m4
fi


#### ------------------------------- Summary ------------------------------- ####
echo "------------------------------------------------"
echo "Addon '$addon_name' generated by $author_name."
echo "Dependencies: Modules ($module_deps) Addons ($addon_deps) WOSS ($has_woss)"
echo "------------------------------------------------"
