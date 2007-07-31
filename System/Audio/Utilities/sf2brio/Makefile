# source files.
SRC = main.cpp play_midifile.cpp

OBJ = $(SRC:.cpp=.o)

# OUT sf2brio
OUT = sfconvert
MOBILEER = ../../../../ThirdParty/Mobileer

# include directories
INCLUDES = -I. -I../include/ -I/usr/local/include -I../../LibSndFile -I../../DspUtil -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include -I../../../Include -I../../../Audio/Include -I$(MOBILEER)/Include -I$(MOBILEER)/Source/examples
# -I/usr/include/linux

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O2

# compiler
CCC = g++

# library paths
LIBDIRS = -L../ -L/usr/local/lib -L../../LibSndFile/x86 -L../../DspUtil/x86 -L$(MOBILEER)/Libs/x86
LIBS = -lsndfile -ldsputil -lme2000

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
main.o: ../../LibSndFile/sfconfig.h ../../LibSndFile/config.h
main.o: ../../LibSndFile/sndfile.h /usr/include/sys/time.h
main.o: ../../../../ThirdParty/Mobileer/Include/spmidi.h
main.o: ../../../../ThirdParty/Mobileer/Include/spmidi_config.h
main.o: ../../../../ThirdParty/Mobileer/Include/spmidi_errors.h
main.o: ../../DspUtil/Dsputil.h ../../DspUtil/util.h ../../DspUtil/fir.h
main.o: ../../DspUtil/mix.h ../../DspUtil/src.h main.h
play_midifile.o: /usr/include/stdio.h /usr/include/features.h
play_midifile.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
play_midifile.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
play_midifile.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stddef.h
play_midifile.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
play_midifile.o: /usr/include/libio.h /usr/include/_G_config.h
play_midifile.o: /usr/include/wchar.h /usr/include/bits/wchar.h
play_midifile.o: /usr/include/gconv.h
play_midifile.o: /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h
play_midifile.o: /usr/include/bits/stdio_lim.h
play_midifile.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
play_midifile.o: /usr/include/sys/types.h /usr/include/time.h
play_midifile.o: /usr/include/endian.h /usr/include/bits/endian.h
play_midifile.o: /usr/include/sys/select.h /usr/include/bits/select.h
play_midifile.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
play_midifile.o: /usr/include/sys/sysmacros.h
play_midifile.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
play_midifile.o: /usr/include/sys/time.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/spmidi.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/spmidi_config.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/spmidi_errors.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/spmidi_play.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/spmidi_util.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/spmidi_errortext.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/midifile_player.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/midifile_parser.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Include/spmidi_load.h
play_midifile.o: ../../../../ThirdParty/Mobileer/Source/examples/midifile_names.h
play_midifile.o: ../../DspUtil/Dsputil.h /usr/include/math.h
play_midifile.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
play_midifile.o: /usr/include/bits/mathcalls.h ../../DspUtil/util.h main.h
play_midifile.o: ../../DspUtil/src.h
