
# $Revision: 1.14 $
#
#
INC=../include


#DEBUG=-DTSKSW_DEBUG
IDENT=-DAUTO_START $(DEBUG)

CFLAGS=-O2 -I. -I$(INC) -Wall -Werror $(IDENT)
MKDEPEND=$(CC) -M
STARTADDR=0x80100000
ENTRY=startup

OBJS= \
	error.o \
	main.o \
	pmemory.o \
	printk.o \
	api.o \
	thread.o \
	time.o \
	flag.o \
	lowlib.o \
	posix.o \
	ready.o \
	sync.o \
	rendezvous.o

all: version.h core.bin

core.bin: $(OBJS) mpu/libmpu.a arch/libarch.a
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) \
		mpu/startup.o $(OBJS) \
		mpu/libmpu.a arch/libarch.a ../lib/libnc.a 
	strip -s -R .comment -R .note -R .note.ABI-tag $@

version.h: version.sh
	./version.sh > version.h

clean:
	rm -f $(OBJS)
	rm -f version.h
	rm -f core.bin
