# Source files.
SRC = Dsputil.cpp fir.cpp src.cpp shape.cpp util.cpp midi.cpp
# eq.cpp Dsputil2.cpp 

OBJ = $(SRC:.cpp=.o)

# CPU : x86, arm
CPU = x86
# Compiler: x86: g++   ARM: arm-linux-gcc
CCC = g++

OUT = libdsputil.a

# Include directories
INCLUDES = -I. \
    -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include 
#    -I$(ROOTFS_PATH)/usr/include/linux
#   DOESN'T LIKE THIS -I$(ROOTFS_PATH)/usr/include \

# C++ compiler flags (-g -O2 -Wall)(-wformat for printf() )
CCFLAGS = -O2
CFLAGS = -O2

# Library paths
LIBS = -L../ -L$(ROOTFS_PATH)/usr/local/lib -lm

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
#	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) Makefile.bak $(CPU)/$(OUT)

