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
ROOT=../../../..
CFLAGS=-I. -I$(ROOT)/kernel/core -I$(ROOT)/kernel/lib/libkernel -I$(ROOT)/lib/libc/others $(IDENT) -fno-builtin-printf
CC=gcc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/lib/libc/libgt.a $(ROOT)/lib/libc/libc.a #$(ROOT)/kernel/lib/libkernel/libitron.a
LD=ld
AS=as

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= frtm.o stack.o interp.o funcs.o dict.o demos.o posix.o debug.o
IOBJS= 

all: frtm

frtm: $(OBJS) $(IOBJS) $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(IOBJS) $(LIBS) $(KERNLIBS)

frtm.o: frtm.c frtm.h errmes.h
stack.o: stack.c frtm.h
interp.o: interp.c frtm.h funcs.h posix.h
funcs.o: funcs.c frtm.h
dict.o: dict.c frtm.h
demos.o: demos.c frtm.h
posix.o: frtm.h
debug.o: frtm.h

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS)
	rm -f frtm
