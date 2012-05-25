CFLAGS=-Wall

all: mksfs statfs sfsck

mksfs: mkfs.o
	cc $(CFLAGS) -o mksfs mkfs.o

statfs: statfs.o
	cc $(CFLAGS) -o statfs statfs.o

clean:
	rm -f statfs mksfs sfsck
	rm -f *.o
