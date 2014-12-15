# Source files.
SRC = main.cpp

OBJ = $(SRC:.cpp=.o)
# CPU : x86, arm
CPU = x86
# Compiler: x86: g++   ARM: arm-linux-gcc
CCC = g++

OUT = sfinfo
BRIO=/home/lfu/workspace/Brio2
MOBILEER   = $(BRIO)/ThirdParty/Mobileer
BRIO_AUDIO = $(BRIO)/System/Audio

# Include directories
INCLUDES = -I. \
    -I/usr/local/include \
    -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include \
    -I$(BRIO_AUDIO)/LibSndFile \
    -I$(BRIO_AUDIO)/DspUtil \
    -I$(BRIO_AUDIO)/Include \
    -I$(MOBILEER)/Include \
    -I$(MOBILEER)/Source/examples
# -I/usr/include/linux
# -I../../../Include

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g

# Library paths
LIBDIRS = -L../ -L/usr/local/lib -L$(BRIO_AUDIO)/LibSndFile/$(CPU) -L$(BRIO_AUDIO)/DspUtil/$(CPU) -L$(MOBILEER)/Libs/$(CPU)
LIBS = -lsndfile -ldsputil

# compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

default: dep $(OUT)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) -o $(OUT) $(INCLUDES) $(CCFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJ) $(LIBS) -static
	cp $(OUT) ./$(CPU)

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 

# DO NOT DELETE

main.o: /usr/include/math.h /usr/include/features.h /usr/include/sys/cdefs.h
main.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
main.o: /usr/include/gnu/stubs-32.h /usr/include/bits/huge_val.h
main.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
main.o: /usr/include/stdlib.h
main.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
main.o: /usr/include/sys/types.h /usr/include/bits/types.h
main.o: /usr/include/bits/typesizes.h /usr/include/time.h
main.o: /usr/include/endian.h /usr/include/bits/endian.h
main.o: /usr/include/sys/select.h /usr/include/bits/select.h
main.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
main.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
main.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h
main.o: /usr/include/bits/wchar.h /usr/include/gconv.h
main.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/Dsputil.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/util.h
main.o: /usr/include/sys/time.h
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sfconfig.h
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/config.h
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sndfile.h
