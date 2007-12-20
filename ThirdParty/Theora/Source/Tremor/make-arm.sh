#!/bin/bash

# Build Ogg Vorbis/Tremor libs for ARM
./autogen.sh --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/Didj/Base/Brio --enable-shared=yes LDFLAGS="-Wl,--rpath-link -Wl,$ROOTFS_PATH/Didj/Base/Brio/lib" 
make
cp -a ./.libs/libvorbisidec.so* ../../Libs/arm

