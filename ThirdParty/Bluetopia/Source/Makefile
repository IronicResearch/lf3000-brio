
ifndef CC
   CC = gcc
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = $(HOME)/workspace/LinuxDist/packages/ss1/BluetopiaPM
endif

INCLDDIRS = -I. -I../Include -I$(BLUETOPIA_PATH)/include           \
	    -I$(BLUETOPIA_PATH)/include/client                     \
	    -I$(BLUETOPIA_PATH)/Bluetopia/include                  \
	    -I$(BLUETOPIA_PATH)/Bluetopia/profiles/GATT/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CXXFLAGS = -fPIC -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 $(GLOBCFLAGS)

LDFLAGS = -L$(BLUETOPIA_PATH)/lib/client -L$(BLUETOPIA_PATH)/Bluetopia/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPM $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = BluetopiaIO.o BTAddr.o

LIB = libBluetopiaIO.so

TEST = Loader

.PHONY:
all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared -Wl,-soname,$(LIB),-export-dynamic -o $(LIB) $(OBJS) -Wl,$(LDLIBS) -Wl,$(LDFLAGS)

$(TEST): $(LIB)
	$(CXX) -I. -DTEST_HARNESS -o $(TEST) $(TEST).cpp -ldl 
	
.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm *.o
	-rm *.so

