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
#

AM_CXXFLAGS = -Wall -ggdb3

lib_LTLIBRARIES = libuwdacap.la libuwdacaptracer.la 

libuwdacap_la_SOURCES = uw-mac-DACAP-alter.h uw-mac-DACAP-alter.cpp \
			   initlib.cc

libuwdacap_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@  
libuwdacap_la_LDFLAGS =  @NS_LDFLAGS@  @NSMIRACLE_LDFLAGS@   
libuwdacap_la_LIBADD =   @NS_LIBADD@   @NSMIRACLE_LIBADD@   


nodist_libuwdacap_la_SOURCES = dacap-embeddedtcl.cc 

libuwdacaptracer_la_SOURCES = uw-mac-DACAP-tracer.cc

libuwdacaptracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ @DESERT_CPPFLAGS@
libuwdacaptracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ @DESERT_LDFLAGS@
libuwdacaptracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ @DESERT_LIBADD@

BUILT_SOURCES = dacap-embeddedtcl.cc 
CLEANFILES = dacap-embeddedtcl.cc

TCL_FILES =  uw-mac-DACAP-defaults.tcl

dacap-embeddedtcl.cc: Makefile $(TCL_FILES)
			cat $(VPATH)/$(TCL_FILES) | @TCL2CPP@ DacapInitTclCode > dacap-embeddedtcl.cc

EXTRA_DIST = $(TCL_FILES)
