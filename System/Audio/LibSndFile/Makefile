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

# DO NOT DELETE

aiff.o: sfconfig.h config.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
aiff.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
aiff.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/ctype.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_touplow.h
aiff.o: sndfile.h sfendian.h
aiff.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
aiff.o: common.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
common.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
common.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/ctype.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_touplow.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
common.o: sndfile.h sfendian.h sfconfig.h config.h
common.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
common.o: common.h
double64.o: sfconfig.h config.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
double64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
double64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
double64.o: sndfile.h sfendian.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
double64.o: common.h float_cast.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
double64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
float32.o: sfconfig.h config.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
float32.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
float32.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
float32.o: sndfile.h sfendian.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
float32.o: common.h float_cast.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
float32.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
sndfile.o: sfconfig.h config.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
sndfile.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/ctype.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_touplow.h
sndfile.o: sndfile.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
sndfile.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
sndfile.o: sfendian.h
sndfile.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
sndfile.o: common.h
strings.o: sfconfig.h config.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
strings.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
strings.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
strings.o: sndfile.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
strings.o: common.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
strings.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
wav.o: sfconfig.h config.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
wav.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
wav.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/ctype.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_touplow.h
wav.o: sndfile.h sfendian.h
wav.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
wav.o: common.h wav_w64.h
command.o: sfconfig.h config.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
command.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
command.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
command.o: sndfile.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
command.o: common.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
command.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
dither.o: sfconfig.h config.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
dither.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
dither.o: sndfile.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
dither.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
dither.o: sfendian.h
dither.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
dither.o: common.h
file_io.o: sfconfig.h config.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
file_io.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
file_io.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
file_io.o: sf_unistd.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/fcntl.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/fcntl.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/errno.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/errno.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/errno_values.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/stat.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stat.h
file_io.o: sndfile.h common.h
file_io.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
pcm.o: sfconfig.h config.h sndfile.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
pcm.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
pcm.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
pcm.o: sfendian.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
pcm.o: float_cast.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
pcm.o: common.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
pcm.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
snprintf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
snprintf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/assert.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/errno.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/errno.h
snprintf.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/errno_values.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
sndfileutil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
sndfileutil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
sndfileutil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
sndfileutil.o: sndfileutil.h sfconfig.h config.h sndfile.h
w64.o: sfconfig.h config.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/ctype.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_touplow.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
w64.o: sndfile.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
w64.o: sfendian.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
w64.o: common.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
w64.o: wav_w64.h
wav_w64.o: sfconfig.h config.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
wav_w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
wav_w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/ctype.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_touplow.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
wav_w64.o: sndfile.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
wav_w64.o: sfendian.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdint.h
wav_w64.o: common.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
wav_w64.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
wav_w64.o: wav_w64.h
