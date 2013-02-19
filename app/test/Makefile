# $Revision: 1.4 $
#
#

#
# コンパイルフラグ一覧
#
#

IDENT=
DEBUG=

ROOT=../..
CFLAGS=-O3 -Wall -Werror -I. -I$(ROOT)/include -I$(ROOT)/kernel \
		-I$(ROOT)/lib/libc/others $(IDENT)
MKDEPEND=$(CC) -M
STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/lib/libc.a $(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a \
		$(ROOT)/lib/libserv.a

.o.c:
	$(CC) $(CFLAGS) -c $< -o $@

OBJS= test.o

all: test psauxtst2 porttest lowtest rdtest cat

test: test.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) test.o $(LIBS) $(KERNLIBS)

psauxtst2: psauxtst2.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) psauxtst2.o $(LIBS) $(KERNLIBS)

porttest: porttest.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) porttest.o $(LIBS) $(KERNLIBS)

lowtest: lowtest.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) lowtest.o $(LIBS) $(KERNLIBS)

rdtest: rdtest.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) rdtest.o $(LIBS) $(KERNLIBS)

cat: cat.o $(LIBS) $(KERNLIBS)
	$(LD) -Bstatic -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) cat.o $(LIBS) $(KERNLIBS)

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f test
	rm -f *.o
	rm -f psauxtst2
	rm -f porttest
	rm -f lowtest
	rm -f rdtest
	rm -f cat
