# $Revision: 1.1 $
#
#

#
# コンパイルフラグ一覧
#
#

# -D DOS
# -D USE_LOWLIB		POSIX マネージャを呼び出すとき、lowlib を使用する。
#
#IDENT=-DI386 -DUSE_LOWLIB
#DEBUG=-DTSKSW_DEBUG

#PATH=.:/bin:/usr/bin:../../tool/kp
CPP=/lib/cpp
ROOT=../../..
CFLAGS=-O -I. -I$(ROOT) -I$(ROOT)/kernel/core -I$(ROOT)/kernel/lib/libkernel -I$(ROOT)/lib/libc/others $(IDENT)
CC=gcc
KP=kp -input-code euc

STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/lib/libc/libc.a
#LD=/usr/local/bin/ld
#AS=/usr/local/bin/as
LD=ld
AS=as

#%.o: %.c
#	kp -input-code euc -input $< -output kptmp.c
#	$(CC) $(CFLAGS) -c kptmp.c -o $@
#	rm -f kptmp.c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= pager.o
IOBJS= 

all: pager

pager: $(OBJS) $(IOBJS) $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(IOBJS) $(LIBS) $(KERNLIBS)
	strip $@

pager.o: pager.c

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS)
	rm -f pager
