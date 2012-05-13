# $Revision: 1.2 $
#
#

IDENT=-DTIMER_TEST -DI386 -DVGA
DEBUG=-DTSKSW_DEBUG

CPP=/lib/cpp
ROOT=../..
CFLAGS=-O2 -I. -I$(ROOT)/kernel/core $(IDENT)
CC=gcc
AS=/usr/bin/as
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=start
LIBS=$(ROOT)/kernel/lib/libkernel/libitron.a $(ROOT)/lib/libc/libc.a
LD=ld

%.o: %.c wconsole.h wconsole_int.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= main.o misc.o missing.o
SRCS= main.c misc.c missing.c

all: 
	${MAKE} wconsole

wconsole: $(OBJS) $(SRCS) Makefile $(LIBS)
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) wconsole


main.o: wconsole.h


-include .depend
