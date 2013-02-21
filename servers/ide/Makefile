# $Revision: 1.8 $
#
#

#DEBUG=-DDEBUG
IDENT=$(DEBUG)

ROOT=../..
CFLAGS=-O2 -Wall -Werror -I. -I$(ROOT)/include -I$(ROOT)/kernel -I$(ROOT)/lib/libserv $(IDENT)
MKDEPEND=$(CC) -M
STARTADDR=0x80340000
ENTRY=start
LIBS=$(ROOT)/lib/libserv.a $(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a \
	$(ROOT)/kernel/mpu/libmpu.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

OBJS=ide.o idelow.o


all: ide

ide: $(OBJS) $(SRCS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

tags:
	etags *.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) ide


ide.o: ide.h ide.c
