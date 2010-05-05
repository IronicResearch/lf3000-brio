#!/bin/bash

# Build libjpeg lib from source
LIBJPEG_LIB_VER=jpeg-8
LIBJPEG_LIB_SRC=jpegsrc.v8.tar.gz
LIBJPEG_LIB_DIR=jpeg

set -e

. $PROJECT_PATH/scripts/functions

# make sure all of the environment variables are good
check_vars

#exit if the user is root
check_user

# parse args
set_standard_opts $*

pushd .

if [ ! -e $LIBJPEG_LIB_SRC ]; then
	wget http://www.ijg.org/files/$LIBJPEG_LIB_SRC
fi

if [ "$CLEAN" == "1" -o ! -e $LIBJPEG_LIB_DIR ]; then
	rm -rf $LIBJPEG_LIB_DIR
	tar -xzf $LIBJPEG_LIB_SRC
	mv $LIBJPEG_LIB_VER $LIBJPEG_LIB_DIR
fi

# build and copy shared libs to rootfs
pushd $LIBJPEG_LIB_DIR
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/LF/Base/Brio/lib --enable-shared=yes
make clean
make
cp -a .libs/*.so* ../../Libs/arm
./configure --prefix=$ROOTFS_PATH/LF/Base/Brio/lib --enable-shared=yes
make clean
make
cp -a .libs/*.so* ../../Libs/x86
popd

popd

exit 0
