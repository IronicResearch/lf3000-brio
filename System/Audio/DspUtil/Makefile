# source files.
SRC =  	Dsputil.cpp Dsputil2.cpp briomixer.cpp fir.cpp mix.cpp src.cpp util.cpp

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

Dsputil.o: /usr/include/stdio.h /usr/include/features.h
Dsputil.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
Dsputil.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
Dsputil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
Dsputil.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
Dsputil.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
Dsputil.o: /usr/include/bits/wchar.h /usr/include/gconv.h
Dsputil.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
Dsputil.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
Dsputil.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
Dsputil.o: /usr/include/endian.h /usr/include/bits/endian.h
Dsputil.o: /usr/include/sys/select.h /usr/include/bits/select.h
Dsputil.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
Dsputil.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
Dsputil.o: /usr/include/alloca.h /usr/include/math.h
Dsputil.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
Dsputil.o: /usr/include/bits/mathcalls.h /usr/include/string.h ./util.h
Dsputil.o: /usr/include/sys/time.h ./Dsputil.h ./Dsputil2.h
Dsputil2.o: /usr/include/stdio.h /usr/include/features.h
Dsputil2.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
Dsputil2.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
Dsputil2.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
Dsputil2.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
Dsputil2.o: /usr/include/libio.h /usr/include/_G_config.h
Dsputil2.o: /usr/include/wchar.h /usr/include/bits/wchar.h
Dsputil2.o: /usr/include/gconv.h
Dsputil2.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
Dsputil2.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
Dsputil2.o: /usr/include/stdlib.h /usr/include/sys/types.h
Dsputil2.o: /usr/include/time.h /usr/include/endian.h
Dsputil2.o: /usr/include/bits/endian.h /usr/include/sys/select.h
Dsputil2.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
Dsputil2.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
Dsputil2.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
Dsputil2.o: /usr/include/math.h /usr/include/bits/huge_val.h
Dsputil2.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
Dsputil2.o: /usr/include/string.h ./Dsputil.h ./util.h
Dsputil2.o: /usr/include/sys/time.h ./Dsputil2.h
briomixer.o: /usr/include/stdio.h /usr/include/features.h
briomixer.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
briomixer.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
briomixer.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
briomixer.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
briomixer.o: /usr/include/libio.h /usr/include/_G_config.h
briomixer.o: /usr/include/wchar.h /usr/include/bits/wchar.h
briomixer.o: /usr/include/gconv.h
briomixer.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
briomixer.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
briomixer.o: /usr/include/stdlib.h /usr/include/sys/types.h
briomixer.o: /usr/include/time.h /usr/include/endian.h
briomixer.o: /usr/include/bits/endian.h /usr/include/sys/select.h
briomixer.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
briomixer.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
briomixer.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
briomixer.o: /usr/include/math.h /usr/include/bits/huge_val.h
briomixer.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
briomixer.o: ./Dsputil.h ./util.h /usr/include/sys/time.h briomixer.h mix.h
fir.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
fir.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
fir.o: /usr/include/gnu/stubs-32.h
fir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
fir.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
fir.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
fir.o: /usr/include/bits/wchar.h /usr/include/gconv.h
fir.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
fir.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
fir.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
fir.o: /usr/include/endian.h /usr/include/bits/endian.h
fir.o: /usr/include/sys/select.h /usr/include/bits/select.h
fir.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
fir.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
fir.o: /usr/include/alloca.h /usr/include/math.h /usr/include/bits/huge_val.h
fir.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h ./util.h
fir.o: /usr/include/sys/time.h fir.h ./Dsputil.h
mix.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
mix.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
mix.o: /usr/include/gnu/stubs-32.h
mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
mix.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
mix.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
mix.o: /usr/include/bits/wchar.h /usr/include/gconv.h
mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
mix.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mix.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
mix.o: /usr/include/endian.h /usr/include/bits/endian.h
mix.o: /usr/include/sys/select.h /usr/include/bits/select.h
mix.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
mix.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
mix.o: /usr/include/alloca.h /usr/include/math.h /usr/include/bits/huge_val.h
mix.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h ./Dsputil.h
mix.o: ./util.h /usr/include/sys/time.h mix.h
src.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
src.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
src.o: /usr/include/gnu/stubs-32.h
src.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
src.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
src.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
src.o: /usr/include/bits/wchar.h /usr/include/gconv.h
src.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
src.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
src.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
src.o: /usr/include/endian.h /usr/include/bits/endian.h
src.o: /usr/include/sys/select.h /usr/include/bits/select.h
src.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
src.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
src.o: /usr/include/alloca.h /usr/include/math.h /usr/include/bits/huge_val.h
src.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h ./Dsputil.h
src.o: ./util.h /usr/include/sys/time.h fir.h src.h
util.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
util.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
util.o: /usr/include/gnu/stubs-32.h
util.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
util.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
util.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
util.o: /usr/include/bits/wchar.h /usr/include/gconv.h
util.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
util.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
util.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
util.o: /usr/include/endian.h /usr/include/bits/endian.h
util.o: /usr/include/sys/select.h /usr/include/bits/select.h
util.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
util.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
util.o: /usr/include/alloca.h /usr/include/math.h
util.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
util.o: /usr/include/bits/mathcalls.h /usr/include/string.h ./util.h
util.o: /usr/include/sys/time.h
