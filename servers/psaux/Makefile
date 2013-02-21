
IDENT=$(DEBUG)

ROOT=../..

CFLAGS=-O2 -Wall -Werror -I. -I$(ROOT)/include -I$(ROOT)/kernel -I$(ROOT)/lib/libserv $(IDENT)

LIBS=$(ROOT)/lib/libserv.a $(ROOT)/lib/libitron.a $(ROOT)/lib/libnc.a \
	$(ROOT)/kernel/mpu/libmpu.a

STARTADDR=0x80350000
ENTRY=start

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

OBJS=psaux.o  kbc.o 
SRCS=psaux.c  kbc.c

all: psaux

psaux: $(OBJS) 
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@

clean:
	rm -f $(OBJS) psaux



