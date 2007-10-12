# source files.
SRC =  	Dsputil.cpp Dsputil2.cpp briomixer.cpp eq.cpp fir.cpp mix.cpp src.cpp shape.cpp util.cpp

OBJ = $(SRC:.cpp=.o)

OUT = x86/libdsputil.a

# include directories
INCLUDES = -I. -I../include/ -I/usr/local/include -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include
# -I/usr/include/linux

# C++ compiler flags (-g -O2 -Wall)(-wformat for printf() )
CCFLAGS = -g

# compiler
CCC = g++

# library paths
LIBS = -L../ -L/usr/local/lib -lm

# compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

default: dep $(OUT)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) Makefile.bak 
#	rm -f $(OBJ) $(OUT) Makefile.bak 

# DO NOT DELETE

Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
Dsputil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
Dsputil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
Dsputil.o: ./util.h
Dsputil.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
Dsputil.o: ./Dsputil.h ./Dsputil2.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
Dsputil2.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
Dsputil2.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
Dsputil2.o: ./Dsputil.h ./util.h
Dsputil2.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
Dsputil2.o: ./Dsputil2.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
briomixer.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
briomixer.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
briomixer.o: ./Dsputil.h ./util.h
briomixer.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
briomixer.o: briomixer.h mix.h eq.h shape.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
eq.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h eq.h ./Dsputil.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
eq.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
eq.o: ./util.h /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
eq.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
fir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
fir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
fir.o: ./util.h
fir.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
fir.o: fir.h ./Dsputil.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
mix.o: ./Dsputil.h ./util.h
mix.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
mix.o: mix.h eq.h shape.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
shape.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h shape.h
shape.o: ./Dsputil.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
shape.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
shape.o: ./util.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
shape.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
src.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
src.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
src.o: ./Dsputil.h ./util.h
src.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
src.o: fir.h src.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdio.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/features.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_config.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_arch_features.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/cdefs.h
util.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/types.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wordsize.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/kernel_types.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/typesizes.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/pthreadtypes.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sched.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/uClibc_stdio.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/wchar.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/wchar.h
util.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/stdio_lim.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/stdlib.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/types.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/time.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/endian.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/endian.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/select.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/select.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/sigset.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/time.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/sysmacros.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/alloca.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/math.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/huge_val.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathdef.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/bits/mathcalls.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/string.h
util.o: ./util.h
util.o: /scratchbox/compilers/arm-gcc4.1-uclibc20061004/include/sys/time.h
