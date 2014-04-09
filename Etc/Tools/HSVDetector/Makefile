# basic variables
CC=g++
CFLAGS=-c -Wall
LDFLAGS=

# list of sources
SOURCES=hsvDetect.cpp
OBJECTS=$(SOURCES:.cpp=.o)

# name of executable
EXECUTABLE=hsv_detect

CFLAGS += -g $(shell pkg-config --cflags opencv)
LDFLAGS += -g $(shell pkg-config --libs opencv)

all:$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):$(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
