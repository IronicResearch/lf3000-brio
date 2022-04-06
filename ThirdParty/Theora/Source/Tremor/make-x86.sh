#!/bin/bash

# Build Ogg Vorbis/Tremor libs for x86
./configure --enable-shared=yes
TARGET_CPU=x86 make
cp -a ./.libs/libvorbisidec.so* ../../Libs/x86

