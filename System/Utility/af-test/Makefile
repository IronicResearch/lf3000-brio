all: t1 t2 install

# arm-linux-g++ is for the embedded device; regular g++ is for Ubuntu desktop
CXX=arm-linux-g++ -DNOBRIO
#CXX=g++ -DNOBRIO

t1: t1.cpp AtomicFile.h AtomicFile.o

t2: t2.cpp AtomicFile.h AtomicFile.o

install:
	cp t1 $(ROOTFS_PATH)/usr/local/bin
	cp t2 $(ROOTFS_PATH)/usr/local/bin

clean:
	rm -f t1 t2 *.o *~
