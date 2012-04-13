# $Revision: 1.4 $
#
#

#
# コンパイルフラグ一覧
#
#

IDENT=
DEBUG=

#PATH=.:/bin:/usr/bin:../../tool/kp
CPP=/lib/cpp
ROOT=../..
CFLAGS=-Wall -I. -I$(ROOT)/kernel -I$(ROOT)/kernel/ITRON -I$(ROOT)/kernel/ITRON/h -I$(ROOT)/kernel/ITRON/i386 -I$(ROOT)/lib/libc/others $(IDENT)
CC=gcc
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/lib/libc/libc.a #$(ROOT)/kernel/ITRON/kernlib/libitron.a 
#LD=/usr/local/bin/ld
#AS=/usr/local/bin/as
LD=ld
AS=as

#%.o: %.c
#	kp -input-code euc -input $< -output kptmp.c
#	$(CC) $(CFLAGS) -c kptmp.c -o $@
#	rm -f kptmp.c
.o.c:
	$(CC) $(CFLAGS) -c $< -o $@

.o.s:
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= test.o

all: test psauxtst2

test: $(OBJS) $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS) $(KERNLIBS)

psauxtst2: psauxtst2.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) psauxtst2.o $(LIBS) $(KERNLIBS)

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f test
	rm -f $(OBJS)
	rm -f psauxtst2 psauxtst2.o
