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

aiff.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
aiff.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
aiff.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/ctype.h
aiff.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_touplow.h sndfile.h
aiff.o: sfendian.h /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
common.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
common.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/ctype.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_touplow.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h sndfile.h
common.o: sfendian.h sfconfig.h config.h
common.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
double64.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
double64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
double64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h sndfile.h
double64.o: sfendian.h /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h
double64.o: common.h float_cast.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
double64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
float32.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
float32.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
float32.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h sndfile.h
float32.o: sfendian.h /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
float32.o: float_cast.h /opt/RidgeRun/arm-eabi-uclibc/include/math.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
float32.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
sndfile.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
sndfile.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/ctype.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_touplow.h
sndfile.o: sndfile.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
sndfile.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h sfendian.h
sndfile.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
strings.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
strings.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
strings.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h sndfile.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h common.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
strings.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h
wav.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
wav.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
wav.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/ctype.h
wav.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_touplow.h sndfile.h
wav.o: sfendian.h /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
wav.o: wav_w64.h
command.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
command.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
command.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h sndfile.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h common.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
command.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h
dither.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
dither.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h sndfile.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
dither.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h sfendian.h
dither.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
file_io.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
file_io.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
file_io.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h sf_unistd.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/fcntl.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/fcntl.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/errno.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/errno.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/errno_values.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/stat.h
file_io.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stat.h sndfile.h
file_io.o: common.h /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h
pcm.o: sfconfig.h config.h sndfile.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
pcm.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
pcm.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h sfendian.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h float_cast.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h common.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
pcm.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
snprintf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
snprintf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/assert.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/errno.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/errno.h
snprintf.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/errno_values.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
sndfileutil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
sndfileutil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
sndfileutil.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h sndfileutil.h
sndfileutil.o: sfconfig.h config.h sndfile.h
w64.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/ctype.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_touplow.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h sndfile.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h sfendian.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h wav_w64.h
wav_w64.o: sfconfig.h config.h /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
wav_w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
wav_w64.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/ctype.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_touplow.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h sndfile.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h sfendian.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdint.h common.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
wav_w64.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h wav_w64.h
