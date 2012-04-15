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

#PATH=.:/bin:/usr/bin:../../tool/kp
CPP=/lib/cpp
ROOT=..
CFLAGS=-O2 -fno-builtin -Wall -I. -I$(ROOT) -I$(ROOT)/ITRON -I$(ROOT)/ITRON/kernlib $(IDENT)
CC=gcc
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=start
KERNLIBS=$(ROOT)/lib/libkernel/libkernel.a $(ROOT)/../lib/libc/libnative.a
LIBS=../lib/frtm/libfrtm.a lib/libs.a libCmd/libCmd.a
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

OBJS= start.o main.o device.o posix_if.o

all: init

init: $(OBJS) $(LIBS) $(KERNLIBS)
	${MAKE} -C ../lib/frtm
	${MAKE} -C lib
	${MAKE} -C libCmd
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS) $(KERNLIBS)

main.s: main.c
	cc $(CFLAGS) -S main.c

main.o: init.h main.c

device.o: init.h device.c

start.o: init.h start.c

filesystem.o: init.h filesystem.c

lib.o: init.h lib.c

lib/libs.a:
	${MAKE} -C lib

libCmd/libCmd.a:
	${MAKE} -C libCmd

../lib/frtm/libfrtm.a:
	${MAKE} -C ../lib/frtm

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f init
	rm -f $(OBJS)
	rm -f $(ADDOBJS)
	${MAKE} -C lib clean
	${MAKE} -C libCmd clean
	${MAKE} -C ../lib/frtm clean

