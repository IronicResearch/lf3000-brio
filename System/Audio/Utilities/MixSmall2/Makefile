# source files.
SRC = main.cpp mix_small.cpp mix_smallf.cpp dsputil_mix.cpp

OBJ = $(SRC:.cpp=.o)

OUT = tinymixer

# include directories
INCLUDES = -I. -I/usr/local/include -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include 

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O2

# compiler
CCC = g++

# library paths
LIBDIRS = -L/usr/local/lib 
LIBS = 

# compile flags (-g)
LDFLAGS = 

.SUFFIXES: .cpp

default: dep $(OUT) 

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) -o $(OUT) $(INCLUDES) $(CCFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJ) $(LIBS) 

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 


# DO NOT DELETE

main.o: /usr/include/stdlib.h /usr/include/features.h
main.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
main.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
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
main.o: /usr/include/ctype.h mix_common.h /usr/include/math.h
main.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
main.o: /usr/include/bits/mathcalls.h dsputil_mix.h mix_small.h mix_smallf.h
mix_small.o: /usr/include/stdio.h /usr/include/features.h
mix_small.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
mix_small.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
mix_small.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
mix_small.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
mix_small.o: /usr/include/libio.h /usr/include/_G_config.h
mix_small.o: /usr/include/wchar.h /usr/include/bits/wchar.h
mix_small.o: /usr/include/gconv.h
mix_small.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
mix_small.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mix_small.o: /usr/include/stdlib.h /usr/include/sys/types.h
mix_small.o: /usr/include/time.h /usr/include/endian.h
mix_small.o: /usr/include/bits/endian.h /usr/include/sys/select.h
mix_small.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
mix_small.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
mix_small.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
mix_small.o: /usr/include/math.h /usr/include/bits/huge_val.h
mix_small.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
mix_small.o: /usr/include/strings.h mix_small.h dsputil_mix.h mix_common.h
mix_smallf.o: /usr/include/stdio.h /usr/include/features.h
mix_smallf.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
mix_smallf.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
mix_smallf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
mix_smallf.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
mix_smallf.o: /usr/include/libio.h /usr/include/_G_config.h
mix_smallf.o: /usr/include/wchar.h /usr/include/bits/wchar.h
mix_smallf.o: /usr/include/gconv.h
mix_smallf.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
mix_smallf.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mix_smallf.o: /usr/include/stdlib.h /usr/include/sys/types.h
mix_smallf.o: /usr/include/time.h /usr/include/endian.h
mix_smallf.o: /usr/include/bits/endian.h /usr/include/sys/select.h
mix_smallf.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
mix_smallf.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
mix_smallf.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
mix_smallf.o: /usr/include/math.h /usr/include/bits/huge_val.h
mix_smallf.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
mix_smallf.o: mix_common.h dsputil_mix.h mix_smallf.h
dsputil_mix.o: /usr/include/stdio.h /usr/include/features.h
dsputil_mix.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
dsputil_mix.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
dsputil_mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
dsputil_mix.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
dsputil_mix.o: /usr/include/libio.h /usr/include/_G_config.h
dsputil_mix.o: /usr/include/wchar.h /usr/include/bits/wchar.h
dsputil_mix.o: /usr/include/gconv.h
dsputil_mix.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
dsputil_mix.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
dsputil_mix.o: /usr/include/stdlib.h /usr/include/sys/types.h
dsputil_mix.o: /usr/include/time.h /usr/include/endian.h
dsputil_mix.o: /usr/include/bits/endian.h /usr/include/sys/select.h
dsputil_mix.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
dsputil_mix.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
dsputil_mix.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
dsputil_mix.o: /usr/include/math.h /usr/include/bits/huge_val.h
dsputil_mix.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
dsputil_mix.o: /usr/include/strings.h dsputil_mix.h
