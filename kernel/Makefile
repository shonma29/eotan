# $Revision: 1.14 $
#
#
INC=../include


#DEBUG=-DTSKSW_DEBUG
DEBUG=-DKEYBOARD_DEBUG
IDENT=-DTIMER_TEST -DI386 -DAUTO_START $(DEBUG)

LD=ld
AS=as
CFLAGS=-Os -fno-builtin -I. -Impu -Iarch -I$(INC) -Wall $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
STARTADDR=0x80020000
ENTRY=startup

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

OBJS= \
	startup.o \
	devconfig.o \
	console.o \
	error.o \
	main.o \
	kalloc.o \
	pmemory.o \
	printk.o \
	api.o \
	task.o \
	time.o \
	virtual_mem.o \
	tss.o \
	interrupt.o \
	fpu.o \
	timer.o \
	gdt.o \
	message.o \
	eventflag.o \
	locore.o \
	handler.o  \
	keyboard.o \
	lowlib.o \
	posix.o

SRCS=main.c memory.c task.c time.c version.c error.c dummy.c printk.c \
     console.c interrupt.c syscall.c strncmpk.c timer.c \
     virtual_mem.c memory.c pmemory.c \
     startup.S

all: version.h core.bin

core.bin: $(OBJS) ../lib/libnc.a
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(OTHEROBJ) \
		../lib/libnc.a
	strip -s -R .comment -R .note -R .note.ABI-tag $@

version.h: version.sh
	./version.sh > version.h

../lib/libnc.a:
	${MAKE} -C ../lib/nano-c

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS)
	rm -f version.h
	rm -f core.bin
	${MAKE} -C ../lib/nano-c clean
	${MAKE} -C ../servers clean

include CFILES
include SFILES
