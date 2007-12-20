#!/bin/bash

# Build Ogg libs for x86
./configure --enable-shared=yes
make
cp -a ./src/.libs/libogg.so* ../../Libs/x86

