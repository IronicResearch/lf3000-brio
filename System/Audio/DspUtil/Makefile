# source files.
SRC =  	Dsputil.cpp Dsputil2.cpp

OBJ = $(SRC:.cpp=.o)

OUT = libdsputil.a

# include directories
INCLUDES = -I. -I../include/ -I/usr/local/include -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include
# -I/usr/include/linux

# C++ compiler flags (-g -O2 -Wall)
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
	rm -f $(OBJ) $(OUT) Makefile.bak 
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
Dsputil.o: /usr/include/bits/mathcalls.h /usr/include/string.h ./Util.h
Dsputil.o: ./Dsputil.h ./Dsputil2.h
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
Dsputil2.o: /usr/include/string.h ./Dsputil.h ./Util.h ./Dsputil2.h
