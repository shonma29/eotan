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
ROOT=../..
CFLAGS=-O2 -fno-builtin -I. -I$(ROOT) -I$(ROOT)/ITRON -I$(ROOT)/ITRON/kernlib -I$(ROOT)/ITRON/h -I$(ROOT)/ITRON/i386 -I$(ROOT)/../lib/libc/others $(IDENT)
CC=gcc
KP=kp -input-code euc
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/../lib/libc/libc.a $(ROOT)/ITRON/kernlib/libkernel.a
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

OBJS= frtm.o stack.o interp.o funcs.o dict.o demos.o fgraph.o posix.o debug.o
WOBJS= frtmw.o stack.o interpw.o funcs.o dict.o posixw.o fgraphw.o

all: libfrtm.a libfrtm-w.a

libfrtm.a: $(OBJS)
	ar rv libfrtm.a $(OBJS)
	ranlib libfrtm.a

libfrtm-w.a: $(WOBJS)
	ar rv libfrtm-w.a $(WOBJS)
	ranlib libfrtm-w.a

frtm: $(OBJS) $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(IOBJS) $(LIBS) $(KERNLIBS)

frtm.o: frtm.c frtm.h errmes.h
stack.o: stack.c frtm.h
interp.o: interp.c frtm.h funcs.h posix.h
funcs.o: funcs.c frtm.h
dict.o: dict.c frtm.h
demos.o: demos.c frtm.h
fgraph.o: fgraph.c frtm.h
posix.o: frtm.h
debug.o: frtm.h

frtmw.o: frtm.c frtm.h errmes.h
	$(CC) $(CFLAGS) -DWCONSOLE -c frtm.c -o $@
interpw.o: interp.c frtm.h funcs.h
	$(CC) $(CFLAGS) -DWCONSOLE -c interp.c -o $@
fgraphw.o: fgraph.c frtm.h funcs.h
	$(CC) $(CFLAGS) -DWCONSOLE -c fgraph.c -o $@
posixw.o: posix.c frtm.h funcs.h
	$(CC) $(CFLAGS) -DWCONSOLE -c posix.c -o $@

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS)
	rm -f libfrtm.a
	rm -f $(WOBJS)
	rm -f libfrtm-w.a
