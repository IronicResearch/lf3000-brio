# source files.
SRC = main.cpp sf.cpp
OBJ = $(SRC:.cpp=.o)

# CPU : x86, arm
CPU = x86
# Compiler: x86: g++   ARM: arm-linux-gcc
CCC = g++

OUT = sf2brio
BRIO=$(HOME)/workspace/Brio2
BRIO_AUDIO = $(BRIO)/ThirdParty

# include directories
INCLUDES = -I. \
    -I/usr/local/include \
    -I$(BRIO)/System/Include \
    -I$(BRIO_AUDIO)/LibSndFile/Include

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O2

# library paths
LIBDIRS = -L../ -L/usr/local/lib -L$(BRIO_AUDIO)/LibSndFile/Libs/$(CPU)
LIBS = -lsndfile #-static

# compile flags (-g)
LDFLAGS = 

.SUFFIXES: .cpp

default: dep $(OUT) 

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) -o $(CPU)/$(OUT) $(INCLUDES) $(CCFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJ) $(LIBS)
#	echo "Copied to ~/nfsroot"
#	cp $(CPU)/$(OUT) $(LEAPFROG_PLUGIN_ROOT)/Tools

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(CPU)/$(OUT) Makefile.bak 


# DO NOT DELETE

main.o: /usr/include/math.h /usr/include/features.h /usr/include/sys/cdefs.h
main.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
main.o: /usr/include/gnu/stubs-32.h /usr/include/bits/huge_val.h
main.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
main.o: /usr/include/stdlib.h /usr/include/sys/types.h
main.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
main.o: /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h
main.o: /usr/include/sys/select.h /usr/include/bits/select.h
main.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
main.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
main.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h
main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
main.o: /home/dmilici/workspace/Brio2/ThirdParty/LibSndFile/Include/sndfile.h
main.o: /usr/include/sys/time.h main.h sf.h
sf.o: /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h
sf.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
sf.o: /usr/include/gnu/stubs-32.h /usr/include/sys/types.h
sf.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
sf.o: /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h
sf.o: /usr/include/sys/select.h /usr/include/bits/select.h
sf.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
sf.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
sf.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
sf.o: /usr/include/_G_config.h /usr/include/wchar.h
sf.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h sf.h
sf.o: /home/dmilici/workspace/Brio2/ThirdParty/LibSndFile/Include/sndfile.h
