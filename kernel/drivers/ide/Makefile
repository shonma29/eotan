# $Revision: 1.8 $
#
#

#DEBUG=-DDEBUG
IDENT=-DI386 -DIBMPC $(DEBUG)

CPP=/lib/cpp
ROOT=../../ITRON
CFLAGS=-O2 -I. -I$(ROOT) -I$(ROOT)/h -I$(ROOT)/i386 -I$(ROOT)/kernlib -fno-builtin $(IDENT)
CC=gcc
AS=/usr/local/bin/as
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x80130000
ENTRY=start
LIBS=$(ROOT)/kernlib/libkernel.a
#LD=/usr/local/bin/ld
LD=ld

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS=ide.o idelow.o misc.o


all: ide

ide: $(OBJS) $(SRCS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)

tags:
	etags *.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) ide


ide.o: ide.h ide.c
