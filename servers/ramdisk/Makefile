# $Revision: 1.8 $
#
#

IDENT=-DI386 $(DEBUG)

CPP=/lib/cpp
ROOT=../..
CFLAGS=-O2 -I. -I$(ROOT)/kernel -fno-builtin $(IDENT)
CC=gcc
AS=/usr/bin/as
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=start
LIBS=$(ROOT)/lib/libserv.a $(ROOT)/lib/libc.a \
	$(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a
LD=ld

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= main.o

all: ramdisk

ramdisk: $(OBJS) $(SRCS) $(LIBS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) ramdisk


main.o: ramdisk.h
