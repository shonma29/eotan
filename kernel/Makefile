# $Revision: 1.14 $
#
#
INC=../include


#DEBUG=-DTSKSW_DEBUG
DEBUG=-DKEYBOARD_DEBUG
IDENT=-DTIMER_TEST -DI386 -DAUTO_START $(DEBUG)

LD=ld
AS=as
CFLAGS=-Os -fno-builtin -I. -Impu -Iarch -I$(INC) -Wall -Werror $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
STARTADDR=0x80020000
ENTRY=startup

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

OBJS= \
	error.o \
	main.o \
	kalloc.o \
	pmemory.o \
	printk.o \
	api.o \
	thread.o \
	time.o \
	message.o \
	flag.o \
	lowlib.o \
	posix.o \
	ready.o \
	sync.o \
	rendezvous.o

all: version.h core.bin

core.bin: $(OBJS) ../lib/libnc.a mpu/libmpu.a arch/libarch.a
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) \
		mpu/startup.o $(OBJS) \
		../lib/libnc.a mpu/libmpu.a arch/libarch.a
	strip -s -R .comment -R .note -R .note.ABI-tag $@

version.h: version.sh
	./version.sh > version.h

../lib/libnc.a:
	${MAKE} -C ../lib/nano-c

mpu/libmpu.a:
	${MAKE} -C mpu

arch/libarch.a:
	${MAKE} -C arch

clean:
	rm -f $(OBJS)
	rm -f version.h
	rm -f core.bin
	${MAKE} -C ../lib/nano-c clean
	${MAKE} -C mpu clean
	${MAKE} -C arch clean
	${MAKE} -C ../servers clean
