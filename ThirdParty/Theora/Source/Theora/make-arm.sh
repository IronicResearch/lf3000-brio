#!/bin/bash

# Build Theora libs for ARM
./configure --host=arm-linux --build=x86-linux --prefix=$ROOTFS_PATH/Didj/Base/Brio --enable-shared=yes --disable-float --disable-encode --disable-sdltest --without-sdl --with-ogg-includes=$BRIO_PATH/ThirdParty/Theora/Include --with-ogg-libraries=$BRIO_PATH/ThirdParty/Theora/Libs/arm LDFLAGS="-L$ROOTFS_PATH/Didj/Base/Brio/lib -Wl,--rpath-link -Wl,$ROOTFS_PATH/Didj/Base/Brio/lib"
make
cp -a ./lib/.libs/libtheora.so* ../../Libs/arm

