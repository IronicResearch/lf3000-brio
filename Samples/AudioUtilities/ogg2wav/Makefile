# source files
SRC = oggi2wav.cpp
OBJ = $(SRC:.cpp=.o)
OUT = oggi2wav

BRIO=/home/lfu/workspace/Brio2
BRIO_AUDIO = $(BRIO)/System/Audio
#THEORA = $(BRIO)/ThirdParty/Theora
TREMOR2 = $(BRIO_AUDIO)/Tremor2

# include directories
INCLUDES = -I. -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include -I$(TREMOR2) -I$(BRIO_AUDIO)/DspUtil -I$(BRIO_AUDIO)/LibSndFile 
# -I$(THEORA)/Source/examples
# -I/usr/include/linux
# -I$(BRIO_AUDIO)/Include 

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O2

# compiler
CCC = g++

# library paths
LIBDIRS = -L../ -L$(TREMOR2)/x86 -L$(BRIO_AUDIO)/DspUtil/x86 -L$(BRIO_AUDIO)/LibSndFile/x86
# -L$(BRIO_AUDIO)/LibSndFile/x86 -L/usr/local/lib 
LIBS = -lsndfile -ldsputil -lvorbisidec
#LIBS = -lsndfile -ldsputil -lvorbisidec

# compile flags (-g)
LDFLAGS = 

.SUFFIXES: .cpp

default: dep $(OUT) 

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) -o $(OUT) $(INCLUDES) $(CCFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJ) $(LIBS) 
#-static


depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 

# DO NOT DELETE

oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/signal.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
oggi2wav.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/signum.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/siginfo.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigaction.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigcontext.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/asm/sigcontext.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigstack.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigthread.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/fcntl.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/fcntl.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/unistd.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/posix_opt.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/confname.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/getopt.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/assert.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/pthread.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sched.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/initspin.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
oggi2wav.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/linux/rtc.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/ioctl.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/ioctls.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/asm/ioctls.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/asm/ioctl.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/ioctl-types.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/ttydefaults.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/Tremor2/ivorbiscodec.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/Tremor2/ogg.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/Tremor2/os_types.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/Tremor2/config_types.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/Tremor2/ivorbisfile.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sfconfig.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/config.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sndfile.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/Dsputil.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
oggi2wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/util.h
oggi2wav.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sndfileutil.h
