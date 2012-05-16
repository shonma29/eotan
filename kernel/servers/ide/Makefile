# $Revision: 1.8 $
#
#

#DEBUG=-DDEBUG
IDENT=-DI386 -DIBMPC $(DEBUG)

CPP=/lib/cpp
ROOT=../..
CFLAGS=-O2 -I. -I$(ROOT)/core -I$(ROOT)/lib/libkernel -fno-builtin $(IDENT)
CC=gcc
AS=/usr/local/bin/as
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x80130000
ENTRY=start
LIBS=$(ROOT)/lib/libkernel/libkernel.a
LD=ld

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS=ide.o idelow.o misc.o


all: ide

ide: $(OBJS) $(SRCS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s $@

tags:
	etags *.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) ide


ide.o: ide.h ide.c
