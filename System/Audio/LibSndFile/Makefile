# Source files.
SRC =  	aiff.c common.c sndfile.c strings.c pcm.c wav.c command.c file_io.c snprintf.c sndfileutil.c  wav_w64.c w64.c
#  double64.c float32.c  dither.c  
# WAS:    x86: 221482-> 137286  arm: 127140->112364

OBJ = $(SRC:.c=.o)

# CPU : x86, arm
CPU = x86
# Compiler: x86: g++   ARM: arm-linux-gcc
CCC = g++

OUT = libsndfile.a

# Include directories
INCLUDES = -I. \
        -I(ROOTFS_PATH)/usr/local/include \
        -I(ROOTFS_PATH)/usr/include/sys
#        -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include \
# -I/usr/include/linux

# C++ compiler flags (-g -O2 -Wall) -DREDUCED_FORDIDJ -DNO_DITHER -DFORMAT_ALL
CCFLAGS = -O2 -DNO_DITHER -DNO_DOUBLE64
CFLAGS = -O2 -DNO_DITHER -DNO_DOUBLE64
#-DREDUCED_FORDIDJ

# Library paths
LIBS = -L../ -L/usr/local/lib -lm

# Compile flags
LDFLAGS = -O2

.SUFFIXES: .cpp

default: dep $(OUT)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(CPU)/$(OUT) $(OBJ)

depend: dep

dep:
#	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) Makefile.bak $(CPU)/$(OUT) $(CPU)/*.a

