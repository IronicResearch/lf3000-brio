#cc -O -c -I/usr/X11R6/include otk_lib.c -o otk.o                              
#    Link with:    -lGLU -lGL -lXmu -lXext -lX11                                       

# source files.
SRC =  	gadget_lib.cpp letter2vector2.cpp otk_lib.cpp

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


