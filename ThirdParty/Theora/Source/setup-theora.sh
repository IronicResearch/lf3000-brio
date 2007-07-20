#!/bin/bash

# Build theora libs from source available via SVN.
THEORA_LIB_VER=libtheora-1.0alpha7
THEORA_LIB_SRC=$THEORA_LIB_VER.tar.gz
THEORA_LIB_DIR=Theora

set -e

. $PROJECT_PATH/scripts/functions

# make sure all of the environment variables are good
check_vars

# exit if the user is root
check_user

# parse args
set_standard_opts $*

pushd .

if [ "$CLEAN" == "1" -o ! -e $THEORA_LIB_DIR ]; then
	rm -rf $THEORA_LIB_DIR
	tar -xzf $THEORA_LIB_SRC
	mv $THEORA_LIB_VER $THEORA_LIB_DIR
fi

# build and copy shared libs to rootfs
pushd $THEORA_LIB_DIR
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/usr/local/lib --enable-shared=yes --disable-float --disable-encode --disable-sdltest --without-sdl --with-ogg=$ROOTFS_PATH/usr/local LDFLAGS="-L$ROOTFS_PATH/usr/local/lib -Wl,--rpath-link -Wl,$ROOTFS_PATH/usr/local/lib"
make
# cp -a ./lib/.libs/libtheora.so* $ROOTFS_PATH/usr/local/lib/
# cp -R ./include/theora $ROOTFS_PATH/usr/local/include/
popd

popd

exit 0
