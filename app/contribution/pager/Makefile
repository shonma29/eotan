# $Revision: 1.1 $
#
#

#
# コンパイルフラグ一覧
#
#

#IDENT=
#DEBUG=

ROOT=../../..
CFLAGS=-O3 -Wall -Werror -I. -I$(ROOT)/include -I$(ROOT)/kernel \
		-I$(ROOT)/lib/libc/others $(IDENT)

STARTADDR=0x00001000
ENTRY=_main
LIBS=$(ROOT)/lib/libc.a
KERNLIBS=$(ROOT)/lib/libitron.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

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
