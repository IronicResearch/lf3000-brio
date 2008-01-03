# Source files.
SRC =  	Dsputil.cpp Dsputil2.cpp briomixer.cpp eq.cpp fir.cpp iir.cpp mix.cpp src.cpp shape.cpp util.cpp midi.cpp

OBJ = $(SRC:.cpp=.o)

# CPU : x86, arm
CPU = x86
# Compiler: x86: g++   ARM: arm-linux-gcc
CCC = g++

OUT = libdsputil.a

# Include directories
INCLUDES = -I. \
    -I../include/ \
    -I/usr/local/include \
    -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include
# -I/usr/include/linux

# C++ compiler flags (-g -O2 -Wall)(-wformat for printf() )
CCFLAGS = -O2

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
	rm -f $(OBJ) Makefile.bak $(CPU)/$(OUT)

# DO NOT DELETE

Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
Dsputil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
Dsputil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h ./util.h
Dsputil.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h ./Dsputil.h
Dsputil.o: ./Dsputil2.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
Dsputil2.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
Dsputil2.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
Dsputil2.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h ./Dsputil.h
Dsputil2.o: ./util.h /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h
Dsputil2.o: ./Dsputil2.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
briomixer.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
briomixer.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
briomixer.o: ./Dsputil.h ./util.h
briomixer.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h briomixer.h
briomixer.o: mix.h eq.h shape.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
eq.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h eq.h ./Dsputil.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
eq.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h ./util.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
eq.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
fir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
fir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h ./util.h
fir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h fir.h ./Dsputil.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
iir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
iir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h ./util.h
iir.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h iir.h ./Dsputil.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
mix.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h ./Dsputil.h
mix.o: ./util.h /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h mix.h eq.h
mix.o: shape.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
shape.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h shape.h
shape.o: ./Dsputil.h /opt/RidgeRun/arm-eabi-uclibc/include/math.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
shape.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h ./util.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
shape.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
src.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
src.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
src.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h ./Dsputil.h
src.o: ./util.h /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h fir.h src.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdio.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
util.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_stdio.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/wchar.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wchar.h
util.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/stdio_lim.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/stdlib.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/types.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/endian.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/endian.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/sysmacros.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/alloca.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h ./util.h
util.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/string.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/features.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_config.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/uClibc_arch_features.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/cdefs.h
midi.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h midi.h ./util.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/math.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/huge_val.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathdef.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/mathcalls.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/time.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/types.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/wordsize.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/kernel_types.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/typesizes.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/pthreadtypes.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/time.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/time.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/sys/select.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/select.h
midi.o: /opt/RidgeRun/arm-eabi-uclibc/include/bits/sigset.h
