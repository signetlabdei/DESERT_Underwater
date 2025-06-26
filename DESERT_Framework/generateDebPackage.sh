#!/bin/bash

SCRIPT_DIR=$(pwd)
BUILD_DIR=/tmp/desert

rm -rf .unpacked_folder
rm -rf $BUILD_DIR

source commonVariables.sh
./install.sh --target LOCAL --inst_mode release --dest_folder $BUILD_DIR --addons ALL

ARCH=$(dpkg-architecture -q DEB_BUILD_ARCH)
CODENAME=$(lsb_release -sc)

cd $BUILD_DIR
mkdir -p DEBIAN

echo \
"Package: desert
Version: $DESERT_VERSION
Maintainer: SIGNET Research Group
Architecture: $ARCH
Depends: libtool, gfortran, bison, flex
Description: DESERT Underwater is a complete set of public C++ libraries that extend the NS-Miracle simulator to support the design and implementation of underwater network protocols. Its creation stems from the will to push the studies on underwater networking beyond simulations. Implementing research solutions on actual devices, in fact, is of key importance to realize a communication and networking architecture that allows heterogeneous nodes to communicate reliably in the underwater environment." > DEBIAN/control

cd ..

PKG_NAME=desert_${DESERT_VERSION}${CODENAME}_${ARCH}.deb

dpkg-deb --build desert $PKG_NAME

cp /tmp/$PKG_NAME $SCRIPT_DIR
