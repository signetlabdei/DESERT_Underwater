#
# Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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


AC_INIT(DESERT,3.3.0)
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
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/application/uwApplication'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/transport/uwudp'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwsun'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwstaticrouting'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwflooding'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwicrp'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwip'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/network/uwPositionBasedRouting'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwmmac_clmsgs'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwmll'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-csma-aloha'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-csma-ca'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwdacap'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwpolling'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwaloha'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-ofdm-aloha'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-smart-ofdm'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwsr'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-t-lohi'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uw-csma-aloha-trigger'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwUFetch'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwtdma'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwtdma-frame'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/data_link/uwtokenbus'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwmodem'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwmodemcsa'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwconnector'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwevologicss2cmodem'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwmphypatch'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysical'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwhmmphysical'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysicalrogersmodel'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwgainfromdb'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysicaldb'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysicalfromdb'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uw-al'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uw-al/packer_common'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uw-al/packer_mac'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphysicalracun'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwhermesphy'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwofdmphy'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwahoi_phy'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwahoimodem'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwoptical_phy'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwopticalbeampattern'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwphy_clmsgs'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/utility/msg-display'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/mobility/uwdriftposition'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/mobility/uwgmposition'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/mobility/uwrandomlib'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/mobility/uwsmposition'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/interference/uwinterference'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/interference/uwinterferenceofdm'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/propagation/uwoptical_propagation'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/channel/uwoptical_channel'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/statistics/uwstats_utilities'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/ranging/uwranging_tokenbus'
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/ranging/uwranging_tdma'


DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'$(GCC6_FLAG)'

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
    application/uwApplication/Makefile
    transport/uwudp/Makefile
    network/uwsun/Makefile
    network/uwstaticrouting/Makefile
    network/uwflooding/Makefile
    network/uwicrp/Makefile
    network/uwip/Makefile
    network/uwPositionBasedRouting/Makefile
    data_link/uwmmac_clmsgs/Makefile
    data_link/uwmll/Makefile
    data_link/uw-csma-aloha/Makefile
    data_link/uw-csma-ca/Makefile
    data_link/uwdacap/Makefile
    data_link/uwpolling/Makefile
    data_link/uwaloha/Makefile
    data_link/uw-ofdm-aloha/Makefile
    data_link/uw-smart-ofdm/Makefile
    data_link/uwsr/Makefile
    data_link/uw-t-lohi/Makefile
    data_link/uwUFetch/Makefile
    data_link/uw-csma-aloha-trigger/Makefile
    data_link/uwtdma/Makefile
    data_link/uwtdma-frame/Makefile
    data_link/uwtokenbus/Makefile
    physical/uwmodem/Makefile
    physical/uwmodemcsa/Makefile
    physical/uwconnector/Makefile
    physical/uwevologicss2cmodem/Makefile
    physical/uwmphypatch/Makefile
    physical/uwphysical/Makefile
    physical/uwhmmphysical/Makefile
    physical/uwphysicalrogersmodel/Makefile
    physical/uwgainfromdb/Makefile
    physical/uwphysicaldb/Makefile
    physical/uwphysicalfromdb/Makefile
    physical/uw-al/Makefile
    physical/uw-al/packer_common/Makefile
    physical/uw-al/packer_mac/Makefile    
    physical/uwhermesphy/Makefile
    physical/uwofdmphy/Makefile
    physical/uwahoi_phy/Makefile
    physical/uwahoimodem/Makefile
    physical/uwoptical_phy/Makefile
    physical/uwopticalbeampattern/Makefile
    physical/uwphy_clmsgs/Makefile
    utility/msg-display/Makefile
    mobility/uwdriftposition/Makefile
    mobility/uwgmposition/Makefile
    mobility/uwrandomlib/Makefile
    mobility/uwsmposition/Makefile
    interference/uwinterference/Makefile
    interference/uwinterferenceofdm/Makefile
    propagation/uwoptical_propagation/Makefile
    channel/uwoptical_channel/Makefile
    statistics/uwstats_utilities/Makefile
    ranging/uwranging_tokenbus/Makefile
    ranging/uwranging_tdma/Makefile
    ])

AC_OUTPUT
