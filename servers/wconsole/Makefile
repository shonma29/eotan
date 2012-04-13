# $Revision: 1.2 $
#
#

IDENT=-DTIMER_TEST -DI386 -DVGA
DEBUG=-DTSKSW_DEBUG

CPP=/lib/cpp
ROOT=../../kernel
CFLAGS=-O2 -I. -I$(ROOT)/ITRON -I$(ROOT)/ITRON/kernlib -I$(ROOT) $(IDENT)
CC=gcc
#AS=/usr/local/bin/as
AS=/usr/bin/as
KP=kp -input-code euc
MKDEPEND=$(CC) -M
#STARTADDR=0x801A0000
STARTADDR=0x00001000
ENTRY=start
LIBS=$(ROOT)/lib/frtm/libfrtm-w.a $(ROOT)/POSIX/libc/libc.a $(ROOT)/ITRON/kernlib/libitron.a
#LD=/usr/local/bin/ld
LD=ld

%.o: %.c wconsole.h wconsole_int.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= main.o misc.o graphic.o missing.o
SRCS= main.c misc.c graphic.c missing.c

all: 
	${MAKE} -C $(ROOT)/lib/frtm
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
