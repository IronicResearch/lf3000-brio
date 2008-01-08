# Source files.
SRC =  	aiff.c common.c double64.c float32.c sndfile.c strings.c wav.c command.c dither.c file_io.c pcm.c snprintf.c sndfileutil.c w64.c wav_w64.c

OBJ = $(SRC:.c=.o)

# CPU : x86, arm
CPU = x86
# Compiler: x86: g++   ARM: arm-linux-gcc
CCC = g++

OUT = libsndfile.a

# Include directories
INCLUDES = -I. \
        -I../include/ \
        -I/usr/local/include \
        -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include \
        -I/usr/include/sys
# -I/usr/include/linux

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g

# Library paths
LIBS = -L../ -L/usr/local/lib -lm

# Compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

default: dep $(OUT)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(CPU)/$(OUT) $(OBJ)

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) Makefile.bak 
#	rm -f $(OBJ) $(OUT) Makefile.bak 

