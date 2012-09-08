# $Revision: 1.4 $
#
#

#
# コンパイルフラグ一覧
#
#

IDENT=
DEBUG=

CPP=/lib/cpp
ROOT=../..
CFLAGS=-Wall -I. -I$(ROOT)/kernel -I$(ROOT)/lib/libc/others $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/lib/libc.a
LD=ld
AS=as

.o.c:
	$(CC) $(CFLAGS) -c $< -o $@

.o.s:
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= test.o

all: test psauxtst2

test: test.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) test.o $(LIBS) $(KERNLIBS)

psauxtst2: psauxtst2.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) psauxtst2.o $(LIBS) $(KERNLIBS)

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f test
	rm -f *.o
	rm -f psauxtst2 psauxtst2.o
