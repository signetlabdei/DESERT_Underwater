#
# Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted ptdmaided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials ptdmaided with the distribution.
# 3. Neither the name of the University of Padova (SIGNET lab) nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PtdmaIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
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

# 1. Define the path where you extracted libtorch. 
# You can hardcode it here, or pass it when running make: `make LIBTORCH_DIR=/path/to/libtorch`

LIBTORCH_DIR ?= $(srcdir)/libtorch

# 2. Define LibTorch compilation and linking flags
TORCH_CPPFLAGS = -I$(LIBTORCH_DIR)/include -I$(LIBTORCH_DIR)/include/torch/csrc/api/include
# The -Wl,-rpath is crucial so the simulation finds the .so files at runtime
TORCH_LDFLAGS = -L$(LIBTORCH_DIR)/lib -Wl,-rpath,$(LIBTORCH_DIR)/lib
TORCH_LIBS = -ltorch -ltorch_cpu -lc10

AM_CXXFLAGS = -Wall -ggdb3 -std=c++17

lib_LTLIBRARIES = libuwmultimodal_rl.la

libuwmultimodal_rl_la_SOURCES = initlib.cc uwmulti-traffic-control.cc uwmulti-multiphy-controller.cpp uwmulti-traffic-range-crt.cc uwmulti-traffic-rl.cpp uwmulti-traffic-rl-clmsg.cpp rl/Agent.cpp rl/DDQNAgent.cpp rl/PPOAgent.cpp rl/DSGSARSAAgent.cpp rl/DSGQlearningAgent.cpp rl/State.cpp 

libuwmultimodal_rl_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ @DESERT_CPPFLAGS@ $(TORCH_CPPFLAGS)
libuwmultimodal_rl_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ @DESERT_LDFLAGS@ @DESERT_LDFLAGS_BUILD@ $(TORCH_LDFLAGS)
libuwmultimodal_rl_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ @DESERT_LIBADD@ $(TORCH_LIBS)

nodist_libuwmultimodal_rl_la_SOURCES = InitTcl.cc

BUILT_SOURCES = InitTcl.cc

CLEANFILES = InitTcl.cc

TCL_FILES =  uwmulti-traffic-control-default.tcl

InitTcl.cc: Makefile $(TCL_FILES)
		cat $(VPATH)/$(TCL_FILES) | @TCL2CPP@ UwMultiTrafficControlInitTclCode > InitTcl.cc

EXTRA_DIST = $(TCL_FILES)
