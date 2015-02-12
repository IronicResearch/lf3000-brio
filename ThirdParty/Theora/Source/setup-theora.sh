#!/bin/bash

# Build theora libs from source available via SVN.
THEORA_LIB_VER=libtheora-1.0
THEORA_LIB_SRC=$THEORA_LIB_VER.tar.gz
THEORA_LIB_DIR=$THEORA_LIB_VER

set -e

. $PROJECT_PATH/scripts/functions

# make sure all of the environment variables are good
check_vars

# exit if the user is root
check_user

# parse args
set_standard_opts $*

pushd .

if [ ! -e $THEORA_LIB_SRC ]; then
	wget http://downloads.xiph.org/releases/theora/$THEORA_LIB_SRC
fi

if [ "$CLEAN" == "1" -o ! -e $THEORA_LIB_DIR ]; then
	rm -rf $THEORA_LIB_DIR
	tar -xzf $THEORA_LIB_SRC
fi

# build and copy shared libs to rootfs
pushd $THEORA_LIB_DIR
./configure --host=arm-linux --build=x86-linux --prefix=${ROOTFS_PATH}/usr/local --exec-prefix=${ROOTFS_PATH}/usr/local --enable-shared=yes --disable-float --disable-encode --disable-examples --disable-oggtest --disable-vorbistest --disable-sdltest --with-ogg=${ROOTFS_PATH}/usr/local --with-ogg-includes=${ROOTFS_PATH}/usr/local/include --with-ogg-libraries=${ROOTFS_PATH}/usr/local/lib 

# use the LF1000 IDCT instead of the software IDCT
if [ "x$USE_IDCT" != "x" ]; then
	mkdir -p lib/dec/arm
	cp ../state_lf1000.c lib/dec/arm/
	cp ../idct_lf1000.c lib/dec/arm/
	patch -p1 < ../Theora-use-idct.patch
fi

make
make install
cp -a ./lib/.libs/libtheora.so* ../../Libs/arm/ 
cp -R ./include/theora/*.h ../../Include/theora/
popd

popd

exit 0
