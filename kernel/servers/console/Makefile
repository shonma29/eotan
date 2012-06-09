# $Revision: 1.7 $
#
#

IDENT=-DI386
DEBUG=

CPP=/lib/cpp
ROOT=../..
CFLAGS= -Wall -I. -I$(ROOT)/core -I$(ROOT)/lib/libkernel $(IDENT)
OFLAGS= -O2 -fno-builtin $(CFLAGS)
CC=gcc
AS=/usr/bin/as
MKDEPEND=$(CC) -M
STARTADDR=0x80100000
ENTRY=start
LIBS=$(ROOT)/lib/libkernel.a $(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a
LD=ld

%.o: %.c
	$(CC) $(OFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= main.o cga.o
SRCS= main.c cga.c

all: console

console: $(OBJS) $(SRCS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) console


main.o: console.h

-include .depend
