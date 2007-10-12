# source files.
SRC = main.cpp sf.cpp

OBJ = $(SRC:.cpp=.o)

OUT = sf2brio
BRIO=/home/lfu/workspace/Brio2
#MOBILEER = $(BRIO)/ThirdParty/Mobileer
BRIO_AUDIO = $(BRIO)/System/Audio

# include directories
INCLUDES = -I. -I/usr/local/include -I$(BRIO_AUDIO)/LibSndFile -I$(BRIO_AUDIO)/DspUtil -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include -I$(BRIO_AUDIO)/Include 
#-I$(MOBILEER)/Include -I$(MOBILEER)/Source/examples
# -I/usr/include/linux
# -I../../../Include

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O2

# compiler
CCC = g++

# library paths
LIBDIRS = -L../ -L/usr/local/lib -L$(BRIO_AUDIO)/LibSndFile/x86 -L$(BRIO_AUDIO)/DspUtil/x86 
#-L$(MOBILEER)/Libs/x86
LIBS = -lsndfile -ldsputil 
#-lme2000

# compile flags (-g)
LDFLAGS = 

.SUFFIXES: .cpp

default: dep $(OUT) 

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) -o $(OUT) $(INCLUDES) $(CCFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJ) $(LIBS) -static


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
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sfconfig.h
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/config.h
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sndfile.h
main.o: /usr/include/sys/time.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/Dsputil.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/util.h
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sndfileutil.h
main.o: main.h /home/lfu/workspace/Brio2/System/Audio/DspUtil/src.h sf.h
sf.o: /usr/include/math.h /usr/include/features.h /usr/include/sys/cdefs.h
sf.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
sf.o: /usr/include/gnu/stubs-32.h /usr/include/bits/huge_val.h
sf.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
sf.o: /usr/include/stdlib.h
sf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
sf.o: /usr/include/sys/types.h /usr/include/bits/types.h
sf.o: /usr/include/bits/typesizes.h /usr/include/time.h /usr/include/endian.h
sf.o: /usr/include/bits/endian.h /usr/include/sys/select.h
sf.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
sf.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
sf.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
sf.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
sf.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
sf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
sf.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h sf.h
sf.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sfconfig.h
sf.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/config.h
sf.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sndfile.h
sf.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/util.h
sf.o: /usr/include/sys/time.h
