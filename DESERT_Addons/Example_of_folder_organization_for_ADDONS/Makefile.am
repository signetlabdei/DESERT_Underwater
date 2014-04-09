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

AM_CXXFLAGS = -Wall -ggdb3

lib_LTLIBRARIES = lib<addon-library-name_A>.la lib<addon-library-name_B>.la
check_PROGRAMS = 

TESTS =

#***
# library_A
lib<addon-library-name_A>_la_SOURCES = <src-file-name_01>.cc \
                                       <src-file-name_01>.h  \
                                       <src-file-name_02>.cc \
                                       <src-file-name_02>.h
lib<addon-library-name_A>_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@  @DESERT_CPPFLAGS@
lib<addon-library-name_A>_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@  @DESERT_LDFLAGS@ @DESERT_LDFLAGS_BUILD@
lib<addon-library-name_A>_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@  @DESERT_LIBADD@
#*

#***
# library_B
lib<addon-library-name_B>_la_SOURCES = <src-file-name_03>.cc \
                                       <src-file-name_03>.h
lib<addon-library-name_B>_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ @DESERT_CPPFLAGS@
lib<addon-library-name_B>_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ @DESERT_LDFLAGS@ @DESERT_LDFLAGS_BUILD@
lib<addon-library-name_B>_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ @DESERT_LIBADD@
#*

nodist_lib<addon-library-name_A>_la_SOURCES = <foo>InitTcl.cc
BUILT_SOURCES = <foo>InitTcl.cc
CLEANFILES = <foo>InitTcl.cc

TCL_FILES = <protocoll-name>-default.tcl

<foo>InitTcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ <foo>InitTclCode > <foo>InitTcl.cc

EXTRA_DIST = $(TCL_FILES)
