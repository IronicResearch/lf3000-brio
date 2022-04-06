#!/bin/bash

# Build Ogg libs for ARM
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/Didj/Base/Brio --enable-shared=yes
make
cp -a ./src/.libs/libogg.so* ../../Libs/arm

