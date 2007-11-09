# source files.
SRC = main.cpp play_midifile.cpp

OBJ = $(SRC:.cpp=.o)

# CPU : x86, arm
CPU = x86
# Compiler: x86: g++   ARM: arm-linux-gcc
CCC = g++

OUT = sfconvert
BRIO=/home/lfu/workspace/Brio2
MOBILEER = $(BRIO)/ThirdParty/Mobileer
BRIO_AUDIO = $(BRIO)/System/Audio

# include directories
INCLUDES = -I. \
    -I/usr/local/include \
    -I/usr/lib/gcc/i486-linux-gnu/4.1.2/include \
    -I$(BRIO_AUDIO)/LibSndFile \
    -I$(BRIO_AUDIO)/DspUtil \
    -I$(BRIO_AUDIO)/Include \
    -I$(MOBILEER)/Include \
    -I$(MOBILEER)/Source/examples
# -I/usr/include/linux
# -I../../../Include

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O2


# library paths
LIBDIRS = -L../ -L/usr/local/lib -L$(BRIO_AUDIO)/LibSndFile/$(CPU) -L$(BRIO_AUDIO)/DspUtil/$(CPU) -L$(MOBILEER)/Libs/$(CPU)
LIBS = -lsndfile -ldsputil -lme2000

# compile flags (-g)
LDFLAGS = 

.SUFFIXES: .cpp

default: dep $(OUT) 

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) -o $(OUT) $(INCLUDES) $(CCFLAGS) $(LDFLAGS) $(LIBDIRS) $(OBJ) $(LIBS) -static
	cp $(OUT) ./$(CPU)

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
main.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi.h
main.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_config.h
main.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_errors.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/Dsputil.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/util.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/eq.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/fir.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/iir.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/mix.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/shape.h
main.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/src.h main.h sf.h
main.o: /home/lfu/workspace/Brio2/System/Audio/LibSndFile/sndfileutil.h
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
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_config.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_errors.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_play.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_util.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_errortext.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/midifile_player.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/midifile_parser.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Include/spmidi_load.h
play_midifile.o: /home/lfu/workspace/Brio2/ThirdParty/Mobileer/Source/examples/midifile_names.h
play_midifile.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/Dsputil.h
play_midifile.o: /usr/include/math.h /usr/include/bits/huge_val.h
play_midifile.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
play_midifile.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/util.h main.h
play_midifile.o: /home/lfu/workspace/Brio2/System/Audio/DspUtil/src.h
