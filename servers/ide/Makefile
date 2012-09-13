# $Revision: 1.8 $
#
#

#DEBUG=-DDEBUG
IDENT=-DI386 $(DEBUG)

CPP=/lib/cpp
ROOT=../..
CFLAGS=-O2 -I. -I$(ROOT)/kernel -I$(ROOT)/lib/libserv -fno-builtin $(IDENT)
CC=gcc
AS=/usr/local/bin/as
MKDEPEND=$(CC) -M
STARTADDR=0x80130000
ENTRY=start
LIBS=$(ROOT)/lib/libserv.a $(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a \
	$(ROOT)/kernel/mpu/libmpu.a
LD=ld

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS=ide.o idelow.o


all: ide

ide: $(OBJS) $(SRCS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

tags:
	etags *.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) ide


ide.o: ide.h ide.c
