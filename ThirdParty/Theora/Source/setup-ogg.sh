#!/bin/bash

# Build ogg libs from source
OGG_LIB_VER=libogg-1.1.4
OGG_LIB_SRC=$OGG_LIB_VER.tar.gz
OGG_LIB_DIR=Ogg

set -e

. $PROJECT_PATH/scripts/functions

# make sure all of the environment variables are good
check_vars

# exit if the user is root
check_user

# parse args
set_standard_opts $*

pushd .

if [ ! -e $OGG_LIB_SRC ]; then
	wget http://downloads.xiph.org/releases/ogg/$OGG_LIB_SRC
fi

if [ "$CLEAN" == "1" -o ! -e $OGG_LIB_DIR ]; then
	rm -rf $OGG_LIB_DIR
	tar -xzf $OGG_LIB_SRC
	mv $OGG_LIB_VER $OGG_LIB_DIR
fi

# build and copy shared libs to rootfs
pushd $OGG_LIB_DIR
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/usr/local --enable-shared=yes
make
make install
cp -a ./src/.libs/libogg.so* ../../Libs/arm/ 
cp -R ./include/ogg/*.h ../../Include/ogg/
popd

popd

exit 0
