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
#DEBUG=

CPP=/lib/cpp
ROOT=../../..
CFLAGS=-O -Wall -Werror -fno-builtin -I. -I$(ROOT)/include -I$(ROOT)/kernel -I$(ROOT)/lib/libc/others $(IDENT)
CC=gcc

STARTADDR=0x00001000
ENTRY=_main
LIBS=$(ROOT)/lib/libc.a
KERNLIBS=$(ROOT)/lib/libitron.a
LD=ld
AS=as

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= pager.o
IOBJS= 

all: pager

pager: $(OBJS) $(IOBJS) $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) \
		$(IOBJS) $(LIBS) $(KERNLIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

pager.o: pager.c

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS)
	rm -f pager
