# $Revision: 1.6 $
#
#

IDENT=-DI386
DEBUG=

CPP=/lib/cpp
ROOT=../..
COMMON=$(ROOT)/include
CFLAGS=-O -Wall -Werror -I. -I$(ROOT)/kernel -I$(ROOT)/lib/libserv -I$(COMMON) \
       -fno-builtin $(IDENT)
CC=gcc
AS=/usr/bin/as
MKDEPEND=$(CC) -M
STARTADDR=0x80110000
ENTRY=start
LIBS=$(ROOT)/lib/libserv.a $(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a
LD=ld

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= main.o interrupt.o misc.o

all: keyboard

keyboard: $(OBJS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) keyboard


main.o: keyboard.h

interrupt.c: keyboard.h $(COMMON)/key_def.h $(COMMON)/keycode.h
