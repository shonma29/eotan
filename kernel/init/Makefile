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
IDENT=-DI386 -DUSE_LOWLIB
DEBUG=-DTSKSW_DEBUG

CPP=/lib/cpp
ROOT=..
CFLAGS=-O2 -fno-builtin -Wall -I. -I$(ROOT)/core $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=start
KERNLIBS=$(ROOT)/lib/libkernel/libkernel.a $(ROOT)/../lib/libc/libnative.a
LD=ld
AS=as

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= start.o main.o device.o posix_if.o command.o stdlib.o

all: init

init: $(OBJS) $(KERNLIBS)
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(KERNLIBS)
	strip -s $@

main.o: init.h main.c

device.o: init.h device.c

start.o: init.h start.c

command.o: command.c

stdlib.o: stdlib.c

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f init
	rm -f $(OBJS)

