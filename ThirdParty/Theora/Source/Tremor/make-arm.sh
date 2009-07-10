#!/bin/bash

# Build Ogg Vorbis/Tremor libs for ARM
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/usr/local --enable-shared=yes 
TARGET_CPU=arm make
cp -a ./.libs/libvorbisidec.so* ../../Libs/arm

