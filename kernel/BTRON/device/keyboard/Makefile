# $Revision: 1.6 $
#
#

IDENT=-DI386
DEBUG=-DTSKSW_DEBUG

CPP=/lib/cpp
ROOT=../../..
COMMON=../../../../include
#CFLAGS=-I. -I$(ROOT) -I$(ROOT)/i386 -I$(ROOT)/h -b i486-linuxaout -I../../kernlib \
#       -fno-builtin $(IDENT)
CFLAGS=-O -I. -I$(ROOT) -I$(ROOT)/ITRON -I$(ROOT)/ITRON/i386 -I$(ROOT)/ITRON/h -I../../kernlib -I$(COMMON) \
       -fno-builtin $(IDENT)
CC=gcc
#AS=/usr/local/bin/as
AS=/usr/bin/as
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x80110000
ENTRY=start
LIBS=$(ROOT)/ITRON/kernlib/libkernel.a
#LD=/usr/local/bin/ld
LD=ld

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= main.o interrupt.o misc.o

all: keyboard

keyboard: $(OBJS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) keyboard


main.o: keyboard.h

interrupt.c: keyboard.h $(COMMON)/key_def.h $(COMMON)/keycode.h
