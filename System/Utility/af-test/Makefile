all: t1 t2 install

# CXX=arm-linux-g++ -DNOBRIO
CXX=g++ -DNOBRIO

t1: t1.cpp AtomicFile.h AtomicFile.o

t2: t2.cpp AtomicFile.h AtomicFile.o

install:
	cp t1 ~lfu
	cp t2 ~lfu

clean:
	rm -f t1 t2 *.o *~
