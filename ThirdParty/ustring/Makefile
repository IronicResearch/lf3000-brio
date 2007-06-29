CROSS_COMPILE ?= arm-linux-
CXX = $(CROSS_COMPILE)g++
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld
INSTALL_PREFIX ?= /home/lfu/nfsroot/

CPPFLAGS = -I./ -I./include-priv -fpic

OBJS = glibmm/ustring.o glib/gutf8.o glibmm/convert.o \
	glib/guniprop.o glib/gunicollate.o glib/gmem.o glib/gunidecomp.o \
	glib/gmessages.o glib/ghash.o glib/gatomic.o glib/gthread.o glib/gstring.o \
	glib/gstrfuncs.o glib/gutils.o glib/gerror.o glib/gdataset.o glib/gconvert.o \
	glibmm/error.o glib/gtree.o glib/gprimes.o glib/grand.o glib/gmain.o \
	glib/garray.o glib/gslist.o glibmm/exception.o glib/gprintf.o \
	glib/gfileutils.o glib/glist.o glib/gqsort.o glibmm/wrap_init.o \
	glib/giochannel.o glib/gmarkup.o glib/gshell.o glib/gspawn.o

#FIXME/BSK
#LIB = libs/libustring.so
LIB = libustring.so

all: $(LIB)

$(LIB):$(OBJS)
	$(CC) -shared -Wl,-soname,$@,-export-dynamic -o $@ $^ libs/libcharset.a

hello:hello.cc $(LIB)
	$(CXX) $(CPPFLAGS) -L./libs -o $@ $< -lustring -liconv -lintl

install: hello
	cp $< $(INSTALL_PREFIX)
	cp -a libs/*.so* $(INSTALL_PREFIX)/usr/local/lib/

clean:
	rm -f $(OBJS) $(LIB) hello
