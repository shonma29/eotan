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
ROOT=../..
CFLAGS=-O2 -fno-builtin -Wall -I. -I$(ROOT) -I$(ROOT)/ITRON -I../ITRON/kernlib -I$(ROOT)/ITRON/h -I$(ROOT)/ITRON/i386 $(IDENT)
CC=gcc
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=start
KERNLIBS=$(ROOT)/ITRON/kernlib/libkernel.a $(ROOT)/POSIX/libc/libnative.a
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

OBJS= start.o main.o device.o posix_if.o #ether.o

all: init

init: $(OBJS) $(LIBS) $(KERNLIBS)
	cd ../lib/frtm; ${MAKE}
	cd lib; ${MAKE}
	cd libCmd; ${MAKE}
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS) $(KERNLIBS)

main.s: main.c
	cc $(CFLAGS) -S main.c

main.o: init.h main.c

device.o: init.h device.c

start.o: init.h start.c

filesystem.o: init.h filesystem.c

lib.o: init.h lib.c

lib/libs.a:
	cd lib; ${MAKE}

libCmd/libCmd.a:
	cd libCmd; ${MAKE}

../lib/frtm/libfrtm.a:
	cd ../lib/frtm; ${MAKE}

dosfs/libdosfs.a:
	cd dosfs; ${MAKE}

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f init
	rm -f $(OBJS)
	rm -f $(ADDOBJS)
	cd lib; ${MAKE} clean
	cd dosfs; ${MAKE} clean
	cd libCmd; ${MAKE} clean
	cd ../lib/frtm; ${MAKE} clean

