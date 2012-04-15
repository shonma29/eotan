# $Revision: 1.7 $
#
#

IDENT=-DTIMER_TEST -DI386 -DVGA
DEBUG=-DTSKSW_DEBUG

CPP=/lib/cpp
ROOT=../..
CFLAGS= -Wall -I. -I$(ROOT)/core -I$(ROOT)/lib/libkernel $(IDENT)
OFLAGS= -O2 -fno-builtin $(CFLAGS)
CC=gcc
AS=/usr/bin/as
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x80100000
ENTRY=start
LIBS=$(ROOT)/lib/libkernel/libkernel.a
LD=ld

%.o: %.c
	$(CC) $(OFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= main.o misc.o vga.o utils.o
SRCS= main.c misc.c vga.c utils.c

all: console

console: $(OBJS) $(SRCS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) console


main.o: console.h

utils.o: utils.c
	$(CC) $(CFLAGS) -c $< -o $@

-include .depend
