#!/bin/bash

# Build Ogg Theroa libs for x86
./configure --enable-shared=yes
make
cp -a ./lib/.libs/libtheora.so* ../../Libs/x86

