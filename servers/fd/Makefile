# $Revision: 1.8 $
#
#

# -DFDDEBUG	... FDC のデバッグ用マクロ
#DEBUG= -DFDDEBUG
IDENT=$(DEBUG)


ROOT=../..
CFLAGS=-O -Wall -Werror -I. -I$(ROOT)/include -I$(ROOT)/kernel -I$(ROOT)/lib/libserv $(IDENT)
MKDEPEND=$(CC) -M
STARTADDR=0x80120000
ENTRY=start
LIBS=$(ROOT)/lib/libserv.a $(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a \
	$(ROOT)/kernel/mpu/libmpu.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $*.s | $(AS) -o $*.o

OBJS= fd.o misc.o version.o spec.o fdc.o dma.o

all: fd765a

fd765a: $(OBJS) $(SRCS) Makefile
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS) fd765a


fd.o: fd.h
fdc.o: fd.h
dma.o: dma.h
