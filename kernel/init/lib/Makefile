# $Revision: 1.9 $
#
#

IDENT=-DTIMER_TEST -DI386
DEBUG=-DTSKSW_DEBUG

CPP=/lib/cpp
ROOT=../..
CFLAGS=-O2 -DDEBUG -I -I. -I$(ROOT) -I$(ROOT)/ITRON -I$(ROOT)/ITRON/i386 -I$(ROOT)/ITRON/h -fno-builtin
CC=gcc
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=_start
#LD=/usr/local/bin/ld
#AS=/usr/local/bin/as
LD=ld
AS=as

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= string.o stdlib.o atoi.o

all: libs.a

libs.a: $(OBJS)
	ar rv libs.a $(OBJS)
	ranlib libs.a


tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) libs.a


