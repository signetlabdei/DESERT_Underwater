#!/bin/sh

# Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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

# @name_file:   dockert-init.sh
# @author:      Maksym Komar, Emanuele Coccolo
# @last_update: 2022.11.15
# @version:     0.0.1
# @brief_description: docker creation with required OS. Run with args <distro>:<version_tag>, e.g. debian:bullseye

docker_run() {
    docker run -it --rm \
        -v $PWD/../../../:/work \
        -v $HOME/.ssh:/root/.ssh \
        -v $HOME/.git:/root/.git \
        -v $HOME/.bash_history:/root/.bash_history \
        -v $HOME/.screenrc:/root/.screenrc \
        -v $HOME/.tmux.conf:/root/.tmux.conf \
        -w /work \
        --security-opt seccomp:unconfined $@
}

#add check args
if test $# -eq 0; then
    printf "%s\n" "Missing args -> OS version of distro"
    printf "%s\n" "e.g. ./docker-init.sh debian:bullseye"
    return 1
fi

docker_run $@
