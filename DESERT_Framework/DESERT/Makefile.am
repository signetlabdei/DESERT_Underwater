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


DISTCHECK_CONFIGURE_FLAGS = @NS_ALLINONE_DISTCHECK_CONFIGURE_FLAGS@ \
                            @NSMIRACLE_DISTCHECK_CONFIGURE_FLAGS@ 

ACLOCAL_AMFLAGS = -I m4 

EXTRA_DIST = autogen.sh 

SUBDIRS = m4 \
    interference/uwinterference \
    statistics/uwstats_utilities \
    utility/msg-display \
    propagation/uwem_propagation \
    propagation/uwoptical_propagation \
    mobility/uwdriftposition \
    mobility/uwgmposition \
    mobility/uwrandomlib \
    mobility/uwsmposition \
    physical/uwphy_clmsgs \
    physical/uwphysical \
    physical/uwhmmphysical \
    physical/uwphysicalnoise \
    physical/uwgainfromdb \
    physical/uwphysicaldb\
    physical/uwphysicalrogersmodel\
    physical/uwphysicalfromdb\
    physical/uw-al \
    physical/uw-al/packer_common \
    physical/uw-al/packer_mac \
    physical/uwmodem \
    physical/uwconnector \
    physical/uwmodemcsa \
    physical/uwevologicss2cmodem \
    physical/uwmphypatch \
    physical/uwhermesphy \
    physical/uwofdmphy \
    interference/uwinterferenceofdm \
    physical/uwahoi_phy \
    physical/uwahoimodem \
    physical/uwmodamodem \
    physical/uwem_phy \
    physical/uwem_antenna \
    physical/uwoptical_phy \
    physical/uwopticalbeampattern \
    network/uwip \
    transport/uwudp \
    application/uwcbr \
    application/uwvbr \
    application/uwApplication \
    network/uwstaticrouting \
    network/uwflooding \
    network/uwsun \
    network/uwicrp \
    network/uwPositionBasedRouting \
    data_link/uwmll \
    data_link/uwmmac_clmsgs \
    data_link/uw-csma-aloha \
    data_link/uw-csma-ca \
    data_link/uwdacap \
    data_link/uwpolling \
    data_link/uwaloha \
    data_link/uw-ofdm-aloha \
    data_link/uw-smart-ofdm \
    data_link/uwsr \
    data_link/uw-t-lohi \
    data_link/uwUFetch \
    data_link/uw-aloha-q \
    data_link/uw-aloha-q-sync \
    data_link/uw-csma-aloha-trigger \
    data_link/uwtdma \
    data_link/uwtdma-frame \
    data_link/uwtokenbus \
    data_link/uwjammer \
    channel/uwem_channel \
    channel/uwoptical_channel \
    ranging/uwranging_tokenbus \
    ranging/uwranging_tdma \
    ranging/uwtap \
    ranging/uwranging_tdoa 
