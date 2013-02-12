# $Revision: 1.38 $
#
#

#
# コンパイルフラグ一覧
#
#

# -D DOS
# -D USE_LOWLIB		POSIX マネージャを呼び出すとき、lowlib を使用する。
#
IDENT=
DEBUG=

ROOT=../..
CFLAGS=-O2 -fno-builtin -Wall -Werror -I. -I$(ROOT)/include -I$(ROOT)/kernel $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=main
KERNLIBS= $(ROOT)/lib/libserv.a $(ROOT)/lib/libc.a \
	$(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a

LD=ld
AS=as

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

OBJS= main.o device.o posix_if.o lowlib.o

all: init

init: $(OBJS) $(KERNLIBS)
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(KERNLIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

main.o: init.h main.c

device.o: init.h device.c

lowlib.o: lowlib.c

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f init
	rm -f $(OBJS)

