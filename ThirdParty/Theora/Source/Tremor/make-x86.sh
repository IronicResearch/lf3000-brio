#!/bin/bash

# Build Ogg Vorbis/Tremor libs for x86
./configure --enable-shared=yes
make
cp -a ./.libs/libvorbisidec.so* ../../Libs/x86

