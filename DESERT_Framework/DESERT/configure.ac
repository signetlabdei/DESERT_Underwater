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


AC_INIT(DESERT,2.0.0)
AM_INIT_AUTOMAKE 
AM_PROG_AR

AC_CONFIG_MACRO_DIR([m4])

AC_MSG_NOTICE([DESERT AC_PACKAGE_VERSION]) 

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

#AC_ARG_WITH_WOSS 
#AC_CHECK_WOSS([have_woss=yes],[have_woss=no])

DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/application/uwcbr'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/application/uwvbr'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/transport/uwudp'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwsun'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwstaticrouting'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwicrp'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwip'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwflooding'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwmll'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-csma-aloha'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwdacap'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwpolling'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwaloha'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwsr'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-t-lohi'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-csma-aloha-triggered'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwUFetch'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwmphy_modem'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/ms2c_evologics'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwmphypatch'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysical'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwgainfromdb'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysicaldb'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uw-al'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uw-al/packer_common'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uw-al/packer_mac'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysicalracun'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/mobility/uwdriftposition'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/mobility/uwgmposition'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/interference/uwinterference'

#if test x$have_woss != xyes ; then
    #AC_MSG_WARN([---------------------------------------------------------------------])
    #AC_MSG_WARN([Could not find WOSS, is --with-woss set correctly?])
    #AC_MSG_WARN([If the --with-woss parameter has been omitted when you ran the configure script])
    #AC_MSG_WARN([this warning can be ignored.])
    #AC_MSG_WARN([---------------------------------------------------------------------])
#fi  


AC_SUBST(DESERT_CPPFLAGS) 

AC_SUBST(DESERT_LDFLAGS) 

AC_SUBST(DESERT_LIBADD) 

AC_DEFINE(CPP_NAMESPACE,std) 

AC_CHECK_LIB(pthread, pthread_create, [LIBS="$LIBS -lpthread"]) 

AC_CONFIG_FILES([
    Makefile
    m4/Makefile
    application/uwcbr/Makefile
    application/uwvbr/Makefile
    transport/uwudp/Makefile
    network/uwsun/Makefile
    network/uwstaticrouting/Makefile
    network/uwicrp/Makefile
    network/uwip/Makefile
    network/uwflooding/Makefile
    data_link/uwmll/Makefile
    data_link/uw-csma-aloha/Makefile
    data_link/uwdacap/Makefile
    data_link/uwpolling/Makefile
    data_link/uwaloha/Makefile
    data_link/uwsr/Makefile
    data_link/uw-t-lohi/Makefile
    data_link/uwUFetch/Makefile
    data_link/uw-csma-aloha-triggered/Makefile
    physical/uwmphy_modem/Makefile
    physical/ms2c_evologics/Makefile
    physical/uwmphypatch/Makefile
    physical/uwphysical/Makefile
    physical/uwgainfromdb/Makefile
    physical/uwphysicaldb/Makefile
    physical/uw-al/Makefile
    physical/uw-al/packer_common/Makefile
    physical/uw-al/packer_mac/Makefile
    mobility/uwdriftposition/Makefile
    mobility/uwgmposition/Makefile
    interference/uwinterference/Makefile
    ])

AC_OUTPUT 
