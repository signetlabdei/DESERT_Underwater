#!/usr/bin/env bash
wget https://github.com/Pro/raspi-toolchain/releases/latest/download/raspi-toolchain.tar.gz  -O /opt/raspi-toolchain.tar.gz
cd /opt/
tar xzf /opt/raspi-toolchain.tar.gz
cd -
cat << EOF > /opt/cross-pi-gcc/environment
export ARCH="arm-linux-gnu"
export CXX=arm-linux-gnueabihf-g++
export PATH=/opt/cross-pi-gcc/bin:/opt/cross-pi-gcc/libexec/gcc/arm-linux-gnueabihf/8.3.0:\$PATH
export CC=arm-linux-gnueabihf-gcc
export STRIP=arm-linux-gnueabihf-strip
EOF
chown -R $(whoami):$(whoami) /opt/cross-pi-gcc/
