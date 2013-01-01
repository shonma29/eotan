# $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/Makefile,v 1.9 2000/05/25 08:03:11 naniwa Exp $
#
#
# $Log: Makefile,v $
# Revision 1.9  2000/05/25 08:03:11  naniwa
# minor change
#
# Revision 1.8  2000/01/15 15:27:50  naniwa
# comment out DEBUG option
#
# Revision 1.7  1999/07/30 08:25:05  naniwa
# minor fix
#
# Revision 1.6  1999/04/18 18:43:51  monaka
# Fix for moving Port-manager and libkernel.a  to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
#
# Revision 1.5  1999/03/30 13:20:00  monaka
# Minor changes.
#
# Revision 1.4  1997/10/11 16:24:05  night
# デバッグ用 print 文を DEBUG マクロが定義されているときのみ有効になるよ
# う修正。
#
# Revision 1.3  1997/09/21 13:32:35  night
# コンパイル時にデバッグマクロ (DEBUG) を定義しておくようにした。
#
# Revision 1.2  1997/08/31 13:13:42  night
# とりあえず、OS ファイルに入れるところまでできた。
#
# Revision 1.1  1996/11/11  13:36:04  night
# IBM PC 版への最初の登録
#
# ----------------
#
# Revision 1.4  1995/07/06  15:22:02  night
# ld コマンドを /usr/local/bin/ld.old と定義した。
#
# Revision 1.3  1995/03/18  14:22:31  night
# jmp_user.o mm.o pm.o fm.o の各ファイル用の定義の追加。
#
# Revision 1.2  1995/02/20  15:19:33  night
# 以下のファイルについての記述を追加。
#   global.[co] signal.[co]
#
#
#

.PHONY: clean SYSCALLS

ENTRY=lowlib_init
STARTADDR=0x80290000

#%.o: %.S
#	$(CPP) $(ASFLAGS) $*.S | $(AS) -o $*.o
%.o: %.S
	$(CC) $(CFLAGS) -c $*.S

OBJS =	lowlib.o api.o global.o signal.o jmp_user.o
SRCS =	lowlib.c api.c global.c signal.c jmp_user.S

ROOT=../..
LIBS =	posix-client/libposix-client.a $(ROOT)/lib/libserv.a \
	$(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a

# DEBUG  = -DDEBUG
IDENT  = $(DEBUG)
CFLAGS = -O2 -Wall -Werror -I$(ROOT)/include -I$(ROOT)/kernel $(IDENT)
CC=gcc

all: lowlib

lowlib: $(OBJS) SYSCALLS
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

SYSCALLS:
	$(MAKE) -C posix-client

clean:
	rm -f $(OBJS) lowlib
	$(MAKE) -C posix-client clean

