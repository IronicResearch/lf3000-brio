#!/bin/bash

# Build Vorbis/Tremor fixed-point libs from source available via SVN.
# Patch for memory leak already updated in SVN source.
TREMOR_LIB_DIR=Tremor

set -e

. $PROJECT_PATH/scripts/functions

# make sure all of the environment variables are good
check_vars

# exit if the user is root
check_user

# parse args
set_standard_opts $*

pushd .

if [ "$CLEAN" == "1" -o ! -e $TREMOR_LIB_DIR ]; then
	rm -rf $TREMOR_LIB_DIR
	svn export http://svn.xiph.org/trunk/Tremor/ $TREMOR_LIB_DIR
fi

# build and copy shared libs to rootfs
pushd $TREMOR_LIB_DIR
./autogen.sh --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/Didj/Base/Brio --enable-shared=yes LDFLAGS="-Wl,--rpath-link -Wl,$ROOTFS_PATH/Didj/Base/Brio/lib" 
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/Didj/Base/Brio --enable-shared=yes LDFLAGS="-Wl,--rpath-link -Wl,$ROOTFS_PATH/Didj/Base/Brio/lib"  
make
# make install
# cp -a ./.libs/libvorbisidec.so* $ROOTFS_PATH/usr/local/lib/
# cp    ./*.h $ROOTFS_PATH/usr/local/include/tremor/
popd

popd

exit 0
